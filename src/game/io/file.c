// File helpers: extensions, existence and size checks, reading a whole file or a part of it, with the fallback to
// the 555 folder. A file missing from the game directory fails at once.
//
// Callers live with these:
// - io_read_file_part's name buffer is 65 bytes, and longer names overflow it;
// - io_read_file_into_buffer returns 1 without checking the read.

#include "io/file.h"

#include "core/text.h"
#include "io/io_data.h"
#include "platform/platform_file.h"
#include "platform/status_log.h"
#include "window/window_data.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

enum file_limit {
    FILE_EXTENSION_LENGTH = 3,
    DIR_555_LENGTH = 4,
    HIGHSCORE_FILE_SIZE = 7600,
};

static const char *find_extension(const char *path)
{
    char c;
    do {
        c = *path++;
        if (c == '.') {
            return path;
        }
    } while (c != 0);
    return NULL;
}

void file_store_extension(const char *path)
{
    const char *extension = find_extension(path);
    if (extension) {
        memcpy(file_extension, extension, FILE_EXTENSION_LENGTH);
        file_extension[FILE_EXTENSION_LENGTH] = 0;
    }
}

void file_change_extension(char *path)
{
    char *extension = (char *) find_extension(path);
    if (extension) {
        memcpy(extension, file_extension, FILE_EXTENSION_LENGTH);
        extension[FILE_EXTENSION_LENGTH] = 0;
    }
}

void file_set_extension(char *path, const char *extension)
{
    memcpy(file_extension, extension, FILE_EXTENSION_LENGTH);
    file_change_extension(path);
}

static int32_t open_for_reading(const char *path)
{
    return platform_file_open(path, PLATFORM_OPEN_READ);
}

static int32_t is_readable(const char *path)
{
    int32_t fd = open_for_reading(path);
    if (fd < 0) {
        return 0;
    }
    platform_file_close(fd);
    return 1;
}

int32_t file_exists(const char *path)
{
    return is_readable(path);
}

static int32_t size_and_close(int32_t fd)
{
    int32_t size = platform_file_length(fd);
    platform_file_close(fd);
    return size;
}

int32_t file_get_size(const char *path)
{
    int32_t fd = open_for_reading(path);
    if (fd == -1) {
        return 0;
    }
    return size_and_close(fd);
}

int32_t file_get_size_from_offset(const char *path, int32_t offset)
{
    int32_t fd = open_for_reading(path);
    if (fd == -1) {
        return 0;
    }
    return size_and_close(fd) - offset;
}

static int32_t read_from_descriptor(int32_t fd, void *buffer, int32_t size, int32_t offset, int32_t result)
{
    if (platform_file_seek(fd, offset, SEEK_SET) != -1) {
        result = platform_file_read(fd, buffer, (uint32_t) size);
    }
    platform_file_close(fd);
    return result;
}

static int32_t read_part(const char *path, void *buffer, int32_t size, int32_t offset, int32_t result)
{
    int32_t fd = open_for_reading(path);
    if (fd == -1) {
        return result;
    }
    return read_from_descriptor(fd, buffer, size, offset, result);
}

static int32_t read_part_from_fallbacks(const char *path, void *buffer, int32_t size, int32_t offset, int32_t result)
{
    char name[65];
    memset(name, 0, sizeof(name));
    file_store_extension(path);
    string_to_upper(file_extension);
    char *name_end = name;
    if (strcmp(file_extension_555, file_extension) == 0) {
        strcpy(name, image_555_directory);
        name_end = name + DIR_555_LENGTH;
    }
    strcpy(name_end, path);
    return read_part(name, buffer, size, offset, result);
}

int32_t io_read_file_part(const char *path, void *buffer, int32_t size, int32_t offset)
{
    int32_t result = 0;
    int32_t fd = open_for_reading(path);
    if (fd != -1) {
        result = read_from_descriptor(fd, buffer, size, offset, result);
        if (result >= 1) {
            return result;
        }
    }
    return read_part_from_fallbacks(path, buffer, size, offset, result);
}

int32_t io_read_file_into_buffer(const char *path, void *buffer, int32_t offset)
{
    FILE *file = platform_fopen(path, file_mode_read_binary);
    if (!file) {
        return 0;
    }
    int32_t length = platform_stream_length(file);
    fseek(file, offset, SEEK_SET);
    fread(buffer, 1, (uint32_t) (length - offset), file);
    fclose(file);
    return 1;
}

bool highscore_read_file(void *table, const char *path)
{
    FILE *file = platform_fopen(path, file_mode_read_binary);
    if (!file) {
        return false;
    }
    size_t count = fread(table, HIGHSCORE_FILE_SIZE, 1, file);
    fclose(file);
    return count == 1;
}
