// The temple complex: the site check of the build tool, its three parts and decoration tiles added to the map,
// their images by orientation, view and upgrades, the two gatehouse annexes and their drawing, and the city
// bonuses of its god (Ra, Seth, Bast, Osiris).
//
// A decoration tile is checked inside the map with the map origin added, but placed without it.

#include "building/temple_complex.h"
#include "building/building.h"
#include "building/building_figure_helpers.h"
#include "building/building_image_group.h"
#include "building/building_type.h"
#include "building/construction_load.h"
#include "building/construction_place.h"
#include "building/gatehouse_place.h"
#include "building/monument_object.h"
#include "building/monument_spots.h"
#include "building/pyramid_images.h"
#include "city/gods.h"
#include "city/ratings.h"
#include "figure/figure.h"
#include "figure/figure_type.h"
#include "game/game_internal.h"
#include "game/undo_build.h"
#include "graphics/image_collection.h"
#include "graphics/image_draw.h"
#include "map/grid.h"
#include "map/map_data.h"
#include "map/terrain.h"
#include "map/tiles.h"
#include "map/tiles_road.h"
#include "widget/construction_ghost.h"
#include <stdint.h>

int32_t construction_temple_complex_can_place(int32_t x, int32_t y)
{
    int32_t view = map_info.orientation;
    int32_t index = construction_orientation + (view / 2) * 4;
    int32_t x0 = temple_complex_origin[index * 2] + x;
    int32_t y0 = temple_complex_origin[index * 2 + 1] + y;
    for (int32_t r = 0; r < TEMPLE_COMPLEX_SITE_ROWS; r++) {
        for (int32_t k = 0; k < TEMPLE_COMPLEX_SITE_COLUMNS; k++) {
            int32_t rot = construction_orientation * 2;
            int32_t variant;
            if (view == 0) {
                variant = (rot == 0) ? 0 : rot == 2 ? 1 : rot == 4 ? 2 : 3;
            } else if (view == 2) {
                variant = (rot == 0) ? 3 : rot == 2 ? 0 : rot == 4 ? 1 : 2;
            } else if (view == 4) {
                variant = (rot == 0) ? 2 : rot == 2 ? 3 : rot == 4 ? 0 : 1;
            } else {
                variant = (rot == 0) ? 1 : rot == 2 ? 2 : rot == 4 ? 3 : 0;
            }
            int32_t tx, ty;
            switch (variant) {
                case 0:
                    tx = x0 + k;
                    ty = y0 + r;
                    break;
                case 1:
                    tx = x0 - r;
                    ty = y0 - k;
                    break;
                case 2:
                    tx = x0 - k;
                    ty = y0 - r;
                    break;
                default:
                    tx = x0 + r;
                    ty = y0 + k;
                    break;
            }
            int32_t offset = map_info.grid_start + ty * GRID_SIZE + tx;
            uint32_t terrain = map_grids.terrain[offset] & TERRAIN_NOT_EMPTY;
            if (map_grids.figure_ids[offset] != 0) {
                return TEMPLE_COMPLEX_SITE_BLOCKED;
            }
            if (construction_area_has_entry_point(tx, ty, tx, ty)) {
                return TEMPLE_COMPLEX_SITE_ENTRY_POINT;
            }
            if (terrain != 0) {
                return TEMPLE_COMPLEX_SITE_BLOCKED;
            }
        }
    }
    return TEMPLE_COMPLEX_SITE_FITS;
}

static int32_t has_temple_complex_of(const city_player *player, int32_t god)
{
    return player->has_temple_complex != 0 && building_temple_god(player->temple_complex_building_id) == god;
}

void temple_complex_ra_kingdom_bonus(void)
{
    if (has_temple_complex_of(city_player_current(), GOD_RA)) {
        city_ratings_change_kingdom(TEMPLE_COMPLEX_RA_KINGDOM_BONUS);
    }
}

int32_t city_military_training_bonus(int32_t active)
{
    int32_t extra = 0;
    if (has_temple_complex_of(city_player_current(), GOD_SETH)) {
        extra = TEMPLE_COMPLEX_SETH_TRAINING_BONUS;
    }
    return ((active != 0) ? MILITARY_TRAINING_ACADEMY_BONUS : 0) + extra;
}

int32_t city_temple_complex_wage_bonus(void)
{
    const city_player *current = city_player_current();
    if (has_temple_complex_of(current, GOD_RA) && building_temple_complex_has_upgrade(current->temple_complex_building_id, TEMPLE_COMPLEX_ORACLE) != 0) {
        return TEMPLE_COMPLEX_RA_WAGE_BONUS;
    }
    return 0;
}

int32_t city_food_consumption_percentage(int32_t player)
{
    if (has_temple_complex_of(city_player_get(player), GOD_BAST)) {
        return TEMPLE_COMPLEX_FOOD_CONSUMPTION_PERCENTAGE;
    }
    const city_player *current = city_player_current();
    if (has_temple_complex_of(current, GOD_OSIRIS) && building_temple_complex_has_upgrade(current->temple_complex_building_id, TEMPLE_COMPLEX_ALTAR) != 0) {
        return TEMPLE_COMPLEX_FOOD_CONSUMPTION_PERCENTAGE;
    }
    return FOOD_CONSUMPTION_PERCENTAGE;
}

void temple_complex_set_tile(int32_t x, int32_t y, int32_t image_index, int32_t image_offset)
{
    if (map_grid_is_inside(map_info.diamond_x_min + x, map_info.diamond_y_min + y) == 0) {
        return;
    }
    int32_t offset = map_info.grid_start + y * GRID_SIZE + x;
    map_tile_cursor_offset = offset;
    map_grids.images[offset] = image_group(image_index) + image_offset;
    map_grids.edge[offset] |= MAP_EDGE_DRAW_TILE;
    map_grids.canals[offset] = 0;
    map_grids.bitfields[offset] &= 0xF0;
    map_grids.building_ids[offset] = (int16_t) city_player_current()->temple_complex_building_id;
    map_grids.terrain[offset] |= TERRAIN_BUILDING;
}

enum {
    TEMPLE_DECOR_VARIANTS_FIRST = 32,
    TEMPLE_DECOR_VARIANTS_PER_ORIENTATION = 4,
};

static int32_t temple_part_group(int32_t index)
{
    return gatehouse_parts.part_groups[index];
}

static const int32_t *decoration_tile(int32_t record)
{
    return &temple_complex_tiles[record * TEMPLE_DECOR_RECORD_INTS];
}

static int32_t temple_origin_x(int32_t half)
{
    return temple_complex_origin[half * 2];
}

static int32_t temple_origin_y(int32_t half)
{
    return temple_complex_origin[(half * 2) + 1];
}

static int32_t decoration_variant(int32_t half, int32_t view_half)
{
    return temple_complex_origin[TEMPLE_DECOR_VARIANTS_FIRST + half + (view_half * TEMPLE_DECOR_VARIANTS_PER_ORIENTATION)];
}

static int32_t temple_image_flip(int32_t direction)
{
    int32_t orient = map_info.orientation;
    if (orient == 0 || orient == 4) {
        return direction == 2 || direction == 6 ? 3 : 0;
    }
    return direction != 2 && direction != 6 ? 3 : 0;
}

void temple_complex_place_decoration(int32_t building_id)
{
    building *b = building_get(building_id);
    int32_t direction = b->data.temple_complex.orientation;
    int32_t half = direction / 2;
    int32_t adj_x = b->x + temple_origin_x(half);
    int32_t adj_y = b->y + temple_origin_y(half);
    int32_t variant = decoration_variant(half, map_info.orientation / 2);
    int32_t row_x = adj_x;
    int32_t row_y = adj_y;
    int32_t record = 0;
    for (int32_t row = 0; row < TEMPLE_COMPLEX_SITE_ROWS; row++) {
        int32_t x_cursor = adj_x;
        int32_t y_cursor = adj_y;
        int32_t dx = adj_x - adj_y;
        int32_t dy = adj_y - adj_x;
        for (int32_t col = 0; col < TEMPLE_COMPLEX_SITE_COLUMNS; col++) {
            int32_t x;
            int32_t y;
            if (direction == 0) {
                x = x_cursor;
                y = row_y;
            } else if (direction == 2) {
                x = row_x;
                y = y_cursor;
            } else if (direction == 4) {
                x = y_cursor + dx;
                y = row_x + dy;
            } else {
                x = row_y + dx;
                y = x_cursor + dy;
            }
            const int32_t *entry = decoration_tile(record);
            if (entry[0] != 0) {
                int32_t offset = entry[4];
                if (variant == 0) {
                    offset = entry[1];
                } else if (variant == 2) {
                    offset = entry[2];
                } else if (variant == 4) {
                    offset = entry[3];
                }
                temple_complex_set_tile(x, y, entry[0], offset);
                map_tiles_update_empty_land_around(x, y, 1);
            }
            record++;
            x_cursor++;
            y_cursor--;
        }
        row_x--;
        row_y++;
    }
}

void building_temple_complex_update_image(int32_t building_id, int32_t image_offset)
{
    const building *main = building_get(building_id);
    if (main->prev_part_building_id != 0) {
        return;
    }
    int32_t direction = main->data.temple_complex.orientation;
    int32_t flip = temple_image_flip(direction);
    int32_t complex_id = city_player_current()->temple_complex_building_id;
    int32_t part = 0;
    for (int32_t id = building_id; id != 0; id = building_get(id)->next_part_building_id) {
        int32_t index = part + (((map_info.orientation / 2) + ((direction / 2) * 4)) * 3);
        int32_t group = temple_part_group(index);
        int32_t image;
        if (group == IMAGE_GROUP_TEMPLE_COMPLEX_ALTAR) {
            if (building_temple_complex_has_upgrade(complex_id, TEMPLE_COMPLEX_ALTAR)) {
                image = image_group(IMAGE_GROUP_TEMPLE_COMPLEX_UPGRADE) + 2 + (flip != 0);
            } else {
                image = image_group(IMAGE_GROUP_TEMPLE_COMPLEX_ALTAR) + flip + image_offset;
            }
        } else if (group == IMAGE_GROUP_TEMPLE_COMPLEX_ORACLE) {
            if (building_temple_complex_has_upgrade(complex_id, TEMPLE_COMPLEX_ORACLE)) {
                image = image_group(IMAGE_GROUP_TEMPLE_COMPLEX_UPGRADE) + (flip != 0);
            } else {
                image = image_group(IMAGE_GROUP_TEMPLE_COMPLEX_ORACLE) + flip + image_offset;
            }
        } else {
            image = image_group(group) + flip + image_offset;
        }
        building *b = building_get(id);
        map_building_tiles_add(id, b->x, b->y, TEMPLE_COMPLEX_PART_SIZE, TEMPLE_COMPLEX_PART_SIZE, image, TERRAIN_BUILDING);
        part++;
    }
    temple_complex_place_decoration(building_id);
}

enum { ANNEX_SLOTS_PER_FACING = 4, ANNEX_GOD_GROUPS_FIRST = 4 };

static const gatehouse_part *annex_part(int32_t facing, int32_t index)
{
    return &gatehouse_parts.parts[gatehouse_orientation_parts[(facing * ANNEX_SLOTS_PER_FACING) + index]];
}

static int32_t annex_god_group(int32_t status)
{
    return complex_part_groups[ANNEX_GOD_GROUPS_FIRST + status];
}

void building_gatehouse_update_image(int32_t building_id)
{
    building *b = building_get(building_id);
    int32_t x = b->x;
    int32_t y = b->y;
    int32_t status = city_gods_with_status(GOD_STATUS_PATRON);
    if (status == -1) {
        status = city_gods_with_status(GOD_STATUS_LOCAL);
        if (status == -1) {
            status = 0;
        }
    }
    int32_t god_group = annex_god_group(status);
    int32_t direction = b->data.temple_complex.orientation;
    int32_t facing = direction + map_info.orientation;
    if (facing >= 8) {
        facing -= 8;
    }
    int32_t flip = 0;
    if ((uint32_t) facing <= 7 && (facing == 0 || facing == 3 || facing == 4 || facing == 7)) {
        flip = 1;
    }
    for (int32_t i = 0; i < GATEHOUSE_PART_COUNT; i++) {
        int32_t group = complex_part_groups[i];
        const gatehouse_part *part = annex_part(facing, i);
        int32_t off_a = part->dy;
        int32_t off_b = part->dx;
        int32_t tile_x = x;
        int32_t tile_y = y;
        if (direction == 0) {
            tile_x = x + off_b;
            tile_y = y + off_a;
        } else if (direction == 1) {
            tile_x = x + off_a;
            tile_y = y + off_b;
        }
        if (group == GATEHOUSE_TOWER_PLACEHOLDER_GROUP) {
            group = god_group;
        }
        int32_t size = part->size;
        uint32_t terrain = ((uint8_t) part->on_road == 0) ? (TERRAIN_GATEHOUSE | TERRAIN_BUILDING) : (TERRAIN_GATEHOUSE | TERRAIN_ROAD | TERRAIN_BUILDING);
        map_building_tiles_add(building_id, tile_x, tile_y, size, size, image_group(group) + flip, terrain);
    }
    map_tiles_update_region_walls(x, y, TEMPLE_COMPLEX_ANNEX_WALL_REGION);
}

void temple_complex_draw_annex(int32_t building_id)
{
    building *b = building_get(building_id);
    int32_t status = city_gods_with_status(GOD_STATUS_PATRON);
    if (status == -1) {
        status = city_gods_with_status(GOD_STATUS_LOCAL);
        if (status == -1) {
            status = 0;
        }
    }
    int32_t god_group = annex_god_group(status);
    int32_t direction = b->data.temple_complex.orientation;
    int32_t facing = direction + map_info.orientation;
    if (facing >= 8) {
        facing -= 8;
    }
    int32_t x_adjust = 64;
    int32_t y_adjust = -90;
    int32_t flip = 0;
    if ((uint32_t) facing <= 7) {
        if (facing == 0 || facing == 3 || facing == 4 || facing == 7) {
            x_adjust = -1;
            y_adjust = -65;
            flip = 1;
        } else {
            x_adjust = 12;
            y_adjust = -64;
        }
    }
    for (int32_t slot_index = TEMPLE_COMPLEX_ANNEX_STATUE; slot_index <= TEMPLE_COMPLEX_ANNEX_GOD; slot_index++) {
        const gatehouse_part *part = annex_part(facing, slot_index);
        int32_t off_a = part->dy;
        int32_t off_b = part->dx;
        int32_t tile_x = b->x;
        int32_t tile_y = b->y;
        if (direction == 0) {
            tile_x = b->x + off_b;
            tile_y = b->y + off_a;
        } else if (direction == 1) {
            tile_x = b->x + off_a;
            tile_y = b->y + off_b;
        }
        int32_t image;
        int32_t pixel_shift_x = 0;
        int32_t pixel_shift_y = 0;
        if (slot_index == TEMPLE_COMPLEX_ANNEX_STATUE) {
            image = image_group(IMAGE_GROUP_TEMPLE_COMPLEX_STATUE) + flip;
            pixel_shift_x = x_adjust;
            pixel_shift_y = y_adjust;
        } else {
            int32_t size = part->size;
            int32_t orient = map_info.orientation;
            if (orient == 2) {
                tile_x += size - 1;
            } else if (orient == 4) {
                tile_x += size - 1;
                tile_y += size - 1;
            } else if (orient != 0) {
                tile_y += size - 1;
            }
            image = image_group(god_group) + flip;
        }
        int32_t pixel_x = 0;
        int32_t pixel_y = 0;
        city_view_tile_to_pixel(tile_x, tile_y, &pixel_x, &pixel_y);
        if (b->state == BUILDING_STATE_DELETED) {
            city_view.color_mask = GHOST_COLOR_BLOCKED;
        }
        monument_draw_image(image, pixel_x + pixel_shift_x, pixel_y + pixel_shift_y);
        image_draw_isometric_top(image, pixel_x + pixel_shift_x, pixel_y + pixel_shift_y);
        city_view.color_mask = 0;
    }
}

static int32_t temple_part_delta_x(int32_t direction)
{
    return gatehouse_parts.part_steps[direction].x;
}

static int32_t temple_part_delta_y(int32_t direction)
{
    return gatehouse_parts.part_steps[direction].y;
}

void building_temple_complex_add_to_map(int32_t building_id, int32_t x, int32_t y, int32_t create_a, int32_t create_b, int32_t place)
{
    building *b = building_get(building_id);
    if (b->prev_part_building_id != 0) {
        return;
    }
    int32_t direction;
    if (place) {
        b->data.temple_complex.upgrades = 0;
        city_player *city = city_player_current();
        city->has_temple_complex = 1;
        city->temple_complex_upgrades = 2;
        city->temple_complex_building_id = building_id;
        direction = construction_orientation * 2 - map_info.orientation;
        if (direction < 0) {
            direction += 8;
        }
        b->data.temple_complex.orientation = (uint8_t) direction;
    } else {
        direction = b->data.temple_complex.orientation;
    }
    int32_t tile_id = building_id;
    for (int32_t part = 0; part < TEMPLE_COMPLEX_PARTS; part++) {
        int32_t base = (map_info.orientation >> 1) + ((direction >> 1) * 4);
        int32_t part_x = x + (temple_part_delta_x(direction) * part);
        int32_t part_y = y + (temple_part_delta_y(direction) * part);
        int32_t group;
        if (part == 0) {
            group = temple_part_group(base * TEMPLE_COMPLEX_PARTS);
            if (place) {
                b->prev_part_building_id = 0;
            }
        } else if (part == 1) {
            if (place) {
                int32_t created = building_create(create_a, create_b, part_x, part_y, 0, 0);
                building *created_part = building_get(created);
                tile_id = created;
                created_part->data.temple_complex.orientation = (uint8_t) direction;
                game_undo_add_building(created);
                created_part->prev_part_building_id = (int16_t) building_id;
                b->next_part_building_id = (int16_t) created;
                created_part->next_part_building_id = 0;
            }
            group = IMAGE_GROUP_TEMPLE_COMPLEX_ORACLE;
        } else {
            int32_t link = b->next_part_building_id;
            if (place) {
                int32_t created = building_create(create_a, create_b, part_x, part_y, 0, 0);
                building *created_part = building_get(created);
                tile_id = created;
                created_part->data.temple_complex.orientation = (uint8_t) direction;
                game_undo_add_building(created);
                created_part->prev_part_building_id = (int16_t) link;
                building_get(link)->next_part_building_id = (int16_t) created;
                created_part->next_part_building_id = 0;
            }
            group = temple_part_group(part + (base * TEMPLE_COMPLEX_PARTS));
        }
        if (place && (uint32_t) direction <= 6 && (direction == 0 || direction == 2 || direction == 4 || direction == 6)) {
            int32_t subtype = (direction == 2 || direction == 6) ? part + 3 : part;
            building_get(tile_id)->subtype = (int16_t) subtype;
        }
        int32_t orient = map_info.orientation;
        int32_t flip = 0;
        if (orient == 0 || orient == 4) {
            if (direction == 2 || direction == 6) {
                flip = 3;
            }
        } else if (direction != 2 && direction != 6) {
            flip = 3;
        }
        int32_t image = image_group(group) + flip;
        map_building_tiles_add(tile_id, part_x, part_y, TEMPLE_COMPLEX_PART_SIZE, TEMPLE_COMPLEX_PART_SIZE, image, TERRAIN_BUILDING);
    }
    temple_complex_place_decoration(building_main(tile_id));
    city_player_current()->bast_culture_decay_skip = (building_temple_god(tile_id) == GOD_BAST) ? TEMPLE_COMPLEX_BAST_CULTURE_DECAY_SKIP : 0;
}
