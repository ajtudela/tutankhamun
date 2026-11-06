// Measuring and drawing text, numbers, amounts and years, the text field's cursor and the hash mode.

#ifndef GRAPHICS_TEXT_DRAW_H
#define GRAPHICS_TEXT_DRAW_H

#include <stdint.h>

enum text_number_prefix {
    NUMBER_PREFIX_NONE = '@',
    NUMBER_PREFIX_PLUS = '+',
};

enum text_hash_mode {
    TEXT_HASH_SPACE,
    TEXT_HASH_SUBSTITUTE,
};

enum text_line_buffer {
    TEXT_LINE_BUFFER_BYTES = 200,
};

enum text_year_bc_spacing {
    TEXT_YEAR_BC_SPACED,
    TEXT_YEAR_BC_CLOSER,
    TEXT_YEAR_BC_CLOSEST,
};

uint8_t text_hash_next_char(void);
uint8_t text_letter_image_id(int32_t letter_index);
int32_t text_draw_character(int32_t font, uint8_t letter_index, int32_t x, int32_t y);
int32_t text_get_width(const uint8_t *str, int32_t font);
int32_t text_get_letter_width(uint8_t c, int32_t font);
int32_t text_get_word_width(const uint8_t *str, int32_t font);
void text_draw(const uint8_t *str, int32_t x, int32_t y, int32_t font, int32_t color);
void text_draw_centered(const uint8_t *str, int32_t x, int32_t y, int32_t box_width, int32_t font, int32_t color);
void text_draw_multiline(const char *str, int32_t x, int32_t y, int32_t box_width, int32_t font, int32_t color);
void text_draw_number(int32_t value, int32_t prefix, const char *postfix, int32_t x, int32_t y, int32_t font, int32_t color);
void text_draw_number_centered(int32_t value, int32_t prefix, const char *postfix, int32_t x, int32_t y, int32_t box_width, int32_t font, int32_t color);
int32_t lang_text_get_width(int32_t group, int32_t index, int32_t font);
void lang_text_draw(int32_t group, int32_t index, int32_t x, int32_t y, int32_t font, int32_t color);
void lang_text_draw_centered(int32_t group, int32_t index, int32_t x, int32_t y, int32_t box_width, int32_t font, int32_t color);
void lang_text_draw_multiline(int32_t group, int32_t index, int32_t x, int32_t y, int32_t box_width, int32_t font, int32_t color);
void lang_text_draw_amount(int32_t group, int32_t type, int32_t amount, int32_t x, int32_t y, int32_t font, int32_t color);
void lang_text_draw_year(int32_t bc_spacing, int32_t year, int32_t x, int32_t y, int32_t font, int32_t color);
void graphics_draw_text_cursor(int32_t x, int32_t y, int32_t color);

#endif
