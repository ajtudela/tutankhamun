// Drawing an image of the image collection and the clip helpers: how an image meets the clip rectangle
// (inside, cut on a side, or not visible).

#ifndef GRAPHICS_IMAGE_DRAW_H
#define GRAPHICS_IMAGE_DRAW_H

#include <stdint.h>

enum image_clip {
    IMAGE_CLIP_NONE = 0,
    IMAGE_CLIP_LEFT = 1,
    IMAGE_CLIP_RIGHT = 2,
    IMAGE_CLIP_TOP = 3,
    IMAGE_CLIP_BOTTOM = 4,
    IMAGE_CLIP_INVISIBLE = 5,
};

int32_t image_draw(int32_t image_id, int32_t x, int32_t y);
int32_t image_draw_mirrored(int32_t image_id, int32_t x, int32_t y);
int32_t image_draw_isometric_top(int32_t image_id, int32_t x, int32_t y);
void graphics_clip_horizontal(void);
void graphics_clip_vertical(void);

#endif
