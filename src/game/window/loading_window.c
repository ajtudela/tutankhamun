// The loading screen of the start-up: the background image while the game's graphics files load, one stage drawn after
// each group of files, and as the window's background.

#include "window/loading_window.h"
#include "graphics/image_collection.h"
#include "graphics/image_draw.h"
#include "graphics/label_animation_slots.h"
#include "window/intermezzo.h"
#include "window/window_image_group.h"
#include <stdint.h>

void window_loading_draw_background(void)
{
    image_draw(image_group(IMAGE_GROUP_LOADING), ui_origin_x, ui_origin_y);
    label_animation[LABEL_SLOT_REFRESH_COUNTDOWN] = 100;
}

void window_loading_draw_stage(int32_t stage)
{
    image_draw(image_group(IMAGE_GROUP_LOADING_STAGE) + stage, ui_origin_x, ui_origin_y);
    label_animation[LABEL_SLOT_REFRESH_COUNTDOWN] = 100;
}
