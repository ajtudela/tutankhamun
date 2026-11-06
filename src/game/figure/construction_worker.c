// Construction guild workers: the carpenters, bricklayers, stonemasons and tomb artisans, the guilds' spawning of them,
// the task chains that send a worker to a monument, and the obelisk's progress step. The tunable values are
// guild_worker_rules from config/guild_worker_rules.json.
//
// The worker dispatch has these slips, all of them kept:
// - the artisan works the grand tomb through the medium tomb object;
// - sending a worker clears the roam counter of figure_current_id, not of the new worker;
// - a carpenter whose destination is no monument disappears where it is;
// - the stonemason sets TERRAIN_USAGE_ROADS_OR_MONUMENT for every target.

#include "figure/construction_worker.h"
#include "building/abu_simbel.h"
#include "building/building.h"
#include "building/building_data.h"
#include "building/building_figure_helpers.h"
#include "building/building_type.h"
#include "building/caesareum.h"
#include "building/library.h"
#include "building/lighthouse.h"
#include "building/mausoleum.h"
#include "building/monument_internal.h"
#include "building/obelisk.h"
#include "building/pyramid_blocks.h"
#include "building/pyramid_figure.h"
#include "building/pyramid_parts.h"
#include "building/royal_tomb.h"
#include "building/sphinx.h"
#include "building/sun_temple.h"
#include "core/calc.h"
#include "core/crt_compat.h"
#include "core/random.h"
#include "figure/boats.h"
#include "figure/figure.h"
#include "figure/figure_action.h"
#include "figure/figure_core.h"
#include "figure/figure_data.h"
#include "figure/figure_image_group.h"
#include "figure/figure_type.h"
#include "figure/military.h"
#include "figure/movement.h"
#include "graphics/image_collection.h"
#include "map/grid.h"
#include "map/road_access.h"
#include "map/terrain.h"
#include <stdint.h>


int32_t guild_worker_rules[GUILD_WORKER_RULE_COUNT] = {
    [GUILD_WORKER_RULE_MAX_ROAM_TICKS] = 512,
    [GUILD_WORKER_RULE_FERRY_WAIT_TICKS] = 100,
    [GUILD_WORKER_RULE_GHOST_TICKS] = 10,
    [GUILD_WORKER_RULE_TASK_SEARCH_BASE_TICKS] = 30,
    [GUILD_WORKER_RULE_TASK_SEARCH_RANDOM_TICKS] = 30,
    [GUILD_WORKER_RULE_GUILD_ROAD_RADIUS] = 2,
    [GUILD_WORKER_RULE_MAUSOLEUM_HEAD_START_TICKS] = 10,
    [GUILD_WORKER_RULE_ABU_SIMBEL_SPOTS_IN_A_ROW] = 4,
    [GUILD_WORKER_RULE_LOAD] = 100,
    [GUILD_WORKER_RULE_CARPENTER_PROGRESS] = 400,
};

static int32_t is_royal_tomb(int32_t type)
{
    return type == BUILDING_SMALL_ROYAL_TOMB || type == BUILDING_MEDIUM_ROYAL_TOMB || type == BUILDING_LARGE_ROYAL_TOMB || type == BUILDING_GRAND_ROYAL_TOMB;
}

static int32_t image_direction(const figure *f)
{
    int32_t dir = f->direction - map_info.orientation;
    if (dir < 0) {
        dir += 8;
    }
    figure_image_direction = dir;
    return dir;
}

static int32_t corpse_image(int32_t base, const figure *f)
{
    return base + figure_corpse_offsets[f->wait_ticks / 2];
}

static void decrease_guild_figure_count(const figure *f)
{
    building_get(f->building_id)->data.industry.worker_count--;
}

static int32_t worker_begin(figure *f, int32_t home_type)
{
    if (++f->animation_frame >= GUILD_WORKER_ANIMATION_FRAMES) {
        f->animation_frame = 0;
    }
    f->terrain_usage = TERRAIN_USAGE_ROADS;
    f->use_cross_country = 0;
    f->max_roam_length = (int16_t) guild_worker_rules[GUILD_WORKER_RULE_MAX_ROAM_TICKS];
    const building *home = building_get(f->building_id);
    if (home->state != BUILDING_STATE_IN_USE || home->type != home_type) {
        f->state = FIGURE_STATE_DEAD;
        return 0;
    }
    return 1;
}

static void wait_for_ferry(figure *f, int32_t figure_id)
{
    f->animation_frame = 0;
    if (++f->wait_ticks > guild_worker_rules[GUILD_WORKER_RULE_FERRY_WAIT_TICKS]) {
        f->wait_ticks = 0;
        if (!figure_is_queued_for_ferry(figure_id)) {
            f->action_state = FIGURE_ACTION_GUILD_WORKER_ROAM;
        }
    }
}

static void roam_near_guild(figure *f, int32_t figure_id, int32_t return_action)
{
    if (f->action_state != FIGURE_ACTION_GUILD_WORKER_ROAM) {
        f->animation_frame = 0;
        return;
    }
    if (++f->roam_length >= f->max_roam_length) {
        const building *home = building_get(f->building_id);
        if (!map_closest_road_within_radius(home->x, home->y, home->size, guild_worker_rules[GUILD_WORKER_RULE_GUILD_ROAD_RADIUS])) {
            f->action_state = FIGURE_ACTION_CORPSE;
            figure_movement_roam_ticks(figure_id, 1);
            return;
        }
        f->action_state = (uint8_t) return_action;
        f->destination_x = map_result_x;
        f->destination_y = map_result_y;
        figure_route_remove(figure_id);
        f->roam_length = 0;
    }
    figure_movement_roam_ticks(figure_id, 1);
}

static void look_for_task(figure *f, int32_t figure_id, int32_t (*find_task)(int32_t, int32_t, int32_t), int32_t found_action)
{
    f->wait_ticks++;
    random_generate_next();
    if (f->wait_ticks > ((random_generator_state.random1_15bit % guild_worker_rules[GUILD_WORKER_RULE_TASK_SEARCH_RANDOM_TICKS]) +
                            guild_worker_rules[GUILD_WORKER_RULE_TASK_SEARCH_BASE_TICKS])) {
        if (find_task(f->x, f->y, figure_id)) {
            f->action_state = (uint8_t) found_action;
            figure_route_remove(figure_id);
            f->roam_length = 0;
        }
        f->wait_ticks = 0;
    }
}

static void return_and_disappear(figure *f, int32_t figure_id, int32_t speed)
{
    figure_movement_move_ticks(figure_id, speed);
    if (f->direction == DIRECTION_AT_DESTINATION || f->direction == DIRECTION_LOST || f->direction == DIRECTION_REROUTE) {
        if (f->direction == DIRECTION_REROUTE) {
            figure_route_remove(figure_id);
        }
        f->state = FIGURE_STATE_DEAD;
        decrease_guild_figure_count(f);
    }
}

static void set_destination_home_road(figure *f, const building *home)
{
    f->destination_x = home->road_access_x;
    f->destination_y = home->road_access_y;
}

static void waiting_mason_image(figure *f, int32_t figure_id, int32_t base, int32_t phase, int32_t frames, int32_t dir)
{
    int32_t v = f->wait_ticks / 2 + figure_id * 13;
    switch ((v / phase) & 3) {
        case 0:
            f->image_id = base + ((v >> 2) & 1) * 8 + dir;
            break;
        case 1:
            f->image_id = base + (v % frames) * 8 + dir;
            break;
        case 2:
            if ((v & 7) == 4) {
                switch (crt_rand() & 3) {
                    case 0:
                        f->direction = (int8_t) ((f->direction + 1) & 7);
                        break;
                    case 1:
                        f->direction = (int8_t) ((f->direction - 1) & 7);
                        break;
                }
                f->image_id = base + figure_image_direction;
            }
            break;
        case 3:
            f->image_id = base + dir;
            break;
    }
}

void figure_carpenter_action(void)
{
    int32_t figure_id = figure_current_id;
    figure *f = figure_get(figure_id);
    if (!worker_begin(f, BUILDING_CARPENTERS_GUILD)) {
        return;
    }
    int32_t speed = figure_properties[f->type].speed;
    const building *home = building_get(f->building_id);
    switch (f->action_state) {
        case FIGURE_ACTION_CORPSE:
            figure_combat_handle_corpse(figure_id);
            if (f->state == FIGURE_STATE_DEAD) {
                decrease_guild_figure_count(f);
            }
            break;
        case FIGURE_ACTION_ATTACK:
            f->is_ghost = 0;
            figure_combat_handle_attack(figure_id);
            break;
        case FIGURE_ACTION_ON_FERRY:
            f->image_id = 0;
            break;
        case FIGURE_ACTION_WAIT_FOR_FERRY:
            wait_for_ferry(f, figure_id);
            break;
        case FIGURE_ACTION_GUILD_WORKER_ROAM:
            if (f->is_ghost) {
                if (++f->wait_ticks > guild_worker_rules[GUILD_WORKER_RULE_GHOST_TICKS]) {
                    f->is_ghost = 0;
                }
                break;
            }
            look_for_task(f, figure_id, monument_find_task_carpenter, FIGURE_ACTION_CARPENTER_GO_TO_SITE);
            roam_near_guild(f, figure_id, FIGURE_ACTION_CARPENTER_GO_TO_GUILD_ROAD);
            break;
        case FIGURE_ACTION_CARPENTER_GO_TO_SITE: {
            f->is_ghost = 0;
            f->terrain_usage = TERRAIN_USAGE_PREFER_ROADS;
            int32_t target_id = map_grids.building_ids[f->destination_grid_offset];
            int32_t target_type = target_id ? building_get(target_id)->type : 0;
            if (target_type == BUILDING_SUN_TEMPLE || target_type == BUILDING_CAESAREUM || target_type == BUILDING_ALEXANDRIA_LIBRARY ||
                target_type == BUILDING_LIGHTHOUSE || target_type == BUILDING_MAUSOLEUM) {
                f->terrain_usage = TERRAIN_USAGE_ROADS_OR_MONUMENT;
            }
            figure_movement_move_ticks(figure_id, speed);
            if (f->direction == DIRECTION_AT_DESTINATION) {
                switch (target_type) {
                    case BUILDING_PYRAMID:
                        f->action_state = FIGURE_ACTION_CARPENTER_CLIMB_RAMP;
                        pyramid_figure_start_climb(figure_id);
                        break;
                    case BUILDING_SPHINX:
                    case BUILDING_SMALL_OBELISK:
                    case BUILDING_LARGE_OBELISK:
                    case BUILDING_SUN_TEMPLE:
                    case BUILDING_CAESAREUM:
                    case BUILDING_ALEXANDRIA_LIBRARY:
                    case BUILDING_ABU_SIMBEL:
                    case BUILDING_LIGHTHOUSE:
                    case BUILDING_MAUSOLEUM:
                        f->action_state = FIGURE_ACTION_CARPENTER_WORK;
                        f->wait_ticks = 0;
                        break;
                    default:
                        f->action_state = FIGURE_ACTION_CARPENTER_RETURN;
                        break;
                }
            } else if (f->direction == DIRECTION_LOST) {
                f->action_state = FIGURE_ACTION_GUILD_WORKER_ROAM;
                f->wait_ticks = 0;
            } else if (f->direction == DIRECTION_REROUTE) {
                figure_route_remove(figure_id);
            }
            break;
        }
        case FIGURE_ACTION_CARPENTER_WORK: {
            f->is_ghost = 0;
            int32_t building_id = map_grids.building_ids[f->destination_grid_offset];
            if (!building_id) {
                f->action_state = FIGURE_ACTION_GUILD_WORKER_ROAM;
                f->wait_ticks = 0;
                break;
            }
            if (home->state != BUILDING_STATE_IN_USE) {
                f->state = FIGURE_STATE_DEAD;
                break;
            }
            switch (building_get(building_id)->type) {
                case BUILDING_PYRAMID:
                    pyramid_carpenter_work_tick(building_id, figure_id);
                    break;
                case BUILDING_SPHINX:
                    sphinx_figure_work_tick(building_id, figure_id, FIGURE_ACTION_CARPENTER_RETURN);
                    break;
                case BUILDING_SMALL_OBELISK:
                case BUILDING_LARGE_OBELISK:
                    obelisk_figure_work_tick(building_id, figure_id, FIGURE_ACTION_CARPENTER_RETURN);
                    break;
                case BUILDING_SUN_TEMPLE:
                    sun_temple_figure_work_tick(building_id, figure_id, FIGURE_ACTION_CARPENTER_RETURN);
                    break;
                case BUILDING_CAESAREUM:
                    caesareum_figure_work_tick(&caesareum_object.base, building_id, figure_id, FIGURE_ACTION_CARPENTER_RETURN);
                    break;
                case BUILDING_LIGHTHOUSE:
                    lighthouse_figure_work_tick(&lighthouse_object.base, building_id, figure_id, FIGURE_ACTION_CARPENTER_RETURN);
                    break;
                case BUILDING_ALEXANDRIA_LIBRARY:
                    library_figure_work_tick(&library_object.base, building_id, figure_id, FIGURE_ACTION_CARPENTER_RETURN);
                    break;
                case BUILDING_ABU_SIMBEL:
                    abu_simbel_figure_work_tick(&abu_simbel_object.base, building_id, figure_id, FIGURE_ACTION_CARPENTER_RETURN);
                    if (f->action_state == FIGURE_ACTION_CARPENTER_RETURN) {
                        if (++f->retry_count >= guild_worker_rules[GUILD_WORKER_RULE_ABU_SIMBEL_SPOTS_IN_A_ROW]) {
                            f->retry_count = 0;
                        } else {
                            abu_simbel_find_work_for_figure_with_wait(&abu_simbel_object, f->x, f->y, figure_id, 1);
                        }
                    }
                    break;
                case BUILDING_MAUSOLEUM:
                    mausoleum_figure_work_tick(building_id, figure_id, FIGURE_ACTION_GUILD_WORKER_ROAM);
                    if (f->action_state == FIGURE_ACTION_GUILD_WORKER_ROAM) {
                        f->wait_ticks = (int16_t) guild_worker_rules[GUILD_WORKER_RULE_MAUSOLEUM_HEAD_START_TICKS];
                    }
                    break;
                default:
                    f->action_state = FIGURE_ACTION_CARPENTER_RETURN;
                    set_destination_home_road(f, home);
                    break;
            }
            break;
        }
        case FIGURE_ACTION_CARPENTER_RETURN:
            f->is_ghost = 0;
            f->terrain_usage = TERRAIN_USAGE_ROADS_OR_MONUMENT;
            return_and_disappear(f, figure_id, speed);
            break;
        case FIGURE_ACTION_CARPENTER_CLIMB_RAMP:
            f->is_ghost = 0;
            pyramid_figure_walk_on_ramp(figure_id, 1, FIGURE_ACTION_CARPENTER_RETURN, FIGURE_ACTION_CARPENTER_WORK);
            break;
        case FIGURE_ACTION_CARPENTER_GO_TO_GUILD_ROAD:
            f->is_ghost = 0;
            f->terrain_usage = TERRAIN_USAGE_ROADS_OR_MONUMENT;
            figure_movement_move_ticks(figure_id, speed);
            if (f->direction == DIRECTION_AT_DESTINATION) {
                f->action_state = FIGURE_ACTION_GUILD_WORKER_ROAM;
                f->wait_ticks = 0;
                f->is_ghost = 1;
            } else if (f->direction == DIRECTION_LOST || f->direction == DIRECTION_REROUTE) {
                if (f->direction == DIRECTION_REROUTE) {
                    figure_route_remove(figure_id);
                }
                f->state = FIGURE_STATE_DEAD;
                decrease_guild_figure_count(f);
            }
            break;
    }

    int32_t dir = image_direction(f);
    if (f->action_state == FIGURE_ACTION_CORPSE) {
        f->image_id = corpse_image(image_group(IMAGE_GROUP_FIGURE_CARPENTER_DEATH), f);
    } else if (f->action_state == FIGURE_ACTION_CARPENTER_WORK) {
        f->image_id = image_group(IMAGE_GROUP_FIGURE_CARPENTER_WORK) + (f->animation_frame / 2) * 8 + dir;
    } else {
        f->image_id = image_group(IMAGE_GROUP_FIGURE_CARPENTER) + f->animation_frame * 8 + dir;
    }
}

void figure_bricklayer_action(void)
{
    int32_t figure_id = figure_current_id;
    figure *f = figure_get(figure_id);
    int32_t carries_block = 0;
    int32_t speed = figure_properties[f->type].speed;
    if (f->wait_ticks < 0) {
        f->wait_ticks = 0;
    }
    if (!worker_begin(f, BUILDING_BRICKLAYERS_GUILD)) {
        return;
    }
    const building *home = building_get(f->building_id);
    switch (f->action_state) {
        case FIGURE_ACTION_CORPSE:
            figure_combat_handle_corpse(figure_id);
            if (f->state == FIGURE_STATE_DEAD) {
                decrease_guild_figure_count(f);
            }
            break;
        case FIGURE_ACTION_ATTACK:
            figure_combat_handle_attack(figure_id);
            break;
        case FIGURE_ACTION_ON_FERRY:
            f->image_id = 0;
            break;
        case FIGURE_ACTION_WAIT_FOR_FERRY:
            wait_for_ferry(f, figure_id);
            break;
        case FIGURE_ACTION_GUILD_WORKER_ROAM:
            look_for_task(f, figure_id, figure_bricklayer_find_monument_task, FIGURE_ACTION_BRICKLAYER_GO_TO_SITE);
            roam_near_guild(f, figure_id, FIGURE_ACTION_BRICKLAYER_RETURN);
            break;
        case FIGURE_ACTION_BRICKLAYER_RETURN:
            f->terrain_usage = TERRAIN_USAGE_PREFER_ROADS;
            figure_movement_move_ticks(figure_id, speed);
            if (f->direction == DIRECTION_AT_DESTINATION || f->direction == DIRECTION_LOST) {
                f->state = FIGURE_STATE_DEAD;
                decrease_guild_figure_count(f);
            } else if (f->direction == DIRECTION_REROUTE) {
                figure_route_remove(figure_id);
            }
            break;
        case FIGURE_ACTION_BRICKLAYER_GO_TO_SITE:
            f->terrain_usage = TERRAIN_USAGE_PREFER_ROADS;
            figure_movement_move_ticks(figure_id, speed);
            if (f->direction == DIRECTION_AT_DESTINATION) {
                int32_t building_id = map_grids.building_ids[f->destination_grid_offset];
                if (building_id) {
                    int32_t type = building_get(building_id)->type;
                    if (type == BUILDING_PYRAMID) {
                        f->action_state = FIGURE_ACTION_BRICKLAYER_CLIMB_RAMP;
                        pyramid_figure_start_climb(figure_id);
                    } else if (type == BUILDING_SPHINX) {
                        f->action_state = FIGURE_ACTION_BRICKLAYER_WORK;
                        f->wait_ticks = 0;
                    } else {
                        f->action_state = FIGURE_ACTION_BRICKLAYER_RETURN;
                    }
                } else if (building_get(f->destination_building_id)->type == BUILDING_PYRAMID && (map_grids.terrain[f->destination_grid_offset] & TERRAIN_MONUMENT)) {
                    f->action_state = FIGURE_ACTION_BRICKLAYER_WORK;
                    f->wait_ticks = 0;
                } else {
                    f->action_state = FIGURE_ACTION_BRICKLAYER_RETURN;
                }
            } else if (f->direction == DIRECTION_LOST || f->direction == DIRECTION_REROUTE) {
                if (f->direction == DIRECTION_REROUTE) {
                    figure_route_remove(figure_id);
                }
                f->action_state = FIGURE_ACTION_GUILD_WORKER_ROAM;
                f->wait_ticks = 0;
            }
            break;
        case FIGURE_ACTION_BRICKLAYER_WAIT_FOR_BLOCK:
            if (f->pyramid_walk_mode == 0) {
                f->action_state = FIGURE_ACTION_BRICKLAYER_RETURN;
                break;
            }
            if (++f->wait_ticks < 0) {
                f->wait_ticks = 0;
            }
            if (building_get(f->pyramid_work.part)->data.pyramid.worker_figure_id != figure_id) {
                f->action_state = FIGURE_ACTION_BRICKLAYER_RETURN;
                f->pyramid_walk_mode = 0;
            }
            break;
        case FIGURE_ACTION_BRICKLAYER_WORK: {
            int32_t building_id = map_grids.building_ids[f->destination_grid_offset];
            if (!building_id) {
                int32_t target_id = f->destination_building_id;
                if (building_get(target_id)->type == BUILDING_PYRAMID && (map_grids.terrain[f->destination_grid_offset] & TERRAIN_MONUMENT)) {
                    pyramid_build_complex_temple(target_id, figure_id, FIGURE_ACTION_BRICKLAYER_RETURN);
                } else {
                    f->action_state = FIGURE_ACTION_GUILD_WORKER_ROAM;
                    f->wait_ticks = 0;
                }
                break;
            }
            if (home->state != BUILDING_STATE_IN_USE) {
                f->state = FIGURE_STATE_DEAD;
                break;
            }
            building *b = building_get(building_id);
            if (b->type == BUILDING_PYRAMID) {
                if (b->data.pyramid.stage == 0) {
                    carries_block = 1;
                }
                pyramid_part_place_block(building_id, figure_id);
            } else if (b->type == BUILDING_SPHINX) {
                sphinx_figure_work_tick(building_id, figure_id, FIGURE_ACTION_BRICKLAYER_RETURN);
            } else {
                b->monument_remaining_blocks--;
                f->action_state = FIGURE_ACTION_BRICKLAYER_RETURN;
                set_destination_home_road(f, home);
            }
            break;
        }
        case FIGURE_ACTION_BRICKLAYER_CLIMB_RAMP:
            pyramid_figure_walk_on_ramp(figure_id, 0, FIGURE_ACTION_BRICKLAYER_RETURN, FIGURE_ACTION_BRICKLAYER_FIND_PART);
            break;
        case FIGURE_ACTION_BRICKLAYER_FIND_PART:
            pyramid_figure_find_next_part(
                figure_id, 1, FIGURE_ACTION_BRICKLAYER_RETURN, FIGURE_ACTION_BRICKLAYER_RETURN, FIGURE_ACTION_BRICKLAYER_WALK_TO_PART);
            break;
        case FIGURE_ACTION_BRICKLAYER_WALK_TO_PART:
            if (pyramid_figure_climb_step(figure_id, 1)) {
                pyramid_figure_snap_to_part(figure_id);
                f->action_state = building_get(f->pyramid_work.part)->data.pyramid.stage == 0 ? FIGURE_ACTION_BRICKLAYER_WAIT_FOR_BLOCK
                                                                                              : FIGURE_ACTION_BRICKLAYER_WORK;
                f->wait_ticks = 0;
            }
            break;
    }

    int32_t dir = image_direction(f);
    switch (f->action_state) {
        case FIGURE_ACTION_CORPSE:
            f->image_id = corpse_image(image_group(IMAGE_GROUP_FIGURE_BRICKLAYER_DEATH), f);
            break;
        case FIGURE_ACTION_BRICKLAYER_WAIT_FOR_BLOCK:
            waiting_mason_image(f, figure_id, image_group(IMAGE_GROUP_FIGURE_BRICKLAYER_BLOCK), 9, 9, dir);
            break;
        case FIGURE_ACTION_BRICKLAYER_WORK:
            if (carries_block && f->wait_ticks < pyramid_block_ticks_a) {
                if (f->animation_frame >= 9) {
                    f->animation_frame = 0;
                }
                f->image_id = image_group(IMAGE_GROUP_FIGURE_BRICKLAYER_BLOCK) + f->animation_frame * 8 + dir;
            } else {
                f->image_id = image_group(IMAGE_GROUP_FIGURE_BRICKLAYER_WORK) + (f->wait_ticks % 23) * 8 + dir;
            }
            break;
        default:
            f->image_id = image_group(IMAGE_GROUP_FIGURE_BRICKLAYER) + f->animation_frame * 8 + dir;
            break;
    }
}

void figure_stonemason_action(void)
{
    int32_t figure_id = figure_current_id;
    figure *f = figure_get(figure_id);
    int32_t carries_block = 0;
    if (f->wait_ticks < 0) {
        f->wait_ticks = 0;
    }
    if (++f->animation_frame >= GUILD_WORKER_ANIMATION_FRAMES) {
        f->animation_frame = 0;
    }
    f->terrain_usage = TERRAIN_USAGE_ROADS;
    f->use_cross_country = 0;
    f->is_ghost = 0;
    f->max_roam_length = (int16_t) guild_worker_rules[GUILD_WORKER_RULE_MAX_ROAM_TICKS];
    int32_t speed = figure_properties[f->type].speed;
    const building *home = building_get(f->building_id);
    if (home->state != BUILDING_STATE_IN_USE || home->type != BUILDING_STONEMASONS_GUILD) {
        f->state = FIGURE_STATE_DEAD;
        return;
    }
    if (f->pyramid_walk_mode > 0) {
        const building *part = building_get(f->pyramid_work.part);
        if (part->type != BUILDING_PYRAMID || part->state == BUILDING_STATE_UNUSED) {
            f->action_state = FIGURE_ACTION_STONEMASON_RETURN;
            f->pyramid_walk_mode = 0;
        }
    }
    switch (f->action_state) {
        case FIGURE_ACTION_CORPSE:
            figure_combat_handle_corpse(figure_id);
            if (f->state == FIGURE_STATE_DEAD) {
                decrease_guild_figure_count(f);
            }
            break;
        case FIGURE_ACTION_ATTACK:
            figure_combat_handle_attack(figure_id);
            break;
        case FIGURE_ACTION_ON_FERRY:
            f->image_id = 0;
            break;
        case FIGURE_ACTION_WAIT_FOR_FERRY:
            wait_for_ferry(f, figure_id);
            break;
        case FIGURE_ACTION_GUILD_WORKER_ROAM:
            look_for_task(f, figure_id, monument_find_task_stonemason, FIGURE_ACTION_STONEMASON_GO_TO_SITE);
            roam_near_guild(f, figure_id, FIGURE_ACTION_STONEMASON_RETURN);
            break;
        case FIGURE_ACTION_STONEMASON_GO_TO_SITE: {
            f->terrain_usage = TERRAIN_USAGE_ROADS_OR_MONUMENT;
            int32_t grid_offset = f->destination_grid_offset;
            int32_t building_id = map_grids.building_ids[grid_offset];
            figure_movement_move_ticks(figure_id, speed);
            if (f->direction == DIRECTION_AT_DESTINATION) {
                if (building_id) {
                    const building *b = building_get(building_id);
                    switch (b->type) {
                        case BUILDING_PYRAMID:
                            f->action_state = FIGURE_ACTION_STONEMASON_CLIMB_RAMP;
                            pyramid_figure_start_climb(figure_id);
                            break;
                        case BUILDING_SUN_TEMPLE:
                            f->action_state = b->subtype != 0 ? FIGURE_ACTION_STONEMASON_WAIT_FOR_BLOCK : FIGURE_ACTION_STONEMASON_WORK;
                            f->wait_ticks = 0;
                            break;
                        case BUILDING_SPHINX:
                        case BUILDING_SMALL_OBELISK:
                        case BUILDING_LARGE_OBELISK:
                        case BUILDING_SMALL_ROYAL_TOMB:
                        case BUILDING_MEDIUM_ROYAL_TOMB:
                        case BUILDING_LARGE_ROYAL_TOMB:
                        case BUILDING_GRAND_ROYAL_TOMB:
                        case BUILDING_ABU_SIMBEL:
                            f->action_state = FIGURE_ACTION_STONEMASON_WORK;
                            f->wait_ticks = 0;
                            break;
                        case BUILDING_CAESAREUM:
                        case BUILDING_LIGHTHOUSE:
                        case BUILDING_ALEXANDRIA_LIBRARY:
                        case BUILDING_MAUSOLEUM:
                            f->action_state = FIGURE_ACTION_STONEMASON_WAIT_FOR_BLOCK;
                            f->wait_ticks = 0;
                            break;
                        default:
                            f->action_state = FIGURE_ACTION_STONEMASON_RETURN;
                            break;
                    }
                } else {
                    int32_t target_id = f->destination_building_id;
                    if (building_get(target_id)->type != BUILDING_PYRAMID) {
                        f->action_state = FIGURE_ACTION_STONEMASON_RETURN;
                    } else if (map_grids.terrain[grid_offset] & TERRAIN_MONUMENT) {
                        f->action_state = FIGURE_ACTION_STONEMASON_WORK;
                        f->wait_ticks = 0;
                    } else {
                        pyramid_complex_queue_refresh(target_id);
                        pyramid_complex_process_refresh();
                        f->action_state = FIGURE_ACTION_STONEMASON_RETURN;
                    }
                }
            } else if (f->direction == DIRECTION_LOST || f->direction == DIRECTION_REROUTE) {
                int32_t target_type = building_get(f->destination_building_id)->type;
                if (target_type == BUILDING_PYRAMID && (map_grids.terrain[grid_offset] & TERRAIN_MONUMENT)) {
                    map_figure_delete(figure_id);
                    f->x = f->destination_x;
                    f->y = f->destination_y;
                    f->grid_offset = f->destination_grid_offset;
                    map_figure_add(figure_id);
                    f->action_state = FIGURE_ACTION_STONEMASON_WORK;
                    f->wait_ticks = 0;
                } else {
                    figure_route_remove(figure_id);
                    if (target_type != BUILDING_ALEXANDRIA_LIBRARY && target_type != BUILDING_ABU_SIMBEL && target_type != BUILDING_CAESAREUM &&
                        target_type != BUILDING_LIGHTHOUSE) {
                        f->action_state = FIGURE_ACTION_GUILD_WORKER_ROAM;
                    }
                    f->wait_ticks = 0;
                }
            }
            break;
        }
        case FIGURE_ACTION_STONEMASON_WAIT_FOR_BLOCK: {
            int32_t building_id = map_grids.building_ids[f->destination_grid_offset];
            if (++f->wait_ticks < 0) {
                f->wait_ticks = 0;
            }
            if (f->direction >= DIRECTION_COUNT) {
                random_generate_next();
                f->direction = (int8_t) random_generator_state.random1_3bit;
            }
            const building *b = building_get(building_id);
            switch (b->type) {
                case BUILDING_PYRAMID:
                    if (f->pyramid_walk_mode == 0) {
                        f->action_state = FIGURE_ACTION_STONEMASON_RETURN;
                    } else if (b->data.pyramid.worker_figure_id != figure_id) {
                        f->action_state = FIGURE_ACTION_STONEMASON_RETURN;
                        f->pyramid_walk_mode = 0;
                    }
                    break;
                case BUILDING_CAESAREUM:
                case BUILDING_LIGHTHOUSE:
                case BUILDING_ALEXANDRIA_LIBRARY:
                    if (building_get(building_id - b->subtype)->data.monument.worker_figure_id != figure_id) {
                        f->action_state = FIGURE_ACTION_STONEMASON_RETURN;
                    }
                    break;
                case BUILDING_SUN_TEMPLE:
                case BUILDING_SMALL_ROYAL_TOMB:
                case BUILDING_MEDIUM_ROYAL_TOMB:
                case BUILDING_LARGE_ROYAL_TOMB:
                case BUILDING_GRAND_ROYAL_TOMB:
                case BUILDING_MAUSOLEUM:
                    if (b->data.monument.worker_figure_id != figure_id) {
                        f->action_state = FIGURE_ACTION_STONEMASON_RETURN;
                    }
                    break;
                default:
                    figure_route_remove(figure_id);
                    f->action_state = FIGURE_ACTION_GUILD_WORKER_ROAM;
                    f->wait_ticks = 0;
                    break;
            }
            break;
        }
        case FIGURE_ACTION_STONEMASON_WORK: {
            int32_t building_id = map_grids.building_ids[f->destination_grid_offset];
            if (!building_id) {
                int32_t target_id = f->destination_building_id;
                if (building_get(target_id)->type == BUILDING_PYRAMID && (map_grids.terrain[f->destination_grid_offset] & TERRAIN_MONUMENT)) {
                    pyramid_build_complex_temple(target_id, figure_id, FIGURE_ACTION_STONEMASON_RETURN);
                } else {
                    f->action_state = FIGURE_ACTION_GUILD_WORKER_ROAM;
                    f->wait_ticks = 0;
                }
                break;
            }
            building *b = building_get(building_id);
            switch (b->type) {
                case BUILDING_PYRAMID:
                    if (b->data.pyramid.stage == 0) {
                        carries_block = 1;
                    }
                    pyramid_part_place_block(building_id, figure_id);
                    break;
                case BUILDING_SPHINX:
                    sphinx_figure_work_tick(building_id, figure_id, FIGURE_ACTION_STONEMASON_RETURN);
                    break;
                case BUILDING_SMALL_OBELISK:
                case BUILDING_LARGE_OBELISK:
                    obelisk_figure_work_tick(building_id, figure_id, FIGURE_ACTION_STONEMASON_RETURN);
                    break;
                case BUILDING_SUN_TEMPLE:
                    sun_temple_figure_work_tick(building_id, figure_id, FIGURE_ACTION_STONEMASON_RETURN);
                    break;
                case BUILDING_CAESAREUM:
                    caesareum_figure_work_tick(&caesareum_object.base, building_id, figure_id, FIGURE_ACTION_STONEMASON_RETURN);
                    break;
                case BUILDING_SMALL_ROYAL_TOMB:
                case BUILDING_MEDIUM_ROYAL_TOMB:
                case BUILDING_LARGE_ROYAL_TOMB:
                case BUILDING_GRAND_ROYAL_TOMB:
                    royal_tomb_figure_work_tick(royal_tomb_object_for_type(building_get(building_id)->type), building_id, figure_id, FIGURE_ACTION_STONEMASON_RETURN);
                    break;
                case BUILDING_LIGHTHOUSE:
                    lighthouse_figure_work_tick(&lighthouse_object.base, building_id, figure_id, FIGURE_ACTION_STONEMASON_RETURN);
                    break;
                case BUILDING_ALEXANDRIA_LIBRARY:
                    library_figure_work_tick(&library_object.base, building_id, figure_id, FIGURE_ACTION_STONEMASON_RETURN);
                    break;
                case BUILDING_ABU_SIMBEL:
                    abu_simbel_figure_work_tick(&abu_simbel_object.base, building_id, figure_id, FIGURE_ACTION_STONEMASON_RETURN);
                    break;
                case BUILDING_MAUSOLEUM:
                    mausoleum_figure_work_tick(building_id, figure_id, FIGURE_ACTION_STONEMASON_RETURN);
                    break;
                default:
                    b->monument_remaining_blocks--;
                    f->action_state = FIGURE_ACTION_STONEMASON_RETURN;
                    set_destination_home_road(f, home);
                    break;
            }
            break;
        }
        case FIGURE_ACTION_STONEMASON_RETURN:
            if (f->destination_x != home->road_access_x || f->destination_y != home->road_access_y) {
                set_destination_home_road(f, home);
                figure_route_remove(figure_id);
            }
            return_and_disappear(f, figure_id, speed);
            break;
        case FIGURE_ACTION_STONEMASON_CLIMB_RAMP:
            pyramid_figure_walk_on_ramp(figure_id, 0, FIGURE_ACTION_STONEMASON_RETURN, FIGURE_ACTION_STONEMASON_FIND_PART);
            break;
        case FIGURE_ACTION_STONEMASON_FIND_PART:
            pyramid_figure_find_next_part(
                figure_id, 1, FIGURE_ACTION_STONEMASON_RETURN, FIGURE_ACTION_STONEMASON_NEXT_CASING_PART, FIGURE_ACTION_STONEMASON_WALK_TO_PART);
            break;
        case FIGURE_ACTION_STONEMASON_WALK_TO_PART:
            if (pyramid_figure_climb_step(figure_id, 1)) {
                pyramid_figure_snap_to_part(figure_id);
                f->action_state = building_get(f->pyramid_work.part)->data.pyramid.stage != 0 ? FIGURE_ACTION_STONEMASON_WORK
                                                                                              : FIGURE_ACTION_STONEMASON_WAIT_FOR_BLOCK;
                f->wait_ticks = 0;
            }
            break;
        case FIGURE_ACTION_STONEMASON_NEXT_CASING_PART:
            pyramid_stonemason_arrive_part(figure_id);
            break;
    }

    int32_t dir = image_direction(f);
    switch (f->action_state) {
        case FIGURE_ACTION_CORPSE:
            f->image_id = corpse_image(image_group(IMAGE_GROUP_FIGURE_STONEMASON_DEATH), f);
            break;
        case FIGURE_ACTION_STONEMASON_WAIT_FOR_BLOCK:
            waiting_mason_image(f, figure_id, image_group(IMAGE_GROUP_FIGURE_STONEMASON_BLOCK), 12, 12, dir);
            break;
        case FIGURE_ACTION_STONEMASON_WORK:
            if (carries_block && f->wait_ticks < pyramid_block_ticks_a) {
                f->image_id = image_group(IMAGE_GROUP_FIGURE_STONEMASON_BLOCK) + f->animation_frame * 8 + dir;
            } else {
                if (f->animation_frame >= 7) {
                    f->animation_frame = 0;
                }
                f->image_id = image_group(IMAGE_GROUP_FIGURE_STONEMASON_WORK) + f->animation_frame * 8 + dir;
            }
            break;
        default:
            f->image_id = image_group(IMAGE_GROUP_FIGURE_STONEMASON) + f->animation_frame * 8 + dir;
            break;
    }
}

void figure_tomb_artisan_action(void)
{
    int32_t figure_id = figure_current_id;
    figure *f = figure_get(figure_id);
    if (!worker_begin(f, BUILDING_ARTISANS_GUILD)) {
        return;
    }
    int32_t speed = figure_properties[f->type].speed;
    building *home = building_get(f->building_id);
    switch (f->action_state) {
        case FIGURE_ACTION_CORPSE:
            figure_combat_handle_corpse(figure_id);
            if (f->state == FIGURE_STATE_DEAD) {
                home->data.industry.worker_count--;
                home->data.industry.reserved_2 = 0;
            }
            break;
        case FIGURE_ACTION_ATTACK:
            f->is_ghost = 0;
            figure_combat_handle_attack(figure_id);
            break;
        case FIGURE_ACTION_ON_FERRY:
            f->image_id = 0;
            break;
        case FIGURE_ACTION_WAIT_FOR_FERRY:
            wait_for_ferry(f, figure_id);
            break;
        case FIGURE_ACTION_GUILD_WORKER_ROAM:
            if (f->is_ghost) {
                if (++f->wait_ticks > guild_worker_rules[GUILD_WORKER_RULE_GHOST_TICKS]) {
                    f->is_ghost = 0;
                }
                break;
            }
            f->wait_ticks++;
            random_generate_next();
            if (f->wait_ticks > ((random_generator_state.random1_15bit % guild_worker_rules[GUILD_WORKER_RULE_TASK_SEARCH_RANDOM_TICKS]) +
                                    guild_worker_rules[GUILD_WORKER_RULE_TASK_SEARCH_BASE_TICKS])) {
                if (figure_tomb_artisan_find_paint_task(f->x, f->y, figure_id) || figure_tomb_artisan_find_lamp_task(f->x, f->y, figure_id)) {
                    f->action_state = FIGURE_ACTION_TOMB_ARTISAN_GO_TO_SITE;
                    figure_route_remove(figure_id);
                    f->roam_length = 0;
                }
                f->wait_ticks = 0;
            }
            roam_near_guild(f, figure_id, FIGURE_ACTION_TOMB_ARTISAN_RETURN);
            break;
        case FIGURE_ACTION_TOMB_ARTISAN_GO_TO_SITE: {
            f->is_ghost = 0;
            f->terrain_usage = TERRAIN_USAGE_PREFER_ROADS;
            int32_t building_id = map_grids.building_ids[f->destination_grid_offset];
            int32_t to_royal_tomb = building_id && is_royal_tomb(building_get(building_id)->type);
            if (to_royal_tomb) {
                f->terrain_usage = TERRAIN_USAGE_ROADS_OR_MONUMENT;
            }
            figure_movement_move_ticks(figure_id, speed);
            if (f->direction == DIRECTION_AT_DESTINATION) {
                if (to_royal_tomb) {
                    f->action_state = FIGURE_ACTION_TOMB_ARTISAN_WORK;
                    f->wait_ticks = 0;
                } else {
                    f->action_state = FIGURE_ACTION_TOMB_ARTISAN_RETURN;
                }
            } else if (f->direction == DIRECTION_LOST) {
                f->action_state = FIGURE_ACTION_GUILD_WORKER_ROAM;
                f->wait_ticks = 0;
            } else if (f->direction == DIRECTION_REROUTE) {
                figure_route_remove(figure_id);
            }
            break;
        }
        case FIGURE_ACTION_TOMB_ARTISAN_WORK: {
            f->is_ghost = 0;
            int32_t building_id = map_grids.building_ids[f->destination_grid_offset];
            if (!building_id) {
                f->action_state = FIGURE_ACTION_GUILD_WORKER_ROAM;
                f->wait_ticks = 0;
                break;
            }
            if (home->state != BUILDING_STATE_IN_USE) {
                f->state = FIGURE_STATE_DEAD;
                break;
            }
            switch (building_get(building_id)->type) {
                case BUILDING_SMALL_ROYAL_TOMB:
                    royal_tomb_figure_work_tick(&small_royal_tomb_object.base, building_id, figure_id, FIGURE_ACTION_TOMB_ARTISAN_RETURN);
                    break;
                case BUILDING_MEDIUM_ROYAL_TOMB:
                    royal_tomb_figure_work_tick(&medium_royal_tomb_object.base, building_id, figure_id, FIGURE_ACTION_TOMB_ARTISAN_RETURN);
                    break;
                case BUILDING_LARGE_ROYAL_TOMB:
                    royal_tomb_figure_work_tick(&large_royal_tomb_object.base, building_id, figure_id, FIGURE_ACTION_TOMB_ARTISAN_RETURN);
                    break;
                case BUILDING_GRAND_ROYAL_TOMB:
                    royal_tomb_figure_work_tick(&medium_royal_tomb_object.base, building_id, figure_id, FIGURE_ACTION_TOMB_ARTISAN_RETURN);
                    break;
                default:
                    f->action_state = FIGURE_ACTION_TOMB_ARTISAN_RETURN;
                    set_destination_home_road(f, home);
                    break;
            }
            break;
        }
        case FIGURE_ACTION_TOMB_ARTISAN_RETURN:
            f->is_ghost = 0;
            f->terrain_usage = TERRAIN_USAGE_ROADS_OR_MONUMENT;
            return_and_disappear(f, figure_id, speed);
            break;
    }

    int32_t dir = image_direction(f);
    if (f->action_state == FIGURE_ACTION_CORPSE) {
        f->image_id = corpse_image(image_group(IMAGE_GROUP_FIGURE_TOMB_ARTISAN_DEATH), f);
    } else if (f->action_state == FIGURE_ACTION_TOMB_ARTISAN_WORK) {
        f->image_id = image_group(IMAGE_GROUP_FIGURE_TOMB_ARTISAN_WORK) + (f->animation_frame / 2) * 8 + dir;
    } else {
        f->image_id = image_group(IMAGE_GROUP_FIGURE_TOMB_ARTISAN) + f->animation_frame * 8 + dir;
    }
}

int32_t monument_find_task_carpenter(int32_t x, int32_t y, int32_t figure_id)
{
    int32_t building_id;
    if ((building_id = pyramid_find_ramp_task(figure_id)) != 0 || (building_id = sphinx_find_work_for_figure(figure_id)) != 0 ||
        (building_id = obelisk_find_work_for_figure(figure_id)) != 0 ||
        (building_id = caesareum_find_work_for_figure(&caesareum_object.base, x, y, figure_id)) != 0 ||
        (building_id = lighthouse_find_work_for_figure(&lighthouse_object.base, x, y, figure_id)) != 0 ||
        (building_id = sun_temple_find_work_for_figure(figure_id)) != 0 ||
        (building_id = library_find_work_for_figure(&library_object.base, x, y, figure_id)) != 0 ||
        (building_id = abu_simbel_find_work_for_figure(&abu_simbel_object.base, x, y, figure_id)) != 0) {
        return building_id;
    }
    return mausoleum_find_work_for_figure(figure_id);
}

int32_t monument_find_task_stonemason(int32_t x, int32_t y, int32_t figure_id)
{
    int32_t building_id;
    if ((building_id = pyramid_find_task_stonemason(figure_id)) != 0 || (building_id = sphinx_find_work_for_figure(figure_id)) != 0 ||
        (building_id = obelisk_find_work_for_figure(figure_id)) != 0 || (building_id = sun_temple_find_work_for_figure(figure_id)) != 0 ||
        (building_id = library_find_work_for_figure(&library_object.base, x, y, figure_id)) != 0 ||
        (building_id = abu_simbel_find_work_for_figure(&abu_simbel_object.base, x, y, figure_id)) != 0 ||
        (building_id = lighthouse_find_work_for_figure(&lighthouse_object.base, x, y, figure_id)) != 0 ||
        (building_id = small_royal_tomb_find_work_for_figure(&small_royal_tomb_object.base, x, y, figure_id)) != 0 ||
        (building_id = medium_royal_tomb_find_work_for_figure(&medium_royal_tomb_object.base, x, y, figure_id)) != 0 ||
        (building_id = large_royal_tomb_find_work_for_figure(&large_royal_tomb_object.base, x, y, figure_id)) != 0 ||
        (building_id = grand_royal_tomb_find_work_for_figure(&grand_royal_tomb_object.base, x, y, figure_id)) != 0 ||
        (building_id = caesareum_find_work_for_figure(&caesareum_object.base, x, y, figure_id)) != 0) {
        return building_id;
    }
    return mausoleum_find_work_for_figure(figure_id);
}

static int32_t find_tomb_artisan_task(int32_t figure_id, int32_t stage)
{
    for (int32_t id = 1; id < MAX_BUILDINGS; id++) {
        const building *b = building_get(id);
        if (b->state != BUILDING_STATE_IN_USE) {
            continue;
        }
        tomb_object *object = royal_tomb_object_for_type(b->type);
        if (!object) {
            continue;
        }
        if (b->data.royal_tomb.stage == stage && royal_tomb_check_work_for_figure(object, id, figure_id, 1)) {
            return id;
        }
    }
    return 0;
}

int32_t figure_tomb_artisan_find_paint_task(int32_t x, int32_t y, int32_t figure_id)
{
    (void) x;
    (void) y;
    return find_tomb_artisan_task(figure_id, TOMB_ARTISAN_PAINT_STAGE);
}

int32_t figure_tomb_artisan_find_lamp_task(int32_t x, int32_t y, int32_t figure_id)
{
    (void) x;
    (void) y;
    return find_tomb_artisan_task(figure_id, TOMB_ARTISAN_LAMP_STAGE);
}

int32_t figure_bricklayer_find_monument_task(int32_t x, int32_t y, int32_t figure_id)
{
    return pyramid_find_task_bricklayer(figure_id);
}

static int32_t guild_can_spawn(int32_t building_id, int32_t one_below_25)
{
    building *b = building_get(building_id);
    if (b->houses_covered <= 0) {
        b->show_on_problem_overlay = 2;
    }
    if (!map_has_road_access(b->x, b->y, b->size)) {
        return 0;
    }
    if (b->houses_covered <= 50) {
        generate_labor_seeker(building_id);
    }
    int32_t percentage = calc_percentage(b->num_workers, building_model_table[b->type].laborers);
    int32_t max_workers;
    if (percentage >= 100) {
        max_workers = 4;
    } else if (percentage >= 75) {
        max_workers = 3;
    } else if (percentage >= 50) {
        max_workers = 2;
    } else if (percentage >= 25 || (one_below_25 && percentage >= 1)) {
        max_workers = 1;
    } else {
        return 0;
    }
    if (b->data.industry.worker_count >= max_workers) {
        return 0;
    }
    b->figure_spawn_delay = 0;
    return b->data.industry.worker_count < max_workers;
}

static int32_t guild_create_worker(int32_t building_id, int32_t figure_type)
{
    building *b = building_get(building_id);
    int32_t figure_id = figure_create(b->player, figure_type, map_result_x, map_result_y, 0);
    figure *f = figure_get(figure_id);
    f->action_state = FIGURE_ACTION_GUILD_WORKER_ROAM;
    f->building_id = (int16_t) building_id;
    f->wait_ticks = 0;
    return figure_id;
}

static void guild_send_worker(int32_t figure_id, int16_t *stock)
{
    figure_get(figure_id)->action_state = FIGURE_ACTION_CARPENTER_GO_TO_SITE;
    figure_route_remove(figure_id);
    *stock -= (int16_t) guild_worker_rules[GUILD_WORKER_RULE_LOAD];
    figure_get(figure_current_id)->roam_length = 0;
    if (*stock < 0) {
        *stock = 0;
    }
}

static void guild_spawn_ghost(int32_t figure_id)
{
    figure *f = figure_get(figure_id);
    f->is_ghost = 1;
    f->action_state = FIGURE_ACTION_CORPSE;
}

void building_carpenters_guild_spawn_figure(int32_t building_id)
{
    if (!guild_can_spawn(building_id, 1)) {
        return;
    }
    building *b = building_get(building_id);
    if (b->data.industry.progress < guild_worker_rules[GUILD_WORKER_RULE_CARPENTER_PROGRESS]) {
        return;
    }
    int32_t figure_id = guild_create_worker(building_id, FIGURE_CARPENTER);
    b->data.industry.worker_count++;
    figure_movement_init_roaming(figure_id);
    figure *f = figure_get(figure_id);
    f->roam_length = 0;
    if (monument_find_task_carpenter(f->x, f->y, figure_id)) {
        b->data.industry.progress = 0;
        guild_send_worker(figure_id, &b->loads_stored);
    } else {
        guild_spawn_ghost(figure_id);
    }
}

void building_stonemasons_guild_spawn_figure(int32_t building_id)
{
    if (!guild_can_spawn(building_id, 0)) {
        return;
    }
    building *b = building_get(building_id);
    int32_t figure_id = guild_create_worker(building_id, FIGURE_STONEMASON);
    figure_movement_init_roaming(figure_id);
    b->data.industry.worker_count++;
    figure_get(figure_id)->roam_length = 0;
    b->data.industry.progress = 0;
}

void building_bricklayers_guild_spawn_figure(int32_t building_id)
{
    if (!guild_can_spawn(building_id, 0)) {
        return;
    }
    building *b = building_get(building_id);
    int32_t figure_id = guild_create_worker(building_id, FIGURE_BRICKLAYER);
    figure_movement_init_roaming(figure_id);
    b->data.industry.worker_count++;
    figure_get(figure_id)->roam_length = 0;
    b->data.industry.progress = 0;
    b->loads_stored = 0;
}

void building_artisans_guild_spawn_figure(int32_t building_id)
{
    if (!guild_can_spawn(building_id, 0)) {
        return;
    }
    building *b = building_get(building_id);
    int32_t figure_id = guild_create_worker(building_id, FIGURE_TOMB_ARTISAN);
    figure_movement_init_roaming(figure_id);
    b->data.industry.worker_count++;
    figure *f = figure_get(figure_id);
    f->roam_length = 0;
    b->data.industry.progress = 0;
    if (b->second_input_stock >= guild_worker_rules[GUILD_WORKER_RULE_LOAD] && figure_tomb_artisan_find_paint_task(f->x, f->y, figure_id)) {
        guild_send_worker(figure_id, &b->second_input_stock);
    } else if (b->loads_stored >= guild_worker_rules[GUILD_WORKER_RULE_LOAD] && figure_tomb_artisan_find_lamp_task(f->x, f->y, figure_id)) {
        guild_send_worker(figure_id, &b->loads_stored);
    } else {
        guild_spawn_ghost(figure_id);
    }
}

