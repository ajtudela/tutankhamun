// City helpers outside the tick: the population lost with a removed home, the population history and the axis of the
// population graph, the plague spreading from a tile to the healthy houses around it, the costs of the three gifts to
// the king, and the text id of a city message type.
//
// The start year of the graph axis is the end year minus months / 12, even when the start month falls in the year before.

#include "city/population_extra.h"

#include "building/building.h"
#include "building/building_data.h"
#include "building/building_type.h"
#include "city/city_internal.h"
#include "city/finance_estimates.h"
#include "city/health.h"
#include "city/message.h"
#include "city/population.h"
#include "figure/figure_data.h"
#include "game/game_data.h"
#include "map/grid.h"
#include "map/map_data.h"
#include "window/window_data.h"

gift_cost_rule gift_cost_rules[GIFT_SIZES] = {
    [GIFT_MODEST] = { 8, 20 },
    [GIFT_GENEROUS] = { 4, 50 },
    [GIFT_LAVISH] = { 2, 100 },
};

void city_population_remove_home_removed(int32_t player_id, int32_t amount)
{
    city_player *player = city_player_get(player_id);
    player->lost_removal += amount;
    city_population_remove_from_census(player_id, amount);
    player->population = 0;
    int16_t *ages = player->population_at_age;
    for (int32_t age = 0; age < CENSUS_AGES; age++) {
        player->population += ages[age];
    }
    int32_t population = player->population;
    if (population > player->population_highest_ever) {
        player->population_highest_ever = population;
    }
}

int32_t city_population_at_month(int32_t months_back, int32_t offset)
{
    const city_player *player = city_player_current();
    int32_t start = 0;
    if (months_back < player->monthly_population_count) {
        start = player->monthly_population_next_index - months_back;
        if (start < 0) {
            start += MONTHLY_POPULATION_ENTRIES;
        }
    }
    int32_t index = offset + start;
    if (index >= MONTHLY_POPULATION_ENTRIES) {
        index -= MONTHLY_POPULATION_ENTRIES;
    }
    return player->monthly_population[index];
}

void city_population_graph_axis(int32_t months)
{
    if (city_player_current()->monthly_population_count <= months) {
        month_axis_start_month = 0;
        month_axis_start_year = scenario_data.start.year;
        month_axis_end_month = months % MONTHS_PER_YEAR;
        month_axis_end_year = (months / MONTHS_PER_YEAR) + scenario_data.start.year;
        return;
    }
    int32_t years = months / MONTHS_PER_YEAR;
    month_axis_end_year = game_time_year;
    month_axis_end_month = game_time_month - 1;
    if (month_axis_end_month < 0) {
        month_axis_end_month += MONTHS_PER_YEAR;
        month_axis_end_year--;
    }
    month_axis_start_month = month_axis_end_month - (months % MONTHS_PER_YEAR);
    if (month_axis_start_month < 0) {
        month_axis_start_month += MONTHS_PER_YEAR;
    }
    month_axis_start_year = month_axis_end_year - years;
}

void city_health_plague_spread(int32_t x, int32_t y)
{
    int32_t x_min = x - PLAGUE_SPREAD_RADIUS;
    int32_t y_min = y - PLAGUE_SPREAD_RADIUS;
    int32_t x_max = x + PLAGUE_SPREAD_RADIUS;
    int32_t y_max = y + PLAGUE_SPREAD_RADIUS;
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
    int32_t row_step = x_min - x_max + GRID_SIZE - 1;
    map_result_y = y_min;
    map_tile_cursor_offset = map_info.grid_start + (y_min * GRID_SIZE) + x_min;
    for (; map_result_y <= y_max; map_result_y++, map_tile_cursor_offset += row_step) {
        for (map_result_x = x_min; map_result_x <= x_max; map_result_x++, map_tile_cursor_offset++) {
            int32_t id = map_grids.building_ids[map_tile_cursor_offset];
            if (!id) {
                continue;
            }
            const building *b = building_get(id);
            if (b->state && b->type >= BUILDING_HOUSE_CRUDE_HUT && b->type <= BUILDING_HOUSE_PALATIAL_ESTATE && b->house_population != 0 &&
                !b->data.house.is_sick) {
                city_health_infect_house(id);
            }
        }
    }
}

void city_kingdom_gift_costs_update(void)
{
    city_player *player = city_player_current();
    int32_t savings = player->personal_savings;
    for (int32_t gift = GIFT_MODEST; gift < GIFT_SIZES; gift++) {
        const gift_cost_rule *rule = &gift_cost_rules[gift];
        player->gift_cost[gift] = (savings / rule->savings_divisor) + rule->base_cost;
    }
}

int32_t city_message_get_text_id(int32_t type)
{
    return (type < MESSAGE_TYPE_SECOND_TEXT_GROUP) ? type + MESSAGE_FIRST_TEXT_GROUP_OFFSET : type + MESSAGE_SECOND_TEXT_GROUP_OFFSET;
}
