// The game's file helpers (io/file.c): opening, reading and seeking files, with the fallbacks to the 555 folder and
// to the CD.

#ifndef GAME_IO_FILE_H
#define GAME_IO_FILE_H

#include "io/file_path.h"

#include <stdbool.h>
#include <stdint.h>

void file_store_extension(const char *path);
void file_change_extension(char *path);
void file_set_extension(char *path, const char *extension);
int32_t file_exists(const char *path);
int32_t file_get_size(const char *path);
int32_t file_get_size_from_offset(const char *path, int32_t offset);
int32_t io_read_file_part(const char *path, void *buffer, int32_t size, int32_t offset);
int32_t io_read_file_into_buffer(const char *path, void *buffer, int32_t offset);
bool highscore_read_file(void *table, const char *path);

#endif
