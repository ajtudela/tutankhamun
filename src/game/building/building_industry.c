// Industry production and dock water access: the daily production progress of every industry (labor, inputs,
// curses, the Ptah complex bonus), the harvest flag of each crop and the harvests (meadow and floodplain, the Osiris
// blessing, soil fertility), the farm queries, and the open water check of the docks.
//
// Clearing a crop's harvest flag restarts the crop scan at crop 0 but not the month index.

#include "building/building_industry.h"
#include "building/building.h"
#include "building/building_figure_helpers.h"
#include "building/building_type.h"
#include "city/city_player.h"
#include "city/gods.h"
#include "city/labor.h"
#include "city/resource_type.h"
#include "core/calc.h"
#include "figure/figure.h"
#include "figure/figure_action.h"
#include "figure/figure_type.h"
#include "figure/laborer.h"
#include "flood/flood_types.h"
#include "game/game_data.h"
#include "game/game_internal.h"
#include "map/building_tiles.h"
#include "map/construction_terrain.h"
#include "map/farm.h"
#include "map/grid.h"
#include "map/map_data.h"
#include "map/routing_water.h"
#include "map/terrain.h"
#include "map/terrain_query.h"
#include "map/water.h"
#include "window/window_data.h"
#include <stdbool.h>
#include <stdint.h>

building_production building_productions[BUILDING_TYPE_COUNT] = {
    [BUILDING_GRAIN_FARM] = { .output = RESOURCE_GRAIN, .counted_as = RESOURCE_GRAIN },
    [BUILDING_LETTUCE_FARM] = { .output = RESOURCE_LETTUCE, .counted_as = RESOURCE_LETTUCE },
    [BUILDING_POMEGRANATES_FARM] = { .output = RESOURCE_POMEGRANATES, .counted_as = RESOURCE_POMEGRANATES },
    [BUILDING_FLAX_FARM] = { .output = RESOURCE_FLAX, .counted_as = RESOURCE_FLAX },
    [BUILDING_HENNA_FARM] = { .output = RESOURCE_HENNA, .counted_as = RESOURCE_HENNA },
    [BUILDING_CHICKPEAS_FARM] = { .output = RESOURCE_CHICKPEAS, .counted_as = RESOURCE_CHICKPEAS },
    [BUILDING_BARLEY_FARM] = { .output = RESOURCE_BARLEY, .counted_as = RESOURCE_BARLEY },
    [BUILDING_CATTLE_RANCH] = { .output = RESOURCE_MEAT, .input = INDUSTRY_INPUT_STRAW, .counted_as = RESOURCE_MEAT, .takes_input = 1 },
    [BUILDING_FIG_FARM] = { .output = RESOURCE_FIGS, .counted_as = RESOURCE_FIGS },
    [BUILDING_REED_GATHERER] = { .output = RESOURCE_REEDS, .counted_as = RESOURCE_REEDS },
    [BUILDING_STONE_QUARRY] = { .output = RESOURCE_STONE, .counted_as = RESOURCE_STONE },
    [BUILDING_SANDSTONE_QUARRY] = { .output = RESOURCE_SANDSTONE, .counted_as = RESOURCE_SANDSTONE },
    [BUILDING_LIMESTONE_QUARRY] = { .output = RESOURCE_LIMESTONE, .counted_as = RESOURCE_LIMESTONE },
    [BUILDING_GRANITE_QUARRY] = { .output = RESOURCE_GRANITE, .counted_as = RESOURCE_GRANITE },
    [BUILDING_GOLD_MINE] = { .output = RESOURCE_GOLD, .counted_as = RESOURCE_GOLD },
    [BUILDING_COPPER_MINE] = { .output = RESOURCE_COPPER, .counted_as = RESOURCE_COPPER },
    [BUILDING_GEMSTONE_MINE] = { .output = RESOURCE_GEMS, .counted_as = RESOURCE_GEMS },
    [BUILDING_WOOD_CUTTER] = { .output = RESOURCE_TIMBER, .counted_as = RESOURCE_TIMBER },
    [BUILDING_CLAY_PIT] = { .output = RESOURCE_CLAY, .counted_as = RESOURCE_CLAY },
    [BUILDING_BREWERY] = { .output = RESOURCE_BEER, .input = INDUSTRY_INPUT_BARLEY, .counted_as = RESOURCE_BEER, .takes_input = 1 },
    [BUILDING_WEAVER] = { .output = RESOURCE_LINEN, .input = INDUSTRY_INPUT_FLAX, .counted_as = RESOURCE_LINEN, .takes_input = 1 },
    [BUILDING_PAPYRUS_MAKER] = { .output = RESOURCE_PAPYRUS, .input = INDUSTRY_INPUT_REEDS, .counted_as = RESOURCE_PAPYRUS, .takes_input = 1 },
    [BUILDING_BRICKWORKS] = { .output = RESOURCE_BRICKS, .input = INDUSTRY_INPUT_CLAY, .second_input = INDUSTRY_INPUT_STRAW, .counted_as = RESOURCE_BRICKS, .takes_input = 1 },
    [BUILDING_LAMP_MAKER] = { .output = RESOURCE_LAMPS, .input = INDUSTRY_INPUT_OIL, .second_input = INDUSTRY_INPUT_POTTERY, .counted_as = RESOURCE_LAMPS, .takes_input = 1 },
    [BUILDING_PAINT_MAKER] = { .output = RESOURCE_PAINT, .input = INDUSTRY_INPUT_HENNA, .counted_as = RESOURCE_PAINT, .takes_input = 1 },
    [BUILDING_CHARIOT_MAKER] = { .output = RESOURCE_CHARIOTS, .input = INDUSTRY_INPUT_TIMBER, .counted_as = RESOURCE_CHARIOTS, .takes_input = 1 },
    [BUILDING_WEAPONSMITH] = { .output = RESOURCE_WEAPONS, .input = INDUSTRY_INPUT_COPPER, .counted_as = RESOURCE_WEAPONS, .takes_input = 1 },
    [BUILDING_JEWELER] = { .output = RESOURCE_LUXURY_GOODS, .input = INDUSTRY_INPUT_GEMS, .counted_as = RESOURCE_LUXURY_GOODS, .takes_input = 1 },
    [BUILDING_POTTER] = { .output = RESOURCE_POTTERY, .input = INDUSTRY_INPUT_CLAY, .counted_as = RESOURCE_POTTERY, .takes_input = 1 },
    [BUILDING_CARPENTERS_GUILD] = { .input = INDUSTRY_INPUT_TIMBER, .takes_input = 1 },
    [BUILDING_ARTISANS_GUILD] = { .input = INDUSTRY_INPUT_PAINT, .second_input = INDUSTRY_INPUT_CLAY, .takes_input = 1 },
    [BUILDING_RECRUITER] = { .input = INDUSTRY_INPUT_WEAPONS, .second_input = INDUSTRY_INPUT_CHARIOTS },
    [BUILDING_SHIPWRIGHT] = { .input = INDUSTRY_INPUT_TIMBER, .takes_input = 1 },
    [BUILDING_VILLAGE_PALACE_UP] = { .input = INDUSTRY_INPUT_GOLD, .takes_input = 1, .input_stock_max = 100000 },
    [BUILDING_TOWN_PALACE_UP] = { .input = INDUSTRY_INPUT_GOLD, .takes_input = 1, .input_stock_max = 100000 },
    [BUILDING_CITY_PALACE] = { .input = INDUSTRY_INPUT_GOLD, .takes_input = 1, .input_stock_max = 100000 },
    [BUILDING_LIBRARY] = { .input = INDUSTRY_INPUT_PAPYRUS, .takes_input = 1, .input_stock_max = 500 },
    [BUILDING_SCRIBAL_SCHOOL] = { .input = INDUSTRY_INPUT_PAPYRUS, .takes_input = 1, .input_stock_max = 300 },
    [BUILDING_MORTUARY] = { .input = INDUSTRY_INPUT_LINEN, .takes_input = 1, .input_stock_max = 500 },
    [BUILDING_SENET_HOUSE] = { .input = INDUSTRY_INPUT_BEER, .takes_input = 1, .input_stock_max = 300 },
    [BUILDING_ZOO] = { .input = INDUSTRY_INPUT_GAMEMEAT, .second_input = INDUSTRY_INPUT_STRAW, .takes_input = 1 },
    [BUILDING_HUNTING_LODGE] = { .counted_as = RESOURCE_GAMEMEAT },
    [BUILDING_BRICKLAYERS_GUILD] = { .takes_input = 1 },
    [BUILDING_VILLAGE_PALACE] = { .input_stock_max = 100000 },
    [BUILDING_TOWN_PALACE] = { .input_stock_max = 100000 },
    [BUILDING_TAX_COLLECTOR] = { .input_stock_max = 100000 },
    [BUILDING_TAX_COLLECTOR_UP] = { .input_stock_max = 100000 },
    [BUILDING_COURTHOUSE] = { .input_stock_max = 100000 },
};

int32_t industry_rules[INDUSTRY_RULE_COUNT] = {
    [INDUSTRY_RULE_PROGRESS_DONE] = 200,
    [INDUSTRY_RULE_INPUT_LOAD] = 100,
    [INDUSTRY_RULE_BRICKWORKS_STRAW_LOAD] = 25,
    [INDUSTRY_RULE_INPUT_STOCK_DEFAULT] = 200,
};

enum industry_value {
    FARM_HARVEST_LOADS_PER_PERCENT_SHIFT = 3,
    FARM_FERTILITY_TAKEN_WATERED = 50,
    FARM_FERTILITY_TAKEN_DRY = 80,
    DOCK_SIZE_TILES = 3,
};

enum industry_labor_days {
    LABOR_DAYS_FIRST_LABORER = 0,
    LABOR_DAYS_BLESSING = 2,
};

enum ptah_bonus {
    PTAH_BONUS_NONE = 0,
    PTAH_BONUS_COMPLEX = 1,
    PTAH_BONUS_FIRST_UPGRADE = 2,
};

bool building_type_is_farm(int32_t type)
{
    switch (type) {
        case BUILDING_BARLEY_FARM:
        case BUILDING_FLAX_FARM:
        case BUILDING_GRAIN_FARM:
        case BUILDING_LETTUCE_FARM:
        case BUILDING_POMEGRANATES_FARM:
        case BUILDING_CHICKPEAS_FARM:
        case BUILDING_FIG_FARM:
        case BUILDING_HENNA_FARM:
            return 1;
        default:
            return 0;
    }
}

bool building_is_farm(int32_t building_id)
{
    return building_type_is_farm(building_get(building_id)->type);
}

bool building_is_on_floodplain(int32_t building_id)
{
    const building *b = building_get(building_id);
    return map_terrain_has_floodplain(b->x, b->y, FARM_SIZE, FARM_SIZE);
}

bool building_is_floodplain_farm(int32_t building_id)
{
    return building_is_farm(building_id) && building_is_on_floodplain(building_id);
}

void farm_update_harvest_flags(void)
{
    uint8_t *flags = farm_harvest_flags;
    int32_t previous = (game_time_month - 1) % 11;
    if (game_time_day != 0) {
        return;
    }
    for (int32_t crop = 0; crop < FARM_CROP_ROWS; crop++) {
        for (int32_t i = 0; i < FARM_HARVESTS_PER_CROP; i++) {
            int32_t month = farm_harvest_months[(crop * FARM_HARVESTS_PER_CROP) + i];
            if (month == previous && flags[crop]) {
                flags[crop] = 0;
                crop = 0;
                continue;
            }
            if (month == -1) {
                break;
            }
            if (month == game_time_month) {
                flags[crop] = 1;
                break;
            }
        }
    }
}

bool building_farm_update_production(int32_t building_id)
{
    int32_t id = building_id;
    if (!building_is_farm(id)) {
        return 0;
    }
    building *b = building_get(id);
    if (b->data.industry.curse_days_left) {
        b->data.industry.curse_days_left--;
        return 1;
    }
    int32_t growth = building_farm_expected_produce(id, 0);
    if (growth) {
        b->data.industry.progress = (int16_t) (b->data.industry.progress + growth);
        if (b->data.industry.progress > farm_max_progress) {
            b->data.industry.progress = (int16_t) farm_max_progress;
        }
    }
    map_building_place_farm(id, 0);
    return 1;
}

static int32_t ptah_bonus_of(int32_t type)
{
    switch (type) {
        case BUILDING_SHIPWRIGHT:
        case BUILDING_CLAY_PIT:
        case BUILDING_WEAVER:
        case BUILDING_JEWELER:
        case BUILDING_GOLD_MINE:
        case BUILDING_GEMSTONE_MINE:
        case BUILDING_COPPER_MINE:
            return PTAH_BONUS_COMPLEX;
        case BUILDING_STONE_QUARRY:
        case BUILDING_LIMESTONE_QUARRY:
        case BUILDING_CARPENTERS_GUILD:
        case BUILDING_BRICKLAYERS_GUILD:
        case BUILDING_STONEMASONS_GUILD:
        case BUILDING_BRICKWORKS:
        case BUILDING_GRANITE_QUARRY:
        case BUILDING_SANDSTONE_QUARRY:
        case BUILDING_ARTISANS_GUILD:
            return PTAH_BONUS_FIRST_UPGRADE;
        default:
            return PTAH_BONUS_NONE;
    }
}

static int32_t has_ptah_bonus(int32_t type)
{
    int32_t bonus = ptah_bonus_of(type);
    const city_player *player = city_player_current();
    return bonus != PTAH_BONUS_NONE && player->has_temple_complex && building_temple_god(player->temple_complex_building_id) == GOD_PTAH &&
        (bonus == PTAH_BONUS_COMPLEX || building_temple_complex_has_upgrade(player->temple_complex_building_id, 1));
}

static int32_t second_input_load(int32_t type)
{
    return industry_rules[(type != BUILDING_BRICKWORKS) ? INDUSTRY_RULE_INPUT_LOAD : INDUSTRY_RULE_BRICKWORKS_STRAW_LOAD];
}

int32_t building_takes_input(int32_t type)
{
    return building_productions[type].takes_input;
}

int32_t building_input_stock_max(int32_t type)
{
    int32_t max = building_productions[type].input_stock_max;
    return (max != 0) ? max : industry_rules[INDUSTRY_RULE_INPUT_STOCK_DEFAULT];
}

static void produce(building *b, int32_t id)
{
    if (building_is_floodplain_farm(id)) {
        if (!b->data.industry.worker_count || city_labor_industry_is_mothballed(id)) {
            b->num_workers = 0;
        } else {
            b->num_workers = (int16_t) building_model_table[b->type].laborers;
        }
    }
    if (b->houses_covered < 1 && !building_is_floodplain_farm(id)) {
        return;
    }
    if (b->num_workers <= 0) {
        return;
    }
    if (b->subtype != INDUSTRY_INPUT_NONE && b->loads_stored < industry_rules[INDUSTRY_RULE_INPUT_LOAD]) {
        return;
    }
    if (b->second_input_resource != INDUSTRY_INPUT_NONE && b->second_input_stock < second_input_load(b->type)) {
        return;
    }
    if (b->data.industry.curse_days_left) {
        b->data.industry.curse_days_left--;
        return;
    }
    if (b->data.industry.labor_days_left[LABOR_DAYS_BLESSING]) {
        b->data.industry.labor_days_left[LABOR_DAYS_BLESSING]--;
    }
    if (!building_farm_update_production(id)) {
        b->data.industry.progress = (int16_t) (b->data.industry.progress + b->num_workers);
    }
    if (has_ptah_bonus(b->type)) {
        b->data.industry.progress = (int16_t) (b->data.industry.progress + (b->num_workers / 2));
    }
    if (!building_is_farm(id)) {
        int32_t done = industry_rules[INDUSTRY_RULE_PROGRESS_DONE];
        int32_t max = (b->subtype != INDUSTRY_INPUT_NONE) ? (2 * done) : done;
        if (b->data.industry.progress > max) {
            b->data.industry.progress = (int16_t) max;
        }
    }
}

void building_industry_update_production(void)
{
    farm_update_harvest_flags();
    for (int32_t id = 1; id <= building_highest_id; id++) {
        building *b = building_get(id);
        if (b->state == BUILDING_STATE_IN_USE && b->output_resource) {
            produce(b, id);
        } else if (b->type == BUILDING_CARPENTERS_GUILD && b->state != BUILDING_STATE_UNUSED && b->num_workers > 0 &&
            (b->subtype == INDUSTRY_INPUT_NONE || b->loads_stored > 0)) {
            b->data.industry.progress = (int16_t) (b->data.industry.progress + b->num_workers);
            int32_t max = 2 * industry_rules[INDUSTRY_RULE_PROGRESS_DONE];
            if (b->data.industry.progress > max) {
                b->data.industry.progress = (int16_t) max;
            }
        }
    }
}

void building_dock_update_open_water_access(void)
{
    map_routing_calculate_distances_water_boat(scenario_data.river_entry_x, scenario_data.river_entry_y);
    for (int32_t id = 1; id <= building_highest_id; id++) {
        building *b = building_get(id);
        if (b->state == BUILDING_STATE_IN_USE && !b->house_size && (b->type == BUILDING_DOCK || b->type == BUILDING_DOCK_UP)) {
            b->has_water_access = map_terrain_is_adjacent_to_open_water(b->x, b->y, DOCK_SIZE_TILES) ? 1 : 0;
        }
    }
}

bool building_is_crop_farm(int32_t building_id)
{
    return building_type_is_farm(building_get(building_id)->type);
}

int32_t building_industry_has_produced_resource(int32_t building_id)
{
    building *b = building_get(building_id);
    int32_t done = industry_rules[INDUSTRY_RULE_PROGRESS_DONE];
    int32_t needed = ((b->subtype != INDUSTRY_INPUT_NONE) ? done : 0) + done;
    return b->data.industry.progress >= needed;
}

void building_industry_start_new_production(int32_t building_id)
{
    building *b = building_get(building_id);
    b->data.industry.progress = 0;
    if (b->subtype != INDUSTRY_INPUT_NONE && b->loads_stored != 0) {
        b->loads_stored = (int16_t) (b->loads_stored - industry_rules[INDUSTRY_RULE_INPUT_LOAD]);
    }
    if (b->second_input_resource != INDUSTRY_INPUT_NONE && b->second_input_stock != 0) {
        b->second_input_stock = (int16_t) (b->second_input_stock - second_input_load(b->type));
    }
}

int32_t building_farm_crop(int32_t type)
{
    switch (type) {
        case BUILDING_BARLEY_FARM:
            return FARM_CROP_BARLEY;
        case BUILDING_FLAX_FARM:
            return FARM_CROP_FLAX;
        case BUILDING_GRAIN_FARM:
            return FARM_CROP_GRAIN;
        case BUILDING_LETTUCE_FARM:
            return FARM_CROP_LETTUCE;
        case BUILDING_POMEGRANATES_FARM:
            return FARM_CROP_POMEGRANATES;
        case BUILDING_CHICKPEAS_FARM:
            return FARM_CROP_CHICKPEAS;
        case BUILDING_REED_GATHERER:
            return FARM_CROP_REEDS;
        case BUILDING_FIG_FARM:
            return FARM_CROP_FIGS;
        case BUILDING_HENNA_FARM:
            return FARM_CROP_HENNA;
        default:
            return FARM_CROP_NONE;
    }
}

static void send_laborer_back(building *b)
{
    int32_t laborer_id = b->data.industry.worker_figure_id;
    if (laborer_id != 0) {
        figure *f = figure_get(laborer_id);
        if (f->state != FIGURE_STATE_DEAD && f->type == FIGURE_LABORER && f->action_state != FIGURE_ACTION_ON_FERRY &&
            f->action_state != FIGURE_ACTION_WAIT_FOR_FERRY) {
            figure_laborer_find_work(&floods_data, laborer_id);
        }
        b->data.industry.worker_figure_id = 0;
    }
}

int32_t farm_harvest_loads(int32_t id)
{
    if (game_time_day != 0 && game_time_day != 7) {
        return 0;
    }
    int32_t crop = building_is_farm(id) ? building_farm_crop(building_get(id)->type) : FARM_CROP_NONE;
    bool floodplain = building_is_on_floodplain(id);
    if (floodplain) {
        if (floods_data.state != FLOOD_STATE_IMMINENT) {
            return 0;
        }
    } else if (farm_harvest_flags[crop] == 0 || game_time_day != 0) {
        return 0;
    }
    int32_t loads = farm_growth_percentage_of(id) << FARM_HARVEST_LOADS_PER_PERCENT_SHIFT;
    building_industry_start_new_production(id);
    map_building_place_farm(id, 0);
    farm_reduce_fertility_of(id, farm_has_water_access_of(id) ? FARM_FERTILITY_TAKEN_WATERED : FARM_FERTILITY_TAKEN_DRY);
    if (floodplain) {
        if (city_player_current()->osiris_double_farm_yield != 0) {
            loads += loads;
            osiris_double_yield_used = 1;
        }
        building *b = building_get(id);
        b->data.industry.labor_days_left[LABOR_DAYS_FIRST_LABORER] = 0;
        b->num_workers = 0;
        send_laborer_back(b);
    }
    return loads;
}

int32_t farm_growth_percentage_of(int32_t building_id)
{
    return calc_percentage(building_get(building_id)->data.industry.progress, farm_max_progress);
}

int32_t farm_has_water_access_of(int32_t building_id)
{
    return building_get(building_id)->has_water_access != 0;
}

void farm_reduce_fertility_of(int32_t building_id, int32_t percent)
{
    if (!building_is_on_floodplain(building_id)) {
        return;
    }
    building *b = building_get(building_id);
    int32_t base = map_grid_offset(b->x, b->y);
    map_tile_cursor_offset = base;
    for (int32_t row = 0; row < FARM_SIZE; row++) {
        for (int32_t col = 0; col < FARM_SIZE; col++) {
            int32_t offset = building_square_tile_offset(row, col) + base;
            int32_t fertility = (map_grids.soil_fertility[offset] * (100 - percent)) / 100;
            if (fertility < 1) {
                fertility = 1;
            }
            map_grids.soil_fertility[offset] = (uint8_t) fertility;
        }
    }
}
