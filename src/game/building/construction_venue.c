// The entertainment venues placed over road crossings (booth, bandstand, pavilion, festival square): the site
// check, the crossing test, the venue patterns and the places of the parts off the road; and the removal of a
// venue's tiles (rubble on non-road tiles when a wall part was removed).

#include "building/construction_venue.h"
#include "building/building_create.h"
#include "building/building_image_group.h"
#include "building/building_type.h"
#include "building/construction_load.h"
#include "building/construction_place.h"
#include "city/city_player.h"
#include "game/undo_build.h"
#include "graphics/image_collection.h"
#include "map/map_data.h"
#include "map/routing_terrain.h"
#include "map/terrain.h"
#include "map/tiles_internal.h"
#include "map/tiles_road.h"
#include <stdbool.h>
#include <string.h>

static int32_t venue_size(int32_t type)
{
    switch (type) {
        case BUILDING_BOOTH:
            return 2;
        case BUILDING_BANDSTAND:
            return 3;
        case BUILDING_PAVILION:
            return 4;
        case BUILDING_FESTIVAL_SQUARE:
            return 5;
        default:
            return 5;
    }
}

bool construction_venue_pattern(int32_t x, int32_t y, int32_t size)
{
    switch (size) {
        case 2:
            return venue_patterns2[x * 2 + y];
        case 3:
            return venue_patterns3[x * 3 + y];
        case 4:
            return venue_patterns4[x * 4 + y];
        case 5:
            return venue_patterns5[x * 5 + y];
        default:
            return 1;
    }
}

bool construction_venue_is_crossing(int32_t x, int32_t y, int32_t x0, int32_t y0, int32_t size)
{
    int32_t offset = map_info.grid_start + y * GRID_SIZE + x;
    if (!(map_grids.terrain[offset] & TERRAIN_ROAD)) {
        return 0;
    }
    int32_t dist[4];
    dist[0] = y - y0;
    dist[1] = x0 - x + size - 1;
    dist[2] = y0 - y + size - 1;
    dist[3] = x - x0;
    int32_t roads = 0, roads_inside = 0, last_open = 0;
    for (int32_t i = 0; i < 4; i++) {
        int32_t d = venue_direction_offsets[i];
        uint32_t road = map_grids.terrain[offset + d] & TERRAIN_ROAD;
        int32_t n = dist[i];
        if (n >= 2) {
            const uint32_t *p = &map_grids.terrain[offset + d * 2];
            for (int32_t k = n - 1; k != 0; k--) {
                road &= *p & TERRAIN_ROAD;
                p += d;
            }
        }
        if (road) {
            roads++;
            if (n != 0) {
                roads_inside++;
            }
        } else {
            last_open = i;
        }
    }
    if (size == 2 && roads >= 3 && roads_inside >= size && (dist[last_open] == 0 || roads == 4)) {
        return 1;
    }
    if (roads >= 4 && roads_inside >= 3) {
        return 1;
    }
    if (size != 5 && roads == 3 && roads_inside == roads && dist[last_open] == 0) {
        return 1;
    }
    return 0;
}

int32_t construction_venue_can_place(int32_t type)
{
    int32_t x0 = map_info.hover_x;
    int32_t y0 = map_info.hover_y;
    int32_t crossing = 0, clear = 1, free = 1;
    int32_t size = venue_size(type);
    int32_t row = 0;
    for (int32_t y = y0; y < size + y0; y++, row++) {
        int32_t col = 0;
        int32_t base = x0 + y * GRID_SIZE;
        for (int32_t x = x0; x < size + x0; x++, col++, base++) {
            if (construction_venue_pattern(col, row, size) && construction_venue_is_crossing(x, y, x0, y0, size)) {
                crossing = 1;
            }
            int32_t offset = map_info.grid_start + base;
            const flood_tile *node = floods_data.tiles[offset];
            if (node != 0 && node->is_floodplain == 0) {
                clear = 0;
            }
            uint32_t terrain = map_grids.terrain[offset];
            if ((terrain & TERRAIN_NOT_EMPTY) && !(terrain & TERRAIN_ROAD)) {
                clear = 0;
            }
            if (map_grids.building_ids[offset] > 0) {
                free = 0;
            }
        }
    }
    return crossing && clear && free;
}

void construction_venue_fill_road_grid(int32_t x, int32_t y, int32_t type)
{
    int32_t size;
    if (type == BUILDING_BOOTH) {
        size = 2;
    } else if (type == BUILDING_BANDSTAND) {
        size = 3;
    } else if (type == BUILDING_PAVILION) {
        size = 4;
    } else {
        size = (type != BUILDING_FESTIVAL_SQUARE) ? 1 : 5;
    }
    for (int32_t i = 0; i < 25; i++) {
        venue_road_grid[i] = 0;
    }
    for (int32_t dy = 0; dy < size; dy++) {
        int32_t row = map_info.grid_start + (y + dy) * GRID_SIZE;
        for (int32_t dx = 0; dx < size; dx++) {
            venue_road_grid[dx * 5 + dy] = (map_grids.terrain[row + x + dx] & TERRAIN_ROAD) != 0;
        }
    }
}

bool construction_venue_free_spots(int32_t size, int32_t w, int32_t h, int32_t *count, int32_t *spots)
{
    if (spots == 0) {
        return 0;
    }
    int32_t n = 0;
    int32_t rows = (h > 1) ? size - h + 1 : size;
    int32_t cols = (w > 1) ? size - w + 1 : size;
    for (int32_t y = 0; y < rows; y++) {
        for (int32_t x = 0; x < cols; x++) {
            int32_t ok = 1;
            for (int32_t j = 0; j < h && ok; j++) {
                for (int32_t k = 0; k < w; k++) {
                    if (venue_road_grid[(x + k) * 5 + y + j] != 0) {
                        ok = 0;
                        break;
                    }
                }
            }
            if (ok) {
                spots[n * 2] = x;
                spots[n * 2 + 1] = y;
                n++;
            }
        }
    }
    *count = n;
    return 1;
}

int32_t construction_venue_take_spot(int32_t x, int32_t y, int32_t *out_x, int32_t *out_y, int32_t venue_type, int32_t part_type)
{
    int32_t size = 0;
    if (venue_type == BUILDING_BOOTH) {
        size = 2;
    } else if (venue_type == BUILDING_BANDSTAND) {
        size = 3;
    } else if (venue_type == BUILDING_PAVILION) {
        size = 4;
    } else if (venue_type == BUILDING_FESTIVAL_SQUARE) {
        size = 5;
    }
    int32_t w = 1, h = 1, across = 0;
    int32_t spots[32];
    if (part_type == BUILDING_BANDSTAND) {
        h = 2;
        spots[0] = 1;
        spots[1] = 1;
        across = 0;
    } else if (part_type == BUILDING_BOOTH) {
        h = 1;
        spots[0] = 0;
        spots[1] = 0;
    } else if (part_type == BUILDING_PAVILION) {
        w = 2;
        h = 2;
        spots[0] = 2;
        spots[1] = 2;
    } else {
        spots[0] = 0;
        spots[1] = 0;
    }
    int32_t best = 5;
    int32_t count;
    construction_venue_free_spots(size, w, h, &count, spots);
    if (count <= 0) {
        if (part_type == BUILDING_BANDSTAND) {
            across = 1;
            construction_venue_free_spots(size, h, w, &count, spots);
        }
        if (count <= 0) {
            *out_x = spots[0] + x;
            *out_y = spots[1] + y;
            return -1;
        }
    }
    int32_t chosen = 0;
    for (int32_t i = 0; i < count; i++) {
        int32_t rank;
        if (size == 2) {
            break;
        } else if (size == 3) {
            rank = venue_spot_rank3[spots[i * 2] * 3 + spots[i * 2 + 1]];
        } else if (size == 4) {
            rank = venue_spot_rank4[spots[i * 2] * 4 + spots[i * 2 + 1]];
        } else {
            rank = 0;
        }
        if (rank < best) {
            best = rank;
            chosen = i;
        }
    }
    int32_t dx = spots[chosen * 2];
    int32_t dy = spots[chosen * 2 + 1];
    *out_x = x + dx;
    *out_y = y + dy;
    int32_t index = dx * 5 + dy;
    if (part_type == BUILDING_BANDSTAND) {
        venue_road_grid[index] = 1;
        if (across) {
            venue_road_grid[index + 5] = 1;
        } else {
            venue_road_grid[index + 1] = 1;
        }
        return across;
    }
    if (part_type == BUILDING_PAVILION) {
        venue_road_grid[index] = 1;
        venue_road_grid[index + 5] = 1;
        venue_road_grid[index + 1] = 1;
        venue_road_grid[index + 6] = 1;
        return across;
    }
    venue_road_grid[index] = 1;
    return across;
}

static void restore_map(void)
{
    memcpy(map_grids.terrain, map_terrain_backup, GRID_CELLS * 4);
    memcpy(map_grids.canals, map_canals_backup, GRID_CELLS);
    memcpy(map_grids.bitfields, map_bitfields_backup, GRID_CELLS);
    memcpy(map_grids.edge, map_edge_backup, GRID_CELLS);
    map_images_restore_free_tiles();
}

static void link_parts(int32_t prev, int32_t next)
{
    building_get(prev)->next_part_building_id = (int16_t) next;
    building_get(next)->prev_part_building_id = (int16_t) prev;
}

static int32_t place_booth(int32_t measure_only, int32_t x_end, int32_t y_end)
{
    int32_t x = (x_end >= map_info.grid_width) ? map_info.grid_width - 1 : x_end;
    int32_t y = (y_end >= map_info.grid_height) ? map_info.grid_height - 1 : y_end;
    int32_t offset = map_info.grid_start + y * GRID_SIZE + x;
    if (construction_venue_can_place(BUILDING_BOOTH) == 0) {
        return 0;
    }
    construction_venue_fill_road_grid(x, y, BUILDING_BOOTH);
    int32_t bx, by;
    if (construction_venue_take_spot(x, y, &bx, &by, BUILDING_BOOTH, BUILDING_BOOTH) == -1) {
        return 0;
    }
    if (measure_only != 0) {
        return 1;
    }
    restore_map();
    routed_building_items_placed = 1;
    int32_t id = building_create(city_current_player, BUILDING_BOOTH, bx, by, 0, 0);
    building *booth = building_get(id);
    booth->venue_has_plaza = 1;
    booth->venue_plaza_grid_offset = offset;
    map_building_tiles_add(id, bx, by, 1, 1, image_group(IMAGE_GROUP_BOOTH), TERRAIN_BUILDING);
    game_undo_add_building(id);
    map_add_venue_plaza_tiles(offset, BUILDING_BOOTH, id);
    return 1;
}

static int32_t place_bandstand(int32_t measure_only, int32_t x_end, int32_t y_end)
{
    int32_t x = x_end, y = y_end;
    int32_t offset = map_info.grid_start + y * GRID_SIZE + x;
    construction_venue_fill_road_grid(x, y, BUILDING_BANDSTAND);
    if (construction_venue_can_place(BUILDING_BANDSTAND) == 0) {
        return 0;
    }
    int32_t sx, sy, bx, by;
    int32_t across = construction_venue_take_spot(x, y, &sx, &sy, BUILDING_BANDSTAND, BUILDING_BANDSTAND);
    if (across == -1) {
        return 0;
    }
    if (construction_venue_take_spot(x, y, &bx, &by, BUILDING_BANDSTAND, BUILDING_BOOTH) == -1) {
        return 0;
    }
    if (measure_only != 0) {
        return 1;
    }
    restore_map();
    routed_building_items_placed = 1;
    int32_t image_main, image_next;
    venue_orientation_images(across, &image_main, &image_next);
    int32_t stand_id = building_create(city_current_player, BUILDING_BANDSTAND, sx, sy, 0, 0);
    building *stand = building_get(stand_id);
    stand->venue_has_plaza = 1;
    stand->venue_orientation = across;
    stand->venue_plaza_grid_offset = offset;
    map_building_tiles_add(stand_id, sx, sy, 1, 1, image_main, TERRAIN_BUILDING);
    game_undo_add_building(stand_id);
    int32_t nx = (across == 0) ? sx : (sx + 1);
    int32_t ny = (across == 0) ? (sy + 1) : sy;
    int32_t next_id = building_create(city_current_player, BUILDING_BANDSTAND, nx, ny, 0, 0);
    building *next = building_get(next_id);
    map_building_tiles_add(next_id, nx, ny, 1, 1, image_next, TERRAIN_BUILDING);
    next->venue_plaza_grid_offset = offset;
    next->venue_has_plaza = 0;
    game_undo_add_building(next_id);
    int32_t booth_id = building_create(city_current_player, BUILDING_BOOTH, bx, by, 0, 0);
    building *booth = building_get(booth_id);
    booth->venue_has_plaza = 0;
    booth->venue_plaza_grid_offset = offset;
    map_building_tiles_add(booth_id, bx, by, 1, 1, image_group(IMAGE_GROUP_BOOTH), TERRAIN_BUILDING);
    game_undo_add_building(booth_id);
    link_parts(stand_id, next_id);
    link_parts(next_id, booth_id);
    map_add_venue_plaza_tiles(offset, BUILDING_BANDSTAND, stand_id);
    return 1;
}

static int32_t place_pavilion(int32_t measure_only, int32_t x_end, int32_t y_end)
{
    int32_t x = (x_end >= map_info.grid_width) ? map_info.grid_width - 1 : x_end;
    int32_t y = (y_end >= map_info.grid_height) ? map_info.grid_height - 1 : y_end;
    int32_t offset = map_info.grid_start + y * GRID_SIZE + x;
    construction_venue_fill_road_grid(x, y, BUILDING_PAVILION);
    if (construction_venue_can_place(BUILDING_PAVILION) == 0) {
        return 0;
    }
    int32_t px, py, sx, sy, bx, by;
    if (construction_venue_take_spot(x, y, &px, &py, BUILDING_PAVILION, BUILDING_PAVILION) == -1) {
        return 0;
    }
    int32_t across = construction_venue_take_spot(x, y, &sx, &sy, BUILDING_PAVILION, BUILDING_BANDSTAND);
    if (across == -1) {
        return 0;
    }
    if (construction_venue_take_spot(x, y, &bx, &by, BUILDING_PAVILION, BUILDING_BOOTH) == -1) {
        return 0;
    }
    if (measure_only != 0) {
        return 1;
    }
    restore_map();
    routed_building_items_placed = 1;
    int32_t pavilion_id = building_create(city_current_player, BUILDING_PAVILION, px, py, 0, 0);
    building *pavilion = building_get(pavilion_id);
    pavilion->venue_has_plaza = 1;
    pavilion->venue_plaza_grid_offset = offset;
    map_building_tiles_add(pavilion_id, px, py, 2, 2, image_group(IMAGE_GROUP_PAVILION), TERRAIN_BUILDING);
    game_undo_add_building(pavilion_id);
    int32_t booth_id = building_create(city_current_player, BUILDING_BOOTH, bx, by, 0, 0);
    building *booth = building_get(booth_id);
    booth->venue_has_plaza = 0;
    booth->venue_plaza_grid_offset = offset;
    map_building_tiles_add(booth_id, bx, by, 1, 1, image_group(IMAGE_GROUP_BOOTH), TERRAIN_BUILDING);
    game_undo_add_building(booth_id);
    int32_t image_main, image_next;
    venue_orientation_images(across, &image_main, &image_next);
    int32_t stand_id = building_create(city_current_player, BUILDING_BANDSTAND, sx, sy, 0, 0);
    building *stand = building_get(stand_id);
    stand->venue_has_plaza = 0;
    stand->venue_orientation = across;
    stand->venue_plaza_grid_offset = offset;
    map_building_tiles_add(stand_id, sx, sy, 1, 1, image_main, TERRAIN_BUILDING);
    game_undo_add_building(stand_id);
    int32_t nx = (across == 0) ? sx : (sx + 1);
    int32_t ny = (across == 0) ? (sy + 1) : sy;
    int32_t next_id = building_create(city_current_player, BUILDING_BANDSTAND, nx, ny, 0, 0);
    building *next = building_get(next_id);
    map_building_tiles_add(next_id, nx, ny, 1, 1, image_next, TERRAIN_BUILDING);
    next->venue_has_plaza = 0;
    next->venue_plaza_grid_offset = offset;
    stand->venue_orientation = across;
    booth->venue_orientation = across;
    pavilion->venue_orientation = across;
    game_undo_add_building(next_id);
    link_parts(pavilion_id, booth_id);
    link_parts(booth_id, stand_id);
    link_parts(stand_id, next_id);
    map_add_venue_plaza_tiles(offset, BUILDING_PAVILION, pavilion_id);
    return 1;
}

static int32_t place_festival_square(int32_t measure_only, int32_t x_end, int32_t y_end)
{
    city_player *player = city_player_current();
    if (player->festival_square_x != -1 && player->festival_square_y != -1) {
        return 0;
    }
    if (construction_venue_can_place(BUILDING_FESTIVAL_SQUARE) == 0) {
        return 0;
    }
    if (measure_only != 0) {
        return 1;
    }
    restore_map();
    int32_t x = (x_end >= map_info.grid_width) ? map_info.grid_width - 1 : x_end;
    int32_t y = (y_end >= map_info.grid_height) ? map_info.grid_height - 1 : y_end;
    routed_building_items_placed = 1;
    int32_t offset = map_info.grid_start + y * GRID_SIZE + x;
    player->festival_square_x = x;
    player->festival_square_y = y;
    construction_venue_fill_road_grid(x, y, BUILDING_FESTIVAL_SQUARE);
    int32_t id = building_create(city_current_player, BUILDING_FESTIVAL_SQUARE, x, y, 0, 0);
    game_undo_add_building(id);
    building *square = building_get(id);
    square->venue_plaza_grid_offset = offset;
    square->venue_has_plaza = 1;
    map_add_venue_plaza_tiles(offset, BUILDING_FESTIVAL_SQUARE, id);
    player->festival_square_placed = 1;
    return 1;
}

int32_t construction_place_booth(int32_t measure_only, int32_t x_start, int32_t y_start, int32_t x_end, int32_t y_end)
{
    return place_booth(measure_only, x_end, y_end);
}
int32_t construction_place_bandstand(int32_t measure_only, int32_t x_start, int32_t y_start, int32_t x_end, int32_t y_end)
{
    return place_bandstand(measure_only, x_end, y_end);
}
int32_t construction_place_pavilion(int32_t measure_only, int32_t x_start, int32_t y_start, int32_t x_end, int32_t y_end)
{
    return place_pavilion(measure_only, x_end, y_end);
}
int32_t construction_place_festival_square(int32_t measure_only, int32_t x_start, int32_t y_start, int32_t x_end, int32_t y_end)
{
    return place_festival_square(measure_only, x_end, y_end);
}

void venue_remove_tiles(int32_t building_id, int32_t type)
{
    const building *venue = building_get(building_id);
    if (venue->type == BUILDING_FESTIVAL_SQUARE) {
        city_player_current()->festival_square_placed = 0;
    }
    int32_t size;
    switch (type) {
        case BUILDING_BANDSTAND:
            size = 3;
            break;
        case BUILDING_BOOTH:
            size = 2;
            break;
        case BUILDING_PAVILION:
            size = 4;
            break;
        case BUILDING_FESTIVAL_SQUARE:
            size = 5;
            break;
        default:
            size = 0;
            break;
    }
    int32_t offset = venue->venue_plaza_grid_offset;
    int32_t x = (offset - map_info.grid_start) % GRID_SIZE;
    int32_t y = (offset - map_info.grid_start) / GRID_SIZE;
    if (size > 0) {
        for (int32_t dy = 0; dy < size; dy++) {
            for (int32_t dx = 0; dx < size; dx++) {
                map_grids.sprite[offset] = 0;
                map_grids.canals[offset] = 0;
                map_grids.bitfields[offset] &= 0xE0;
                map_grids.building_ids[offset] = 0;
                map_grids.building_damage[offset] = 0;
                map_grids.edge[offset] = (uint8_t) ((map_grids.edge[offset] & 0xC0) | 0x40);
                uint32_t terrain = map_grids.terrain[offset] & 0x3052EC4;
                map_grids.images[offset] = 0;
                map_grids.terrain[offset] = terrain;
                map_grids.rubble_building_type[offset] = (uint8_t) type;
                if (terrain & TERRAIN_ROAD) {
                    map_tiles_set_road(x + dx, y + dy, 1);
                } else if (venue_remove_leaves_rubble != 0) {
                    map_tiles_set_rubble_image(x + dx, y + dy);
                }
                offset++;
            }
            offset += GRID_SIZE - size;
        }
    }
    int32_t x_end = x + size;
    int32_t y_end = y + size;
    venue_remove_leaves_rubble = 0;
    map_tiles_update_region_empty_land(x, y, x_end, y_end, 0, 0);
    map_tiles_update_region_empty_land(x - 5, y - 5, x_end + 5, y_end + 5, 1, 0);
    map_tiles_update_region_meadow(x, y, x_end, y_end);
    map_tiles_update_region_floodplain(x, y, x_end, y_end);
    map_tiles_update_region_water(x, y, x_end, y_end, 0);
    map_tiles_update_region_marshland(x, y, x_end, y_end);
    map_tiles_update_region_rubble(x, y, x_end, y_end);
    map_tiles_update_all_plazas(x, y, x_end, y_end, 1);
    map_routing_update_land_citizen_rect(x, y, x_end, y_end, 1);
}
