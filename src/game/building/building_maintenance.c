// Building maintenance: road and kingdom access, burning ruins and the spread of fire, fire and collapse risk,
// and the criminals of unhappy houses; with the helpers that burn a building down, turn linked parts to rubble
// and remove the last placed building.
//
// Kept as the game does it, though each looks like a bug:
// - a small house never gets its own ruin image, as the house size is read after it is cleared;
// - the sphinx takes its road access from the tile left of its grid offset;
// - tomb robbers are possible by a check on the fifth character of mission text 307.

#include "building/building_maintenance.h"
#include "building/building.h"
#include "building/building_create.h"
#include "building/building_delete.h"
#include "building/building_figure_helpers.h"
#include "building/building_image_group.h"
#include "building/building_type.h"
#include "building/caesareum.h"
#include "building/construction_venue.h"
#include "building/destroy_enemy.h"
#include "building/destroy_paths.h"
#include "building/gatehouse_place.h"
#include "building/house_evolve.h"
#include "building/house_population.h"
#include "building/house_size.h"
#include "building/library.h"
#include "building/lighthouse.h"
#include "building/mausoleum.h"
#include "building/monument.h"
#include "building/monument_access.h"
#include "building/monument_work_spots.h"
#include "building/royal_tomb.h"
#include "city/message.h"
#include "city/population.h"
#include "city/resource_type.h"
#include "core/crt_compat.h"
#include "core/random.h"
#include "figure/figure.h"
#include "figure/figure_action.h"
#include "figure/figure_type.h"
#include "figure/military.h"
#include "game/game_data.h"
#include "game/game_internal.h"
#include "game/tutorial.h"
#include "graphics/image_collection.h"
#include "map/building_tiles.h"
#include "map/grid.h"
#include "map/map_data.h"
#include "map/road_access.h"
#include "map/routing_citizen.h"
#include "map/routing_terrain.h"
#include "map/terrain.h"
#include "map/tiles.h"
#include "scenario/scenario_climate.h"
#include "sound/sound_play.h"
#include "text/lang.h"
#include "widget/city_view.h"
#include "widget/widget_data.h"
#include <stdbool.h>
#include <stdint.h>

static const float CRIMINALS_PER_CITIZEN = 0.02f;

int32_t fire_spread_direction;

enum maintenance_limit {
    BURNING_LIST_MAX = 500,
    MAX_LINKED_PARTS = 525,
    BUILDING_RISK_MAX = 1000,
    RUIN_BURN_DAYS = 32,
    KINGDOM_ROAD_REPAIRS = 15,
    HOUSE_NO_ROAD_DAYS = 4,
    LOT_UNCONNECTED_DAYS = 8,
    TEMPLE_COMPLEX_ACCESS_SIZE = 3,
    FIRE_RISK_DAY_MASK = 7,
    FIRE_RISK_FACTORS = 5,
    DESERT_FIRE_RISK_BONUS = 3,
    BURNING_DAMAGE_FACTOR = 10,
    CRIMINAL_DAYS = 10,
    TOMB_ROBBERS_PER_DAY = 3,
    TOMB_ROBBER_COUNTERS = 35,
    MISSION_TEXT_GROUP = 307,
    CRIMINAL_WAIT_MASK = 0xF,
    CRIMINAL_WAIT_BASE = 10,
    FIRE_DIRECTION_MASK = 7,
};

enum maintenance_message {
    MESSAGE_SOUND_INTERVAL = 15000,
    MESSAGE_DELAY = 12,
};

static int32_t is_venue_or_festival_square(int32_t type)
{
    return type == BUILDING_BOOTH || type == BUILDING_BANDSTAND || type == BUILDING_PAVILION || type == BUILDING_FESTIVAL_SQUARE;
}

static int32_t is_water_building(int32_t type)
{
    return type == BUILDING_DOCK || type == BUILDING_FISHING_WHARF || type == BUILDING_SHIPWRIGHT || type == BUILDING_TRANSPORT_WHARF ||
        type == BUILDING_WARSHIP_WHARF || type == BUILDING_DOCK_UP;
}

static int32_t ruin_image(int32_t small_house, int32_t grid_offset)
{
    int32_t variant = map_grids.random[grid_offset] & (BURNING_RUIN_VARIANTS - 1);
    if (small_house) {
        return image_group(IMAGE_GROUP_RUIN_SMALL_HOUSE);
    }
    return image_group(IMAGE_GROUP_RUINS) + (variant * BURNING_RUIN_IMAGE_STEP);
}

static int32_t extra_ruin_tiles(int32_t size)
{
    return (size >= 2 && size <= 6) ? (size * size) : 0;
}

static void start_burning(building *ruin, int32_t plague)
{
    ruin->figure_id4 = 0;
    ruin->fire_duration = (int16_t) ((ruin->house_figure_generation_delay & BURNING_RUIN_FIRE_DAYS_MASK) + 1);
    ruin->fire_proof = 1;
    ruin->ruin_has_plague = (uint8_t) plague;
}

void building_destroy_by_fire(int32_t building_id, int32_t plague)
{
    building *b = building_get(building_id);
    if (b->type == BUILDING_GATEHOUSE_UP) {
        building_gatehouse_destroy_by_fire(building_id);
        return;
    }
    b->fire_risk = 0;
    b->damage_risk = 0;
    undo_available = 0;
    if (b->house_size && b->house_population != 0) {
        city_population_remove_home_removed(city_current_player, b->house_population);
    }
    b->house_population = 0;
    b->house_size = 0;
    b->output_resource = RESOURCE_NONE;
    b->distance_from_entry = 0;
    building_clear_related_data(building_id);
    if (is_venue_or_festival_square(b->type)) {
        int32_t main_id = building_main_part_id(building_id);
        if (main_id > 0 && main_id == building_id) {
            venue_remove_tiles(main_id, building_get(main_id)->type);
        }
    }
    int32_t water_routing = is_water_building(b->type);
    int32_t small_house = b->house_size && b->subtype <= 1;
    int32_t tiles = extra_ruin_tiles(b->size);
    map_building_tiles_remove(building_id, b->x, b->y);
    if (map_grids.terrain[b->grid_offset] & TERRAIN_WATER) {
        b->state = BUILDING_STATE_DELETED_BY_GAME;
    } else {
        b->type = BUILDING_BURNING_RUIN;
        b->figure_id4 = 0;
        b->fire_duration = (int16_t) ((b->house_figure_generation_delay & BURNING_RUIN_FIRE_DAYS_MASK) + 1);
        b->fire_proof = 1;
        b->size = 1;
        b->ruin_has_plague = (uint8_t) plague;
        map_building_tiles_add(building_id, b->x, b->y, 1, 1, ruin_image(small_house, b->grid_offset), TERRAIN_BUILDING);
    }
    for (int32_t i = 0; i < tiles - 1; i++) {
        int32_t x = b->x + gatehouse_rubble_offsets[2 * (i + 1)];
        int32_t y = b->y + gatehouse_rubble_offsets[(2 * (i + 1)) + 1];
        if (map_grids.terrain[map_grid_offset(x, y)] & TERRAIN_WATER) {
            continue;
        }
        int32_t ruin_id = building_create(city_current_player, BUILDING_BURNING_RUIN, x, y, 0, 0);
        building *ruin = building_get(ruin_id);
        map_building_tiles_add(ruin_id, ruin->x, ruin->y, 1, 1, ruin_image(small_house, ruin->grid_offset), TERRAIN_BUILDING);
        start_burning(ruin, plague);
    }
    if (water_routing) {
        map_routing_update_water();
    }
}

static void destroy_part(int32_t part_id, int32_t on_fire)
{
    if (on_fire) {
        building_destroy_by_fire(part_id, 0);
    } else {
        building *part = building_get(part_id);
        map_building_tiles_set_rubble(part_id, part->x, part->y, part->size, part->size);
        part->state = BUILDING_STATE_RUBBLE;
    }
}

void building_destroy_linked_parts(int32_t building_id, int32_t on_fire)
{
    int32_t part_id = building_id;
    for (int32_t i = 0; i < MAX_LINKED_PARTS; i++) {
        part_id = building_get(part_id)->prev_part_building_id;
        if (part_id <= 0) {
            break;
        }
        destroy_part(part_id, on_fire);
    }
    part_id = building_id;
    for (int32_t i = 0; i < MAX_LINKED_PARTS; i++) {
        part_id = building_get(part_id)->next_part_building_id;
        if (part_id <= 0) {
            break;
        }
        destroy_part(part_id, on_fire);
    }
}

void building_destroy_multipart_by_fire(int32_t building_id)
{
    int32_t part_id = building_main(building_id);
    for (;;) {
        building *part = building_get(part_id);
        part->fire_duration = 1;
        part->fire_risk = BUILDING_RISK_MAX;
        building_destroy_by_fire(part_id, 0);
        if (part->next_part_building_id < 1) {
            break;
        }
        part_id = part->next_part_building_id;
    }
}

void building_gatehouse_set_rubble(int32_t building_id)
{
    building *b = building_get(building_id);
    int32_t orientation = b->data.oriented.orientation + map_info.orientation;
    if (orientation >= 8) {
        orientation -= 8;
    }
    const int32_t *parts = &gatehouse_orientation_parts[orientation * GATEHOUSE_PART_COUNT];
    for (int32_t i = 0; i < GATEHOUSE_PART_COUNT; i++) {
        const gatehouse_part *part = &gatehouse_parts.parts[parts[i]];
        int32_t x;
        int32_t y;
        switch (b->data.oriented.orientation) {
            case 0:
                x = b->x + part->dx;
                y = b->y + part->dy;
                break;
            case 1:
                x = b->x + part->dy;
                y = b->y + part->dx;
                break;
            default:
                x = b->x;
                y = b->y;
                break;
        }
        map_building_tiles_set_rubble(building_id, x, y, part->size, part->size);
    }
}

static void collapse(int32_t building_id, building *b)
{
    b->state = BUILDING_STATE_RUBBLE;
    undo_available = 0;
    if (b->type == BUILDING_GATEHOUSE_UP) {
        building_gatehouse_set_rubble(building_id);
    } else {
        map_building_tiles_set_rubble(building_id, b->x, b->y, b->size, b->size);
    }
}

void building_maintenance_destroy_last_placed(void)
{
    int32_t highest_sequence = 0;
    int32_t last_id = 0;
    for (int32_t id = 1; id <= building_highest_id; id++) {
        building *b = building_get(id);
        if ((b->state == BUILDING_STATE_IN_USE || b->state == BUILDING_STATE_CREATED) && (int16_t) b->created_sequence > highest_sequence) {
            highest_sequence = (int16_t) b->created_sequence;
            last_id = id;
        }
    }
    if (!last_id) {
        return;
    }
    int32_t id = building_main(last_id);
    building *b = building_get(id);
    city_message_use_popup = 1;
    city_message_post(MESSAGE_LAST_BUILDING_DESTROYED, 0, b->grid_offset, 0, 0);
    collapse(id, b);
    if (is_venue_or_festival_square(b->type)) {
        venue_remove_tiles(id, b->type);
    }
    figure_create_explosion_cloud(b->x, b->y, b->size, 1);
    building_destroy_linked_parts(id, 0);
    map_routing_update_land_citizen_rect(b->x - b->size, b->y - b->size, b->x + b->size, b->y + b->size, 1);
    map_routing_update_land_noncitizen_rect(b->x - b->size, b->y - b->size, b->x + b->size, b->y + b->size);
}

static void set_road_access(building *b, int32_t offset)
{
    b->road_network_id = map_grids.road_network[offset];
    b->distance_from_entry = (int16_t) routing_grids.distance[offset];
    b->road_access_x = (int16_t) map_result_x;
    b->road_access_y = (int16_t) map_result_y;
}

static void set_road_access_at_result(building *b)
{
    int32_t x = map_result_x;
    int32_t y = map_result_y;
    int32_t offset = map_grid_offset(x, y);
    b->road_network_id = map_grids.road_network[offset];
    b->distance_from_entry = (int16_t) routing_grids.distance[offset];
    b->road_access_x = (int16_t) x;
    b->road_access_y = (int16_t) y;
}

static void set_road_access_if_found(building *b, uint8_t found)
{
    if (found) {
        set_road_access_at_result(b);
    }
}

static void set_road_access_around(building *b, int32_t offset)
{
    if (offset) {
        set_road_access(b, offset - 1);
    }
}

static int32_t update_house_access(building *b, int32_t unconnected_house)
{
    if (!map_closest_road_within_radius(b->x, b->y, b->size, HOUSE_ROAD_RADIUS)) {
        b->distance_from_entry = 0;
        b->unreachable_ticks++;
        if (b->unreachable_ticks > HOUSE_NO_ROAD_DAYS) {
            if (b->house_population) {
                figure_create_homeless(city_current_player, b->x, b->y, b->house_population);
                b->house_population = 0;
                b->unreachable_ticks = 0;
            }
            b->state = BUILDING_STATE_UNDO;
        }
    } else if (routing_grids.distance[map_tile_cursor_offset] == 0) {
        b->distance_from_entry = 0;
        int32_t empty_lot = b->type == BUILDING_HOUSE_CRUDE_HUT && b->house_population == 0;
        if (b->unreachable_ticks == 0 && (empty_lot || unconnected_house == 0)) {
            unconnected_house = b->grid_offset;
        }
        b->unreachable_ticks++;
        if (b->unreachable_ticks > LOT_UNCONNECTED_DAYS) {
            b->unreachable_ticks = 0;
            if (empty_lot) {
                b->state = BUILDING_STATE_UNDO;
            }
        }
    } else {
        b->distance_from_entry = (int16_t) routing_grids.distance[map_tile_cursor_offset];
        b->unreachable_ticks = 0;
    }
    return unconnected_house;
}

static void update_building_access(building *b, int32_t id)
{
    city_player *player = city_player_current();
    b->distance_from_entry = 0;
    switch (b->type) {
        case BUILDING_STORAGE_YARD:
        case BUILDING_STORAGE_YARD_UP:
            if (player->trade_center_building_id == 0) {
                player->trade_center_building_id = id;
            }
            b->distance_from_entry = 0;
            set_road_access_around(b, map_reachable_road_around(b->x, b->y, STORAGE_YARD_SIZE));
            return;
        case BUILDING_SPHINX:
            set_road_access_around(b, map_sphinx_access_tile(b->x, b->y));
            return;
        case BUILDING_SUN_TEMPLE:
            if (sun_temple_get_access_tile(id, &map_result_x, &map_result_y)) {
                b->road_access_x = (int16_t) map_result_x;
                b->road_access_y = (int16_t) map_result_y;
            }
            return;
        case BUILDING_ALEXANDRIA_LIBRARY:
            set_road_access_if_found(b, library_get_work_tile(&library_object.base, id, &map_result_x, &map_result_y));
            return;
        case BUILDING_ABU_SIMBEL:
            set_road_access_if_found(b, abu_simbel_get_work_tile(&abu_simbel_object.base, id, &map_result_x, &map_result_y));
            return;
        case BUILDING_CAESAREUM:
            set_road_access_if_found(b, caesareum_get_work_tile(&caesareum_object.base, id, &map_result_x, &map_result_y));
            return;
        case BUILDING_LIGHTHOUSE:
            set_road_access_if_found(b, lighthouse_get_work_tile(&lighthouse_object.base, id, &map_result_x, &map_result_y));
            return;
        case BUILDING_MAUSOLEUM:
            mausoleum_get_work_tile(id, &map_result_x, &map_result_y);
            b->road_access_x = (int16_t) map_result_x;
            b->road_access_y = (int16_t) map_result_y;
            return;
        case BUILDING_SMALL_ROYAL_TOMB:
            set_road_access_if_found(b, small_royal_tomb_get_access_tile(&small_royal_tomb_object.base, id, &map_result_x, &map_result_y));
            return;
        case BUILDING_MEDIUM_ROYAL_TOMB:
            set_road_access_if_found(b, medium_royal_tomb_get_entry_tile(&medium_royal_tomb_object.base, id, &map_result_x, &map_result_y));
            return;
        case BUILDING_LARGE_ROYAL_TOMB:
            set_road_access_if_found(b, large_royal_tomb_get_entry_tile(&large_royal_tomb_object.base, id, &map_result_x, &map_result_y));
            return;
        case BUILDING_GRAND_ROYAL_TOMB:
            set_road_access_if_found(b, grand_royal_tomb_get_entry_tile(&grand_royal_tomb_object.base, id, &map_result_x, &map_result_y));
            return;
        case BUILDING_TEMPLE_COMPLEX_OSIRIS:
        case BUILDING_TEMPLE_COMPLEX_RA:
        case BUILDING_TEMPLE_COMPLEX_PTAH:
        case BUILDING_TEMPLE_COMPLEX_SETH:
        case BUILDING_TEMPLE_COMPLEX_BAST:
        case BUILDING_TEMPLE_COMPLEX_GOD_5:
        case BUILDING_TEMPLE_COMPLEX_GOD_6:
        case BUILDING_TEMPLE_COMPLEX_GOD_7:
        case BUILDING_TEMPLE_COMPLEX_GOD_8:
        case BUILDING_TEMPLE_COMPLEX_GOD_9:
            set_road_access_around(b, map_reachable_road_triple(b->x, b->y, TEMPLE_COMPLEX_ACCESS_SIZE));
            return;
        case BUILDING_STORAGE_ROOM: {
            building *yard = building_get(building_main(id));
            b->road_network_id = yard->road_network_id;
            b->distance_from_entry = yard->distance_from_entry;
            b->road_access_x = yard->road_access_x;
            b->road_access_y = yard->road_access_y;
            return;
        }
        case BUILDING_PYRAMID: {
            int32_t entry_offset;
            pyramid_get_entry_tile(id, &entry_offset, &map_result_x, &map_result_y);
            set_road_access(b, entry_offset);
            return;
        }
        default:
            set_road_access_around(b, map_reachable_road_around(b->x, b->y, b->size));
            return;
    }
}

static void update_routing_after_repair(void)
{
    map_routing_update_land_citizen();
    map_routing_update_land_noncitizen();
    map_routing_update_walls();
    map_tiles_update_all_walls();
    map_tiles_update_region_canals(0, 0, map_info.grid_width - 1, map_info.grid_height - 1, 0);
    map_tiles_update_region_empty_land(0, 0, map_info.grid_width - 1, map_info.grid_height - 1, 0, 0);
    map_tiles_update_region_meadow(0, 0, map_info.grid_width - 1, map_info.grid_height - 1);
    map_tiles_update_all_gardens();
}

void building_maintenance_check_kingdom_access(void)
{
    const city_player *player = city_player_current();
    int32_t unconnected_house = 0;
    map_routing_citizen_can_travel_over_land(player->entry_point_x, player->entry_point_y, -2, -2);
    for (int32_t id = 1; id <= building_highest_id; id++) {
        building *b = building_get(id);
        if (b->state != BUILDING_STATE_IN_USE) {
            continue;
        }
        if (b->house_size) {
            unconnected_house = update_house_access(b, unconnected_house);
        } else {
            update_building_access(b, id);
        }
    }
    if (routing_grids.distance[player->exit_point_grid_offset] != 0) {
        if (unconnected_house) {
            city_warning_show(WARNING_CITY_ISOLATED_FROM_KINGDOM_ROAD);
            city_warning_show(WARNING_RESTORE_ACCESS_OR_SECTOR_WILL_STAGNATE);
            city_view_go_to_grid_offset(unconnected_house);
        }
        return;
    }
    for (int32_t i = 0; i < KINGDOM_ROAD_REPAIRS; i++) {
        map_routing_delete_first_wall_or_canal(player->entry_point_x, player->entry_point_y, -2, -2);
        map_routing_delete_first_wall_or_canal(player->exit_point_x, player->exit_point_y, -2, -2);
        update_routing_after_repair();
        map_routing_citizen_can_travel_over_land(player->entry_point_x, player->entry_point_y, -2, -2);
        if (routing_grids.distance[player->exit_point_grid_offset] != 0) {
            city_message_use_popup = 1;
            city_message_post(MESSAGE_ROAD_TO_KINGDOM_RESTORED, 0, 0, 0, 0);
            undo_available = 0;
            return;
        }
    }
    building_maintenance_destroy_last_placed();
}

static int32_t can_catch_fire(int32_t building_id)
{
    return building_id != 0 && building_get(building_id)->fire_proof == 0 && building_is_not_monument(building_id);
}

static int32_t fire_spread_target(const building *ruin)
{
    int32_t direction = fire_spread_direction;
    int32_t left = direction - 1;
    if (left < 0) {
        left = DIRECTION_TOP_LEFT;
    }
    int32_t right = direction + 1;
    if (right > DIRECTION_TOP_LEFT) {
        right = DIRECTION_TOP;
    }
    int32_t target = map_grids.building_ids[map_grid_direction_delta[direction] + ruin->grid_offset];
    if (can_catch_fire(target)) {
        return target;
    }
    target = map_grids.building_ids[map_grid_direction_delta[left] + ruin->grid_offset];
    if (can_catch_fire(target)) {
        return target;
    }
    target = map_grids.building_ids[map_grid_direction_delta[right] + ruin->grid_offset];
    return can_catch_fire(target) ? target : 0;
}

void building_maintenance_update_burning_ruins(void)
{
    int32_t changed = 0;
    building_burning_size = 0;
    building_list_burning_index = 0;
    building_burning_total = 0;
    for (int32_t id = 1; id <= building_highest_id; id++) {
        building *b = building_get(id);
        if (b->state != BUILDING_STATE_IN_USE || b->type != BUILDING_BURNING_RUIN) {
            continue;
        }
        if (b->fire_duration < 0) {
            b->fire_duration = 0;
        }
        int16_t duration = ++b->fire_duration;
        if (duration > RUIN_BURN_DAYS) {
            b->state = BUILDING_STATE_RUBBLE;
            undo_available = 0;
            map_building_tiles_set_rubble(id, b->x, b->y, b->size, b->size);
            changed = 1;
            continue;
        }
        if (b->ruin_has_plague) {
            continue;
        }
        building_list_burning[building_list_burning_index] = (int16_t) id;
        building_burning_total++;
        building_list_burning_index++;
        if (building_list_burning_index >= BURNING_LIST_MAX) {
            building_list_burning_index = BURNING_LIST_MAX - 1;
        }
        if ((duration & ((scenario_data.climate_type == CLIMATE_DESERT) ? 3 : BURNING_RUIN_FIRE_DAYS_MASK)) != 0) {
            continue;
        }
        random_generate_next();
        if ((b->house_figure_generation_delay & 3) != (random_generator_state.random1_15bit & 3)) {
            continue;
        }
        int32_t target = fire_spread_target(b);
        if (!target) {
            continue;
        }
        building_destroy_by_fire(target, 0);
        building_destroy_linked_parts(target, 1);
        sound_effect_play(SOUND_EFFECT_FIRE_SPLASH);
        changed = 1;
    }
    if (changed) {
        map_routing_update_land_citizen();
        map_routing_update_land_noncitizen();
    }
}

int32_t building_is_checked_for_fire(const building *b, int32_t id)
{
    int32_t type = b->type;
    if (b->state != BUILDING_STATE_IN_USE || type == BUILDING_BURNING_RUIN || b->fire_duration != 0) {
        return 0;
    }
    if ((type == BUILDING_SENET_HOUSE || type == BUILDING_ZOO || type == BUILDING_SPHINX) && b->prev_part_building_id != 0) {
        return 0;
    }
    if (type == BUILDING_STORAGE_ROOM) {
        return 0;
    }
    if (building_type_is_temple_complex(type) && b->prev_part_building_id != 0) {
        return 0;
    }
    return building_main(id) == id;
}

int32_t building_burns_whole(int32_t type)
{
    return is_venue_or_festival_square(type) || building_type_is_temple_complex(type);
}

static void message_sound_interval(int32_t *last_time)
{
    int32_t now = time_current_millis;
    if (now - *last_time > MESSAGE_SOUND_INTERVAL) {
        *last_time = now;
    } else {
        city_message_posted_once = 0;
    }
}

static void post_fire_message(building *b)
{
    message_sound_interval(&message_fire_last_time);
    if (!tutorial_flags_a[TUTORIAL_FLAG_FIRE]) {
        tutorial_set_flag(TUTORIAL_FLAG_FIRE);
        return;
    }
    message_sound_interval(&message_fire_last_time);
    if (city_message_delay[MESSAGE_CATEGORY_FIRE] <= 0) {
        city_message_use_popup = 1;
        city_message_delay[MESSAGE_CATEGORY_FIRE] = MESSAGE_DELAY;
    }
    city_message_post(MESSAGE_FIRE, b->type, b->grid_offset, 0, 0);
    city_message_count[MESSAGE_CATEGORY_FIRE]++;
}

static void post_collapse_message(building *b)
{
    message_sound_interval(&message_collapse_last_time);
    if (!tutorial_flags_a[TUTORIAL_FLAG_COLLAPSE]) {
        tutorial_set_flag(TUTORIAL_FLAG_COLLAPSE);
        return;
    }
    if (city_message_delay[MESSAGE_CATEGORY_COLLAPSE] <= 0) {
        city_message_use_popup = 1;
        city_message_delay[MESSAGE_CATEGORY_COLLAPSE] = MESSAGE_DELAY;
    }
    city_message_post(MESSAGE_COLLAPSE, b->type, b->grid_offset, 0, 0);
    city_message_count[MESSAGE_CATEGORY_COLLAPSE]++;
}

static void add_damage_risk(building *b)
{
    int16_t damage = (int16_t) building_model_table[b->type].damage_risk_increment;
    int16_t old_damage = b->damage_risk;
    b->damage_risk = (int16_t) (damage + old_damage);
    if (b->fire_duration != 0) {
        b->damage_risk = (int16_t) (((int16_t) building_model_table[b->type].damage_risk_increment * BURNING_DAMAGE_FACTOR) + damage + old_damage);
    }
    if (b->house_size && b->subtype <= 1) {
        b->damage_risk = 0;
    }
    if (map_grids.terrain[b->grid_offset] & TERRAIN_FLOODPLAIN) {
        b->damage_risk = 0;
    }
}

static void add_fire_risk(building *b, int16_t factor)
{
    int16_t model_fire = (int16_t) building_model_table[b->type].fire_risk_increment;
    if (b->house_size) {
        if (b->house_population <= 0) {
            b->fire_risk = 0;
        } else {
            b->fire_risk = (int16_t) (b->fire_risk + (int16_t) (model_fire * factor));
        }
        return;
    }
    if (scenario_data.climate_type == CLIMATE_DESERT && building_model_table[b->type].fire_risk_increment != 0) {
        b->fire_risk = (int16_t) (b->fire_risk + DESERT_FIRE_RISK_BONUS);
    }
    b->fire_risk = (int16_t) (b->fire_risk + (int16_t) ((int16_t) building_model_table[b->type].fire_risk_increment * factor));
    if (building_model_table[b->type].fire_risk_increment == 0) {
        b->fire_risk = 0;
    }
}

void building_maintenance_check_fire_collapse(void)
{
    int32_t collapsed = 0;
    city_player *player = city_player_current();
    player->protesters = 0;
    player->thefts = 0;
    if (houses_frozen == 1) {
        return;
    }
    random_generate_next();
    int32_t random_day = random_generator_state.random1_3bit;
    for (int32_t id = 1; id <= building_highest_id; id++) {
        building *b = building_get(id);
        if (!building_is_checked_for_fire(b, id)) {
            continue;
        }
        int32_t random_building = (map_grids.random[b->grid_offset] + id) & FIRE_RISK_DAY_MASK;
        add_damage_risk(b);
        if (b->damage_risk >= BUILDING_RISK_MAX) {
            post_collapse_message(b);
            collapse(id, b);
            figure_create_explosion_cloud(b->x, b->y, b->size, 1);
            building_destroy_linked_parts(id, 0);
            collapsed = 1;
            continue;
        }
        random_generate_next();
        int16_t factor = (int16_t) ((random_generator_state.random1_15bit % FIRE_RISK_FACTORS) + 1);
        if (random_building != random_day) {
            continue;
        }
        add_fire_risk(b, factor);
        if (b->fire_risk < BUILDING_RISK_MAX) {
            continue;
        }
        b->fire_duration = 1;
        post_fire_message(b);
        if (building_burns_whole(b->type)) {
            building_destroy_multipart_by_fire(id);
        } else {
            b->fire_duration = 1;
            building_destroy_by_fire(id, 0);
            building_destroy_linked_parts(id, 1);
        }
    }
    if (collapsed) {
        map_routing_update_land_citizen();
        map_routing_update_land_noncitizen();
    }
}

static int32_t tomb_robber_counter(int32_t kind)
{
    switch (kind) {
        case 0:
        case 7:
        case 9:
        case 12:
        case 14:
        case 16:
        case 17:
        case 18:
        case 19:
        case 22:
        case 23:
        case 24:
        case 25:
        case 27:
        case 29:
            return 1;
        default:
            return 0;
    }
}

static int32_t is_tomb(int32_t type)
{
    return type == BUILDING_MAUSOLEUM || type == BUILDING_SMALL_ROYAL_TOMB || type == BUILDING_MEDIUM_ROYAL_TOMB || type == BUILDING_LARGE_ROYAL_TOMB ||
        type == BUILDING_GRAND_ROYAL_TOMB;
}

bool figure_criminal_tomb_robbers_possible(void)
{
    char c = lang_get_string(MISSION_TEXT_GROUP, settings_data.mission_id)[4];
    if (c == 'A' || c == 'V') {
        for (int32_t id = 1; id <= building_highest_id; id++) {
            const building *b = building_get(id);
            if (is_tomb(b->type) && (int16_t) b->created_sequence == -1) {
                return 1;
            }
        }
    }
    for (int32_t kind = 0; kind < TOMB_ROBBER_COUNTERS; kind++) {
        if (tomb_robber_counter(kind) && scenario_data.provisions_sent[kind + RESOURCE_MIN] > 0) {
            return 1;
        }
    }
    return 0;
}

static int32_t criminal_is_active(int32_t building_id, int32_t type)
{
    building *b = building_get(building_id);
    if (b->figure_id > 0) {
        figure *f = figure_get(b->figure_id);
        if (f->state != FIGURE_STATE_NONE && (f->type == type || f->type == FIGURE_NONE) && f->building_id == building_id) {
            return 1;
        }
        b->figure_id = 0;
    }
    return 0;
}

void figure_generate_criminal(int32_t building_id, int32_t tomb_robber_flag)
{
    int32_t tomb_robber = (uint8_t) tomb_robber_flag;
    building *b = building_get(building_id);
    if (criminal_is_active(building_id, FIGURE_CRIMINAL) || criminal_is_active(building_id, FIGURE_TOMB_ROBBER)) {
        return;
    }
    if (!map_closest_road_within_radius(b->x, b->y, b->size, HOUSE_ROAD_RADIUS)) {
        return;
    }
    int32_t figure_id = figure_create(b->player, tomb_robber ? FIGURE_TOMB_ROBBER : FIGURE_CRIMINAL, map_result_x, map_result_y, DIRECTION_BOTTOM);
    if (!figure_id) {
        return;
    }
    b->figure_id = (int16_t) figure_id;
    figure *f = figure_get(figure_id);
    f->action_state = tomb_robber ? FIGURE_ACTION_TOMB_ROBBER_WAITING : FIGURE_ACTION_CRIMINAL_WAITING;
    f->building_id = (int16_t) building_id;
    f->wait_ticks = (int16_t) ((b->house_figure_generation_delay & CRIMINAL_WAIT_MASK) + CRIMINAL_WAIT_BASE);
}

void figure_generate_criminals(void)
{
    criminal_day_counter++;
    if (criminal_day_counter < CRIMINAL_DAYS) {
        return;
    }
    criminal_day_counter = 0;
    int32_t budget = (int32_t) ((double) city_player_current()->population * (double) CRIMINALS_PER_CITIZEN);
    int32_t tomb_robbers = TOMB_ROBBERS_PER_DAY;
    for (int32_t id = 1; id <= building_highest_id; id++) {
        if (budget == 0) {
            return;
        }
        building *b = building_get(id);
        if (b->state != BUILDING_STATE_IN_USE || !b->house_size || b->house_population == 0 || b->crime_risk != BUILDING_RISK_MAX) {
            continue;
        }
        int32_t tomb_robber = figure_criminal_tomb_robbers_possible() && tomb_robbers != 0;
        figure_generate_criminal(id, tomb_robber);
        budget--;
        b->crime_risk = house_models[b->subtype].crime_base;
        if (tomb_robbers > 0) {
            tomb_robbers--;
        }
    }
}

void building_maintenance_update_fire_direction(void)
{
    fire_spread_direction = crt_rand() & FIRE_DIRECTION_MASK;
}
