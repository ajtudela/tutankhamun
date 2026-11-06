// Text drawing and measuring with the font images: characters, strings, centred and multiline text, numbers,
// amounts and years, the lang_text_* variants and the blinking cursor of a text field. Drawing supports the hash
// mode ('#' replaced by substituted characters) and records where the input cursor falls.
//
// Widths are not the same everywhere:
// - measuring and drawing use different space widths and letter spacing for the large fonts;
// - tall letters are raised except the c cedilla;
// - after a text the width grows by one more space.

#include "graphics/text_draw.h"
#include "core/text.h"
#include "graphics/graphics_data.h"
#include "graphics/graphics_present.h"
#include "graphics/graphics_primitives.h"
#include "graphics/image_collection.h"
#include "graphics/image_draw.h"
#include "text/lang.h"
#include "widget/widget_data.h"
#include "window/window_data.h"
#include "window/window_image_group.h"
#include <stdint.h>
#include <string.h>

int32_t g_line_height;

int32_t g_shadow_b;

int32_t g_cursor_x;

uint8_t g_skip_double_space;

char hash_subst_buffer[100];

const uint8_t letter_image_ids[240] = {
        0, 63, 64, 0, 0, 65, 0, 74, 67, 68, 66, 70, 78, 69, 79, 77, 62, 53, 54, 55, 56, 57, 58, 59,
        60, 61, 72, 73, 0, 71, 0, 75, 0, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41,
        42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 0, 99, 0, 0, 80, 0, 1, 2, 3, 4, 5, 6, 7,
        8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 0, 0, 0, 0, 0,
        101, 97, 86, 84, 81, 83, 1, 103, 129, 85, 87, 89, 110, 93, 105, 27, 106, 103, 109, 96, 93, 95, 100, 99,
        25, 123, 107, 0, 111, 0, 0, 0, 82, 127, 94, 98, 102, 108, 1, 15, 128, 0, 0, 0, 0, 0, 0, 0,
        130, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 128, 114, 112, 113, 113, 105, 131, 109, 101,
        116, 106, 115, 115, 119, 117, 118, 118, 0, 108, 122, 120, 121, 121, 123, 0, 132, 126, 124, 125, 107, 51, 0, 104,
        83, 82, 84, 81, 81, 133, 103, 101, 87, 86, 88, 85, 91, 90, 92, 89, 0, 102, 95, 94, 96, 96, 93, 0,
        134, 99, 98, 100, 97, 25, 0, 25, 53, 53, 53, 0, 53, 53, 54, 0, 53, 54, 53, 0, 54, 53, 53, 0,
};

int32_t text_cursor_blink_time;

int32_t text_cursor_blink_on;

enum text_metric {
    TEXT_MEASURE_SPACE_LARGE = 10,
    TEXT_DRAW_SPACE_LARGE = 8,
    TEXT_SPACE_SMALL = 4,
    TEXT_SPACE_NORMAL = 6,
    TEXT_LETTER_SPACE = 4,
    TEXT_LINE_HEIGHT_LARGE = 23,
    TEXT_LINE_HEIGHT_LARGE_B = 24,
    TEXT_LINE_HEIGHT_SMALL = 9,
    TEXT_LINE_HEIGHT_NORMAL = 11,
    TEXT_GLYPH_RAISE = 3,
    TEXT_CURSOR_WIDTH_AT_END = 4,
    TEXT_MULTILINE_LINE_GAP = 5,
};

enum text_limit {
    TEXT_MEASURE_MAX_CHARS = 10000,
    TEXT_WORD_MAX_CHARS = 2000,
    TEXT_MULTILINE_MAX_LINES = 100,
};

enum text_letter {
    TEXT_LETTER_TALL_FIRST = 0x60,
    TEXT_LETTER_C_CEDILLA = 199,
    TEXT_CHAR_C_CEDILLA = 0xE7,
};

enum text_era_text {
    TEXT_ERA_BC,
    TEXT_ERA_AD,
};

enum text_year_bc_pixels {
    TEXT_YEAR_BC_CLOSER_PIXELS = 8,
    TEXT_YEAR_BC_CLOSEST_PIXELS = 11,
};

enum text_cursor {
    TEXT_CURSOR_OFF_MILLIS = 200,
    TEXT_CURSOR_ON_MILLIS = 400,
    TEXT_CURSOR_BAR_Y = 14,
    TEXT_CURSOR_BAR_HEIGHT = 2,
    TEXT_CURSOR_BEAM_TOP = -3,
    TEXT_CURSOR_BEAM_STEM_BOTTOM = 13,
    TEXT_CURSOR_BEAM_LEFT = -3,
    TEXT_CURSOR_BEAM_RIGHT = 1,
    TEXT_CURSOR_BEAM_STEM_X = -1,
};

uint8_t text_hash_next_char(void)
{
    char c = hash_subst_buffer[g_hash_pos];
    if (c == 0) {
        g_hash_pos = 0;
        return ' ';
    }
    g_hash_pos++;
    return (uint8_t) c;
}

uint8_t text_letter_image_id(int32_t letter_index)
{
    if (letter_index < 0 || letter_index >= (int32_t) sizeof(letter_image_ids)) {
        return 0;
    }
    return letter_image_ids[letter_index];
}

int32_t text_draw_character(int32_t font, uint8_t letter_index, int32_t x, int32_t y)
{
    g_last_image_id = text_letter_image_id(letter_index);
    if (g_last_image_id == 0) {
        return 0;
    }
    int32_t image_id = (g_last_image_id - 1) + font;
    int32_t raise = image_header_get(image_id)->height - g_line_height;
    if (raise < 0) {
        raise = 0;
    }
    if (letter_index < TEXT_LETTER_TALL_FIRST) {
        raise = 0;
    }
    if (letter_index == TEXT_LETTER_C_CEDILLA || letter_index == TEXT_CHAR_C_CEDILLA) {
        raise = 0;
    }
    image_draw(image_id, x, (y - TEXT_GLYPH_RAISE) - raise);
    return image_header_get(image_id)->width;
}

int32_t text_get_width(const uint8_t *str, int32_t font)
{
    int32_t width = 0;
    int32_t chars_left = TEXT_MEASURE_MAX_CHARS;
    int32_t space_width;
    int32_t letter_spacing;
    if (font == image_group(IMAGE_GROUP_FONT_LARGE) || font == image_group(IMAGE_GROUP_FONT_LARGE_B)) {
        space_width = TEXT_MEASURE_SPACE_LARGE;
        letter_spacing = 1;
    } else if (font == image_group(IMAGE_GROUP_FONT_SMALL)) {
        space_width = TEXT_SPACE_SMALL;
        letter_spacing = 1;
    } else {
        space_width = TEXT_SPACE_NORMAL;
        letter_spacing = 0;
    }
    while (*str && chars_left > 0) {
        uint8_t c = *str++;
        if (c == ' ') {
            width += space_width;
        } else {
            g_last_image_id = text_letter_image_id(c - ' ');
            if (g_last_image_id != 0) {
                width += image_header_get((g_last_image_id - 1) + font)->width + letter_spacing;
            }
        }
        chars_left--;
    }
    return width;
}

int32_t text_get_letter_width(uint8_t c, int32_t font)
{
    if (c == 0) {
        return 0;
    }
    if (c == ' ') {
        return TEXT_LETTER_SPACE;
    }
    g_last_image_id = text_letter_image_id(c - ' ');
    if (g_last_image_id == 0) {
        return 0;
    }
    return image_header_get((g_last_image_id - 1) + font)->width + 1;
}

static int32_t draw_letter_with_shadow(int32_t font, uint8_t letter_index, int32_t x, int32_t y, int32_t color)
{
    if (text_draw_shadow != 0) {
        g_draw_color = (uint16_t) color;
        return text_draw_character(font, letter_index, x, y);
    }
    if (g_shadow_c != 0) {
        g_draw_color = 0;
        text_draw_character(font, letter_index, x, y - 1);
        g_draw_color = 0;
        text_draw_character(font, letter_index, x, y + 1);
        g_draw_color = (uint16_t) color;
        return text_draw_character(font, letter_index, x, y);
    }
    if (g_shadow_b != 0) {
        g_draw_color = 0;
        text_draw_character(font, letter_index, x + 1, y + 1);
        g_draw_color = (uint16_t) color;
        return text_draw_character(font, letter_index, x - 1, y);
    }
    g_draw_color = (uint16_t) color;
    return text_draw_character(font, letter_index, x, y);
}

static void remember_cursor(int32_t width)
{
    g_cursor_x = g_text_width;
    text_cursor_y = g_draw_y_accum;
    g_cursor_seen = 1;
    g_cursor_width = width;
}

void text_draw(const uint8_t *str, int32_t x, int32_t y, int32_t font, int32_t color)
{
    int32_t space_width;
    int32_t letter_spacing;
    if (font == image_group(IMAGE_GROUP_FONT_LARGE)) {
        space_width = TEXT_DRAW_SPACE_LARGE;
        g_line_height = TEXT_LINE_HEIGHT_LARGE;
        letter_spacing = 0;
    } else if (font == image_group(IMAGE_GROUP_FONT_LARGE_B)) {
        space_width = TEXT_DRAW_SPACE_LARGE;
        g_line_height = TEXT_LINE_HEIGHT_LARGE_B;
        letter_spacing = 0;
    } else if (font == image_group(IMAGE_GROUP_FONT_SMALL)) {
        space_width = TEXT_SPACE_SMALL;
        g_line_height = TEXT_LINE_HEIGHT_SMALL;
        letter_spacing = 1;
    } else {
        space_width = TEXT_SPACE_NORMAL;
        g_line_height = TEXT_LINE_HEIGHT_NORMAL;
        letter_spacing = 0;
    }
    g_draw_color = (uint16_t) color;
    uint8_t c = *str;
    while (c != 0) {
        uint8_t raw = c;
        if (c == '#') {
            if (g_hash_mode == TEXT_HASH_SUBSTITUTE) {
                c = text_hash_next_char();
            } else {
                c = ' ';
            }
        }
        int32_t advance = 0;
        if (c == '_') {
            c = ' ';
        }
        if (c >= ' ') {
            uint8_t letter_index = (uint8_t) (c - ' ');
            advance = space_width;
            if (text_letter_image_id(letter_index) != 0) {
                advance = draw_letter_with_shadow(font, letter_index, x, y, color) + letter_spacing;
            }
            if (g_char_index == capture_cursor_position[keyboard_field_index] && g_cursor_seen == 0) {
                remember_cursor(advance);
            }
            x += advance;
            g_text_width += advance;
        }
        if (g_hash_pos == 0) {
            str++;
            c = *str;
        } else {
            c = text_hash_next_char();
        }
        g_char_index++;
        if (g_multiline_flag == 0 && g_skip_double_space != 0 && raw == c && (c == ' ' || c == '_')) {
            g_text_width -= advance;
            x -= advance;
        }
    }
    if (g_cursor_seen == 0) {
        remember_cursor(TEXT_CURSOR_WIDTH_AT_END);
    }
    g_text_width += space_width;
    g_hash_mode = TEXT_HASH_SPACE;
    g_hash_pos = 0;
    g_multiline_flag = 0;
}

void text_draw_centered(const uint8_t *str, int32_t x, int32_t y, int32_t box_width, int32_t font, int32_t color)
{
    int32_t offset = (box_width - text_get_width(str, font)) / 2;
    if (offset < 0) {
        offset = 0;
    }
    g_cursor_x_offset = offset;
    text_draw(str, offset + x, y, font, color);
}

static const uint8_t *lang_text(int32_t group, int32_t index)
{
    return (const uint8_t *) lang_get_string(group, index);
}

int32_t lang_text_get_width(int32_t group, int32_t index, int32_t font)
{
    return text_get_width(lang_text(group, index), font);
}

void lang_text_draw(int32_t group, int32_t index, int32_t x, int32_t y, int32_t font, int32_t color)
{
    text_draw(lang_text(group, index), x, y, font, color);
}

void lang_text_draw_centered(int32_t group, int32_t index, int32_t x, int32_t y, int32_t box_width, int32_t font, int32_t color)
{
    text_draw_centered(lang_text(group, index), x, y, box_width, font, color);
}

static const uint8_t *format_number(int32_t value, int32_t prefix, const char *postfix)
{
    string_from_int(value, 1, 0);
    if ((char) prefix != 0) {
        string_number_buffer[0] = (char) prefix;
    }
    int32_t i = (int32_t) strlen(string_number_buffer);
    while (*postfix) {
        string_number_buffer[i++] = *postfix++;
    }
    string_number_buffer[i] = 0;
    return (const uint8_t *) string_number_buffer;
}

void text_draw_number(int32_t value, int32_t prefix, const char *postfix, int32_t x, int32_t y, int32_t font, int32_t color)
{
    text_draw(format_number(value, prefix, postfix), x, y, font, color);
}

void text_draw_number_centered(int32_t value, int32_t prefix, const char *postfix, int32_t x, int32_t y, int32_t box_width, int32_t font, int32_t color)
{
    text_draw_centered(format_number(value, prefix, postfix), x, y, box_width, font, color);
}

void lang_text_draw_amount(int32_t group, int32_t type, int32_t amount, int32_t x, int32_t y, int32_t font, int32_t color)
{
    int32_t plural_offset = 1;
    if (amount == 1 || amount == -1) {
        plural_offset = 0;
    }
    g_text_width = 0;
    char prefix = (amount < 0) ? '-' : ' ';
    int32_t abs_amount = (amount < 0) ? -amount : amount;
    text_draw_number(abs_amount, prefix, empty_text, x, y, font, color);
    lang_text_draw(group, plural_offset + type, g_text_width + x, y, font, color);
}

void lang_text_draw_year(int32_t bc_spacing, int32_t year, int32_t x, int32_t y, int32_t font, int32_t color)
{
    g_text_width = 0;
    if (year < 0) {
        text_draw_number(-year, ' ', postfix_none, x, y, font, color);
        if (bc_spacing == TEXT_YEAR_BC_CLOSER) {
            g_text_width -= TEXT_YEAR_BC_CLOSER_PIXELS;
        } else if (bc_spacing == TEXT_YEAR_BC_CLOSEST) {
            g_text_width -= TEXT_YEAR_BC_CLOSEST_PIXELS;
        }
        lang_text_draw(LANG_GROUP_ERAS, TEXT_ERA_BC, g_text_width + x, y, font, color);
        return;
    }
    lang_text_draw(LANG_GROUP_ERAS, TEXT_ERA_AD, x, y, font, color);
    text_draw_number(year, ' ', postfix_none, g_text_width + x, y, font, color);
}

int32_t text_get_word_width(const uint8_t *str, int32_t font)
{
    int32_t word_seen = 0;
    g_word_chars = 0;
    int32_t width = 0;
    for (int32_t chars = 1; chars < TEXT_WORD_MAX_CHARS; chars++) {
        uint8_t c = *str++;
        if (c == 0) {
            return width;
        }
        if (c == ' ') {
            if (word_seen) {
                return width;
            }
            width += TEXT_LETTER_SPACE;
        } else if (c == '$') {
            if (word_seen) {
                return width;
            }
        } else if (c >= ' ') {
            width += text_get_letter_width(c, font);
            word_seen = 1;
        }
        g_word_chars++;
    }
    return width;
}

static const char *fill_line(const char *str, int32_t box_width, int32_t font, int32_t *has_more)
{
    int32_t line_index = 0;
    int32_t current_width = 0;
    while (current_width < box_width) {
        current_width += text_get_word_width((const uint8_t *) str, font);
        if (current_width >= box_width) {
            if (current_width == 0) {
                *has_more = 0;
            }
            break;
        }
        for (int32_t n = g_word_chars; n > 0; n--) {
            tmp_line_buffer[line_index++] = *str++;
        }
        if (*str == 0) {
            *has_more = 0;
            break;
        }
    }
    return str;
}

void text_draw_multiline(const char *str, int32_t x, int32_t y, int32_t box_width, int32_t font, int32_t color)
{
    int32_t has_more = 1;
    for (int32_t lines = 1; lines < TEXT_MULTILINE_MAX_LINES; lines++) {
        memset(tmp_line_buffer, 0, TEXT_LINE_BUFFER_BYTES);
        if (has_more) {
            str = fill_line(str, box_width, font, &has_more);
        }
        g_hash_mode = TEXT_HASH_SUBSTITUTE;
        g_text_width = 0;
        g_multiline_flag = 1;
        text_draw((const uint8_t *) tmp_line_buffer, x, y, font, color);
        if (g_max_line_width < g_text_width) {
            g_max_line_width = g_text_width;
        }
        y += TEXT_MULTILINE_LINE_GAP + g_line_height;
        g_draw_y_accum += TEXT_MULTILINE_LINE_GAP + g_line_height;
        if (!has_more) {
            return;
        }
    }
}

void lang_text_draw_multiline(int32_t group, int32_t index, int32_t x, int32_t y, int32_t box_width, int32_t font, int32_t color)
{
    text_draw_multiline(lang_get_string(group, index), x, y, box_width, font, color);
}

static int32_t cursor_blink_visible(void)
{
    int32_t elapsed = time_current_millis - text_cursor_blink_time;
    if (text_cursor_blink_on == 0) {
        if (elapsed < TEXT_CURSOR_OFF_MILLIS) {
            return 0;
        }
        text_cursor_blink_time = time_current_millis;
        text_cursor_blink_on = 1;
    } else if (text_cursor_blink_on == 1 && elapsed >= TEXT_CURSOR_ON_MILLIS) {
        text_cursor_blink_time = time_current_millis;
        text_cursor_blink_on = 0;
        return 0;
    }
    return 1;
}

void graphics_draw_text_cursor(int32_t x, int32_t y, int32_t color)
{
    if (!cursor_blink_visible()) {
        return;
    }
    int32_t cursor_x = g_cursor_x + x;
    int32_t cursor_y = text_cursor_y + y;
    if (capture_insert_mode == 0) {
        graphics_fill_rect(cursor_x, cursor_y + TEXT_CURSOR_BAR_Y, g_cursor_width, TEXT_CURSOR_BAR_HEIGHT, color);
        return;
    }
    int32_t top = cursor_y + TEXT_CURSOR_BEAM_TOP;
    int32_t bottom = cursor_y + TEXT_CURSOR_BAR_Y;
    graphics_draw_line(cursor_x + TEXT_CURSOR_BEAM_LEFT, top, cursor_x + TEXT_CURSOR_BEAM_RIGHT, top, color);
    graphics_draw_line(cursor_x + TEXT_CURSOR_BEAM_STEM_X, top, cursor_x + TEXT_CURSOR_BEAM_STEM_X, cursor_y + TEXT_CURSOR_BEAM_STEM_BOTTOM, color);
    graphics_draw_line(cursor_x + TEXT_CURSOR_BEAM_LEFT, bottom, cursor_x + TEXT_CURSOR_BEAM_RIGHT, bottom, color);
}
