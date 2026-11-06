// The functions of file_read_all.c that other files call.

#ifndef IO_FILE_READ_ALL_H
#define IO_FILE_READ_ALL_H

#include <stdint.h>

int32_t io_read_file_whole(const char *path, void *buffer);

#endif
