// House services, culture and desirability: the daily decay of service coverage and tax coverage, the house
// culture values (entertainment, education, health, religion), the desirability of every building, and a house's
// tax income as the overlays show it.
//
// The decay is computed on signed bytes.

#include "building/house_services.h"
#include "building/building.h"
#include "building/building_type.h"
#include "building/house_evolve.h"
#include "city/culture.h"
#include "city/gods.h"
#include "map/building_tiles.h"
#include <stdint.h>

int32_t house_service_rules[HOUSE_SERVICE_RULE_COUNT] = {
    [HOUSE_SERVICE_RULE_COVERAGE_POINTS_DIVISOR] = 5,
    [HOUSE_SERVICE_RULE_JUGGLER_POINTS] = 10,
    [HOUSE_SERVICE_RULE_MUSICIAN_POINTS] = 20,
    [HOUSE_SERVICE_RULE_DANCER_POINTS] = 30,
    [HOUSE_SERVICE_RULE_SENET_POINTS] = 40,
    [HOUSE_SERVICE_RULE_ZOO_POINTS] = 40,
    [HOUSE_SERVICE_RULE_ZOO_FILLS_ABOVE] = 60,
    [HOUSE_SERVICE_RULE_ENTERTAINMENT_MAX] = 100,
    [HOUSE_SERVICE_RULE_WATER_DESIRABILITY] = 10,
    [HOUSE_SERVICE_RULE_ELEVATION_1_DESIRABILITY] = 10,
    [HOUSE_SERVICE_RULE_ELEVATION_2_DESIRABILITY] = 12,
    [HOUSE_SERVICE_RULE_ELEVATION_3_DESIRABILITY] = 14,
    [HOUSE_SERVICE_RULE_ELEVATION_4_DESIRABILITY] = 16,
    [HOUSE_SERVICE_RULE_ELEVATION_5_DESIRABILITY] = 18,
};

enum venue_coverage_average {
    VENUE_COVERAGES = 5,
};

enum desirability_search {
    DESIRABILITY_SEARCH_START = -9999,
};

void house_service_decay_houses_covered(void)
{
    for (int32_t id = 1; id <= building_highest_id; id++) {
        building *b = building_get(id);
        if (b->state == BUILDING_STATE_UNUSED || b->type == BUILDING_MUD_TOWER || b->type == BUILDING_BRICK_TOWER || b->type == BUILDING_TOWER) {
            continue;
        }
        if (b->houses_covered > 1) {
            b->houses_covered--;
        } else {
            b->houses_covered = 0;
        }
    }
}

static void decay_service(uint8_t *value)
{
    *value = (*value > 1) ? (uint8_t) (*value - 1) : 0;
}

static void decay_house_services(building_house *house)
{
    decay_service(&house->juggler);
    decay_service(&house->musician);
    decay_service(&house->dancer);
    decay_service(&house->senet);
    decay_service(&house->zoo);
    decay_service(&house->courthouse);
    decay_service(&house->school);
    decay_service(&house->library);
    decay_service(&house->dentist);
    decay_service(&house->apothecary);
    decay_service(&house->physician);
    decay_service(&house->water_supply);
    decay_service(&house->mortuary);
    for (int32_t god = 0; god < GOD_COUNT; god++) {
        decay_service(&house->temple[god]);
    }
}

void house_service_decay_culture(void)
{
    for (int32_t id = 1; id <= building_highest_id; id++) {
        building *b = building_get(id);
        if (b->state != BUILDING_STATE_IN_USE || !b->house_size) {
            continue;
        }
        decay_house_services(&b->data.house);
    }
}

void house_service_calculate_culture_aggregates(void)
{
    for (int32_t id = 1; id <= building_highest_id; id++) {
        building *b = building_get(id);
        if (b->state != BUILDING_STATE_IN_USE || !b->house_size) {
            continue;
        }
        b->data.house.education = 0;
        int32_t average = (culture_coverage[CULTURE_COVERAGE_ZOO] + culture_coverage[CULTURE_COVERAGE_SENET] + culture_coverage[CULTURE_COVERAGE_DANCERS] +
                              culture_coverage[CULTURE_COVERAGE_MUSICIANS] + culture_coverage[CULTURE_COVERAGE_JUGGLERS]) /
            VENUE_COVERAGES;
        b->data.house.health = 0;
        b->data.house.num_gods = 0;
        int8_t entertainment = (int8_t) (average / house_service_rules[HOUSE_SERVICE_RULE_COVERAGE_POINTS_DIVISOR]);
        if (b->data.house.juggler) {
            entertainment += house_service_rules[HOUSE_SERVICE_RULE_JUGGLER_POINTS];
        }
        if (b->data.house.musician) {
            entertainment += house_service_rules[HOUSE_SERVICE_RULE_MUSICIAN_POINTS];
        }
        if (b->data.house.dancer) {
            entertainment += house_service_rules[HOUSE_SERVICE_RULE_DANCER_POINTS];
        }
        if (b->data.house.senet) {
            entertainment += house_service_rules[HOUSE_SERVICE_RULE_SENET_POINTS];
        }
        if (b->data.house.zoo) {
            if (entertainment <= house_service_rules[HOUSE_SERVICE_RULE_ZOO_FILLS_ABOVE]) {
                entertainment += house_service_rules[HOUSE_SERVICE_RULE_ZOO_POINTS];
            } else {
                entertainment = (int8_t) house_service_rules[HOUSE_SERVICE_RULE_ENTERTAINMENT_MAX];
            }
        }
        if (entertainment > house_service_rules[HOUSE_SERVICE_RULE_ENTERTAINMENT_MAX]) {
            entertainment = (int8_t) house_service_rules[HOUSE_SERVICE_RULE_ENTERTAINMENT_MAX];
        }
        b->data.house.entertainment = entertainment;
        if (b->data.house.school || b->data.house.library) {
            b->data.house.education = HOUSE_EDUCATION_SCHOOL_OR_LIBRARY;
            if (b->data.house.school && b->data.house.library) {
                b->data.house.education = HOUSE_EDUCATION_SCHOOL_AND_LIBRARY;
            }
        }
        for (int32_t god = 0; god < GOD_COUNT; god++) {
            if (b->data.house.temple[god]) {
                b->data.house.num_gods++;
            }
        }
        if (b->data.house.physician) {
            b->data.house.health++;
        }
        if (b->data.house.mortuary) {
            b->data.house.health++;
        }
    }
}

void building_update_desirability(void)
{
    for (int32_t id = 1; id <= building_highest_id; id++) {
        building *b = building_get(id);
        if (b->state != BUILDING_STATE_IN_USE) {
            continue;
        }
        int32_t grid_offset = b->grid_offset;
        int32_t size = b->size;
        if (size == 1) {
            b->desirability = map_grids.desirability[grid_offset];
        } else {
            int32_t max = DESIRABILITY_SEARCH_START;
            for (int32_t row = 0; row < size; row++) {
                for (int32_t col = 0; col < size; col++) {
                    int32_t value = map_grids.desirability[grid_offset + building_square_tile_offset(row, col)];
                    if (value > max) {
                        max = value;
                    }
                }
            }
            b->desirability = (int8_t) max;
        }
        if (b->is_adjacent_to_water) {
            b->desirability += house_service_rules[HOUSE_SERVICE_RULE_WATER_DESIRABILITY];
        }
        int32_t elevation = map_grids.elevation[grid_offset];
        if (elevation == 1) {
            b->desirability += house_service_rules[HOUSE_SERVICE_RULE_ELEVATION_1_DESIRABILITY];
        } else if (elevation == 2) {
            b->desirability += house_service_rules[HOUSE_SERVICE_RULE_ELEVATION_2_DESIRABILITY];
        } else if (elevation == 3) {
            b->desirability += house_service_rules[HOUSE_SERVICE_RULE_ELEVATION_3_DESIRABILITY];
        } else if (elevation == 4) {
            b->desirability += house_service_rules[HOUSE_SERVICE_RULE_ELEVATION_4_DESIRABILITY];
        } else if (elevation >= 5) {
            b->desirability += house_service_rules[HOUSE_SERVICE_RULE_ELEVATION_5_DESIRABILITY];
        }
    }
}

void house_service_decay_tax_collector(void)
{
    for (int32_t id = 1; id <= building_highest_id; id++) {
        building *b = building_get(id);
        if (b->state == BUILDING_STATE_IN_USE && b->house_tax_coverage) {
            b->house_tax_coverage--;
        }
    }
}

int32_t building_house_tax_income(int32_t building_id)
{
    const building *b = building_get(building_id);
    if (b->house_size == 0) {
        return 0;
    }
    return (b->data.house.tax_income + (HOUSE_TAX_INCOME_UNIT / 2)) / HOUSE_TAX_INCOME_UNIT;
}
