// Service coverage: what a walker gives the houses and buildings within its radius on every tile it enters. Each
// service has its provider: house access to schools, temples, entertainment and health, tax access, the risk
// reductions of the engineer, fireman and crime walkers, the cures, the missionary, and the market trader's food and
// goods. The radii, access values and amounts are service_rules from config/service_rules.json.
//
// The coverage values are not always what the names say:
// - the crime base is read for every building with its subtype as the house level, past the house models too;
// - the Bast complex's upgrade value 10 matches only a complex with both upgrades;
// - the apothecary's health provider covers nothing, so its seen value only fades;
// - the access, tax access and market demand values wrap in their byte and short fields.

#include "figure/service.h"
#include "building/building.h"
#include "building/building_data.h"
#include "building/building_figure_helpers.h"
#include "building/building_type.h"
#include "building/construction_place.h"
#include "building/house_evolve.h"
#include "building/temple_complex.h"
#include "city/city_player.h"
#include "city/gods.h"
#include "city/population_extra.h"
#include "city/resource_type.h"
#include "figure/figure.h"
#include "figure/figure_action.h"
#include "figure/figure_data.h"
#include "figure/figure_type.h"
#include "graphics/graphics_data.h"
#include "map/grid.h"
#include "map/map_data.h"
#include "map/service_visit.h"
#include "map/terrain.h"
#include "widget/widget_data.h"
#include "window/window_data.h"
#include <stddef.h>
#include <stdint.h>

int32_t house_models_end[23];

int32_t service_crime_seen;

int32_t service_max_damage_seen;

int32_t service_max_fire_seen;

int32_t service_max_tax_multiplier;

int32_t service_rules[SERVICE_RULE_COUNT] = {
    [SERVICE_RULE_RADIUS] = 2,
    [SERVICE_RULE_MISSIONARY_RADIUS] = 4,
    [SERVICE_RULE_HOUSE_ACCESS] = 96,
    [SERVICE_RULE_HOUSE_TAX_ACCESS] = 50,
    [SERVICE_RULE_RISK_SEEN_START] = 1000,
    [SERVICE_RULE_CRIME_RISK_MAX] = 1000,
    [SERVICE_RULE_HOUSES_COVERED_MAX] = 300,
    [SERVICE_RULE_SEEN_FADE] = 10,
    [SERVICE_RULE_MARKET_DEMAND] = 10,
    [SERVICE_RULE_FOOD_PER_PERSON] = 2,
    [SERVICE_RULE_FOOD_STOCK_PER_PERSON] = 6,
    [SERVICE_RULE_LARGE_HOUSE_GOODS_FACTOR] = 4,
    [SERVICE_RULE_GOODS_STOCK_FACTOR] = 2,
};

static void *building_field(building *b, size_t field_offset)
{
    return (uint8_t *) b + field_offset;
}

static int32_t house_model_crime_base(int32_t level)
{
    int32_t models = (int32_t) (sizeof(house_models) / sizeof(house_model));
    if (level < 0) {
        return 0;
    }
    if (level < models) {
        return house_models[level].crime_base;
    }
    int32_t word = ((level * (int32_t) sizeof(house_model)) + (int32_t) offsetof(house_model, crime_base) - (int32_t) sizeof(house_models)) /
        (int32_t) sizeof(int32_t);
    int32_t end_words = (int32_t) (sizeof(house_models_end) / sizeof(int32_t));
    if (word < end_words) {
        return house_models_end[word];
    }
    word -= end_words;
    int32_t palette_words = (int32_t) (sizeof(system_palette) / sizeof(int32_t));
    if (word < palette_words) {
        return (int32_t) system_palette[word];
    }
    word -= palette_words;
    if (word < (int32_t) (sizeof(city_view) / sizeof(int32_t))) {
        return ((const int32_t *) &city_view)[word];
    }
    return 0;
}

typedef struct {
    int32_t x_min, y_min, x_max, y_max;
} area;

static int32_t area_begin(area *a, int32_t x, int32_t y, int32_t radius)
{
    a->x_max = x + radius;
    a->y_max = y + radius;
    a->x_min = x - radius;
    a->y_min = y - radius;
    if (a->x_min < 0) {
        a->x_min = 0;
    }
    if (a->y_min < 0) {
        a->y_min = 0;
    }
    if (a->x_max >= map_info.grid_width) {
        a->x_max = map_info.grid_width - 1;
    }
    if (a->y_max >= map_info.grid_height) {
        a->y_max = map_info.grid_height - 1;
    }
    map_result_y = a->y_min;
    map_tile_cursor_offset = map_info.grid_start + (a->y_min * GRID_SIZE) + a->x_min;
    return a->y_min <= a->y_max;
}

typedef int32_t (*tile_fn)(int32_t grid_offset, void *context);

static int32_t area_walk(const area *a, tile_fn fn, void *context)
{
    int32_t count = 0;
    for (map_result_y = a->y_min; map_result_y <= a->y_max; map_result_y++) {
        for (map_result_x = a->x_min; map_result_x <= a->x_max; map_result_x++) {
            count += fn(map_tile_cursor_offset, context);
            map_tile_cursor_offset++;
        }
        map_tile_cursor_offset += a->x_min - a->x_max + (GRID_SIZE - 1);
    }
    return count;
}

static int32_t area_run(int32_t x, int32_t y, int32_t radius, tile_fn fn, void *context)
{
    area a;
    if (!area_begin(&a, x, y, radius)) {
        return 0;
    }
    return area_walk(&a, fn, context);
}

static int32_t is_inhabited_house(int32_t building_id)
{
    if (building_id <= 0) {
        return 0;
    }
    const building *b = building_get(building_id);
    return b->house_size && b->house_population > 0;
}

static int32_t house_access_tile(int32_t grid_offset, void *context)
{
    int32_t building_id = map_grids.building_ids[grid_offset];
    if (!is_inhabited_house(building_id)) {
        return 0;
    }
    uint8_t *access = building_field(building_get(building_id), (size_t) context);
    *access = (uint8_t) service_rules[SERVICE_RULE_HOUSE_ACCESS];
    return 1;
}

static int32_t provide_house_access(int32_t x, int32_t y, int32_t offset)
{
    return area_run(x, y, service_rules[SERVICE_RULE_RADIUS], house_access_tile, (void *) (intptr_t) offset);
}

static int32_t count_house_tile(int32_t grid_offset, void *context)
{
    (void) context;
    return is_inhabited_house(map_grids.building_ids[grid_offset]);
}

int32_t service_provide_labor_seeker_coverage(int32_t x, int32_t y)
{
    return area_run(x, y, service_rules[SERVICE_RULE_RADIUS], count_house_tile, 0);
}

static int32_t tax_tile(int32_t grid_offset, void *context)
{
    (void) context;
    int32_t building_id = map_grids.building_ids[grid_offset];
    if (!is_inhabited_house(building_id)) {
        return 0;
    }
    building *b = building_get(building_id);
    int32_t multiplier = house_models[b->subtype].tax_multiplier;
    if (multiplier > service_max_tax_multiplier) {
        service_max_tax_multiplier = multiplier;
    }
    b->house_tax_coverage = (uint8_t) service_rules[SERVICE_RULE_HOUSE_TAX_ACCESS];
    return 1;
}

int32_t service_provide_tax_collector_coverage(int32_t x, int32_t y)
{
    service_max_tax_multiplier = 0;
    return area_run(x, y, service_rules[SERVICE_RULE_RADIUS], tax_tile, 0);
}

int32_t service_provide_water_coverage(int32_t x, int32_t y)
{
    if (map_sweep_countdown > 0) {
        return 0;
    }
    return provide_house_access(x, y, offsetof(building, data.house.water_supply));
}

static int32_t religion_tile(int32_t grid_offset, void *context)
{
    int32_t building_id = map_grids.building_ids[grid_offset];
    if (!is_inhabited_house(building_id)) {
        return 0;
    }
    int32_t god = (int32_t) (intptr_t) context;
    if (god >= GOD_OSIRIS && god <= GOD_BAST) {
        building_get(building_id)->data.house.temple[god] = (uint8_t) service_rules[SERVICE_RULE_HOUSE_ACCESS];
    }
    return 1;
}

int32_t service_provide_religion_coverage(int32_t x, int32_t y, int32_t god)
{
    return area_run(x, y, service_rules[SERVICE_RULE_RADIUS], religion_tile, (void *) (intptr_t) god);
}

static int32_t engineer_tile(int32_t grid_offset, void *context)
{
    (void) context;
    int32_t building_id = map_grids.building_ids[grid_offset];
    if (building_id < 1) {
        if (map_grids.terrain[grid_offset] & TERRAIN_WALL) {
            int32_t damage = (int16_t) map_grids.building_damage[grid_offset];
            if (damage > service_max_damage_seen) {
                service_max_damage_seen = damage;
            }
            int16_t left = (int16_t) (damage - building_model_get(BUILDING_ARCHITECT_POST)->service_reduction);
            map_grids.building_damage[grid_offset] = (left < 0) ? 0 : (uint16_t) left;
        }
        return 0;
    }
    building *b = building_get(building_main(building_id));
    int32_t damage = b->damage_risk;
    if (damage > service_max_damage_seen) {
        service_max_damage_seen = damage;
    }
    int16_t left = (int16_t) (damage - building_model_get(BUILDING_ARCHITECT_POST)->service_reduction);
    b->damage_risk = (left < 0) ? 0 : left;
    return b->house_size && b->house_population > 0;
}

int32_t service_provide_engineer_coverage(int32_t x, int32_t y)
{
    service_max_damage_seen = service_rules[SERVICE_RULE_RISK_SEEN_START];
    return area_run(x, y, service_rules[SERVICE_RULE_RADIUS], engineer_tile, 0);
}

static int32_t fireman_tile(int32_t grid_offset, void *context)
{
    (void) context;
    int32_t building_id = map_grids.building_ids[grid_offset];
    if (!building_id) {
        return 0;
    }
    int32_t main_id = building_main(building_id);
    building *b = building_get(main_id);
    int32_t risk = b->fire_risk;
    if (risk > service_max_fire_seen) {
        service_max_fire_seen = risk;
    }
    int16_t left = (int16_t) (risk - building_model_get(BUILDING_FIREHOUSE)->service_reduction);
    b->fire_risk = (left < 0) ? 0 : left;
    return main_id > 0 && b->house_size && b->house_population > 0;
}

int32_t service_provide_fireman_coverage(int32_t x, int32_t y)
{
    service_max_fire_seen = service_rules[SERVICE_RULE_RISK_SEEN_START];
    return area_run(x, y, service_rules[SERVICE_RULE_RADIUS], fireman_tile, 0);
}

static int32_t crime_tile(int32_t grid_offset, void *context)
{
    int32_t building_id = map_grids.building_ids[grid_offset];
    if (!building_id) {
        return 0;
    }
    building *b = building_get(building_id);
    int32_t crime = b->crime_risk;
    if (crime > service_crime_seen) {
        service_crime_seen = crime;
    }
    crime -= (int32_t) (intptr_t) context;
    b->crime_risk = crime;
    int32_t crime_base = house_model_crime_base(b->subtype);
    if (crime < crime_base) {
        b->crime_risk = crime_base;
    }
    if (b->crime_risk > service_rules[SERVICE_RULE_CRIME_RISK_MAX]) {
        b->crime_risk = service_rules[SERVICE_RULE_CRIME_RISK_MAX];
    }
    return 0;
}

void service_provide_crime_coverage(int32_t x, int32_t y, int32_t reduction)
{
    service_crime_seen = 0;
    area_run(x, y, service_rules[SERVICE_RULE_RADIUS], crime_tile, (void *) (intptr_t) reduction);
}

static int32_t is_house_type(const building *b)
{
    return b->type >= BUILDING_HOUSE_CRUDE_HUT && b->type <= BUILDING_HOUSE_PALATIAL_ESTATE && b->state;
}

static int32_t health_tile(int32_t grid_offset, void *context)
{
    const int32_t *p = context;
    int32_t building_id = map_grids.building_ids[grid_offset];
    if (building_id) {
        building *b = building_get(building_id);
        if (is_house_type(b)) {
            int32_t *value = building_field(b, p[0]);
            *value -= p[1];
            if (*value < 0) {
                *value = 0;
            }
        }
    }
    return 0;
}

int32_t service_provide_apothecary_health(int32_t x, int32_t y, int32_t amount)
{
    int32_t p[2] = { offsetof(building, house_malaria_risk), amount };
    area_run(x, y, service_rules[SERVICE_RULE_RADIUS], health_tile, p);
    return 0;
}

void service_provide_physician_health(int32_t x, int32_t y, int32_t amount)
{
    int32_t p[2] = { offsetof(building, house_plague_risk), amount };
    area_run(x, y, service_rules[SERVICE_RULE_RADIUS], health_tile, p);
}

static int32_t bast_complex_tile(int32_t grid_offset, void *context)
{
    (void) context;
    int32_t building_id = map_grids.building_ids[grid_offset];
    if (building_id) {
        building *b = building_get(building_id);
        if (is_house_type(b)) {
            b->data.house.is_sick = 0;
        }
    }
    return 0;
}

int32_t service_provide_bast_complex_coverage(int32_t x, int32_t y)
{
    area_run(x, y, service_rules[SERVICE_RULE_RADIUS], bast_complex_tile, 0);
    return 0;
}

static int32_t magistrate_tile(int32_t grid_offset, void *context)
{
    house_access_tile(grid_offset, context);
    return 0;
}

void service_provide_magistrate_coverage(int32_t x, int32_t y)
{
    area_run(x, y, service_rules[SERVICE_RULE_RADIUS], magistrate_tile, (void *) offsetof(building, data.house.courthouse));
}

static int32_t missionary_tile(int32_t grid_offset, void *context)
{
    (void) context;
    int32_t building_id = map_grids.building_ids[grid_offset];
    if (building_id > 0) {
        building *b = building_get(building_id);
        if (b->type == BUILDING_NATIVE_HUT || b->type == BUILDING_NATIVE_MEETING) {
            b->sentiment.native_anger = 0;
        }
    }
    return 0;
}

int32_t service_provide_missionary_coverage(int32_t x, int32_t y)
{
    area_run(x, y, service_rules[SERVICE_RULE_MISSIONARY_RADIUS], missionary_tile, 0);
    return 1;
}

static int32_t food_index(int32_t resource)
{
    switch (resource) {
        case RESOURCE_GRAIN:
            return INVENTORY_GRAIN;
        case RESOURCE_MEAT:
            return INVENTORY_MEAT;
        case RESOURCE_LETTUCE:
            return INVENTORY_LETTUCE;
        case RESOURCE_CHICKPEAS:
            return INVENTORY_CHICKPEAS;
        case RESOURCE_POMEGRANATES:
            return INVENTORY_POMEGRANATES;
        case RESOURCE_FIGS:
            return INVENTORY_FIGS;
        case RESOURCE_FISH:
            return INVENTORY_FISH;
        case RESOURCE_GAMEMEAT:
            return INVENTORY_GAMEMEAT;
        case RESOURCE_POTTERY:
            return INVENTORY_POTTERY;
        case RESOURCE_BEER:
            return INVENTORY_BEER;
        case RESOURCE_LINEN:
            return INVENTORY_LINEN;
        case RESOURCE_LUXURY_GOODS:
            return INVENTORY_JEWELRY;
        default:
            return INVENTORY_NONE;
    }
}

static void deliver_good(building *market, building *house, int32_t need, int32_t demand, int32_t good)
{
    if (house->size > 1) {
        need *= service_rules[SERVICE_RULE_LARGE_HOUSE_GOODS_FACTOR];
    }
    if (!need) {
        return;
    }
    market->data.market.good_demands[demand] = (int16_t) service_rules[SERVICE_RULE_MARKET_DEMAND];
    int16_t *market_stock = &market->data.market.inventory[good];
    int16_t *house_stock = &house->data.house.inventory[good];
    int16_t stock = *market_stock;
    if (stock <= 0) {
        return;
    }
    int16_t have = *house_stock;
    int32_t want = (need * service_rules[SERVICE_RULE_GOODS_STOCK_FACTOR]) - have;
    if (want <= 0) {
        return;
    }
    if (want > stock) {
        *house_stock = (int16_t) (have + stock);
        *market_stock = 0;
    } else {
        *house_stock = (int16_t) (have + want);
        *market_stock -= (int16_t) want;
    }
}

typedef struct {
    building *market;
} market_context;

static int32_t market_tile(int32_t grid_offset, void *context)
{
    building *market = ((market_context *) context)->market;
    int32_t building_id = map_grids.building_ids[grid_offset];
    if (!is_inhabited_house(building_id)) {
        return 0;
    }
    building *house = building_get(building_id);
    int32_t level = house->subtype;
    if (level < HOUSE_LEVEL_PALATIAL_ESTATE) {
        level++;
    }
    int32_t population = house->house_population;
    int32_t types_left = house_models[level].food_types;
    if (types_left > 0 && types_left < 4) {
        types_left++;
    }
    for (int32_t i = 0; i < city_foods.count; i++) {
        int32_t resource = city_foods.list[i];
        if (!resource) {
            continue;
        }
        int32_t index = food_index(resource);
        int16_t *market_stock = &market->data.market.inventory[index];
        int16_t stock = *market_stock;
        if (stock <= 0 || !types_left) {
            continue;
        }
        int16_t *house_stock = &house->data.house.inventory[index];
        int16_t have = *house_stock;
        int32_t amount = (population * service_rules[SERVICE_RULE_FOOD_STOCK_PER_PERSON]) - have;
        int32_t share = population * service_rules[SERVICE_RULE_FOOD_PER_PERSON];
        if (share < amount) {
            amount = share;
        }
        if (amount <= 0) {
            continue;
        }
        if (index >= INVENTORY_GRAIN && index <= INVENTORY_GAMEMEAT) {
            int32_t takes_whole_amount = (index == INVENTORY_GRAIN) || (index == INVENTORY_FISH);
            if (stock >= amount) {
                if (!takes_whole_amount) {
                    amount >>= 1;
                }
                *house_stock = (int16_t) (have + amount);
                *market_stock -= (int16_t) amount;
            } else if (!takes_whole_amount && stock >= (amount >> 1)) {
                amount >>= 1;
                *house_stock = (int16_t) (have + amount);
                *market_stock -= (int16_t) amount;
            } else {
                *house_stock = (int16_t) (have + stock);
                *market_stock = 0;
            }
        }
        types_left--;
    }
    const house_model *model = &house_models[level];
    deliver_good(market, house, model->beer, MARKET_DEMAND_BEER, INVENTORY_BEER);
    deliver_good(market, house, model->pottery, MARKET_DEMAND_POTTERY, INVENTORY_POTTERY);
    deliver_good(market, house, model->linen, MARKET_DEMAND_LINEN, INVENTORY_LINEN);
    deliver_good(market, house, model->jewelry, MARKET_DEMAND_JEWELRY, INVENTORY_JEWELRY);
    return 1;
}

int32_t service_provide_market_goods(int32_t market_id, int32_t x, int32_t y)
{
    market_context context = { building_get(market_id) };
    return area_run(x, y, service_rules[SERVICE_RULE_RADIUS], market_tile, &context);
}

static void update_seen(figure *f, int32_t value)
{
    uint8_t seen = f->min_max_seen;
    if (value > seen) {
        f->min_max_seen = (uint8_t) value;
    } else if (seen > service_rules[SERVICE_RULE_SEEN_FADE]) {
        f->min_max_seen = (uint8_t) (seen - service_rules[SERVICE_RULE_SEEN_FADE]);
    } else {
        f->min_max_seen = 0;
    }
}

static int32_t temple_god(int32_t type)
{
    if (type >= BUILDING_TEMPLE_OSIRIS && type <= BUILDING_TEMPLE_BAST) {
        return type - BUILDING_TEMPLE_OSIRIS;
    }
    if (type >= BUILDING_TEMPLE_COMPLEX_OSIRIS && type <= BUILDING_TEMPLE_COMPLEX_BAST) {
        return type - BUILDING_TEMPLE_COMPLEX_OSIRIS;
    }
    return -1;
}

static int32_t priest_coverage(figure *f)
{
    const city_player *player = city_player_current();
    int32_t temple_id = f->building_id;
    int32_t complex_id = player->temple_complex_building_id;
    int32_t houses = 0;
    int32_t god = temple_god(building_get(temple_id)->type);
    if (god >= GOD_OSIRIS) {
        houses = service_provide_religion_coverage(f->x, f->y, god);
    }
    if (!player->has_temple_complex) {
        return houses;
    }
    if (building_temple_god(complex_id) != building_temple_god(temple_id)) {
        return houses;
    }
    switch (building_temple_god(complex_id)) {
        case GOD_RA:
            if (building_temple_complex_has_upgrade(complex_id, TEMPLE_COMPLEX_ALTAR)) {
                service_provide_crime_coverage(f->x, f->y, building_model_get(BUILDING_TEMPLE_COMPLEX_RA)->service_reduction);
                f->min_max_seen = (uint8_t) service_crime_seen;
            }
            break;
        case GOD_SETH:
            if (building_temple_complex_has_upgrade(complex_id, TEMPLE_COMPLEX_ORACLE)) {
                service_provide_crime_coverage(f->x, f->y, building_model_get(BUILDING_TEMPLE_COMPLEX_SETH)->service_reduction);
                f->min_max_seen = (uint8_t) service_crime_seen;
            }
            break;
        case GOD_BAST:
            if (building_temple_complex_has_upgrade(complex_id, SERVICE_BAST_COMPLEX_UPGRADE)) {
                service_provide_bast_complex_coverage(f->x, f->y);
            }
            break;
    }
    return houses;
}

static int32_t provide_coverage(int32_t figure_id)
{
    figure *f = figure_get(figure_id);
    int32_t type = f->type;
    if (type == FIGURE_PATRICIAN) {
        return 0;
    }
    int32_t x = f->x;
    int32_t y = f->y;
    int32_t houses = 0;
    switch (type) {
        case FIGURE_LABOR_SEEKER:
            houses = service_provide_labor_seeker_coverage(x, y);
            break;
        case FIGURE_TAX_COLLECTOR:
            houses = service_provide_tax_collector_coverage(x, y);
            f->min_max_seen = (uint8_t) service_max_tax_multiplier;
            break;
        case FIGURE_MARKET_TRADER:
            houses = service_provide_market_goods(f->building_id, x, y);
            break;
        case FIGURE_WATER_CARRIER:
            houses = service_provide_water_coverage(x, y);
            break;
        case FIGURE_PHYSICIAN:
            houses = service_provide_physician_coverage(x, y);
            service_provide_physician_health(f->x, f->y, building_model_get(BUILDING_PHYSICIAN)->service_reduction);
            break;
        case FIGURE_TEACHER:
            houses = service_provide_school_coverage(x, y);
            break;
        case FIGURE_LIBRARIAN:
            houses = service_provide_library_coverage(x, y);
            break;
        case FIGURE_DENTIST:
            houses = service_provide_dentist_coverage(x, y);
            break;
        case FIGURE_HERBALIST:
            houses = service_provide_apothecary_coverage(x, y);
            update_seen(f, service_provide_apothecary_health(f->x, f->y, building_model_get(BUILDING_APOTHECARY)->service_reduction));
            break;
        case FIGURE_PLAGUED_CITIZEN:
            city_health_plague_spread(x, y);
            break;
        case FIGURE_EMBALMER:
            houses = service_provide_mortuary_coverage(x, y);
            break;
        case FIGURE_MISSIONARY:
            houses = service_provide_missionary_coverage(x, y);
            break;
        case FIGURE_PRIEST:
            houses = priest_coverage(f);
            break;
        case FIGURE_ARCHITECT:
            houses = service_provide_engineer_coverage(x, y);
            update_seen(f, service_max_damage_seen);
            break;
        case FIGURE_FIREMAN:
            houses = service_provide_fireman_coverage(x, y);
            f->min_max_seen = (uint8_t) houses;
            break;
        case FIGURE_CONSTABLE:
            service_provide_crime_coverage(x, y, building_model_get(BUILDING_POLICE_STATION)->service_reduction);
            f->min_max_seen = (uint8_t) service_crime_seen;
            break;
        case FIGURE_MAGISTRATE:
            service_provide_crime_coverage(x, y, building_model_get(BUILDING_COURTHOUSE)->service_reduction);
            service_provide_magistrate_coverage(f->x, f->y);
            f->min_max_seen = (uint8_t) service_crime_seen;
            break;
        case FIGURE_JUGGLER:
            houses = service_provide_juggler_coverage(x, y);
            break;
        case FIGURE_MUSICIAN:
            houses = service_provide_musician_coverage(x, y);
            break;
        case FIGURE_DANCER:
            houses = service_provide_dancer_coverage(x, y);
            break;
        case FIGURE_SENET_PLAYER:
            houses = service_provide_senet_coverage(x, y);
            break;
        case FIGURE_ZOOKEEPER:
            houses = service_provide_zoo_coverage(x, y);
            break;
    }
    if (f->building_id) {
        building *b = building_get(f->building_id);
        b->houses_covered += (int16_t) houses;
        if (b->houses_covered > service_rules[SERVICE_RULE_HOUSES_COVERED_MAX]) {
            b->houses_covered = (int16_t) service_rules[SERVICE_RULE_HOUSES_COVERED_MAX];
        }
    }
    map_service_visit_increment(figure_type_coverage_tag[type], f->grid_offset);
    return 0;
}

int32_t service_provide_school_coverage(int32_t x, int32_t y)
{
    return provide_house_access(x, y, offsetof(building, data.house.school));
}

int32_t service_provide_library_coverage(int32_t x, int32_t y)
{
    return provide_house_access(x, y, offsetof(building, data.house.library));
}

int32_t service_provide_dentist_coverage(int32_t x, int32_t y)
{
    return provide_house_access(x, y, offsetof(building, data.house.dentist));
}

int32_t service_provide_physician_coverage(int32_t x, int32_t y)
{
    return provide_house_access(x, y, offsetof(building, data.house.physician));
}

int32_t service_provide_apothecary_coverage(int32_t x, int32_t y)
{
    return provide_house_access(x, y, offsetof(building, data.house.apothecary));
}

int32_t service_provide_mortuary_coverage(int32_t x, int32_t y)
{
    return provide_house_access(x, y, offsetof(building, data.house.mortuary));
}

int32_t service_provide_juggler_coverage(int32_t x, int32_t y)
{
    return provide_house_access(x, y, offsetof(building, data.house.juggler));
}

int32_t service_provide_senet_coverage(int32_t x, int32_t y)
{
    return provide_house_access(x, y, offsetof(building, data.house.senet));
}

int32_t service_provide_zoo_coverage(int32_t x, int32_t y)
{
    return provide_house_access(x, y, offsetof(building, data.house.zoo));
}

int32_t service_provide_musician_coverage(int32_t x, int32_t y)
{
    return provide_house_access(x, y, offsetof(building, data.house.musician));
}

int32_t service_provide_dancer_coverage(int32_t x, int32_t y)
{
    return provide_house_access(x, y, offsetof(building, data.house.dancer));
}

int32_t figure_service_provide_coverage(int32_t figure_id)
{
    return provide_coverage(figure_id);
}
