// The program's own update (updater.h). The release "nightly" holds, per build, <asset>.tar (the game folder:
// tutankhamun.exe and config\) and <asset>.version, two lines written by the CI:
//     <build> <commit>
//     <size of the tar> <its sha256>
// The check reads the .version; a larger build than ours is downloaded into update\download.tar, its size and sha256
// compared, and unpacked into update\staging. When the game ends the staged program replaces ours (renamed to
// tutankhamun.exe.old first), the other staged files are moved over theirs and update\ is removed.

#include "platform/updater.h"
#include "platform/http.h"
#include "platform/log.h"
#include "platform/version.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32) && defined(TUTANKHAMUN_UPDATER)
#define UPDATER_SUPPORTED 1
#include <windows.h>
#include "app/app_files.h"
#endif

static volatile int32_t state = UPDATER_OFF;
static volatile int32_t progress;
static volatile int32_t new_build;
static int32_t disabled;
static int32_t restart_requested;

void updater_disable(void)
{
    disabled = 1;
}

updater_state updater_current_state(void)
{
    return (updater_state) state;
}

int32_t updater_progress_percent(void)
{
    return progress;
}

int32_t updater_new_build(void)
{
    return new_build;
}

void updater_restart_request(void)
{
    restart_requested = 1;
}

#if !defined(UPDATER_SUPPORTED)

void updater_start(void)
{
}

void updater_finish(void)
{
}

#else

#if defined(_WIN64)
#define UPDATER_ASSET "windows_build_x64"
#else
#define UPDATER_ASSET "windows_build"
#endif

enum updater_limits {
    UPDATER_PATH = 1024,
    UPDATER_URL = 1024,
    UPDATER_VERSION_TEXT = 512,
    UPDATER_TAR_BLOCK = 512,
    UPDATER_COPY_CHUNK = 65536,
    UPDATER_MAX_SIZE = 256 * 1024 * 1024,
};

static const char release_url[] = "https://github.com/dalerank/tutankhamun/releases/download/nightly/";
static const wchar_t program_name[] = L"tutankhamun.exe";

static char base_url[UPDATER_URL];
static wchar_t program_path[UPDATER_PATH];
static wchar_t program_dir[UPDATER_PATH];
static wchar_t update_dir[UPDATER_PATH];
static wchar_t staging_dir[UPDATER_PATH];
static wchar_t start_dir[UPDATER_PATH];

static void set_state(updater_state value)
{
    InterlockedExchange((volatile LONG *) &state, (LONG) value);
}

// ---- SHA-256 (FIPS 180-4) ----

typedef struct sha256 {
    uint32_t h[8];
    uint64_t length;
    uint8_t block[64];
    uint32_t used;
} sha256;

static const uint32_t sha256_k[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5, 0xd807aa98,
    0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786,
    0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da, 0x983e5152, 0xa831c66d, 0xb00327c8,
    0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819,
    0xd6990624, 0xf40e3585, 0x106aa070, 0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a,
    0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7,
    0xc67178f2,
};

static uint32_t rotate_right(uint32_t value, int32_t bits)
{
    return (value >> bits) | (value << (32 - bits));
}

static void sha256_block(sha256 *s, const uint8_t *block)
{
    uint32_t w[64];
    for (int32_t i = 0; i < 16; i++) {
        w[i] = ((uint32_t) block[i * 4] << 24) | ((uint32_t) block[i * 4 + 1] << 16) | ((uint32_t) block[i * 4 + 2] << 8)
            | block[i * 4 + 3];
    }
    for (int32_t i = 16; i < 64; i++) {
        uint32_t s0 = rotate_right(w[i - 15], 7) ^ rotate_right(w[i - 15], 18) ^ (w[i - 15] >> 3);
        uint32_t s1 = rotate_right(w[i - 2], 17) ^ rotate_right(w[i - 2], 19) ^ (w[i - 2] >> 10);
        w[i] = w[i - 16] + s0 + w[i - 7] + s1;
    }
    uint32_t a = s->h[0], b = s->h[1], c = s->h[2], d = s->h[3], e = s->h[4], f = s->h[5], g = s->h[6], h = s->h[7];
    for (int32_t i = 0; i < 64; i++) {
        uint32_t t1 = h + (rotate_right(e, 6) ^ rotate_right(e, 11) ^ rotate_right(e, 25)) + ((e & f) ^ (~e & g)) + sha256_k[i] + w[i];
        uint32_t t2 = (rotate_right(a, 2) ^ rotate_right(a, 13) ^ rotate_right(a, 22)) + ((a & b) ^ (a & c) ^ (b & c));
        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }
    s->h[0] += a;
    s->h[1] += b;
    s->h[2] += c;
    s->h[3] += d;
    s->h[4] += e;
    s->h[5] += f;
    s->h[6] += g;
    s->h[7] += h;
}

static void sha256_init(sha256 *s)
{
    static const uint32_t initial[8] = { 0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19 };
    memcpy(s->h, initial, sizeof(initial));
    s->length = 0;
    s->used = 0;
}

static void sha256_add(sha256 *s, const uint8_t *data, size_t size)
{
    s->length += size;
    while (size > 0) {
        size_t take = 64 - s->used;
        if (take > size) {
            take = size;
        }
        memcpy(s->block + s->used, data, take);
        s->used += (uint32_t) take;
        data += take;
        size -= take;
        if (s->used == 64) {
            sha256_block(s, s->block);
            s->used = 0;
        }
    }
}

static void sha256_hex(sha256 *s, char *hex)
{
    uint64_t bits = s->length * 8;
    uint8_t pad = 0x80;
    sha256_add(s, &pad, 1);
    pad = 0;
    while (s->used != 56) {
        sha256_add(s, &pad, 1);
    }
    uint8_t length[8];
    for (int32_t i = 0; i < 8; i++) {
        length[i] = (uint8_t) (bits >> (56 - i * 8));
    }
    sha256_add(s, length, 8);
    for (int32_t i = 0; i < 8; i++) {
        snprintf(hex + i * 8, 9, "%08x", s->h[i]);
    }
}

typedef struct text_sink {
    char text[UPDATER_VERSION_TEXT];
    size_t used;
} text_sink;

static int32_t sink_text(void *context, const uint8_t *data, uint32_t size)
{
    text_sink *sink = (text_sink *) context;
    if (sink->used + size >= sizeof(sink->text)) {
        return 0;
    }
    memcpy(sink->text + sink->used, data, size);
    sink->used += size;
    sink->text[sink->used] = 0;
    return 1;
}

typedef struct file_sink {
    FILE *file;
    sha256 hash;
    uint64_t written;
    uint64_t expected;
} file_sink;

static int32_t sink_file(void *context, const uint8_t *data, uint32_t size)
{
    file_sink *sink = (file_sink *) context;
    if (sink->written + size > sink->expected || fwrite(data, 1, size, sink->file) != size) {
        return 0;
    }
    sha256_add(&sink->hash, data, size);
    sink->written += size;
    InterlockedExchange((volatile LONG *) &progress, (LONG) (sink->written * 100 / sink->expected));
    return 1;
}

// ---- files ----

static void join(wchar_t *out, const wchar_t *dir, const wchar_t *name)
{
    _snwprintf(out, UPDATER_PATH, L"%ls\\%ls", dir, name);
    out[UPDATER_PATH - 1] = 0;
}

static void make_parent_dirs(const wchar_t *path, size_t root_length)
{
    wchar_t dir[UPDATER_PATH];
    wcsncpy(dir, path, UPDATER_PATH - 1);
    dir[UPDATER_PATH - 1] = 0;
    for (size_t i = root_length + 1; dir[i]; i++) {
        if (dir[i] == L'\\') {
            dir[i] = 0;
            CreateDirectoryW(dir, 0);
            dir[i] = L'\\';
        }
    }
}

static void remove_tree(const wchar_t *dir)
{
    wchar_t pattern[UPDATER_PATH], path[UPDATER_PATH];
    WIN32_FIND_DATAW found;
    join(pattern, dir, L"*");
    HANDLE find = FindFirstFileW(pattern, &found);
    if (find != INVALID_HANDLE_VALUE) {
        do {
            if (wcscmp(found.cFileName, L".") == 0 || wcscmp(found.cFileName, L"..") == 0) {
                continue;
            }
            join(path, dir, found.cFileName);
            if (found.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                remove_tree(path);
            } else {
                SetFileAttributesW(path, FILE_ATTRIBUTE_NORMAL);
                DeleteFileW(path);
            }
        } while (FindNextFileW(find, &found));
        FindClose(find);
    }
    RemoveDirectoryW(dir);
}

// moves every file under from to the same place under to; 0 when one could not be moved
static int32_t move_tree(const wchar_t *from, const wchar_t *to)
{
    wchar_t pattern[UPDATER_PATH], source[UPDATER_PATH], target[UPDATER_PATH];
    WIN32_FIND_DATAW found;
    int32_t ok = 1;
    CreateDirectoryW(to, 0);
    join(pattern, from, L"*");
    HANDLE find = FindFirstFileW(pattern, &found);
    if (find == INVALID_HANDLE_VALUE) {
        return 1;
    }
    do {
        if (wcscmp(found.cFileName, L".") == 0 || wcscmp(found.cFileName, L"..") == 0) {
            continue;
        }
        join(source, from, found.cFileName);
        join(target, to, found.cFileName);
        if (found.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            ok &= move_tree(source, target);
        } else if (!MoveFileExW(source, target, MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED)) {
            platform_log("updater: cannot move %ls to %ls (error %lu)", source, target, (unsigned long) GetLastError());
            ok = 0;
        }
    } while (FindNextFileW(find, &found));
    FindClose(find);
    return ok;
}

static int32_t file_exists(const wchar_t *path)
{
    DWORD attributes = GetFileAttributesW(path);
    return attributes != INVALID_FILE_ATTRIBUTES && !(attributes & FILE_ATTRIBUTE_DIRECTORY);
}

// ---- the tar of the CI (ustar: "tar --format=ustar -cf <asset>.tar -C <folder> .") ----

static uint64_t tar_octal(const char *field, size_t size)
{
    uint64_t value = 0;
    for (size_t i = 0; i < size && field[i] >= '0' && field[i] <= '7'; i++) {
        value = value * 8 + (uint64_t) (field[i] - '0');
    }
    return value;
}

static int32_t tar_name(const uint8_t *header, char *name, size_t size)
{
    char full[256];
    const char *entry = (const char *) header;
    if (memcmp(header + 257, "ustar", 5) == 0 && header[345]) {
        snprintf(full, sizeof(full), "%.155s/%.100s", (const char *) header + 345, entry);
    } else {
        snprintf(full, sizeof(full), "%.100s", entry);
    }
    const char *start = full;
    while (start[0] == '.' && start[1] == '/') {
        start += 2;
    }
    if (strstr(start, "..") || strchr(start, ':') || start[0] == '/' || start[0] == '\\') {
        return -1;
    }
    snprintf(name, size, "%s", start);
    size_t length = strlen(name);
    while (length > 0 && name[length - 1] == '/') {
        name[--length] = 0;
    }
    for (char *c = name; *c; c++) {
        if (*c == '/') {
            *c = '\\';
        }
    }
    return length > 0 && strcmp(name, ".") != 0;
}

static int32_t untar(const wchar_t *tar, const wchar_t *into)
{
    FILE *file = _wfopen(tar, L"rb");
    if (!file) {
        return 0;
    }
    static uint8_t chunk[UPDATER_COPY_CHUNK];
    uint8_t header[UPDATER_TAR_BLOCK];
    int32_t ok = 1, files = 0;
    size_t root_length = wcslen(into);
    CreateDirectoryW(into, 0);
    while (ok && fread(header, 1, sizeof(header), file) == sizeof(header)) {
        int32_t empty = 1;
        for (int32_t i = 0; i < UPDATER_TAR_BLOCK && empty; i++) {
            empty = header[i] == 0;
        }
        if (empty) {
            break;
        }
        char name[256];
        wchar_t wide[UPDATER_PATH / 2], path[UPDATER_PATH];
        uint64_t size = tar_octal((const char *) header + 124, 12);
        uint64_t padded = (size + UPDATER_TAR_BLOCK - 1) / UPDATER_TAR_BLOCK * UPDATER_TAR_BLOCK;
        char type = (char) header[156];
        int32_t named = tar_name(header, name, sizeof(name));
        if (named < 0) {
            platform_log("updater: the archive names a path outside its folder");
            ok = 0;
            break;
        }
        if (named && MultiByteToWideChar(CP_UTF8, 0, name, -1, wide, UPDATER_PATH / 2) == 0) {
            ok = 0;
            break;
        }
        if (named) {
            join(path, into, wide);
        }
        if (named && type == '5') {
            make_parent_dirs(path, root_length);
            CreateDirectoryW(path, 0);
        }
        if (named && (type == '0' || type == 0)) {
            make_parent_dirs(path, root_length);
            FILE *out = _wfopen(path, L"wb");
            uint64_t left = size;
            ok = out != 0;
            while (ok && left > 0) {
                size_t take = left > sizeof(chunk) ? sizeof(chunk) : (size_t) left;
                ok = fread(chunk, 1, take, file) == take && fwrite(chunk, 1, take, out) == take;
                left -= take;
            }
            if (out) {
                fclose(out);
            }
            if (ok && padded > size) {
                ok = fseek(file, (long) (padded - size), SEEK_CUR) == 0;
            }
            files++;
        } else if (ok && padded > 0) {
            ok = fseek(file, (long) padded, SEEK_CUR) == 0;
        }
    }
    fclose(file);
    return ok && files > 0;
}

// ---- the check and the download (the update thread) ----

static int32_t read_remote_version(int32_t *build, uint64_t *size, char *hash)
{
    char url[UPDATER_URL];
    text_sink sink;
    memset(&sink, 0, sizeof(sink));
    snprintf(url, sizeof(url), "%s%s.version", base_url, UPDATER_ASSET);
    if (!http_get(url, sink_text, &sink)) {
        return 0;
    }
    char commit[64];
    unsigned long long tar_size = 0;
    if (sscanf(sink.text, "%d %63s %llu %64s", build, commit, &tar_size, hash) != 4 || strlen(hash) != 64 || tar_size == 0
        || tar_size > UPDATER_MAX_SIZE) {
        platform_log("updater: the version file is not understood");
        return 0;
    }
    *size = tar_size;
    platform_log("updater: the newest build is %d (%s), this is %d (%s)", *build, commit, version_build(), version_commit());
    return 1;
}

static int32_t download(uint64_t size, const char *hash)
{
    char url[UPDATER_URL];
    wchar_t tar[UPDATER_PATH], staged[UPDATER_PATH];
    snprintf(url, sizeof(url), "%s%s.tar", base_url, UPDATER_ASSET);
    remove_tree(update_dir);
    CreateDirectoryW(update_dir, 0);
    join(tar, update_dir, L"download.tar");
    file_sink sink;
    memset(&sink, 0, sizeof(sink));
    sink.expected = size;
    sha256_init(&sink.hash);
    sink.file = _wfopen(tar, L"wb");
    if (!sink.file) {
        platform_log("updater: cannot write %ls", tar);
        return 0;
    }
    int32_t got = http_get(url, sink_file, &sink);
    fclose(sink.file);
    char digest[65];
    sha256_hex(&sink.hash, digest);
    if (!got || sink.written != size || strcmp(digest, hash) != 0) {
        platform_log("updater: the download is broken (%llu of %llu bytes, sha256 %s)", (unsigned long long) sink.written,
            (unsigned long long) size, digest);
        return 0;
    }
    if (!untar(tar, staging_dir)) {
        platform_log("updater: cannot unpack the download");
        return 0;
    }
    DeleteFileW(tar);
    join(staged, staging_dir, program_name);
    if (!file_exists(staged)) {
        platform_log("updater: the download has no %ls", program_name);
        return 0;
    }
    return 1;
}

static DWORD WINAPI update_thread(LPVOID unused)
{
    int32_t build = 0;
    uint64_t size = 0;
    char hash[65];
    if (!read_remote_version(&build, &size, hash)) {
        set_state(UPDATER_NO_ANSWER);
        return 0;
    }
    if (build <= version_build()) {
        set_state(UPDATER_UP_TO_DATE);
        return 0;
    }
    InterlockedExchange((volatile LONG *) &new_build, (LONG) build);
    set_state(UPDATER_DOWNLOADING);
    if (!download(size, hash)) {
        remove_tree(update_dir);
        set_state(UPDATER_FAILED);
        return 0;
    }
    platform_log("updater: build %d is ready", build);
    set_state(UPDATER_READY);
    return 0;
}

static int32_t check_wanted(void)
{
    char ini[UPDATER_PATH];
    if (disabled) {
        return 0;
    }
    return !app_ini_path(ini, sizeof(ini)) || app_ini_read_int(ini, "update", "check", 1) != 0;
}

void updater_start(void)
{
    DWORD length = GetModuleFileNameW(0, program_path, UPDATER_PATH);
    if (length == 0 || length >= UPDATER_PATH) {
        return;
    }
    wcscpy(program_dir, program_path);
    wchar_t *slash = wcsrchr(program_dir, L'\\');
    if (!slash) {
        return;
    }
    *slash = 0;
    join(update_dir, program_dir, L"update");
    join(staging_dir, update_dir, L"staging");
    GetCurrentDirectoryW(UPDATER_PATH, start_dir);

    // the program that the last update replaced
    wchar_t old[UPDATER_PATH];
    _snwprintf(old, UPDATER_PATH, L"%ls.old", program_path);
    if (file_exists(old)) {
        DeleteFileW(old);
    }
    if (!check_wanted()) {
        platform_log("updater: the check is off");
        return;
    }
    const char *test_url = getenv("TUTANKHAMUN_UPDATE_URL");
    snprintf(base_url, sizeof(base_url), "%s", test_url && test_url[0] ? test_url : release_url);
    set_state(UPDATER_CHECKING);
    HANDLE thread = CreateThread(0, 0, update_thread, 0, 0, 0);
    if (!thread) {
        set_state(UPDATER_OFF);
        return;
    }
    CloseHandle(thread);
}

static int32_t install(void)
{
    wchar_t old[UPDATER_PATH], staged[UPDATER_PATH];
    _snwprintf(old, UPDATER_PATH, L"%ls.old", program_path);
    join(staged, staging_dir, program_name);
    DeleteFileW(old);
    if (!MoveFileExW(program_path, old, MOVEFILE_REPLACE_EXISTING)) {
        platform_log("updater: cannot rename the program (error %lu)", (unsigned long) GetLastError());
        return 0;
    }
    if (!MoveFileExW(staged, program_path, MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED)) {
        platform_log("updater: cannot put the new program in place (error %lu)", (unsigned long) GetLastError());
        MoveFileExW(old, program_path, MOVEFILE_REPLACE_EXISTING);
        return 0;
    }
    int32_t moved = move_tree(staging_dir, program_dir);
    remove_tree(update_dir);
    platform_log("updater: build %d installed%s", new_build, moved ? "" : ", some files are old");
    return 1;
}

static void start_new_program(void)
{
    // the same command line, from the folder the program was started in
    wchar_t *command = _wcsdup(GetCommandLineW());
    STARTUPINFOW startup;
    PROCESS_INFORMATION process;
    memset(&startup, 0, sizeof(startup));
    startup.cb = sizeof(startup);
    if (command && CreateProcessW(program_path, command, 0, 0, FALSE, 0, 0, start_dir[0] ? start_dir : 0, &startup, &process)) {
        CloseHandle(process.hThread);
        CloseHandle(process.hProcess);
    } else {
        platform_log("updater: cannot start the new program (error %lu)", (unsigned long) GetLastError());
    }
    free(command);
}

void updater_finish(void)
{
    if (state != UPDATER_READY) {
        return;
    }
    if (install() && restart_requested) {
        start_new_program();
    }
}

#endif
