// House evolution and consumption: once a day every house runs the evolve rule of its level (merge, evolve,
// expand, devolve after a delay or split), and on days 0 and 7 consumes beer, pottery, linen and jewelry. Frozen
// houses keep their level.
//
// Two limits of the evolve rules stay:
// - the top level's evolve desirability is never reached;
// - a good amount that is not a multiple of 4 consumes only 0 or 1, by its remainder and the day of the month.

#include "building/house_evolve.h"
#include "building/building.h"
#include "building/building_type.h"
#include "building/house_evolve_checks.h"
#include "building/house_size.h"
#include "city/city_player.h"
#include "game/game_data.h"
#include "map/routing_terrain.h"
#include "map/tiles.h"
#include <stdint.h>

int32_t house_evolve_current_id;

typedef void (*house_evolve_callback)(void);

enum house_evolve_action {
    HOUSE_ACTION_NONE,
    HOUSE_ACTION_CHANGE_TO,
    HOUSE_ACTION_EXPAND,
    HOUSE_ACTION_DEVOLVE_SPLIT,
};

enum house_consumption {
    HOUSE_CONSUMPTION_DAY_FIRST = 0,
    HOUSE_CONSUMPTION_DAY_SECOND = 7,
    HOUSE_CONSUMPTION_QUARTERS = 4,
    HOUSE_CONSUMED_GOODS = 4,
};

typedef struct {
    uint8_t needs_population;
    uint8_t merge;
    uint8_t evolve;
    uint8_t devolve;
    uint8_t evolve_type;
    uint8_t expand_tiles;
    uint8_t devolve_type;
    uint8_t split_size;
} evolve_rule;

static const evolve_rule evolve_rules[HOUSE_LEVEL_COUNT] = {
    [HOUSE_LEVEL_CRUDE_HUT] = { .needs_population = 1, .merge = 1, .evolve = HOUSE_ACTION_CHANGE_TO, .evolve_type = BUILDING_HOUSE_STURDY_HUT },
    [HOUSE_LEVEL_STURDY_HUT] = { .needs_population = 1, .merge = 1, .evolve = HOUSE_ACTION_CHANGE_TO, .evolve_type = BUILDING_HOUSE_MEAGER_SHANTY,
        .devolve = HOUSE_ACTION_CHANGE_TO, .devolve_type = BUILDING_HOUSE_CRUDE_HUT },
    [HOUSE_LEVEL_MEAGER_SHANTY] = { .merge = 1, .evolve = HOUSE_ACTION_CHANGE_TO, .evolve_type = BUILDING_HOUSE_COMMON_SHANTY,
        .devolve = HOUSE_ACTION_CHANGE_TO, .devolve_type = BUILDING_HOUSE_STURDY_HUT },
    [HOUSE_LEVEL_COMMON_SHANTY] = { .merge = 1, .evolve = HOUSE_ACTION_CHANGE_TO, .evolve_type = BUILDING_HOUSE_ROUGH_COTTAGE,
        .devolve = HOUSE_ACTION_CHANGE_TO, .devolve_type = BUILDING_HOUSE_MEAGER_SHANTY },
    [HOUSE_LEVEL_ROUGH_COTTAGE] = { .merge = 1, .evolve = HOUSE_ACTION_CHANGE_TO, .evolve_type = BUILDING_HOUSE_ORDINARY_COTTAGE,
        .devolve = HOUSE_ACTION_CHANGE_TO, .devolve_type = BUILDING_HOUSE_COMMON_SHANTY },
    [HOUSE_LEVEL_ORDINARY_COTTAGE] = { .merge = 1, .evolve = HOUSE_ACTION_CHANGE_TO, .evolve_type = BUILDING_HOUSE_MODEST_HOMESTEAD,
        .devolve = HOUSE_ACTION_CHANGE_TO, .devolve_type = BUILDING_HOUSE_ROUGH_COTTAGE },
    [HOUSE_LEVEL_MODEST_HOMESTEAD] = { .merge = 1, .evolve = HOUSE_ACTION_CHANGE_TO, .evolve_type = BUILDING_HOUSE_SPACIOUS_HOMESTEAD,
        .devolve = HOUSE_ACTION_CHANGE_TO, .devolve_type = BUILDING_HOUSE_ORDINARY_COTTAGE },
    [HOUSE_LEVEL_SPACIOUS_HOMESTEAD] = { .merge = 1, .evolve = HOUSE_ACTION_CHANGE_TO, .evolve_type = BUILDING_HOUSE_MODEST_APARTMENT,
        .devolve = HOUSE_ACTION_CHANGE_TO, .devolve_type = BUILDING_HOUSE_MODEST_HOMESTEAD },
    [HOUSE_LEVEL_MODEST_APARTMENT] = { .merge = 1, .evolve = HOUSE_ACTION_CHANGE_TO, .evolve_type = BUILDING_HOUSE_SPACIOUS_APARTMENT,
        .devolve = HOUSE_ACTION_CHANGE_TO, .devolve_type = BUILDING_HOUSE_SPACIOUS_HOMESTEAD },
    [HOUSE_LEVEL_SPACIOUS_APARTMENT] = { .merge = 1, .evolve = HOUSE_ACTION_EXPAND, .expand_tiles = HOUSE_TILES_2X2,
        .devolve = HOUSE_ACTION_CHANGE_TO, .devolve_type = BUILDING_HOUSE_MODEST_APARTMENT },
    [HOUSE_LEVEL_COMMON_RESIDENCE] = { .evolve = HOUSE_ACTION_CHANGE_TO, .evolve_type = BUILDING_HOUSE_SPACIOUS_RESIDENCE,
        .devolve = HOUSE_ACTION_DEVOLVE_SPLIT, .split_size = 2 },
    [HOUSE_LEVEL_SPACIOUS_RESIDENCE] = { .evolve = HOUSE_ACTION_CHANGE_TO, .evolve_type = BUILDING_HOUSE_ELEGANT_RESIDENCE,
        .devolve = HOUSE_ACTION_CHANGE_TO, .devolve_type = BUILDING_HOUSE_COMMON_RESIDENCE },
    [HOUSE_LEVEL_ELEGANT_RESIDENCE] = { .evolve = HOUSE_ACTION_CHANGE_TO, .evolve_type = BUILDING_HOUSE_FANCY_RESIDENCE,
        .devolve = HOUSE_ACTION_CHANGE_TO, .devolve_type = BUILDING_HOUSE_SPACIOUS_RESIDENCE },
    [HOUSE_LEVEL_FANCY_RESIDENCE] = { .evolve = HOUSE_ACTION_EXPAND, .expand_tiles = HOUSE_TILES_3X3,
        .devolve = HOUSE_ACTION_CHANGE_TO, .devolve_type = BUILDING_HOUSE_ELEGANT_RESIDENCE },
    [HOUSE_LEVEL_COMMON_MANOR] = { .evolve = HOUSE_ACTION_CHANGE_TO, .evolve_type = BUILDING_HOUSE_SPACIOUS_MANOR,
        .devolve = HOUSE_ACTION_DEVOLVE_SPLIT, .split_size = 3 },
    [HOUSE_LEVEL_SPACIOUS_MANOR] = { .evolve = HOUSE_ACTION_CHANGE_TO, .evolve_type = BUILDING_HOUSE_ELEGANT_MANOR,
        .devolve = HOUSE_ACTION_CHANGE_TO, .devolve_type = BUILDING_HOUSE_COMMON_MANOR },
    [HOUSE_LEVEL_ELEGANT_MANOR] = { .evolve = HOUSE_ACTION_CHANGE_TO, .evolve_type = BUILDING_HOUSE_STATELY_MANOR,
        .devolve = HOUSE_ACTION_CHANGE_TO, .devolve_type = BUILDING_HOUSE_SPACIOUS_MANOR },
    [HOUSE_LEVEL_STATELY_MANOR] = { .evolve = HOUSE_ACTION_EXPAND, .expand_tiles = HOUSE_TILES_4X4,
        .devolve = HOUSE_ACTION_CHANGE_TO, .devolve_type = BUILDING_HOUSE_ELEGANT_MANOR },
    [HOUSE_LEVEL_MODEST_ESTATE] = { .evolve = HOUSE_ACTION_CHANGE_TO, .evolve_type = BUILDING_HOUSE_PALATIAL_ESTATE,
        .devolve = HOUSE_ACTION_DEVOLVE_SPLIT, .split_size = 4 },
    [HOUSE_LEVEL_PALATIAL_ESTATE] = { .evolve = HOUSE_ACTION_CHANGE_TO, .evolve_type = BUILDING_HOUSE_MODEST_ESTATE,
        .devolve = HOUSE_ACTION_CHANGE_TO, .devolve_type = BUILDING_HOUSE_MODEST_ESTATE },
};

int32_t house_evolve_is_frozen(void)
{
    if (houses_frozen == 0) {
        return 0;
    }
    int32_t id = house_evolve_current_id;
    const building *b = building_get(id);
    if (!b->house_is_merged) {
        building_house_change_to(id, b->type);
    }
    return 1;
}

int32_t house_check_desirability(int32_t building_id)
{
    const building *b = building_get(building_id);
    int32_t level = b->subtype;
    int32_t evolve = house_models[level].evolve_desirability;
    if (level >= HOUSE_LEVEL_MAX) {
        evolve = HOUSE_TOP_LEVEL_EVOLVE_DESIRABILITY;
    }
    int32_t desirability = b->desirability;
    if (desirability <= house_models[level].devolve_desirability) {
        return HOUSE_EVOLVE_DEVOLVE;
    }
    return (desirability >= evolve) ? HOUSE_EVOLVE_UP : HOUSE_EVOLVE_STAY;
}

static void evolve_expand(int32_t num_tiles)
{
    int32_t id = house_evolve_current_id;
    if (!building_house_can_expand(id, num_tiles)) {
        return;
    }
    if (num_tiles == HOUSE_TILES_2X2) {
        building_get(id)->house_is_merged = 0;
    }
    building_house_split_houses_in_area(id, num_tiles);
    int32_t prepared = building_house_prepare_for_merge(id, num_tiles);
    if (num_tiles == HOUSE_TILES_2X2) {
        building_house_expand_to_common_residence(id, prepared);
    } else if (num_tiles == HOUSE_TILES_3X3) {
        building_house_expand_to_common_manor(id, prepared);
    } else {
        building_house_expand_to_modest_estate(id, prepared);
    }
    house_evolve_expanded = 1;
    map_tiles_update_all_gardens();
}

static void devolve(const evolve_rule *rule)
{
    int32_t id = house_evolve_current_id;
    if (rule->devolve == HOUSE_ACTION_CHANGE_TO) {
        building_house_change_to(id, rule->devolve_type);
    } else if (rule->split_size == 2) {
        building_house_split_size2(id);
    } else if (rule->split_size == 3) {
        building_house_devolve_from_common_manor(id);
    } else {
        building_house_devolve_from_modest_estate(id);
    }
}

static void evolve_level(int32_t level)
{
    const evolve_rule *rule = &evolve_rules[level];
    int32_t id = house_evolve_current_id;
    if (rule->needs_population && (building_get(id)->house_population <= 0)) {
        return;
    }
    if (rule->merge) {
        building_house_merge(id);
    }
    if (house_evolve_is_frozen()) {
        return;
    }
    house_evolve_status = house_check_desirability(id);
    if (!house_has_required_goods_and_services(id, 0)) {
        house_evolve_status = HOUSE_EVOLVE_DEVOLVE;
        if (rule->devolve == HOUSE_ACTION_NONE) {
            return;
        }
    } else {
        if ((level < HOUSE_LEVEL_MAX) && (house_evolve_status == HOUSE_EVOLVE_UP)) {
            house_evolve_status = house_has_required_goods_and_services(id, house_evolve_status);
        }
        if (rule->devolve == HOUSE_ACTION_NONE) {
            if ((house_evolve_status != HOUSE_EVOLVE_STAY) && (house_evolve_status != HOUSE_EVOLVE_DEVOLVE)) {
                building_house_change_to(id, rule->evolve_type);
            }
            return;
        }
        if (house_evolve_status != HOUSE_EVOLVE_DEVOLVE) {
            building_get(id)->data.house.devolve_delay = 0;
            if (house_evolve_status == HOUSE_EVOLVE_STAY) {
                return;
            }
            if (rule->evolve == HOUSE_ACTION_EXPAND) {
                evolve_expand(rule->expand_tiles);
            } else {
                building_house_change_to(id, rule->evolve_type);
            }
            return;
        }
    }
    building *b = building_get(id);
    if (b->data.house.devolve_delay >= HOUSE_DEVOLVE_DELAY_DAYS) {
        b->data.house.devolve_delay = 0;
        devolve(rule);
        return;
    }
    b->data.house.devolve_delay++;
}

void house_evolve_crude_hut(void)
{
    evolve_level(HOUSE_LEVEL_CRUDE_HUT);
}

void house_evolve_sturdy_hut(void)
{
    evolve_level(HOUSE_LEVEL_STURDY_HUT);
}

void house_evolve_meager_shanty(void)
{
    evolve_level(HOUSE_LEVEL_MEAGER_SHANTY);
}

void house_evolve_common_shanty(void)
{
    evolve_level(HOUSE_LEVEL_COMMON_SHANTY);
}

void house_evolve_rough_cottage(void)
{
    evolve_level(HOUSE_LEVEL_ROUGH_COTTAGE);
}

void house_evolve_ordinary_cottage(void)
{
    evolve_level(HOUSE_LEVEL_ORDINARY_COTTAGE);
}

void house_evolve_modest_homestead(void)
{
    evolve_level(HOUSE_LEVEL_MODEST_HOMESTEAD);
}

void house_evolve_spacious_homestead(void)
{
    evolve_level(HOUSE_LEVEL_SPACIOUS_HOMESTEAD);
}

void house_evolve_modest_apartment(void)
{
    evolve_level(HOUSE_LEVEL_MODEST_APARTMENT);
}

void house_evolve_spacious_apartment(void)
{
    evolve_level(HOUSE_LEVEL_SPACIOUS_APARTMENT);
}

void house_evolve_common_residence(void)
{
    evolve_level(HOUSE_LEVEL_COMMON_RESIDENCE);
}

void house_evolve_spacious_residence(void)
{
    evolve_level(HOUSE_LEVEL_SPACIOUS_RESIDENCE);
}

void house_evolve_elegant_residence(void)
{
    evolve_level(HOUSE_LEVEL_ELEGANT_RESIDENCE);
}

void house_evolve_fancy_residence(void)
{
    evolve_level(HOUSE_LEVEL_FANCY_RESIDENCE);
}

void house_evolve_common_manor(void)
{
    evolve_level(HOUSE_LEVEL_COMMON_MANOR);
}

void house_evolve_spacious_manor(void)
{
    evolve_level(HOUSE_LEVEL_SPACIOUS_MANOR);
}

void house_evolve_elegant_manor(void)
{
    evolve_level(HOUSE_LEVEL_ELEGANT_MANOR);
}

void house_evolve_stately_manor(void)
{
    evolve_level(HOUSE_LEVEL_STATELY_MANOR);
}

void house_evolve_modest_estate(void)
{
    evolve_level(HOUSE_LEVEL_MODEST_ESTATE);
}

void house_evolve_palatial_estate(void)
{
    evolve_level(HOUSE_LEVEL_PALATIAL_ESTATE);
}

static int32_t consumed_amount(int32_t value, const int32_t *by_remainder)
{
    int32_t remainder = value % HOUSE_CONSUMPTION_QUARTERS;
    if (remainder == 0) {
        return value / HOUSE_CONSUMPTION_QUARTERS;
    }
    return by_remainder[(remainder < 0) ? 0 : remainder];
}

static void consume(int16_t *stock, int32_t amount)
{
    if (amount <= 0) {
        return;
    }
    if (amount > *stock) {
        *stock = 0;
    } else {
        *stock = (int16_t) (*stock - amount);
    }
}

void house_consume_goods(int32_t building_id)
{
    building *b = building_get(building_id);
    const house_model *m = &house_models[b->subtype];
    int32_t big = b->size > 1;
    int32_t pottery = big ? (m->pottery << 2) : m->pottery;
    int32_t linen = big ? (m->linen << 2) : m->linen;
    int32_t jewelry = big ? (m->jewelry << 2) : m->jewelry;
    int32_t beer = big ? (m->beer * 4) : m->beer;
    int32_t by_remainder[HOUSE_CONSUMPTION_QUARTERS] = { 0, 0, 0, 0 };
    switch (game_time_day % HOUSE_CONSUMPTION_QUARTERS) {
        case 0:
            by_remainder[3] = 0;
            by_remainder[1] = 1;
            by_remainder[2] = 1;
            break;
        case 1:
        case 3:
            by_remainder[3] = 1;
            by_remainder[1] = 0;
            by_remainder[2] = 0;
            break;
        case 2:
            by_remainder[1] = 0;
            by_remainder[2] = 1;
            by_remainder[3] = 1;
            break;
    }
    int16_t *inventory = b->data.house.inventory;
    consume(&inventory[INVENTORY_BEER], consumed_amount(beer, by_remainder));
    consume(&inventory[INVENTORY_POTTERY], consumed_amount(pottery, by_remainder));
    consume(&inventory[INVENTORY_LINEN], consumed_amount(linen, by_remainder));
    consume(&inventory[INVENTORY_JEWELRY], consumed_amount(jewelry, by_remainder));
}

void city_houses_reset_demands(void)
{
    city_player *player = city_player_current();
    player->houses_evolve_demand_1 = 0;
    player->houses_evolve_demand_2 = 0;
    player->houses_evolve_demand_3 = 0;
    player->houses_evolve_demand_4 = 0;
    player->houses_missing_water_supply = 0;
    player->houses_missing_well = 0;
    player->houses_missing_more_entertainment = 0;
    player->houses_missing_entertainment = 0;
    player->houses_missing_more_education = 0;
    player->houses_missing_education = 0;
    player->houses_missing_religion = 0;
    player->houses_missing_second_religion = 0;
    player->houses_missing_third_religion = 0;
    player->houses_requiring_academy = 0;
    player->houses_missing_dentist = 0;
    player->houses_missing_courthouse = 0;
    player->houses_missing_health = 0;
    player->houses_missing_more_health = 0;
    player->houses_missing_food = 0;
    player->houses_requiring_school = 0;
    player->houses_requiring_library = 0;
    player->houses_requiring_dentist = 0;
    player->houses_requiring_courthouse = 0;
    player->houses_requiring_health = 0;
    player->houses_requiring_religion = 0;
    player->houses_missing_any_courthouse = 0;
    player->houses_requiring_more_courthouse = 0;
}

void building_house_process_evolve_and_consume_goods(void)
{
    city_houses_reset_demands();
    house_evolve_expanded = 0;
    for (house_evolve_current_id = 1; house_evolve_current_id < MAX_BUILDINGS; house_evolve_current_id++) {
        const building *b = building_get(house_evolve_current_id);
        if ((b->state != BUILDING_STATE_IN_USE) || (b->type < BUILDING_HOUSE_CRUDE_HUT) || (b->type > BUILDING_HOUSE_PALATIAL_ESTATE)) {
            continue;
        }
        building_house_check_for_corruption(house_evolve_current_id);
        house_evolve_status = HOUSE_EVOLVE_STAY;
        house_evolve_callbacks[b->type - BUILDING_HOUSE_CRUDE_HUT]();
        if (game_time_day == HOUSE_CONSUMPTION_DAY_FIRST) {
            house_consume_goods(house_evolve_current_id);
        }
        if (game_time_day == HOUSE_CONSUMPTION_DAY_SECOND) {
            house_consume_goods(house_evolve_current_id);
        }
    }
    if (house_evolve_expanded) {
        map_routing_update_land_citizen();
        map_routing_update_land_noncitizen();
    }
}
