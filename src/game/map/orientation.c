// View rotation: after the view is turned, the terrain, the buildings whose image depends on the view (statues,
// shrines, the sphinx, water buildings, the water lift, venues, temple complexes, monuments), the corner tiles that
// draw each building, the bridges, the walls and the entry and exit flags are updated. Also the water building tiles,
// the flooded test of a building and the free tile search near a point.
//
// What the rotation leaves as it is:
// - water buildings keep some of their old bitfield bits before the size is set;
// - removing a flag also clears the ore and rock-near-monument bits of its tile;
// - the ship bridges' sprites are not turned with the view.

#include "map/orientation.h"

#include "building/abu_simbel.h"
#include "building/building.h"
#include "building/building_figure_helpers.h"
#include "building/building_image_group.h"
#include "building/building_type.h"
#include "building/caesareum.h"
#include "building/construction_load.h"
#include "building/construction_place.h"
#include "building/destroy_enemy.h"
#include "building/library.h"
#include "building/lighthouse.h"
#include "building/mausoleum.h"
#include "building/monument_object.h"
#include "building/pyramid_parts.h"
#include "building/royal_tomb.h"
#include "building/sun_temple.h"
#include "building/temple_complex.h"
#include "figure/figure_data.h"
#include "io/io_data.h"
#include "map/bridge.h"
#include "map/map_data.h"
#include "map/routing_terrain.h"
#include "map/terrain.h"
#include "map/tiles_internal.h"
#include "widget/city_view.h"
#include "widget/city_view_rotate.h"
#include "widget/widget_data.h"

#include <stdint.h>

uint8_t water_lift_has_water_access;

static void add_water_building(int32_t building_id, int32_t x, int32_t y, int32_t size, int32_t image_id, uint8_t bitfields_mask, uint8_t bitfields_size)
{
    int32_t corner_x;
    int32_t corner_y;
    switch (city_view_orientation) {
        case DIRECTION_TOP:
            corner_x = 0;
            corner_y = size - 1;
            break;
        case DIRECTION_RIGHT:
            corner_x = 0;
            corner_y = 0;
            break;
        case DIRECTION_BOTTOM:
            corner_x = size - 1;
            corner_y = 0;
            break;
        default:
            corner_x = size - 1;
            corner_y = size - 1;
            break;
    }
    if (x < 0 || y < 0 || x + size - 1 >= map_info.grid_width || y + size - 1 >= map_info.grid_height) {
        return;
    }
    int32_t base = tile_offset(x, y);
    map_tile_cursor_offset = base;
    for (int32_t dy = 0; dy < size; dy++) {
        for (int32_t dx = 0; dx < size; dx++) {
            int32_t offset = base + dy * GRID_SIZE + dx;
            uint32_t terrain = map_grids.terrain[offset];
            map_grids.terrain[offset] = terrain | TERRAIN_BUILDING;
            if (!(terrain & TERRAIN_WATER)) {
                map_grids.terrain[offset] = (terrain & TERRAIN_KEPT_BY_BUILDING) | TERRAIN_BUILDING;
            }
            map_grids.building_ids[offset] = (int16_t) building_id;
            map_grids.bitfields[offset] = (map_grids.bitfields[offset] & bitfields_mask) | bitfields_size;
            map_grids.images[offset] = image_id;
            map_grids.edge[offset] = (uint8_t) ((dy * MAP_EDGE_ROW_ONE) + dx);
            if (dx == corner_x && dy == corner_y) {
                map_grids.edge[offset] |= MAP_EDGE_DRAW_TILE;
            }
        }
    }
}

void map_building_tiles_add_water_size2(int32_t building_id, int32_t x, int32_t y, int32_t image_id)
{
    add_water_building((int16_t) building_id, x, y, 2, image_id, 0xE1, 1);
}

void map_building_tiles_add_water_size3(int32_t building_id, int32_t x, int32_t y, int32_t image_id)
{
    add_water_building((int16_t) building_id, x, y, 3, image_id, 0xEF, 2);
}

static int32_t flooded(int32_t offset)
{
    return !floodplain_node_dry(offset);
}

int32_t map_terrain_2x2_is_flooded(int32_t x, int32_t y)
{
    int32_t offset = tile_offset(x, y);
    return flooded(offset) || flooded(offset + 1) || flooded(offset + GRID_SIZE) || flooded(offset + GRID_SIZE + 1);
}

int32_t map_building_is_flooded(int32_t building_id)
{
    const building *b = building_get(building_id);
    return (uint8_t) map_terrain_2x2_is_flooded(b->x, b->y);
}

static const int32_t water_lift_groups[VIEW_TURNS] = {
    IMAGE_GROUP_WATER_LIFT_TURN_0,
    IMAGE_GROUP_WATER_LIFT_TURN_1,
    IMAGE_GROUP_WATER_LIFT_TURN_2,
    IMAGE_GROUP_WATER_LIFT_TURN_3,
};

static const int32_t water_lift_blood_groups[VIEW_TURNS] = {
    IMAGE_GROUP_WATER_LIFT_TURN_0_BLOOD,
    IMAGE_GROUP_WATER_LIFT_TURN_1_BLOOD,
    IMAGE_GROUP_WATER_LIFT_TURN_2_BLOOD,
    IMAGE_GROUP_WATER_LIFT_TURN_3_BLOOD,
};

static int32_t water_lift_flood_variant(void)
{
    if (!water_lift_flooded) {
        return 0;
    }
    return water_lift_has_water_access ? WATER_LIFT_IMAGE_FLOODED_WITH_WATER_ACCESS : WATER_LIFT_IMAGE_FLOODED;
}

int32_t map_water_lift_image(int32_t direction, int32_t blood)
{
    int32_t turn = ((direction >= 0) && (direction < VIEW_TURNS - 1)) ? direction : (VIEW_TURNS - 1);
    const int32_t *groups = (char) blood ? water_lift_blood_groups : water_lift_groups;
    return water_lift_flood_variant() + image_group(groups[turn]);
}

static int32_t view_direction(int32_t building_id)
{
    int32_t direction = building_get(building_id)->data.oriented.orientation - (map_info.orientation / 2);
    return (direction < 0) ? (direction + VIEW_TURNS) : direction;
}

void map_building_update_water_lift_image(int32_t id)
{
    const building *b = building_get(id);
    water_lift_flooded = (uint8_t) map_building_is_flooded(id);
    water_lift_has_water_access = (b->has_water_access != 0);
    int32_t direction = view_direction(id);
    int32_t x = b->x;
    int32_t y = b->y;
    int32_t blood = map_sweep_has_passed(x);
    map_building_tiles_add_water_size2(id, x, y, map_water_lift_image(direction, blood));
}

static void add_tiles(int32_t id, int32_t image_id, uint32_t terrain)
{
    const building *b = building_get(id);
    map_building_tiles_add(id, b->x, b->y, b->size, b->size, image_id, terrain);
}

static void add_water_building_tiles(int32_t id, int32_t size, int32_t group, int32_t blood_group)
{
    const building *b = building_get(id);
    int32_t direction = view_direction(id);
    int32_t x = b->x;
    int32_t y = b->y;
    int32_t image = image_group((blood_group && map_sweep_has_passed(x)) ? blood_group : group) + direction;
    if (size == 2) {
        map_building_tiles_add_water_size2(id, x, y, image);
    } else {
        map_building_tiles_add_water_size3(id, x, y, image);
    }
}

static int32_t sphinx_image(int32_t id)
{
    int32_t g0 = image_group(IMAGE_GROUP_SPHINX_END_A);
    int32_t g1 = image_group(IMAGE_GROUP_SPHINX_MIDDLE);
    int32_t g2 = image_group(IMAGE_GROUP_SPHINX_END_B);
    int32_t subtype = building_get(id)->subtype;
    int32_t odd = (subtype == 1) || (subtype == 4);
    int32_t first = (subtype == 0) || (subtype == 3);
    switch (map_info.orientation) {
        case DIRECTION_TOP:
            return (first ? g0 : odd ? g1 : g2) + 1;
        case DIRECTION_BOTTOM:
            return (first ? g2 : odd ? g1 : g0) + 1;
        case DIRECTION_LEFT:
            return first ? g0 : odd ? g1 : g2;
        default:
            return first ? g2 : odd ? g1 : g0;
    }
}

static void update_venue(int32_t id, int32_t type)
{
    const building *b = building_get(id);
    if (building_main(id) == id && b->venue_has_plaza != 0) {
        map_add_venue_plaza_tiles(b->venue_plaza_grid_offset, type, id);
    }
}

static int32_t statue_variants_group(int32_t size)
{
    if (size == 1) {
        return IMAGE_GROUP_SMALL_STATUE_VARIANTS;
    }
    return (size == 2) ? IMAGE_GROUP_MEDIUM_STATUE_VARIANTS : IMAGE_GROUP_LARGE_STATUE_VARIANTS;
}

static int32_t statue_image(const building *b, int32_t type)
{
    int32_t orientation_data = b->data.oriented.orientation;
    int32_t direction = (orientation_data & STATUE_DATA_DIRECTION) - (map_info.orientation / 2) - 1;
    if (direction < 0) {
        direction += VIEW_TURNS;
    }
    int32_t variant = orientation_data >> STATUE_DATA_VARIANT_SHIFT;
    if (variant < STATUE_VARIANTS_IN_OWN_GROUP) {
        return image_group(building_property_get(type)->image_group) + (variant * VIEW_TURNS) + direction;
    }
    return image_group(statue_variants_group(b->size)) + ((variant - STATUE_VARIANTS_IN_OWN_GROUP) * VIEW_TURNS) + direction;
}

static void update_building(int32_t id)
{
    const building *b = building_get(id);
    int32_t type = b->type;
    if (type >= BUILDING_SHRINE_OSIRIS && type <= BUILDING_SHRINE_GOD_9) {
        add_tiles(id, image_group(building_property_get(type)->image_group), TERRAIN_BUILDING);
        return;
    }
    if (type >= BUILDING_SMALL_STATUE && type <= BUILDING_LARGE_STATUE) {
        add_tiles(id, statue_image(b, type), TERRAIN_BUILDING);
        return;
    }
    switch (type) {
        case BUILDING_MUD_GATEHOUSE:
        case BUILDING_MUD_GATEHOUSE_UP:
        case BUILDING_BRICK_GATEHOUSE:
        case BUILDING_BRICK_GATEHOUSE_UP:
        case BUILDING_TRIUMPHAL_ARCH:
            return;
        case BUILDING_GATEHOUSE:
        case BUILDING_GATEHOUSE_UP:
            building_gatehouse_update_image(id);
            return;
        case BUILDING_SPHINX:
            add_tiles(id, sphinx_image(id), TERRAIN_BUILDING);
            return;
        case BUILDING_SHIPWRIGHT:
            add_water_building_tiles(id, 3, IMAGE_GROUP_SHIPWRIGHT, IMAGE_GROUP_SHIPWRIGHT_BLOOD);
            return;
        case BUILDING_FISHING_WHARF:
            add_water_building_tiles(id, 2, IMAGE_GROUP_FISHING_WHARF, IMAGE_GROUP_FISHING_WHARF_BLOOD);
            return;
        case BUILDING_TRANSPORT_WHARF:
            add_water_building_tiles(id, 2, IMAGE_GROUP_TRANSPORT_WHARF, IMAGE_GROUP_TRANSPORT_WHARF_BLOOD);
            return;
        case BUILDING_WARSHIP_WHARF:
            add_water_building_tiles(id, 3, IMAGE_GROUP_WARSHIP_WHARF, IMAGE_GROUP_WARSHIP_WHARF_BLOOD);
            return;
        case BUILDING_WATER_LIFT:
            map_building_update_water_lift_image(id);
            return;
        case BUILDING_DOCK:
            add_water_building_tiles(id, 3, IMAGE_GROUP_DOCK, IMAGE_GROUP_DOCK_BLOOD);
            return;
        case BUILDING_DOCK_UP:
            add_water_building_tiles(id, 3, IMAGE_GROUP_DOCK_UP, 0);
            return;
        case BUILDING_FERRY_LANDING:
            add_water_building_tiles(id, 2, IMAGE_GROUP_FERRY_LANDING, IMAGE_GROUP_FERRY_LANDING_BLOOD);
            return;
        case BUILDING_BOOTH:
        case BUILDING_PAVILION:
        case BUILDING_FESTIVAL_SQUARE:
            update_venue(id, type);
            return;
        case BUILDING_BANDSTAND:
            if (building_get(b->next_part_building_id)->type == BUILDING_BANDSTAND) {
                venue_place_adjacent_tiles(id, b->x, b->y);
            }
            update_venue(id, type);
            return;
        default:
            break;
    }
    if (type >= BUILDING_TEMPLE_COMPLEX_OSIRIS && type <= BUILDING_TEMPLE_COMPLEX_BAST) {
        building_temple_complex_update_image(id, 0);
        return;
    }
    int32_t image;
    switch (type) {
        case BUILDING_SUN_TEMPLE:
            sun_temple_update_image(id);
            break;
        case BUILDING_PYRAMID_ENTRY_TILE:
            add_tiles(id, image_group(IMAGE_GROUP_PYRAMID_ENTRY_TILE), TERRAIN_BUILDING | TERRAIN_ROAD);
            break;
        case BUILDING_MORTUARY_TEMPLE:
            image = ((map_info.orientation / 2) & 1) + image_group(IMAGE_GROUP_MORTUARY_TEMPLE);
            if (image) {
                add_tiles(id, image, TERRAIN_BUILDING);
            }
            break;
        case BUILDING_CAUSEWAY:
            image = ((map_info.orientation / 2) & 1) + image_group(IMAGE_GROUP_CAUSEWAY);
            if (image) {
                add_tiles(id, image, TERRAIN_BUILDING);
            }
            break;
        case BUILDING_VALLEY_TEMPLE:
            image = (map_info.orientation / 2) + image_group(IMAGE_GROUP_VALLEY_TEMPLE);
            if (image) {
                add_tiles(id, image, TERRAIN_BUILDING);
            }
            break;
        case BUILDING_MAUSOLEUM:
            mausoleum_part_update_position(id);
            break;
        case BUILDING_ALEXANDRIA_LIBRARY:
            library_update_image(&library_object.base, id);
            break;
        case BUILDING_CAESAREUM:
            caesareum_update_image(&caesareum_object.base, id);
            break;
        case BUILDING_LIGHTHOUSE:
            lighthouse_update_image(&lighthouse_object.base, id);
            break;
        case BUILDING_SMALL_ROYAL_TOMB:
            royal_tomb_update_image(&small_royal_tomb_object.base, id);
            break;
        case BUILDING_ABU_SIMBEL:
            abu_simbel_update_image(&abu_simbel_object.base, id);
            break;
        case BUILDING_MEDIUM_ROYAL_TOMB:
            royal_tomb_update_image(&medium_royal_tomb_object.base, id);
            break;
        case BUILDING_LARGE_ROYAL_TOMB:
            royal_tomb_update_image(&large_royal_tomb_object.base, id);
            break;
        case BUILDING_GRAND_ROYAL_TOMB:
            royal_tomb_update_image(&grand_royal_tomb_object.base, id);
            break;
        default:
            break;
    }
}

void map_orientation_update_buildings(void)
{
    for (int32_t id = 1; id <= building_highest_id; id++) {
        if (building_get(id)->state) {
            update_building(id);
        }
    }
    pyramids_update_all_images();
}

int32_t map_terrain_find_free_tile(int32_t x, int32_t y, int32_t size, int32_t radius, int32_t skip_offset)
{
    int32_t x0 = x - radius;
    int32_t x1 = x + size - 1 + radius;
    int32_t y1 = y + size - 1 + radius;
    map_result_y = y - radius;
    if (x0 < 0) {
        x0 = 0;
    }
    if (map_result_y < 0) {
        map_result_y = 0;
    }
    if (x1 >= map_info.grid_width) {
        x1 = map_info.grid_width - 1;
    }
    if (y1 >= map_info.grid_height) {
        y1 = map_info.grid_height - 1;
    }
    int32_t offset = tile_offset(x0, map_result_y);
    for (; map_result_y <= y1; map_result_y++) {
        for (map_result_x = x0; map_result_x <= x1; map_result_x++, offset++) {
            if (offset != skip_offset && map_grids.terrain[offset] == 0) {
                return 1;
            }
        }
        offset += x0 - x1 + (GRID_SIZE - 1);
    }
    return 0;
}

static int32_t edge_side(int32_t x, int32_t y)
{
    if (x == 0) {
        return DIRECTION_RIGHT;
    }
    if (x == map_info.grid_width - 1) {
        return DIRECTION_LEFT;
    }
    if (y == 0) {
        return DIRECTION_TOP;
    }
    return (y == map_info.grid_height - 1) ? DIRECTION_BOTTOM : -1;
}

static void place_flag(int32_t x, int32_t y, int32_t image_offset, int32_t *flag_x, int32_t *flag_y, int32_t *flag_offset)
{
    int32_t side = edge_side(x, y);
    if (side < 0) {
        return;
    }
    for (int32_t radius = 1; radius < ENTRY_EXIT_FLAG_SEARCH_RADIUS; radius++) {
        if (map_terrain_find_free_tile(x, y, 1, radius, map_grid_offset(x, y))) {
            break;
        }
    }
    *flag_y = map_result_y;
    *flag_x = map_result_x;
    int32_t offset = map_grid_offset(map_result_x, map_result_y);
    *flag_offset = offset;
    map_grids.terrain[offset] |= TERRAIN_ROCK;
    int32_t direction = city_view_orientation + side;
    if (direction >= DIRECTION_COUNT) {
        direction -= DIRECTION_COUNT;
    }
    map_grids.images[offset] = (direction / 2) + image_offset + image_group(GROUP_TERRAIN_ENTRY_EXIT_FLAGS);
}

void map_tiles_update_entry_exit_flags(int32_t remove)
{
    if (remove) {
        map_grids.terrain[tile_offset(city_entry_flag_x, city_entry_flag_y)] &= ~TERRAIN_ROCKS;
        map_grids.terrain[tile_offset(city_exit_flag_x, city_exit_flag_y)] &= ~TERRAIN_ROCKS;
        return;
    }
    place_flag(scenario_data.entry_point_x, scenario_data.entry_point_y, ENTRY_FLAG_IMAGE, &city_entry_flag_x, &city_entry_flag_y, &city_entry_flag_offset);
    place_flag(scenario_data.exit_point_x, scenario_data.exit_point_y, EXIT_FLAG_IMAGE, &city_exit_flag_x, &city_exit_flag_y, &city_exit_flag_offset);
}

void map_orientation_update_corner_tiles(void)
{
    int32_t orientation = city_view_orientation;
    for (int32_t y = 0; y < map_info.grid_height; y++) {
        int32_t offset = map_row_start_offset[y];
        for (int32_t x = map_row_x_start[y]; x < map_row_x_end[y]; x++, offset++) {
            int32_t size = map_grids.bitfields[offset] & MAP_BITFIELD_SIZE;
            uint8_t edge = map_grids.edge[offset];
            int32_t position = edge & (MAP_EDGE_ROW | MAP_EDGE_COLUMN);
            if (size == 0) {
                map_grids.edge[offset] = edge | MAP_EDGE_DRAW_TILE;
                continue;
            }
            edge &= ~MAP_EDGE_DRAW_TILE;
            map_grids.edge[offset] = edge;
            if (size > 5) {
                continue;
            }
            int32_t corner;
            switch (orientation) {
                case DIRECTION_TOP:
                    corner = size * MAP_EDGE_ROW_ONE;
                    break;
                case DIRECTION_RIGHT:
                    corner = 0;
                    break;
                case DIRECTION_BOTTOM:
                    corner = size;
                    break;
                case DIRECTION_LEFT:
                    corner = (size * MAP_EDGE_ROW_ONE) + size;
                    break;
                default:
                    corner = -1;
                    break;
            }
            if (position == corner) {
                map_grids.edge[offset] = edge | MAP_EDGE_DRAW_TILE;
            }
        }
    }
}

void map_orientation_rotate_bridges(void)
{
    int32_t left = rotation_direction == 1;
    for (int32_t y = 0; y < map_info.grid_height; y++) {
        int32_t offset = map_row_start_offset[y];
        for (int32_t x = map_row_x_start[y]; x < map_row_x_end[y]; x++, offset++) {
            uint8_t *sprite = &map_grids.sprite[offset];
            if (!(map_grids.terrain[offset] & TERRAIN_WATER) || *sprite == 0) {
                continue;
            }
            switch (*sprite) {
                case BRIDGE_SPRITE_LOW_RAMP_DOWN_LEFT:
                    *sprite = left ? BRIDGE_SPRITE_LOW_RAMP_DOWN_TOP : BRIDGE_SPRITE_LOW_RAMP_DOWN_BOTTOM;
                    break;
                case BRIDGE_SPRITE_LOW_SPAN_ALONG_X:
                    *sprite = BRIDGE_SPRITE_LOW_SPAN_ALONG_Y;
                    break;
                case BRIDGE_SPRITE_LOW_RAMP_DOWN_RIGHT:
                    *sprite = left ? BRIDGE_SPRITE_LOW_RAMP_DOWN_BOTTOM : BRIDGE_SPRITE_LOW_RAMP_DOWN_TOP;
                    break;
                case BRIDGE_SPRITE_LOW_RAMP_DOWN_BOTTOM:
                    *sprite = left ? BRIDGE_SPRITE_LOW_RAMP_DOWN_LEFT : BRIDGE_SPRITE_LOW_RAMP_DOWN_RIGHT;
                    break;
                case BRIDGE_SPRITE_LOW_SPAN_ALONG_Y:
                    *sprite = BRIDGE_SPRITE_LOW_SPAN_ALONG_X;
                    break;
                case BRIDGE_SPRITE_LOW_RAMP_DOWN_TOP:
                    *sprite = left ? BRIDGE_SPRITE_LOW_RAMP_DOWN_RIGHT : BRIDGE_SPRITE_LOW_RAMP_DOWN_LEFT;
                    break;
                default:
                    break;
            }
        }
    }
}

void map_orientation_change(void)
{
    map_tiles_update_entry_exit_flags(1);
    undo_available = 0;
    map_orientation_update_corner_tiles();
    map_tiles_update_all();
    map_orientation_update_buildings();
    map_orientation_rotate_bridges();
    map_routing_update_walls();
    figure_tower_sentry_reroute();
    city_view_calculate_map_bounds();
}
