// House size changes: merging four huts into a 2x2 house, expanding a house onto its neighbours when it evolves
// into the next size, splitting houses back into 1x1 houses, devolving the larger houses, changing a house's type
// and image, turning it into a vacant lot, and the position check of a house against its tile.

#include "building/house_size.h"
#include "building/building.h"
#include "building/building_create.h"
#include "building/building_image_group.h"
#include "building/building_type.h"
#include "building/house_evolve.h"
#include "graphics/image_collection.h"
#include "map/building_tiles.h"
#include "map/grid.h"
#include "map/routing_terrain.h"
#include "map/terrain.h"
#include "map/tiles.h"
#include <stdint.h>

int32_t house_merge_inventory[12];

int32_t house_expand_y;

int32_t house_expand_x;

static int32_t house_image(const building *b)
{
    int32_t level = b->subtype;
    return image_group(house_image_group[level]) + house_image_offset[level];
}

static int32_t merged_house_image(const building *b)
{
    int32_t level = b->subtype;
    return image_group(house_image_group[level]) + (house_image_offset[level] ? HOUSE_MERGED_IMAGE_WITH_OFFSET : HOUSE_MERGED_IMAGE);
}

static int32_t tile_is_takeable(int32_t offset, int32_t building_id, int32_t level, int32_t same_level_unmerged)
{
    if (!(map_grids.terrain[offset] & TERRAIN_BUILDING)) {
        return 0;
    }
    int32_t id = map_grids.building_ids[offset];
    if (id == building_id) {
        return 1;
    }
    const building *other = building_get(id);
    if (other->state != BUILDING_STATE_IN_USE || !other->house_size) {
        return 0;
    }
    if (same_level_unmerged) {
        return (other->subtype == (int16_t) level) && !other->house_is_merged;
    }
    return other->subtype <= (int16_t) level;
}

int32_t building_house_can_expand(int32_t building_id, int32_t num_tiles)
{
    building *b = building_get(building_id);
    int32_t base = b->grid_offset;
    int32_t level = b->subtype;
    for (int32_t pass = 0; pass < HOUSE_EXPAND_PASSES; pass++) {
        for (int32_t dir = 0; dir < HOUSE_EXPAND_DIRECTIONS; dir++) {
            int32_t area = base + house_expand_direction[dir];
            int32_t ok = 0;
            for (int32_t i = 0; i < num_tiles; i++) {
                int32_t offset = area + house_tile_offsets[i];
                uint32_t terrain = map_grids.terrain[offset];
                if ((pass >= HOUSE_EXPAND_PASS_EMPTY_LAND) && ((terrain & TERRAIN_NOT_CLEAR) == 0)) {
                    ok++;
                }
                if (terrain & TERRAIN_BUILDING) {
                    if (tile_is_takeable(offset, building_id, level, 0)) {
                        ok++;
                    }
                } else if ((pass == HOUSE_EXPAND_PASS_GARDENS) && (terrain & TERRAIN_GARDEN)) {
                    ok++;
                }
            }
            if (ok == num_tiles) {
                house_expand_x = b->x + house_expand_delta[dir * 2];
                house_expand_y = b->y + house_expand_delta[(dir * 2) + 1];
                return 1;
            }
        }
    }
    b->data.house.no_space_to_expand = 1;
    return 0;
}

void building_house_check_for_corruption(int32_t building_id)
{
    building *b = building_get(building_id);
    b->data.house.no_space_to_expand = 0;
    if ((b->grid_offset == map_grid_offset(b->x, b->y)) && (map_grids.building_ids[b->grid_offset] == building_id)) {
        return;
    }
    building_corrupt_houses_a++;
    int32_t offset = map_info.grid_start;
    for (int32_t y = 0; y < map_info.grid_height; y++) {
        for (int32_t x = 0; x < map_info.grid_width; x++, offset++) {
            if (map_grids.building_ids[offset] == building_id) {
                int32_t relative = offset - map_info.grid_start;
                b->grid_offset = offset;
                b->x = (int16_t) (relative % GRID_SIZE);
                b->y = (int16_t) (relative / GRID_SIZE);
                return;
            }
        }
        offset += map_info.grid_border_size;
    }
    b->state = BUILDING_STATE_RUBBLE;
    building_corrupt_houses_b++;
}

int32_t building_house_merge(int32_t building_id)
{
    const building *b = building_get(building_id);
    if (b->house_is_merged || ((map_grids.random[b->grid_offset] & HOUSE_MERGE_RANDOM_MASK) >= HOUSE_MERGE_RANDOM_BELOW)) {
        return 0;
    }
    int32_t count = 0;
    for (int32_t i = 0; i < HOUSE_TILES_2X2; i++) {
        if (tile_is_takeable(b->grid_offset + house_tile_offsets[i], building_id, b->subtype, 1)) {
            count++;
        }
    }
    if (count != HOUSE_TILES_2X2) {
        return 0;
    }
    house_expand_x = b->x + house_expand_delta[0];
    house_expand_y = b->y + house_expand_delta[1];
    building_house_merge_to_size2(building_id, building_house_prepare_for_merge(building_id, HOUSE_TILES_2X2));
    return 1;
}

void building_house_split_houses_in_area(int32_t building_id, int32_t num_tiles)
{
    int32_t area = map_grid_offset(house_expand_x, house_expand_y);
    for (int32_t i = 0; i < num_tiles; i++) {
        int32_t offset = area + house_tile_offsets[i];
        if (!(map_grids.terrain[offset] & TERRAIN_BUILDING)) {
            continue;
        }
        int32_t id = map_grids.building_ids[offset];
        if (id == building_id) {
            continue;
        }
        const building *other = building_get(id);
        if (!other->house_size) {
            continue;
        }
        if (other->house_is_merged == 1) {
            building_house_split_merged(id);
        } else if (other->size == 2) {
            building_house_split_size2(id);
        } else if (other->size == 3) {
            building_house_split_size3(id);
        }
    }
}

int32_t building_house_prepare_for_merge(int32_t building_id, int32_t num_tiles)
{
    for (int32_t i = 0; i < HOUSE_INVENTORY_SLOTS; i++) {
        house_merge_inventory[i] = 0;
    }
    int32_t population = 0;
    int32_t area = map_grid_offset(house_expand_x, house_expand_y);
    for (int32_t i = 0; i < num_tiles; i++) {
        int32_t offset = area + house_tile_offsets[i];
        if (!(map_grids.terrain[offset] & TERRAIN_BUILDING)) {
            continue;
        }
        int32_t id = map_grids.building_ids[offset];
        if (id == building_id) {
            continue;
        }
        building *other = building_get(id);
        if (!other->house_size) {
            continue;
        }
        population += other->house_population;
        for (int32_t n = 0; n < HOUSE_INVENTORY_SLOTS; n++) {
            house_merge_inventory[n] += other->data.house.inventory[n];
        }
        other->house_population = 0;
        other->state = BUILDING_STATE_DELETED_BY_GAME;
    }
    return population;
}

static void take_merged(building *b, int32_t population)
{
    b->house_population = (int16_t) (b->house_population + (int16_t) population);
    for (int32_t i = 0; i < HOUSE_INVENTORY_SLOTS; i++) {
        b->data.house.inventory[i] = (int16_t) (b->data.house.inventory[i] + (int16_t) house_merge_inventory[i]);
    }
}

static void move_to_expand_area(building *b, int32_t building_id)
{
    map_building_tiles_remove(building_id, b->x, b->y);
    b->x = (int16_t) house_expand_x;
    b->y = (int16_t) house_expand_y;
    b->grid_offset = map_grid_offset(house_expand_x, house_expand_y);
}

static void expand_to(int32_t building_id, int32_t population, int32_t type, int32_t size, int32_t random_variant)
{
    building *b = building_get(building_id);
    b->type = (int16_t) type;
    b->subtype = (int16_t) (type - BUILDING_HOUSE_CRUDE_HUT);
    b->size = (uint8_t) size;
    b->house_size = (uint8_t) size;
    take_merged(b, population);
    int32_t image = house_image(b);
    if (random_variant) {
        image += map_grids.random[b->grid_offset] & 1;
    }
    move_to_expand_area(b, building_id);
    map_building_tiles_add(building_id, b->x, b->y, b->size, b->size, image, TERRAIN_BUILDING);
}

void building_house_expand_to_common_residence(int32_t building_id, int32_t population)
{
    expand_to(building_id, population, BUILDING_HOUSE_COMMON_RESIDENCE, 2, 1);
}

void building_house_expand_to_common_manor(int32_t building_id, int32_t population)
{
    expand_to(building_id, population, BUILDING_HOUSE_COMMON_MANOR, 3, 0);
}

void building_house_expand_to_modest_estate(int32_t building_id, int32_t population)
{
    expand_to(building_id, population, BUILDING_HOUSE_MODEST_ESTATE, 4, 0);
}

void building_house_merge_to_size2(int32_t building_id, int32_t population)
{
    building *b = building_get(building_id);
    b->size = 2;
    b->house_size = 2;
    take_merged(b, population);
    int32_t image = merged_house_image(b);
    move_to_expand_area(b, building_id);
    b->house_is_merged = 1;
    map_building_tiles_add(building_id, b->x, b->y, 2, 2, image, TERRAIN_BUILDING);
}

static void split_into_huts(int32_t building_id, int32_t side, int32_t new_type)
{
    building *b = building_get(building_id);
    int32_t parts = side * side;
    int32_t population = b->house_population;
    int32_t share[HOUSE_INVENTORY_SLOTS];
    int32_t rest[HOUSE_INVENTORY_SLOTS];
    for (int32_t i = 0; i < HOUSE_INVENTORY_SLOTS; i++) {
        share[i] = b->data.house.inventory[i] / parts;
        rest[i] = b->data.house.inventory[i] % parts;
    }
    map_building_tiles_remove(building_id, b->x, b->y);
    if (new_type) {
        b->type = (int16_t) new_type;
        b->subtype = (int16_t) (new_type - BUILDING_HOUSE_CRUDE_HUT);
    }
    b->size = 1;
    b->house_size = 1;
    b->house_is_merged = 0;
    int16_t population_share = (int16_t) (population / parts);
    b->house_population = (int16_t) ((population % parts) + population_share);
    for (int32_t i = 0; i < HOUSE_INVENTORY_SLOTS; i++) {
        b->data.house.inventory[i] = (int16_t) (rest[i] + share[i]);
    }
    b->distance_from_entry = 0;
    int32_t image = house_image(b);
    map_building_tiles_add(building_id, b->x, b->y, 1, 1, image + (map_grids.random[b->grid_offset] & 1), TERRAIN_BUILDING);
    int32_t x = b->x;
    int32_t y = b->y;
    for (int32_t dy = 0; dy < side; dy++) {
        for (int32_t dx = 0; dx < side; dx++) {
            if (!dx && !dy) {
                continue;
            }
            int32_t id = building_create(city_current_player, b->type, x + dx, y + dy, 0, 0);
            building *part = building_get(id);
            part->house_population = population_share;
            for (int32_t i = 0; i < HOUSE_INVENTORY_SLOTS; i++) {
                part->data.house.inventory[i] = (int16_t) share[i];
            }
            part->distance_from_entry = 0;
            map_building_tiles_add(id, part->x, part->y, 1, 1, image + (map_grids.random[part->grid_offset] & 1), TERRAIN_BUILDING);
        }
    }
}

void building_house_split_merged(int32_t building_id)
{
    split_into_huts(building_id, 2, 0);
}

void building_house_split_size2(int32_t building_id)
{
    split_into_huts(building_id, 2, BUILDING_HOUSE_SPACIOUS_APARTMENT);
}

void building_house_split_size3(int32_t building_id)
{
    split_into_huts(building_id, 3, BUILDING_HOUSE_SPACIOUS_APARTMENT);
}

static void devolve_to_smaller(int32_t building_id, int32_t type, int32_t size, int32_t divisor, int32_t random_variant)
{
    building *b = building_get(building_id);
    int32_t population = b->house_population;
    int32_t share[HOUSE_INVENTORY_SLOTS];
    int32_t rest[HOUSE_INVENTORY_SLOTS];
    for (int32_t i = 0; i < HOUSE_INVENTORY_SLOTS; i++) {
        share[i] = b->data.house.inventory[i] / divisor;
        rest[i] = b->data.house.inventory[i] % divisor;
    }
    map_building_tiles_remove(building_id, b->x, b->y);
    b->type = (int16_t) type;
    b->subtype = (int16_t) (type - BUILDING_HOUSE_CRUDE_HUT);
    b->size = (uint8_t) size;
    b->house_size = (uint8_t) size;
    b->house_is_merged = 0;
    b->house_population = (int16_t) ((population / divisor) + (population % divisor));
    for (int32_t i = 0; i < HOUSE_INVENTORY_SLOTS; i++) {
        b->data.house.inventory[i] = (int16_t) (rest[i] + share[i]);
    }
    b->distance_from_entry = 0;
    int32_t image = house_image(b);
    if (random_variant) {
        image += map_grids.random[b->grid_offset] & 1;
    }
    map_building_tiles_add(building_id, b->x, b->y, size, size, image, TERRAIN_BUILDING);
    int32_t x = b->x;
    int32_t y = b->y;
    int32_t base = map_grid_offset(x, y);
    for (int32_t i = 0; i <= size; i++) {
        map_grids.terrain[base + size + (i * GRID_SIZE)] |= TERRAIN_GARDEN;
        map_grids.terrain[base + (size * GRID_SIZE) + i] |= TERRAIN_GARDEN;
    }
    map_tiles_update_all_gardens();
    map_routing_update_land_citizen_rect(x, y, x + size, y + size, 1);
    map_routing_update_land_noncitizen_rect(x, y, x + size, y + size);
}

void building_house_devolve_from_common_manor(int32_t building_id)
{
    devolve_to_smaller(building_id, BUILDING_HOUSE_FANCY_RESIDENCE, 2, HOUSE_DEVOLVE_3X3_SHARE, 1);
}

void building_house_devolve_from_modest_estate(int32_t building_id)
{
    devolve_to_smaller(building_id, BUILDING_HOUSE_STATELY_MANOR, 3, HOUSE_DEVOLVE_4X4_SHARE, 0);
}

void building_house_change_to(int32_t building_id, int32_t type)
{
    building *b = building_get(building_id);
    b->type = (int16_t) type;
    b->subtype = (int16_t) (b->type - BUILDING_HOUSE_CRUDE_HUT);
    int32_t level = b->subtype;
    int32_t image;
    if (!b->house_is_merged) {
        image = house_image(b);
        if (house_image_variants[level]) {
            image += map_grids.random[b->grid_offset] & (house_image_variants[level] - 1);
        }
    } else {
        image = merged_house_image(b);
    }
    map_building_tiles_add(building_id, b->x, b->y, b->size, b->size, image, TERRAIN_BUILDING);
}

void building_house_change_to_vacant_lot(int32_t building_id)
{
    building *b = building_get(building_id);
    int32_t image = image_group(IMAGE_GROUP_VACANT_LOT);
    b->type = BUILDING_HOUSE_CRUDE_HUT;
    b->subtype = HOUSE_LEVEL_CRUDE_HUT;
    if (!b->house_is_merged) {
        map_grids.images[b->grid_offset] = image;
        return;
    }
    map_building_tiles_remove(building_id, b->x, b->y);
    b->house_is_merged = 0;
    b->size = 1;
    b->house_size = 1;
    map_building_tiles_add(building_id, b->x, b->y, 1, 1, image, TERRAIN_BUILDING);
    int32_t x = b->x;
    int32_t y = b->y;
    for (int32_t n = 1; n < HOUSE_TILES_2X2; n++) {
        int32_t id = building_create(city_current_player, b->type, x + (n & 1), y + (n >> 1), 0, 0);
        building *part = building_get(id);
        part->house_population = 0;
        part->distance_from_entry = 0;
        map_building_tiles_add(id, part->x, part->y, 1, 1, image, TERRAIN_BUILDING);
    }
}
