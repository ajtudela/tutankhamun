// City labor of the tick: the labor category of every building in use, the workers each category needs, the workers
// handed to the categories (by the player's priorities when short), the unemployment and the workers-needed message,
// and each building's share of its category's workers. A ferry landing opens its route when both landings have
// workers.
//
// An industry counts as mothballed by its output resource only, and the weight pass computes a percentage for category 8
// and then drops it.

#include "city/labor.h"

#include "building/building.h"
#include "building/building_industry.h"
#include "building/building_type.h"
#include "building/construction_place.h"
#include "city/city_data.h"
#include "city/city_player.h"
#include "city/message.h"
#include "city/resource_type.h"
#include "core/calc.h"
#include "figure/boats.h"
#include "game/game_data.h"
#include "game/game_internal.h"
#include "map/routing_terrain.h"
#include "window/window_data.h"

#include <stdbool.h>

enum labor_employment_rule {
    WORKERS_NEEDED_MESSAGE_DELAY = 6,
    SENATE_UNEMPLOYMENT_STEP = 5,
};

typedef struct {
    int32_t category;
    int32_t workers;
} labor_round;

static const labor_round labor_round_robin[LABOR_PRIORITY_CATEGORIES] = {
    { LABOR_CATEGORY_FOOD_PRODUCTION, 4 },
    { LABOR_CATEGORY_INDUSTRY_COMMERCE, 3 },
    { LABOR_CATEGORY_INFRASTRUCTURE, 2 },
    { LABOR_CATEGORY_RELIGION, 3 },
    { LABOR_CATEGORY_WATER_HEALTH, 1 },
    { LABOR_CATEGORY_MILITARY, 2 },
    { LABOR_CATEGORY_GOVERNMENT, 1 },
    { LABOR_CATEGORY_ENTERTAINMENT, 1 },
    { LABOR_CATEGORY_EDUCATION, 1 },
};

static bool is_floodplain_farm(int32_t building_id)
{
    return building_is_floodplain_farm(building_id) != 0;
}

int32_t city_labor_industry_is_mothballed(int32_t building_id)
{
    building *b = building_get(building_id);
    return (city_player_current()->mothballed[b->output_resource]) != 0;
}

void city_labor_calculate_workers_needed_per_category(void)
{
    city_player *player = city_player_current();
    for (int32_t i = 0; i < LABOR_CATEGORY_COUNT; i++) {
        labor_category *c = &player->labor_categories[i];
        c->buildings = 0;
        c->total_houses_covered = 0;
        c->workers_allocated = 0;
        c->workers_needed = 0;
    }
    for (int32_t id = 1; id <= building_highest_id; id++) {
        building *b = building_get(id);
        if (b->state != BUILDING_STATE_IN_USE) {
            continue;
        }
        int32_t category = building_property_get(b->type)->labor_category;
        if (category == LABOR_CATEGORY_FOOD_PRODUCTION && is_floodplain_farm(id)) {
            category = LABOR_CATEGORY_NONE;
        }
        b->labor_category = (int8_t) category;
        if (category < 0) {
            continue;
        }
        if (category == LABOR_CATEGORY_RELIGION && b->prev_part_building_id != 0) {
            continue;
        }
        if ((category == LABOR_CATEGORY_FOOD_PRODUCTION || category == LABOR_CATEGORY_INDUSTRY_COMMERCE) && city_labor_industry_is_mothballed(id)) {
            continue;
        }
        int32_t houses = b->houses_covered;
        if (houses <= 0) {
            continue;
        }
        labor_category *c = &player->labor_categories[category];
        c->workers_needed += building_model_table[b->type].laborers;
        c->total_houses_covered += houses;
        c->buildings++;
    }
}

void city_labor_allocate_workers_to_categories(void)
{
    city_player *player = city_player_current();
    labor_category *c = player->labor_categories;
    int32_t total_needed = 0;
    for (int32_t i = 0; i < LABOR_CATEGORY_COUNT; i++) {
        c[i].workers_allocated = 0;
    }
    for (int32_t i = 0; i < LABOR_CATEGORY_COUNT; i++) {
        total_needed += c[i].workers_needed;
    }
    player->workers_needed = 0;
    int32_t remaining = player->workers_available;
    if (total_needed <= remaining) {
        for (int32_t i = 0; i < LABOR_CATEGORY_COUNT; i++) {
            c[i].workers_allocated = c[i].workers_needed;
        }
        player->workers_employed = total_needed;
    } else {
        for (int32_t priority = 1; priority <= LABOR_PRIORITY_MAX; priority++) {
            for (int32_t i = 0; i < LABOR_PRIORITY_CATEGORIES; i++) {
                if (c[i].priority == priority) {
                    int32_t to_allocate = c[i].workers_needed;
                    if (to_allocate > remaining) {
                        to_allocate = remaining;
                    }
                    c[i].workers_allocated = to_allocate;
                    remaining -= to_allocate;
                    break;
                }
            }
            if (remaining <= 0) {
                break;
            }
        }
        if (player->workers_available > 1) {
            for (int32_t round = 1; remaining > 0;) {
                for (int32_t i = 0; i < LABOR_PRIORITY_CATEGORIES; i++) {
                    labor_category *cat = &c[labor_round_robin[i].category];
                    if (cat->priority != 0) {
                        continue;
                    }
                    int32_t missing = cat->workers_needed - cat->workers_allocated;
                    if (missing <= 0) {
                        continue;
                    }
                    int32_t to_allocate = labor_round_robin[i].workers;
                    if (to_allocate > remaining) {
                        to_allocate = remaining;
                    }
                    if (to_allocate > missing) {
                        to_allocate = missing;
                    }
                    remaining -= to_allocate;
                    cat->workers_allocated += to_allocate;
                    if (remaining <= 0) {
                        break;
                    }
                }
                if (remaining <= 0) {
                    break;
                }
                round++;
                if (round >= player->workers_available) {
                    break;
                }
            }
        }
        player->workers_employed = player->workers_available;
        for (int32_t i = 0; i < LABOR_PRIORITY_CATEGORIES; i++) {
            player->workers_needed += c[i].workers_needed - c[i].workers_allocated;
        }
    }
    player->workers_unemployed = player->workers_available - player->workers_employed;
    player->unemployment_percentage = calc_percentage(player->workers_available - player->workers_employed, player->workers_available);
}

void city_labor_check_employment(void)
{
    city_player *player = city_player_current();
    int32_t orig_needed = player->workers_needed;
    city_labor_allocate_workers_to_categories();
    int32_t unemployment = player->unemployment_percentage;
    int32_t senate = player->unemployment_percentage_for_senate;
    if (unemployment < senate || (unemployment - SENATE_UNEMPLOYMENT_STEP) < senate) {
        player->unemployment_percentage_for_senate = unemployment;
    } else {
        player->unemployment_percentage_for_senate = senate + SENATE_UNEMPLOYMENT_STEP;
    }
    if (player->unemployment_percentage_for_senate > 100) {
        player->unemployment_percentage_for_senate = 100;
    }
    if (orig_needed == 0 && player->workers_needed > 0 && game_time_year >= scenario_data.start.year && city_message_delay[MESSAGE_CATEGORY_WORKERS_NEEDED] <= 0) {
        city_message_delay[MESSAGE_CATEGORY_WORKERS_NEEDED] = WORKERS_NEEDED_MESSAGE_DELAY;
        city_message_post(MESSAGE_WORKERS_NEEDED, 0, 0, 0, 0);
    }
}

void city_labor_set_building_worker_weight(void)
{
    const labor_category *categories = city_player_current()->labor_categories;
    for (int32_t id = 1; id <= building_highest_id; id++) {
        building *b = building_get(id);
        if (b->state != BUILDING_STATE_IN_USE) {
            continue;
        }
        int32_t category = building_property_get(b->type)->labor_category;
        if (category == LABOR_CATEGORY_FOOD_PRODUCTION) {
            if (is_floodplain_farm(id)) {
                continue;
            }
        } else if (category == LABOR_CATEGORY_RELIGION) {
            if (b->prev_part_building_id != 0) {
                continue;
            }
        } else if (category < 0) {
            continue;
        }
        int32_t total = categories[category].total_houses_covered;
        b->percentage_houses_covered = 0;
        if (b->houses_covered != 0) {
            b->percentage_houses_covered = (int16_t) calc_percentage(b->houses_covered * 100, total);
        }
    }
}

static void update_ferry_routes(void)
{
    for (int32_t id = 1; id <= building_highest_id; id++) {
        const building *landing = building_get(id);
        if (landing->state != BUILDING_STATE_IN_USE || landing->type != BUILDING_FERRY_LANDING) {
            continue;
        }
        const building *other_landing = building_get(landing->linked_building_id);
        if (landing->num_workers != 0 && other_landing->num_workers > 0) {
            map_routing_set_ferry_route(id, FERRY_ROUTE_ADD, FERRY_ROUTE_OPEN);
        } else {
            map_routing_set_ferry_route(id, FERRY_ROUTE_ADD, FERRY_ROUTE_CLOSED);
            ferry_queues_release(id);
        }
    }
}

static void hand_out_leftover_workers(int32_t *workers_left)
{
    for (int32_t id = 1; id <= building_highest_id; id++) {
        building *b = building_get(id);
        if (b->state != BUILDING_STATE_IN_USE) {
            continue;
        }
        int32_t category = building_property_get(b->type)->labor_category;
        if (category == LABOR_CATEGORY_FOOD_PRODUCTION) {
            if (is_floodplain_farm(id)) {
                continue;
            }
            if (city_labor_industry_is_mothballed(id)) {
                continue;
            }
        } else if (category < 0 || category == LABOR_CATEGORY_MILITARY) {
            continue;
        } else if (category == LABOR_CATEGORY_INDUSTRY_COMMERCE) {
            if (city_labor_industry_is_mothballed(id)) {
                continue;
            }
        } else if (category == LABOR_CATEGORY_ENTERTAINMENT && b->prev_part_building_id != 0) {
            continue;
        }
        if (b->percentage_houses_covered <= 0 || workers_left[category] == 0) {
            continue;
        }
        int32_t laborers = building_model_table[b->type].laborers;
        int32_t workers = b->num_workers;
        if (workers >= laborers) {
            continue;
        }
        int32_t missing = laborers - workers;
        if (missing <= workers_left[category]) {
            b->num_workers = (int16_t) (workers + missing);
            workers_left[category] -= missing;
        } else {
            b->num_workers = (int16_t) (workers + workers_left[category]);
            workers_left[category] = 0;
        }
    }
}

void city_labor_allocate_workers_to_buildings(void)
{
    int32_t workers_left[LABOR_CATEGORY_COUNT];
    labor_category *c = city_player_current()->labor_categories;
    for (int32_t i = 0; i < LABOR_CATEGORY_COUNT; i++) {
        workers_left[i] = (c[i].workers_allocated < c[i].workers_needed);
    }
    for (int32_t id = 1; id <= building_highest_id; id++) {
        building *b = building_get(id);
        if (b->state != BUILDING_STATE_IN_USE) {
            continue;
        }
        int32_t category = building_property_get(b->type)->labor_category;
        if ((category == LABOR_CATEGORY_FOOD_PRODUCTION || category == LABOR_CATEGORY_INDUSTRY_COMMERCE) && is_floodplain_farm(id)) {
            continue;
        }
        if (category < 0) {
            continue;
        }
        if (category == LABOR_CATEGORY_RELIGION && b->prev_part_building_id != 0) {
            continue;
        }
        b->num_workers = 0;
        if (category == LABOR_CATEGORY_FOOD_PRODUCTION || category == LABOR_CATEGORY_INDUSTRY_COMMERCE) {
            if (city_labor_industry_is_mothballed(id)) {
                continue;
            }
        } else if (category == LABOR_CATEGORY_ENTERTAINMENT) {
            if ((b->type == BUILDING_BOOTH || b->type == BUILDING_BANDSTAND || b->type == BUILDING_PAVILION) && b->prev_part_building_id != 0) {
                continue;
            }
        } else if (category == LABOR_CATEGORY_MILITARY) {
            if ((b->type == BUILDING_WEAPONSMITH || b->type == BUILDING_CHARIOT_MAKER) && city_labor_industry_is_mothballed(id)) {
                continue;
            }
        }
        int32_t weight = b->percentage_houses_covered;
        if (weight <= 0) {
            continue;
        }
        int32_t laborers = building_model_table[b->type].laborers;
        if (workers_left[category] == 0) {
            b->num_workers = (int16_t) laborers;
        } else {
            int32_t workers = ((c[category].workers_allocated * weight) / 100) / 100;
            if (workers >= laborers) {
                workers = laborers;
            }
            b->num_workers = (int16_t) workers;
            workers_left[category] += workers;
        }
    }
    for (int32_t i = 0; i < LABOR_CATEGORY_COUNT; i++) {
        if (workers_left[i]) {
            workers_left[i]--;
        }
    }
    for (int32_t i = 0; i < LABOR_CATEGORY_COUNT; i++) {
        workers_left[i] = (workers_left[i] < c[i].workers_allocated) ? (c[i].workers_allocated - workers_left[i]) : 0;
    }
    hand_out_leftover_workers(workers_left);
    update_ferry_routes();
}
