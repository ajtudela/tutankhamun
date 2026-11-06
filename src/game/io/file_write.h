// The file writers, file dialog listing and settings_save of file_write.c.

#ifndef IO_FILE_WRITE_H
#define IO_FILE_WRITE_H

#include <stdint.h>

void settings_save(void);
void dir_list_files(char maps);
int32_t io_write_file_part(const char *path, const void *buffer, int32_t size, int32_t offset);
int32_t io_append_file(const char *path, const void *buffer, int32_t size);
int32_t io_write_file(const char *path, const void *buffer, int32_t size);

#endif
