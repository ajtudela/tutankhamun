// House population: the room of the houses reachable from the entry, immigrants and emigrants, eviction from
// overcrowded houses, the yearly births and deaths, the working people and the population milestone messages.
//
// The migration has these peculiarities:
// - immigrants fill houses with room for 8 or more first; the pass for empty houses never runs;
// - the newcomers counter adds the day's total, not the people of one call;
// - a house emptied by eviction is removed as by undo.

#include "building/house_population.h"
#include "building/building.h"
#include "building/house_evolve.h"
#include "building/house_size.h"
#include "city/city_player.h"
#include "city/message.h"
#include "city/population.h"
#include "city/sentiment.h"
#include "core/calc.h"
#include "figure/figure.h"
#include "figure/figure_action.h"
#include "figure/figure_type.h"

enum {
    MIGRANT_GROUP_MIN = 4,
    IMMIGRANT_GROUP_MAX = 4,
    EMIGRANT_GROUP_MAX = 4,
    IMMIGRANT_LARGE_ROOM = 8,
    WORKING_AGE_PERCENT = 60,
    CLOSEST_HOUSE_MAX_DISTANCE = 1000,
    DEATH_PASSES = 4,
};

static const int32_t population_milestones[] = { 500, 1000, 2000, 3000, 5000, 10000, 15000, 20000, 25000 };

int32_t house_population_max_people(const building *b)
{
    int32_t max_people = house_models[b->subtype].max_people;
    if (b->house_is_merged) {
        max_people <<= 2;
    }
    return max_people;
}

static int32_t is_house_in_use(const building *b)
{
    return b->state == BUILDING_STATE_IN_USE && b->house_size;
}

static int32_t is_house_reachable(const building *b)
{
    return b->distance_from_entry > 0;
}

void house_population_fill_building_list_with_houses(void)
{
    int32_t count = 0;
    building_list_large_size = 0;
    for (int32_t id = 1; id <= building_highest_id; id++) {
        if (is_house_in_use(building_get(id))) {
            building_list_large[count++] = (int16_t) id;
        }
    }
    building_list_large_size = count;
}

void house_population_update_room(void)
{
    city_population_clear_player2_capacity();
    house_population_fill_building_list_with_houses();
    building_list_large_index = 0;
    int32_t count = building_list_large_size;
    if (count <= 0) {
        return;
    }
    building_list_large_index = count;
    city_player *player = city_player_current();
    for (int32_t i = 0; i < count; i++) {
        building *b = building_get(building_list_large[i]);
        b->house_population_room = 0;
        if (!is_house_reachable(b)) {
            continue;
        }
        int32_t max_people = house_population_max_people(b);
        int32_t population = b->house_population;
        player->total_capacity += max_people;
        player->room_in_houses += max_people - population;
        if (!b->data.house.is_sick) {
            b->house_population_room = (int16_t) (max_people - population);
        }
        if (b->house_population > b->house_highest_population) {
            b->house_highest_population = b->house_population;
        }
    }
}

static int32_t take_migration_batch(int32_t amount, int32_t *queue_size)
{
    if (amount <= 0) {
        return 0;
    }
    if (amount >= MIGRANT_GROUP_MIN) {
        return amount;
    }
    amount += *queue_size;
    if (amount >= MIGRANT_GROUP_MIN) {
        *queue_size = 0;
        return amount;
    }
    *queue_size = amount;
    return 0;
}

static void post_population_milestones(const city_player *player)
{
    int32_t count = (int32_t) (sizeof(population_milestones) / sizeof(population_milestones[0]));
    for (int32_t i = 0; i < count; i++) {
        if (player->population >= population_milestones[i] && !city_message_population_shown[POPULATION_SHOWN_FIRST_MILESTONE + i]) {
            city_message_use_popup = 1;
            city_message_post(MESSAGE_POPULATION_500 + i, 0, 0, NULL, 0);
            city_message_population_shown[POPULATION_SHOWN_FIRST_MILESTONE + i] = 1;
        }
    }
}

void house_population_update_migration(void)
{
    city_migration_update(city_current_player);
    city_player *player = city_player_current();
    player->immigrated_today = 0;
    player->emigrated_today = 0;
    player->refused_immigrants_today = 0;
    int32_t immigrants = take_migration_batch(player->immigration_amount_per_batch, &player->immigration_queue_size);
    if (immigrants > 0) {
        house_population_create_immigrants(immigrants);
    }
    int32_t emigrants = take_migration_batch(player->emigration_amount_per_batch, &player->emigration_queue_size);
    if (emigrants > 0) {
        house_population_create_emigrants(emigrants);
    }
    player->immigration_amount_per_batch = 0;
    player->emigration_amount_per_batch = 0;
    if (player->yearly_update_requested) {
        city_population_yearly_advance_ages_and_deaths();
        city_population_yearly_calculate_births();
        city_population_yearly_recalculate(city_current_player);
    }
    house_population_calculate_working_people();
    post_population_milestones(player);
}

void house_population_calculate_working_people(void)
{
    int32_t plebs = 0;
    int32_t patricians = 0;
    building_list_large_index = 0;
    int32_t count = building_list_large_size;
    if (count > 0) {
        building_list_large_index = count;
        for (int32_t i = 0; i < count; i++) {
            building *b = building_get(building_list_large[i]);
            if (b->house_population <= 0) {
                continue;
            }
            if (b->subtype >= HOUSE_LEVEL_COMMON_MANOR) {
                patricians += b->house_population;
            } else {
                plebs += b->house_population;
            }
        }
    }
    city_player *player = city_player_current();
    player->percentage_plebs = calc_percentage(plebs, patricians + plebs);
    player->working_age = calc_adjust_with_percentage(city_population_people_of_working_age(), WORKING_AGE_PERCENT);
    player->workers_available = calc_adjust_with_percentage(player->working_age, player->percentage_plebs);
}

static void forget_dead_immigrants(void)
{
    for (int32_t i = 0; i < building_list_large_size; i++) {
        building *b = building_get(building_list_large[i]);
        if (b->immigrant_figure_id && figure_get(b->immigrant_figure_id)->state != FIGURE_STATE_ALIVE) {
            b->immigrant_figure_id = 0;
        }
    }
}

static int32_t can_take_immigrants(const building *b, int32_t min_room)
{
    return is_house_reachable(b) && b->house_population_room >= min_room && !b->immigrant_figure_id;
}

static int32_t send_immigrants_to_large_rooms(int32_t remaining)
{
    int32_t i = 0;
    while (i < building_list_large_size && remaining > 0) {
        int32_t id = building_list_large[i];
        building_list_large_index = ++i;
        if (can_take_immigrants(building_get(id), IMMIGRANT_LARGE_ROOM)) {
            int32_t group = (remaining > IMMIGRANT_GROUP_MAX) ? IMMIGRANT_GROUP_MAX : remaining;
            figure_create_immigrant(id, group);
            remaining -= group;
        }
        i = building_list_large_index;
    }
    return remaining;
}

static int32_t send_immigrants_to_any_room(int32_t remaining)
{
    building_list_large_index = 0;
    while (building_list_large_index < building_list_large_size && remaining > 0) {
        int32_t id = building_list_large[building_list_large_index++];
        building *b = building_get(id);
        if (can_take_immigrants(b, 1)) {
            int32_t group = (remaining > b->house_population_room) ? b->house_population_room : remaining;
            figure_create_immigrant(id, group);
            remaining -= group;
        }
    }
    return remaining;
}

void house_population_create_immigrants(int32_t num_people)
{
    if (building_list_large_size > 0 && num_people > 0) {
        forget_dead_immigrants();
    }
    int32_t remaining = send_immigrants_to_large_rooms(num_people);
    remaining = send_immigrants_to_any_room(remaining);
    city_player *player = city_player_current();
    player->immigrated_today += num_people - remaining;
    player->migration_newcomers += player->immigrated_today;
    if (remaining != 0 && num_people == remaining) {
        player->refused_immigrants_today += remaining;
    }
}

void house_population_create_emigrants(int32_t num_people)
{
    int32_t remaining = num_people;
    for (int32_t level = HOUSE_LEVEL_CRUDE_HUT; remaining > 0 && level < HOUSE_LEVEL_COMMON_RESIDENCE; level++) {
        building_list_large_index = 0;
        while (building_list_large_index < building_list_large_size && remaining > 0) {
            int32_t id = building_list_large[building_list_large_index++];
            building *b = building_get(id);
            int32_t population = b->house_population;
            if (population <= 0 || b->subtype != level) {
                continue;
            }
            int32_t group = (population < EMIGRANT_GROUP_MAX) ? population : EMIGRANT_GROUP_MAX;
            if (remaining < group) {
                group = remaining;
            }
            figure_create_emigrant(id, group);
            remaining -= group;
        }
    }
    city_player_current()->emigrated_today += num_people - remaining;
}

int32_t house_population_closest_house_with_room(int32_t x, int32_t y)
{
    int32_t min_distance = CLOSEST_HOUSE_MAX_DISTANCE;
    int32_t min_id = 0;
    for (int32_t id = 1; id <= building_highest_id; id++) {
        building *b = building_get(id);
        if (!is_house_in_use(b) || !can_take_immigrants(b, 1)) {
            continue;
        }
        int32_t distance = calc_maximum_distance(x, y, b->x, b->y);
        if (distance < min_distance) {
            min_distance = distance;
            min_id = id;
        }
    }
    return min_id;
}

void figure_create_immigrant(int32_t building_id, int32_t num_people)
{
    city_player *player = city_player_current();
    int32_t figure_id = figure_create(city_current_player, FIGURE_IMMIGRANT, player->entry_point_x, player->entry_point_y, 0);
    building *b = building_get(building_id);
    figure *f = figure_get(figure_id);
    b->immigrant_figure_id = (int16_t) figure_id;
    f->action_state = FIGURE_ACTION_IMMIGRANT_CREATED;
    f->immigrant_building_id = (int16_t) building_id;
    f->wait_ticks = (int16_t) ((b->house_figure_generation_delay & 0x7F) + 10);
    f->migrant_num_people = (uint8_t) num_people;
}

void figure_create_emigrant(int32_t building_id, int32_t num_people)
{
    building *b = building_get(building_id);
    city_population_add(city_current_player, -num_people);
    if (num_people < b->house_population) {
        b->house_population = (int16_t) (b->house_population - num_people);
    } else {
        b->house_population = 0;
        building_house_change_to_vacant_lot(building_id);
    }
    int32_t figure_id = figure_create(city_current_player, FIGURE_EMIGRANT, b->x, b->y, 0);
    figure *f = figure_get(figure_id);
    f->action_state = FIGURE_ACTION_EMIGRANT_CREATED;
    f->wait_ticks = 0;
    f->migrant_num_people = (uint8_t) num_people;
}

static int32_t next_building_id(int32_t id)
{
    return (id + 1 >= MAX_BUILDINGS) ? 1 : (id + 1);
}

int32_t house_population_add_to_city(int32_t player_id, int32_t num_people)
{
    city_player *player = city_player_get(player_id);
    int32_t added = 0;
    int32_t id = player->last_used_house_add;
    for (int32_t tries = 1; tries < MAX_BUILDINGS && added < num_people; tries++) {
        id = next_building_id(id);
        building *b = building_get(id);
        if (!is_house_in_use(b) || !is_house_reachable(b) || b->house_population <= 0) {
            continue;
        }
        player->last_used_house_add = id;
        int32_t max_people = house_population_max_people(b);
        int32_t population = b->house_population;
        if (max_people - population > 0) {
            added++;
            population++;
            b->house_population = (int16_t) population;
            b->house_population_room = (int16_t) (max_people - population);
        }
    }
    return added;
}

int32_t house_population_remove_from_city(int32_t player_id, int32_t num_people)
{
    city_player *player = city_player_get(player_id);
    int32_t removed = 0;
    int32_t id = player->last_used_house_remove;
    for (int32_t tries = 1; tries < DEATH_PASSES * MAX_BUILDINGS && removed < num_people; tries++) {
        id = next_building_id(id);
        building *b = building_get(id);
        if (!is_house_in_use(b)) {
            continue;
        }
        player->last_used_house_remove = (int16_t) id;
        if (b->house_population > 0) {
            removed++;
            b->house_population--;
        }
    }
    return removed;
}

static int32_t is_house_counted(const building *b)
{
    switch (b->state) {
        case BUILDING_STATE_UNUSED:
        case BUILDING_STATE_UNDO:
        case BUILDING_STATE_DELETED_BY_GAME:
        case BUILDING_STATE_DELETED:
            return 0;
        default:
            return b->house_size != 0;
    }
}

int32_t house_population_calculate_people_per_type(int32_t player_id)
{
    city_player *player = city_player_get(player_id);
    int32_t total = 0;
    player->people_in_huts_and_shanties = 0;
    player->people_in_manors_and_up = 0;
    player->people_in_huts = 0;
    player->people_in_residences_and_up = 0;
    int32_t highest = building_highest_id;
    for (int32_t id = 1; id <= highest; id++) {
        building *b = building_get(id);
        if (!is_house_counted(b)) {
            continue;
        }
        int32_t population = b->house_population;
        int32_t level = b->subtype;
        total += population;
        if (level <= HOUSE_LEVEL_STURDY_HUT) {
            player->people_in_huts += population;
        }
        if (level <= HOUSE_LEVEL_COMMON_SHANTY) {
            player->people_in_huts_and_shanties += population;
        }
        if (level >= HOUSE_LEVEL_COMMON_RESIDENCE) {
            player->people_in_residences_and_up += population;
        }
        if (level >= HOUSE_LEVEL_COMMON_MANOR) {
            player->people_in_manors_and_up += population;
        }
    }
    return total;
}

void figure_create_homeless(int32_t player_id, int32_t x, int32_t y, int32_t num_people)
{
    int32_t figure_id = figure_create(player_id, FIGURE_HOMELESS, x, y, 0);
    figure *f = figure_get(figure_id);
    f->action_state = FIGURE_ACTION_HOMELESS_CREATED;
    f->wait_ticks = 0;
    f->migrant_num_people = (uint8_t) num_people;
    city_population_remove_homeless(player_id, num_people);
}

void house_population_evict_overcrowded(void)
{
    building_list_large_index = 0;
    while (building_list_large_index < building_list_large_size) {
        building *b = building_get(building_list_large[building_list_large_index++]);
        if (b->house_population_room >= 0) {
            continue;
        }
        int32_t num_people = -b->house_population_room;
        figure_create_homeless(city_current_player, b->x, b->y, num_people);
        if (num_people >= b->house_population) {
            b->state = BUILDING_STATE_UNDO;
        } else {
            b->house_population = (int16_t) (b->house_population - num_people);
        }
    }
}
