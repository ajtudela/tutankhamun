// Panels of 16-pixel blocks drawn from the panel images: the outer (dialog) panel, the inner panel, the panel
// without border and the inner panel's horizontal bar. A style's images are a square grid whose first and last
// rows and columns are the border and whose middle pieces repeat along the panel.

#include "graphics/panel_draw.h"
#include "graphics/button_border.h"
#include "graphics/graphics_data.h"
#include "graphics/image_collection.h"
#include "graphics/image_draw.h"
#include "widget/widget_data.h"
#include "window/window_image_group.h"
#include <stdint.h>

enum panel_grid {
    OUTER_PANEL_GRID = 12,
    INNER_PANEL_GRID = 7,
};

static int32_t panel_piece(int32_t position, int32_t blocks, int32_t grid)
{
    if (position == 0) {
        return 0;
    }
    if (position < blocks - 1) {
        return 1 + ((position - 1) % (grid - 2));
    }
    return grid - 1;
}

static int32_t inner_piece(int32_t position, int32_t grid)
{
    return 1 + (position % (grid - 2));
}

static void draw_block(int32_t image, int32_t x, int32_t y)
{
    g_draw_x = x;
    g_draw_y = y;
    g_last_image_id = image;
    image_draw(image, x, y);
}

static void draw_bordered_panel(int32_t first_image, int32_t grid, int32_t x, int32_t y, int32_t width_blocks, int32_t height_blocks)
{
    for (int32_t row = 0; row < height_blocks; row++) {
        int32_t row_image = first_image + (panel_piece(row, height_blocks, grid) * grid);
        for (int32_t column = 0; column < width_blocks; column++) {
            draw_block(row_image + panel_piece(column, width_blocks, grid), (column * UI_BLOCK_SIZE) + x, (row * UI_BLOCK_SIZE) + y);
        }
    }
}

void outer_panel_draw(int32_t x, int32_t y, int32_t width_blocks, int32_t height_blocks)
{
    draw_bordered_panel(image_group(IMAGE_GROUP_OUTER_PANEL), OUTER_PANEL_GRID, x, y, width_blocks, height_blocks);
}

void inner_panel_draw(int32_t x, int32_t y, int32_t width_blocks, int32_t height_blocks)
{
    draw_bordered_panel(image_group(IMAGE_GROUP_INNER_PANEL), INNER_PANEL_GRID, x, y, width_blocks, height_blocks);
}

void panel_draw_bar(int32_t x, int32_t y, int32_t blocks)
{
    int32_t row_image = image_group(IMAGE_GROUP_INNER_PANEL) + ((INNER_PANEL_GRID - 1) * INNER_PANEL_GRID);
    for (int32_t i = 0; i < blocks; i++) {
        draw_block(row_image + panel_piece(i, blocks, INNER_PANEL_GRID), (i * UI_BLOCK_SIZE) + x, y);
    }
}

void panel_draw_unbordered(int32_t x, int32_t y, int32_t width_blocks, int32_t height_blocks)
{
    for (int32_t row = 0; row < height_blocks; row++) {
        int32_t row_image = image_group(IMAGE_GROUP_OUTER_PANEL) + (inner_piece(row, OUTER_PANEL_GRID) * OUTER_PANEL_GRID);
        for (int32_t column = 0; column < width_blocks; column++) {
            draw_block(row_image + inner_piece(column, OUTER_PANEL_GRID), (column * UI_BLOCK_SIZE) + x, (row * UI_BLOCK_SIZE) + y);
        }
    }
}
