// The platform's directories: creating and removing the player and saved game directories, and listing the files
// that match a pattern (the file dialogs).

#ifndef PLATFORM_FILE_H
#define PLATFORM_FILE_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    void *handle;
    const char *name;
    uint64_t native[42];
} platform_dir_listing;

bool platform_dir_create(const char *path);
bool platform_dir_remove(const char *path);
bool platform_dir_first(platform_dir_listing *listing, const char *pattern);
bool platform_dir_next(platform_dir_listing *listing);
void platform_dir_close(platform_dir_listing *listing);

#endif
