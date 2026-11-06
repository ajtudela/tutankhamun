// Discord Rich Presence over the Discord client's local pipe (\\.\pipe\discord-ipc-0..9, an 8-byte header of opcode
// and length before each JSON payload). A worker thread does all the pipe I/O: the handshake with the application id,
// the retries while Discord is not running (15 s before the first connection, 5 s after), the latest activity as
// SET_ACTIVITY, the answers to pings and CLOSE when stopped. The game thread only stores the activity under a lock.
//
// A C port of Akhenaten's src/platform/discord_rpc.cpp (https://github.com/dalerank/Akhenaten, commit 8ec540693), for
// Windows only. Copyright (C) the Akhenaten authors. Licence: GNU Affero General Public License v3.0
// (LICENSES/AGPL-3.0.txt).

#include "platform/discord_rpc.h"

#include "platform/log.h"

#include <windows.h>
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
    DISCORD_PIPE_PATH_SIZE = 48,
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
    CRITICAL_SECTION lock;
    HANDLE stop_event;
    HANDLE activity_event;
    HANDLE thread;
    HANDLE pipe;
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

static void connection_lost(const char *reason)
{
    platform_log("discord: connection lost (%s)", reason);
    pipe_close();
}

static bool pipe_open(void)
{
    for (int32_t i = 0; i < DISCORD_PIPE_COUNT; i++) {
        wchar_t path[DISCORD_PIPE_PATH_SIZE];
        swprintf(path, DISCORD_PIPE_PATH_SIZE, L"\\\\.\\pipe\\discord-ipc-%d", i);
        HANDLE pipe = CreateFileW(path, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
        if (pipe != INVALID_HANDLE_VALUE) {
            discord_log("opened the pipe discord-ipc-%d", i);
            discord.pipe = pipe;
            return true;
        }
    }
    discord_log("no discord-ipc pipe (Discord is not running)");
    return false;
}

static bool pipe_available(DWORD *available)
{
    return PeekNamedPipe(discord.pipe, NULL, 0, NULL, available, NULL) != 0;
}

static bool pipe_read_exact(void *data, uint32_t length)
{
    uint8_t *at = data;
    DWORD started = GetTickCount();
    while (length > 0) {
        DWORD available = 0;
        if (!pipe_available(&available)) {
            return false;
        }
        if (available == 0) {
            if (GetTickCount() - started >= DISCORD_READ_TIMEOUT_MS) {
                return false;
            }
            Sleep(DISCORD_READ_POLL_MS);
            continue;
        }
        DWORD wanted = length < available ? length : available;
        DWORD read = 0;
        if (!ReadFile(discord.pipe, at, wanted, &read, NULL) || read == 0) {
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
    DWORD size = (DWORD) (DISCORD_HEADER_SIZE + length);
    DWORD written = 0;
    return WriteFile(discord.pipe, packet, size, &written, NULL) && written == size;
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
        DWORD available = 0;
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

static void text_to_utf8(const char *text, char *utf8, int32_t size)
{
    wchar_t wide[DISCORD_RPC_TEXT_SIZE];
    int32_t wide_length = MultiByteToWideChar(CP_ACP, 0, text, -1, wide, DISCORD_RPC_TEXT_SIZE);
    if (wide_length <= 0 || WideCharToMultiByte(CP_UTF8, 0, wide, -1, utf8, size, NULL, NULL) <= 0) {
        utf8[0] = 0;
    }
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
    json_append(json, sizeof(json), "{\"cmd\":\"SET_ACTIVITY\",\"args\":{\"pid\":%lu,\"activity\":{", GetCurrentProcessId());
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
    EnterCriticalSection(&discord.lock);
    bool wanted = discord.activity_pending || (fresh_connection && discord.activity_known);
    if (wanted) {
        *activity = discord.activity;
    }
    discord.activity_pending = false;
    LeaveCriticalSection(&discord.lock);
    return wanted;
}

static void activity_send_taken(bool fresh_connection)
{
    discord_activity activity;
    if (activity_take(&activity, fresh_connection)) {
        activity_send(&activity);
    }
}

static bool stop_requested(void)
{
    return WaitForSingleObject(discord.stop_event, 0) == WAIT_OBJECT_0;
}

static DWORD WINAPI worker_run(LPVOID unused)
{
    HANDLE wake[] = { discord.stop_event, discord.activity_event };
    bool ever_connected = false;
    while (!stop_requested()) {
        if (!pipe_connected()) {
            if (!connect_to_discord()) {
                WaitForSingleObject(discord.stop_event, ever_connected ? DISCORD_RETRY_MS : DISCORD_FIRST_RETRY_MS);
                continue;
            }
            ever_connected = true;
            activity_send_taken(true);
        }
        if (WaitForMultipleObjects(ARRAYSIZE(wake), wake, FALSE, DISCORD_IDLE_WAIT_MS) == WAIT_OBJECT_0) {
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
    return 0;
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
    discord.pipe = INVALID_HANDLE_VALUE;
    discord.nonce = 0;
    discord.activity_pending = false;
    discord.activity_known = false;
    InitializeCriticalSection(&discord.lock);
    discord.stop_event = CreateEventW(NULL, TRUE, FALSE, NULL);
    discord.activity_event = CreateEventW(NULL, FALSE, FALSE, NULL);
    if (discord.stop_event && discord.activity_event) {
        discord.thread = CreateThread(NULL, 0, worker_run, NULL, 0, NULL);
    }
    if (!discord.thread) {
        worker_release();
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
    EnterCriticalSection(&discord.lock);
    snprintf(discord.activity.details, sizeof(discord.activity.details), "%s", details ? details : "");
    snprintf(discord.activity.state, sizeof(discord.activity.state), "%s", state ? state : "");
    discord.activity_pending = true;
    discord.activity_known = true;
    LeaveCriticalSection(&discord.lock);
    SetEvent(discord.activity_event);
    discord_log("activity queued: %s | %s", details ? details : "", state ? state : "");
}

void discord_rpc_stop(void)
{
    if (!discord.running) {
        return;
    }
    discord.running = false;
    SetEvent(discord.stop_event);
    if (WaitForSingleObject(discord.thread, DISCORD_STOP_WAIT_MS) != WAIT_OBJECT_0) {
        platform_log("discord: the worker did not stop in time");
        return;
    }
    worker_release();
    platform_log("discord: presence stopped");
}
