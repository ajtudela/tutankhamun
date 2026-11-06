// City health, updated monthly: every house gains malaria and plague risk (by its level, the city health and the
// distance to marsh and water), a full risk is an outbreak that kills or drives out people and spreads the plague to
// the houses around, and the city health moves towards a target from the houses' services, the population and Bast's
// temple complex. The outbreaks are posted as messages.

#include "city/health.h"

#include "building/building.h"
#include "building/building_data.h"
#include "building/building_figure_helpers.h"
#include "building/house_evolve.h"
#include "building/temple_complex.h"
#include "city/city_data.h"
#include "city/city_disasters.h"
#include "city/city_internal.h"
#include "city/message.h"
#include "city/population.h"
#include "core/random.h"
#include "figure/animals.h"
#include "figure/figure_data.h"
#include "game/game_data.h"
#include "game/tutorial.h"
#include "map/grid.h"
#include "scenario/scenario_climate.h"
#include "widget/widget_data.h"
#include "window/window_data.h"

#include <math.h>
#include <stdbool.h>

static const float HEALTH_COVERAGE_SCALE = 0.01f;

enum {
    HEALTH_POPULATION_BONUS_ENTRIES = 10,
};

int32_t health_rules[HEALTH_RULE_COUNT] = {
    [HEALTH_RULE_SICK_MONTHS] = 2,
    [HEALTH_RULE_TUTORIAL_HEALTH] = 75,
    [HEALTH_RULE_MONTHLY_STEP] = 2,
    [HEALTH_RULE_WARNING_MIN_POPULATION] = 300,
    [HEALTH_RULE_PLAGUE_RADIUS_HEALTH_STEP] = 20,
    [HEALTH_RULE_PLAGUE_MAX_RADIUS] = 4,
    [HEALTH_RULE_BAST_ALTAR_PERCENTAGE] = 105,
    [HEALTH_RULE_DISEASE_CHANCE] = 40,
    [HEALTH_RULE_MORTUARY_POINTS] = 25,
    [HEALTH_RULE_PHYSICIAN_POINTS] = 15,
    [HEALTH_RULE_FOOD_POINTS] = 10,
    [HEALTH_RULE_SECOND_FOOD_POINTS] = 5,
    [HEALTH_RULE_THIRD_FOOD_POINTS] = 5,
    [HEALTH_RULE_PHYSICIAN_AND_FOOD_POINTS] = 40,
    [HEALTH_RULE_MALARIA_HEALTH_LEVEL] = 50,
    [HEALTH_RULE_MALARIA_HEALTH_WEIGHT] = 2,
    [HEALTH_RULE_PLAGUE_MARSH_DISTANCE] = 55,
    [HEALTH_RULE_PLAGUE_MARSH_WEIGHT] = 2,
    [HEALTH_RULE_PLAGUE_BONUS] = 16,
    [HEALTH_RULE_PLAGUE_DESERT_BONUS] = 8,
    [HEALTH_RULE_PLAGUE_DESERT_DIVISOR] = 2,
};

void city_health_infect_house(int32_t building_id)
{
    building *house = building_get(building_id);
    city_population_add(city_current_player, -house->house_population);
    house->house_population = 0;
    house->data.house.is_sick = 1;
    house->house_sick_months = health_rules[HEALTH_RULE_SICK_MONTHS];
    house->house_plague_risk = 0;
    house->house_malaria_risk = 0;
    if (settings_data.is_custom_scenario == 0 && settings_data.mission_id == tutorial_briefing_tables.tutorial.entries[TUTORIAL_FLAG_DISEASE].mission && tutorial_flags_a[TUTORIAL_FLAG_DISEASE] == 0) {
        tutorial_set_flag(TUTORIAL_FLAG_DISEASE);
    }
}

void city_health_malaria(int32_t building_id)
{
    city_health_infect_house(building_id);
    city_view.disease_message_malaria_building = building_id;
    city_view.disease_message_malaria = 1;
}

bool city_health_spread_plague(int32_t building_id)
{
    city_health_infect_house(building_id);
    city_view.disease_message_plague = 1;
    city_view.disease_message_plague_building = building_id;
    int32_t radius = (HEALTH_MAX - city_player_current()->health) / health_rules[HEALTH_RULE_PLAGUE_RADIUS_HEALTH_STEP];
    if (game_time_month == 9 || game_time_month == 10 || game_time_month == 11) {
        radius++;
    }
    if (radius <= 0) {
        return 0;
    }
    if (radius > health_rules[HEALTH_RULE_PLAGUE_MAX_RADIUS]) {
        radius = health_rules[HEALTH_RULE_PLAGUE_MAX_RADIUS];
    }
    building *source = building_get(building_id);
    int32_t x_min = source->x - radius;
    int32_t x_max = source->x + radius;
    int32_t y_min = source->y - radius;
    int32_t y_max = source->y + radius;
    if (x_min < 0) {
        x_min = 0;
    }
    if (y_min < 0) {
        y_min = 0;
    }
    if (x_max >= map_info.grid_width) {
        x_max = map_info.grid_width - 1;
    }
    if (y_max >= map_info.grid_height) {
        y_max = map_info.grid_height - 1;
    }
    for (int32_t y = y_min; y <= y_max; y++) {
        for (int32_t x = x_min; x <= x_max; x++) {
            int32_t id = map_grids.building_ids[map_info.grid_start + (y * GRID_SIZE) + x];
            if (id == 0) {
                continue;
            }
            building *house = building_get(id);
            if (house->state == BUILDING_STATE_UNUSED || house->house_size == 0 || house->house_population == 0) {
                continue;
            }
            int32_t dx = ((x - source->x) > (source->x - x)) ? (x - source->x) : (source->x - x);
            int32_t dy = ((y - source->y) > (source->y - y)) ? (y - source->y) : (source->y - y);
            house->house_plague_distance = (dx > dy) ? dx : dy;
        }
    }
    return 1;
}

void city_health_decrease_plague_distance(void)
{
    for (int32_t id = 1; id <= building_highest_id; id++) {
        building *house = building_get(id);
        if (house->state != BUILDING_STATE_UNUSED && house->house_size != 0 && house->house_population != 0 && house->house_plague_distance != 0) {
            if (--house->house_plague_distance == 0) {
                city_health_infect_house(id);
            }
        }
    }
}

void city_health_post_disease_messages(void)
{
    if (city_view.disease_message_other) {
        city_message_use_popup = 1;
        building *b = building_get(city_view.disease_message_other_building);
        city_message_post(MESSAGE_A_PLAGUE, b->type, b->grid_offset, NULL, 0);
        city_view.disease_message_other = 0;
    }
    if (city_view.disease_message_malaria) {
        city_message_use_popup = 1;
        building *b = building_get(city_view.disease_message_malaria_building);
        city_message_post(MESSAGE_DISEASE_STRIKES, b->type, b->grid_offset, NULL, 0);
        city_view.disease_message_malaria = 0;
    }
    if (city_view.disease_message_plague) {
        city_message_use_popup = 1;
        building *b = building_get(city_view.disease_message_plague_building);
        city_message_post(MESSAGE_MALARIA, b->type, b->grid_offset, NULL, 0);
        city_view.disease_message_plague = 0;
    }
}

bool city_health_roll_disease_chance(void)
{
    random_generate_next();
    return (random_generator_state.random1_15bit % 100) < (health_rules[HEALTH_RULE_DISEASE_CHANCE] - city_player_current()->health);
}

static int32_t nearest_distance(building *b, int32_t mask, int32_t max_distance)
{
    int32_t found_x;
    int32_t found_y;
    if (!animal_find_nearest_figure_of_types(b->x, b->y, 0, NULL, mask, &found_x, &found_y, max_distance, 0)) {
        return max_distance;
    }
    int32_t dy = b->y - found_y;
    int32_t dx = b->x - found_x;
    return crt_ftol_double(sqrt((double) ((dy * dy) + (dx * dx))));
}

void city_health_update_house(int32_t building_id, bool recompute)
{
    random_generate_next();
    const city_player *player = city_player_current();
    building *house = building_get(building_id);
    int32_t level = house->subtype;
    int32_t health_shortfall = health_rules[HEALTH_RULE_MALARIA_HEALTH_LEVEL] - player->health;
    house->house_plague_risk += house_models[level].disease_risk + (health_shortfall * health_rules[HEALTH_RULE_MALARIA_HEALTH_WEIGHT]);
    if (house->house_plague_risk < 0) {
        house->house_plague_risk = 0;
    }
    if (house->house_plague_risk > HOUSE_RISK_MAX) {
        house->house_plague_risk = HOUSE_RISK_MAX;
    }
    int32_t marsh_distance;
    int32_t water_distance;
    if (!recompute) {
        marsh_distance = house->house_marsh_distance;
        water_distance = house->house_water_distance;
    } else {
        int32_t max_distance = map_info.grid_width;
        if (map_info.grid_width < map_info.grid_height) {
            max_distance = map_info.grid_height;
        }
        marsh_distance = nearest_distance(house, TERRAIN_MARSH, max_distance);
        water_distance = nearest_distance(house, TERRAIN_WATER, max_distance);
        house->house_water_distance = water_distance;
        house->house_marsh_distance = marsh_distance;
    }
    int32_t divisor = 1;
    int32_t bonus = health_rules[HEALTH_RULE_PLAGUE_BONUS];
    if (scenario_data.climate_type == CLIMATE_CENTRAL) {
        water_distance = 1;
    } else if (scenario_data.climate_type == CLIMATE_DESERT) {
        bonus = health_rules[HEALTH_RULE_PLAGUE_DESERT_BONUS];
        divisor = health_rules[HEALTH_RULE_PLAGUE_DESERT_DIVISOR];
    }
    int32_t marsh_nearness = (health_rules[HEALTH_RULE_PLAGUE_MARSH_DISTANCE] - marsh_distance) * health_rules[HEALTH_RULE_PLAGUE_MARSH_WEIGHT];
    int32_t change = (house_models[level].malaria_risk - player->health + marsh_nearness - water_distance + bonus) / divisor;
    house->house_malaria_risk += change;
    if (house->house_malaria_risk < 0) {
        house->house_malaria_risk = 0;
    }
    if (house->house_malaria_risk > HOUSE_RISK_MAX) {
        house->house_malaria_risk = HOUSE_RISK_MAX;
    }
}

void city_health_update(void)
{
    city_player *player = city_player_current();
    player->plague_houses = 0;
    int32_t tutorial = 0;
    if (settings_data.mission_id < 2 && settings_data.is_custom_scenario == 0) {
        tutorial = 1;
        player->health = health_rules[HEALTH_RULE_TUTORIAL_HEALTH];
        player->health_target = health_rules[HEALTH_RULE_TUTORIAL_HEALTH];
    } else {
        int32_t people = 0;
        int32_t healthy = 0;
        for (int32_t id = 1; id < MAX_BUILDINGS; id++) {
            building *house = building_get(id);
            if (house->state != BUILDING_STATE_IN_USE || house->house_size == 0 || house->house_population == 0) {
                continue;
            }
            people += house->house_population;
            city_health_update_house(id, city_health_recompute_distances);
            if (house->house_malaria_risk == HOUSE_RISK_MAX) {
                city_health_spread_plague(id);
                player->plague_houses++;
            } else if (house->house_plague_risk == HOUSE_RISK_MAX) {
                city_health_malaria(id);
            }
            int32_t coverage = 0;
            if (house->data.house.mortuary) {
                coverage = health_rules[HEALTH_RULE_MORTUARY_POINTS];
            }
            uint8_t physician = house->data.house.physician;
            if (physician) {
                coverage += health_rules[HEALTH_RULE_PHYSICIAN_POINTS];
            }
            uint8_t food = house->data.house.food_types;
            if (food) {
                coverage += health_rules[HEALTH_RULE_FOOD_POINTS];
            }
            if (food > 1) {
                coverage += health_rules[HEALTH_RULE_SECOND_FOOD_POINTS];
            }
            if (food > 2) {
                coverage += health_rules[HEALTH_RULE_THIRD_FOOD_POINTS];
            }
            if (food && physician) {
                coverage += health_rules[HEALTH_RULE_PHYSICIAN_AND_FOOD_POINTS];
            }
            double scaled = (double) coverage * (double) HEALTH_COVERAGE_SCALE;
            healthy += crt_ftol_double(scaled * (double) house->house_population);
        }
        city_health_recompute_distances = 0;
        player->health_target = calc_percentage(healthy, people);
        int32_t bonus = 0;
        for (int32_t i = 0; i < HEALTH_POPULATION_BONUS_ENTRIES; i++) {
            if (player->population < health_population_bonus[i * 2]) {
                bonus = health_population_bonus[(i * 2) + 1];
                break;
            }
        }
        player->health_target += bonus;
        if (player->has_temple_complex && building_temple_god(player->temple_complex_building_id) == GOD_BAST &&
            building_temple_complex_has_upgrade(player->temple_complex_building_id, TEMPLE_COMPLEX_ALTAR)) {
            player->health_target = calc_adjust_with_percentage(player->health_target, health_rules[HEALTH_RULE_BAST_ALTAR_PERCENTAGE]);
        }
        if (city_health_roll_disease_chance()) {
            city_health_start_plague(0);
        }
        int32_t old_health = player->health;
        int32_t warning = 0;
        if (old_health < player->health_target) {
            player->health = old_health + health_rules[HEALTH_RULE_MONTHLY_STEP];
            if (player->health > player->health_target) {
                player->health = player->health_target;
            }
            int32_t health = player->health;
            if (((health / 10) != (old_health / 10)) && player->population > health_rules[HEALTH_RULE_WARNING_MIN_POPULATION] && health >= 10) {
                if (health < 20) {
                    warning = WARNING_PLAGUE_LIKELY_HEALTH_IMPROVING;
                } else if (health < 30) {
                    warning = WARNING_RISK_OF_PLAGUE_BUT_IMPROVING;
                } else if (health < 40) {
                    warning = WARNING_HEALTH_IMPROVING_PLAGUE_STILL_POSSIBLE;
                }
            }
        } else if (old_health > player->health_target) {
            player->health = old_health - health_rules[HEALTH_RULE_MONTHLY_STEP];
            if (player->health < player->health_target) {
                player->health = player->health_target;
            }
            int32_t health = player->health;
            if ((((health / 10) != (old_health / 10)) && player->population > health_rules[HEALTH_RULE_WARNING_MIN_POPULATION]) || health < 20) {
                if (health < 10) {
                    warning = WARNING_CITY_HEALTH_APPALLING_PLAGUE_IMMINENT;
                } else if (health < 20) {
                    warning = WARNING_CITY_HEALTH_TERRIBLE_PLAGUE_LIKELY;
                } else if (health < 30) {
                    warning = WARNING_CITY_HEALTH_WORSENING_SERIOUS_RISK;
                } else if (health < 40) {
                    warning = WARNING_CITY_HEALTH_GETTING_WORSE_PLAGUE_COULD_STRIKE;
                }
            }
        }
        if (warning) {
            city_warning_show(warning);
        }
        if (player->health > HEALTH_MAX) {
            player->health = HEALTH_MAX;
        }
        if (player->health < 0) {
            player->health = 0;
        }
    }
    for (int32_t id = 1; id < MAX_BUILDINGS; id++) {
        building *house = building_get(id);
        if (house->state == BUILDING_STATE_IN_USE && house->house_size != 0 && house->data.house.is_sick && house->house_sick_months != 0) {
            if (--house->house_sick_months == 0) {
                house->data.house.is_sick = 0;
            }
        }
    }
    if (!tutorial) {
        city_health_decrease_plague_distance();
        city_health_post_disease_messages();
    }
}
