// Discord Rich Presence over the Discord client's local pipe (\\.\pipe\discord-ipc-0..9 on Windows, the Unix socket
// discord-ipc-0..9 in $XDG_RUNTIME_DIR, $TMPDIR or /tmp and their Flatpak and Snap folders elsewhere; an 8-byte header
// of opcode and length before each JSON payload). A worker thread does all the pipe I/O: the handshake with the application id,
// the retries while Discord is not running (15 s before the first connection, 5 s after), the latest activity as
// SET_ACTIVITY, the answers to pings and CLOSE when stopped. The game thread only stores the activity under a lock.
//
// A C port of Akhenaten's src/platform/discord_rpc.cpp (https://github.com/dalerank/Akhenaten, commit 8ec540693), with
// the pipe, the lock, the events and the thread on Windows or POSIX. Copyright (C) the Akhenaten authors. Licence: GNU Affero General Public License v3.0
// (LICENSES/AGPL-3.0.txt).

#include "platform/discord_rpc.h"

#include "platform/log.h"

#if defined(_WIN32)
#include <windows.h>
#else
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#endif
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <wchar.h>

enum discord_opcode {
    DISCORD_OPCODE_HANDSHAKE = 0,
    DISCORD_OPCODE_FRAME = 1,
    DISCORD_OPCODE_CLOSE = 2,
    DISCORD_OPCODE_PING = 3,
    DISCORD_OPCODE_PONG = 4,
};

enum discord_sizes {
    DISCORD_PIPE_COUNT = 10,
    DISCORD_PIPE_PATH_SIZE = 108,
    DISCORD_HEADER_SIZE = 8,
    DISCORD_LENGTH_OFFSET = 4,
    DISCORD_UTF8_TEXT_SIZE = DISCORD_RPC_TEXT_SIZE * 3,
    DISCORD_JSON_TEXT_SIZE = DISCORD_UTF8_TEXT_SIZE * 2,
    DISCORD_PACKET_SIZE = 2048,
    DISCORD_INCOMING_KEPT = 1024,
    DISCORD_INCOMING_MAX = 65536,
    DISCORD_LOG_LINE_SIZE = 2400,
    DISCORD_MIN_FIELD_LENGTH = 2,
};

enum discord_timing_ms {
    DISCORD_READ_TIMEOUT_MS = 2000,
    DISCORD_READ_POLL_MS = 10,
    DISCORD_FIRST_RETRY_MS = 15000,
    DISCORD_RETRY_MS = 5000,
    DISCORD_IDLE_WAIT_MS = 500,
    DISCORD_STOP_WAIT_MS = 4000,
};

typedef struct discord_activity {
    char details[DISCORD_RPC_TEXT_SIZE];
    char state[DISCORD_RPC_TEXT_SIZE];
} discord_activity;

typedef struct discord_packet {
    uint32_t opcode;
    uint32_t length;
    char payload[DISCORD_INCOMING_KEPT + 1];
} discord_packet;

typedef struct discord_rpc_state {
    bool verbose;
    bool running;
#if defined(_WIN32)
    CRITICAL_SECTION lock;
    HANDLE stop_event;
    HANDLE activity_event;
    HANDLE thread;
    HANDLE pipe;
#else
    pthread_mutex_t lock;
    pthread_cond_t wake;
    bool stop_signalled;
    bool activity_signalled;
    bool thread_started;
    bool thread_done;
    pthread_t thread;
    int pipe;
#endif
    bool activity_pending;
    bool activity_known;
    discord_activity activity;
    char app_id[DISCORD_RPC_APP_ID_SIZE];
    int64_t start_time;
    uint32_t nonce;
} discord_rpc_state;

static discord_rpc_state discord;

static void discord_log(const char *format, ...)
{
    if (!discord.verbose) {
        return;
    }
    char line[DISCORD_LOG_LINE_SIZE];
    va_list args;
    va_start(args, format);
    vsnprintf(line, sizeof(line), format, args);
    va_end(args);
    platform_log("discord: %s", line);
}

#if defined(_WIN32)
static bool pipe_connected(void)
{
    return discord.pipe != INVALID_HANDLE_VALUE;
}

static void pipe_close(void)
{
    if (pipe_connected()) {
        CloseHandle(discord.pipe);
    }
    discord.pipe = INVALID_HANDLE_VALUE;
}

static bool pipe_open_index(int32_t index)
{
    wchar_t path[DISCORD_PIPE_PATH_SIZE];
    swprintf(path, DISCORD_PIPE_PATH_SIZE, L"\\\\.\\pipe\\discord-ipc-%d", index);
    HANDLE pipe = CreateFileW(path, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (pipe == INVALID_HANDLE_VALUE) {
        return false;
    }
    discord.pipe = pipe;
    return true;
}

static bool pipe_available(uint32_t *available)
{
    DWORD bytes = 0;
    bool ok = PeekNamedPipe(discord.pipe, NULL, 0, NULL, &bytes, NULL) != 0;
    *available = bytes;
    return ok;
}

static bool pipe_read(void *data, uint32_t wanted, uint32_t *read)
{
    DWORD bytes = 0;
    bool ok = ReadFile(discord.pipe, data, wanted, &bytes, NULL) && bytes > 0;
    *read = bytes;
    return ok;
}

static bool pipe_write(const void *data, uint32_t size)
{
    DWORD written = 0;
    return WriteFile(discord.pipe, data, size, &written, NULL) && written == size;
}

static uint32_t clock_ms(void)
{
    return GetTickCount();
}

static void sleep_ms(uint32_t ms)
{
    Sleep(ms);
}

static unsigned long process_id(void)
{
    return GetCurrentProcessId();
}

static void text_to_utf8(const char *text, char *utf8, int32_t size)
{
    wchar_t wide[DISCORD_RPC_TEXT_SIZE];
    int32_t wide_length = MultiByteToWideChar(CP_ACP, 0, text, -1, wide, DISCORD_RPC_TEXT_SIZE);
    if (wide_length <= 0 || WideCharToMultiByte(CP_UTF8, 0, wide, -1, utf8, size, NULL, NULL) <= 0) {
        utf8[0] = 0;
    }
}

static void lock_enter(void)
{
    EnterCriticalSection(&discord.lock);
}

static void lock_leave(void)
{
    LeaveCriticalSection(&discord.lock);
}

static bool stop_requested(void)
{
    return WaitForSingleObject(discord.stop_event, 0) == WAIT_OBJECT_0;
}

static void stop_wait(uint32_t ms)
{
    WaitForSingleObject(discord.stop_event, ms);
}

static bool wake_wait_stops(uint32_t ms)
{
    HANDLE wake[] = { discord.stop_event, discord.activity_event };
    return WaitForMultipleObjects(ARRAYSIZE(wake), wake, FALSE, ms) == WAIT_OBJECT_0;
}

static void activity_signal(void)
{
    SetEvent(discord.activity_event);
}

static void stop_signal(void)
{
    SetEvent(discord.stop_event);
}

static void handle_close(HANDLE *handle)
{
    if (*handle) {
        CloseHandle(*handle);
    }
    *handle = NULL;
}

static void worker_release(void)
{
    handle_close(&discord.thread);
    handle_close(&discord.stop_event);
    handle_close(&discord.activity_event);
    DeleteCriticalSection(&discord.lock);
}

static void worker_run(void);

static DWORD WINAPI worker_thread(LPVOID unused)
{
    worker_run();
    return 0;
}

static bool worker_start(void)
{
    discord.pipe = INVALID_HANDLE_VALUE;
    InitializeCriticalSection(&discord.lock);
    discord.stop_event = CreateEventW(NULL, TRUE, FALSE, NULL);
    discord.activity_event = CreateEventW(NULL, FALSE, FALSE, NULL);
    if (discord.stop_event && discord.activity_event) {
        discord.thread = CreateThread(NULL, 0, worker_thread, NULL, 0, NULL);
    }
    if (!discord.thread) {
        worker_release();
        return false;
    }
    return true;
}

static bool worker_join(void)
{
    return WaitForSingleObject(discord.thread, DISCORD_STOP_WAIT_MS) == WAIT_OBJECT_0;
}
#else
static bool pipe_connected(void)
{
    return discord.pipe >= 0;
}

static void pipe_close(void)
{
    if (pipe_connected()) {
        close(discord.pipe);
    }
    discord.pipe = -1;
}

static bool socket_open(const char *dir, int32_t index)
{
    struct sockaddr_un address;
    memset(&address, 0, sizeof(address));
    address.sun_family = AF_UNIX;
    int written = snprintf(address.sun_path, sizeof(address.sun_path), "%s/discord-ipc-%d", dir, index);
    if (written <= 0 || (size_t) written >= sizeof(address.sun_path)) {
        return false;
    }
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) {
        return false;
    }
#if defined(SO_NOSIGPIPE)
    int one = 1;
    setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, &one, sizeof(one));
#endif
    if (connect(fd, (const struct sockaddr *) &address, sizeof(address)) != 0) {
        close(fd);
        return false;
    }
    discord.pipe = fd;
    return true;
}

static bool pipe_open_index(int32_t index)
{
    static const char *const subfolders[] = { "", "/app/com.discordapp.Discord", "/snap.discord", "/app/com.discordapp.DiscordCanary" };
    const char *roots[] = { getenv("XDG_RUNTIME_DIR"), getenv("TMPDIR"), getenv("TMP"), getenv("TEMP"), "/tmp" };
    for (size_t r = 0; r < sizeof(roots) / sizeof(roots[0]); r++) {
        if (!roots[r] || !roots[r][0]) {
            continue;
        }
        for (size_t f = 0; f < sizeof(subfolders) / sizeof(subfolders[0]); f++) {
            char dir[DISCORD_PIPE_PATH_SIZE];
            snprintf(dir, sizeof(dir), "%s%s", roots[r], subfolders[f]);
            if (socket_open(dir, index)) {
                return true;
            }
        }
    }
    return false;
}

static bool pipe_available(uint32_t *available)
{
    int bytes = 0;
    if (ioctl(discord.pipe, FIONREAD, &bytes) != 0) {
        return false;
    }
    if (bytes == 0) {
        char probe;
        ssize_t peeked = recv(discord.pipe, &probe, 1, MSG_PEEK | MSG_DONTWAIT);
        if (peeked == 0 || (peeked < 0 && errno != EAGAIN && errno != EWOULDBLOCK)) {
            return false;
        }
    }
    *available = (uint32_t) bytes;
    return true;
}

static bool pipe_read(void *data, uint32_t wanted, uint32_t *read)
{
    ssize_t bytes = recv(discord.pipe, data, wanted, 0);
    *read = bytes > 0 ? (uint32_t) bytes : 0;
    return bytes > 0;
}

static bool pipe_write(const void *data, uint32_t size)
{
#if defined(MSG_NOSIGNAL)
    int flags = MSG_NOSIGNAL;
#else
    int flags = 0;
#endif
    const uint8_t *at = data;
    while (size > 0) {
        ssize_t written = send(discord.pipe, at, size, flags);
        if (written <= 0) {
            return false;
        }
        at += written;
        size -= (uint32_t) written;
    }
    return true;
}

static uint32_t clock_ms(void)
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (uint32_t) ((uint64_t) now.tv_sec * 1000u + (uint64_t) now.tv_nsec / 1000000u);
}

static void sleep_ms(uint32_t ms)
{
    struct timespec pause = { (time_t) (ms / 1000u), (long) (ms % 1000u) * 1000000L };
    nanosleep(&pause, NULL);
}

static unsigned long process_id(void)
{
    return (unsigned long) getpid();
}

static void text_to_utf8(const char *text, char *utf8, int32_t size)
{
    int32_t at = 0;
    for (const unsigned char *c = (const unsigned char *) text; *c; c++) {
        int32_t needed = *c < 0x80 ? 1 : 2;
        if (at + needed >= size) {
            break;
        }
        if (*c < 0x80) {
            utf8[at++] = (char) *c;
        } else {
            utf8[at++] = (char) (0xC0 | (*c >> 6));
            utf8[at++] = (char) (0x80 | (*c & 0x3F));
        }
    }
    utf8[at] = 0;
}

static void lock_enter(void)
{
    pthread_mutex_lock(&discord.lock);
}

static void lock_leave(void)
{
    pthread_mutex_unlock(&discord.lock);
}

static bool stop_requested(void)
{
    lock_enter();
    bool stop = discord.stop_signalled;
    lock_leave();
    return stop;
}

static struct timespec deadline_after(uint32_t ms)
{
    struct timespec deadline;
    clock_gettime(CLOCK_REALTIME, &deadline);
    deadline.tv_sec += (time_t) (ms / 1000u);
    deadline.tv_nsec += (long) (ms % 1000u) * 1000000L;
    if (deadline.tv_nsec >= 1000000000L) {
        deadline.tv_sec++;
        deadline.tv_nsec -= 1000000000L;
    }
    return deadline;
}

static void stop_wait(uint32_t ms)
{
    struct timespec deadline = deadline_after(ms);
    lock_enter();
    while (!discord.stop_signalled && pthread_cond_timedwait(&discord.wake, &discord.lock, &deadline) == 0) {
    }
    lock_leave();
}

static bool wake_wait_stops(uint32_t ms)
{
    struct timespec deadline = deadline_after(ms);
    lock_enter();
    while (!discord.stop_signalled && !discord.activity_signalled && pthread_cond_timedwait(&discord.wake, &discord.lock, &deadline) == 0) {
    }
    bool stop = discord.stop_signalled;
    discord.activity_signalled = false;
    lock_leave();
    return stop;
}

static void activity_signal(void)
{
    lock_enter();
    discord.activity_signalled = true;
    pthread_cond_broadcast(&discord.wake);
    lock_leave();
}

static void stop_signal(void)
{
    lock_enter();
    discord.stop_signalled = true;
    pthread_cond_broadcast(&discord.wake);
    lock_leave();
}

static void worker_release(void)
{
    pthread_cond_destroy(&discord.wake);
    pthread_mutex_destroy(&discord.lock);
    discord.thread_started = false;
}

static void worker_run(void);

static void *worker_thread(void *unused)
{
    worker_run();
    lock_enter();
    discord.thread_done = true;
    pthread_cond_broadcast(&discord.wake);
    lock_leave();
    return NULL;
}

static bool worker_start(void)
{
    discord.pipe = -1;
    discord.stop_signalled = false;
    discord.activity_signalled = false;
    discord.thread_done = false;
    pthread_mutex_init(&discord.lock, NULL);
    pthread_cond_init(&discord.wake, NULL);
    discord.thread_started = pthread_create(&discord.thread, NULL, worker_thread, NULL) == 0;
    if (!discord.thread_started) {
        worker_release();
        return false;
    }
    return true;
}

static bool worker_join(void)
{
    struct timespec deadline = deadline_after(DISCORD_STOP_WAIT_MS);
    lock_enter();
    while (!discord.thread_done && pthread_cond_timedwait(&discord.wake, &discord.lock, &deadline) == 0) {
    }
    bool done = discord.thread_done;
    lock_leave();
    if (done) {
        pthread_join(discord.thread, NULL);
    } else {
        pthread_detach(discord.thread);
    }
    return done;
}
#endif

static void connection_lost(const char *reason)
{
    platform_log("discord: connection lost (%s)", reason);
    pipe_close();
}

static bool pipe_open(void)
{
    for (int32_t i = 0; i < DISCORD_PIPE_COUNT; i++) {
        if (pipe_open_index(i)) {
            discord_log("opened the pipe discord-ipc-%d", i);
            return true;
        }
    }
    discord_log("no discord-ipc pipe (Discord is not running)");
    return false;
}

static bool pipe_read_exact(void *data, uint32_t length)
{
    uint8_t *at = data;
    uint32_t started = clock_ms();
    while (length > 0) {
        uint32_t available = 0;
        if (!pipe_available(&available)) {
            return false;
        }
        if (available == 0) {
            if (clock_ms() - started >= DISCORD_READ_TIMEOUT_MS) {
                return false;
            }
            sleep_ms(DISCORD_READ_POLL_MS);
            continue;
        }
        uint32_t wanted = length < available ? length : available;
        uint32_t read = 0;
        if (!pipe_read(at, wanted, &read)) {
            return false;
        }
        at += read;
        length -= read;
    }
    return true;
}

static void put_u32(uint8_t *at, uint32_t value)
{
    at[0] = (uint8_t) value;
    at[1] = (uint8_t) (value >> 8);
    at[2] = (uint8_t) (value >> 16);
    at[3] = (uint8_t) (value >> 24);
}

static uint32_t get_u32(const uint8_t *at)
{
    return (uint32_t) at[0] | ((uint32_t) at[1] << 8) | ((uint32_t) at[2] << 16) | ((uint32_t) at[3] << 24);
}

static bool packet_send(uint32_t opcode, const char *json)
{
    uint8_t packet[DISCORD_HEADER_SIZE + DISCORD_PACKET_SIZE];
    size_t length = strlen(json);
    if (length > DISCORD_PACKET_SIZE) {
        return false;
    }
    put_u32(packet, opcode);
    put_u32(packet + DISCORD_LENGTH_OFFSET, (uint32_t) length);
    memcpy(packet + DISCORD_HEADER_SIZE, json, length);
    return pipe_write(packet, (uint32_t) (DISCORD_HEADER_SIZE + length));
}

static bool payload_skip(uint32_t length)
{
    char discarded[DISCORD_INCOMING_KEPT];
    while (length > 0) {
        uint32_t chunk = length < sizeof(discarded) ? length : (uint32_t) sizeof(discarded);
        if (!pipe_read_exact(discarded, chunk)) {
            return false;
        }
        length -= chunk;
    }
    return true;
}

static bool packet_read(discord_packet *packet)
{
    uint8_t header[DISCORD_HEADER_SIZE];
    if (!pipe_read_exact(header, sizeof(header))) {
        return false;
    }
    packet->opcode = get_u32(header);
    packet->length = get_u32(header + DISCORD_LENGTH_OFFSET);
    if (packet->length > DISCORD_INCOMING_MAX) {
        return false;
    }
    uint32_t kept = packet->length < DISCORD_INCOMING_KEPT ? packet->length : DISCORD_INCOMING_KEPT;
    if (!pipe_read_exact(packet->payload, kept)) {
        return false;
    }
    packet->payload[kept] = 0;
    return payload_skip(packet->length - kept);
}

static void packet_answer(const discord_packet *packet)
{
    discord_log("incoming opcode %u: %.512s", packet->opcode, packet->payload);
    if (packet->opcode == DISCORD_OPCODE_PING) {
        if (!packet_send(DISCORD_OPCODE_PONG, packet->payload)) {
            connection_lost("the pong was not written");
        }
    } else if (packet->opcode == DISCORD_OPCODE_CLOSE) {
        connection_lost("Discord closed it");
    }
}

static void incoming_drain(void)
{
    while (pipe_connected()) {
        uint32_t available = 0;
        if (!pipe_available(&available)) {
            connection_lost("the pipe is closed");
            return;
        }
        if (available < DISCORD_HEADER_SIZE) {
            return;
        }
        discord_packet packet;
        if (!packet_read(&packet)) {
            connection_lost("an incoming packet was not read");
            return;
        }
        packet_answer(&packet);
    }
}

static void json_escape(const char *text, char *escaped, size_t size)
{
    size_t at = 0;
    for (; *text && at + 2 < size; text++) {
        char c = *text;
        if (c == '"' || c == '\\') {
            escaped[at++] = '\\';
            escaped[at++] = c;
        } else if ((unsigned char) c >= ' ') {
            escaped[at++] = c;
        }
    }
    escaped[at] = 0;
}

static bool handshake(void)
{
    char app_id[DISCORD_RPC_APP_ID_SIZE * 2];
    char json[DISCORD_PACKET_SIZE];
    json_escape(discord.app_id, app_id, sizeof(app_id));
    snprintf(json, sizeof(json), "{\"v\":1,\"client_id\":\"%s\"}", app_id);
    discord_log("handshake %s", json);
    if (!packet_send(DISCORD_OPCODE_HANDSHAKE, json)) {
        discord_log("the handshake was not written");
        return false;
    }
    discord_packet ready;
    if (!packet_read(&ready)) {
        discord_log("no answer to the handshake");
        return false;
    }
    discord_log("handshake answer opcode %u: %.256s", ready.opcode, ready.payload);
    return ready.opcode == DISCORD_OPCODE_FRAME;
}

static bool connect_to_discord(void)
{
    if (!pipe_open()) {
        return false;
    }
    if (!handshake()) {
        pipe_close();
        return false;
    }
    platform_log("discord: connected");
    return true;
}

static void json_append(char *json, size_t size, const char *format, ...)
{
    size_t length = strlen(json);
    va_list args;
    va_start(args, format);
    vsnprintf(json + length, size - length, format, args);
    va_end(args);
}

static void json_append_text(char *json, size_t size, const char *name, const char *text)
{
    char utf8[DISCORD_UTF8_TEXT_SIZE];
    char escaped[DISCORD_JSON_TEXT_SIZE];
    text_to_utf8(text, utf8, (int32_t) sizeof(utf8));
    if (strlen(utf8) < DISCORD_MIN_FIELD_LENGTH) {
        return;
    }
    json_escape(utf8, escaped, sizeof(escaped));
    json_append(json, size, "\"%s\":\"%s\",", name, escaped);
}

static void activity_send(const discord_activity *activity)
{
    char json[DISCORD_PACKET_SIZE] = "";
    json_append(json, sizeof(json), "{\"cmd\":\"SET_ACTIVITY\",\"args\":{\"pid\":%lu,\"activity\":{", process_id());
    json_append_text(json, sizeof(json), "details", activity->details);
    json_append_text(json, sizeof(json), "state", activity->state);
    json_append(json, sizeof(json),
        "\"timestamps\":{\"start\":%lld},\"assets\":{\"large_image\":\"icon\",\"large_text\":\"Tutankhamun\"}}},\"nonce\":\"%u\"}",
        (long long) discord.start_time, ++discord.nonce);
    discord_log("SET_ACTIVITY %s", json);
    if (!packet_send(DISCORD_OPCODE_FRAME, json)) {
        connection_lost("SET_ACTIVITY was not written");
    }
}

static bool activity_take(discord_activity *activity, bool fresh_connection)
{
    lock_enter();
    bool wanted = discord.activity_pending || (fresh_connection && discord.activity_known);
    if (wanted) {
        *activity = discord.activity;
    }
    discord.activity_pending = false;
    lock_leave();
    return wanted;
}

static void activity_send_taken(bool fresh_connection)
{
    discord_activity activity;
    if (activity_take(&activity, fresh_connection)) {
        activity_send(&activity);
    }
}

static void worker_run(void)
{
    bool ever_connected = false;
    while (!stop_requested()) {
        if (!pipe_connected()) {
            if (!connect_to_discord()) {
                stop_wait(ever_connected ? DISCORD_RETRY_MS : DISCORD_FIRST_RETRY_MS);
                continue;
            }
            ever_connected = true;
            activity_send_taken(true);
        }
        if (wake_wait_stops(DISCORD_IDLE_WAIT_MS)) {
            break;
        }
        if (pipe_connected()) {
            activity_send_taken(false);
        }
        incoming_drain();
    }
    if (pipe_connected()) {
        packet_send(DISCORD_OPCODE_CLOSE, "{}");
        pipe_close();
    }
}

void discord_rpc_set_verbose(bool verbose)
{
    discord.verbose = verbose;
}

bool discord_rpc_start(const char *app_id)
{
    if (discord.running) {
        return true;
    }
    if (!app_id || !app_id[0]) {
        platform_log("discord: no application id, the presence is off");
        return false;
    }
    snprintf(discord.app_id, sizeof(discord.app_id), "%s", app_id);
    discord.start_time = (int64_t) time(NULL);
    discord.nonce = 0;
    discord.activity_pending = false;
    discord.activity_known = false;
    if (!worker_start()) {
        platform_log("discord: the worker thread did not start");
        return false;
    }
    discord.running = true;
    platform_log("discord: presence started for the application %s", discord.app_id);
    return true;
}

bool discord_rpc_running(void)
{
    return discord.running;
}

void discord_rpc_set_activity(const char *details, const char *state)
{
    if (!discord.running) {
        return;
    }
    lock_enter();
    snprintf(discord.activity.details, sizeof(discord.activity.details), "%s", details ? details : "");
    snprintf(discord.activity.state, sizeof(discord.activity.state), "%s", state ? state : "");
    discord.activity_pending = true;
    discord.activity_known = true;
    lock_leave();
    activity_signal();
    discord_log("activity queued: %s | %s", details ? details : "", state ? state : "");
}

void discord_rpc_stop(void)
{
    if (!discord.running) {
        return;
    }
    discord.running = false;
    stop_signal();
    if (!worker_join()) {
        platform_log("discord: the worker did not stop in time");
        return;
    }
    worker_release();
    platform_log("discord: presence stopped");
}
