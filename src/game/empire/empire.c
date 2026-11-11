// The empire: trade cities, map objects and routes; the import and export checks of the trade figures, the
// produce checks of the advisors, the stock a city keeps before it exports, the yearly reset of the traded amounts,
// the arrival of new traders (ships need a dock, caravans come from the entry point), the images of the map objects
// and the message tokens that name the player, a city, an amount or a resource.
//
// Details of the checks and images:
// - the produce checks of lamps and bricks fall back to what our city sells, checked in a fixed order;
// - an ornament's image is capped at the last ornament image compared as unsigned 16 bits;
// - a route with fewer than two way points is marked unused when a city's route is looked up.

#include "empire/empire.h"

#include "building/building.h"
#include "building/building_data.h"
#include "building/building_type.h"
#include "building/monument_spots.h"
#include "city/city_data.h"
#include "city/city_player.h"
#include "city/gods.h"
#include "city/message.h"
#include "city/resource_type.h"
#include "city/resources.h"
#include "editor/editor_allowed_buildings.h"
#include "empire/empire_data.h"
#include "empire/empire_route.h"
#include "figure/figure.h"
#include "figure/figure_action.h"
#include "figure/figure_core.h"
#include "figure/figure_data.h"
#include "figure/figure_type.h"
#include "game/game_internal.h"
#include "graphics/image_collection.h"
#include "scenario/event_helpers.h"
#include "scenario/event_manager.h"
#include "text/lang.h"
#include "text/text_data.h"
#include "window/window_image_group.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

char token_buffer_amount[8];

char token_buffer_title[128];

enum {
    LUXURY_KINDS = 16,
    LUXURY_KIND_FIRST_TEXT = 38,
    FOOD_LIST_ENTRIES = 6,
    LOAD_UNITS = 100,
    ROUTE_TRAVEL_DIVISOR = 400,
    RANK_TEXT_FIRST = 11,
    RESOURCE_NAME_TEXT_FIRST = 54,
};

int32_t empire_trade_rules[EMPIRE_TRADE_RULE_COUNT] = {
    [EMPIRE_TRADE_RULE_IMPORT_DEBT_LIMIT] = -4999,
    [EMPIRE_TRADE_RULE_LIMIT_SMALL] = 15,
    [EMPIRE_TRADE_RULE_LIMIT_MEDIUM] = 25,
    [EMPIRE_TRADE_RULE_LIMIT_LARGE] = 40,
    [EMPIRE_TRADE_RULE_SEA_ENTRY_DELAY] = 30,
    [EMPIRE_TRADE_RULE_LAND_ENTRY_DELAY] = 4,
    [EMPIRE_TRADE_RULE_NEW_CITY_ENTRY_DELAY] = 4,
    [EMPIRE_TRADE_RULE_NO_DOCKS_MESSAGE_TICKS] = 384,
    [EMPIRE_TRADE_RULE_SHIP_LOADS] = 12,
    [EMPIRE_TRADE_RULE_TRADER_WAIT_TICKS] = 10,
    [EMPIRE_TRADE_RULE_KEEP_WORKSHOP_AMOUNT] = 200,
    [EMPIRE_TRADE_RULE_KEEP_MONUMENT_AMOUNT] = 500,
    [EMPIRE_TRADE_RULE_KEEP_OTHER_AMOUNT] = 1000,
    [EMPIRE_TRADE_RULE_FOOD_POPULATION_SMALL] = 1000,
    [EMPIRE_TRADE_RULE_FOOD_POPULATION_MEDIUM] = 2500,
    [EMPIRE_TRADE_RULE_FOOD_POPULATION_LARGE] = 5000,
};

static int32_t route_limit(const empire_city *c, int32_t resource)
{
    return city_gods_ra_adjust_percentage(trade_route_limits[c->route_id][resource]);
}

int32_t empire_city_has_trade_limit(int32_t city_id, int32_t resource)
{
    return route_limit(&empire_cities[city_id], resource) > 0;
}

int32_t empire_can_export_resource_to_city(int32_t player_id, int32_t city_id, int32_t resource)
{
    const city_player *player = city_player_get(player_id);
    int32_t status = player->trade_status[resource];
    if (status != TRADE_STATUS_EXPORT && status != TRADE_STATUS_EXPORT_SURPLUS) {
        return 0;
    }
    if (resource == RESOURCE_UNUSED || resource < 0 || resource > RESOURCE_MARBLE) {
        return 0;
    }
    const empire_city *c = &empire_cities[city_id];
    int32_t limit = route_limit(c, resource);
    if (city_id != 0 && limit <= trade_route_traded[c->route_id][resource]) {
        return 0;
    }
    if (status == TRADE_STATUS_EXPORT) {
        if (player->storage_stored[resource] <= player->trading_amount[resource] * LOAD_UNITS) {
            return 0;
        }
    } else if (player->storage_stored[resource] <= trade_export_keep[resource]) {
        return 0;
    }
    if (city_id != 0 && !c->buys[resource]) {
        return 0;
    }
    return 1;
}

int32_t empire_can_import_resource_from_city(int32_t player_id, int32_t city_id, int32_t resource)
{
    if (city_player_current()->treasury < empire_trade_rules[EMPIRE_TRADE_RULE_IMPORT_DEBT_LIMIT]) {
        return 0;
    }
    const city_player *player = city_player_get(player_id);
    int32_t status = player->trade_status[resource];
    if (status != TRADE_STATUS_IMPORT && status != TRADE_STATUS_IMPORT_AS_NEEDED) {
        return 0;
    }
    if (resource == RESOURCE_UNUSED || resource < 0 || resource > RESOURCE_MARBLE) {
        return 0;
    }
    const empire_city *c = &empire_cities[city_id];
    if (!c->sells[resource]) {
        return 0;
    }
    int32_t limit = route_limit(c, resource);
    if (limit <= trade_route_traded[c->route_id][resource]) {
        return 0;
    }
    int32_t stock = player->storage_stored[resource];
    if (resource < RESOURCE_MIN) {
        return 0;
    }
    if (status == TRADE_STATUS_IMPORT) {
        return stock < player->trading_amount[resource] * LOAD_UNITS;
    }
    return stock < trade_export_keep[resource];
}

int32_t empire_can_import_resource(int32_t resource)
{
    for (int32_t id = 0; id < EMPIRE_CITY_COUNT; id++) {
        const empire_city *c = &empire_cities[id];
        if (c->in_use && empire_city_type_is_trading(c->type) && c->is_open && c->sells[resource] == 1 && empire_city_has_trade_limit(id, resource)) {
            return 1;
        }
    }
    return 0;
}

int32_t empire_can_import_resource_potentially(int32_t resource)
{
    for (int32_t id = 0; id < EMPIRE_CITY_COUNT; id++) {
        const empire_city *c = &empire_cities[id];
        if (c->in_use && empire_city_type_is_trading(c->type) && c->sells[resource] == 1 && empire_city_has_trade_limit(id, resource)) {
            return 1;
        }
    }
    return 0;
}

static int32_t our_city_sells(int32_t resource)
{
    for (int32_t id = 0; id < EMPIRE_CITY_COUNT; id++) {
        const empire_city *c = &empire_cities[id];
        if (c->in_use && c->type == EMPIRE_CITY_OURS && c->sells[resource] == 1) {
            return 1;
        }
    }
    return 0;
}

static int32_t raw_can_be_had(int32_t raw)
{
    return empire_can_produce_resource(raw) || empire_can_produce_resource_potentially(raw);
}

int32_t empire_can_produce_resource(int32_t resource)
{
    int32_t result;
    switch (resource) {
        case RESOURCE_BEER:
            result = raw_can_be_had(RESOURCE_BARLEY);
            break;
        case RESOURCE_LINEN:
            result = raw_can_be_had(RESOURCE_FLAX);
            break;
        case RESOURCE_PAINT:
            result = raw_can_be_had(RESOURCE_HENNA);
            break;
        case RESOURCE_LAMPS:
            if ((empire_can_produce_resource(RESOURCE_POTTERY) || empire_can_produce_resource_potentially(RESOURCE_POTTERY)) &&
                empire_can_import_resource(RESOURCE_OIL)) {
                return 1;
            }
            if (!empire_can_import_resource_potentially(RESOURCE_POTTERY)) {
                return our_city_sells(resource);
            }
            result = empire_can_import_resource(RESOURCE_OIL);
            break;
        case RESOURCE_LUXURY_GOODS:
            result = raw_can_be_had(RESOURCE_GEMS);
            break;
        case RESOURCE_POTTERY:
            result = raw_can_be_had(RESOURCE_CLAY);
            break;
        case RESOURCE_WEAPONS:
            if (!scenario_data.allowed_buildings[ALLOWED_WEAPONSMITH]) {
                return 0;
            }
            result = raw_can_be_had(RESOURCE_COPPER);
            break;
        case RESOURCE_CHARIOTS:
            if (!scenario_data.allowed_buildings[ALLOWED_CHARIOT_MAKER]) {
                return 0;
            }
            result = raw_can_be_had(RESOURCE_TIMBER);
            break;
        case RESOURCE_BRICKS:
            if (raw_can_be_had(RESOURCE_CLAY) && empire_can_produce_resource(RESOURCE_GRAIN)) {
                return 1;
            }
            if (raw_can_be_had(RESOURCE_CLAY) && empire_can_import_resource_potentially(RESOURCE_STRAW)) {
                return 1;
            }
            if (empire_can_produce_resource(RESOURCE_GRAIN) && empire_can_import_resource_potentially(RESOURCE_CLAY)) {
                return 1;
            }
            if (!empire_can_import_resource_potentially(RESOURCE_STRAW)) {
                return our_city_sells(resource);
            }
            result = empire_can_import_resource_potentially(RESOURCE_CLAY);
            break;
        case RESOURCE_PAPYRUS:
            result = raw_can_be_had(RESOURCE_REEDS);
            break;
        case RESOURCE_STRAW:
            result = empire_can_produce_resource(RESOURCE_GRAIN);
            break;
        default:
            return our_city_sells(resource);
    }
    if (result) {
        return 1;
    }
    return our_city_sells(resource);
}

int32_t empire_can_produce_resource_potentially(int32_t resource)
{
    int32_t result;
    switch (resource) {
        case RESOURCE_BEER:
            result = raw_can_be_had(RESOURCE_BARLEY);
            break;
        case RESOURCE_LINEN:
            result = raw_can_be_had(RESOURCE_FLAX);
            break;
        case RESOURCE_PAINT:
            result = raw_can_be_had(RESOURCE_HENNA);
            break;
        case RESOURCE_LUXURY_GOODS:
            result = raw_can_be_had(RESOURCE_GEMS);
            break;
        case RESOURCE_POTTERY:
            result = raw_can_be_had(RESOURCE_CLAY);
            break;
        case RESOURCE_WEAPONS:
            result = raw_can_be_had(RESOURCE_COPPER);
            break;
        case RESOURCE_CHARIOTS:
            result = raw_can_be_had(RESOURCE_TIMBER);
            break;
        case RESOURCE_BRICKS:
            if (!empire_can_produce_resource(RESOURCE_GRAIN) && !empire_can_produce_resource_potentially(RESOURCE_STRAW)) {
                return our_city_sells(resource);
            }
            if (empire_can_produce_resource(RESOURCE_CLAY)) {
                return 1;
            }
            result = empire_can_produce_resource_potentially(RESOURCE_CLAY);
            break;
        case RESOURCE_LAMPS:
            if (!empire_can_produce_resource(RESOURCE_POTTERY)) {
                return our_city_sells(resource);
            }
            result = empire_can_import_resource_potentially(RESOURCE_OIL);
            break;
        case RESOURCE_PAPYRUS:
            result = raw_can_be_had(RESOURCE_REEDS);
            break;
        default:
            result = empire_can_import_resource_potentially(resource);
            break;
    }
    if (result) {
        return 1;
    }
    return our_city_sells(resource);
}

void empire_city_reset_yearly_trade_amounts(void)
{
    for (int32_t id = 0; id < EMPIRE_CITY_COUNT; id++) {
        const empire_city *c = &empire_cities[id];
        if (c->in_use && c->is_open) {
            memset(&trade_route_traded[c->route_id][RESOURCE_MIN], 0, (RESOURCE_MAX - RESOURCE_MIN) * sizeof(int32_t));
        }
    }
}

int32_t empire_city_get_for_trade_route(uint32_t route_id)
{
    for (int32_t id = 0; id < EMPIRE_CITY_COUNT; id++) {
        const empire_city *c = &empire_cities[id];
        if (c->in_use && c->route_id == route_id) {
            return id;
        }
    }
    return 0;
}

int32_t empire_count_luxury_goods_kinds(int32_t *kinds)
{
    for (int32_t id = 1; id < EMPIRE_CITY_COUNT; id++) {
        const empire_city *c = &empire_cities[id];
        if (!c->in_use || !c->is_open) {
            continue;
        }
        int16_t status = city_player_current()->trade_status[RESOURCE_LUXURY_GOODS];
        if ((status == TRADE_STATUS_IMPORT || status == TRADE_STATUS_IMPORT_AS_NEEDED) && c->sells[RESOURCE_LUXURY_GOODS] &&
            empire_city_has_trade_limit(id, RESOURCE_LUXURY_GOODS)) {
            kinds[empire_culture_table[c->name_id].luxury_kind - LUXURY_KIND_FIRST_TEXT]++;
        }
    }
    int32_t count = 0;
    for (int32_t i = 0; i < LUXURY_KINDS; i++) {
        if (kinds[i] != 0) {
            count++;
        }
    }
    return count;
}

static void count_trade_problem_month(int16_t *duration)
{
    if (*duration > 0) {
        (*duration)--;
    } else {
        *duration = 0;
    }
}

static int32_t trader_size(int32_t city_id, const empire_city *c)
{
    int32_t resources = 0;
    int32_t size = 0;
    for (int32_t r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
        if ((c->buys[r] && empire_city_has_trade_limit(city_id, r)) || (c->sells[r] && empire_city_has_trade_limit(city_id, r))) {
            resources++;
            int32_t limit = trade_route_limits[c->route_id][r];
            if (limit > empire_trade_rules[EMPIRE_TRADE_RULE_LIMIT_MEDIUM]) {
                size += 3;
            } else if (limit > empire_trade_rules[EMPIRE_TRADE_RULE_LIMIT_SMALL]) {
                size += 2;
            } else if (limit > 0) {
                size++;
            }
        }
    }
    if (resources > 1) {
        if ((size % resources) == 0) {
            size = size / resources;
        } else {
            size = (size / resources) + 1;
        }
    }
    return size;
}

static int32_t free_trader_slot(const empire_city *c, int32_t size)
{
    int32_t slots = size < 3 ? size : 3;
    for (int32_t slot = 0; slot < slots; slot++) {
        if (c->trader_figures[slot] == 0) {
            return slot;
        }
    }
    return -1;
}

static int32_t river_entry_missing(void)
{
    return scenario_data.river_entry_x == -1 && scenario_data.river_entry_y == -1;
}

void empire_city_generate_trader(void)
{
    city_player *player = city_player_current();
    player->num_sea_routes = 0;
    player->num_land_routes = 0;
    player->luxury_goods_kinds = 0;
    int32_t kinds[LUXURY_KINDS] = { 0 };
    kinds[0] = building_count_industry_total[RESOURCE_LUXURY_GOODS] > 0;
    player->luxury_goods_kinds += empire_count_luxury_goods_kinds(kinds);
    count_trade_problem_month(&player->sea_trade_problem_duration);
    count_trade_problem_month(&player->land_trade_problem_duration);
    for (int32_t id = 1; id < EMPIRE_CITY_COUNT; id++) {
        empire_city *c = &empire_cities[id];
        if (!c->in_use || !c->is_open) {
            continue;
        }
        if (c->is_sea_trade == 1) {
            if (building_count_total[BUILDING_DOCK] <= 0) {
                if (city_message_count[MESSAGE_CATEGORY_NO_WORKING_DOCK] <= 0) {
                    city_message_use_popup = 1;
                    city_message_post(MESSAGE_NO_WORKING_DOCK, 0, 0, 0, 0);
                    city_message_count[MESSAGE_CATEGORY_NO_WORKING_DOCK] = empire_trade_rules[EMPIRE_TRADE_RULE_NO_DOCKS_MESSAGE_TICKS];
                } else {
                    city_message_count[MESSAGE_CATEGORY_NO_WORKING_DOCK]--;
                }
                continue;
            }
            if (river_entry_missing()) {
                continue;
            }
            player->num_sea_routes++;
        } else {
            player->num_land_routes++;
        }
        int32_t size = trader_size(id, c);
        if (size <= 0 || c->trader_blocked != 0) {
            continue;
        }
        int32_t slot = free_trader_slot(c, size);
        if (slot < 0) {
            continue;
        }
        if (c->trader_entry_delay > 0) {
            c->trader_entry_delay--;
            continue;
        }
        int32_t delay_rule = c->is_sea_trade == 1 ? EMPIRE_TRADE_RULE_SEA_ENTRY_DELAY : EMPIRE_TRADE_RULE_LAND_ENTRY_DELAY;
        c->trader_entry_delay = (int16_t) empire_trade_rules[delay_rule];
        if (c->is_sea_trade == 1) {
            if (building_count_total[BUILDING_DOCK] <= 0 || river_entry_missing() || player->sea_trade_problem_duration != 0 || player->ra_no_traders_months > 0) {
                continue;
            }
            int32_t ship_id = figure_create(city_current_player, FIGURE_TRADE_SHIP, scenario_data.river_entry_x, scenario_data.river_entry_y, 0);
            figure *ship = &city_figures.records[ship_id];
            ship->resource_amount = (int16_t) empire_trade_rules[EMPIRE_TRADE_RULE_SHIP_LOADS];
            ship->trader_amount_bought = 0;
            ship->empire_city_id = (uint8_t) id;
            ship->action_state = FIGURE_ACTION_TRADE_SHIP_CREATED;
            ship->wait_ticks = (int16_t) empire_trade_rules[EMPIRE_TRADE_RULE_TRADER_WAIT_TICKS];
            c->trader_figures[slot] = (int16_t) ship_id;
            ship->collecting_item_id = 0;
            return;
        }
        if (player->land_trade_problem_duration != 0 || player->ra_no_traders_months > 0) {
            continue;
        }
        int32_t caravan_id = figure_create(city_current_player, FIGURE_TRADE_CARAVAN, player->entry_point_x, player->entry_point_y, 0);
        c->trader_figures[slot] = (int16_t) caravan_id;
        figure *caravan = &city_figures.records[caravan_id];
        caravan->empire_city_id = (uint8_t) id;
        caravan->action_state = FIGURE_ACTION_TRADE_CARAVAN_CREATED;
        caravan->wait_ticks = (int16_t) empire_trade_rules[EMPIRE_TRADE_RULE_TRADER_WAIT_TICKS];
        int32_t donkey1_id = figure_create(city_current_player, FIGURE_TRADE_CARAVAN_DONKEY, player->entry_point_x, player->entry_point_y, 0);
        city_figures.records[donkey1_id].action_state = FIGURE_ACTION_TRADE_CARAVAN_CREATED;
        city_figures.records[donkey1_id].leading_figure_id = (int16_t) caravan_id;
        int32_t donkey2_id = figure_create(city_current_player, FIGURE_TRADE_CARAVAN_DONKEY, player->entry_point_x, player->entry_point_y, 0);
        city_figures.records[donkey2_id].action_state = FIGURE_ACTION_TRADE_CARAVAN_CREATED;
        city_figures.records[donkey2_id].leading_figure_id = (int16_t) donkey1_id;
        return;
    }
}

static int16_t workshop_material(int32_t resource)
{
    switch (resource) {
        case RESOURCE_MEAT:
            return 21;
        case RESOURCE_GAMEMEAT:
            return 22;
        case RESOURCE_STRAW:
            return 13;
        case RESOURCE_WEAPONS:
            return 8;
        case RESOURCE_CLAY:
            return 5;
        case RESOURCE_BRICKS:
            return 7;
        case RESOURCE_POTTERY:
            return 24;
        case RESOURCE_BARLEY:
            return 9;
        case RESOURCE_BEER:
            return 19;
        case RESOURCE_FLAX:
            return 1;
        case RESOURCE_LINEN:
            return 16;
        case RESOURCE_GEMS:
            return 10;
        case RESOURCE_TIMBER:
            return 4;
        case RESOURCE_GOLD:
            return 15;
        case RESOURCE_REEDS:
            return 11;
        case RESOURCE_PAPYRUS:
            return 12;
        case RESOURCE_STONE:
            return 3;
        case RESOURCE_LIMESTONE:
            return 6;
        case RESOURCE_CHARIOTS:
            return 14;
        case RESOURCE_COPPER:
            return 17;
        case RESOURCE_SANDSTONE:
            return 18;
        case RESOURCE_OIL:
            return 23;
        case RESOURCE_HENNA:
            return 20;
        case RESOURCE_PAINT:
            return 25;
        case RESOURCE_LAMPS:
            return 26;
        default:
            return -1;
    }
}

static int32_t uses_materials(int32_t type)
{
    if (type >= BUILDING_BREWERY && type <= BUILDING_POTTER) {
        return 1;
    }
    switch (type) {
        case BUILDING_SENET_HOUSE:
        case BUILDING_MORTUARY:
        case BUILDING_SCRIBAL_SCHOOL:
        case BUILDING_LIBRARY:
        case BUILDING_SHIPWRIGHT:
        case BUILDING_CARPENTERS_GUILD:
        case BUILDING_BRICKLAYERS_GUILD:
        case BUILDING_VILLAGE_PALACE_UP:
        case BUILDING_TOWN_PALACE_UP:
        case BUILDING_CITY_PALACE:
        case BUILDING_CATTLE_RANCH:
        case BUILDING_PAPYRUS_MAKER:
        case BUILDING_BRICKWORKS:
        case BUILDING_CHARIOT_MAKER:
        case BUILDING_ZOO:
        case BUILDING_ARTISANS_GUILD:
        case BUILDING_LAMP_MAKER:
        case BUILDING_PAINT_MAKER:
            return 1;
        default:
            return 0;
    }
}

int32_t building_count_workshops_using_resource(int32_t resource)
{
    int16_t material = workshop_material(resource);
    int32_t count = 0;
    if (building_highest_id < 1) {
        return 0;
    }
    for (int32_t id = 1; id <= building_highest_id; id++) {
        const building *b = building_get(id);
        if (b->state != BUILDING_STATE_IN_USE || b->num_workers == 0 || !uses_materials(b->type)) {
            continue;
        }
        if (b->subtype == material || b->second_input_resource == material) {
            count++;
        }
    }
    return count;
}

static int32_t food_keep_amount(int32_t food)
{
    int32_t population = city_player_current()->population;
    int32_t level;
    if (population < empire_trade_rules[EMPIRE_TRADE_RULE_FOOD_POPULATION_SMALL]) {
        level = 0;
    } else if (population < empire_trade_rules[EMPIRE_TRADE_RULE_FOOD_POPULATION_MEDIUM]) {
        level = 1;
    } else if (population < empire_trade_rules[EMPIRE_TRADE_RULE_FOOD_POPULATION_LARGE]) {
        level = 2;
    } else {
        level = 3;
    }
    return trade_recommends[(level * FOOD_LIST_ENTRIES) + food];
}

static int32_t material_keep_amount(int32_t resource)
{
    switch (resource) {
        case RESOURCE_REEDS:
        case RESOURCE_CLAY:
        case RESOURCE_GEMS:
        case RESOURCE_BARLEY:
        case RESOURCE_FLAX:
        case RESOURCE_STRAW:
        case RESOURCE_COPPER:
        case RESOURCE_TIMBER:
        case RESOURCE_HENNA:
            return (building_count_workshops_using_resource(resource) + 1) * empire_trade_rules[EMPIRE_TRADE_RULE_KEEP_WORKSHOP_AMOUNT];
        case RESOURCE_STONE:
        case RESOURCE_LIMESTONE:
        case RESOURCE_BRICKS:
        case RESOURCE_GRANITE:
        case RESOURCE_SANDSTONE:
            return monuments_unplaced_resource_need(resource) + empire_trade_rules[EMPIRE_TRADE_RULE_KEEP_MONUMENT_AMOUNT];
        default:
            return empire_trade_rules[EMPIRE_TRADE_RULE_KEEP_OTHER_AMOUNT];
    }
}

void empire_update_export_keep(void)
{
    int32_t amount[RESOURCE_MAX];
    memset(amount, 0, sizeof(amount));
    for (scenario_event *event = scenario_request_next_active(&scenario_events, 0); event;
        event = scenario_request_next_active(&scenario_events, event)) {
        if (scenario_request_is_waiting(event)) {
            int32_t resource = event->item.value;
            if (resource >= RESOURCE_MIN && resource < RESOURCE_MAX) {
                amount[resource] += event->amount.value * LOAD_UNITS;
            }
        }
    }
    for (int32_t resource = RESOURCE_MIN; resource < RESOURCE_MAX; resource++) {
        amount[resource] += (scenario_data.provisions_needed[resource] - scenario_data.provisions_sent[resource]) * LOAD_UNITS;
        int32_t base = 0;
        int32_t is_food = 0;
        for (int32_t food = 0; food < FOOD_LIST_ENTRIES; food++) {
            if ((empire_food_resources[food] == RESOURCE_GRAIN && resource < RESOURCE_MAX_FOOD) || empire_food_resources[food] == resource) {
                base = food_keep_amount(food);
                is_food = 1;
                break;
            }
        }
        if (!is_food) {
            base = material_keep_amount(resource);
        }
        trade_export_keep[resource] = city_resource_cap_amount(resource, base + amount[resource]);
    }
}

int32_t empire_object_city_image(int32_t object_id)
{
    const empire_object *o = &empire_map_objects[object_id];
    switch (o->city_type) {
        case EMPIRE_CITY_OURS:
            if (scenario_data.initial_rank != PLAYER_RANK_PHARAOH) {
                return image_group(IMAGE_GROUP_EMPIRE_CITIES);
            }
            return image_group(IMAGE_GROUP_EMPIRE_CITY_OURS);
        case EMPIRE_CITY_PHARAOH_TRADING:
            return image_group(IMAGE_GROUP_EMPIRE_CITY_OURS);
        case EMPIRE_CITY_PHARAOH:
            return image_group(IMAGE_GROUP_EMPIRE_CITY_DISTANT);
        case EMPIRE_CITY_EGYPTIAN_TRADING:
            return image_group(IMAGE_GROUP_EMPIRE_CITIES) + 6;
        case EMPIRE_CITY_EGYPTIAN:
            return image_group(IMAGE_GROUP_EMPIRE_FOREIGN) + 7;
        case EMPIRE_CITY_FOREIGN_TRADING:
            return empire_culture_table[o->name_id].image_offset + image_group(IMAGE_GROUP_EMPIRE_CITIES);
        case EMPIRE_CITY_FOREIGN:
            return empire_culture_table[o->name_id].image_offset + image_group(IMAGE_GROUP_EMPIRE_FOREIGN);
        default:
            return 0;
    }
}

void empire_object_set_images(void)
{
    int32_t max = image_group(IMAGE_GROUP_EMPIRE_ORNAMENTS_MAX);
    for (int32_t id = 0; id < EMPIRE_OBJECT_COUNT; id++) {
        empire_object *o = &empire_map_objects[id];
        if (!o->in_use) {
            continue;
        }
        if (o->type == EMPIRE_OBJECT_CITY) {
            o->image_id = (uint16_t) empire_object_city_image(id);
            max = image_group(IMAGE_GROUP_EMPIRE_ORNAMENTS_MAX);
        } else if (o->type == EMPIRE_OBJECT_ORNAMENT) {
            uint16_t image = (uint16_t) (o->image_offset + (int16_t) image_group(IMAGE_GROUP_EMPIRE_ORNAMENTS));
            o->image_id = image;
            if ((int32_t) image > max) {
                o->image_id = (uint16_t) max;
            }
        }
    }
}

void empire_object_set_city_images(void)
{
    for (int32_t id = 0; id < EMPIRE_OBJECT_COUNT; id++) {
        empire_object *o = &empire_map_objects[id];
        if (!o->in_use || o->type != EMPIRE_OBJECT_CITY) {
            continue;
        }
        uint16_t image = (uint16_t) empire_object_city_image(id);
        int32_t count = image_collection.image_count;
        o->image_id = image;
        const image_header *header = &image_collection.headers[(int32_t) image < count ? image : 0];
        o->width = header->width;
        o->height = header->height;
    }
}

int32_t empire_route_find_for_city(int32_t city_id, uint8_t *ours_first)
{
    int32_t ours = 0;
    const empire_object *o = &empire_map_objects[0];
    while (!o->in_use || o->type != EMPIRE_OBJECT_CITY || o->city_type != EMPIRE_CITY_OURS) {
        o++;
        ours++;
        if (o >= empire_map_objects + EMPIRE_OBJECT_COUNT) {
            return -1;
        }
    }
    int32_t theirs = empire_cities[city_id].map_object;
    for (int32_t id = 0; id < EMPIRE_ROUTES; id++) {
        empire_route *r = &empire_map_routes.routes[id];
        if (r->num_points <= 1) {
            r->in_use = 0;
        }
        if (r->in_use && (r->first_object == theirs || r->last_object == theirs) && (r->first_object == ours || r->last_object == ours)) {
            *ours_first = r->first_object == ours;
            return id;
        }
    }
    return -1;
}

void empire_update_yearly_trade_stats(void)
{
    city_player *player = city_player_current();
    player->hunting_lodges_working = building_count_industry_active[RESOURCE_GAMEMEAT];
    player->game_meat_last_month = player->game_meat_this_month;
    player->game_meat_this_month = 0;
    player->fishing_wharfs_working = building_count_industry_active[RESOURCE_FISH];
    player->fish_last_month = player->fish_this_month;
    player->fish_this_month = 0;
}

int32_t empire_route_travel_months(const empire_route *route)
{
    int32_t value = (route->path_length * (int32_t) route->step) / ROUTE_TRAVEL_DIVISOR;
    return value < 1 ? 1 : value;
}

static const city_message *shown_message(void)
{
    return &city_messages[city_message_shown_index];
}

static int16_t shown_message_city(void)
{
    return message_use_alternate ? shown_message()->phrases[9] : shown_message()->phrases[3];
}

static int16_t shown_message_resource(void)
{
    return message_use_alternate ? shown_message()->phrases[13] : shown_message()->phrases[5];
}

const char *message_token_player_rank(void)
{
    return lang_get_string(LANG_GROUP_PLAYER_RANKS, city_player_current()->player_rank + RANK_TEXT_FIRST);
}

const char *message_token_player_name(void)
{
    if (scenario_data.player_title_id != 0) {
        sprintf(token_buffer_title, fmt_two_strings, settings_data.player_name, lang_get_string(LANG_GROUP_PLAYER_INCARNATIONS, scenario_data.player_title_id));
        return token_buffer_title;
    }
    return settings_data.player_name;
}

const char *message_token_city_name(void)
{
    int32_t group = settings_data.classical_names ? LANG_GROUP_CITY_NAMES_CLASSICAL : LANG_GROUP_CITY_NAMES;
    return lang_get_string(group, empire_cities[shown_message_city()].name_id);
}

const char *message_token_amount(void)
{
    int32_t amount = message_use_alternate ? shown_message()->phrases[12] : shown_message()->phrases[4];
    int32_t resource = shown_message_resource();
    switch (resource) {
        case RESOURCE_STONE:
        case RESOURCE_LIMESTONE:
        case RESOURCE_GRANITE:
        case RESOURCE_SANDSTONE:
        case RESOURCE_MARBLE:
            break;
        default:
            if (resource != RESOURCE_CHARIOTS && resource != RESOURCE_WEAPONS && resource != RESOURCE_DEBEN) {
                amount *= LOAD_UNITS;
            }
            break;
    }
    sprintf(token_buffer_amount, "%d", amount);
    return token_buffer_amount;
}

const char *message_token_resource_name(void)
{
    return lang_get_string(LANG_GROUP_RESOURCE_NAMES, shown_message_resource() + RESOURCE_NAME_TEXT_FIRST);
}
