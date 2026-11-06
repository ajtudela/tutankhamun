// Save-file compression: the PKWare Data Compression Library 1.11 (implode and explode) and the game's
// memory-to-memory wrappers around it, the same library as in Caesar III. The wrappers keep a CRC-32 of the
// uncompressed data (stored, never checked).
//
// The compressor reuses the match found for the next byte instead of searching again, so the output is the game's byte
// for byte, and explode writes its last block even when it is empty.

#include "io/zip.h"

#include "io/io_data.h"
#include "io/zip_library.h"
#include "io/zip_tables.h"
#include "platform/status_log.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

enum zip_mode {
    ZIP_MODE_COMPRESS = 1,
    ZIP_MODE_DECOMPRESS = 2,
};

enum zip_rule {
    PK_TABLE_NO_COPY = -1,
};

static const uint32_t zip_crc_initial = 0xFFFFFFFF;

typedef struct {
    uint8_t *data;
    uint32_t pos;
    uint32_t size;
} zip_buffer;

typedef struct {
    zip_buffer buffers[3];
    int32_t mode;
    uint32_t crc;
    uint32_t compressed_size;
    int32_t stop;
} zip_token;

uint32_t zip_input_func(uint8_t *buffer, uint32_t *size, void *param)
{
    zip_token *token = param;
    uint32_t length = *size;
    if (token->stop == 1) {
        return 0;
    }
    zip_buffer *input = token->mode == ZIP_MODE_COMPRESS ? &token->buffers[0] : &token->buffers[1];
    if (input->pos < input->size) {
        if (input->size - input->pos < length) {
            length = input->size - input->pos;
        }
        memcpy(buffer, input->data + input->pos, length);
        input->pos += length;
    } else {
        length = 0;
    }
    if (token->mode == ZIP_MODE_COMPRESS) {
        token->crc = pk_crc32(buffer, &length, &token->crc);
    }
    return length;
}

void zip_output_func(uint8_t *buffer, uint32_t *size, void *param)
{
    zip_token *token = param;
    uint32_t length = *size;
    if (token->stop == 1) {
        return;
    }
    zip_buffer *output;
    if (token->mode == ZIP_MODE_COMPRESS) {
        token->compressed_size += length;
        output = &token->buffers[1];
    } else {
        output = &token->buffers[2];
    }
    if (output->pos >= output->size) {
        log_info_text("ERR:COMP2 Out of buffer space.", NULL);
        token->stop = 1;
        return;
    }
    if (output->size - output->pos < length) {
        log_info_text("ERR:COMP1 Corrupt.", NULL);
        token->stop = 1;
        return;
    }
    memcpy(output->data + output->pos, buffer, length);
    output->pos += length;
    if (token->mode == ZIP_MODE_DECOMPRESS) {
        token->crc = pk_crc32(buffer, &length, &token->crc);
    }
}

int32_t zip_compress(uint32_t *crc, uint32_t *compressed_size, const void *data, void *out, int32_t size)
{
    pk_implode_state *buf = calloc(1, PK_WORK_BUFFER_SIZE);
    if (!buf) {
        return 0;
    }
    zip_token token;
    memset(&token, 0, sizeof(token));
    token.buffers[0].data = (uint8_t *) data;
    token.buffers[0].size = (uint32_t) size;
    token.buffers[1].data = out;
    token.buffers[1].size = (uint32_t) size;
    token.mode = ZIP_MODE_COMPRESS;
    token.crc = zip_crc_initial;
    int32_t ok = 1;
    if (pk_implode(zip_input_func, zip_output_func, buf, &token, &zip_compress_type, &zip_dictionary_size) == PK_SUCCESS && !token.stop) {
        token.crc = ~token.crc;
        *crc = token.crc;
        *compressed_size = token.compressed_size;
    } else {
        log_info_text("ERR:COMP Error occurred while compressing.", NULL);
        ok = 0;
    }
    free(buf);
    return ok;
}

int32_t zip_decompress(uint32_t *crc, const void *data, int32_t size, void *out, int32_t out_size)
{
    pk_explode_state *buf = calloc(1, PK_WORK_BUFFER_SIZE);
    if (!buf) {
        return 0;
    }
    zip_token token;
    memset(&token, 0, sizeof(token));
    token.buffers[1].data = (uint8_t *) data;
    token.buffers[1].size = (uint32_t) size;
    token.buffers[2].data = out;
    token.buffers[2].size = (uint32_t) out_size;
    token.mode = ZIP_MODE_DECOMPRESS;
    token.crc = zip_crc_initial;
    int32_t ok = 1;
    if (pk_explode(zip_input_func, zip_output_func, buf, &token) == PK_SUCCESS && !token.stop) {
        token.crc = ~token.crc;
        *crc = token.crc;
    } else {
        log_info_text("ERR:COMP Error uncompressing.", NULL);
        ok = 0;
    }
    free(buf);
    return ok;
}

uint32_t pk_crc32(const uint8_t *buffer, const uint32_t *size, const uint32_t *crc)
{
    uint32_t value = *crc;
    for (uint32_t length = *size; length; length--) {
        value = pk_crc_table[(*buffer++ ^ value) & 0xFF] ^ (value >> 8);
    }
    return value;
}

uint32_t pk_implode(pk_read_func *read_buf, pk_write_func *write_buf, pk_implode_state *buf, void *param, const uint32_t *type, const uint32_t *dsize)
{
    buf->read_buf = read_buf;
    buf->write_buf = write_buf;
    buf->dsize_bytes = *dsize;
    buf->ctype = *type;
    buf->param = param;
    buf->dsize_bits = 4;
    buf->dsize_mask = 0x0F;
    switch (*dsize) {
        case 0x1000:
            buf->dsize_bits++;
            buf->dsize_mask |= 0x20;
        case 0x800:
            buf->dsize_bits++;
            buf->dsize_mask |= 0x10;
        case 0x400:
            break;
        default:
            return PK_INVALID_DICTSIZE;
    }
    uint32_t code;
    if (*type == PK_CMP_BINARY) {
        for (code = 0; code < 0x100; code++) {
            buf->nch_bits[code] = 9;
            buf->nch_codes[code] = (uint16_t) (code * 2);
        }
    } else if (*type == PK_CMP_ASCII) {
        for (code = 0; code < 0x100; code++) {
            buf->nch_bits[code] = (uint8_t) (pk_literal_bits[code] + 1);
            buf->nch_codes[code] = (uint16_t) (pk_literal_code[code] * 2);
        }
    } else {
        return PK_INVALID_MODE;
    }
    for (int32_t length_code = 0; length_code < 0x10; length_code++) {
        uint32_t extra_bits = pk_copy_length_extra_bits[length_code];
        uint32_t base_bits = pk_copy_length_base_bits[length_code];
        for (uint32_t extra = 0; extra < (1u << extra_bits); extra++) {
            buf->nch_bits[code] = (uint8_t) (extra_bits + base_bits + 1);
            buf->nch_codes[code] = (uint16_t) ((extra << (base_bits + 1)) | (pk_copy_length_base_code[length_code] * 2) | 1);
            code++;
        }
    }
    pk_memcpy(buf->dist_codes, pk_copy_offset_code, sizeof(buf->dist_codes));
    pk_memcpy(buf->dist_bits, pk_copy_offset_bits, sizeof(buf->dist_bits));
    pk_implode_data(buf);
    return PK_SUCCESS;
}

static void write_literal(pk_implode_state *buf, uint8_t value)
{
    pk_implode_write_bits(buf, buf->nch_bits[value], buf->nch_codes[value]);
}

static void write_end_of_stream(pk_implode_state *buf)
{
    pk_implode_write_bits(buf, buf->nch_bits[PK_LITERAL_END], buf->nch_codes[PK_LITERAL_END]);
    if (buf->out_bits) {
        buf->out_bytes++;
    }
    buf->write_buf(buf->out_buff, &buf->out_bytes, buf->param);
}

static uint32_t write_copy(pk_implode_state *buf, uint32_t length)
{
    pk_implode_write_bits(buf, buf->nch_bits[length + 0xFE], buf->nch_codes[length + 0xFE]);
    if (length == 2) {
        uint32_t index = buf->distance >> 2;
        pk_implode_write_bits(buf, buf->dist_bits[index], buf->dist_codes[index]);
        pk_implode_write_bits(buf, 2, buf->distance & 3);
    } else {
        uint32_t index = buf->distance >> buf->dsize_bits;
        pk_implode_write_bits(buf, buf->dist_bits[index], buf->dist_codes[index]);
        pk_implode_write_bits(buf, buf->dsize_bits, buf->dsize_mask & buf->distance);
    }
    return length;
}

void pk_implode_data(pk_implode_state *buf)
{
    int32_t phase = 0;
    int32_t eof = 0;
    uint8_t *input = buf->work_buff + buf->dsize_bytes + PK_MAX_REP_LENGTH;
    buf->out_buff[0] = (uint8_t) buf->ctype;
    buf->out_buff[1] = (uint8_t) buf->dsize_bits;
    buf->out_bytes = 2;
    pk_memset(&buf->out_buff[2], 0, 0x800);
    buf->out_bits = 0;
    while (1) {
        uint32_t to_read = 0x1000;
        uint32_t loaded = 0;
        do {
            uint32_t size = to_read;
            uint32_t read = buf->read_buf(buf->work_buff + buf->dsize_bytes + PK_MAX_REP_LENGTH + loaded, &size, buf->param);
            if (!read) {
                if (!loaded && !phase) {
                    write_end_of_stream(buf);
                    return;
                }
                eof = 1;
                break;
            }
            loaded += read;
            to_read -= read;
        } while (to_read);
        uint8_t *input_end = buf->work_buff + buf->dsize_bytes + loaded;
        if (eof) {
            input_end += PK_MAX_REP_LENGTH;
        }
        if (phase == 0) {
            pk_implode_analyze_input(buf, input, input_end + 1);
            phase++;
            if (buf->dsize_bytes != 0x1000) {
                phase++;
            }
        } else if (phase == 1) {
            pk_implode_analyze_input(buf, input - buf->dsize_bytes + PK_MAX_REP_LENGTH, input_end + 1);
            phase++;
        } else {
            pk_implode_analyze_input(buf, input - buf->dsize_bytes, input_end + 1);
        }
        while (input < input_end) {
            uint32_t length = pk_implode_determine_copy(buf, input);
            while (1) {
                if (length == 0 || (length == 2 && buf->distance >= 0x100)) {
                    write_literal(buf, *input++);
                    break;
                }
                if (eof && input + length > input_end) {
                    length = (uint32_t) (input_end - input);
                    if (length < 2 || (length == 2 && buf->distance >= 0x100)) {
                        write_literal(buf, *input++);
                    } else {
                        input += write_copy(buf, length);
                    }
                    break;
                }
                if (length >= 8 || input + 1 >= input_end) {
                    input += write_copy(buf, length);
                    break;
                }
                uint32_t distance = buf->distance;
                uint32_t next_length = pk_implode_determine_copy(buf, input + 1);
                if (next_length <= length || (next_length <= length + 1 && distance <= 0x80)) {
                    buf->distance = distance;
                    input += write_copy(buf, length);
                    break;
                }
                write_literal(buf, *input++);
                length = next_length;
            }
        }
        if (eof) {
            write_end_of_stream(buf);
            return;
        }
        input -= 0x1000;
        pk_memcpy(buf->work_buff, buf->work_buff + 0x1000, buf->dsize_bytes + PK_MAX_REP_LENGTH);
    }
}

static void extend_match_next(pk_implode_state *buf, const uint8_t *input, int16_t *pos, int16_t *prefix, int32_t length)
{
    do {
        if (input[*pos] == input[*prefix] || (*prefix = buf->match_next[*prefix]) == -1) {
            (*pos)++;
            (*prefix)++;
            buf->match_next[*pos] = *prefix;
        }
    } while (*pos < length);
}

uint32_t pk_implode_determine_copy(pk_implode_state *buf, uint8_t *input)
{
    uint16_t *hash_entry = &buf->hash_to_index[input[1] * 5 + input[0] * 4];
    uint32_t index = *hash_entry;
    uint32_t min_offset = (uint32_t) (input - buf->work_buff) - buf->dsize_bytes + 1;
    while (buf->hash_offs[index] < min_offset) {
        index++;
    }
    *hash_entry = (uint16_t) index;
    uint8_t *prev_input = input - 1;
    uint8_t *rep = buf->work_buff + buf->hash_offs[index];
    if (prev_input <= rep) {
        return 0;
    }
    int32_t rep_length = 1;
    while (1) {
        if (rep[rep_length - 1] == input[rep_length - 1] && *rep == *input) {
            uint8_t *match = rep + 1;
            const uint8_t *in = input;
            int32_t length = 2;
            do {
                match++;
                if (*match != in[2]) {
                    break;
                }
                length++;
                in++;
            } while (length < PK_MAX_REP_LENGTH);
            if (length >= rep_length) {
                buf->distance = (uint32_t) (input - match + length - 1);
                rep_length = length;
                if (length > 10) {
                    break;
                }
            }
        }
        index++;
        rep = buf->work_buff + buf->hash_offs[index];
        if (prev_input <= rep) {
            return rep_length < 2 ? 0 : (uint32_t) rep_length;
        }
    }
    if (rep_length == PK_MAX_REP_LENGTH) {
        buf->distance--;
        return PK_MAX_REP_LENGTH;
    }
    if (buf->work_buff + buf->hash_offs[index + 1] >= prev_input) {
        return rep_length;
    }
    int16_t prefix = 0;
    int16_t pos = 1;
    buf->match_next[0] = -1;
    buf->match_next[1] = 0;
    extend_match_next(buf, input, &pos, &prefix, rep_length);
    int32_t length = rep_length;
    uint8_t *match_end = buf->work_buff + buf->hash_offs[index] + rep_length;
    while (1) {
        length = buf->match_next[length];
        if (length == -1) {
            length = 0;
        }
        do {
            index++;
            rep = buf->work_buff + buf->hash_offs[index];
            if (rep >= prev_input) {
                return rep_length;
            }
        } while (rep + length < match_end);
        if (rep[rep_length - 2] == input[rep_length - 2]) {
            if (rep + length != match_end) {
                length = 0;
                match_end = rep;
            }
        } else {
            do {
                index++;
                rep = buf->work_buff + buf->hash_offs[index];
                if (rep >= prev_input) {
                    return rep_length;
                }
            } while (rep[rep_length - 2] != input[rep_length - 2] || *rep != *input);
            length = 2;
            match_end = rep + 2;
        }
        while (input[length] == *match_end) {
            length++;
            if (length >= PK_MAX_REP_LENGTH) {
                break;
            }
            match_end++;
        }
        if (length >= rep_length) {
            buf->distance = (uint32_t) (input - rep - 1);
            if (length > rep_length) {
                rep_length = length;
                if (length == PK_MAX_REP_LENGTH) {
                    return PK_MAX_REP_LENGTH;
                }
                extend_match_next(buf, input, &pos, &prefix, length);
            }
        }
    }
}

void pk_implode_write_bits(pk_implode_state *buf, uint32_t num_bits, uint32_t value)
{
    if (num_bits > 8) {
        num_bits -= 8;
        pk_implode_write_bits(buf, 8, value);
        value >>= 8;
    }
    uint32_t bits_used = buf->out_bits;
    buf->out_buff[buf->out_bytes] |= (uint8_t) (value << bits_used);
    buf->out_bits += num_bits;
    if (buf->out_bits > 8) {
        buf->out_bytes++;
        buf->out_buff[buf->out_bytes] = (uint8_t) (value >> (8 - bits_used));
        buf->out_bits &= 7;
    } else {
        buf->out_bits &= 7;
        if (!buf->out_bits) {
            buf->out_bytes++;
        }
    }
    if (buf->out_bytes >= 0x800) {
        pk_implode_flush_full_buffer(buf);
    }
}

void pk_memset(void *buffer, int32_t fill_byte, uint32_t length)
{
    uint8_t *p = buffer;
    while (length--) {
        *p++ = (uint8_t) fill_byte;
    }
}

void pk_implode_flush_full_buffer(pk_implode_state *buf)
{
    uint32_t size = 0x800;
    buf->write_buf(buf->out_buff, &size, buf->param);
    uint8_t new_first_byte = buf->out_buff[0x800];
    uint8_t last_byte = buf->out_buff[buf->out_bytes];
    buf->out_bytes -= 0x800;
    pk_memset(buf->out_buff, 0, sizeof(buf->out_buff));
    if (buf->out_bytes) {
        buf->out_buff[0] = new_first_byte;
    }
    if (buf->out_bits) {
        buf->out_buff[buf->out_bytes] = last_byte;
    }
}

void pk_implode_analyze_input(pk_implode_state *buf, uint8_t *start, uint8_t *end)
{
    memset(buf->hash_to_index, 0, sizeof(buf->hash_to_index));
    uint8_t *p = start;
    do {
        buf->hash_to_index[p[1] * 5 + p[0] * 4]++;
        p++;
    } while (p < end);
    uint16_t total = 0;
    for (int32_t i = 0; i < 2304; i++) {
        total += buf->hash_to_index[i];
        buf->hash_to_index[i] = total;
    }
    p = end - 1;
    do {
        uint16_t index = --buf->hash_to_index[p[0] * 4 + p[1] * 5];
        buf->hash_offs[index] = (uint16_t) (p - buf->work_buff);
        p--;
    } while (p >= start);
}

uint32_t pk_explode(pk_read_func *read_buf, pk_write_func *write_buf, pk_explode_state *buf, void *param)
{
    buf->read_buf = read_buf;
    buf->write_buf = write_buf;
    buf->param = param;
    buf->in_pos = sizeof(buf->in_buff);
    buf->in_bytes = buf->read_buf(buf->in_buff, &buf->in_pos, buf->param);
    if (buf->in_bytes <= 4) {
        return PK_TOO_FEW_INPUT_BYTES;
    }
    buf->ctype = buf->in_buff[0];
    buf->dsize_bits = buf->in_buff[1];
    buf->bit_buff = buf->in_buff[2];
    buf->extra_bits = 0;
    buf->in_pos = 3;
    if (buf->dsize_bits < 4 || buf->dsize_bits > 6) {
        return PK_INVALID_DICTSIZE;
    }
    buf->dsize_mask = 0xFFFF >> (16 - buf->dsize_bits);
    if (buf->ctype != PK_CMP_BINARY) {
        if (buf->ctype != PK_CMP_ASCII) {
            return PK_INVALID_MODE;
        }
        pk_memcpy(buf->ch_bits_asc, pk_literal_bits, sizeof(buf->ch_bits_asc));
        pk_explode_construct_literal_tables(buf);
    }
    pk_memcpy(buf->len_bits, pk_copy_length_base_bits, sizeof(buf->len_bits));
    pk_explode_construct_jump_table(0x10, buf->len_bits, pk_copy_length_base_code, buf->length_codes);
    pk_memcpy(buf->ex_len_bits, pk_copy_length_extra_bits, sizeof(buf->ex_len_bits));
    pk_memcpy(buf->len_base, pk_copy_length_base_value, sizeof(buf->len_base));
    pk_memcpy(buf->dist_bits, pk_copy_offset_bits, sizeof(buf->dist_bits));
    pk_explode_construct_jump_table(0x40, buf->dist_bits, pk_copy_offset_code, buf->dist_pos_codes);
    return pk_explode_data(buf) == PK_DECODE_ERROR ? PK_ERROR_DECODING : PK_SUCCESS;
}

uint32_t pk_explode_data(pk_explode_state *buf)
{
    uint32_t next;
    buf->out_pos = 0x1000;
    while (1) {
        next = pk_explode_decode_next_token(buf);
        if (next >= PK_LITERAL_END) {
            break;
        }
        if (next < 0x100) {
            buf->out_buff[buf->out_pos++] = (uint8_t) next;
        } else {
            next -= 0xFE;
            uint32_t distance = pk_explode_get_copy_offset(buf, next);
            if (!distance) {
                next = PK_DECODE_ERROR;
                break;
            }
            uint8_t *target = &buf->out_buff[buf->out_pos];
            const uint8_t *source = target - distance;
            buf->out_pos += next;
            do {
                *target++ = *source++;
            } while (--next);
        }
        if (buf->out_pos >= 0x2000) {
            next = 0x1000;
            buf->write_buf(&buf->out_buff[0x1000], &next, buf->param);
            pk_memcpy(buf->out_buff, &buf->out_buff[0x1000], buf->out_pos - 0x1000);
            buf->out_pos -= 0x1000;
        }
    }
    uint32_t remaining = buf->out_pos - 0x1000;
    buf->write_buf(&buf->out_buff[0x1000], &remaining, buf->param);
    return next;
}

uint32_t pk_explode_decode_next_token(pk_explode_state *buf)
{
    if (buf->bit_buff & 1) {
        if (pk_explode_set_bits_used(buf, 1)) {
            return PK_DECODE_ERROR;
        }
        uint32_t index = buf->length_codes[buf->bit_buff & 0xFF];
        if (pk_explode_set_bits_used(buf, buf->len_bits[index])) {
            return PK_DECODE_ERROR;
        }
        uint32_t extra_bits = buf->ex_len_bits[index];
        if (extra_bits) {
            uint32_t extra = ((1u << extra_bits) - 1) & buf->bit_buff;
            if (pk_explode_set_bits_used(buf, extra_bits) && index + extra != 0x10E) {
                return PK_DECODE_ERROR;
            }
            index = buf->len_base[index] + extra;
        }
        return index + 0x100;
    }
    if (pk_explode_set_bits_used(buf, 1)) {
        return PK_DECODE_ERROR;
    }
    uint32_t value;
    if (buf->ctype == PK_CMP_BINARY) {
        value = buf->bit_buff & 0xFF;
        if (pk_explode_set_bits_used(buf, 8)) {
            return PK_DECODE_ERROR;
        }
        return value;
    }
    value = buf->bit_buff & 0xFF;
    if (value == 0) {
        if (pk_explode_set_bits_used(buf, 8)) {
            return PK_DECODE_ERROR;
        }
        value = buf->literal_codes_8[buf->bit_buff & 0xFF];
    } else {
        value = buf->literal_codes[value];
        if (value == 0xFF) {
            if ((buf->bit_buff & 0x3F) == 0) {
                if (pk_explode_set_bits_used(buf, 6)) {
                    return PK_DECODE_ERROR;
                }
                value = buf->literal_codes_6[buf->bit_buff & 0x7F];
            } else {
                if (pk_explode_set_bits_used(buf, 4)) {
                    return PK_DECODE_ERROR;
                }
                value = buf->literal_codes_4[buf->bit_buff & 0xFF];
            }
        }
    }
    if (pk_explode_set_bits_used(buf, buf->ch_bits_asc[value])) {
        return PK_DECODE_ERROR;
    }
    return value;
}

uint32_t pk_explode_get_copy_offset(pk_explode_state *buf, uint32_t length)
{
    uint32_t index = buf->dist_pos_codes[buf->bit_buff & 0xFF];
    if (pk_explode_set_bits_used(buf, buf->dist_bits[index])) {
        return 0;
    }
    uint32_t offset;
    if (length == 2) {
        offset = (index << 2) | (buf->bit_buff & 3);
        if (pk_explode_set_bits_used(buf, 2)) {
            return 0;
        }
    } else {
        offset = (index << buf->dsize_bits) | (buf->dsize_mask & buf->bit_buff);
        if (pk_explode_set_bits_used(buf, buf->dsize_bits)) {
            return 0;
        }
    }
    return offset + 1;
}

int32_t pk_explode_set_bits_used(pk_explode_state *buf, uint32_t num_bits)
{
    if (buf->extra_bits >= num_bits) {
        buf->extra_bits -= num_bits;
        buf->bit_buff >>= num_bits;
        return 0;
    }
    buf->bit_buff >>= buf->extra_bits;
    if (buf->in_pos == buf->in_bytes) {
        buf->in_pos = sizeof(buf->in_buff);
        buf->in_bytes = buf->read_buf(buf->in_buff, &buf->in_pos, buf->param);
        if (!buf->in_bytes) {
            return 1;
        }
        buf->in_pos = 0;
    }
    buf->bit_buff |= (uint32_t) buf->in_buff[buf->in_pos++] << 8;
    buf->bit_buff >>= num_bits - buf->extra_bits;
    buf->extra_bits = buf->extra_bits - num_bits + 8;
    return 0;
}

void pk_explode_construct_jump_table(int32_t size, const uint8_t *bits, const uint8_t *codes, uint8_t *jump)
{
    for (int32_t i = size - 1; i >= 0; i--) {
        uint32_t bit = bits[i];
        uint32_t code = codes[i];
        do {
            jump[code] = (uint8_t) i;
            code += 1u << bit;
        } while (code < 0x100);
    }
}

void pk_explode_construct_literal_tables(pk_explode_state *buf)
{
    for (int32_t i = 0xFF; i >= 0; i--) {
        uint8_t *bits = &buf->ch_bits_asc[i];
        uint32_t code = pk_literal_code[i];
        uint32_t index;
        if (*bits <= 8) {
            index = code;
            do {
                buf->literal_codes[index] = (uint8_t) i;
                index += 1u << *bits;
            } while (index < 0x100);
        } else if ((code & 0xFF) == 0) {
            *bits -= 8;
            index = code >> 8;
            do {
                buf->literal_codes_8[index] = (uint8_t) i;
                index += 1u << *bits;
            } while (index < 0x100);
        } else {
            buf->literal_codes[code & 0xFF] = 0xFF;
            if ((code & 0x3F) == 0) {
                *bits -= 6;
                index = code >> 6;
                do {
                    buf->literal_codes_6[index] = (uint8_t) i;
                    index += 1u << *bits;
                } while (index < 0x80);
            } else {
                *bits -= 4;
                index = code >> 4;
                do {
                    buf->literal_codes_4[index] = (uint8_t) i;
                    index += 1u << *bits;
                } while (index < 0x100);
            }
        }
    }
}

void pk_memcpy(void *dst, const void *src, uint32_t length)
{
    uint8_t *d = dst;
    const uint8_t *s = src;
    while (length--) {
        *d++ = *s++;
    }
}

static int32_t table_equals_copy(const void *table, size_t size, int32_t copy_offset)
{
    const uint8_t *reference = (const uint8_t *) pk_tables_reference;
    return (copy_offset == PK_TABLE_NO_COPY) || (memcmp(table, reference + copy_offset, size) == 0);
}

int32_t zip_tables_equal_reference(void)
{
    static const struct {
        const void *table;
        int32_t implode_copy_offset;
        int32_t explode_copy_offset;
        size_t size;
    } tables[] = {
        { pk_crc_table, 0, PK_TABLE_NO_COPY, sizeof(pk_crc_table) },
        { pk_copy_offset_bits, 0x400, 0x878, sizeof(pk_copy_offset_bits) },
        { pk_copy_offset_code, 0x440, 0x8B8, sizeof(pk_copy_offset_code) },
        { pk_copy_length_extra_bits, 0x480, 0x8F8, sizeof(pk_copy_length_extra_bits) },
        { pk_copy_length_base_bits, 0x490, 0x928, sizeof(pk_copy_length_base_bits) },
        { pk_copy_length_base_code, 0x4A0, 0x938, sizeof(pk_copy_length_base_code) },
        { pk_copy_length_base_value, PK_TABLE_NO_COPY, 0x908, sizeof(pk_copy_length_base_value) },
        { pk_literal_bits, 0x4B0, 0x948, sizeof(pk_literal_bits) },
        { pk_literal_code, 0x5B0, 0xA48, sizeof(pk_literal_code) },
    };
    int32_t equal = 1;
    for (size_t i = 0; i < sizeof(tables) / sizeof(tables[0]); i++) {
        if (!table_equals_copy(tables[i].table, tables[i].size, tables[i].implode_copy_offset)) {
            equal = 0;
        }
        if (!table_equals_copy(tables[i].table, tables[i].size, tables[i].explode_copy_offset)) {
            equal = 0;
        }
    }
    return equal;
}
