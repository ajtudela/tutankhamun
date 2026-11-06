// The functions of empire_window.c that other files call.

#ifndef WINDOW_EMPIRE_WINDOW_H
#define WINDOW_EMPIRE_WINDOW_H

#include <stdint.h>

enum empire_window_image_group {
    IMAGE_GROUP_EMPIRE_TRADE_ARROWS = 440,
    IMAGE_GROUP_EMPIRE_BORDER = 441,
    IMAGE_GROUP_EMPIRE_MARKER_LAND = 447,
    IMAGE_GROUP_EMPIRE_MARKER_SEA = 448,
    IMAGE_GROUP_EMPIRE_CITY_FLAG = 491,
};

void empire_map_object_at_mouse(int32_t *object, int32_t *moving);

void button_empire_open(void);
void button_empire_exit(void);
void window_empire_draw_border(void);
void window_go_empire(void);
void empire_button_open_trade_route(void);
void empire_trade_route_open_answer(void);
void window_empire_draw_event_tooltip(void);
void window_empire_draw_info_panel(void);
void window_empire_draw_city_trade(void);
void window_empire_draw_background(void);
void window_empire_draw_map(void);
void window_empire_draw_city_name(void);
void button_empire_help(void);
int32_t empire_resource_sold_and_bought(int32_t object, int32_t resource);
int32_t empire_animation_next_frame(int32_t image_id, int32_t frame);

#endif
