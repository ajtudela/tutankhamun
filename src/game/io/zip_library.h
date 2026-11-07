// The PKWare Data Compression Library inside io/zip.c: its result codes, callbacks, state structures and
// functions. The state layout is part of the output: the matcher reads past the used part of its tables.

#ifndef IO_ZIP_LIBRARY_H
#define IO_ZIP_LIBRARY_H

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

enum pk_result {
    PK_SUCCESS = 0,
    PK_INVALID_DICTSIZE = 1,
    PK_INVALID_MODE = 2,
    PK_TOO_FEW_INPUT_BYTES = 3,
    PK_ERROR_DECODING = 4,
};

enum pk_compression_type {
    PK_CMP_BINARY = 0,
    PK_CMP_ASCII = 1,
};

enum pk_code {
    PK_LITERAL_END = 0x305,
    PK_DECODE_ERROR = 0x306,
    PK_MAX_REP_LENGTH = 0x204,
};

enum pk_buffer_size {
    PK_WORK_BUFFER_SIZE = 0x8DD8,
};

typedef uint32_t pk_read_func(uint8_t *buffer, uint32_t *size, void *param);
typedef void pk_write_func(uint8_t *buffer, uint32_t *size, void *param);

typedef struct {
    uint32_t distance;
    uint32_t out_bytes;
    uint32_t out_bits;
    uint32_t dsize_bits;
    uint32_t dsize_mask;
    uint32_t ctype;
    uint32_t dsize_bytes;
    uint8_t dist_bits[64];
    uint8_t dist_codes[64];
    uint8_t nch_bits[774];
    uint16_t nch_codes[774];
    uint16_t reserved_1;
    void *param;
    pk_read_func *read_buf;
    pk_write_func *write_buf;
    int16_t match_next[516];
    uint32_t reserved_2;
    uint16_t hash_to_index[2304];
    uint16_t hash_to_index_end;
    uint8_t out_buff[2050];
    uint8_t work_buff[8708];
    uint16_t hash_offs[8708];
} pk_implode_state;

static_assert(sizeof(void *) != 4 || sizeof(pk_implode_state) == PK_WORK_BUFFER_SIZE, "pk_implode_state fills the original work buffer on 32 bits");
static_assert(offsetof(pk_implode_state, hash_offs) - offsetof(pk_implode_state, match_next) == 0x4014, "the matcher's tables keep their distances");

typedef struct {
    uint32_t reserved_1;
    uint32_t ctype;
    uint32_t out_pos;
    uint32_t dsize_bits;
    uint32_t dsize_mask;
    uint32_t bit_buff;
    uint32_t extra_bits;
    uint32_t in_pos;
    uint32_t in_bytes;
    void *param;
    pk_read_func *read_buf;
    pk_write_func *write_buf;
    uint8_t out_buff[0x2204];
    uint8_t in_buff[0x800];
    uint8_t dist_pos_codes[0x100];
    uint8_t length_codes[0x100];
    uint8_t literal_codes[0x100];
    uint8_t literal_codes_4[0x100];
    uint8_t literal_codes_6[0x80];
    uint8_t literal_codes_8[0x100];
    uint8_t ch_bits_asc[0x100];
    uint8_t dist_bits[0x40];
    uint8_t len_bits[0x10];
    uint8_t ex_len_bits[0x10];
    uint16_t len_base[0x10];
} pk_explode_state;

uint32_t zip_input_func(uint8_t *buffer, uint32_t *size, void *param);
void zip_output_func(uint8_t *buffer, uint32_t *size, void *param);
uint32_t pk_crc32(const uint8_t *buffer, const uint32_t *size, const uint32_t *crc);
uint32_t pk_implode(pk_read_func *read_buf, pk_write_func *write_buf, pk_implode_state *buf, void *param, const uint32_t *type, const uint32_t *dsize);
void pk_implode_data(pk_implode_state *buf);
uint32_t pk_implode_determine_copy(pk_implode_state *buf, uint8_t *input);
void pk_implode_write_bits(pk_implode_state *buf, uint32_t num_bits, uint32_t value);
void pk_memset(void *buffer, int32_t fill_byte, uint32_t length);
void pk_implode_flush_full_buffer(pk_implode_state *buf);
void pk_implode_analyze_input(pk_implode_state *buf, uint8_t *start, uint8_t *end);
uint32_t pk_explode(pk_read_func *read_buf, pk_write_func *write_buf, pk_explode_state *buf, void *param);
uint32_t pk_explode_data(pk_explode_state *buf);
uint32_t pk_explode_decode_next_token(pk_explode_state *buf);
uint32_t pk_explode_get_copy_offset(pk_explode_state *buf, uint32_t length);
int32_t pk_explode_set_bits_used(pk_explode_state *buf, uint32_t num_bits);
void pk_explode_construct_jump_table(int32_t size, const uint8_t *bits, const uint8_t *codes, uint8_t *jump);
void pk_explode_construct_literal_tables(pk_explode_state *buf);
void pk_memcpy(void *dst, const void *src, uint32_t length);

#endif
