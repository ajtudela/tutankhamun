// Work camp laborers and their sled trains: the camp's spawning of laborers, the laborer's task search (floodplain
// farms first outside the flood, else the monuments with work), the farm work, and the monument deliveries: fetching
// a load at a storage yard and pulling it on a sled, up the pyramid ramps too, to the mason who waits for it. The
// tunable values are laborer_rules from config/laborer_rules.json.
//
// Small slips in the laborer's states:
// - a farm task sets the destination y of figure_current_id;
// - the attack state advances the animation frame a second time;
// - states without a handler go idle;
// - the count of tiles leveled in a row is not reset when it runs out.

#include "figure/laborer.h"
#include "building/building.h"
#include "building/building_data.h"
#include "building/building_figure_helpers.h"
#include "building/building_industry.h"
#include "building/building_storage.h"
#include "building/building_type.h"
#include "building/caesareum.h"
#include "building/library.h"
#include "building/lighthouse.h"
#include "building/mausoleum.h"
#include "building/monument.h"
#include "building/monument_data.h"
#include "building/monument_internal.h"
#include "building/monument_spots.h"
#include "building/monument_tables.h"
#include "building/pyramid_blocks.h"
#include "building/pyramid_figure.h"
#include "building/pyramid_level.h"
#include "building/royal_tomb.h"
#include "building/sun_temple.h"
#include "city/city_data.h"
#include "city/labor.h"
#include "city/resource_type.h"
#include "core/calc.h"
#include "figure/boats.h"
#include "figure/figure.h"
#include "figure/figure_action.h"
#include "figure/figure_core.h"
#include "figure/figure_data.h"
#include "figure/figure_image_group.h"
#include "figure/figure_type.h"
#include "figure/military.h"
#include "figure/movement.h"
#include "flood/flood_types.h"
#include "game/game_data.h"
#include "graphics/image_collection.h"
#include "map/grid.h"
#include "map/map_data.h"
#include "map/road_access.h"
#include "map/routing.h"
#include "map/terrain.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

int32_t laborer_rules[LABORER_RULE_COUNT] = {
    [LABORER_RULE_LOAD] = 400,
    [LABORER_RULE_YARD_WAIT_TICKS] = 4,
    [LABORER_RULE_RAMP_ROUTE_RETRY_TICKS] = 30,
    [LABORER_RULE_RAMP_ROUTE_GIVE_UP_TICKS] = 120,
    [LABORER_RULE_DELIVERED_WAIT_TICKS] = 42,
    [LABORER_RULE_FERRY_WAIT_TICKS] = 100,
    [LABORER_RULE_MAX_ROAM_TICKS] = 512,
    [LABORER_RULE_FARM_WALK_TICKS] = 3200,
    [LABORER_RULE_HOME_ROAD_RADIUS] = 2,
    [LABORER_RULE_MAX_OUT] = 4,
    [LABORER_RULE_LABOR_SEEKER_HOUSES] = 50,
    [LABORER_RULE_SPAWN_DELAY_FULL] = 3,
    [LABORER_RULE_SPAWN_DELAY_75_PERCENT] = 7,
    [LABORER_RULE_SPAWN_DELAY_50_PERCENT] = 15,
    [LABORER_RULE_SPAWN_DELAY_25_PERCENT] = 29,
    [LABORER_RULE_SPAWN_DELAY_LOW] = 47,
    [LABORER_RULE_FARM_SEARCH_DISTANCE] = 40,
    [LABORER_RULE_FARM_BUSY_LABOR_DAYS] = 48,
    [LABORER_RULE_FARM_LABOR_DAYS] = 96,
    [LABORER_RULE_CAMP_LABOR_DAYS] = 32,
    [LABORER_RULE_LEVEL_TILES_IN_A_ROW] = 4,
    [LABORER_RULE_MONUMENT_WORK_SEARCHES] = 4,
};

static void clear_tomb_request(const figure *f)
{
    tomb_object *tomb = royal_tomb_object_for_type(building_get(f->destination_building_id)->type);
    if (tomb) {
        tomb->material_requested = 0;
    }
}

static int32_t part_count(int32_t size_variant)
{
    return scenario_pyramid_style == PYRAMID_STYLE_MASTABA ? mastaba_part_count[size_variant] : pyramid_part_count[size_variant];
}

int32_t figure_find_train_leader(int32_t figure_id, int32_t *position)
{
    int32_t id = figure_get(figure_id)->leading_figure_id;
    *position = 1;
    for (int32_t i = 0; i < LABORER_TRAIN_MAX_LINKS; i++) {
        const figure *link = figure_get(id);
        if (link->type == FIGURE_LABORER) {
            return id;
        }
        (*position)++;
        id = link->leading_figure_id;
    }
    return 0;
}

int32_t figure_laborer_partner_still_working(int32_t figure_id)
{
    figure *f = figure_get(figure_id);
    int32_t mason_id = f->leading_figure_id;
    if (mason_id == 0) {
        f->action_state = FIGURE_ACTION_LABORER_IDLE;
        return 0;
    }
    int32_t part_id = map_grids.building_ids[figure_get(mason_id)->destination_grid_offset];
    const building *part = building_get(part_id);
    int32_t active;
    switch (part->type) {
        case BUILDING_PYRAMID:
            active = pyramid_part_has_active_worker(part_id);
            break;
        case BUILDING_SUN_TEMPLE:
            active = sun_temple_part_has_active_worker(part_id);
            break;
        case BUILDING_ALEXANDRIA_LIBRARY:
            active = library_part_has_active_worker(&library_object.base, part_id);
            break;
        case BUILDING_CAESAREUM:
            active = caesareum_part_has_active_worker(&caesareum_object.base, part_id);
            break;
        case BUILDING_LIGHTHOUSE:
            active = lighthouse_part_has_active_worker(&lighthouse_object.base, part_id);
            break;
        case BUILDING_MAUSOLEUM:
            if (mausoleum_part_has_active_worker(part_id)) {
                return 1;
            }
            f->action_state = FIGURE_ACTION_LABORER_IDLE;
            return 0;
        default:
            f->action_state = FIGURE_ACTION_LABORER_IDLE;
            return 0;
    }
    if (active) {
        return 1;
    }
    int32_t new_mason_id = pyramid_find_mason_waiting_for_block(part_id, 1);
    if (new_mason_id != 0) {
        f->leading_figure_id = (int16_t) new_mason_id;
        figure_get(new_mason_id)->leading_figure_id = (int16_t) figure_id;
        return 1;
    }
    if (part->monument_remaining_blocks == 0) {
        f->action_state = FIGURE_ACTION_LABORER_IDLE;
    }
    return 0;
}

static void take_load(figure *f, int32_t yard_id)
{
    f->resource_amount = 0;
    int32_t resource = f->collecting_item_id;
    int32_t missing = building_storageyard_remove_resource(city_current_player, yard_id, resource, laborer_rules[LABORER_RULE_LOAD]);
    if (missing == laborer_rules[LABORER_RULE_LOAD]) {
        f->resource_amount = 0;
    } else if (missing == 0) {
        f->resource_amount = (int16_t) laborer_rules[LABORER_RULE_LOAD];
    } else {
        building_storageyard_add_resource(city_current_player, yard_id, resource, laborer_rules[LABORER_RULE_LOAD] - missing);
        f->resource_amount = 0;
    }
}

static void reset_new_train_figure(figure *t, int32_t partner_id)
{
    t->action_state = FIGURE_ACTION_LABORER_IDLE;
    t->leading_figure_id = (int16_t) partner_id;
    t->building_id = 0;
    t->wait_ticks = 0;
    t->roam_length = 0;
}

static void spawn_sled_train(int32_t figure_id, int32_t monument_id)
{
    int32_t player = building_get(monument_id)->player;
    const figure *f = figure_get(figure_id);
    int32_t previous_id = figure_id;
    int32_t side = 0;
    for (int32_t i = 0; i < LABORER_SLED_PULLERS; i++) {
        int32_t puller_id = figure_create(player, FIGURE_SLED_PULLER, f->x, f->y, 0);
        figure *puller = figure_get(puller_id);
        reset_new_train_figure(puller, previous_id);
        puller->resource_id = (uint8_t) side;
        side = 1 - side;
        previous_id = puller_id;
    }
    int32_t sled_id = figure_create(player, FIGURE_SLED, f->x, f->y, 0);
    figure *sled = figure_get(sled_id);
    sled->action_state = FIGURE_ACTION_LABORER_IDLE;
    sled->resource_id = f->collecting_item_id;
    sled->leading_figure_id = (int16_t) previous_id;
    sled->building_id = 0;
    sled->wait_ticks = 0;
    sled->roam_length = 0;
}

static void set_destination_road(figure *f, const building *b)
{
    f->destination_x = b->road_access_x;
    f->destination_y = b->road_access_y;
    f->destination_grid_offset = b->grid_offset;
}

static int32_t partner_part(int32_t figure_id)
{
    const figure *f = figure_get(figure_id);
    int32_t mason_id = f->leading_figure_id;
    if (mason_id == 0) {
        return 0;
    }
    const figure *mason = figure_get(mason_id);
    if (mason->leading_figure_id != figure_id) {
        return 0;
    }
    return map_grids.building_ids[mason->destination_grid_offset];
}

static void at_storage_yard_lamps(int32_t figure_id, figure *f, const building *home)
{
    take_load(f, f->destination_building_id);
    if (f->resource_amount == 0) {
        f->resource_id = RESOURCE_NONE;
        f->action_state = FIGURE_ACTION_LABORER_IDLE;
        figure_route_remove(figure_id);
        f->animation_frame = 0;
        return;
    }
    figure_route_remove(figure_id);
    f->action_state = FIGURE_ACTION_LABORER_SLED_TO_TOMB;
    f->destination_building_id = f->tomb_building_id;
    set_destination_road(f, building_get(f->tomb_building_id));
    f->leading_figure_id = 0;
    f->wait_ticks = 0;
    f->roam_length = 0;
    int32_t sled_id = figure_create(home->player, FIGURE_SLED, f->x, f->y, 0);
    figure *sled = figure_get(sled_id);
    sled->action_state = FIGURE_ACTION_LABORER_IDLE;
    sled->resource_id = f->collecting_item_id;
    sled->leading_figure_id = (int16_t) figure_id;
    sled->building_id = 0;
    sled->wait_ticks = 0;
    sled->roam_length = 0;
    sled->source_x = 0;
    sled->source_y = 0;
    f->animation_frame = 0;
}

static void at_storage_yard(int32_t figure_id, figure *f, const building *home)
{
    f->is_ghost = 0;
    f->terrain_usage = TERRAIN_USAGE_PREFER_ROADS;
    if (f->collecting_item_id == RESOURCE_LAMPS) {
        at_storage_yard_lamps(figure_id, f, home);
        return;
    }
    int32_t part_id = partner_part(figure_id);
    if (part_id == 0) {
        figure_route_remove(figure_id);
        f->state = FIGURE_STATE_DEAD;
        return;
    }
    int32_t active;
    switch (building_get(part_id)->type) {
        case BUILDING_PYRAMID:
            active = pyramid_part_has_active_worker(part_id);
            break;
        case BUILDING_SUN_TEMPLE:
            active = sun_temple_part_has_active_worker(part_id);
            break;
        case BUILDING_MAUSOLEUM:
            active = mausoleum_part_has_active_worker(part_id);
            break;
        case BUILDING_ALEXANDRIA_LIBRARY:
            active = library_part_has_active_worker(&library_object.base, part_id);
            break;
        case BUILDING_CAESAREUM:
            active = caesareum_part_has_active_worker(&caesareum_object.base, part_id);
            break;
        case BUILDING_LIGHTHOUSE:
            active = lighthouse_part_has_active_worker(&lighthouse_object.base, part_id);
            break;
        default:
            active = 1;
            break;
    }
    if (!active) {
        figure_route_remove(figure_id);
        f->state = FIGURE_STATE_DEAD;
        return;
    }
    int32_t yard_id = f->destination_building_id;
    f->wait_ticks++;
    if (f->wait_ticks <= laborer_rules[LABORER_RULE_YARD_WAIT_TICKS]) {
        f->animation_frame = 0;
        return;
    }
    take_load(f, yard_id);
    if (f->resource_amount == 0) {
        f->resource_id = RESOURCE_NONE;
        f->action_state = FIGURE_ACTION_LABORER_IDLE;
        figure_route_remove(figure_id);
        f->animation_frame = 0;
        return;
    }
    figure_route_remove(figure_id);
    int32_t monument_id = map_grids.building_ids[figure_get(f->leading_figure_id)->destination_grid_offset];
    const building *monument = building_get(monument_id);
    switch (monument->type) {
        case BUILDING_PYRAMID: {
            f->action_state = FIGURE_ACTION_LABORER_SLED_AT_RAMP;
            f->wait_ticks = 0;
            f->destination_building_id = (int16_t) monument_id;
            pyramid_set_figure_destination_entry(figure_id, monument_id);
            int32_t at_entry = f->x == f->destination_x && f->y == f->destination_y;
            if (!at_entry) {
                figure_route_add(figure_id);
                if (f->routing_path_id <= 0) {
                    building_storageyard_add_resource(city_current_player, yard_id, f->collecting_item_id, laborer_rules[LABORER_RULE_LOAD]);
                    f->action_state = FIGURE_ACTION_LABORER_IDLE;
                    return;
                }
            }
            spawn_sled_train(figure_id, monument_id);
            if (at_entry) {
                f->action_state = FIGURE_ACTION_LABORER_CLIMB_RAMP;
                pyramid_laborer_pair_with_mason(figure_id);
                pyramid_figure_start_climb(figure_id);
            }
            break;
        }
        case BUILDING_SUN_TEMPLE:
        case BUILDING_CAESAREUM:
        case BUILDING_LIGHTHOUSE:
        case BUILDING_ALEXANDRIA_LIBRARY:
        case BUILDING_MAUSOLEUM:
            switch (monument->type) {
                case BUILDING_SUN_TEMPLE:
                    f->action_state = FIGURE_ACTION_LABORER_SLED_TO_SUN_TEMPLE;
                    break;
                case BUILDING_CAESAREUM:
                    f->action_state = FIGURE_ACTION_LABORER_SLED_TO_CAESAREUM;
                    break;
                case BUILDING_LIGHTHOUSE:
                    f->action_state = FIGURE_ACTION_LABORER_SLED_TO_LIGHTHOUSE;
                    break;
                case BUILDING_ALEXANDRIA_LIBRARY:
                    f->action_state = FIGURE_ACTION_LABORER_SLED_TO_LIBRARY;
                    break;
                default:
                    f->action_state = FIGURE_ACTION_LABORER_SLED_TO_MAUSOLEUM;
                    break;
            }
            set_destination_road(f, monument);
            f->roam_length = 0;
            spawn_sled_train(figure_id, monument_id);
            break;
        default:
            f->action_state = FIGURE_ACTION_LABORER_IDLE;
            break;
    }
    f->wait_ticks = 0;
    f->animation_frame = 0;
}

static void to_pyramid(int32_t figure_id, figure *f, int32_t speed)
{
    f->terrain_usage = TERRAIN_USAGE_ROADS_OR_MONUMENT;
    f->is_ghost = 0;
    figure_movement_move_ticks(figure_id, speed);
    f->wait_ticks++;
    if (f->wait_ticks > LABORER_LEVEL_CHECK_TICKS) {
        f->wait_ticks = 0;
        int32_t part_id = map_grids.building_ids[f->destination_grid_offset];
        const building *part = building_get(part_id);
        if (part_id != 0 && part->type == BUILDING_PYRAMID && part->data.pyramid.stage > PYRAMID_STAGE_CASING) {
            int32_t x, y;
            if (!pyramid_find_level_tile(part_id - part->subtype, &x, &y, 1)) {
                f->action_state = FIGURE_ACTION_LABORER_IDLE;
            }
        }
    }
    switch (f->direction) {
        case DIRECTION_AT_DESTINATION: {
            int32_t part_id = map_grids.building_ids[f->destination_grid_offset];
            const building *part = building_get(part_id);
            if (part_id == 0 || part->type != BUILDING_PYRAMID) {
                f->action_state = FIGURE_ACTION_LABORER_IDLE;
            } else if (part->data.pyramid.stage > PYRAMID_STAGE_CASING) {
                f->action_state = pyramid_laborer_take_level_tile(figure_id) ? FIGURE_ACTION_LABORER_TO_LEVEL_TILE : FIGURE_ACTION_LABORER_IDLE;
            } else {
                f->action_state = FIGURE_ACTION_LABORER_CLIMB_RAMP;
                pyramid_laborer_pair_with_mason(figure_id);
                pyramid_figure_start_climb(figure_id);
            }
            break;
        }
        case DIRECTION_LOST:
            f->state = FIGURE_STATE_DEAD;
            break;
        case DIRECTION_REROUTE:
            figure_route_remove(figure_id);
            break;
    }
}

static void climb_ramp(int32_t figure_id, figure *f)
{
    f->is_ghost = 0;
    int32_t mason_id = f->leading_figure_id;
    int32_t part_id = mason_id != 0 ? map_grids.building_ids[figure_get(mason_id)->destination_grid_offset] : 0;
    if (part_id == 0 || !pyramid_part_has_active_worker(part_id)) {
        int32_t own_part_id = f->pyramid_work.part;
        int32_t new_mason_id = pyramid_find_mason_waiting_for_block(own_part_id, 1);
        if (new_mason_id != 0) {
            f->leading_figure_id = (int16_t) new_mason_id;
            figure_get(new_mason_id)->leading_figure_id = (int16_t) figure_id;
        } else if (building_get(own_part_id)->monument_remaining_blocks != 0) {
            return;
        } else {
            f->action_state = FIGURE_ACTION_LABORER_IDLE;
            f->pyramid_walk_mode = 0;
        }
    }
    pyramid_figure_walk_on_ramp(figure_id, 2, FIGURE_ACTION_LABORER_IDLE, FIGURE_ACTION_LABORER_TO_MASON);
    if (f->action_state == FIGURE_ACTION_LABORER_IDLE) {
        f->pyramid_walk_mode = 0;
    }
}

static void walk_to_mason(int32_t figure_id, figure *f)
{
    f->is_ghost = 0;
    if (pyramid_figure_climb_step(figure_id, 1)) {
        int32_t mason_id = f->leading_figure_id;
        figure *mason = figure_get(mason_id);
        int32_t woken = 0;
        if (mason->type == FIGURE_STONEMASON && mason->action_state == FIGURE_ACTION_STONEMASON_WAIT_FOR_BLOCK) {
            mason->action_state = FIGURE_ACTION_STONEMASON_WORK;
            woken = 1;
        } else if (mason->type == FIGURE_BRICKLAYER && mason->action_state == FIGURE_ACTION_BRICKLAYER_WAIT_FOR_BLOCK) {
            mason->action_state = FIGURE_ACTION_BRICKLAYER_WORK;
            woken = 1;
        }
        if (woken) {
            mason->wait_ticks = 0;
            pyramid_laborer_deliver_block(mason_id);
        }
        f->action_state = FIGURE_ACTION_LABORER_DELIVERED;
        f->wait_ticks = 0;
    } else if (f->direction == DIRECTION_REROUTE) {
        figure_route_remove(figure_id);
    } else if (f->direction == DIRECTION_LOST) {
        f->action_state = FIGURE_ACTION_LABORER_IDLE;
    }
}

static void move_on_road(int32_t figure_id, figure *f, int32_t speed)
{
    f->is_ghost = 0;
    f->terrain_usage = TERRAIN_USAGE_ROADS_OR_MONUMENT;
    figure_movement_move_ticks(figure_id, speed);
}

static void wake_stonemason(figure *mason)
{
    mason->action_state = FIGURE_ACTION_STONEMASON_WORK;
    mason->wait_ticks = 0;
}

static void delivered(figure *f)
{
    f->action_state = FIGURE_ACTION_LABORER_DELIVERED;
    f->wait_ticks = 0;
}

static void after_failed_move(int32_t figure_id, figure *f)
{
    if (f->direction == DIRECTION_REROUTE) {
        figure_route_remove(figure_id);
    } else if (f->direction == DIRECTION_LOST) {
        f->action_state = FIGURE_ACTION_LABORER_IDLE;
    }
}

static void sled_to_monument(int32_t figure_id, figure *f, int32_t speed, int32_t is_mausoleum)
{
    move_on_road(figure_id, f, speed);
    if (f->direction == DIRECTION_AT_DESTINATION) {
        figure *mason = figure_get(f->leading_figure_id);
        if (mason->type == FIGURE_STONEMASON && mason->action_state == FIGURE_ACTION_STONEMASON_WAIT_FOR_BLOCK) {
            wake_stonemason(mason);
        }
        delivered(f);
    } else if (is_mausoleum && f->direction == DIRECTION_REROUTE) {
        figure_route_remove(figure_id);
        f->roam_length = 0;
    } else {
        after_failed_move(figure_id, f);
    }
}

static void sled_to_counted_monument(int32_t figure_id, figure *f, int32_t speed, monument_object *object)
{
    move_on_road(figure_id, f, speed);
    if (f->direction != DIRECTION_AT_DESTINATION) {
        after_failed_move(figure_id, f);
        return;
    }
    figure *mason = figure_get(f->leading_figure_id);
    if (mason->type == FIGURE_STONEMASON && mason->action_state == FIGURE_ACTION_STONEMASON_WAIT_FOR_BLOCK) {
        object->frame++;
        if (object->frame >= object->frame_count) {
            object->frame = 0;
            wake_stonemason(mason);
        }
    }
    delivered(f);
}

static void walk_to_monument(int32_t figure_id, figure *f, int32_t speed, uint8_t work_action, int32_t lost_goes_idle_only)
{
    move_on_road(figure_id, f, speed);
    if (f->direction == DIRECTION_AT_DESTINATION) {
        f->action_state = work_action;
        f->wait_ticks = 0;
    } else if (lost_goes_idle_only) {
        after_failed_move(figure_id, f);
    } else if (f->direction == DIRECTION_REROUTE || f->direction == DIRECTION_LOST) {
        f->action_state = FIGURE_ACTION_LABORER_IDLE;
    }
}

static void monument_work(int32_t figure_id, figure *f, int32_t monument_type)
{
    f->is_ghost = 0;
    int32_t building_id = map_grids.building_ids[f->destination_grid_offset];
    if (building_id == 0) {
        f->action_state = FIGURE_ACTION_LABORER_IDLE;
        return;
    }
    switch (monument_type) {
        case BUILDING_SUN_TEMPLE:
            sun_temple_figure_work_tick(building_id, figure_id, FIGURE_ACTION_LABORER_IDLE);
            break;
        case BUILDING_MAUSOLEUM:
            mausoleum_figure_work_tick(building_id, figure_id, FIGURE_ACTION_LABORER_IDLE);
            break;
        default:
            library_figure_work_tick(&library_object.base, building_id, figure_id, FIGURE_ACTION_LABORER_IDLE);
            break;
    }
    if (f->action_state != FIGURE_ACTION_LABORER_IDLE) {
        return;
    }
    f->retry_count++;
    if (f->retry_count >= laborer_rules[LABORER_RULE_MONUMENT_WORK_SEARCHES]) {
        f->retry_count = 0;
        return;
    }
    switch (monument_type) {
        case BUILDING_SUN_TEMPLE:
            sun_temple_find_work_for_figure(figure_id);
            break;
        case BUILDING_MAUSOLEUM:
            mausoleum_find_work_for_figure(figure_id);
            break;
        default:
            library_find_work_for_figure(&library_object.base, f->x, f->y, figure_id);
            break;
    }
}

static void sled_to_tomb(int32_t figure_id, figure *f, int32_t speed)
{
    f->terrain_usage = TERRAIN_USAGE_ROADS_OR_MONUMENT;
    figure_movement_move_ticks(figure_id, speed);
    switch (f->direction) {
        case DIRECTION_AT_DESTINATION: {
            int32_t tomb_id = f->destination_building_id;
            const building *tomb = building_get(tomb_id);
            tomb_object *object = royal_tomb_object_for_type(tomb->type);
            if (object) {
                object->material_requested = 0;
                building_get(tomb_id - tomb->subtype)->data.royal_tomb.stock += laborer_rules[LABORER_RULE_LOAD];
                royal_tomb_update_part_images(object, tomb_id);
            }
            f->state = FIGURE_STATE_DEAD;
            break;
        }
        case DIRECTION_REROUTE:
            figure_route_remove(figure_id);
            f->state = FIGURE_STATE_DEAD;
            clear_tomb_request(f);
            break;
        case DIRECTION_LOST:
            f->state = FIGURE_STATE_DEAD;
            clear_tomb_request(f);
            break;
    }
}

static void idle(int32_t figure_id, figure *f, const building *home)
{
    f->is_ghost = 1;
    f->retry_count = 0;
    f->animation_frame = 0;
    f->cycle_ticks = 0;
    f->leading_figure_id = 0;
    f->wait_ticks--;
    if (f->wait_ticks > 0) {
        return;
    }
    if (!map_closest_road_within_radius(home->x, home->y, home->size, laborer_rules[LABORER_RULE_HOME_ROAD_RADIUS])) {
        f->action_state = FIGURE_ACTION_CORPSE;
        return;
    }
    if (pyramid_speedup_countdown >= 1) {
        return;
    }
    f->action_state = FIGURE_ACTION_LABORER_GOING_OUT;
    f->destination_x = (int16_t) map_result_x;
    f->destination_y = (int16_t) map_result_y;
    figure_movement_set_cross_country_direction(figure_id, f->cross_country.x, f->cross_country.y, f->destination_x * 15, f->destination_y * 15, 0);
    f->roam_length = 0;
}

static int32_t laborer_image_group(const figure *f)
{
    switch (f->action_state) {
        case FIGURE_ACTION_WAIT_FOR_FERRY:
            return f->action_state_before_attack == FIGURE_ACTION_LABORER_TO_PYRAMID ? IMAGE_GROUP_FIGURE_LABORER_PULLING : IMAGE_GROUP_FIGURE_LABORER;
        case FIGURE_ACTION_LABORER_TO_PYRAMID:
        case FIGURE_ACTION_LABORER_SLED_AT_RAMP:
            return f->leading_figure_id != 0 ? IMAGE_GROUP_FIGURE_LABORER_PULLING : IMAGE_GROUP_FIGURE_LABORER;
        case FIGURE_ACTION_LABORER_TO_MASON:
        case FIGURE_ACTION_LABORER_WALK_TO_MASON:
        case FIGURE_ACTION_LABORER_SLED_TO_SUN_TEMPLE:
        case FIGURE_ACTION_LABORER_SLED_TO_MAUSOLEUM:
        case FIGURE_ACTION_LABORER_SLED_TO_LIBRARY:
        case FIGURE_ACTION_LABORER_SLED_TO_CAESAREUM:
        case FIGURE_ACTION_LABORER_SLED_TO_LIGHTHOUSE:
        case FIGURE_ACTION_LABORER_SLED_TO_TOMB:
            return IMAGE_GROUP_FIGURE_LABORER_PULLING;
        default:
            return IMAGE_GROUP_FIGURE_LABORER;
    }
}

static void update_image(figure *f)
{
    int32_t direction = f->direction - map_info.orientation;
    if (direction < 0) {
        direction += DIRECTION_COUNT;
    }
    figure_image_direction = direction;
    switch (f->action_state) {
        case FIGURE_ACTION_CORPSE:
            f->image_id = image_group(IMAGE_GROUP_FIGURE_LABORER_DEATH) + figure_corpse_offsets[f->wait_ticks / 2];
            return;
        case FIGURE_ACTION_LABORER_LEVEL_WORK:
        case FIGURE_ACTION_LABORER_SUN_TEMPLE_WORK:
        case FIGURE_ACTION_LABORER_MAUSOLEUM_WORK:
        case FIGURE_ACTION_LABORER_LIBRARY_WORK:
            f->image_id = image_group(IMAGE_GROUP_FIGURE_LABORER_WORK) + ((f->animation_frame >> 1) * DIRECTION_COUNT) + direction;
            return;
        default:
            f->image_id = image_group(laborer_image_group(f)) + (f->animation_frame * DIRECTION_COUNT) + direction;
            return;
    }
}

void figure_laborer_action(void)
{
    int32_t figure_id = figure_current_id;
    figure *f = figure_get(figure_id);
    int32_t speed = figure_properties[f->type].speed;
    f->terrain_usage = TERRAIN_USAGE_ROADS;
    f->use_cross_country = 0;
    f->max_roam_length = (int16_t) laborer_rules[LABORER_RULE_MAX_ROAM_TICKS];
    f->animation_frame++;
    if (f->animation_frame >= LABORER_ANIMATION_FRAMES) {
        f->animation_frame = 0;
    }
    f->cycle_ticks++;
    if (f->cycle_ticks >= LABORER_CYCLE_TICKS) {
        f->cycle_ticks = 0;
    }
    building *home = building_get(f->building_id);
    if (home->state != BUILDING_STATE_IN_USE) {
        f->state = FIGURE_STATE_DEAD;
        return;
    }
    switch (f->action_state) {
        case FIGURE_ACTION_CORPSE:
            clear_tomb_request(f);
            figure_combat_handle_corpse(figure_id);
            break;
        case FIGURE_ACTION_ATTACK:
            figure_combat_handle_attack(figure_id);
            f->animation_frame++;
            if (f->animation_frame >= LABORER_ATTACK_FRAMES) {
                f->animation_frame = 0;
            }
            break;
        case FIGURE_ACTION_ON_FERRY:
            f->image_id = 0;
            break;
        case FIGURE_ACTION_WAIT_FOR_FERRY:
            f->animation_frame = 0;
            f->wait_ticks++;
            if (f->wait_ticks > laborer_rules[LABORER_RULE_FERRY_WAIT_TICKS]) {
                f->wait_ticks = 0;
                if (!figure_is_queued_for_ferry(figure_id)) {
                    f->action_state = FIGURE_ACTION_LABORER_IDLE;
                }
            }
            break;
        case FIGURE_ACTION_LABORER_IDLE:
            idle(figure_id, f, home);
            break;
        case FIGURE_ACTION_LABORER_GOING_OUT:
            f->use_cross_country = 1;
            f->is_ghost = 1;
            if (figure_movement_move_ticks_cross_country(figure_id, 1) == 1) {
                figure_laborer_find_work(&floods_data, figure_id);
            }
            f->is_ghost = 1;
            break;
        case FIGURE_ACTION_LABORER_TO_FARM: {
            const building *farm = building_get(f->destination_building_id);
            if (farm->state != BUILDING_STATE_IN_USE) {
                figure_laborer_find_work(&floods_data, figure_id);
                break;
            }
            f->terrain_usage = TERRAIN_USAGE_ROADS_OR_MONUMENT;
            f->is_ghost = 0;
            f->roam_length++;
            if (f->roam_length >= laborer_rules[LABORER_RULE_FARM_WALK_TICKS]) {
                f->state = FIGURE_STATE_DEAD;
            }
            figure_movement_move_ticks(figure_id, speed);
            if (f->direction == DIRECTION_AT_DESTINATION) {
                laborer_arrive_at_farm(&floods_data, figure_id);
            } else if (f->direction == DIRECTION_REROUTE) {
                figure_route_remove(figure_id);
            } else if (f->direction == DIRECTION_LOST) {
                f->state = FIGURE_STATE_DEAD;
            }
            break;
        }
        case FIGURE_ACTION_LABORER_ROAMING:
            f->is_ghost = 0;
            f->roam_length++;
            if (f->roam_length >= f->max_roam_length) {
                if (!map_closest_road_within_radius(home->x, home->y, home->size, laborer_rules[LABORER_RULE_HOME_ROAD_RADIUS])) {
                    f->state = FIGURE_STATE_DEAD;
                } else {
                    f->action_state = FIGURE_ACTION_LABORER_RETURNING;
                    f->destination_x = (int16_t) map_result_x;
                    f->destination_y = (int16_t) map_result_y;
                }
            }
            figure_movement_roam_ticks(figure_id, 1);
            break;
        case FIGURE_ACTION_LABORER_RETURNING:
            figure_movement_move_ticks(figure_id, speed);
            if (f->direction == DIRECTION_AT_DESTINATION || f->direction == DIRECTION_REROUTE || f->direction == DIRECTION_LOST) {
                f->state = FIGURE_STATE_DEAD;
            }
            break;
        case FIGURE_ACTION_LABORER_TO_STORAGE_YARD:
            f->is_ghost = 0;
            f->use_cross_country = 0;
            f->terrain_usage = TERRAIN_USAGE_PREFER_ROADS;
            if (partner_part(figure_id) == 0) {
                figure_route_remove(figure_id);
                f->state = FIGURE_STATE_DEAD;
                break;
            }
            figure_movement_move_ticks(figure_id, speed);
            if (f->direction == DIRECTION_AT_DESTINATION) {
                f->action_state = FIGURE_ACTION_LABORER_AT_STORAGE_YARD;
            } else if (f->direction == DIRECTION_REROUTE) {
                figure_route_remove(figure_id);
                f->state = FIGURE_STATE_DEAD;
                clear_tomb_request(f);
            } else if (f->direction == DIRECTION_LOST) {
                f->state = FIGURE_STATE_DEAD;
                clear_tomb_request(f);
            }
            break;
        case FIGURE_ACTION_LABORER_AT_STORAGE_YARD:
            at_storage_yard(figure_id, f, home);
            break;
        case FIGURE_ACTION_LABORER_SLED_AT_RAMP:
            if (!figure_laborer_partner_still_working(figure_id)) {
                break;
            }
            if (f->routing_path_id <= 0) {
                f->wait_ticks++;
                if (f->wait_ticks % laborer_rules[LABORER_RULE_RAMP_ROUTE_RETRY_TICKS] == 0) {
                    if (f->wait_ticks >= laborer_rules[LABORER_RULE_RAMP_ROUTE_GIVE_UP_TICKS]) {
                        f->action_state = FIGURE_ACTION_LABORER_IDLE;
                    } else {
                        figure_route_add(figure_id);
                    }
                }
            }
            if (!pyramid_laborer_can_enter_ramp(figure_id)) {
                break;
            }
            f->action_state = FIGURE_ACTION_LABORER_TO_PYRAMID;
            to_pyramid(figure_id, f, speed);
            break;
        case FIGURE_ACTION_LABORER_TO_PYRAMID:
            to_pyramid(figure_id, f, speed);
            break;
        case FIGURE_ACTION_LABORER_CLIMB_RAMP:
            climb_ramp(figure_id, f);
            break;
        case FIGURE_ACTION_LABORER_TO_MASON: {
            f->is_ghost = 0;
            int32_t part_id = map_grids.building_ids[figure_get(f->leading_figure_id)->grid_offset];
            if (building_get(part_id)->type == BUILDING_PYRAMID) {
                pyramid_figure_set_walk_target(figure_id, part_id, f->pyramid_work.height, 0);
                f->action_state = FIGURE_ACTION_LABORER_WALK_TO_MASON;
            } else {
                f->action_state = FIGURE_ACTION_LABORER_IDLE;
                f->pyramid_walk_mode = 0;
            }
            break;
        }
        case FIGURE_ACTION_LABORER_WALK_TO_MASON:
            walk_to_mason(figure_id, f);
            break;
        case FIGURE_ACTION_LABORER_TO_LEVEL_TILE:
            f->use_cross_country = 1;
            f->is_ghost = 0;
            if (figure_movement_move_ticks_cross_country(figure_id, 1) == 1) {
                f->action_state = FIGURE_ACTION_LABORER_LEVEL_WORK;
                f->wait_ticks = 0;
            }
            break;
        case FIGURE_ACTION_LABORER_LEVEL_WORK:
            if (pyramid_laborer_level_work(figure_id)) {
                break;
            }
            if (laborer_farm_work_waiting(&floods_data)) {
                f->action_state = FIGURE_ACTION_LABORER_IDLE;
                break;
            }
            f->retry_count++;
            if (f->retry_count >= laborer_rules[LABORER_RULE_LEVEL_TILES_IN_A_ROW]) {
                f->action_state = FIGURE_ACTION_LABORER_IDLE;
                break;
            }
            f->action_state = pyramid_laborer_take_level_tile(figure_id) ? FIGURE_ACTION_LABORER_TO_LEVEL_TILE : FIGURE_ACTION_LABORER_IDLE;
            break;
        case FIGURE_ACTION_LABORER_SLED_TO_SUN_TEMPLE:
            sled_to_monument(figure_id, f, speed, 0);
            break;
        case FIGURE_ACTION_LABORER_SLED_TO_MAUSOLEUM:
            sled_to_monument(figure_id, f, speed, 1);
            break;
        case FIGURE_ACTION_LABORER_SLED_TO_CAESAREUM:
            sled_to_counted_monument(figure_id, f, speed, &caesareum_object.base);
            break;
        case FIGURE_ACTION_LABORER_SLED_TO_LIGHTHOUSE:
            sled_to_counted_monument(figure_id, f, speed, &lighthouse_object.base);
            break;
        case FIGURE_ACTION_LABORER_SLED_TO_LIBRARY:
            sled_to_counted_monument(figure_id, f, speed, &library_object.base);
            break;
        case FIGURE_ACTION_LABORER_TO_SUN_TEMPLE:
            walk_to_monument(figure_id, f, speed, FIGURE_ACTION_LABORER_SUN_TEMPLE_WORK, 0);
            break;
        case FIGURE_ACTION_LABORER_TO_MAUSOLEUM:
            walk_to_monument(figure_id, f, speed, FIGURE_ACTION_LABORER_MAUSOLEUM_WORK, 0);
            break;
        case FIGURE_ACTION_LABORER_TO_LIBRARY:
            walk_to_monument(figure_id, f, speed, FIGURE_ACTION_LABORER_LIBRARY_WORK, 1);
            break;
        case FIGURE_ACTION_LABORER_SUN_TEMPLE_WORK:
            monument_work(figure_id, f, BUILDING_SUN_TEMPLE);
            break;
        case FIGURE_ACTION_LABORER_MAUSOLEUM_WORK:
            monument_work(figure_id, f, BUILDING_MAUSOLEUM);
            break;
        case FIGURE_ACTION_LABORER_LIBRARY_WORK:
            monument_work(figure_id, f, BUILDING_ALEXANDRIA_LIBRARY);
            break;
        case FIGURE_ACTION_LABORER_DELIVERED:
            f->wait_ticks++;
            if (f->wait_ticks > laborer_rules[LABORER_RULE_DELIVERED_WAIT_TICKS]) {
                f->action_state = FIGURE_ACTION_LABORER_IDLE;
                f->pyramid_walk_mode = 0;
                f->wait_ticks = 0;
                f->is_ghost = 1;
            }
            break;
        case FIGURE_ACTION_LABORER_SLED_TO_TOMB:
            sled_to_tomb(figure_id, f, speed);
            break;
        default:
            f->action_state = FIGURE_ACTION_LABORER_IDLE;
            break;
    }
    if (f->state == FIGURE_STATE_DEAD) {
        building_get(f->building_id)->data.industry.worker_count--;
    }
    update_image(f);
}

static int32_t laborer_pulls_train(uint8_t action, int32_t with_tomb_sled)
{
    switch (action) {
        case FIGURE_ACTION_LABORER_TO_PYRAMID:
        case FIGURE_ACTION_LABORER_SLED_AT_RAMP:
        case FIGURE_ACTION_LABORER_CLIMB_RAMP:
        case FIGURE_ACTION_LABORER_TO_MASON:
        case FIGURE_ACTION_LABORER_WALK_TO_MASON:
        case FIGURE_ACTION_LABORER_SLED_TO_SUN_TEMPLE:
        case FIGURE_ACTION_LABORER_SLED_TO_CAESAREUM:
        case FIGURE_ACTION_LABORER_SLED_TO_LIGHTHOUSE:
        case FIGURE_ACTION_LABORER_SLED_TO_LIBRARY:
        case FIGURE_ACTION_LABORER_SLED_TO_MAUSOLEUM:
        case FIGURE_ACTION_LABORER_DELIVERED:
        case FIGURE_ACTION_WAIT_FOR_FERRY:
        case FIGURE_ACTION_ON_FERRY:
            return 1;
        case FIGURE_ACTION_LABORER_SLED_TO_TOMB:
            return with_tomb_sled;
        default:
            return 0;
    }
}

static int32_t front_keeps_pulling(const figure *f, int32_t is_sled)
{
    const figure *front = figure_get(f->leading_figure_id);
    if (front->type == FIGURE_LABORER) {
        return laborer_pulls_train(front->action_state, is_sled);
    }
    return front->type == FIGURE_SLED_PULLER && front->action_state == FIGURE_ACTION_LABORER_IDLE;
}

static void follow_front(int32_t figure_id, const figure *f, const figure *leader, int32_t position, int32_t is_sled)
{
    if (leader->action_state == FIGURE_ACTION_ON_FERRY) {
        return;
    }
    int32_t max_spacing = is_sled ? LABORER_SLED_SPACING : LABORER_PULLER_SPACING;
    int32_t spacing = max_spacing;
    if (leader->action_state == FIGURE_ACTION_LABORER_DELIVERED) {
        int32_t sled_extra = is_sled ? (LABORER_SLED_SPACING - LABORER_PULLER_SPACING) : 0;
        spacing = (LABORER_PULLER_SPACING * position) - leader->wait_ticks + sled_extra;
        if (spacing < 1) {
            spacing = 1;
        } else if (spacing > max_spacing) {
            spacing = max_spacing;
        }
    }
    figure_movement_follow_closely(figure_id, f->leading_figure_id, 1, spacing);
}

static void train_follow(int32_t figure_id, int32_t is_sled)
{
    int32_t position;
    int32_t leader_id = figure_find_train_leader(figure_id, &position);
    const figure *leader = figure_get(leader_id);
    figure *f = figure_get(figure_id);
    if (f->action_state != FIGURE_ACTION_LABORER_IDLE) {
        f->state = FIGURE_STATE_DEAD;
    } else {
        f->is_ghost = 0;
        f->use_cross_country = 0;
        if (front_keeps_pulling(f, is_sled)) {
            follow_front(figure_id, f, leader, position, is_sled);
        } else {
            f->state = FIGURE_STATE_DEAD;
        }
    }
    if (leader->action_state == FIGURE_ACTION_ON_FERRY) {
        f->is_ghost = 1;
    }
}

static int32_t train_direction(const figure *f)
{
    int32_t direction = f->direction - map_info.orientation;
    if (direction < 0) {
        direction += DIRECTION_COUNT;
    }
    return direction;
}

static int32_t sled_image_group(const figure *sled)
{
    int32_t resource = sled->resource_id;
    if (sled->pyramid_walk_mode != 0 && sled->pyramid_work.previous_height < sled->pyramid_work.height) {
        switch (resource) {
            case RESOURCE_STONE:
                return IMAGE_GROUP_FIGURE_SLED_STONE_ON_RAMP;
            case RESOURCE_LIMESTONE:
                return IMAGE_GROUP_FIGURE_SLED_LIMESTONE_ON_RAMP;
            case RESOURCE_GRANITE:
                return IMAGE_GROUP_FIGURE_SLED_GRANITE_ON_RAMP;
            case RESOURCE_SANDSTONE:
                return IMAGE_GROUP_FIGURE_SLED_SANDSTONE_ON_RAMP;
            case RESOURCE_MARBLE:
                return IMAGE_GROUP_FIGURE_SLED_MARBLE_ON_RAMP;
            default:
                return IMAGE_GROUP_FIGURE_SLED_BRICKS_ON_RAMP;
        }
    }
    switch (resource) {
        case RESOURCE_STONE:
            return IMAGE_GROUP_FIGURE_SLED_STONE;
        case RESOURCE_LIMESTONE:
            return IMAGE_GROUP_FIGURE_SLED_LIMESTONE;
        case RESOURCE_GRANITE:
            return IMAGE_GROUP_FIGURE_SLED_GRANITE;
        case RESOURCE_COPPER:
            return IMAGE_GROUP_FIGURE_SLED_COPPER;
        case RESOURCE_SANDSTONE:
            return IMAGE_GROUP_FIGURE_SLED_SANDSTONE;
        case RESOURCE_LAMPS:
            return IMAGE_GROUP_FIGURE_SLED_LAMPS;
        case RESOURCE_MARBLE:
            return IMAGE_GROUP_FIGURE_SLED_MARBLE;
        default:
            return IMAGE_GROUP_FIGURE_SLED_BRICKS;
    }
}

void figure_sled_action(void)
{
    int32_t figure_id = figure_current_id;
    train_follow(figure_id, 1);
    figure *f = figure_get(figure_id);
    f->image_id = image_group(sled_image_group(f)) + train_direction(f);
}

void figure_sled_puller_action(void)
{
    int32_t figure_id = figure_current_id;
    train_follow(figure_id, 0);
    figure *f = figure_get(figure_id);
    int32_t group = f->resource_id != RESOURCE_NONE ? IMAGE_GROUP_FIGURE_LABORER_PULLING : IMAGE_GROUP_FIGURE_SLED_PULLER_FRONT;
    f->image_id = image_group(group) + (f->animation_frame * DIRECTION_COUNT) + train_direction(f);
}

static void follow_on_ground(int32_t figure_id, figure *f, const figure *leader, int32_t distance)
{
    int32_t progress = leader->progress_on_tile - distance;
    if (progress < 0) {
        progress = (uint8_t) (progress + MOVEMENT_STEPS_PER_TILE);
    }
    f->progress_on_tile = (int8_t) progress;
    if (f->progress_on_tile == MOVEMENT_TILE_CHANGE_STEP) {
        map_figure_delete(figure_id);
        int16_t old_x = f->x;
        int16_t old_y = f->y;
        f->x = leader->x;
        f->y = leader->y;
        f->grid_offset = map_info.grid_start + (f->y * GRID_SIZE) + f->x;
        if (f->x != old_x || f->y != old_y) {
            f->previous_tile_x = old_x;
            f->previous_tile_y = old_y;
        }
        map_figure_add(figure_id);
        uint32_t terrain = map_grids.terrain[f->grid_offset];
        if (!(terrain & TERRAIN_ROAD) && !(terrain & TERRAIN_WALKABLE_BUILDING)) {
            f->is_on_road = 0;
        } else {
            f->is_on_road = 1;
            if (terrain & TERRAIN_WATER) {
                uint8_t sprite = map_grids.sprite[f->grid_offset];
                f->height_adjusted_ticks = MOVEMENT_BRIDGE_CLIMB_TICKS;
                f->target_height = (uint8_t) figure_bridge_height(sprite);
            }
        }
    }
    if (f->progress_on_tile == 0) {
        f->previous_tile_direction = f->direction;
        f->direction = leader->direction;
    }
    if (f->height_adjusted_ticks) {
        f->height_adjusted_ticks--;
        if (f->height_adjusted_ticks == 0) {
            f->is_ghost = 0;
            return;
        }
        f->is_ghost = 1;
        if (f->current_height < f->target_height) {
            f->current_height++;
        }
        if (f->current_height > f->target_height) {
            f->current_height--;
        }
    } else if (f->current_height) {
        f->current_height--;
    } else {
        f->is_ghost = 0;
    }
}

static int32_t follow_step_reaches_leader_walk(int32_t figure_id, figure *f, const figure *leader, int32_t climb_step, int32_t distance)
{
    if (f->pyramid_walk_mode != 0) {
        if (f->pyramid_work.part == leader->pyramid_work.part) {
            if ((f->pyramid_work.progress + distance) < leader->pyramid_work.progress) {
                pyramid_figure_climb_step(figure_id, climb_step);
            }
            return 0;
        }
        return f->pyramid_work.part != leader->pyramid_work.previous_part || pyramid_figure_climb_step(figure_id, climb_step);
    }
    if (leader->pyramid_walk_mode != 0) {
        f->progress_on_tile++;
        if (f->progress_on_tile <= LABORER_TRAIN_CATCH_UP_TICKS) {
            return 0;
        }
        f->progress_on_tile = 0;
        return 1;
    }
    follow_on_ground(figure_id, f, leader, distance);
    return 0;
}

static void take_leader_walk(int32_t figure_id, figure *f, const figure *leader, int32_t distance)
{
    int32_t progress = leader->pyramid_work.progress - distance;
    f->pyramid_walk_mode = leader->pyramid_walk_mode;
    f->pyramid_work.total = leader->pyramid_work.total;
    f->pyramid_work.progress = (int16_t) ((progress < 0) ? 0 : progress);
    f->pyramid_work.previous_part = leader->pyramid_work.previous_part;
    f->pyramid_work.part = leader->pyramid_work.part;
    f->pyramid_work.previous_height = leader->pyramid_work.previous_height;
    f->pyramid_work.height = leader->pyramid_work.height;
    f->direction = leader->direction;
    pyramid_figure_climb_step(figure_id, 0);
}

void figure_movement_follow_closely(int32_t figure_id, int32_t leader_id, int32_t climb_step, int32_t distance)
{
    figure *f = figure_get(figure_id);
    const figure *leader = figure_get(leader_id);
    if (f->x == f->source_x && f->y == f->source_y) {
        f->is_ghost = 1;
    }
    if (f->pyramid_walk_mode != 0 && leader->pyramid_walk_mode == 0) {
        f->pyramid_walk_mode = 0;
    }
    if (follow_step_reaches_leader_walk(figure_id, f, leader, climb_step, distance)) {
        take_leader_walk(figure_id, f, leader, distance);
    }
    f->animation_frame = leader->animation_frame;
}

static void work_camp_spawn_figure(int32_t building_id)
{
    building *b = building_get(building_id);
    if (b->houses_covered <= 0) {
        b->show_on_problem_overlay = 2;
    }
    if (!map_has_road_access(b->x, b->y, b->size)) {
        return;
    }
    if (b->houses_covered <= laborer_rules[LABORER_RULE_LABOR_SEEKER_HOUSES]) {
        generate_labor_seeker(building_id);
    }
    int32_t percentage = calc_percentage(b->num_workers, building_model_table[b->type].laborers);
    int32_t delay;
    if (percentage >= 100) {
        delay = laborer_rules[LABORER_RULE_SPAWN_DELAY_FULL];
    } else if (percentage >= 75) {
        delay = laborer_rules[LABORER_RULE_SPAWN_DELAY_75_PERCENT];
    } else if (percentage >= 50) {
        delay = laborer_rules[LABORER_RULE_SPAWN_DELAY_50_PERCENT];
    } else if (percentage >= 25) {
        delay = laborer_rules[LABORER_RULE_SPAWN_DELAY_25_PERCENT];
    } else if (percentage >= 1) {
        delay = laborer_rules[LABORER_RULE_SPAWN_DELAY_LOW];
    } else {
        return;
    }
    if (b->data.industry.worker_count >= laborer_rules[LABORER_RULE_MAX_OUT]) {
        return;
    }
    b->figure_spawn_delay++;
    if (b->figure_spawn_delay <= delay) {
        return;
    }
    b->figure_spawn_delay = 0;
    int32_t figure_id = figure_create(b->player, FIGURE_LABORER, map_result_x, map_result_y, 0);
    b->figure_id = (int16_t) figure_id;
    figure *f = figure_get(figure_id);
    f->action_state = FIGURE_ACTION_LABORER_IDLE;
    b->data.industry.worker_count++;
    f->building_id = (int16_t) building_id;
}

static void add_farm_candidates(const floods *self, const figure *f)
{
    if (floods_is_running(self)) {
        return;
    }
    for (int32_t id = 1; id <= building_highest_id; id++) {
        const building *b = building_get(id);
        if (b->state != BUILDING_STATE_IN_USE || !building_is_farm(id) || !building_is_on_floodplain(id) || b->data.industry.worker_figure_id != 0 ||
            city_labor_industry_is_mothballed(id) != 0 || b->distance_from_entry == 0 || !map_has_road_access(b->x, b->y, b->size)) {
            continue;
        }
        int32_t distance = calc_maximum_distance(f->x, f->y, b->x, b->y);
        int32_t farm_busy = b->data.industry.labor_days_left[0] >= laborer_rules[LABORER_RULE_FARM_BUSY_LABOR_DAYS];
        if (distance > laborer_rules[LABORER_RULE_FARM_SEARCH_DISTANCE] || city_locusts_countdown > 0 || farm_busy) {
            continue;
        }
        building_list_small[building_list_small_size++] = (int16_t) id;
        if (building_list_small_size >= LABORER_TASK_CANDIDATES) {
            building_list_small_size = LABORER_TASK_CANDIDATES - 1;
        }
    }
}

static void add_monument_candidates(int32_t figure_id, int32_t *masons)
{
    for (int32_t id = 1; id <= building_highest_id; id++) {
        const building *b = building_get(id);
        if (b->state != BUILDING_STATE_IN_USE) {
            continue;
        }
        int32_t mason_id = 0;
        int32_t has_work;
        switch (b->type) {
            case BUILDING_PYRAMID:
                if (b->data.pyramid.stage > PYRAMID_STAGE_CASING) {
                    int32_t x, y;
                    has_work = pyramid_find_level_tile(id, &x, &y, 1);
                } else {
                    mason_id = pyramid_find_mason_waiting_for_block(id, 0);
                    has_work = mason_id != 0;
                }
                if (has_work && building_list_small_size < LABORER_TASK_CANDIDATES) {
                    masons[building_list_small_size] = mason_id;
                    building_list_small[building_list_small_size++] = (int16_t) id;
                }
                id += part_count(b->data.pyramid.size_variant) - 1;
                continue;
            case BUILDING_SUN_TEMPLE:
                has_work = sun_temple_check_work_for_figure(id, figure_id, 0);
                break;
            case BUILDING_CAESAREUM:
                has_work = caesareum_check_work_for_figure(&caesareum_object.base, id, figure_id, 0);
                break;
            case BUILDING_LIGHTHOUSE:
                has_work = lighthouse_check_work_for_figure(&lighthouse_object.base, id, figure_id, 0);
                break;
            case BUILDING_ALEXANDRIA_LIBRARY:
                has_work = library_check_work_for_figure(&library_object.base, id, figure_id, 0);
                break;
            case BUILDING_MAUSOLEUM:
                has_work = mausoleum_check_work_for_figure(id, figure_id, 0);
                break;
            case BUILDING_SMALL_ROYAL_TOMB:
            case BUILDING_MEDIUM_ROYAL_TOMB:
            case BUILDING_LARGE_ROYAL_TOMB:
            case BUILDING_GRAND_ROYAL_TOMB:
                has_work = royal_tomb_check_work_for_figure(royal_tomb_object_for_type(b->type), id, figure_id, 0);
                break;
            default:
                continue;
        }
        if (has_work && building_list_small_size < LABORER_TASK_CANDIDATES) {
            masons[building_list_small_size] = 0;
            building_list_small[building_list_small_size++] = (int16_t) id;
        }
    }
}

static int32_t nearest_candidate(const figure *f, const int32_t *masons, int32_t *best_mason_id, int32_t *all_reachable)
{
    int32_t best_distance = LABORER_NO_DISTANCE;
    int32_t best_id = 0;
    for (building_list_small_index = 0; building_list_small_index < building_list_small_size;) {
        int32_t id = building_list_small[building_list_small_index];
        int32_t mason_id = masons[building_list_small_index];
        building_list_small_index++;
        building *b = building_get(id);
        int32_t distance = calc_maximum_distance(f->x, f->y, b->x, b->y);
        if (building_is_farm(id)) {
            distance += b->data.industry.labor_days_left[0];
        }
        if (distance >= best_distance) {
            continue;
        }
        if (building_is_farm(id) && !map_routing_citizen_can_travel_over_road_garden(f->x, f->y, b->road_access_x, b->road_access_y)) {
            b->data.industry.worker_figure_id = -1;
            *all_reachable = 0;
            continue;
        }
        best_distance = distance;
        best_id = id;
        *best_mason_id = mason_id;
    }
    return best_id;
}

static int32_t find_best_task(const floods *self, const figure *f, int32_t figure_id, int32_t *best_mason_id)
{
    int32_t masons[LABORER_TASK_CANDIDATES];
    for (;;) {
        building_list_small_size = 0;
        int32_t all_reachable = 1;
        *best_mason_id = 0;
        add_farm_candidates(self, f);
        if (building_list_small_size == 0) {
            add_monument_candidates(figure_id, masons);
        } else {
            for (int32_t i = 0; i < building_list_small_size; i++) {
                masons[i] = 0;
            }
        }
        if (building_list_small_size <= 0) {
            return 0;
        }
        int32_t best_id = nearest_candidate(f, masons, best_mason_id, &all_reachable);
        if (best_id != 0 || all_reachable) {
            return best_id;
        }
    }
}

static int32_t assign_farm(figure *f, int32_t figure_id, building *farm, int32_t farm_id)
{
    if (!map_closest_road_within_radius(farm->x, farm->y, farm->size, laborer_rules[LABORER_RULE_HOME_ROAD_RADIUS])) {
        return 0;
    }
    farm->data.industry.worker_figure_id = (int16_t) figure_id;
    f->destination_building_id = (int16_t) farm_id;
    f->action_state = FIGURE_ACTION_LABORER_TO_FARM;
    f->destination_x = (int16_t) map_result_x;
    figure_get(figure_current_id)->destination_y = (int16_t) map_result_y;
    f->roam_length = 0;
    return 1;
}

static int32_t pyramid_material(int32_t part_id)
{
    int32_t mudbrick = scenario_pyramid_style == PYRAMID_STYLE_MUDBRICK || scenario_pyramid_style == PYRAMID_STYLE_MASTABA;
    if (pyramid_part_needs_main_material(part_id)) {
        return mudbrick ? RESOURCE_BRICKS : RESOURCE_STONE;
    }
    if (scenario_pyramid_style == PYRAMID_STYLE_STEPPED) {
        return RESOURCE_STONE;
    }
    if (scenario_pyramid_style == PYRAMID_STYLE_MASTABA) {
        return RESOURCE_BRICKS;
    }
    return RESOURCE_LIMESTONE;
}

static int32_t assign_pyramid(figure *f, int32_t figure_id, const building *pyramid, int32_t pyramid_id, int32_t mason_id)
{
    if (pyramid->data.pyramid.stage > PYRAMID_STAGE_CASING) {
        f->destination_building_id = (int16_t) pyramid_id;
        f->leading_figure_id = 0;
        f->action_state = FIGURE_ACTION_LABORER_TO_PYRAMID;
        f->wait_ticks = 0;
        pyramid_set_figure_destination_entry(figure_id, pyramid_id);
        return 1;
    }
    figure *mason = figure_get(mason_id);
    int32_t resource = pyramid_material(map_grids.building_ids[mason->grid_offset]);
    int32_t yard_id = building_storageyard_for_getting(pyramid_id, resource, 1);
    if (yard_id == 0) {
        return 0;
    }
    f->collecting_item_id = (uint8_t) resource;
    f->resource_amount = 0;
    f->destination_building_id = (int16_t) yard_id;
    f->leading_figure_id = (int16_t) mason_id;
    f->action_state = FIGURE_ACTION_LABORER_TO_STORAGE_YARD;
    f->wait_ticks = 0;
    f->destination_x = (int16_t) map_result_x;
    f->destination_y = (int16_t) map_result_y;
    f->roam_length = 0;
    figure_route_remove(figure_id);
    mason->leading_figure_id = (int16_t) figure_id;
    return 1;
}

static int32_t assign_task(figure *f, int32_t figure_id, int32_t task_id, int32_t mason_id)
{
    building *task = building_get(task_id);
    if (building_is_farm(task_id)) {
        return assign_farm(f, figure_id, task, task_id);
    }
    switch (task->type) {
        case BUILDING_PYRAMID:
            return assign_pyramid(f, figure_id, task, task_id, mason_id);
        case BUILDING_SUN_TEMPLE:
            return sun_temple_check_work_for_figure(task_id, figure_id, 1);
        case BUILDING_CAESAREUM:
            return caesareum_check_work_for_figure(&caesareum_object.base, task_id, figure_id, 1);
        case BUILDING_LIGHTHOUSE:
            return lighthouse_check_work_for_figure(&lighthouse_object.base, task_id, figure_id, 1);
        case BUILDING_ALEXANDRIA_LIBRARY:
            return library_check_work_for_figure(&library_object.base, task_id, figure_id, 1);
        case BUILDING_MAUSOLEUM:
            return mausoleum_check_work_for_figure(task_id, figure_id, 1);
        case BUILDING_SMALL_ROYAL_TOMB:
        case BUILDING_MEDIUM_ROYAL_TOMB:
        case BUILDING_LARGE_ROYAL_TOMB:
        case BUILDING_GRAND_ROYAL_TOMB:
            return royal_tomb_check_work_for_figure(royal_tomb_object_for_type(task->type), task_id, figure_id, 1);
        default:
            return 1;
    }
}

static void clear_unreachable_farm_marks(void)
{
    for (int32_t id = 1; id <= building_highest_id; id++) {
        building *b = building_get(id);
        if (b->state == BUILDING_STATE_IN_USE && building_is_farm(id) && b->data.industry.worker_figure_id < 0) {
            b->data.industry.worker_figure_id = 0;
        }
    }
}

void figure_laborer_find_work(floods *self, int32_t figure_id)
{
    figure *f = figure_get(figure_id);
    int32_t mason_id;
    int32_t task_id = find_best_task(self, f, figure_id, &mason_id);
    if (task_id == 0 || !assign_task(f, figure_id, task_id, mason_id)) {
        f->state = FIGURE_STATE_DEAD;
    }
    clear_unreachable_farm_marks();
}

void building_work_camp_spawn_figure(floods *self, int32_t building_id)
{
    work_camp_spawn_figure(building_id);
}

void laborer_arrive_at_farm(floods *self, int32_t figure_id)
{
    (void) self;
    int32_t farm = city_figures.records[figure_id].destination_building_id;
    int32_t home = city_figures.records[figure_id].building_id;
    if (!building_is_farm(farm)) {
        city_buildings.records[home].data.industry.labor_days_left[1] = (uint8_t) laborer_rules[LABORER_RULE_CAMP_LABOR_DAYS];
        return;
    }
    city_figures.records[figure_id].state = FIGURE_STATE_DEAD;
    if (city_locusts_countdown != 0) {
        return;
    }
    building *farm_building = &city_buildings.records[farm];
    city_buildings.records[home].data.industry.labor_days_left[0] = (uint8_t) laborer_rules[LABORER_RULE_CAMP_LABOR_DAYS];
    farm_building->data.industry.labor_days_left[0] = (uint8_t) laborer_rules[LABORER_RULE_FARM_LABOR_DAYS];
    farm_building->data.industry.worker_count++;
    farm_building->data.industry.worker_figure_id = 0;
    farm_building->num_workers = (int16_t) building_model_table[farm_building->type].laborers;
}

bool laborer_farm_work_waiting(floods *self)
{
    if (floods_is_running(self)) {
        return 0;
    }
    for (int32_t id = 1; id <= MAX_BUILDINGS - 1; id++) {
        if (city_buildings.records[id].state == BUILDING_STATE_IN_USE && building_is_farm(id) &&
            building_is_on_floodplain(id) && city_buildings.records[id].data.industry.worker_figure_id == 0 &&
            city_labor_industry_is_mothballed(id) == 0) {
            return 1;
        }
    }
    return 0;
}
