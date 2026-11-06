// The functions of ratings.c that other files call.

#ifndef WINDOW_ADVISOR_RATINGS_H
#define WINDOW_ADVISOR_RATINGS_H

#include <stdint.h>

void window_advisor_ratings_draw_button(int32_t index);
void window_advisor_ratings_draw_background(void);
void window_advisor_ratings_draw_foreground(void);
void button_select_rating(void);
void window_advisor_ratings_draw_column(int32_t x, int32_t y, int32_t value, int32_t has_reached);

#endif
