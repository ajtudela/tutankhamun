// City population: the census of people by age 0..99, the people added and removed by immigration, emigration,
// homelessness and disease, the yearly ageing with deaths and births, and the monthly record of the population graph.
// New people take their ages from the random pool without advancing it.
//
// The yearly update request is written to player 1's record whoever plays, and two counters of that record are cleared
// when a flag there is set.

#include "city/population.h"

#include "building/building_data.h"
#include "building/house_population.h"
#include "city/city_data.h"
#include "city/city_init.h"
#include "city/city_internal.h"
#include "city/city_player.h"
#include "core/random.h"

enum census_rule {
    CENSUS_MAX_MISSES = 100,
    CENSUS_RANDOM_AGE_MASK = 63,
    CENSUS_RANDOM_AGE_LIMIT = 51,
    CENSUS_RANDOM_AGE_FOLD = 30,
    CENSUS_CHILD_AGE_LIMIT = 10,
    CENSUS_CHILD_AGE_SHIFT = 20,
    CENSUS_REMOVE_FIRST_AGE = 10,
    DECENNIUM_20S = 2,
    DECENNIUM_30S = 3,
    DECENNIUM_40S = 4,
};

static int32_t death_rate(int32_t health_tens, int32_t decennium)
{
    return deaths_per_health_per_decennium[decennium + (health_tens * CENSUS_DECENNIA)];
}

static void recalculate_population(int32_t player_id)
{
    city_player *player = city_player_get(player_id);
    player->population = 0;
    int32_t population = 0;
    for (int32_t age = 0; age < CENSUS_AGES; age++) {
        population = player->population + player->population_at_age[age];
        player->population = population;
    }
    if (player->population_highest_ever < population) {
        player->population_highest_ever = population;
    }
}

void city_population_remove_from_census(int32_t player_id, int32_t count)
{
    int16_t *ages = city_player_get(player_id)->population_at_age;
    int32_t misses = 0;
    int32_t index = random_pool_index;
    while (count != 0 && misses < CENSUS_MAX_MISSES) {
        int32_t age = random_pool[index] & CENSUS_RANDOM_AGE_MASK;
        if (ages[age] < 1) {
            misses++;
        } else {
            count--;
            ages[age]--;
            misses = 0;
        }
        index++;
        if (index >= RANDOM_POOL_SIZE) {
            index = 0;
        }
    }
    misses = 0;
    int32_t age = CENSUS_REMOVE_FIRST_AGE;
    while (count != 0 && misses < CENSUS_MAX_MISSES) {
        if (ages[age] < 1) {
            misses++;
        } else {
            count--;
            ages[age]--;
            misses = 0;
        }
        age++;
        if (age >= CENSUS_AGES) {
            age = 0;
        }
    }
}

void city_population_add_to_census(int32_t player_id, int32_t count)
{
    int16_t *ages = city_player_get(player_id)->population_at_age;
    int32_t shift_child = 0;
    int32_t index = random_pool_index;
    while (count != 0) {
        uint32_t age = random_pool[index] & CENSUS_RANDOM_AGE_MASK;
        if (age < CENSUS_RANDOM_AGE_LIMIT) {
            if ((age < CENSUS_CHILD_AGE_LIMIT) && shift_child) {
                age += CENSUS_CHILD_AGE_SHIFT;
            }
        } else {
            age -= CENSUS_RANDOM_AGE_FOLD;
        }
        ages[age]++;
        index++;
        if (index >= RANDOM_POOL_SIZE) {
            index = 0;
        }
        shift_child = !shift_child;
        count--;
    }
}

void city_population_add(int32_t player_id, int32_t amount)
{
    city_player_get(player_id)->last_change = amount;
    if (amount < 0) {
        city_population_remove_from_census(player_id, -amount);
    } else if (amount > 0) {
        city_population_add_to_census(player_id, amount);
    }
    recalculate_population(player_id);
}

void city_population_add_homeless(int32_t player_id, int32_t count)
{
    city_player_get(player_id)->lost_homeless -= count;
    city_population_add_to_census(player_id, count);
    recalculate_population(player_id);
}

void city_population_remove_homeless(int32_t player_id, int32_t count)
{
    city_player_get(player_id)->lost_homeless += count;
    city_population_remove_from_census(player_id, count);
    recalculate_population(player_id);
}

int32_t city_population_people_in_age_decennium(int32_t player_id, int32_t decennium)
{
    const int16_t *ages = &city_player_get(player_id)->population_at_age[decennium * AGES_PER_DECENNIUM];
    int32_t people = 0;
    for (int32_t i = 0; i < AGES_PER_DECENNIUM; i++) {
        people += ages[i];
    }
    return people;
}

void city_population_remove_from_census_in_decennium(int32_t player_id, int32_t decennium, int32_t count)
{
    int16_t *ages = &city_player_get(player_id)->population_at_age[decennium * AGES_PER_DECENNIUM];
    int32_t age = 0;
    int32_t misses = 0;
    while (count != 0 && misses < AGES_PER_DECENNIUM) {
        if (ages[age] < 1) {
            misses++;
        } else {
            count--;
            ages[age]--;
            misses = 0;
        }
        age++;
        if (age >= AGES_PER_DECENNIUM) {
            age = 0;
        }
    }
}

int32_t city_population_people_of_working_age(void)
{
    int32_t in_20s = city_population_people_in_age_decennium(city_current_player, DECENNIUM_20S);
    int32_t in_30s = city_population_people_in_age_decennium(city_current_player, DECENNIUM_30S);
    int32_t in_40s = city_population_people_in_age_decennium(city_current_player, DECENNIUM_40S);
    return in_40s + in_20s + in_30s;
}

int32_t city_population_request_yearly_update(void)
{
    city_player_get(CITY_PLAYER_HUMAN)->yearly_update_requested = 1;
    return house_population_calculate_people_per_type(city_current_player);
}

void city_population_clear_player2_capacity(void)
{
    if (city_player_get(CITY_PLAYER_HUMAN)->in_use) {
        city_player_get(CITY_PLAYER_HUMAN)->total_capacity = 0;
        city_player_get(CITY_PLAYER_HUMAN)->room_in_houses = 0;
    }
}

void city_population_yearly_advance_ages_and_deaths(void)
{
    int32_t player_id = city_current_player;
    city_player *player = city_player_get(player_id);
    int16_t *ages = player->population_at_age;
    int32_t old_age_deaths = ages[OLDEST_AGE];
    for (int32_t age = OLDEST_AGE; age > 0; age--) {
        ages[age] = ages[age - 1];
    }
    ages[0] = 0;
    player->yearly_deaths = 0;
    for (int32_t decennium = CENSUS_DECENNIA - 1; decennium >= 0; decennium--) {
        int32_t people = city_population_people_in_age_decennium(player_id, decennium);
        int32_t deaths = calc_adjust_with_percentage(people, death_rate(player->health / 10, decennium));
        int32_t removed = house_population_remove_from_city(player_id, deaths + old_age_deaths);
        city_population_remove_from_census_in_decennium(player_id, decennium, removed);
        old_age_deaths = 0;
        player->yearly_deaths += removed;
    }
}

void city_population_yearly_calculate_births(void)
{
    int32_t player_id = city_current_player;
    city_player *player = city_player_get(player_id);
    player->yearly_births = 0;
    for (int32_t decennium = CENSUS_DECENNIA - 1; decennium >= 0; decennium--) {
        int32_t people = city_population_people_in_age_decennium(player_id, decennium);
        int32_t births = house_population_add_to_city(player_id, calc_adjust_with_percentage(people, births_per_age_decennium[decennium]));
        player->population_at_age[0] = (int16_t) (player->population_at_age[0] + births);
        player->yearly_births += births;
    }
}

void city_population_yearly_recalculate(int32_t player_id)
{
    city_player *player = city_player_get(player_id);
    player->yearly_update_requested = 0;
    player->population_last_year = player->population;
    recalculate_population(player_id);
    player->lost_removal = 0;
    player->total_all_years += player->population;
    player->total_years++;
    player->average_per_year = player->total_all_years / player->total_years;
}

void city_population_record_monthly(void)
{
    city_player *player = city_player_current();
    player->monthly_population[player->monthly_population_next_index] = player->population;
    player->monthly_population_next_index++;
    if (player->monthly_population_next_index >= MONTHLY_POPULATION_ENTRIES) {
        player->monthly_population_next_index = 0;
    }
    player->monthly_population_count++;
}
