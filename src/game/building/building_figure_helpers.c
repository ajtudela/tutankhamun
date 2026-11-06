// Building helpers for the figure generation and the figures: main part and temple lookups, the labor seeker,
// water supply and bazaar images, festival celebrants, bazaar buyer destinations and the granary and storage yard
// functions of the storage carts.
//
// The storage lookups have two oddities the saved games rely on:
// - a bazaar's storage list checks the refused goods in the bazaar's own storage record;
// - for some pairs of fill percentages the storage choice writes nothing and the caller keeps its old values.

#include "building/building_figure_helpers.h"
#include "building/building.h"
#include "building/building_image_group.h"
#include "building/building_industry.h"
#include "building/building_storage.h"
#include "building/building_type.h"
#include "building/house_evolve.h"
#include "city/city_player.h"
#include "city/gods.h"
#include "city/labor.h"
#include "city/resource_type.h"
#include "core/calc.h"
#include "figure/figure.h"
#include "figure/figure_action.h"
#include "figure/figure_core.h"
#include "figure/figure_type.h"
#include "game/game_internal.h"
#include "graphics/image_collection.h"
#include "io/io_data.h"
#include "map/building_tiles.h"
#include "map/farm.h"
#include "map/grid.h"
#include "map/map_data.h"
#include "map/road_access.h"
#include "map/routing_distance.h"
#include "map/routing_land.h"
#include "map/terrain.h"
#include "map/terrain_query.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

int32_t storageyard_wanted_resource;

int32_t market_tiles[108000];

int32_t market_storages[108000];

int32_t market_counts[27];

enum building_helper_limit {
    MAX_PART_LINKS = 525,
    TEMPLE_COMPLEX_BOTH_UPGRADES = 3,
    WATER_SUPPLY_SIZE = 2,
    FANCY_DESIRABILITY = 30,
    WATER_SUPPLY_FANCY_IMAGE = 2,
    FESTIVAL_GROUPS = 4,
    FESTIVAL_LIST_SIZE = 12,
    BAZAAR_DEMAND = 10,
    MARKET_SLOT_SIZE = 4000,
    MARKET_ITEMS = 4,
    BAZAAR_FOOD_LOW = 600,
    BAZAAR_FOOD_VERY_LOW = 50,
    BAZAAR_GOODS_LOW = 100,
    BAZAAR_STOCK_FULL = 20000,
    BAZAAR_STORAGE_RANGE = 40,
    STORAGE_FULL_LOADS = 400,
    STORAGE_STAFF_PERCENT = 50,
};

enum bazaar_slot {
    BAZAAR_SLOT_NONE = -1,
    BAZAAR_SLOT_GRAIN = 0,
    BAZAAR_SLOT_MEAT = 1,
    BAZAAR_SLOT_LETTUCE = 2,
    BAZAAR_SLOT_CHICKPEAS = 3,
    BAZAAR_SLOT_POMEGRANATES = 4,
    BAZAAR_SLOT_FIGS = 5,
    BAZAAR_SLOT_FISH = 6,
    BAZAAR_SLOT_GAMEMEAT = 7,
    BAZAAR_SLOT_BEER = 8,
    BAZAAR_SLOT_POTTERY = 9,
    BAZAAR_SLOT_LINEN = 10,
    BAZAAR_SLOT_JEWELRY = 11,
    BAZAAR_SLOT_GOODS_END = 12,
};

enum bazaar_demand {
    BAZAAR_DEMAND_POTTERY = 0,
    BAZAAR_DEMAND_JEWELRY = 1,
    BAZAAR_DEMAND_LINEN = 2,
    BAZAAR_DEMAND_BEER = 3,
    BAZAAR_DEMANDS = 4,
};

typedef struct bazaar_good {
    int32_t slot;
    int32_t resource;
} bazaar_good;

static const bazaar_good bazaar_goods_by_demand[BAZAAR_DEMANDS] = {
    [BAZAAR_DEMAND_POTTERY] = { BAZAAR_SLOT_POTTERY, RESOURCE_POTTERY },
    [BAZAAR_DEMAND_JEWELRY] = { BAZAAR_SLOT_JEWELRY, RESOURCE_LUXURY_GOODS },
    [BAZAAR_DEMAND_LINEN] = { BAZAAR_SLOT_LINEN, RESOURCE_LINEN },
    [BAZAAR_DEMAND_BEER] = { BAZAAR_SLOT_BEER, RESOURCE_BEER },
};

int32_t building_main(int32_t id)
{
    for (int32_t i = 0; i < MAX_PART_LINKS; i++) {
        int32_t prev = building_get(id)->prev_part_building_id;
        if (prev < 1) {
            return id;
        }
        id = prev;
    }
    return 0;
}

int32_t building_temple_god(int32_t building_id)
{
    switch (building_get(building_id)->type) {
        case BUILDING_TEMPLE_OSIRIS:
        case BUILDING_TEMPLE_COMPLEX_OSIRIS:
            return GOD_OSIRIS;
        case BUILDING_TEMPLE_RA:
        case BUILDING_TEMPLE_COMPLEX_RA:
            return GOD_RA;
        case BUILDING_TEMPLE_PTAH:
        case BUILDING_TEMPLE_COMPLEX_PTAH:
            return GOD_PTAH;
        case BUILDING_TEMPLE_SETH:
        case BUILDING_TEMPLE_COMPLEX_SETH:
            return GOD_SETH;
        case BUILDING_TEMPLE_BAST:
        case BUILDING_TEMPLE_COMPLEX_BAST:
            return GOD_BAST;
        default:
            return -1;
    }
}

bool building_temple_complex_has_upgrade(int32_t building_id, int32_t upgrade)
{
    uint8_t upgrades = building_get(building_id)->data.temple_complex.upgrades;
    return upgrades == TEMPLE_COMPLEX_BOTH_UPGRADES || upgrades == (uint32_t) upgrade;
}

int32_t building_count_figures_of(int32_t building_id, int32_t figure_type)
{
    int32_t count = 0;
    for (int32_t id = 1; id < MAX_FIGURES; id++) {
        figure *f = figure_get(id);
        if (f->state == FIGURE_STATE_ALIVE && f->type == (uint32_t) figure_type && f->building_id == building_id) {
            count++;
        }
    }
    return count;
}

void generate_labor_seeker(int32_t id)
{
    if (building_is_floodplain_farm(id)) {
        return;
    }
    if (city_player_current()->population <= 0) {
        return;
    }
    building *b = building_get(id);
    int32_t figure_id = b->figure_id2;
    if (figure_id != 0) {
        figure *f = figure_get(figure_id);
        if (f->state == FIGURE_STATE_NONE) {
            b->figure_id2 = 0;
        }
        if (f->type != FIGURE_LABOR_SEEKER) {
            b->figure_id2 = 0;
        }
        if (f->building_id != id) {
            b->figure_id2 = 0;
        }
        return;
    }
    int32_t x = map_result_x;
    int32_t y = map_result_y;
    figure_id = figure_create(b->player, FIGURE_LABOR_SEEKER, x, y, 0);
    b->figure_id2 = (int16_t) figure_id;
    figure *f = figure_get(figure_id);
    f->action_state = FIGURE_ACTION_ROAMER_ROAM;
    f->building_id = (int16_t) id;
    figure_movement_init_roaming(figure_id);
    map_result_x = x;
    map_result_y = y;
}

void water_supply_update_image(int32_t id)
{
    building *b = building_get(id);
    if (b->state != BUILDING_STATE_IN_USE) {
        return;
    }
    int32_t offset = b->grid_offset;
    b->has_water_access = map_terrain_any_tile_in_square_with(b->x, b->y, WATER_SUPPLY_SIZE, TERRAIN_GROUNDWATER) ? 1 : 0;
    if (b->num_workers != 0 && b->has_water_access) {
        int32_t fancy = (map_grids.desirability[offset] < FANCY_DESIRABILITY) ? 0 : WATER_SUPPLY_FANCY_IMAGE;
        map_building_tiles_add(id, b->x, b->y, WATER_SUPPLY_SIZE, WATER_SUPPLY_SIZE, fancy + image_group(IMAGE_GROUP_WATER_SUPPLY), TERRAIN_BUILDING);
    }
}

int32_t building_festival_celebrant_role(int32_t type)
{
    switch (type) {
        case BUILDING_BANDSTAND:
        case BUILDING_CONSERVATORY:
            return FIGURE_MUSICIAN;
        case BUILDING_BOOTH:
        case BUILDING_SENET_HOUSE:
        case BUILDING_JUGGLER_SCHOOL:
        case BUILDING_ZOO:
            return FIGURE_JUGGLER;
        case BUILDING_PAVILION:
        case BUILDING_DANCE_SCHOOL:
            return FIGURE_DANCER;
        case BUILDING_SCRIBAL_SCHOOL:
        case BUILDING_LIBRARY:
            return FIGURE_TEACHER;
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
            return FIGURE_PRIEST;
        default:
            return FIGURE_PATRICIAN;
    }
}

int32_t building_festival_celebrant_name_group(int32_t role, int32_t *name_count)
{
    switch (role) {
        case FIGURE_JUGGLER:
            *name_count = 32;
            return 263;
        case FIGURE_MUSICIAN:
            *name_count = 32;
            return 264;
        case FIGURE_DANCER:
            *name_count = 16;
            return 265;
        case FIGURE_TEACHER:
            *name_count = 32;
            return 271;
        default:
            *name_count = 32;
            return 257;
    }
}

void building_festival_spawn_celebrant(int32_t id)
{
    building *b = building_get(id);
    if (!map_has_road_access(b->x, b->y, b->size)) {
        return;
    }
    int32_t figure_id = figure_create(b->player, FIGURE_FESTIVAL_PRIEST, map_result_x, map_result_y, 0);
    b->figure_id = (int16_t) figure_id;
    figure *f = figure_get(figure_id);
    f->action_state = FIGURE_ACTION_FESTIVAL_PRIEST_CREATED;
    f->building_id = (int16_t) id;
    f->wait_ticks = 0;
    int32_t role = building_festival_celebrant_role(b->type);
    f->resource_id = (uint8_t) role;
    int32_t name_count;
    int32_t group;
    if (role == FIGURE_PRIEST) {
        int32_t god = building_temple_god(f->building_id);
        if (god < GOD_OSIRIS || god > GOD_BAST) {
            return;
        }
        name_count = PRIEST_NAME_COUNT;
        group = PRIEST_NAME_GROUP_FIRST + god;
    } else {
        group = building_festival_celebrant_name_group(role, &name_count);
    }
    int32_t *counter = &figure_name_counter_table.by_slot[FIGURE_NAME_GROUP_COUNTER_BASE - group];
    int32_t n = *counter;
    f->name_id = (int16_t) n;
    n++;
    f->name_group = group;
    *counter = n;
    if (n >= name_count) {
        *counter = 0;
    }
}

static int32_t festival_group_active(int32_t group)
{
    int32_t count = 0;
    for (const int32_t *type = &festival_type_lists[group * FESTIVAL_LIST_SIZE]; *type != 0; type++) {
        count += building_count_active[*type];
    }
    return count;
}

void building_festival_spawn_celebrants(int32_t size)
{
    int32_t wanted[FESTIVAL_GROUPS];
    int32_t count[FESTIVAL_GROUPS];
    int32_t last = FESTIVAL_GROUPS - 1;
    for (int32_t g = 0; g < FESTIVAL_GROUPS; g++) {
        if (g == 0) {
            wanted[0] = (festival_group_rates[0] * size) / 100;
        } else {
            wanted[g] += (festival_group_rates[g] * size) / 100;
        }
        count[g] = festival_group_active(g);
        if (g != last) {
            wanted[g + 1] = 0;
        }
        if (wanted[g] < count[g]) {
            count[g] = wanted[g];
        } else if (count[g] < wanted[g] && g != last) {
            wanted[g + 1] = wanted[g] - count[g];
        }
    }
    if (building_highest_id < 0) {
        return;
    }
    for (int32_t id = 0; id <= building_highest_id; id++) {
        building *b = building_get(id);
        if (b->state != BUILDING_STATE_IN_USE) {
            continue;
        }
        building *main = building_get(building_main(id));
        if (main->num_workers == 0 && main->house_size == 0) {
            continue;
        }
        if (main->house_size != 0 && main->house_population == 0) {
            continue;
        }
        int32_t done = 1;
        for (int32_t g = 0; g < FESTIVAL_GROUPS; g++) {
            if (count[g] == 0) {
                continue;
            }
            done = 0;
            for (const int32_t *type = &festival_type_lists[g * FESTIVAL_LIST_SIZE]; *type != 0; type++) {
                if (b->type == *type) {
                    count[g]--;
                    building_festival_spawn_celebrant(id);
                }
                if (count[g] == 0) {
                    break;
                }
            }
        }
        if (done) {
            return;
        }
    }
}

bool building_bazaar_update_image(int32_t id)
{
    building *b = building_get(id);
    if (b->state != BUILDING_STATE_IN_USE) {
        return 0;
    }
    if (map_grids.desirability[b->grid_offset] > FANCY_DESIRABILITY) {
        map_building_tiles_add(id, b->x, b->y, b->size, b->size, image_group(IMAGE_GROUP_BAZAAR_FANCY), TERRAIN_BUILDING);
        return 1;
    }
    map_building_tiles_add(id, b->x, b->y, b->size, b->size, image_group(IMAGE_GROUP_BAZAAR), TERRAIN_BUILDING);
    return 0;
}

void building_bazaar_update_supply(int32_t id, int32_t radius)
{
    building *bazaar = building_get(id);
    int32_t highest = building_highest_id;
    for (int32_t house_id = 1; house_id <= highest; house_id++) {
        building *h = building_get(house_id);
        if (h->state != BUILDING_STATE_IN_USE || !h->house_size || !h->house_population) {
            continue;
        }
        if (calc_maximum_distance(h->x, h->y, bazaar->x, bazaar->y) > radius) {
            continue;
        }
        int32_t level = h->subtype;
        if (level < HOUSE_LEVEL_MAX) {
            level++;
        }
        const house_model *model = &house_models[level];
        if (model->beer) {
            bazaar->data.market.good_demands[BAZAAR_DEMAND_BEER] = BAZAAR_DEMAND;
        }
        if (model->pottery) {
            bazaar->data.market.good_demands[BAZAAR_DEMAND_POTTERY] = BAZAAR_DEMAND;
        }
        if (model->linen) {
            bazaar->data.market.good_demands[BAZAAR_DEMAND_LINEN] = BAZAAR_DEMAND;
        }
        if (model->jewelry) {
            bazaar->data.market.good_demands[BAZAAR_DEMAND_JEWELRY] = BAZAAR_DEMAND;
        }
    }
}

static int32_t bazaar_slot(int32_t resource)
{
    switch (resource) {
        case RESOURCE_GRAIN:
            return BAZAAR_SLOT_GRAIN;
        case RESOURCE_MEAT:
            return BAZAAR_SLOT_MEAT;
        case RESOURCE_LETTUCE:
            return BAZAAR_SLOT_LETTUCE;
        case RESOURCE_CHICKPEAS:
            return BAZAAR_SLOT_CHICKPEAS;
        case RESOURCE_POMEGRANATES:
            return BAZAAR_SLOT_POMEGRANATES;
        case RESOURCE_FIGS:
            return BAZAAR_SLOT_FIGS;
        case RESOURCE_FISH:
            return BAZAAR_SLOT_FISH;
        case RESOURCE_GAMEMEAT:
            return BAZAAR_SLOT_GAMEMEAT;
        case RESOURCE_POTTERY:
            return BAZAAR_SLOT_POTTERY;
        case RESOURCE_BEER:
            return BAZAAR_SLOT_BEER;
        case RESOURCE_LINEN:
            return BAZAAR_SLOT_LINEN;
        case RESOURCE_LUXURY_GOODS:
            return BAZAAR_SLOT_JEWELRY;
        default:
            return BAZAAR_SLOT_NONE;
    }
}

bool market_list_contains(int32_t item, const int32_t *list)
{
    for (int32_t i = 0; i < MARKET_ITEMS; i++) {
        if (list[i] == item) {
            return 1;
        }
    }
    return 0;
}

static void clear_items(int32_t *items)
{
    for (int32_t i = 0; i < MARKET_ITEMS; i++) {
        items[i] = -1;
    }
}

void market_food_items(int32_t building_id, const int32_t *counts, int32_t *items)
{
    building *b = building_get(building_id);
    int32_t n = 0;
    clear_items(items);
    for (int32_t i = 0; i < city_foods.count; i++) {
        if (city_foods.list[i] == RESOURCE_NONE) {
            continue;
        }
        int32_t slot = bazaar_slot(city_foods.list[i]);
        if (b->data.market.inventory[slot] == 0 && counts[slot] != 0) {
            items[n++] = slot;
            if (n >= MARKET_ITEMS) {
                return;
            }
        }
    }
    for (int32_t i = 0; i < city_foods.count; i++) {
        if (city_foods.list[i] == RESOURCE_NONE) {
            continue;
        }
        int32_t slot = bazaar_slot(city_foods.list[i]);
        if (counts[slot] != 0 && b->data.market.inventory[slot] < BAZAAR_FOOD_LOW && !market_list_contains(slot, items)) {
            items[n++] = slot;
            if (n >= MARKET_ITEMS) {
                return;
            }
        }
    }
}

int32_t market_food_destination(int32_t building_id, const int32_t *counts, const int32_t *best)
{
    building *b = building_get(building_id);
    int16_t lowest = BAZAAR_FOOD_VERY_LOW;
    int32_t chosen = BAZAAR_SLOT_NONE;
    for (int32_t i = 0; i < city_foods.count; i++) {
        if (city_foods.list[i] == RESOURCE_NONE) {
            continue;
        }
        int32_t slot = bazaar_slot(city_foods.list[i]);
        if (b->data.market.inventory[slot] == 0 && counts[slot] != 0) {
            return best[slot];
        }
    }
    if (city_foods.count > 0) {
        for (int32_t i = 0; i < city_foods.count; i++) {
            if (city_foods.list[i] == RESOURCE_NONE) {
                continue;
            }
            int32_t slot = bazaar_slot(city_foods.list[i]);
            if (counts[slot] != 0 && b->data.market.inventory[slot] < lowest) {
                lowest = b->data.market.inventory[slot];
                chosen = slot;
            }
        }
        if (chosen == BAZAAR_SLOT_NONE) {
            for (int32_t i = 0; i < city_foods.count; i++) {
                if (city_foods.list[i] == RESOURCE_NONE) {
                    continue;
                }
                int32_t slot = bazaar_slot(city_foods.list[i]);
                if (counts[slot] != 0 && b->data.market.inventory[slot] < BAZAAR_FOOD_LOW) {
                    chosen = slot;
                }
            }
        }
    }
    for (int32_t i = 0; i < city_foods.count; i++) {
        if (city_foods.list[i] == RESOURCE_NONE) {
            continue;
        }
        int32_t slot = bazaar_slot(city_foods.list[i]);
        if (chosen == slot) {
            b->data.market.fetch_inventory_id = (uint8_t) slot;
            return best[slot];
        }
    }
    return 0;
}

void market_goods_items(int32_t building_id, const int32_t *counts, int32_t *items)
{
    building *b = building_get(building_id);
    uint32_t n = 0;
    clear_items(items);
    for (int32_t slot = BAZAAR_SLOT_BEER; slot < BAZAAR_SLOT_GOODS_END; slot++) {
        if (b->data.market.inventory[slot] == 0 && counts[slot] != 0) {
            items[n++] = slot;
            if (slot == BAZAAR_SLOT_JEWELRY && n >= MARKET_ITEMS) {
                return;
            }
        }
    }
    for (int32_t slot = BAZAAR_SLOT_BEER; slot < BAZAAR_SLOT_GOODS_END; slot++) {
        if (counts[slot] != 0 && b->data.market.inventory[slot] < BAZAAR_GOODS_LOW && !market_list_contains(slot, items)) {
            items[n++] = slot;
            if (slot != BAZAAR_SLOT_JEWELRY && n >= MARKET_ITEMS) {
                return;
            }
        }
    }
}

int32_t market_goods_destination(int32_t building_id, const int32_t *counts, const int32_t *best)
{
    building *b = building_get(building_id);
    for (int32_t slot = BAZAAR_SLOT_BEER; slot < BAZAAR_SLOT_GOODS_END; slot++) {
        if (b->data.market.inventory[slot] == 0 && counts[slot] != 0) {
            b->data.market.fetch_inventory_id = (uint8_t) slot;
            return best[slot];
        }
    }
    int32_t chosen = 0;
    int16_t lowest = BAZAAR_GOODS_LOW;
    for (int32_t slot = BAZAAR_SLOT_BEER; slot < BAZAAR_SLOT_GOODS_END; slot++) {
        if (counts[slot] != 0 && b->data.market.inventory[slot] < lowest) {
            chosen = slot;
            lowest = b->data.market.inventory[slot];
        }
    }
    if (chosen == 0) {
        return 0;
    }
    b->data.market.fetch_inventory_id = (uint8_t) chosen;
    return best[chosen];
}

static void market_add(int32_t slot, int32_t storage_id, building *s)
{
    int32_t n = market_counts[slot];
    market_storages[(slot * MARKET_SLOT_SIZE) + n] = storage_id;
    market_tiles[(slot * MARKET_SLOT_SIZE) + n] = map_info.grid_start + s->road_access_x + (s->road_access_y * GRID_SIZE);
    market_counts[slot] = n + 1;
}

static int32_t market_distance(building *a, building *b)
{
    return calc_maximum_distance(a->x, a->y, b->x, b->y);
}

static void clear_market_tables(void)
{
    for (int32_t i = 0; i < MARKET_SLOTS; i++) {
        market_counts[i] = 0;
    }
    for (int32_t i = 0; i < MARKET_SLOTS * MARKET_SLOT_SIZE; i++) {
        market_tiles[i] = 0;
    }
    for (int32_t i = 0; i < MARKET_SLOTS * MARKET_SLOT_SIZE; i++) {
        market_storages[i] = 0;
    }
    for (int32_t i = 0; i < MARKET_SLOTS; i++) {
        city_buildings.market_nearest_storage[i] = 0;
    }
}

static int32_t is_traded_for_bazaars(int32_t trade_status)
{
    return trade_status == 1 || trade_status == 3;
}

static void collect_storages(building *bazaar, int32_t granaries_respect_stockpile)
{
    const building_storage *own = storage_get(bazaar->storage_id);
    const city_player *player = city_player_current();
    for (int32_t k = 1; k <= building_storage_highest_id; k++) {
        int32_t sid = storage_get(k)->building_id;
        if (!storage_get(k)->in_use || sid == 0) {
            continue;
        }
        building *s = building_get(sid);
        if (s->state != BUILDING_STATE_IN_USE) {
            continue;
        }
        if (s->type == BUILDING_GRANARY || s->type == BUILDING_GRANARY_UP) {
            if (scenario_data.kingdom_supplies_grain != 0 || !s->has_road_access || s->distance_from_entry <= 0 || s->road_network_id != bazaar->road_network_id ||
                market_distance(bazaar, s) >= BAZAAR_STORAGE_RANGE) {
                continue;
            }
            for (int32_t resource = RESOURCE_MIN_FOOD; resource <= RESOURCE_GAMEMEAT; resource++) {
                int32_t slot = bazaar_slot(resource);
                if ((!granaries_respect_stockpile || player->stockpiled[resource] == 0) && s->data.granary.resource_stored[resource] != 0 &&
                    own->resource_state[resource] != STORAGE_STATE_REFUSE && bazaar->data.market.inventory[slot] < BAZAAR_STOCK_FULL) {
                    market_add(slot, sid, s);
                }
            }
        } else if (s->type == BUILDING_STORAGE_YARD || s->type == BUILDING_STORAGE_YARD_UP) {
            if (!s->has_road_access || s->distance_from_entry <= 0 || market_distance(bazaar, s) >= BAZAAR_STORAGE_RANGE) {
                continue;
            }
            for (int32_t resource = RESOURCE_MIN_FOOD; resource <= RESOURCE_GAMEMEAT; resource++) {
                int32_t slot = bazaar_slot(resource);
                if (player->stockpiled[resource] == 0 && building_storageyard_get_amount(sid, resource) > 0 &&
                    own->resource_state[resource] != STORAGE_STATE_REFUSE && bazaar->data.market.inventory[slot] < BAZAAR_STOCK_FULL &&
                    is_traded_for_bazaars(player->trade_status[resource])) {
                    market_add(slot, sid, s);
                }
            }
            for (int32_t g = 0; g < BAZAAR_DEMANDS; g++) {
                int32_t slot = bazaar_goods_by_demand[g].slot;
                int32_t resource = bazaar_goods_by_demand[g].resource;
                if (player->stockpiled[resource] == 0 && building_storageyard_get_amount(sid, resource) > 0 &&
                    own->resource_state[resource] != STORAGE_STATE_REFUSE && bazaar->data.market.inventory[slot] < BAZAAR_STOCK_FULL) {
                    market_add(slot, sid, s);
                }
            }
        }
    }
}

int32_t building_market_get_storage_destination(int32_t id, int32_t second_flag, int32_t *items, uint8_t *routed)
{
    char second = (char) second_flag;
    building *bazaar = building_get(id);
    clear_market_tables();
    collect_storages(bazaar, 1);
    for (int32_t g = 0; g < BAZAAR_DEMANDS; g++) {
        if (bazaar->data.market.good_demands[g] <= 0) {
            market_counts[bazaar_goods_by_demand[g].slot] = 0;
        } else {
            bazaar->data.market.good_demands[g]--;
        }
    }
    if (*routed == 0) {
        for (int32_t i = 0; i < MARKET_SLOTS; i++) {
            if (market_counts[i] != 0) {
                map_routing_calculate_distances(bazaar->road_access_x, bazaar->road_access_y);
                *routed = 1;
                break;
            }
        }
    }
    for (int32_t slot = 0; slot < MARKET_SLOTS; slot++) {
        int32_t count = market_counts[slot];
        if (count == 0) {
            continue;
        }
        int32_t nearest = 0;
        int32_t min_distance = 9999;
        for (int32_t i = 0; i < count; i++) {
            int32_t distance = routing_grids.distance[market_tiles[(slot * MARKET_SLOT_SIZE) + i]];
            if (distance > 0 && distance < min_distance) {
                min_distance = distance;
                nearest = i + 1;
            }
        }
        if (nearest) {
            city_buildings.market_nearest_storage[slot] = building_main(market_storages[(slot * MARKET_SLOT_SIZE) + nearest - 1]);
        } else {
            market_counts[slot] = 0;
        }
    }
    if (!second) {
        market_food_items(id, market_counts, items);
        return market_food_destination(id, market_counts, city_buildings.market_nearest_storage);
    }
    market_goods_items(id, market_counts, items);
    return market_goods_destination(id, market_counts, city_buildings.market_nearest_storage);
}

void building_market_update_buyer_storages(int32_t figure_id, int32_t id)
{
    building *bazaar = building_get(id);
    clear_market_tables();
    collect_storages(bazaar, 0);
    for (int32_t slot = 0; slot < MARKET_SLOTS; slot++) {
        int32_t count = market_counts[slot];
        if (count == 0) {
            continue;
        }
        int32_t nearest =
            map_routing_citizen_find_first_of_road_garden(bazaar->road_access_x, bazaar->road_access_y, &market_tiles[slot * MARKET_SLOT_SIZE], count);
        if (nearest) {
            city_buildings.market_nearest_storage[slot] = building_main(market_storages[(slot * MARKET_SLOT_SIZE) + nearest - 1]);
        }
    }
    building *home = building_get(figure_get(figure_id)->building_id);
    for (int32_t g = 0; g < BAZAAR_DEMANDS; g++) {
        if (home->data.market.good_demands[g] <= 0) {
            market_counts[bazaar_goods_by_demand[g].slot] = 0;
            city_buildings.market_nearest_storage[bazaar_goods_by_demand[g].slot] = 0;
        } else {
            home->data.market.good_demands[g]--;
        }
    }
}

static int32_t is_bazaar(int32_t type)
{
    return type == BUILDING_BAZAAR || type == BUILDING_BAZAAR_UP;
}

int32_t building_market_get_max_food_stock(int32_t id)
{
    int32_t max = 0;
    if (id < 1) {
        return max;
    }
    building *b = building_get(id);
    if (!is_bazaar(b->type)) {
        return 0;
    }
    for (int32_t i = 0; i < city_foods.count; i++) {
        if (city_foods.list[i] != RESOURCE_NONE) {
            int16_t stock = b->data.market.inventory[bazaar_slot(city_foods.list[i])];
            if (max < stock) {
                max = stock;
            }
        }
    }
    return max;
}

int32_t building_market_get_max_goods_stock(int32_t id)
{
    if (id < 1) {
        return 0;
    }
    building *b = building_get(id);
    if (!is_bazaar(b->type)) {
        return 0;
    }
    int32_t max = 0;
    for (int32_t slot = BAZAAR_SLOT_BEER; slot < BAZAAR_SLOT_GOODS_END; slot++) {
        if (max < b->data.market.inventory[slot]) {
            max = b->data.market.inventory[slot];
        }
    }
    return max;
}

int32_t storage_count_wanted(int32_t resource1, int32_t resource2)
{
    return (resource1 != 0) + (resource2 != 0);
}

static void set_wanted(uint8_t *second_cart, uint8_t *first, int32_t *ok, uint8_t second_cart_value, uint8_t first_value, int32_t ok_value)
{
    *ok = ok_value;
    *first = first_value;
    *second_cart = second_cart_value;
}

void storage_choose_wanted(int32_t percent1, int32_t percent2, int32_t count, uint8_t *second_cart, uint8_t *first, int32_t *ok)
{
    if (count == 0) {
        set_wanted(second_cart, first, ok, 0, 0, 0);
        return;
    }
    if (count == 1) {
        *first = 1;
        if (percent1 < 50) {
            *second_cart = 1;
            *ok = 2;
        } else if (percent1 < 100) {
            *second_cart = 0;
            *ok = 1;
        } else {
            *second_cart = 0;
            *ok = 0;
        }
        return;
    }
    if (percent1 > 100 && percent2 > 100) {
        set_wanted(second_cart, first, ok, 0, 0, 0);
    }
    if (percent1 < 50 && percent2 > 100) {
        set_wanted(second_cart, first, ok, 1, 1, 2);
    }
    if (percent1 < 100) {
        if (percent2 > 100) {
            set_wanted(second_cart, first, ok, 0, 1, 1);
        }
        if (percent2 < 100) {
            set_wanted(second_cart, first, ok, 1, 0, 2);
        }
    }
}

static int32_t is_half_staffed(const building *b)
{
    return calc_percentage(b->num_workers, building_model_table[b->type].laborers) >= STORAGE_STAFF_PERCENT;
}

int32_t building_granary_resource_to_send(int32_t building_id, int32_t wanted)
{
    building *b = building_get(building_id);
    const building_storage *s = storage_get(b->storage_id);
    if (!is_half_staffed(b)) {
        return -1;
    }
    if (wanted != 0) {
        for (int32_t resource = RESOURCE_MIN_FOOD; resource < RESOURCE_MAX_FOOD; resource++) {
            if (s->resource_state[resource] == STORAGE_STATE_GET) {
                return 0;
            }
        }
    }
    for (int32_t resource = RESOURCE_MIN_FOOD; resource < RESOURCE_MAX_FOOD; resource++) {
        if (s->resource_state[resource] == STORAGE_STATE_EMPTY && b->data.granary.resource_stored[resource] != 0) {
            return resource;
        }
    }
    return -1;
}

int32_t building_storageyard_get_amount(int32_t id, int32_t resource)
{
    int32_t amount = 0;
    for (int32_t i = 0; i < STORAGE_YARD_ROOMS; i++) {
        id = building_get(id)->next_part_building_id;
        if (id < 1) {
            return 0;
        }
        building *room = building_get(id);
        if (room->subtype != RESOURCE_NONE && room->subtype == resource) {
            amount += room->loads_stored;
        }
    }
    return amount;
}

int32_t calc_distance_with_penalty(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t distance1, int32_t distance2)
{
    int32_t penalty = (distance2 < distance1) ? (distance1 - distance2) : (distance2 - distance1);
    if (distance1 == -1) {
        penalty = 0;
    }
    return calc_maximum_distance(x1, y1, x2, y2) + penalty;
}

static int32_t gets_from_any_network(const building *b, int32_t resource)
{
    switch (b->type) {
        case BUILDING_GRANARY:
        case BUILDING_STORAGE_YARD:
            return storage_get(b->storage_id)->resource_state[resource] == STORAGE_STATE_GET;
        case BUILDING_PYRAMID:
        case BUILDING_SUN_TEMPLE:
        case BUILDING_MAUSOLEUM:
        case BUILDING_ALEXANDRIA_LIBRARY:
        case BUILDING_CAESAREUM:
        case BUILDING_LIGHTHOUSE:
        case BUILDING_SMALL_ROYAL_TOMB:
        case BUILDING_ABU_SIMBEL:
        case BUILDING_MEDIUM_ROYAL_TOMB:
        case BUILDING_LARGE_ROYAL_TOMB:
        case BUILDING_GRAND_ROYAL_TOMB:
            return 1;
        default:
            return 0;
    }
}

static int32_t storage_yard_amount_in_rooms(int32_t yard_id, int32_t resource)
{
    int32_t amount = 0;
    int32_t room_id = yard_id;
    for (int32_t i = 0; i < STORAGE_YARD_ROOMS; i++) {
        room_id = building_get(room_id)->next_part_building_id;
        if (room_id <= 0) {
            continue;
        }
        const building *room = building_get(room_id);
        if (room->loads_stored > 0 && room->subtype == resource) {
            amount += room->loads_stored;
        }
    }
    return amount;
}

int32_t building_storageyard_for_getting(int32_t id, int32_t resource, int32_t full_only_flag)
{
    char full_only = (char) full_only_flag;
    int32_t best_distance = 10000;
    int32_t best_id = 0;
    int32_t is_food = resource >= RESOURCE_MIN_FOOD && resource <= RESOURCE_GAMEMEAT;
    if (city_player_current()->stockpiled[resource] != 0) {
        return 0;
    }
    building *b = building_get(id);
    int32_t same_network = !gets_from_any_network(b, resource);
    if (building_storage_highest_id <= 0) {
        return 0;
    }
    for (int32_t storage = 1; storage <= building_storage_highest_id; storage++) {
        const building_storage *s = storage_get(storage);
        int32_t other_id = s->building_id;
        if (!s->in_use || other_id == 0) {
            continue;
        }
        building *other = building_get(other_id);
        if (other->state != BUILDING_STATE_IN_USE || other_id == id) {
            continue;
        }
        if (same_network && other->road_network_id != b->road_network_id) {
            continue;
        }
        uint8_t state = s->resource_state[resource];
        if (other->type == BUILDING_STORAGE_YARD) {
            int32_t amount = storage_yard_amount_in_rooms(other_id, resource);
            if ((amount >= STORAGE_FULL_LOADS || full_only == 0) && amount > 0 && (state != STORAGE_STATE_GET || full_only != 0)) {
                int32_t distance = calc_distance_with_penalty(other->x, other->y, b->x, b->y, b->distance_from_entry, other->distance_from_entry);
                if (distance < best_distance) {
                    best_distance = distance;
                    best_id = other_id;
                }
            }
        } else if (is_food && other->type == BUILDING_GRANARY && other->data.granary.resource_stored[resource] >= STORAGE_FULL_LOADS &&
            state != STORAGE_STATE_GET) {
            int32_t distance = calc_distance_with_penalty(other->x, other->y, b->x, b->y, b->distance_from_entry, other->distance_from_entry);
            if (distance < best_distance) {
                best_distance = distance;
                best_id = other_id;
            }
        }
    }
    if (best_id > 0) {
        building *best = building_get(best_id);
        map_result_y = best->road_access_y;
        map_result_x = best->road_access_x;
        return best_id;
    }
    return 0;
}

void building_storage_wanted_resources(
    int32_t id, int32_t *resource1, int32_t *resource2, int32_t *percent1, int32_t *percent2, int32_t *destination1, int32_t *destination2)
{
    int32_t percentages[STORAGE_RESOURCES];
    int32_t saved_x = map_result_x;
    int32_t saved_y = map_result_y;
    int32_t percent = 100;
    building *b = building_get(id);
    const building_storage *s = storage_get(b->storage_id);
    int32_t is_granary = b->type == BUILDING_GRANARY || b->type == BUILDING_GRANARY_UP;
    int32_t count = is_granary ? city_foods.count : city_resources.count;
    int32_t any_get = 0;
    if (count <= 0) {
        return;
    }
    for (int32_t i = 0; i < count; i++) {
        int32_t resource = is_granary ? city_foods.list[i] : city_resources.list[i];
        percentages[resource] = 100;
        if (s->resource_state[resource] != STORAGE_STATE_GET) {
            continue;
        }
        any_get = 1;
        int32_t target = s->get_amount[resource];
        if (b->type == BUILDING_STORAGE_YARD) {
            if (building_storageyard_get_amount(id, resource) >= target) {
                percent = 100;
            } else if (resource > RESOURCE_NONE) {
                percent = (building_storageyard_get_amount(id, resource) * 100) / target;
            }
        } else {
            int16_t stock = b->data.granary.resource_stored[resource];
            if (stock >= (int16_t) target) {
                percent = 100;
            } else if (resource > RESOURCE_NONE) {
                percent = (stock * 100) / target;
            }
        }
        percentages[resource] = percent;
    }
    if (!any_get) {
        return;
    }
    for (int32_t i = 0; i < count; i++) {
        int32_t resource = is_granary ? city_foods.list[i] : city_resources.list[i];
        int32_t destination = building_storageyard_for_getting(id, resource, 0);
        if (city_player_current()->stockpiled[resource] != 0 || destination == 0) {
            continue;
        }
        int32_t p = percentages[resource];
        if (p < *percent1) {
            *percent2 = *percent1;
            *resource2 = *resource1;
            *destination2 = *destination1;
            *percent1 = p;
            *resource1 = resource;
            *destination1 = destination;
        } else if (p < *percent2) {
            *percent2 = p;
            *resource2 = resource;
            *destination2 = destination;
        }
    }
    map_result_x = saved_x;
    map_result_y = saved_y;
}

int32_t storage_image_level(int32_t amount)
{
    if (amount == 400) {
        return 3;
    }
    if (amount < 400) {
        if (amount > 299) {
            return 2;
        }
    } else if (amount > 299) {
        return 0;
    }
    if (amount < 200) {
        return 0;
    }
    return 1;
}

int32_t storage_room_image(int32_t resource, int32_t loads)
{
    int32_t base = (resource < RESOURCE_OIL) ? (image_group(IMAGE_GROUP_STORAGE_YARD_GOODS) - 4 + (resource * STORAGE_ROOM_IMAGES_PER_RESOURCE))
                                             : (image_group(IMAGE_GROUP_STORAGE_YARD_GOODS_LATE) - 124 + (resource * STORAGE_ROOM_IMAGES_PER_RESOURCE));
    return base + storage_image_level(loads);
}

int32_t building_granary_remove_resource(int32_t player, int32_t building_id, int32_t resource, int32_t amount)
{
    building *b = building_get(building_id);
    if (amount < 1) {
        return 0;
    }
    city_player *city = city_player_get(player);
    int16_t stock = b->data.granary.resource_stored[resource];
    if (stock <= amount) {
        int32_t stored = city->granary_food_stored[resource];
        b->data.granary.resource_stored[RESOURCE_NONE] += stock;
        b->data.granary.resource_stored[resource] = 0;
        city->granary_food_stored[resource] = stored - stock;
        return amount - stock;
    }
    b->data.granary.resource_stored[resource] = (int16_t) (stock - amount);
    city->granary_food_stored[resource] -= amount;
    b->data.granary.resource_stored[RESOURCE_NONE] += (int16_t) amount;
    return 0;
}

static int32_t room_with_least(int32_t yard_id, int32_t resource)
{
    int16_t least = STORAGE_FULL_LOADS + 1;
    int32_t room_id = 0;
    int32_t part = yard_id;
    for (int32_t i = 0; i < STORAGE_YARD_ROOMS; i++) {
        part = building_get(part)->next_part_building_id;
        if (part > 0 && building_get(part)->subtype == resource) {
            int16_t loads = building_get(part)->loads_stored;
            if (loads > 0 && loads < least) {
                room_id = part;
                least = loads;
            }
        }
    }
    return room_id;
}

int32_t building_storageyard_remove_resource(int32_t player, int32_t id, int32_t resource, int32_t amount)
{
    if (amount < 1) {
        return 0;
    }
    int32_t type = building_get(id)->type;
    if (type != BUILDING_STORAGE_YARD && type != BUILDING_STORAGE_YARD_UP) {
        return amount;
    }
    city_player *city = city_player_get(player);
    int32_t again;
    do {
        int32_t room_id = room_with_least(id, resource);
        if (room_id == 0) {
            return amount;
        }
        building *room = building_get(room_id);
        again = 0;
        int32_t loads = room->loads_stored;
        if (amount < loads) {
            room->loads_stored -= (int16_t) amount;
            city->storage_space[resource] += amount;
            city->storage_stored[resource] -= amount;
            amount = 0;
        } else {
            amount -= loads;
            room->loads_stored = 0;
            room->subtype = RESOURCE_NONE;
            city->storage_space[resource] += loads;
            city->storage_stored[resource] -= loads;
            if (amount > 0) {
                again = 1;
            }
        }
        int16_t left = room->loads_stored;
        map_grids.images[room->grid_offset] = (left > 0) ? storage_room_image(resource, left) : image_group(IMAGE_GROUP_STORAGE_YARD_SPACE);
    } while (again);
    return amount;
}

static int32_t room_with(int32_t id, int32_t resource)
{
    const city_player *player = city_player_current();
    int32_t room_id = id;
    for (int32_t i = 0; i < STORAGE_YARD_ROOMS; i++) {
        room_id = building_get(room_id)->next_part_building_id;
        if (room_id <= 0) {
            continue;
        }
        const building *room = building_get(room_id);
        if (room->loads_stored > 0) {
            int16_t stored = room->subtype;
            if (player->stockpiled[stored] == 0 && stored == resource) {
                return resource;
            }
        }
    }
    return 0;
}

static int32_t recruiter_has_room(int16_t stock, int32_t ordered, int32_t max)
{
    return stock + (ordered * STORAGE_CART_LOAD) < max;
}

static int32_t resource_for_recruiter(int32_t id, const building *b)
{
    if (building_count_active[BUILDING_RECRUITER] <= 0 || formation_num_legions == 0) {
        return 0;
    }
    building *recruiter = building_get(city_player_current()->recruiter_building_id);
    if (b->road_network_id != recruiter->road_network_id) {
        return 0;
    }
    int16_t weapons = recruiter->loads_stored;
    int16_t chariots = recruiter->second_input_stock;
    int32_t max = building_input_stock_max(recruiter->type);
    int32_t weapons_ordered = recruiter->incoming_carts;
    int32_t chariots_ordered = recruiter->incoming_loads;
    if (chariots < weapons) {
        if (recruiter_has_room(chariots, chariots_ordered, max) && room_with(id, RESOURCE_CHARIOTS)) {
            return RESOURCE_CHARIOTS;
        }
        if (recruiter_has_room(weapons, weapons_ordered, max) && room_with(id, RESOURCE_WEAPONS)) {
            return RESOURCE_WEAPONS;
        }
    } else {
        if (recruiter_has_room(weapons, weapons_ordered, max) && room_with(id, RESOURCE_WEAPONS)) {
            return RESOURCE_WEAPONS;
        }
        if (recruiter_has_room(chariots, chariots_ordered, max) && room_with(id, RESOURCE_CHARIOTS)) {
            return RESOURCE_CHARIOTS;
        }
    }
    return 0;
}

static int32_t input_has_room(int16_t stock, int32_t max)
{
    return stock < max && stock + STORAGE_CART_LOAD <= max;
}

static int32_t resource_for_industry(int32_t id, const building *other, uint8_t network)
{
    const city_player *player = city_player_current();
    int32_t room_id = id;
    for (int32_t i = 0; i < STORAGE_YARD_ROOMS; i++) {
        room_id = building_get(room_id)->next_part_building_id;
        if (room_id <= 0 || building_get(room_id)->loads_stored <= 0) {
            continue;
        }
        int32_t resource = building_get(room_id)->subtype;
        if (player->stockpiled[resource] != 0 || other->road_network_id != network) {
            continue;
        }
        int32_t index = building_input_index(resource);
        if (index < 0 || player->workshop_space[index] <= 0) {
            continue;
        }
        int32_t max = building_input_stock_max(other->type);
        if (other->subtype == index) {
            if (input_has_room(other->loads_stored, max)) {
                return resource;
            }
        } else if (other->second_input_resource != INDUSTRY_INPUT_NONE && other->second_input_resource == index) {
            if (input_has_room(other->second_input_stock, max)) {
                return resource;
            }
        }
    }
    return 0;
}

int32_t building_storageyard_resource_to_send(int32_t id, int32_t wanted)
{
    building *b = building_get(id);
    const building_storage *s = storage_get(b->storage_id);
    uint8_t network = map_grids.road_network[map_grid_offset(b->road_access_x, b->road_access_y)];
    if (!is_half_staffed(b)) {
        return -1;
    }
    if (wanted != 0) {
        for (int32_t resource = RESOURCE_MIN; resource < RESOURCE_MAX; resource++) {
            if (s->resource_state[resource] == STORAGE_STATE_GET && city_player_current()->stockpiled[resource] == 0 &&
                building_storageyard_get_amount(id, resource) <= s->get_amount[resource]) {
                storageyard_wanted_resource = resource;
                return 0;
            }
        }
    }
    int32_t for_recruiter = resource_for_recruiter(id, b);
    if (for_recruiter) {
        return for_recruiter;
    }
    for (int32_t other_id = 1; other_id <= building_highest_id; other_id++) {
        building *other = building_get(other_id);
        if (!other->state || other->num_workers == 0 || !building_takes_input(other->type) || city_labor_industry_is_mothballed(other_id)) {
            continue;
        }
        int32_t resource = resource_for_industry(id, other, network);
        if (resource) {
            return resource;
        }
    }
    int32_t room_id = id;
    for (int32_t i = 0; i < STORAGE_YARD_ROOMS; i++) {
        room_id = building_get(room_id)->next_part_building_id;
        if (room_id < 1) {
            continue;
        }
        const building *room = building_get(room_id);
        if (room->loads_stored >= 1 && s->resource_state[room->subtype] == STORAGE_STATE_EMPTY) {
            return room->subtype;
        }
    }
    return -1;
}
