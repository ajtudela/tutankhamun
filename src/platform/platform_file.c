// The platform's files and directories. On Windows the game's paths go to the C runtime and the Windows directory
// functions as they are. Elsewhere a path's backslashes become slashes and every part of it that is not found as
// written is looked up in its directory whatever its case (the game's data files are named in mixed case and the game
// asks for them in its own); a pattern of the file dialogs matches whatever the case, as on Windows.

#if !defined(_WIN32) && !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif

#include "platform/platform_file.h"

#include <string.h>

#if defined(_WIN32)

#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <sys/stat.h>
#include <assert.h>

const char *platform_path_native(const char *path, char *native, uint32_t size)
{
    strncpy(native, path, size - 1);
    native[size - 1] = 0;
    return native;
}

bool platform_path_absolute(const char *path, char *absolute, uint32_t size)
{
    DWORD length = GetFullPathNameA(path, size, absolute, 0);
    if (length == 0 || length >= size) {
        strncpy(absolute, path, size - 1);
        absolute[size - 1] = 0;
        return false;
    }
    return true;
}

bool platform_dir_enter(const char *path)
{
    return SetCurrentDirectoryA(path) != 0;
}

static int32_t open_flags(platform_open_mode mode)
{
    switch (mode) {
        case PLATFORM_OPEN_WRITE:
            return _O_WRONLY | _O_CREAT | _O_BINARY;
        case PLATFORM_OPEN_REPLACE:
            return _O_WRONLY | _O_CREAT | _O_TRUNC | _O_BINARY;
        default:
            return _O_RDONLY | _O_BINARY;
    }
}

int32_t platform_file_open(const char *path, platform_open_mode mode)
{
    return _open(path, open_flags(mode), _S_IREAD | _S_IWRITE);
}

int32_t platform_file_read(int32_t fd, void *buffer, uint32_t size)
{
    return _read(fd, buffer, size);
}

int32_t platform_file_write(int32_t fd, const void *buffer, uint32_t size)
{
    return _write(fd, buffer, size);
}

int32_t platform_file_seek(int32_t fd, int32_t offset, int32_t origin)
{
    return _lseek(fd, offset, origin);
}

int32_t platform_file_length(int32_t fd)
{
    return _filelength(fd);
}

void platform_file_close(int32_t fd)
{
    _close(fd);
}

FILE *platform_fopen(const char *path, const char *mode)
{
    return fopen(path, mode);
}

int32_t platform_stream_length(FILE *file)
{
    return _filelength(_fileno(file));
}

bool platform_file_remove(const char *path)
{
    return remove(path) == 0;
}

bool platform_file_exists(const char *path)
{
    return GetFileAttributesA(path) != INVALID_FILE_ATTRIBUTES;
}

bool platform_dir_exists(const char *path)
{
    DWORD attributes = GetFileAttributesA(path);
    return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

int32_t platform_text_compare_nocase(const char *a, const char *b)
{
    return _stricmp(a, b);
}

int32_t platform_text_compare_nocase_n(const char *a, const char *b, uint32_t count)
{
    return _strnicmp(a, b, count);
}

bool platform_dir_create(const char *path)
{
    return CreateDirectoryA(path, 0) != 0;
}

bool platform_dir_remove(const char *path)
{
    return RemoveDirectoryA(path) != 0;
}

static_assert(sizeof(WIN32_FIND_DATAA) <= sizeof(((platform_dir_listing *) 0)->native), "platform_dir_listing has room for a WIN32_FIND_DATAA");

static void listing_take_name(platform_dir_listing *listing)
{
    listing->name = ((const WIN32_FIND_DATAA *) listing->native)->cFileName;
}

bool platform_dir_first(platform_dir_listing *listing, const char *pattern)
{
    listing->handle = FindFirstFileA(pattern, (WIN32_FIND_DATAA *) listing->native);
    if (listing->handle == INVALID_HANDLE_VALUE) {
        return false;
    }
    listing_take_name(listing);
    return true;
}

bool platform_dir_next(platform_dir_listing *listing)
{
    if (!FindNextFileA(listing->handle, (WIN32_FIND_DATAA *) listing->native)) {
        return false;
    }
    listing_take_name(listing);
    return true;
}

void platform_dir_close(platform_dir_listing *listing)
{
    FindClose(listing->handle);
}

#else

#include <dirent.h>
#include <fcntl.h>
#include <fnmatch.h>
#include <strings.h>
#include <sys/stat.h>
#include <unistd.h>

static bool path_exists(const char *path)
{
    struct stat info;
    return stat(path, &info) == 0;
}

static bool find_name_nocase(const char *dir, char *name)
{
    DIR *listing = opendir(dir[0] ? dir : ".");
    if (!listing) {
        return false;
    }
    bool found = false;
    struct dirent *entry;
    while (!found && (entry = readdir(listing)) != NULL) {
        if (strcasecmp(entry->d_name, name) == 0 && strlen(entry->d_name) == strlen(name)) {
            memcpy(name, entry->d_name, strlen(name));
            found = true;
        }
    }
    closedir(listing);
    return found;
}

static void correct_case(char *path)
{
    char *part = path[0] == '/' ? path + 1 : path;
    while (*part) {
        char *end = strchr(part, '/');
        char kept = end ? *end : 0;
        if (end) {
            *end = 0;
        }
        bool exists = path_exists(path);
        if (!exists) {
            char parent[PLATFORM_PATH_MAX];
            size_t parent_length = (size_t) (part - path);
            memcpy(parent, path, parent_length);
            parent[parent_length] = 0;
            exists = find_name_nocase(parent, part);
        }
        if (end) {
            *end = kept;
        }
        if (!exists || !end) {
            return;
        }
        part = end + 1;
    }
}

const char *platform_path_native(const char *path, char *native, uint32_t size)
{
    uint32_t i = 0;
    for (; path[i] && i + 1 < size; i++) {
        native[i] = path[i] == '\\' ? '/' : path[i];
    }
    native[i] = 0;
    if (native[0] && !path_exists(native)) {
        correct_case(native);
    }
    return native;
}

bool platform_path_absolute(const char *path, char *absolute, uint32_t size)
{
    char native[PLATFORM_PATH_MAX];
    platform_path_native(path, native, sizeof(native));
    if (native[0] == '/') {
        snprintf(absolute, size, "%s", native);
        return true;
    }
    char cwd[PLATFORM_PATH_MAX];
    if (!getcwd(cwd, sizeof(cwd))) {
        snprintf(absolute, size, "%s", native);
        return false;
    }
    int written = snprintf(absolute, size, "%s/%s", cwd, native);
    return written > 0 && (uint32_t) written < size;
}

bool platform_dir_enter(const char *path)
{
    char native[PLATFORM_PATH_MAX];
    return chdir(platform_path_native(path, native, sizeof(native))) == 0;
}

static int open_flags(platform_open_mode mode)
{
    switch (mode) {
        case PLATFORM_OPEN_WRITE:
            return O_WRONLY | O_CREAT;
        case PLATFORM_OPEN_REPLACE:
            return O_WRONLY | O_CREAT | O_TRUNC;
        default:
            return O_RDONLY;
    }
}

int32_t platform_file_open(const char *path, platform_open_mode mode)
{
    char native[PLATFORM_PATH_MAX];
    return open(platform_path_native(path, native, sizeof(native)), open_flags(mode), 0644);
}

int32_t platform_file_read(int32_t fd, void *buffer, uint32_t size)
{
    return (int32_t) read(fd, buffer, size);
}

int32_t platform_file_write(int32_t fd, const void *buffer, uint32_t size)
{
    return (int32_t) write(fd, buffer, size);
}

int32_t platform_file_seek(int32_t fd, int32_t offset, int32_t origin)
{
    return (int32_t) lseek(fd, offset, origin);
}

int32_t platform_file_length(int32_t fd)
{
    struct stat info;
    return fstat(fd, &info) == 0 ? (int32_t) info.st_size : -1;
}

void platform_file_close(int32_t fd)
{
    close(fd);
}

FILE *platform_fopen(const char *path, const char *mode)
{
    char native[PLATFORM_PATH_MAX];
    return fopen(platform_path_native(path, native, sizeof(native)), mode);
}

int32_t platform_stream_length(FILE *file)
{
    fflush(file);
    return platform_file_length(fileno(file));
}

bool platform_file_remove(const char *path)
{
    char native[PLATFORM_PATH_MAX];
    return remove(platform_path_native(path, native, sizeof(native))) == 0;
}

bool platform_file_exists(const char *path)
{
    char native[PLATFORM_PATH_MAX];
    return path_exists(platform_path_native(path, native, sizeof(native)));
}

bool platform_dir_exists(const char *path)
{
    char native[PLATFORM_PATH_MAX];
    struct stat info;
    return stat(platform_path_native(path, native, sizeof(native)), &info) == 0 && S_ISDIR(info.st_mode);
}

int32_t platform_text_compare_nocase(const char *a, const char *b)
{
    return strcasecmp(a, b);
}

int32_t platform_text_compare_nocase_n(const char *a, const char *b, uint32_t count)
{
    return strncasecmp(a, b, count);
}

bool platform_dir_create(const char *path)
{
    char native[PLATFORM_PATH_MAX];
    return mkdir(platform_path_native(path, native, sizeof(native)), 0755) == 0;
}

bool platform_dir_remove(const char *path)
{
    char native[PLATFORM_PATH_MAX];
    return rmdir(platform_path_native(path, native, sizeof(native))) == 0;
}

typedef struct {
    char pattern[256];
} dir_listing_state;

_Static_assert(sizeof(dir_listing_state) <= sizeof(((platform_dir_listing *) 0)->native), "platform_dir_listing has room for its pattern");

bool platform_dir_next(platform_dir_listing *listing)
{
    const dir_listing_state *state = (const dir_listing_state *) listing->native;
    struct dirent *entry;
    while ((entry = readdir((DIR *) listing->handle)) != NULL) {
        if (fnmatch(state->pattern, entry->d_name, FNM_CASEFOLD) == 0) {
            listing->name = entry->d_name;
            return true;
        }
    }
    return false;
}

bool platform_dir_first(platform_dir_listing *listing, const char *pattern)
{
    char native[PLATFORM_PATH_MAX];
    platform_path_native(pattern, native, sizeof(native));
    char *slash = strrchr(native, '/');
    const char *name = slash ? slash + 1 : native;
    dir_listing_state *state = (dir_listing_state *) listing->native;
    strncpy(state->pattern, name, sizeof(state->pattern) - 1);
    state->pattern[sizeof(state->pattern) - 1] = 0;
    if (slash) {
        *slash = 0;
    }
    char directory[PLATFORM_PATH_MAX];
    strcpy(directory, slash ? (native[0] ? native : "/") : ".");
    correct_case(directory);
    listing->handle = opendir(directory);
    if (!listing->handle) {
        return false;
    }
    if (!platform_dir_next(listing)) {
        closedir((DIR *) listing->handle);
        listing->handle = NULL;
        return false;
    }
    return true;
}

void platform_dir_close(platform_dir_listing *listing)
{
    if (listing->handle) {
        closedir((DIR *) listing->handle);
        listing->handle = NULL;
    }
}

#endif
