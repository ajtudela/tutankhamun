// Rich text: the message body drawn or measured, its links and the clicked one.

#ifndef GRAPHICS_RICH_TEXT_H
#define GRAPHICS_RICH_TEXT_H

#include <stdint.h>

enum rich_text_limit {
    RICH_TEXT_MAX_LINKS = 50,
};

typedef struct rich_link {
    int32_t x_start;
    int32_t x_end;
    int32_t y_top;
    int32_t y_bottom;
    int32_t message_id;
} rich_link;

void rich_text_clear_links(void);
void rich_text_add_link(int32_t message_id, int32_t x_start, int32_t x_end, int32_t y);
int32_t rich_text_get_clicked_link(void);
int32_t rich_text_get_word_width(const uint8_t *str);
int32_t rich_text_draw_character(int32_t font, uint8_t letter_index, int32_t x, int32_t y, int32_t measure_only, int32_t large_line);
void rich_text_draw_line(const uint8_t *str, int32_t x, int32_t y, int32_t measure_only, int32_t large_line);
void rich_text_draw(const uint8_t *text, int32_t x_offset, int32_t y_offset, int32_t box_width, int32_t measure_only, int32_t large_line);

#endif
