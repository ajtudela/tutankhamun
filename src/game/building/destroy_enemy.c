// Buildings destroyed by enemies: an invasion landing, a transport landing or enemies hitting a building over its
// damage limit destroy the building or wall at a tile, count it for the city, steal a share of savings or palace
// money by difficulty, and reroute the tower sentries; with the sentry helpers for destroyed walls. Monuments
// are left alone.
//
// A sentry sent off a lost wall gets its route, action and destination set on figure_current_id instead of itself.

#include "building/destroy_enemy.h"
#include "building/building.h"
#include "building/building_figure_helpers.h"
#include "building/building_maintenance.h"
#include "building/building_state.h"
#include "building/building_type.h"
#include "building/construction_venue.h"
#include "building/monument.h"
#include "city/city_player.h"
#include "city/difficulty.h"
#include "city/finance.h"
#include "city/message.h"
#include "core/calc.h"
#include "figure/figure.h"
#include "figure/figure_action.h"
#include "figure/figure_type.h"
#include "figure/military.h"
#include "map/building_tiles.h"
#include "map/grid.h"
#include "map/map_data.h"
#include "map/routing_enemy.h"
#include "map/routing_terrain.h"
#include "map/terrain.h"
#include "map/tiles.h"
#include "window/window_data.h"
#include <stdint.h>

enum destroy_enemy_limit {
    DESTROYED_BUILDINGS_MAX = 12,
    SENTRY_KILL_DISTANCE = 2,
    SENTRY_WALL_RADIUS = 2,
    CROSS_COUNTRY_STEPS_PER_TILE = 15,
};

int32_t enemy_theft_percentages[DIFFICULTY_COUNT] = {
    [DIFFICULTY_VERY_EASY] = 0,
    [DIFFICULTY_EASY] = 15,
    [DIFFICULTY_NORMAL] = 30,
    [DIFFICULTY_HARD] = 60,
    [DIFFICULTY_VERY_HARD] = 100,
};

int32_t building_main_part_id(int32_t building_id)
{
    return building_main(building_id);
}

void figure_kill_tower_sentries_at(int32_t x, int32_t y)
{
    for (int32_t id = 0; id < MAX_FIGURES; id++) {
        figure *f = figure_get(id);
        if (f->state != FIGURE_STATE_ALIVE || f->type != FIGURE_TOWER_SENTRY || f->action_state == FIGURE_ACTION_CORPSE) {
            continue;
        }
        int32_t distance = calc_maximum_distance(x, y, f->x, f->y);
        if (distance < SENTRY_KILL_DISTANCE) {
            f->state = FIGURE_STATE_DEAD;
        }
    }
}

static void send_sentry_to_tower(figure *f, int32_t id)
{
    f->action_state = FIGURE_ACTION_TOWER_SENTRY_AT_REST;
    map_figure_delete(id);
    const building *tower = building_get(f->building_id);
    f->x = tower->x;
    f->source_x = tower->x;
    f->y = tower->y;
    f->source_y = tower->y;
    f->grid_offset = map_grid_offset(f->x, f->y);
    map_figure_add(id);
    f->action_state = FIGURE_ACTION_TOWER_SENTRY_AT_REST;
    figure_route_remove(id);
}

static void send_sentry_onto_wall(figure *f, int32_t id)
{
    figure_route_remove(figure_current_id);
    f->progress_on_tile = 0;
    map_figure_delete(id);
    int32_t x = map_result_x;
    int32_t y = map_result_y;
    f->x = (int16_t) x;
    f->previous_tile_x = (int16_t) x;
    f->y = (int16_t) y;
    f->previous_tile_y = (int16_t) y;
    f->cross_country.x = (int16_t) (x * CROSS_COUNTRY_STEPS_PER_TILE);
    f->cross_country.y = (int16_t) (y * CROSS_COUNTRY_STEPS_PER_TILE);
    f->grid_offset = x + (y * GRID_SIZE) + map_info.grid_start;
    map_figure_add(id);
    figure *current = figure_get(figure_current_id);
    current->action_state = FIGURE_ACTION_TOWER_SENTRY_RETURNING;
    current->destination_x = current->source_x;
    current->destination_y = current->source_y;
}

void figure_tower_sentry_reroute(void)
{
    for (int32_t id = 1; id < MAX_FIGURES; id++) {
        figure *f = figure_get(id);
        if (f->type != FIGURE_TOWER_SENTRY || routing_grids.walls[f->grid_offset] == 0) {
            continue;
        }
        if (map_routing_wall_within_radius(f->x, f->y, 1, SENTRY_WALL_RADIUS) == 0) {
            send_sentry_to_tower(f, id);
        } else {
            send_sentry_onto_wall(f, id);
        }
    }
}

int32_t building_destroy_counts_for_peace(int32_t type)
{
    switch (type) {
        case BUILDING_HOUSE_CRUDE_HUT:
        case BUILDING_HOUSE_STURDY_HUT:
        case BUILDING_POLICE_STATION:
        case BUILDING_ARCHITECT_POST:
        case BUILDING_WELL:
        case BUILDING_FORT:
        case BUILDING_FORT_GROUND:
        case BUILDING_MUD_GATEHOUSE:
        case BUILDING_BRICK_GATEHOUSE:
        case BUILDING_GATEHOUSE:
        case BUILDING_MUD_GATEHOUSE_UP:
        case BUILDING_BRICK_GATEHOUSE_UP:
        case BUILDING_GATEHOUSE_UP:
        case BUILDING_MUD_TOWER:
        case BUILDING_BRICK_TOWER:
        case BUILDING_TOWER:
            return 0;
        default:
            return 1;
    }
}

static int32_t is_venue(int32_t type)
{
    return type == BUILDING_BOOTH || type == BUILDING_BANDSTAND || type == BUILDING_PAVILION || type == BUILDING_FESTIVAL_SQUARE;
}

static void show_palace_robbed(int32_t type)
{
    switch (type) {
        case BUILDING_VILLAGE_PALACE:
        case BUILDING_TOWN_PALACE:
        case BUILDING_TAX_COLLECTOR:
        case BUILDING_TAX_COLLECTOR_UP:
            city_warning_show(WARNING_TAX_COLLECTOR_DESTROYED);
            break;
        case BUILDING_COURTHOUSE:
            city_warning_show(WARNING_COURTHOUSE_DESTROYED);
            break;
        case BUILDING_VILLAGE_PALACE_UP:
        case BUILDING_TOWN_PALACE_UP:
        case BUILDING_CITY_PALACE:
            city_warning_show(WARNING_PALACE_DESTROYED);
            break;
    }
}

static void count_destroyed(building *b, int32_t id)
{
    city_player *player = city_player_current();
    int32_t percentage = enemy_theft_percentages[settings_data.difficulty];
    if (building_is_mansion(id)) {
        player->destroyed_buildings++;
        int32_t amount = calc_adjust_with_percentage(player->personal_savings, percentage);
        if (amount != 0) {
            player->personal_savings -= amount;
            player->savings_lost += amount;
            city_warning_show(WARNING_PERSONAL_SAVINGS_STOLEN);
        }
    } else if (building_is_palace(id)) {
        player->destroyed_buildings++;
        int32_t amount = calc_adjust_with_percentage(b->tax_income_or_storage, percentage);
        if (amount != 0) {
            building_update_money_values();
            city_message_use_popup = 1;
            player->palace_robberies++;
            player->stolen_this_year += amount;
            player->gold_stolen_this_month += amount;
            player->treasury -= amount;
            show_palace_robbed(b->type);
        }
    } else {
        player->destroyed_buildings++;
    }
}

void building_destroy_by_enemy(int32_t x, int32_t y, int32_t grid_offset)
{
    if (map_grids.building_ids[grid_offset] < 1) {
        if (map_grids.terrain[grid_offset] & TERRAIN_WALL) {
            figure_kill_tower_sentries_at(x, y);
        }
        map_building_tiles_set_rubble(0, x, y, 1, 1);
    } else {
        int32_t id = building_main(map_grids.building_ids[grid_offset]);
        building *b = building_get(id);
        if (building_type_is_monument(b->type)) {
            return;
        }
        int32_t type = b->type;
        if (type == BUILDING_GATEHOUSE_UP) {
            building_gatehouse_set_rubble(id);
        } else if (is_venue(type)) {
            int32_t main_id = building_main_part_id(id);
            if (main_id > 0) {
                venue_remove_leaves_rubble = 1;
                venue_remove_tiles(main_id, building_get(main_id)->type);
            }
        } else {
            map_building_tiles_set_rubble(id, b->x, b->y, b->size, b->size);
        }
        if (b->state == BUILDING_STATE_IN_USE) {
            if (building_destroy_counts_for_peace(b->type)) {
                count_destroyed(b, id);
            }
            city_player *player = city_player_current();
            if (player->destroyed_buildings >= DESTROYED_BUILDINGS_MAX) {
                player->destroyed_buildings = DESTROYED_BUILDINGS_MAX;
            }
            b->state = BUILDING_STATE_RUBBLE;
            figure_create_explosion_cloud(b->x, b->y, b->size, 1);
            building_destroy_linked_parts(id, 0);
        }
    }
    figure_tower_sentry_reroute();
    map_tiles_update_region_walls(x, y, 3);
    map_tiles_update_region_canals(x - 2, y - 2, x + 2, y + 2, 0);
    map_routing_update_land_citizen();
    map_routing_update_land_noncitizen();
    map_routing_update_walls();
}

void building_destroy_increase_enemy_damage(int32_t grid_offset, int32_t max_damage)
{
    map_grids.building_damage[grid_offset]++;
    if ((int16_t) map_grids.building_damage[grid_offset] > max_damage) {
        int32_t local = grid_offset - map_info.grid_start;
        building_destroy_by_enemy(local % GRID_SIZE, local / GRID_SIZE, grid_offset);
    }
}
