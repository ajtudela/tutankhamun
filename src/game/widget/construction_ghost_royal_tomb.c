// The royal tombs' placement ghost: the monument base class's ghost with one more rule, the entrance part is
// green only where its tile and one tile beside it are free. Tiles within 3 of the map edge are not drawn.

#include "widget/construction_ghost_royal_tomb.h"
#include "building/building_data.h"
#include "building/monument_object.h"
#include "building/pyramid_images.h"
#include "building/royal_tomb.h"
#include "graphics/image_collection.h"
#include "map/building_tiles.h"
#include "map/grid.h"
#include "map/map_data.h"
#include "map/terrain.h"
#include "map/terrain_image_group.h"
#include "widget/city_view_rotate.h"
#include "widget/construction_ghost.h"
#include "widget/widget_data.h"
#include <stdint.h>

enum tomb_ghost {
    TOMB_MAP_EDGE = 3,
    TOMB_ENTRANCE_NEIGHBOURS = 4,
    TOMB_FOOTPRINT_SIDE = 6,
};

static int32_t entrance_connected(tomb_object *self, int32_t part, int32_t tile)
{
    static const int32_t neighbours[TOMB_ENTRANCE_NEIGHBOURS] = { -GRID_SIZE, GRID_SIZE, -1, 1 };
    int32_t connected = 0;
    for (int32_t n = 0; n < TOMB_ENTRANCE_NEIGHBOURS; n++) {
        if (self->vtable->tile_is_free(self, map_grids.terrain[tile + neighbours[n]], part)) {
            connected = 1;
        }
    }
    return connected;
}

void royal_tomb_draw_placement(tomb_object *self, const monument_part_desc *descs, int32_t count)
{
    int32_t view = map_info.orientation / VIEW_ORIENTATION_STEP;
    int32_t step_x = city_view.tile_half_width + 1;
    int32_t step_y = city_view.tile_half_height;
    int32_t pixel_x, pixel_y;
    city_view_tile_to_pixel(map_info.hover_x, map_info.hover_y, &pixel_x, &pixel_y);
    int32_t offset = map_info.hover_offset - map_info.grid_start;
    int32_t gx = offset % GRID_SIZE;
    int32_t gy = offset / GRID_SIZE;
    for (int32_t part = 0; part < count; part++) {
        const monument_part_desc *desc = &descs[part];
        int32_t is_entrance = part == self->last_part;
        int32_t connected = 0;
        if (is_entrance) {
            connected = entrance_connected(self, part, ((desc->dy + gy) * GRID_SIZE) + desc->dx + map_info.grid_start + gx);
        }
        int32_t x = desc->dx + gx;
        int32_t y = desc->dy + gy;
        map_tile_cursor_offset = map_info.grid_start + (y * GRID_SIZE) + x;
        int32_t size = desc->size;
        for (int32_t row = 0; row < desc->size; row++) {
            for (int32_t col = 0; col < desc->size; col++) {
                int32_t sx = desc->view_x[view] + pixel_x;
                int32_t sy = desc->view_y[view] + step_y + pixel_y;
                if (map_grid_is_inside_size(map_info.diamond_x_min + col + x, map_info.diamond_y_min + row + y, TOMB_MAP_EDGE)) {
                    continue;
                }
                uint32_t terrain = map_grids.terrain[building_square_tile_offset(row, col) + map_tile_cursor_offset];
                int32_t color = GHOST_COLOR_BLOCKED;
                if (self->vtable->tile_is_free(self, terrain, part) && (!is_entrance || connected)) {
                    color = GHOST_COLOR_FREE;
                }
                int32_t index = 0;
                switch (map_info.orientation) {
                    case VIEW_ORIENTATION_NORTH:
                        index = ghost_tomb_footprint_tiles[(col * TOMB_FOOTPRINT_SIDE) + row];
                        break;
                    case VIEW_ORIENTATION_EAST:
                        index = ghost_tomb_footprint_tiles[(row * TOMB_FOOTPRINT_SIDE) + size - col - 1];
                        sx += (size - 1) * step_x;
                        sy -= (size - 1) * step_y;
                        break;
                    case VIEW_ORIENTATION_SOUTH:
                        index = ghost_tomb_footprint_tiles[(size * (TOMB_FOOTPRINT_SIDE + 1)) - (col * TOMB_FOOTPRINT_SIDE) - row - (TOMB_FOOTPRINT_SIDE + 1)];
                        sy -= 2 * (size - 1) * step_y;
                        break;
                    case VIEW_ORIENTATION_WEST:
                        index = ghost_tomb_footprint_tiles[((size - row - 1) * TOMB_FOOTPRINT_SIDE) + col];
                        sx -= (size - 1) * step_x;
                        sy -= (size - 1) * step_y;
                        break;
                    default:
                        break;
                }
                ghost_image_draw_masked(image_group(IMAGE_GROUP_FLAT_TILE), ghost_flat_tile_dx(index) + sx, ghost_flat_tile_dy(index) - step_y + sy, color);
            }
        }
    }
}
