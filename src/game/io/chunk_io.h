// One compressed chunk of a saved game or scenario: a 4-byte header (the compressed size, CHUNK_STORED, or 0 when all
// bytes are zero), then the data. Also the chunk table entry (memory, element size and count, converters by version)
// and the schema record a file keeps per chunk.

#ifndef IO_CHUNK_IO_H
#define IO_CHUNK_IO_H

#include <assert.h>
#include <stdint.h>

typedef void (*chunk_convert_fn)(void *element, int32_t has_data);

typedef struct {
    int32_t old_element_size;
    int32_t old_count;
    int32_t version;
    chunk_convert_fn convert;
} chunk_converter;

typedef struct {
    int32_t compressed;
    void *data;
    int32_t element_size;
    int32_t count;
    const chunk_converter *converter;
} chunk_entry;

typedef struct {
    int32_t compressed;
    uint32_t reserved_1;
    int32_t element_size;
    int32_t count;
    uint32_t reserved_2;
} chunk_schema_record;

enum chunk_schema_size {
    CHUNK_SCHEMA_RECORDS = 300,
};

typedef chunk_schema_record chunk_schema_table[CHUNK_SCHEMA_RECORDS];

static_assert(sizeof(chunk_schema_record) == 20, "chunk_schema_record is a savegame record of 20 bytes");

#define CHUNK_STORED 0x80000000
#define CHUNK_MAX_SIZE 1200000

int32_t io_write_compressed_chunk(int32_t fd, const void *data, int32_t size);

int32_t io_read_compressed_chunk(int32_t fd, void *data, int32_t size);

#endif
