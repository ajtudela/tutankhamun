// The list of resolutions of the display options window (display_resolution_list.c): the supported sizes, the
// display's modes among them, the request of a custom size and the window's rows, draw and buttons.

#ifndef WINDOW_DISPLAY_RESOLUTION_LIST_H
#define WINDOW_DISPLAY_RESOLUTION_LIST_H

#include "graphics/button.h"
#include <stdint.h>

enum display_resolution_list_limits {
    DISPLAY_RESOLUTION_LIST_MAX = 64,
    DISPLAY_RESOLUTION_TEXT_SIZE = 16,
};

typedef struct screen_size {
    int32_t width;
    int32_t height;
} screen_size;

void display_resolution_list_allow(int32_t allowed);
int32_t display_resolution_list_allowed(void);
void display_resolution_set_limit(int32_t width, int32_t height);
int32_t display_resolution_supported(int32_t width, int32_t height);
int32_t display_resolution_layout_mode(int32_t width, int32_t height);
void display_resolution_start_custom(screen_size size);
void display_resolution_request(screen_size size);
void display_resolution_take_request(void);
void display_resolution_apply_custom(void);
int32_t display_resolution_custom_active(void);
void display_resolution_custom_screen(int32_t *width, int32_t *height);
int32_t display_resolution_mode(int32_t width, int32_t height);
screen_size display_resolution_of_mode(int32_t mode);
screen_size display_resolution_nearest(int32_t width, int32_t height);
void display_resolution_list_set_display_modes(const screen_size *modes, int32_t count);
int32_t display_resolution_list_count(void);
screen_size display_resolution_list_entry(int32_t index);
generic_button *display_resolution_list_buttons(void);
int32_t display_resolution_list_button_count(void);
void display_resolution_list_draw(void);

#endif
