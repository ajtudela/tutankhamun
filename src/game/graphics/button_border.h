// Animated labels, button borders and the texts drawn at a dialog's button rectangle; the UI block size and
// the label animation values shared with the panel blocks.

#ifndef GRAPHICS_BUTTON_BORDER_H
#define GRAPHICS_BUTTON_BORDER_H

#include <stdint.h>

enum ui_block {
    UI_BLOCK_SIZE = 16,
};

enum large_label_frame_field {
    LARGE_LABEL_FRAME_IMAGE,
    LARGE_LABEL_FRAME_X,
    LARGE_LABEL_FRAME_Y,
    LARGE_LABEL_FRAME_FIELDS,
};

enum large_label_animation {
    LABEL_ANIMATION_OPENING = 5,
    LABEL_ANIMATION_GROWING = 15,
    LABEL_ANIMATION_BLOCKS_OFFSET = 13,
};

enum large_label_images {
    LABEL_OPENING_IMAGES = 24,
    LABEL_OPENING_IMAGES_PER_STYLE = 8,
};

enum piece_part {
    PIECE_FIRST,
    PIECE_MIDDLE,
    PIECE_LAST,
    PIECE_PARTS,
};

static inline int32_t piece_part(int32_t index, int32_t count)
{
    if (index == 0) {
        return PIECE_FIRST;
    }
    return (index < count - 1) ? PIECE_MIDDLE : PIECE_LAST;
}

int32_t label_animation_click(void);
void large_label_draw_animated(int32_t id, int32_t x, int32_t y, int32_t width_blocks, int32_t type);
void label_draw_animated(int32_t id, int32_t x, int32_t y, int32_t width_blocks, int32_t force, int32_t type);
void button_border_draw(int32_t x, int32_t y, int32_t width, int32_t height, int32_t has_focus);
void button_border_draw_rect(const int16_t *rect, int32_t has_focus, int32_t x, int32_t y);
void lang_text_draw_centered_rect(int32_t group, int32_t index, const int16_t *rect, int32_t font, int32_t color, int32_t x, int32_t y);
void lang_text_draw_rect_ui(
    int32_t group, int32_t index, const int16_t *rect, int32_t font, int32_t color, int32_t dx, int32_t dy, int32_t offset_x, int32_t offset_y);
void text_draw_number_rect(int32_t value, int32_t prefix, const char *postfix, const int16_t *rect, int32_t font, int32_t color, int32_t dx, int32_t dy);
void text_draw_in_rect(const char *text, const int16_t *rect, int32_t font, int32_t color, int32_t x, int32_t y);
void text_draw_prefixed_centered(const char *prefix, const char *text, int32_t x, int32_t y, int32_t width, int32_t font, int32_t color);
void text_draw_number_prefixed_centered(
    int32_t value, const char *prefix, const char *postfix, int32_t x, int32_t y, int32_t width, int32_t font, int32_t color);
void lang_text_draw_at_rect(int32_t group, int32_t index, const int16_t *rect, int32_t font, int32_t color, int32_t dx, int32_t dy);
void lang_text_draw_year_at_rect(int32_t bc_spacing, int32_t year, const int16_t *rect, int32_t font, int32_t color, int32_t dx, int32_t dy);
void text_draw_number_at_rect(int32_t value, int32_t prefix, const char *postfix, const int16_t *rect, int32_t font, int32_t color, int32_t dx, int32_t dy);

#endif
