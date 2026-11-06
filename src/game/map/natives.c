// The natives on the map: their buildings placed from the map's terrain at a mission start, with their land marked
// and each hut tied to its nearest meeting point; the crops' image follows their production progress, and the natives
// walk to the first road or free tile in the ring around a building.
//
// Left as the mission start does it:
// - the land marking leaves the scratch offset and map_result_x / y where its loop ends;
// - a hut keeps its image variant in figure_spawn_delay.

#include "map/natives.h"

#include "building/building.h"
#include "building/building_create.h"
#include "building/building_image_group.h"
#include "building/building_state.h"
#include "building/building_type.h"
#include "city/city_init.h"
#include "city/city_player.h"
#include "core/calc.h"
#include "figure/figure_data.h"
#include "graphics/image_collection.h"
#include "map/building_tiles.h"
#include "map/construction_terrain.h"
#include "map/grid.h"
#include "map/map_data.h"
#include "map/terrain.h"
#include <stdint.h>

enum native_rule {
    NATIVE_LAND_EDGE_BIT = 0x80,
    NATIVE_ANGER_START = 100,
    NATIVE_HUT_SIZE = 1,
    NATIVE_HUT_LAND_RADIUS = 3,
    NATIVE_HUT_EDITOR_RADIUS = 5,
    NATIVE_MEETING_SIZE = 2,
    NATIVE_MEETING_LAND_RADIUS = 6,
    NATIVE_MEETING_EDITOR_RADIUS = 8,
    NATIVE_MEETINGS_LIST_SIZE = 500,
    NATIVE_MEETING_DISTANCE_NONE = 1000,
};

void map_native_crops_update(int32_t building_id)
{
    building *b = building_get(building_id);
    int16_t *frame = &b->data.industry.progress;
    if (++*frame >= NATIVE_CROPS_FRAMES) {
        *frame = 0;
    }
    map_grids.images[b->grid_offset] = *frame + image_group(IMAGE_GROUP_GRAIN_CROPS);
}

int32_t map_native_target_tile(int32_t x, int32_t y, int32_t size)
{
    const int32_t *offsets = building_tile_offsets.rings + (size * BUILDING_RING_OFFSETS_PER_SIZE);
    for (int32_t i = 0; i < BUILDING_RING_OFFSETS_PER_SIZE; i++) {
        if (offsets[i] == 0) {
            return 0;
        }
        uint32_t terrain = map_grids.terrain[offsets[i] + map_grid_offset(x, y)];
        if ((terrain & TERRAIN_ROAD) || (terrain & TERRAIN_NOT_CLEAR) == 0) {
            const int32_t *xy = building_ring_xy + ((i + (size * BUILDING_RING_OFFSETS_PER_SIZE)) * 2);
            map_result_x = xy[0] + x;
            map_result_y = xy[1] + y;
            return 1;
        }
    }
    return 0;
}

static void native_land_mark(int32_t x, int32_t y, int32_t size, int32_t radius, int32_t set)
{
    int32_t x_min = x - radius;
    int32_t x_max = x + size + radius - 1;
    int32_t y_min = y - radius;
    int32_t y_max = y + size + radius - 1;
    if (x_min < 0) {
        x_min = 0;
    }
    if (y_min < 0) {
        y_min = 0;
    }
    if (x_max >= map_info.grid_width) {
        x_max = map_info.grid_width - 1;
    }
    if (y_max >= map_info.grid_height) {
        y_max = map_info.grid_height - 1;
    }
    map_result_y = y_min;
    int32_t offset = map_info.grid_start + y_min * GRID_SIZE + x_min;
    int32_t row_step = x_min - x_max + GRID_SIZE - 1;
    map_tile_cursor_offset = offset;
    if (y_min > y_max) {
        return;
    }
    int32_t rows = y_max - y_min + 1;
    map_result_y = y_min + rows;
    do {
        map_result_x = x_min;
        if (x_min <= x_max) {
            int32_t n = x_max - x_min + 1;
            map_result_x = n + x_min;
            do {
                if (set) {
                    map_grids.edge[offset] |= NATIVE_LAND_EDGE_BIT;
                } else {
                    map_grids.edge[offset] &= (uint8_t) ~NATIVE_LAND_EDGE_BIT;
                }
                offset++;
            } while (--n);
        }
        offset += row_step;
    } while (--rows);
    map_tile_cursor_offset = offset;
}

void map_native_land_mark(int32_t x, int32_t y, int32_t size, int32_t radius)
{
    native_land_mark(x, y, size, radius, 1);
}

void map_native_land_unmark(int32_t x, int32_t y, int32_t size, int32_t radius)
{
    native_land_mark(x, y, size, radius, 0);
}

void map_natives_init(void)
{
    city_player *player = city_player_current();
    int32_t grid_offset = map_info.grid_start;
    for (int32_t y = 0; y < map_info.grid_height; y++, grid_offset += map_info.grid_border_size) {
        for (int32_t x = 0; x < map_info.grid_width; x++, grid_offset++) {
            if (!(map_grids.terrain[grid_offset] & TERRAIN_BUILDING) || map_grids.building_ids[grid_offset] != 0) {
                continue;
            }
            int32_t variant = map_grids.random[grid_offset] & 1;
            if (editor_active == 1) {
                variant = 0;
            }
            int32_t *image = &map_grids.images[grid_offset];
            int32_t type;
            if (*image == scenario_data.native_image_hut) {
                type = BUILDING_NATIVE_HUT;
                *image = image_group(IMAGE_GROUP_NATIVE_HUT);
            } else if (*image == scenario_data.native_image_hut + 1) {
                type = BUILDING_NATIVE_HUT;
                *image = image_group(IMAGE_GROUP_NATIVE_HUT) + 1;
            } else if (*image == scenario_data.native_image_meeting) {
                type = BUILDING_NATIVE_MEETING;
                *image = image_group(IMAGE_GROUP_NATIVE_HUT) + 2;
            } else if (*image == scenario_data.native_image_crops) {
                type = BUILDING_NATIVE_CROPS;
                *image = image_group(IMAGE_GROUP_GRAIN_CROPS) + variant;
            } else {
                map_building_tiles_remove(0, x, y);
                continue;
            }
            int32_t id = building_create(CITY_PLAYER_HUMAN, type, x, y, 0, 0);
            map_grids.building_ids[grid_offset] = (int16_t) id;
            building *b = building_get(id);
            b->state = BUILDING_STATE_IN_USE;
            if (type == BUILDING_NATIVE_CROPS) {
                b->data.native.image_variant = (int16_t) variant;
            } else if (type == BUILDING_NATIVE_MEETING) {
                map_grids.building_ids[grid_offset + 1] = (int16_t) id;
                map_grids.building_ids[grid_offset + GRID_SIZE] = (int16_t) id;
                map_grids.building_ids[grid_offset + GRID_SIZE + 1] = (int16_t) id;
                b->sentiment.native_anger = NATIVE_ANGER_START;
                if (editor_active == 0) {
                    map_native_land_mark(b->x, b->y, NATIVE_MEETING_SIZE, NATIVE_MEETING_LAND_RADIUS);
                } else {
                    map_native_land_unmark(b->x, b->y, NATIVE_MEETING_SIZE, NATIVE_MEETING_EDITOR_RADIUS);
                }
                if (player->main_native_meeting_x == 0) {
                    player->main_native_meeting_x = b->x;
                    player->main_native_meeting_y = b->y;
                }
            } else {
                b->sentiment.native_anger = NATIVE_ANGER_START;
                b->figure_spawn_delay = (uint8_t) variant;
                if (editor_active == 0) {
                    map_native_land_mark(b->x, b->y, NATIVE_HUT_SIZE, NATIVE_HUT_LAND_RADIUS);
                } else {
                    map_native_land_unmark(b->x, b->y, NATIVE_HUT_SIZE, NATIVE_HUT_EDITOR_RADIUS);
                }
            }
        }
    }
    int32_t highest = building_highest_id;
    int32_t count = 0;
    building_list_small_size = 0;
    if (highest < 1) {
        return;
    }
    for (int32_t id = 1; id <= highest; id++) {
        const building *b = building_get(id);
        if (b->state == BUILDING_STATE_IN_USE && b->type == BUILDING_NATIVE_MEETING) {
            building_list_small[count++] = (int16_t) id;
            building_list_small_size = count;
            if (count >= NATIVE_MEETINGS_LIST_SIZE) {
                building_list_small_size = NATIVE_MEETINGS_LIST_SIZE - 1;
                count = NATIVE_MEETINGS_LIST_SIZE - 1;
            }
        }
    }
    if (count <= 0) {
        return;
    }
    for (int32_t id = 1; id <= highest; id++) {
        building *b = building_get(id);
        if (b->state != BUILDING_STATE_IN_USE || b->type != BUILDING_NATIVE_HUT) {
            continue;
        }
        int32_t best = NATIVE_MEETING_DISTANCE_NONE;
        int16_t best_id = 0;
        building_list_small_index = 0;
        int32_t hx = b->x;
        int32_t hy = b->y;
        building_list_small_index = count;
        for (int32_t i = 0; i < count; i++) {
            int32_t meeting = building_list_small[i];
            const building *m = building_get(meeting);
            int32_t mx = m->x;
            int32_t my = m->y;
            int32_t distance = calc_maximum_distance(hx, hy, mx, my);
            if (distance < best) {
                best = distance;
                best_id = (int16_t) meeting;
            }
        }
        b->subtype = best_id;
    }
}
