// The platform's files and directories: opening, reading and writing the game's files by the paths the game makes
// (Windows paths relative to the game directory: on other systems the backslashes become slashes and each part of the
// path is found whatever its case), the player and saved game directories, and the files that match a pattern
// (the file dialogs).

#ifndef PLATFORM_FILE_H
#define PLATFORM_FILE_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

enum platform_path_size {
    PLATFORM_PATH_MAX = 1024,
};

typedef enum {
    PLATFORM_OPEN_READ = 0,
    PLATFORM_OPEN_WRITE = 1,
    PLATFORM_OPEN_REPLACE = 2,
} platform_open_mode;

typedef struct {
    void *handle;
    const char *name;
    uint64_t native[42];
} platform_dir_listing;

const char *platform_path_native(const char *path, char *native, uint32_t size);
bool platform_path_absolute(const char *path, char *absolute, uint32_t size);
bool platform_dir_enter(const char *path);

int32_t platform_file_open(const char *path, platform_open_mode mode);
int32_t platform_file_read(int32_t fd, void *buffer, uint32_t size);
int32_t platform_file_write(int32_t fd, const void *buffer, uint32_t size);
int32_t platform_file_seek(int32_t fd, int32_t offset, int32_t origin);
int32_t platform_file_length(int32_t fd);
void platform_file_close(int32_t fd);

FILE *platform_fopen(const char *path, const char *mode);
int32_t platform_stream_length(FILE *file);
bool platform_file_remove(const char *path);
bool platform_file_exists(const char *path);
bool platform_dir_exists(const char *path);

int32_t platform_text_compare_nocase(const char *a, const char *b);
int32_t platform_text_compare_nocase_n(const char *a, const char *b, uint32_t count);

bool platform_dir_create(const char *path);
bool platform_dir_remove(const char *path);
bool platform_dir_first(platform_dir_listing *listing, const char *pattern);
bool platform_dir_next(platform_dir_listing *listing);
void platform_dir_close(platform_dir_listing *listing);

#endif
