// A panel of 16-pixel blocks from the panel button images, used by the debug panels. The panel kind's
// animation value picks the look: nothing, an opening frame of the large label, or a frame of blocks growing to
// the full size; one kind is drawn red through the colour mask.

#include "graphics/panel_blocks.h"
#include "graphics/button_border.h"
#include "graphics/graphics_data.h"
#include "graphics/image_blit_mask.h"
#include "graphics/image_collection.h"
#include "graphics/image_draw.h"
#include "widget/widget_data.h"
#include "window/window_data.h"
#include "window/window_image_group.h"
#include <stdint.h>

enum panel_images {
    PANEL_PIECES_PER_STYLE = 9,
    PANEL_FIRST_PIECE = 6,
    PANEL_PIECES_PER_ROW = 3,
    PANEL_RED_MASK = 0xF863,
};

static void draw_opening_frame(int32_t value, int32_t x, int32_t y, int32_t width, int32_t height, int32_t style)
{
    const int32_t *frame = &large_label_frames[(value - LABEL_ANIMATION_OPENING) * LARGE_LABEL_FRAME_FIELDS];
    g_draw_x = (((width - 2) * UI_BLOCK_SIZE) / 2) + frame[LARGE_LABEL_FRAME_X] + x;
    g_draw_y = (((height - 2) * UI_BLOCK_SIZE) / 2) + frame[LARGE_LABEL_FRAME_Y] + y;
    int32_t image = frame[LARGE_LABEL_FRAME_IMAGE] + (style * LABEL_OPENING_IMAGES_PER_STYLE) + LABEL_OPENING_IMAGES + image_group(IMAGE_GROUP_PANEL_BUTTON);
    image_draw(image, g_draw_x, g_draw_y);
}

void panel_draw_blocks(int32_t kind, int32_t x, int32_t y, int32_t width, int32_t height, int32_t style)
{
    int32_t value = label_animation[kind];
    if (value < LABEL_ANIMATION_OPENING) {
        return;
    }
    if (value < LABEL_ANIMATION_GROWING) {
        draw_opening_frame(value, x, y, width, height, style);
        return;
    }
    int32_t blocks = value - LABEL_ANIMATION_BLOCKS_OFFSET;
    int32_t columns = blocks;
    if (blocks > width) {
        columns = width;
    } else {
        x += ((width - blocks) * UI_BLOCK_SIZE) / 2;
    }
    int32_t rows = blocks;
    if (blocks > height) {
        rows = height;
    } else {
        y += ((height - blocks) * UI_BLOCK_SIZE) / 2;
    }
    int32_t first_piece = image_group(IMAGE_GROUP_PANEL_BUTTON) + (style * PANEL_PIECES_PER_STYLE) + PANEL_FIRST_PIECE;
    for (int32_t row = 0; row < rows; row++) {
        int32_t row_y = (row * UI_BLOCK_SIZE) + y;
        int32_t row_piece = first_piece + (piece_part(row, rows) * PANEL_PIECES_PER_ROW);
        for (int32_t column = 0; column < columns; column++) {
            g_draw_x = (column * UI_BLOCK_SIZE) + x;
            g_draw_y = row_y;
            g_last_image_id = row_piece + piece_part(column, columns);
            if (kind == PANEL_BLOCKS_RED) {
                image_draw_masked(g_last_image_id, g_draw_x, g_draw_y, PANEL_RED_MASK);
            } else {
                image_draw(g_last_image_id, g_draw_x, g_draw_y);
            }
        }
    }
}
