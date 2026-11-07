// Compressed chunks of saved games and scenarios on top of zip.c: each chunk goes through the game's scratch buffer
// and is stored uncompressed when it does not get smaller.
//
// A stored chunk of 0 bytes reads 0 bytes, which fails like the end of the file, and the compressed size is not checked
// against the scratch buffer.

#include "io/chunk_io.h"

#include "graphics/graphics_data.h"
#include "io/zip.h"
#include "platform/platform_file.h"

#include <stdint.h>

uint8_t *chunk_buffer;

uint32_t chunk_header;

int32_t chunk_size;

uint32_t chunk_crc;

int32_t io_write_compressed_chunk(int32_t fd, const void *data, int32_t size)
{
    chunk_buffer = (uint8_t *) spare_screen;
    if (size > CHUNK_MAX_SIZE) {
        return 0;
    }
    chunk_size = size;
    if (zip_compress(&chunk_crc, &chunk_header, data, spare_screen, size)) {
        platform_file_write(fd, &chunk_header, sizeof(chunk_header));
        platform_file_write(fd, chunk_buffer, chunk_header);
        return 1;
    }
    chunk_header = CHUNK_STORED;
    platform_file_write(fd, &chunk_header, sizeof(chunk_header));
    platform_file_write(fd, data, size);
    return 1;
}

int32_t io_read_compressed_chunk(int32_t fd, void *data, int32_t size)
{
    chunk_buffer = (uint8_t *) spare_screen;
    if (size > CHUNK_MAX_SIZE) {
        return 0;
    }
    chunk_size = size;
    if (!platform_file_read(fd, &chunk_header, sizeof(chunk_header))) {
        return 0;
    }
    if (chunk_header == CHUNK_STORED) {
        if (!platform_file_read(fd, data, size)) {
            return 0;
        }
    } else if (chunk_header == 0) {
        uint8_t *p = data;
        for (uint32_t i = 0; i < (uint32_t) size; i++) {
            p[i] = 0;
        }
    } else {
        if (!platform_file_read(fd, chunk_buffer, chunk_header)) {
            return 0;
        }
        if (!zip_decompress(&chunk_crc, chunk_buffer, chunk_header, data, size)) {
            return 0;
        }
    }
    return 1;
}
