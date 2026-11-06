// Service walkers: the generic roamer and the walkers that roam out of their building giving a service (labor seekers,
// tax collectors, engineers, firemen, constables, magistrates, entertainers, priests, teachers, health walkers, market
// traders, water carriers, missionaries, festival walkers), the worker and the showman. The coverage itself is given
// by the movement (figure/service.c); the tunable values are walker_rules from config/walker_rules.json.
//
// The venue and ruin searches, and one image, behave unusually:
// - the routed venue search skips a venue with a juggler, senet or zookeeper show for those three alike;
// - the distance venue search adds only the juggler's, musician's and dancer's days;
// - a burning ruin already fought counts only within 3 tiles when no other is found;
// - the missionary draws with the labor seeker's image groups.

#include "figure/walkers.h"
#include "building/building.h"
#include "building/building_data.h"
#include "building/building_figure_helpers.h"
#include "building/building_type.h"
#include "building/entertainment.h"
#include "city/city_player.h"
#include "city/gods.h"
#include "core/calc.h"
#include "core/crt_compat.h"
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
#include "map/map_data.h"
#include "map/road_access.h"
#include "map/routing.h"
#include "map/routing_distance.h"
#include "map/routing_land.h"
#include "sound/sound_play.h"
#include <stdint.h>

int32_t burning_ruin_distance;

int32_t enemy_candidates_offsets[100];

int32_t enemy_candidates_ids[100];

int32_t enemy_candidates_count;

int32_t walker_rules[WALKER_RULE_COUNT] = {
    [WALKER_RULE_ROAMER_ROAM_TICKS] = 384,
    [WALKER_RULE_PATRICIAN_ROAM_TICKS] = 128,
    [WALKER_RULE_MISSIONARY_ROAM_TICKS] = 192,
    [WALKER_RULE_TAX_COLLECTOR_ROAM_TICKS] = 512,
    [WALKER_RULE_ENTERTAINER_ROAM_TICKS] = 512,
    [WALKER_RULE_ENGINEER_ROAM_TICKS] = 640,
    [WALKER_RULE_FIREMAN_ROAM_TICKS] = 640,
    [WALKER_RULE_LAW_WALKER_ROAM_TICKS] = 640,
    [WALKER_RULE_MARKET_TRADER_EMPTY_TURN_TICKS] = 96,
    [WALKER_RULE_ENTERTAINER_VENUE_WALK_TICKS] = 3200,
    [WALKER_RULE_HOME_ROAD_RADIUS] = 2,
    [WALKER_RULE_FERRY_WAIT_TICKS] = 100,
    [WALKER_RULE_FIRE_SEARCH_TICKS] = 20,
    [WALKER_RULE_FIRE_SEARCH_DISTANCE] = 25,
    [WALKER_RULE_FIRE_FIGHT_TICKS] = 50,
    [WALKER_RULE_FIRE_OUT_DURATION] = 32,
    [WALKER_RULE_ENEMY_SEARCH_TICKS] = 10,
    [WALKER_RULE_ENEMY_SEARCH_DISTANCE] = 30,
    [WALKER_RULE_FESTIVAL_TRIPS] = 10,
};

static int32_t image_direction(int32_t direction)
{
    int32_t dir = direction - map_info.orientation;
    if (dir < 0) {
        dir += DIRECTION_COUNT;
    }
    figure_image_direction = dir;
    return dir;
}

static int32_t corpse_offset(const figure *f)
{
    return figure_corpse_offsets[f->wait_ticks / 2];
}

static int32_t advance_animation(figure *f)
{
    if (++f->animation_frame >= WALKER_ANIMATION_FRAMES) {
        f->animation_frame = 0;
        return 1;
    }
    return 0;
}

static void set_walker_image(int32_t figure_id, int32_t walk_base, int32_t corpse_base)
{
    figure *f = figure_get(figure_id);
    int32_t dir = image_direction(f->direction);
    if (f->action_state == FIGURE_ACTION_CORPSE) {
        f->image_id = corpse_base + corpse_offset(f);
    } else {
        f->image_id = walk_base + f->animation_frame * 8 + dir;
    }
}

static void set_destination_to_found_road(figure *f)
{
    f->destination_x = (int16_t) map_result_x;
    f->destination_y = (int16_t) map_result_y;
}

static int32_t temple_is_blocked(const building *temple)
{
    return temple->fire_duration != 0;
}

static int32_t home_road(const building *b)
{
    return map_closest_road_within_radius(b->x, b->y, b->size, walker_rules[WALKER_RULE_HOME_ROAD_RADIUS]);
}

void figure_roamer_action(void)
{
    int32_t figure_id = figure_current_id;
    figure *f = figure_get(figure_id);
    int32_t speed = figure_properties[f->type].speed;
    const building *home = building_get(f->building_id);
    switch (f->action_state) {
        case FIGURE_ACTION_ATTACK:
            figure_combat_handle_attack(figure_id);
            return;
        case FIGURE_ACTION_CORPSE:
            figure_combat_handle_corpse(figure_id);
            return;
        case FIGURE_ACTION_ROAMER_ROAM:
            f->is_ghost = 0;
            if (++f->roam_length >= f->max_roam_length) {
                if (!home_road(home)) {
                    f->state = FIGURE_STATE_DEAD;
                    figure_movement_roam_ticks(figure_current_id, 1);
                    return;
                }
                f->action_state = FIGURE_ACTION_ROAMER_RETURN;
                set_destination_to_found_road(f);
                figure_route_remove(figure_id);
                f->roam_length = 0;
            }
            figure_movement_roam_ticks(figure_id, 1);
            return;
        case FIGURE_ACTION_ROAMER_RETURN:
            figure_movement_move_ticks(figure_id, speed);
            if (f->direction == DIRECTION_REROUTE || f->direction == DIRECTION_LOST || f->direction == DIRECTION_AT_DESTINATION) {
                f->state = FIGURE_STATE_DEAD;
            }
            return;
        default:
            f->action_state = FIGURE_ACTION_ROAMER_ROAM;
            f->roam_length = f->max_roam_length;
            return;
    }
}

static figure *roamer_begin(int32_t max_roam)
{
    figure *f = figure_get(figure_current_id);
    f->terrain_usage = TERRAIN_USAGE_ROADS;
    f->use_cross_country = 0;
    f->max_roam_length = (int16_t) max_roam;
    return f;
}

static void plain_roamer_action(int32_t walk_group, int32_t corpse_group, int32_t in_second_walker_slot)
{
    int32_t figure_id = figure_current_id;
    figure *f = roamer_begin(walker_rules[WALKER_RULE_ROAMER_ROAM_TICKS]);
    const building *b = building_get(f->building_id);
    int32_t walker_id = in_second_walker_slot ? b->figure_id2 : b->figure_id;
    if (b->state != BUILDING_STATE_IN_USE || walker_id != figure_id) {
        f->state = FIGURE_STATE_DEAD;
    }
    advance_animation(f);
    figure_roamer_action();
    set_walker_image(figure_current_id, image_group(walk_group), image_group(corpse_group));
}

void figure_teacher_action(void)
{
    plain_roamer_action(IMAGE_GROUP_FIGURE_TEACHER, IMAGE_GROUP_FIGURE_TEACHER_DEATH, 0);
}

void figure_librarian_action(void)
{
    plain_roamer_action(IMAGE_GROUP_FIGURE_LIBRARIAN, IMAGE_GROUP_FIGURE_LIBRARIAN_DEATH, 0);
}

void figure_dentist_action(void)
{
    plain_roamer_action(IMAGE_GROUP_FIGURE_DENTIST, IMAGE_GROUP_FIGURE_DENTIST_DEATH, 0);
}

void figure_physician_action(void)
{
    plain_roamer_action(IMAGE_GROUP_FIGURE_PHYSICIAN, IMAGE_GROUP_FIGURE_PHYSICIAN_DEATH, 0);
}

void figure_apothecary_action(void)
{
    plain_roamer_action(IMAGE_GROUP_FIGURE_APOTHECARY, IMAGE_GROUP_FIGURE_APOTHECARY_DEATH, 0);
}

void figure_embalmer_action(void)
{
    plain_roamer_action(IMAGE_GROUP_FIGURE_EMBALMER, IMAGE_GROUP_FIGURE_EMBALMER_DEATH, 0);
}

void figure_water_carrier_action(void)
{
    plain_roamer_action(IMAGE_GROUP_FIGURE_WATER_CARRIER, IMAGE_GROUP_FIGURE_WATER_CARRIER_DEATH, 0);
}

void figure_labor_seeker_action(void)
{
    plain_roamer_action(IMAGE_GROUP_FIGURE_LABOR_SEEKER, IMAGE_GROUP_FIGURE_LABOR_SEEKER_DEATH, 1);
}

void figure_market_trader_action(void)
{
    int32_t figure_id = figure_current_id;
    figure *f = roamer_begin(walker_rules[WALKER_RULE_ROAMER_ROAM_TICKS]);
    int32_t market_id = f->building_id;
    const building *market = building_get(market_id);
    if (market->state != BUILDING_STATE_IN_USE) {
        f->state = FIGURE_STATE_DEAD;
    }
    if (market->figure_id != figure_id && market->data.market.second_trader_id != figure_id) {
        f->state = FIGURE_STATE_DEAD;
    }
    advance_animation(f);
    if (f->action_state == FIGURE_ACTION_ROAMER_ROAM) {
        int32_t stock = building_market_get_max_food_stock(market_id);
        stock += building_market_get_max_goods_stock(market_id);
        if (f->roam_length >= walker_rules[WALKER_RULE_MARKET_TRADER_EMPTY_TURN_TICKS] && stock <= 0) {
            f->roam_length = f->max_roam_length;
        }
    }
    figure_roamer_action();
    set_walker_image(figure_current_id, image_group(IMAGE_GROUP_FIGURE_MARKET_TRADER), image_group(IMAGE_GROUP_FIGURE_MARKET_TRADER_DEATH));
}

void figure_patrician_action(void)
{
    figure *f = roamer_begin(walker_rules[WALKER_RULE_PATRICIAN_ROAM_TICKS]);
    if (building_get(f->building_id)->state != BUILDING_STATE_IN_USE) {
        f->state = FIGURE_STATE_DEAD;
    }
    advance_animation(f);
    figure_roamer_action();
    set_walker_image(figure_current_id, image_group(IMAGE_GROUP_FIGURE_PATRICIAN), image_group(IMAGE_GROUP_FIGURE_PATRICIAN_DEATH));
}

void figure_priest_action(void)
{
    int32_t figure_id = figure_current_id;
    figure *f = roamer_begin(walker_rules[WALKER_RULE_ROAMER_ROAM_TICKS]);
    int32_t temple_id = f->building_id;
    const building *temple = building_get(temple_id);
    if (temple->state != BUILDING_STATE_IN_USE || temple_is_blocked(temple) || temple->figure_id != figure_id) {
        f->state = FIGURE_STATE_DEAD;
        return;
    }
    advance_animation(f);
    if (f->action_state == FIGURE_ACTION_ROAMER_CREATED) {
        f->action_state = FIGURE_ACTION_ROAMER_ROAM;
    } else {
        figure_roamer_action();
    }
    image_direction(f->direction);
    int32_t walk;
    int32_t corpse;
    switch (building_temple_god(temple_id)) {
        case GOD_RA:
            walk = IMAGE_GROUP_FIGURE_PRIEST_RA;
            corpse = IMAGE_GROUP_FIGURE_PRIEST_RA_DEATH;
            break;
        case GOD_PTAH:
            walk = IMAGE_GROUP_FIGURE_PRIEST_PTAH;
            corpse = IMAGE_GROUP_FIGURE_PRIEST_PTAH_DEATH;
            break;
        case GOD_SETH:
            walk = IMAGE_GROUP_FIGURE_PRIEST_SETH;
            corpse = IMAGE_GROUP_FIGURE_PRIEST_SETH_DEATH;
            break;
        case GOD_BAST:
            walk = IMAGE_GROUP_FIGURE_PRIEST_BAST;
            corpse = IMAGE_GROUP_FIGURE_PRIEST_BAST_DEATH;
            break;
        default:
            walk = IMAGE_GROUP_FIGURE_PRIEST_OSIRIS;
            corpse = IMAGE_GROUP_FIGURE_PRIEST_OSIRIS_DEATH;
            break;
    }
    if (f->action_state == FIGURE_ACTION_CORPSE) {
        f->image_id = image_group(corpse) + corpse_offset(f);
    } else {
        f->image_id = image_group(walk) + f->animation_frame * 8 + figure_image_direction;
    }
}

static void set_cross_country_to_destination(int32_t figure_id)
{
    figure *f = figure_get(figure_id);
    figure_movement_set_cross_country_direction(figure_id, f->cross_country.x, f->cross_country.y, f->destination_x * 15, f->destination_y * 15, 0);
}

static void enter_exit(figure *f, int32_t home_id)
{
    f->use_cross_country = 1;
    f->is_ghost = 1;
    if (figure_movement_move_ticks_cross_country(figure_current_id, 1) != 1) {
        return;
    }
    if (map_grids.building_ids[f->grid_offset] == home_id) {
        f->state = FIGURE_STATE_DEAD;
        return;
    }
    f->action_state = FIGURE_ACTION_SERVICE_ROAMING;
    figure_movement_init_roaming(figure_current_id);
    f->roam_length = 0;
}

typedef struct {
    int32_t max_roam_rule;
    int32_t exit_to_road_access;
    int32_t return_route_remove;
} building_walker;

static figure *building_walker_begin(const building_walker *type, int32_t *speed, int32_t *wrapped)
{
    int32_t figure_id = figure_current_id;
    figure *f = roamer_begin(walker_rules[type->max_roam_rule]);
    *speed = figure_properties[f->type].speed;
    const building *home = building_get(f->building_id);
    if (home->state != BUILDING_STATE_IN_USE || home->figure_id != figure_id) {
        f->state = FIGURE_STATE_DEAD;
    }
    *wrapped = advance_animation(f);
    return f;
}

static int32_t building_walker_state(figure *f, int32_t speed, const building_walker *type)
{
    int32_t figure_id = figure_current_id;
    int32_t home_id = f->building_id;
    const building *home = building_get(home_id);
    switch (f->action_state) {
        case FIGURE_ACTION_SERVICE_CREATED:
            f->is_ghost = 1;
            f->animation_frame = 0;
            if (--f->wait_ticks > 0) {
                return 1;
            }
            if (!home_road(home)) {
                f->state = FIGURE_STATE_DEAD;
                return 1;
            }
            f->action_state = FIGURE_ACTION_SERVICE_ENTER_EXIT;
            if (type->exit_to_road_access) {
                f->destination_x = home->road_access_x;
                f->destination_y = home->road_access_y;
            } else {
                set_destination_to_found_road(f);
            }
            set_cross_country_to_destination(figure_current_id);
            f->roam_length = 0;
            return 1;
        case FIGURE_ACTION_SERVICE_ENTER_EXIT:
            enter_exit(f, home_id);
            return 1;
        case FIGURE_ACTION_SERVICE_ROAMING:
            f->is_ghost = 0;
            if (++f->roam_length >= f->max_roam_length) {
                if (!home_road(home)) {
                    f->state = FIGURE_STATE_DEAD;
                } else {
                    f->action_state = FIGURE_ACTION_SERVICE_RETURNING;
                    set_destination_to_found_road(f);
                    if (type->return_route_remove) {
                        figure_route_remove(figure_current_id);
                    }
                }
            }
            figure_movement_roam_ticks(figure_current_id, 1);
            return 1;
        case FIGURE_ACTION_SERVICE_RETURNING:
            figure_movement_move_ticks(figure_id, speed);
            if (f->direction == DIRECTION_REROUTE || f->direction == DIRECTION_LOST) {
                f->state = FIGURE_STATE_DEAD;
            } else if (f->direction == DIRECTION_AT_DESTINATION) {
                f->action_state = FIGURE_ACTION_SERVICE_ENTER_EXIT;
                f->destination_x = home->x;
                f->destination_y = home->y;
                set_cross_country_to_destination(figure_current_id);
                f->roam_length = 0;
            }
            return 1;
    }
    return 0;
}

static void building_walker_action(const building_walker *type, int32_t reset_wait_ticks, int32_t walk_group, int32_t corpse_group)
{
    int32_t speed;
    int32_t wrapped;
    figure *f = building_walker_begin(type, &speed, &wrapped);
    int32_t figure_id = figure_current_id;
    if (f->action_state == FIGURE_ACTION_ATTACK) {
        figure_combat_handle_attack(figure_id);
    } else if (f->action_state == FIGURE_ACTION_CORPSE) {
        figure_combat_handle_corpse(figure_id);
    } else if (!building_walker_state(f, speed, type)) {
        f->action_state = FIGURE_ACTION_SERVICE_CREATED;
        if (reset_wait_ticks) {
            f->wait_ticks = 0;
        }
    }
    set_walker_image(figure_current_id, image_group(walk_group), image_group(corpse_group));
}

void figure_tax_collector_action(void)
{
    static const building_walker tax_collector = { WALKER_RULE_TAX_COLLECTOR_ROAM_TICKS, 0, 0 };
    building_walker_action(&tax_collector, 0, IMAGE_GROUP_FIGURE_TAX_COLLECTOR, IMAGE_GROUP_FIGURE_TAX_COLLECTOR_DEATH);
}

void figure_engineer_action(void)
{
    static const building_walker engineer = { WALKER_RULE_ENGINEER_ROAM_TICKS, 1, 0 };
    building_walker_action(&engineer, 1, IMAGE_GROUP_FIGURE_ENGINEER, IMAGE_GROUP_FIGURE_ENGINEER_DEATH);
}

static int32_t entertainer_venue(int32_t figure_type, int32_t *first, int32_t *second, int32_t *third)
{
    *first = 0;
    *second = 0;
    *third = 0;
    if (figure_type == FIGURE_JUGGLER) {
        *first = BUILDING_BOOTH;
        *second = BUILDING_BANDSTAND;
        *third = BUILDING_PAVILION;
        return 1;
    }
    if (figure_type == FIGURE_MUSICIAN) {
        *second = BUILDING_BANDSTAND;
        *third = BUILDING_PAVILION;
        return 1;
    }
    if (figure_type == FIGURE_DANCER) {
        *third = BUILDING_PAVILION;
        return 1;
    }
    if (figure_type == FIGURE_SENET_PLAYER) {
        *first = BUILDING_SENET_HOUSE;
        return 1;
    }
    if (figure_type == FIGURE_ZOOKEEPER) {
        *first = BUILDING_ZOO;
        return 1;
    }
    return 0;
}

static int32_t venue_matches(int32_t building_type, int32_t first, int32_t second, int32_t third)
{
    return building_type != BUILDING_NONE && (building_type == first || building_type == second || building_type == third);
}

static int32_t venue_grid(int32_t building_id)
{
    int32_t x = city_buildings.records[building_id].road_access_x;
    int32_t y = city_buildings.records[building_id].road_access_y;
    return map_info.grid_start + x + y * GRID_SIZE;
}

int32_t figure_entertainer_determine_destination(int32_t figure_id)
{
    building_list_small_size = 0;
    figure *f = figure_get(figure_id);
    int32_t figure_type = f->type;
    int32_t first, second, third;
    if ((uint32_t) (figure_type - FIGURE_JUGGLER) > (FIGURE_ZOOKEEPER - FIGURE_JUGGLER) || !entertainer_venue(figure_type, &first, &second, &third)) {
        return 0;
    }
    route_candidates.count = 0;
    if (building_highest_id >= 1) {
        for (int32_t id = 1; id <= building_highest_id; id++) {
            if (city_buildings.records[id].state != BUILDING_STATE_IN_USE || city_buildings.records[id].num_workers <= 0) {
                continue;
            }
            int32_t building_type = city_buildings.records[id].type;
            if (!venue_matches(building_type, first, second, third) || building_main(id) != id) {
                continue;
            }
            if ((figure_type == FIGURE_JUGGLER || figure_type == FIGURE_SENET_PLAYER || figure_type == FIGURE_ZOOKEEPER) &&
                city_buildings.records[id].data.entertainment.show_days_left[0] != 0) {
                continue;
            }
            if (figure_type == FIGURE_MUSICIAN && city_buildings.records[id].data.entertainment.show_days_left[1] != 0) {
                continue;
            }
            if (figure_type == FIGURE_DANCER && city_buildings.records[id].data.entertainment.show_days_left[2] != 0) {
                continue;
            }
            int32_t count = route_candidates.count;
            route_candidates.buildings[count] = id;
            route_candidates.tiles[count] = venue_grid(id);
            route_candidates.count = count + 1;
        }
    }
    if (route_candidates.count > 0) {
        int32_t chosen = figure_route_add_to_closest_of(figure_id, route_candidates.tiles, route_candidates.count);
        if (chosen <= 0) {
            return 0;
        }
        return route_candidates.buildings[chosen - 1];
    }
    int32_t best = ENTERTAINER_NO_VENUE_DISTANCE;
    int32_t best_id = 0;
    map_routing_calculate_distances(f->x, f->y);
    if (building_highest_id >= 1) {
        for (int32_t id = 1; id <= building_highest_id; id++) {
            if (city_buildings.records[id].state != BUILDING_STATE_IN_USE || city_buildings.records[id].num_workers <= 0) {
                continue;
            }
            int32_t building_type = city_buildings.records[id].type;
            if (!venue_matches(building_type, first, second, third) || building_main(id) != id) {
                continue;
            }
            int32_t distance = routing_grids.distance[venue_grid(id)];
            if (figure_type == FIGURE_JUGGLER) {
                uint8_t shown = city_buildings.records[id].data.entertainment.show_days_left[0];
                if (shown != 0) {
                    distance += shown;
                }
            }
            if (figure_type == FIGURE_MUSICIAN) {
                uint8_t shown = city_buildings.records[id].data.entertainment.show_days_left[1];
                if (shown != 0) {
                    distance += shown;
                }
            }
            if (figure_type == FIGURE_DANCER) {
                uint8_t shown = city_buildings.records[id].data.entertainment.show_days_left[2];
                if (shown != 0) {
                    distance += shown;
                }
            }
            if (distance < best) {
                best = distance;
                best_id = id;
            }
        }
    }
    if (best >= ENTERTAINER_NO_VENUE_DISTANCE) {
        return 0;
    }
    f->destination_building_id = (int16_t) best_id;
    f->destination_x = city_buildings.records[best_id].road_access_x;
    f->destination_y = city_buildings.records[best_id].road_access_y;
    return best_id;
}

static void check_building_figure(figure *f, int32_t figure_id)
{
    const building *b = building_get(f->building_id);
    if (b->state != BUILDING_STATE_IN_USE || b->figure_id != figure_id) {
        f->state = FIGURE_STATE_DEAD;
    }
}

void figure_worker_action(void)
{
    int32_t figure_id = figure_current_id;
    figure *f = figure_get(figure_id);
    f->terrain_usage = TERRAIN_USAGE_ROADS;
    f->use_cross_country = 0;
    f->max_roam_length = (int16_t) walker_rules[WALKER_RULE_ROAMER_ROAM_TICKS];
    check_building_figure(f, figure_id);
}

void figure_missionary_action(void)
{
    int32_t figure_id = figure_current_id;
    figure *f = figure_get(figure_id);
    f->terrain_usage = TERRAIN_USAGE_ROADS;
    f->use_cross_country = 0;
    f->max_roam_length = (int16_t) walker_rules[WALKER_RULE_MISSIONARY_ROAM_TICKS];
    check_building_figure(f, figure_id);
    if (++f->animation_frame >= WALKER_ANIMATION_FRAMES) {
        f->animation_frame = 0;
    }
    figure_roamer_action();
    figure_image_direction = f->direction - map_info.orientation;
    if (figure_image_direction < 0) {
        figure_image_direction += DIRECTION_COUNT;
    }
    if (f->action_state == FIGURE_ACTION_CORPSE) {
        f->image_id = figure_corpse_offsets[f->wait_ticks / 2] + image_group(IMAGE_GROUP_FIGURE_MISSIONARY_DEATH);
        return;
    }
    f->image_id = image_group(IMAGE_GROUP_FIGURE_MISSIONARY) + f->animation_frame * 8 + figure_image_direction;
}

void figure_showman_action(void)
{
    figure_get(figure_current_id)->state = FIGURE_STATE_DEAD;
}

void figure_entertainer_action(void)
{
    int32_t figure_id = figure_current_id;
    figure *f = figure_get(figure_id);
    f->cart_image_id = image_group(IMAGE_GROUP_FIGURE_PLAGUE_SHADOW);
    f->terrain_usage = TERRAIN_USAGE_ROADS;
    int32_t speed = figure_properties[f->type].speed;
    f->use_cross_country = 0;
    f->max_roam_length = (int16_t) walker_rules[WALKER_RULE_ENTERTAINER_ROAM_TICKS];
    advance_animation(f);
    int32_t home_id = f->building_id;
    const building *home = building_get(home_id);
    switch (f->action_state) {
        case FIGURE_ACTION_ATTACK:
            figure_combat_handle_attack(figure_id);
            if (++f->animation_frame >= ENTERTAINER_ATTACK_FRAMES) {
                f->animation_frame = 0;
            }
            break;
        case FIGURE_ACTION_CORPSE:
            figure_combat_handle_corpse(figure_id);
            break;
        case FIGURE_ACTION_ENTERTAINER_CREATED:
            f->is_ghost = 1;
            f->animation_frame = 0;
            f->wait_ticks_missile = 0;
            if (--f->wait_ticks > 0) {
                break;
            }
            if (!map_closest_road_within_radius(home->road_access_x, home->road_access_y, home->size, walker_rules[WALKER_RULE_HOME_ROAD_RADIUS])) {
                f->state = FIGURE_STATE_DEAD;
                break;
            }
            f->action_state = FIGURE_ACTION_ENTERTAINER_EXIT;
            set_destination_to_found_road(f);
            set_cross_country_to_destination(figure_current_id);
            f->roam_length = 0;
            break;
        case FIGURE_ACTION_ENTERTAINER_EXIT:
            f->use_cross_country = 1;
            f->is_ghost = 1;
            if (figure_movement_move_ticks_cross_country(figure_id, 1) == 1) {
                int32_t venue_id = figure_entertainer_determine_destination(figure_current_id);
                const building *venue = building_get(venue_id);
                if (!venue_id || !home_road(venue)) {
                    f->state = FIGURE_STATE_DEAD;
                } else {
                    f->destination_building_id = (int16_t) venue_id;
                    f->action_state = FIGURE_ACTION_ENTERTAINER_TO_VENUE;
                    set_destination_to_found_road(f);
                    f->roam_length = 0;
                }
            }
            f->is_ghost = 1;
            break;
        case FIGURE_ACTION_ENTERTAINER_TO_VENUE:
            f->is_ghost = 0;
            if (++f->roam_length >= walker_rules[WALKER_RULE_ENTERTAINER_VENUE_WALK_TICKS]) {
                f->state = FIGURE_STATE_DEAD;
            }
            figure_movement_move_ticks(figure_id, speed);
            if (f->direction == DIRECTION_AT_DESTINATION) {
                figure_entertainer_update_shows(figure_current_id);
                f->state = FIGURE_STATE_DEAD;
            } else if (f->direction == DIRECTION_REROUTE) {
                figure_route_remove(figure_current_id);
            } else if (f->direction == DIRECTION_LOST) {
                f->state = FIGURE_STATE_DEAD;
            }
            break;
        case FIGURE_ACTION_ENTERTAINER_ROAMING:
            f->is_ghost = 0;
            if (++f->roam_length >= f->max_roam_length) {
                if (!home_road(home)) {
                    f->state = FIGURE_STATE_DEAD;
                } else {
                    f->action_state = FIGURE_ACTION_ENTERTAINER_RETURNING;
                    set_destination_to_found_road(f);
                }
            }
            figure_movement_roam_ticks(figure_current_id, 1);
            break;
        case FIGURE_ACTION_ENTERTAINER_RETURNING:
            figure_movement_move_ticks(figure_id, speed);
            if (f->direction == DIRECTION_REROUTE || f->direction == DIRECTION_LOST || f->direction == DIRECTION_AT_DESTINATION) {
                f->state = FIGURE_STATE_DEAD;
            }
            break;
        case FIGURE_ACTION_ON_FERRY:
            f->is_ghost = 1;
            if (++f->wait_ticks > walker_rules[WALKER_RULE_FERRY_WAIT_TICKS]) {
                f->wait_ticks = 0;
                if (!figure_is_queued_for_ferry(figure_id)) {
                    f->action_state = FIGURE_ACTION_ENTERTAINER_CREATED;
                }
            }
            break;
        case FIGURE_ACTION_WAIT_FOR_FERRY:
            f->animation_frame = 0;
            break;
        default:
            f->action_state = FIGURE_ACTION_ENTERTAINER_CREATED;
            break;
    }
    int32_t dir = image_direction((f->direction < DIRECTION_COUNT) ? f->direction : f->previous_tile_direction);
    int32_t walk;
    int32_t corpse;
    switch (f->type) {
        case FIGURE_JUGGLER:
            walk = IMAGE_GROUP_FIGURE_JUGGLER;
            corpse = IMAGE_GROUP_FIGURE_JUGGLER_DEATH;
            break;
        case FIGURE_MUSICIAN:
            walk = IMAGE_GROUP_FIGURE_MUSICIAN;
            corpse = IMAGE_GROUP_FIGURE_MUSICIAN_DEATH;
            break;
        case FIGURE_DANCER:
            walk = IMAGE_GROUP_FIGURE_DANCER;
            corpse = IMAGE_GROUP_FIGURE_DANCER_DEATH;
            break;
        case FIGURE_ZOOKEEPER:
            walk = IMAGE_GROUP_FIGURE_ZOOKEEPER;
            corpse = IMAGE_GROUP_FIGURE_ZOOKEEPER_DEATH;
            break;
        default:
            walk = IMAGE_GROUP_FIGURE_SENET_PLAYER;
            corpse = IMAGE_GROUP_FIGURE_SENET_PLAYER_DEATH;
            break;
    }
    if (f->action_state == FIGURE_ACTION_CORPSE) {
        f->image_id = image_group(corpse) + ((f->wait_ticks >= ENTERTAINER_DEATH_FRAMES) ? (ENTERTAINER_DEATH_FRAMES - 1) : f->wait_ticks);
    } else {
        f->image_id = image_group(walk) + f->animation_frame * 8 + dir;
    }
}

static int32_t max_distance_to(int32_t x, int32_t y, const figure *f)
{
    return calc_maximum_distance(f->x, f->y, x, y);
}

int32_t figure_closest_enemy_for_constable(int32_t x, int32_t y, int32_t max_distance)
{
    enemy_candidates_count = 0;
    figure_search_best_distance = WALKER_NO_TARGET_DISTANCE;
    for (int32_t id = 1; id < MAX_FIGURES; id++) {
        const figure *f = figure_get(id);
        if (f->state != FIGURE_STATE_ALIVE || f->targeted_by_figure_id != 0) {
            continue;
        }
        if (enemy_candidates_count >= 100) {
            break;
        }
        int32_t type = f->type;
        int32_t distance;
        if (type == FIGURE_NATIVE && f->action_state == FIGURE_ACTION_NATIVE_ATTACKING) {
            distance = max_distance_to(x, y, f);
        } else if ((type >= FIGURE_ENEMY_ARCHER && type <= FIGURE_KINGDOM_SOLDIER) || type == FIGURE_BEDOUIN) {
            distance = max_distance_to(x, y, f) * 2;
        } else if (type == FIGURE_CROCODILE || type == FIGURE_HYENA || type == FIGURE_HIPPO || type == FIGURE_LION || type == FIGURE_SCORPION ||
            type == FIGURE_ASP || type == FIGURE_MUMMY) {
            distance = max_distance_to(x, y, f) * 3;
        } else {
            continue;
        }
        if (distance < max_distance) {
            enemy_candidates_ids[enemy_candidates_count] = id;
            enemy_candidates_offsets[enemy_candidates_count] = f->grid_offset;
            enemy_candidates_count++;
        }
    }
    if (enemy_candidates_count != 0) {
        int32_t index = map_routing_citizen_find_first_of_any(x, y, enemy_candidates_offsets, enemy_candidates_count);
        if (index > 0) {
            return enemy_candidates_ids[index - 1];
        }
    }
    return 0;
}

int32_t building_maintenance_get_closest_burning_ruin(int32_t x, int32_t y)
{
    burning_ruin_distance = WALKER_NO_TARGET_DISTANCE;
    int32_t fought_distance = WALKER_NO_TARGET_DISTANCE;
    int32_t best = 0;
    int32_t fought = 0;
    building_burning_size = 0;
    int32_t count = building_list_burning_index;
    if (count <= 0) {
        return 0;
    }
    building_burning_size = count;
    for (int32_t i = 0; i < count; i++) {
        int32_t id = building_list_burning[i];
        const building *b = building_get(id);
        if (b->state != BUILDING_STATE_IN_USE || b->type != BUILDING_BURNING_RUIN || b->ruin_has_plague != 0 || b->distance_from_entry == 0) {
            continue;
        }
        int32_t distance = calc_maximum_distance(b->x, b->y, x, y);
        if (b->figure_id4 == 0) {
            if (distance < burning_ruin_distance) {
                burning_ruin_distance = distance;
                best = id;
            }
        } else if (distance < fought_distance) {
            fought_distance = distance;
            fought = id;
        }
    }
    if (best == 0 && fought_distance < FOUGHT_RUIN_DISTANCE) {
        return fought;
    }
    return best;
}

int32_t fireman_fight_fire(int32_t figure_id)
{
    if (building_burning_total <= 0) {
        return 0;
    }
    figure *f = figure_get(figure_id);
    switch (f->action_state) {
        case FIGURE_ACTION_ATTACK:
        case FIGURE_ACTION_CORPSE:
        case FIGURE_ACTION_SERVICE_CREATED:
        case FIGURE_ACTION_SERVICE_ENTER_EXIT:
        case FIGURE_ACTION_FIREMAN_TO_FIRE:
        case FIGURE_ACTION_FIREMAN_AT_FIRE:
        case FIGURE_ACTION_CONSTABLE_CHASE:
        case FIGURE_ACTION_SERVICE_UNUSED:
            return 0;
    }
    if (++f->wait_ticks_missile < walker_rules[WALKER_RULE_FIRE_SEARCH_TICKS]) {
        return 0;
    }
    int32_t ruin_id = building_maintenance_get_closest_burning_ruin(f->x, f->y);
    if (ruin_id <= 0 || burning_ruin_distance > walker_rules[WALKER_RULE_FIRE_SEARCH_DISTANCE]) {
        return 0;
    }
    building *ruin = building_get(ruin_id);
    f->wait_ticks_missile = 0;
    f->action_state = FIGURE_ACTION_FIREMAN_TO_FIRE;
    f->destination_x = ruin->road_access_x;
    f->destination_y = ruin->road_access_y;
    f->destination_building_id = (int16_t) ruin_id;
    figure_route_remove(figure_id);
    ruin->figure_id4 = (int16_t) figure_id;
    return 1;
}

int32_t constable_fight_enemy(int32_t figure_id)
{
    if (city_player_current()->security_breach_duration <= 0 && enemy_army_totals.enemy_formations <= 0) {
        return 0;
    }
    figure *f = figure_get(figure_id);
    switch (f->action_state) {
        case FIGURE_ACTION_ATTACK:
        case FIGURE_ACTION_CORPSE:
        case FIGURE_ACTION_SERVICE_CREATED:
        case FIGURE_ACTION_SERVICE_ENTER_EXIT:
        case FIGURE_ACTION_FIREMAN_TO_FIRE:
        case FIGURE_ACTION_FIREMAN_AT_FIRE:
        case FIGURE_ACTION_CONSTABLE_CHASE:
        case FIGURE_ACTION_SERVICE_UNUSED:
            return 0;
    }
    if (++f->wait_ticks_next_target < walker_rules[WALKER_RULE_ENEMY_SEARCH_TICKS]) {
        return 0;
    }
    f->wait_ticks_next_target = 0;
    int32_t enemy_id = figure_closest_enemy_for_constable(f->x, f->y, walker_rules[WALKER_RULE_ENEMY_SEARCH_DISTANCE]);
    if (enemy_id <= 0) {
        return 0;
    }
    figure *enemy = figure_get(enemy_id);
    f->wait_ticks_next_target = 0;
    f->action_state = FIGURE_ACTION_CONSTABLE_CHASE;
    f->destination_x = enemy->x;
    f->destination_y = enemy->y;
    f->target_figure_id = (int16_t) enemy_id;
    enemy->targeted_by_figure_id = (int16_t) figure_id;
    f->target_figure_created_sequence = enemy->created_sequence;
    figure_route_remove(figure_id);
    return 1;
}

static void fireman_at_fire(figure *f)
{
    building *ruin = building_get(f->destination_building_id);
    int32_t distance = calc_maximum_distance(f->x, f->y, ruin->x, ruin->y);
    if (ruin->state != BUILDING_STATE_IN_USE || ruin->type != BUILDING_BURNING_RUIN || distance >= 2) {
        f->wait_ticks = 1;
    } else {
        ruin->fire_duration = (int16_t) walker_rules[WALKER_RULE_FIRE_OUT_DURATION];
    }
    int32_t direction = calc_general_direction(f->x, f->y, ruin->x, ruin->y);
    f->attack_direction = (int8_t) direction;
    if (f->attack_direction >= DIRECTION_COUNT) {
        f->attack_direction = 0;
    }
    if (--f->wait_ticks > 0 || f->extinguish_ticks < FIREMAN_FIGHT_FIRE_FRAMES) {
        return;
    }
    f->wait_ticks_missile = (uint8_t) walker_rules[WALKER_RULE_FIRE_SEARCH_TICKS];
    if (fireman_fight_fire(figure_current_id)) {
        return;
    }
    if (!home_road(building_get(f->building_id))) {
        f->state = FIGURE_STATE_DEAD;
        return;
    }
    f->action_state = FIGURE_ACTION_SERVICE_RETURNING;
    set_destination_to_found_road(f);
    figure_route_remove(figure_current_id);
}

void figure_fireman_action(void)
{
    static const building_walker fireman = { WALKER_RULE_FIREMAN_ROAM_TICKS, 1, 1 };
    int32_t speed;
    int32_t wrapped;
    building_walker_begin(&fireman, &speed, &wrapped);
    fireman_fight_fire(figure_current_id);
    int32_t figure_id = figure_current_id;
    figure *f = figure_get(figure_id);
    switch (f->action_state) {
        case FIGURE_ACTION_ATTACK:
            figure_combat_handle_attack(figure_id);
            break;
        case FIGURE_ACTION_CORPSE:
            figure_combat_handle_corpse(figure_id);
            break;
        case FIGURE_ACTION_FIREMAN_TO_FIRE:
            f->terrain_usage = TERRAIN_USAGE_ANY;
            figure_movement_move_ticks(figure_id, speed);
            if (f->direction == DIRECTION_REROUTE || f->direction == DIRECTION_LOST) {
                f->state = FIGURE_STATE_DEAD;
            } else if (f->direction == DIRECTION_AT_DESTINATION) {
                f->action_state = FIGURE_ACTION_FIREMAN_AT_FIRE;
                figure_route_remove(figure_current_id);
                f->roam_length = 0;
                f->wait_ticks = (int16_t) walker_rules[WALKER_RULE_FIRE_FIGHT_TICKS];
                f->extinguish_ticks = 0;
            }
            break;
        case FIGURE_ACTION_FIREMAN_AT_FIRE:
            fireman_at_fire(f);
            break;
        default:
            if (!building_walker_state(f, speed, &fireman)) {
                f->action_state = FIGURE_ACTION_SERVICE_CREATED;
            }
            break;
    }
    int32_t action = f->action_state;
    int32_t dir;
    if (action == FIGURE_ACTION_FIREMAN_AT_FIRE || action == FIGURE_ACTION_ATTACK) {
        dir = f->attack_direction;
    } else {
        dir = (f->direction < DIRECTION_COUNT) ? f->direction : f->previous_tile_direction;
    }
    dir = image_direction(dir);
    int32_t walk = image_group(IMAGE_GROUP_FIGURE_FIREMAN);
    if (action == FIGURE_ACTION_FIREMAN_AT_FIRE) {
        if (++f->extinguish_ticks >= FIREMAN_FIGHT_FIRE_FRAMES) {
            f->image_id = image_group(IMAGE_GROUP_FIGURE_FIREMAN_FIGHT_FIRE) + dir;
        } else {
            f->image_id = image_group(IMAGE_GROUP_FIGURE_FIREMAN_FIGHT_FIRE) + f->extinguish_ticks * 8 + dir;
        }
    } else if (action == FIGURE_ACTION_ATTACK) {
        f->image_id = walk + dir;
    } else if (action == FIGURE_ACTION_CORPSE) {
        f->image_id = image_group(IMAGE_GROUP_FIGURE_FIREMAN_DEATH) + corpse_offset(f);
    } else {
        f->image_id = walk + f->animation_frame * 8 + dir;
    }
}

static void law_walker_chase(figure *f, int32_t speed)
{
    f->terrain_usage = TERRAIN_USAGE_ANY;
    if (!figure_target_is_alive(figure_current_id)) {
        if (!home_road(building_get(f->building_id))) {
            f->state = FIGURE_STATE_DEAD;
        } else {
            f->action_state = FIGURE_ACTION_SERVICE_RETURNING;
            set_destination_to_found_road(f);
            figure_route_remove(figure_current_id);
            f->roam_length = 0;
        }
    }
    figure_movement_move_ticks(figure_current_id, speed);
    if (f->direction == DIRECTION_REROUTE || f->direction == DIRECTION_LOST) {
        f->state = FIGURE_STATE_DEAD;
    } else if (f->direction == DIRECTION_AT_DESTINATION) {
        const figure *target = figure_get(f->target_figure_id);
        f->destination_x = target->x;
        f->destination_y = target->y;
        figure_route_remove(figure_current_id);
    }
}

static void law_walker_action(int32_t is_constable, int32_t walk_group, int32_t corpse_group, int32_t attack_group)
{
    static const building_walker law_walker = { WALKER_RULE_LAW_WALKER_ROAM_TICKS, 0, 1 };
    int32_t speed;
    int32_t wrapped;
    building_walker_begin(&law_walker, &speed, &wrapped);
    if (wrapped && is_constable) {
        constable_fight_enemy(figure_current_id);
    }
    int32_t figure_id = figure_current_id;
    figure *f = figure_get(figure_id);
    switch (f->action_state) {
        case FIGURE_ACTION_ATTACK:
            if (is_constable) {
                sound_figure_play(figure_id, CONSTABLE_ATTACK_SOUND, WALKER_SOUND_VARIANT);
            }
            figure_combat_handle_attack(figure_current_id);
            break;
        case FIGURE_ACTION_CORPSE:
            figure_combat_handle_corpse(figure_id);
            break;
        case FIGURE_ACTION_CONSTABLE_CHASE:
            law_walker_chase(f, speed);
            break;
        default:
            if (!building_walker_state(f, speed, &law_walker)) {
                f->action_state = FIGURE_ACTION_SERVICE_CREATED;
            }
            break;
    }
    int32_t action = f->action_state;
    int32_t dir;
    if (action == FIGURE_ACTION_ATTACK) {
        dir = f->attack_direction;
    } else {
        dir = (f->direction < DIRECTION_COUNT) ? f->direction : f->previous_tile_direction;
    }
    dir = image_direction(dir);
    if (action == FIGURE_ACTION_ATTACK) {
        if (is_constable) {
            sound_figure_play(figure_current_id, CONSTABLE_ATTACK_SOUND, WALKER_SOUND_VARIANT);
        }
        int32_t offset = f->attack_image_offset;
        if (offset < 12) {
            f->image_id = image_group(attack_group) + figure_image_direction;
        } else {
            f->image_id = image_group(attack_group) + (offset - 12) / 2 * 8 + figure_image_direction;
        }
    } else if (action == FIGURE_ACTION_CORPSE) {
        f->image_id = image_group(corpse_group) + corpse_offset(f);
    } else {
        f->image_id = image_group(walk_group) + f->animation_frame * 8 + dir;
    }
}

void figure_constable_action(void)
{
    law_walker_action(1, IMAGE_GROUP_FIGURE_CONSTABLE, IMAGE_GROUP_FIGURE_CONSTABLE_DEATH, IMAGE_GROUP_FIGURE_CONSTABLE_ATTACK);
}

void figure_magistrate_action(void)
{
    law_walker_action(0, IMAGE_GROUP_FIGURE_MAGISTRATE, IMAGE_GROUP_FIGURE_MAGISTRATE_DEATH, IMAGE_GROUP_FIGURE_MAGISTRATE);
}

void figure_festival_priest_action(void)
{
    int32_t figure_id = figure_current_id;
    figure *f = figure_get(figure_id);
    f->use_cross_country = 0;
    f->max_roam_length = (int16_t) walker_rules[WALKER_RULE_ROAMER_ROAM_TICKS];
    int32_t home_id = f->building_id;
    const city_player *player = city_player_current();
    int32_t speed = figure_properties[f->type].speed;
    advance_animation(f);
    switch (f->action_state) {
        case FIGURE_ACTION_FESTIVAL_PRIEST_CREATED:
            f->terrain_usage = TERRAIN_USAGE_ROADS;
            f->festival_trips = walker_rules[WALKER_RULE_FESTIVAL_TRIPS];
            f->destination_x = (int16_t) (player->festival_square_x + 2);
            f->destination_y = (int16_t) (player->festival_square_y + 2);
            f->action_state = FIGURE_ACTION_FESTIVAL_PRIEST_WALKING;
            break;
        case FIGURE_ACTION_FESTIVAL_PRIEST_PICK_SPOT: {
            f->terrain_usage = TERRAIN_USAGE_ANY;
            if (player->festival_square_x == -1 || player->festival_square_y == -1) {
                f->state = FIGURE_STATE_DEAD;
                f->image_id = 0;
                return;
            }
            int32_t x;
            int32_t y;
            do {
                x = ((crt_rand() * FESTIVAL_SQUARE_SIZE) / CRT_RAND_RANGE) + player->festival_square_x;
                y = ((crt_rand() * FESTIVAL_SQUARE_SIZE) / CRT_RAND_RANGE) + player->festival_square_y;
            } while (x == f->x && y == f->y);
            f->destination_x = (int16_t) x;
            f->destination_y = (int16_t) y;
            f->action_state = FIGURE_ACTION_FESTIVAL_PRIEST_WALKING;
            break;
        }
        case FIGURE_ACTION_FESTIVAL_PRIEST_WALKING:
            figure_movement_move_ticks(figure_id, speed);
            if (f->direction == DIRECTION_AT_DESTINATION) {
                f->direction = f->previous_tile_direction;
                f->action_state = FIGURE_ACTION_FESTIVAL_PRIEST_ARRIVED;
            } else if (f->direction == DIRECTION_LOST) {
                f->state = FIGURE_STATE_DEAD;
            } else if (f->direction == DIRECTION_REROUTE) {
                figure_route_remove(figure_current_id);
                f->direction = f->previous_tile_direction;
                f->action_state = FIGURE_ACTION_FESTIVAL_PRIEST_ARRIVED;
                f->festival_trips = 0;
            }
            break;
        case FIGURE_ACTION_FESTIVAL_PRIEST_ARRIVED:
            if (--f->festival_trips > 0) {
                f->action_state = FIGURE_ACTION_FESTIVAL_PRIEST_PICK_SPOT;
            } else {
                f->state = FIGURE_STATE_DEAD;
            }
            break;
        default:
            f->action_state = FIGURE_ACTION_FESTIVAL_PRIEST_PICK_SPOT;
            f->terrain_usage = TERRAIN_USAGE_ROADS;
            break;
    }
    int32_t group;
    int32_t look;
    switch (building_get(home_id)->type) {
        case BUILDING_BANDSTAND:
        case BUILDING_CONSERVATORY:
            group = IMAGE_GROUP_FIGURE_MUSICIAN;
            look = FIGURE_MUSICIAN;
            break;
        case BUILDING_BOOTH:
        case BUILDING_SENET_HOUSE:
        case BUILDING_JUGGLER_SCHOOL:
            group = IMAGE_GROUP_FIGURE_JUGGLER;
            look = FIGURE_JUGGLER;
            break;
        case BUILDING_PAVILION:
        case BUILDING_DANCE_SCHOOL:
            group = IMAGE_GROUP_FIGURE_DANCER;
            look = FIGURE_DANCER;
            break;
        case BUILDING_SCRIBAL_SCHOOL:
        case BUILDING_LIBRARY:
            group = IMAGE_GROUP_FIGURE_TEACHER;
            look = FIGURE_TEACHER;
            break;
        case BUILDING_ZOO:
            group = IMAGE_GROUP_FIGURE_ZOOKEEPER;
            look = FIGURE_ZOOKEEPER;
            break;
        case BUILDING_TEMPLE_OSIRIS:
        case BUILDING_TEMPLE_RA:
        case BUILDING_TEMPLE_PTAH:
        case BUILDING_TEMPLE_SETH:
        case BUILDING_TEMPLE_BAST:
        case BUILDING_TEMPLE_COMPLEX_OSIRIS:
        case BUILDING_TEMPLE_COMPLEX_RA:
        case BUILDING_TEMPLE_COMPLEX_PTAH:
        case BUILDING_TEMPLE_COMPLEX_SETH:
        case BUILDING_TEMPLE_COMPLEX_BAST:
        case BUILDING_TEMPLE_GOD_5:
        case BUILDING_TEMPLE_COMPLEX_GOD_5:
            look = FIGURE_PRIEST;
            switch (building_temple_god(home_id)) {
                case GOD_OSIRIS:
                    group = IMAGE_GROUP_FIGURE_PRIEST_OSIRIS;
                    break;
                case GOD_RA:
                    group = IMAGE_GROUP_FIGURE_PRIEST_RA;
                    break;
                case GOD_SETH:
                    group = IMAGE_GROUP_FIGURE_PRIEST_SETH;
                    break;
                case GOD_BAST:
                    group = IMAGE_GROUP_FIGURE_PRIEST_BAST;
                    break;
                default:
                    group = IMAGE_GROUP_FIGURE_PRIEST_PTAH;
                    break;
            }
            break;
        default:
            group = IMAGE_GROUP_FIGURE_PATRICIAN;
            look = FIGURE_PATRICIAN;
            break;
    }
    f->resource_id = (uint8_t) look;
    int32_t dir = image_direction(f->direction);
    f->image_id = image_group(group) + f->animation_frame * 8 + dir;
}
