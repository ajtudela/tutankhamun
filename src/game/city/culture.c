// City culture. Every day the city-wide coverage of shows, gods, religion, schools, libraries and health services is
// computed as a percentage of the population; the culture update then averages the houses' culture values, counts the
// running shows and the demand for venues, and sets the festival costs (a grand festival without beer becomes a large
// one). Also the temple and shrine building types of a god.
//
// With no god in the city the religion coverage is 0, and the health coverage and the culture update look at every
// building record, not only up to the highest id.

#include "city/culture.h"

#include "building/building.h"
#include "building/building_data.h"
#include "building/building_type.h"
#include "building/entertainment.h"
#include "city/city_data.h"
#include "city/gods.h"
#include "city/kingdom.h"
#include "city/resource_type.h"
#include "core/calc.h"
#include "game/game_internal.h"
#include "window/window_data.h"

enum god_building_table {
    GODS_WITH_TEMPLE_TYPES = 10,
};

int32_t culture_rules[CULTURE_RULE_COUNT] = {
    [CULTURE_RULE_JUGGLERS_PER_VENUE] = 400,
    [CULTURE_RULE_MUSICIANS_PER_VENUE] = 700,
    [CULTURE_RULE_DANCERS_PER_VENUE] = 1200,
    [CULTURE_RULE_SENET_HOUSE_PEOPLE] = 5000,
    [CULTURE_RULE_ZOO_PEOPLE] = 7500,
    [CULTURE_RULE_TEMPLE_COMPLEX_PEOPLE] = 8000,
    [CULTURE_RULE_LOCAL_TEMPLE_PEOPLE] = 750,
    [CULTURE_RULE_LOCAL_SHRINE_PEOPLE] = 300,
    [CULTURE_RULE_PATRON_TEMPLE_PEOPLE] = 375,
    [CULTURE_RULE_PATRON_SHRINE_PEOPLE] = 150,
    [CULTURE_RULE_SCHOOL_CHILDREN] = 300,
    [CULTURE_RULE_LIBRARY_PEOPLE] = 800,
    [CULTURE_RULE_BOOTH_NO_SHOW_WEIGHT] = 1,
    [CULTURE_RULE_BANDSTAND_NO_SHOW_WEIGHT] = 2,
    [CULTURE_RULE_PAVILION_NO_SHOW_WEIGHT] = 3,
    [CULTURE_RULE_SENET_NO_SHOW_WEIGHT] = 100,
    [CULTURE_RULE_ZOO_NO_SHOW_WEIGHT] = 100,
    [CULTURE_RULE_SMALL_FESTIVAL_PEOPLE_PER_DEBEN] = 20,
    [CULTURE_RULE_SMALL_FESTIVAL_BASE_COST] = 10,
    [CULTURE_RULE_LARGE_FESTIVAL_PEOPLE_PER_DEBEN] = 10,
    [CULTURE_RULE_LARGE_FESTIVAL_BASE_COST] = 20,
    [CULTURE_RULE_GRAND_FESTIVAL_PEOPLE_PER_DEBEN] = 5,
    [CULTURE_RULE_GRAND_FESTIVAL_BASE_COST] = 40,
    [CULTURE_RULE_GRAND_FESTIVAL_PEOPLE_PER_BEER_STEP] = 500,
    [CULTURE_RULE_GRAND_FESTIVAL_BEER_STEP] = 100,
};

enum coverage_bound {
    COVERAGE_MAX = 100,
};

enum census_age_end {
    SCHOOL_AGE_END = 14,
    ACADEMY_AGE_END = 21,
};

static int32_t covered_share(int32_t buildings, enum culture_rule people_per_building, int32_t population)
{
    return calc_percentage(buildings * culture_rules[people_per_building], population);
}

static int32_t festival_cost(int32_t population, enum culture_rule people_per_deben, enum culture_rule base_cost)
{
    return (population / culture_rules[people_per_deben]) + culture_rules[base_cost];
}

static int32_t cap_coverage(int32_t value)
{
    return (value > COVERAGE_MAX) ? COVERAGE_MAX : value;
}

int32_t building_type_for_god(int32_t god, int32_t kind)
{
    static const int16_t temples[GODS_WITH_TEMPLE_TYPES] = { BUILDING_TEMPLE_OSIRIS, BUILDING_TEMPLE_RA, BUILDING_TEMPLE_PTAH, BUILDING_TEMPLE_SETH,
        BUILDING_TEMPLE_BAST, BUILDING_TEMPLE_GOD_5, BUILDING_TEMPLE_GOD_6, BUILDING_TEMPLE_GOD_7, BUILDING_TEMPLE_GOD_8, BUILDING_TEMPLE_GOD_9 };
    static const int16_t complexes[GODS_WITH_TEMPLE_TYPES] = { BUILDING_TEMPLE_COMPLEX_OSIRIS, BUILDING_TEMPLE_COMPLEX_RA, BUILDING_TEMPLE_COMPLEX_PTAH,
        BUILDING_TEMPLE_COMPLEX_SETH, BUILDING_TEMPLE_COMPLEX_BAST, BUILDING_TEMPLE_COMPLEX_GOD_5, BUILDING_TEMPLE_COMPLEX_GOD_6, BUILDING_TEMPLE_COMPLEX_GOD_7,
        BUILDING_TEMPLE_COMPLEX_GOD_8, BUILDING_TEMPLE_COMPLEX_GOD_9 };
    if (god < 0 || god >= GODS_WITH_TEMPLE_TYPES) {
        return -1;
    }
    switch (kind) {
        case GOD_BUILDING_TEMPLE:
            return temples[god];
        case GOD_BUILDING_TEMPLE_COMPLEX:
            return complexes[god];
        case GOD_BUILDING_SHRINE:
            return BUILDING_SHRINE_OSIRIS + god;
        default:
            return -1;
    }
}

int32_t city_population_count_school_age(int32_t player_id)
{
    const int16_t *people_at_age = city_player_get(player_id)->population_at_age;
    int32_t count = 0;
    for (int32_t age = 0; age < SCHOOL_AGE_END; age++) {
        count += people_at_age[age];
    }
    return count;
}

int32_t city_population_count_academy_age(int32_t player_id)
{
    const int16_t *people_at_age = city_player_get(player_id)->population_at_age;
    int32_t count = 0;
    for (int32_t age = SCHOOL_AGE_END; age < ACADEMY_AGE_END; age++) {
        count += people_at_age[age];
    }
    return count;
}

void city_culture_update_health_coverage(int32_t *apothecary, int32_t *mortuary, int32_t *physician, int32_t *dentist)
{
    int32_t population = 0;
    int32_t with_apothecary = 0;
    int32_t with_mortuary = 0;
    int32_t with_physician = 0;
    int32_t with_dentist = 0;
    for (int32_t id = 1; id < MAX_BUILDINGS; id++) {
        building *house = building_get(id);
        if (house->state != BUILDING_STATE_IN_USE || !house->house_size || house->house_population == 0) {
            continue;
        }
        int32_t people = house->house_population;
        population += people;
        if (house->data.house.apothecary) {
            with_apothecary += people;
        }
        if (house->data.house.mortuary) {
            with_mortuary += people;
        }
        if (house->data.house.dentist) {
            with_dentist += people;
        }
        if (house->data.house.physician) {
            with_physician += people;
        }
    }
    *apothecary = calc_percentage(with_apothecary, population);
    *mortuary = calc_percentage(with_mortuary, population);
    *dentist = calc_percentage(with_dentist, population);
    *physician = calc_percentage(with_physician, population);
}

static int32_t god_coverage(int32_t god, enum culture_rule people_per_temple, enum culture_rule people_per_shrine)
{
    int32_t complexes = building_count_active[building_type_for_god(god, GOD_BUILDING_TEMPLE_COMPLEX)];
    int32_t temples = building_count_active[building_type_for_god(god, GOD_BUILDING_TEMPLE)];
    int32_t shrines = building_count_active[building_type_for_god(god, GOD_BUILDING_SHRINE)];
    int32_t people = (complexes * culture_rules[CULTURE_RULE_TEMPLE_COMPLEX_PEOPLE]) + (temples * culture_rules[people_per_temple]) +
        (shrines * culture_rules[people_per_shrine]);
    return calc_percentage(people, city_player_current()->population);
}

void city_culture_update_coverage(void)
{
    city_player *player = city_player_current();
    culture_coverage[CULTURE_COVERAGE_JUGGLERS] = covered_share(building_count_active[BUILDING_BOOTH], CULTURE_RULE_JUGGLERS_PER_VENUE, player->population);
    culture_coverage[CULTURE_COVERAGE_MUSICIANS] = covered_share(building_count_active[BUILDING_BANDSTAND], CULTURE_RULE_MUSICIANS_PER_VENUE, player->population);
    culture_coverage[CULTURE_COVERAGE_DANCERS] = covered_share(building_count_active[BUILDING_PAVILION], CULTURE_RULE_DANCERS_PER_VENUE, player->population);
    culture_coverage[CULTURE_COVERAGE_SENET] = covered_share(building_count_active[BUILDING_SENET_HOUSE], CULTURE_RULE_SENET_HOUSE_PEOPLE, player->population);
    culture_coverage[CULTURE_COVERAGE_ZOO] = covered_share(building_count_active[BUILDING_ZOO], CULTURE_RULE_ZOO_PEOPLE, player->population);
    culture_coverage[CULTURE_COVERAGE_JUGGLERS] = cap_coverage(culture_coverage[CULTURE_COVERAGE_JUGGLERS]);
    culture_coverage[CULTURE_COVERAGE_MUSICIANS] = cap_coverage(culture_coverage[CULTURE_COVERAGE_MUSICIANS]);
    culture_coverage[CULTURE_COVERAGE_DANCERS] = cap_coverage(culture_coverage[CULTURE_COVERAGE_DANCERS]);
    culture_coverage[CULTURE_COVERAGE_SENET] = cap_coverage(culture_coverage[CULTURE_COVERAGE_SENET]);
    culture_coverage[CULTURE_COVERAGE_ZOO] = cap_coverage(culture_coverage[CULTURE_COVERAGE_ZOO]);
    int32_t sum = 0;
    int32_t gods = 0;
    for (int32_t god = 0; god < GOD_COUNT; god++) {
        if (scenario_data.god_status[god] == GOD_STATUS_PATRON) {
            culture_coverage[CULTURE_COVERAGE_GOD + god] = god_coverage(god, CULTURE_RULE_PATRON_TEMPLE_PEOPLE, CULTURE_RULE_PATRON_SHRINE_PEOPLE);
            gods++;
        } else if (scenario_data.god_status[god] == GOD_STATUS_LOCAL) {
            culture_coverage[CULTURE_COVERAGE_GOD + god] = god_coverage(god, CULTURE_RULE_LOCAL_TEMPLE_PEOPLE, CULTURE_RULE_LOCAL_SHRINE_PEOPLE);
            gods++;
        } else {
            culture_coverage[CULTURE_COVERAGE_GOD + god] = 0;
        }
        culture_coverage[CULTURE_COVERAGE_GOD + god] = cap_coverage(culture_coverage[CULTURE_COVERAGE_GOD + god]);
        sum += culture_coverage[CULTURE_COVERAGE_GOD + god];
    }
    culture_coverage[CULTURE_COVERAGE_RELIGION] = (gods != 0) ? (sum / gods) : 0;
    player->school_age = city_population_count_school_age(city_current_player);
    player->academy_age = city_population_count_academy_age(city_current_player);
    culture_coverage[CULTURE_COVERAGE_SCHOOL] = covered_share(building_count_active[BUILDING_SCRIBAL_SCHOOL], CULTURE_RULE_SCHOOL_CHILDREN, player->school_age);
    culture_coverage[CULTURE_COVERAGE_LIBRARY] = covered_share(building_count_active[BUILDING_LIBRARY], CULTURE_RULE_LIBRARY_PEOPLE, player->population);
    culture_coverage[CULTURE_COVERAGE_SCHOOL] = cap_coverage(culture_coverage[CULTURE_COVERAGE_SCHOOL]);
    culture_coverage[CULTURE_COVERAGE_LIBRARY] = cap_coverage(culture_coverage[CULTURE_COVERAGE_LIBRARY]);
    city_culture_update_health_coverage(&culture_coverage[CULTURE_COVERAGE_APOTHECARY], &culture_coverage[CULTURE_COVERAGE_MORTUARY],
        &culture_coverage[CULTURE_COVERAGE_PHYSICIAN], &culture_coverage[CULTURE_COVERAGE_DENTIST]);
}

void city_culture_calculate(void)
{
    city_player *player = city_player_current();
    player->average_entertainment = 0;
    player->juggler_shows = 0;
    player->juggler_no_shows_weighted = 0;
    player->musician_shows = 0;
    player->musician_no_shows_weighted = 0;
    player->dancer_shows = 0;
    player->dancer_no_shows_weighted = 0;
    player->senet_shows = 0;
    player->senet_no_shows_weighted = 0;
    player->zoo_shows = 0;
    player->zoo_no_shows_weighted = 0;
    player->venue_needing_shows = VENUE_NONE;
    player->average_religion = 0;
    player->average_education = 0;
    player->average_health = 0;
    int32_t houses = 0;
    for (int32_t id = 1; id < MAX_BUILDINGS; id++) {
        building *house = building_get(id);
        if (house->state != BUILDING_STATE_IN_USE || !house->house_size) {
            continue;
        }
        houses++;
        player->average_entertainment += house->data.house.entertainment;
        player->average_religion += house->data.house.num_gods;
        player->average_education += house->data.house.education;
        player->average_health += house->data.house.health;
    }
    if (houses) {
        player->average_entertainment /= houses;
        player->average_religion /= houses;
        player->average_education /= houses;
        player->average_health /= houses;
    }
    for (int32_t id = 1; id < MAX_BUILDINGS; id++) {
        building *venue = building_get(id);
        if (venue->state != BUILDING_STATE_IN_USE) {
            continue;
        }
        switch (venue->type) {
            case BUILDING_BOOTH:
                if (venue->data.entertainment.show_days_left[SHOW_JUGGLERS]) {
                    player->juggler_shows++;
                } else {
                    player->juggler_no_shows_weighted += culture_rules[CULTURE_RULE_BOOTH_NO_SHOW_WEIGHT];
                }
                break;
            case BUILDING_BANDSTAND:
                if (venue->data.entertainment.show_days_left[SHOW_JUGGLERS]) {
                    player->juggler_shows++;
                } else {
                    player->juggler_no_shows_weighted += culture_rules[CULTURE_RULE_BANDSTAND_NO_SHOW_WEIGHT];
                }
                if (venue->data.entertainment.show_days_left[SHOW_MUSICIANS]) {
                    player->musician_shows++;
                } else {
                    player->musician_no_shows_weighted += culture_rules[CULTURE_RULE_BANDSTAND_NO_SHOW_WEIGHT];
                }
                break;
            case BUILDING_PAVILION:
                if (venue->data.entertainment.show_days_left[SHOW_JUGGLERS]) {
                    player->juggler_shows++;
                } else {
                    player->juggler_no_shows_weighted += culture_rules[CULTURE_RULE_PAVILION_NO_SHOW_WEIGHT];
                }
                if (venue->data.entertainment.show_days_left[SHOW_MUSICIANS]) {
                    player->musician_shows++;
                } else {
                    player->musician_no_shows_weighted += culture_rules[CULTURE_RULE_PAVILION_NO_SHOW_WEIGHT];
                }
                if (venue->data.entertainment.show_days_left[SHOW_DANCERS]) {
                    player->dancer_shows++;
                } else {
                    player->dancer_no_shows_weighted += culture_rules[CULTURE_RULE_PAVILION_NO_SHOW_WEIGHT];
                }
                break;
            case BUILDING_SENET_HOUSE:
                if (venue->data.entertainment.show_days_left[SHOW_JUGGLERS]) {
                    player->senet_shows++;
                } else {
                    player->senet_no_shows_weighted += culture_rules[CULTURE_RULE_SENET_NO_SHOW_WEIGHT];
                }
                break;
            case BUILDING_ZOO:
                if (venue->data.entertainment.show_days_left[SHOW_JUGGLERS]) {
                    player->zoo_shows++;
                } else {
                    player->zoo_no_shows_weighted += culture_rules[CULTURE_RULE_ZOO_NO_SHOW_WEIGHT];
                }
                break;
        }
    }
    int32_t max_demand = 0;
    if (player->juggler_no_shows_weighted > 0) {
        player->venue_needing_shows = VENUE_JUGGLERS;
        max_demand = player->juggler_no_shows_weighted;
    }
    if (player->musician_no_shows_weighted > max_demand) {
        player->venue_needing_shows = VENUE_MUSICIANS;
        max_demand = player->musician_no_shows_weighted;
    }
    if (player->dancer_no_shows_weighted > max_demand) {
        player->venue_needing_shows = VENUE_DANCERS;
        max_demand = player->dancer_no_shows_weighted;
    }
    if (player->senet_no_shows_weighted > max_demand) {
        player->venue_needing_shows = VENUE_SENET;
        max_demand = player->senet_no_shows_weighted;
    }
    if (player->zoo_no_shows_weighted > max_demand) {
        player->venue_needing_shows = VENUE_ZOO;
    }
    player->festival_cost[FESTIVAL_SMALL - 1] = festival_cost(
        player->population, CULTURE_RULE_SMALL_FESTIVAL_PEOPLE_PER_DEBEN, CULTURE_RULE_SMALL_FESTIVAL_BASE_COST);
    player->festival_cost[FESTIVAL_LARGE - 1] = festival_cost(
        player->population, CULTURE_RULE_LARGE_FESTIVAL_PEOPLE_PER_DEBEN, CULTURE_RULE_LARGE_FESTIVAL_BASE_COST);
    player->festival_cost[FESTIVAL_GRAND - 1] = festival_cost(
        player->population, CULTURE_RULE_GRAND_FESTIVAL_PEOPLE_PER_DEBEN, CULTURE_RULE_GRAND_FESTIVAL_BASE_COST);
    int32_t beer_steps = (player->population / culture_rules[CULTURE_RULE_GRAND_FESTIVAL_PEOPLE_PER_BEER_STEP]) + 1;
    player->festival_grand_beer = beer_steps * culture_rules[CULTURE_RULE_GRAND_FESTIVAL_BEER_STEP];
    player->festival_not_enough_beer = 0;
    if (player->storage_stored[RESOURCE_BEER] < player->festival_grand_beer) {
        player->festival_not_enough_beer = 1;
        if (player->festival_selected_size == FESTIVAL_GRAND) {
            player->festival_selected_size = FESTIVAL_LARGE;
        }
    }
}
