// The PKWare Data Compression Library 1.11 the game links with and the game's memory wrappers around it.
// Compressing fails for data that does not get smaller.

#ifndef IO_ZIP_H
#define IO_ZIP_H

#include <stdint.h>

int32_t zip_compress(uint32_t *crc, uint32_t *compressed_size, const void *data, void *out, int32_t size);

int32_t zip_decompress(uint32_t *crc, const void *data, int32_t size, void *out, int32_t out_size);

int32_t zip_tables_equal_reference(void);

#endif
