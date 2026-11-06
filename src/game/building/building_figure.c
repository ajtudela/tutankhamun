// Figure generation of every building type for one tick: service and entertainment walkers, priests, bazaar
// buyers, storage and industry carts, dock workers, boats, gatherers and the like, one function per type or family.
//
// Service buildings follow one pattern: problem overlay, road access, labor seeker, then a walker after a delay
// set by the worker percentage. Temple complexes lower the cost of stock-using walkers or double some carts; the
// shipwright repairs a queued boat or builds the boat the city needs.
//
// Two behaviours that look like slips are part of the game:
// - a few values live for the whole call (bazaars seen, the farm harvest flag, the storage choice), so a building
//   can read what an earlier one left: a grain farm after a harvest also sends straw;
// - a new boat without timber makes no progress, and its order is dropped when the city needs a fishing boat.

#include "building/building_figure.h"
#include "building/building.h"
#include "building/building_figure_helpers.h"
#include "building/building_image_group.h"
#include "building/building_industry.h"
#include "building/building_stocks.h"
#include "building/building_storage.h"
#include "building/building_type.h"
#include "building/shipwright.h"
#include "city/gods.h"
#include "city/labor.h"
#include "city/resource_type.h"
#include "core/calc.h"
#include "core/random.h"
#include "figure/boats.h"
#include "figure/construction_worker.h"
#include "figure/figure.h"
#include "figure/figure_action.h"
#include "figure/figure_core.h"
#include "figure/figure_type.h"
#include "figure/formation.h"
#include "figure/laborer.h"
#include "game/game_data.h"
#include "game/game_internal.h"
#include "game/tutorial.h"
#include "graphics/image_collection.h"
#include "io/io_data.h"
#include "map/building_tiles.h"
#include "map/map_data.h"
#include "map/natives.h"
#include "map/road_access.h"
#include "map/terrain.h"
#include "map/water.h"
#include <stdint.h>

int32_t house_walker_spawned;

enum building_figure_value {
    LABOR_SEEKER_HOUSES = 51,
    LABOR_SEEKER_HOUSES_STORAGE = 101,
    CART_START_WAIT_TICKS = 30,
};

static int32_t *priest_name_counter(int32_t god)
{
    return &figure_name_counter_table.by_slot[FIGURE_NAME_GROUP_COUNTER_BASE - PRIEST_NAME_GROUP_FIRST - god];
}

typedef struct {
    int32_t bazaars;
    uint8_t farm_harvested;
    uint8_t two_carts;
    uint8_t first_is_lower;
    int32_t choose_ok;
    int32_t resource1;
    int32_t resource2;
    int32_t percent1;
    int32_t percent2;
    int32_t destination1;
    int32_t destination2;
} generate_state;

static void mark_problem(building *b)
{
    if (b->houses_covered < 1) {
        b->show_on_problem_overlay = 2;
    }
}

static int32_t has_road_access(building *b)
{
    return map_has_road_access(b->x, b->y, b->size);
}

static void seek_labor(int32_t id, building *b, int32_t limit)
{
    if (b->houses_covered < limit) {
        generate_labor_seeker(id);
    }
}

static int32_t worker_percentage(building *b)
{
    return calc_percentage(b->num_workers, building_model_table[b->type].laborers);
}

static int32_t spawn_delay(int32_t pct, int32_t d100, int32_t d75, int32_t d50, int32_t d25, int32_t d1)
{
    if (pct >= 100) {
        return d100;
    } else if (pct >= 75) {
        return d75;
    } else if (pct >= 50) {
        return d50;
    } else if (pct >= 25) {
        return d25;
    } else if (pct >= 1) {
        return d1;
    }
    return -1;
}

static int32_t spawn_due(building *b, int32_t delay)
{
    uint8_t days = ++b->figure_spawn_delay;
    return days > (uint8_t) delay;
}

static int32_t walker_alive(building *b, int32_t id, int32_t type)
{
    int32_t figure_id = b->figure_id;
    if (figure_id > 0) {
        figure *f = figure_get(figure_id);
        if (f->state && (f->type == type || f->type == FIGURE_NONE) && f->building_id == id) {
            return 1;
        }
        b->figure_id = 0;
    }
    return 0;
}

static int32_t create_walker(building *b, int32_t id, int32_t type, int32_t action, int32_t roam)
{
    int32_t figure_id = figure_create(b->player, type, map_result_x, map_result_y, 0);
    b->figure_id = (int16_t) figure_id;
    figure *f = figure_get(figure_id);
    f->action_state = (uint8_t) action;
    f->building_id = (int16_t) id;
    if (roam) {
        figure_movement_init_roaming(figure_id);
    }
    return figure_id;
}

static void spawn_service_walker(int32_t id, building *b, int32_t type, int32_t labor_limit, const uint8_t *delays, int32_t action, int32_t roam)
{
    mark_problem(b);
    if (walker_alive(b, id, type) || !has_road_access(b)) {
        return;
    }
    seek_labor(id, b, labor_limit);
    int32_t delay = spawn_delay(worker_percentage(b), delays[0], delays[1], delays[2], delays[3], delays[4]);
    if (delay < 0) {
        return;
    }
    if (spawn_due(b, delay)) {
        b->figure_spawn_delay = 0;
        create_walker(b, id, type, action, roam);
    }
}

static const uint8_t delays_service[5] = { 3, 7, 15, 29, 44 };
static const uint8_t delays_fast[5] = { 0, 1, 3, 7, 15 };
static const uint8_t delays_education[5] = { 5, 10, 25, 50, 100 };

static void spawn_house_walker(int32_t id, building *b)
{
    if (!has_road_access(b)) {
        return;
    }
    uint8_t days = ++b->figure_spawn_delay;
    if (days > 40 && !house_walker_spawned) {
        b->figure_spawn_delay = 0;
        house_walker_spawned = 1;
        int32_t figure_id = figure_create(b->player, FIGURE_PATRICIAN, map_result_x, map_result_y, 4);
        figure *f = figure_get(figure_id);
        f->action_state = FIGURE_ACTION_ROAMER_ROAM;
        f->building_id = (int16_t) id;
        figure_movement_init_roaming(figure_id);
    }
}

static void create_entertainer(building *b, int32_t id, int32_t type, int16_t *slot)
{
    b->figure_spawn_delay = 0;
    int32_t figure_id = figure_create(b->player, type, map_result_x, map_result_y, 0);
    *slot = (int16_t) figure_id;
    figure *f = figure_get(figure_id);
    f->action_state = FIGURE_ACTION_ENTERTAINER_ROAMING;
    f->building_id = (int16_t) id;
    figure_movement_init_roaming(figure_id);
}

static void spawn_bandstand(int32_t id, building *b)
{
    mark_problem(b);
    if (building_main(id) != id || !has_road_access(b)) {
        return;
    }
    seek_labor(id, b, LABOR_SEEKER_HOUSES);
    if (b->data.entertainment.juggler_id && b->data.entertainment.musician_id) {
        return;
    }
    int32_t delay = spawn_delay(worker_percentage(b), 3, 7, 15, 29, 44);
    if (delay < 0 || !spawn_due(b, delay)) {
        return;
    }
    if (b->data.entertainment.show_days_left[0] && !b->data.entertainment.juggler_id) {
        create_entertainer(b, id, FIGURE_JUGGLER, &b->data.entertainment.juggler_id);
    } else if (b->data.entertainment.show_days_left[1] && !b->data.entertainment.musician_id) {
        create_entertainer(b, id, FIGURE_MUSICIAN, &b->data.entertainment.musician_id);
    }
}

static void spawn_booth(int32_t id, building *b)
{
    mark_problem(b);
    if (building_main(id) != id || walker_alive(b, id, FIGURE_JUGGLER) || !has_road_access(b)) {
        return;
    }
    seek_labor(id, b, LABOR_SEEKER_HOUSES);
    int32_t delay = spawn_delay(worker_percentage(b), 3, 7, 15, 29, 44);
    if (delay < 0) {
        return;
    }
    if (spawn_due(b, delay) && b->data.entertainment.show_days_left[0]) {
        b->figure_spawn_delay = 0;
        create_walker(b, id, FIGURE_JUGGLER, FIGURE_ACTION_ENTERTAINER_ROAMING, 1);
    }
}

static void spawn_senet_house(int32_t id, building *b)
{
    mark_problem(b);
    if (walker_alive(b, id, FIGURE_SENET_PLAYER) || !has_road_access(b)) {
        return;
    }
    seek_labor(id, b, LABOR_SEEKER_HOUSES);
    int32_t delay = spawn_delay(worker_percentage(b), 7, 15, 30, 50, 80);
    if (delay < 0) {
        return;
    }
    int32_t cost = building_model_table[BUILDING_SENET_HOUSE].walker_cost;
    if (spawn_due(b, delay) && b->loads_stored >= cost) {
        b->loads_stored -= (int16_t) cost;
        b->figure_spawn_delay = 0;
        create_walker(b, id, FIGURE_SENET_PLAYER, FIGURE_ACTION_ENTERTAINER_ROAMING, 1);
    }
}

static void spawn_pavilion(int32_t id, building *b)
{
    mark_problem(b);
    if (building_main(id) != id || !has_road_access(b)) {
        return;
    }
    seek_labor(id, b, LABOR_SEEKER_HOUSES);
    if (b->data.entertainment.juggler_id && b->data.entertainment.musician_id && b->data.entertainment.dancer_id) {
        return;
    }
    int32_t delay = spawn_delay(worker_percentage(b), 6, 12, 20, 40, 70);
    if (delay < 0 || !spawn_due(b, delay)) {
        return;
    }
    if (b->data.entertainment.show_days_left[0] && !b->data.entertainment.juggler_id) {
        create_entertainer(b, id, FIGURE_JUGGLER, &b->data.entertainment.juggler_id);
    }
    if (b->data.entertainment.show_days_left[1] && !b->data.entertainment.musician_id) {
        create_entertainer(b, id, FIGURE_MUSICIAN, &b->data.entertainment.musician_id);
    }
    if (b->data.entertainment.show_days_left[2] && !b->data.entertainment.dancer_id) {
        create_entertainer(b, id, FIGURE_DANCER, &b->data.entertainment.dancer_id);
    }
}

static void spawn_entertainment_school(int32_t id, building *b, int32_t type, const uint8_t *delays)
{
    mark_problem(b);
    if (!has_road_access(b)) {
        return;
    }
    seek_labor(id, b, LABOR_SEEKER_HOUSES);
    int32_t delay = spawn_delay(worker_percentage(b), delays[0], delays[1], delays[2], delays[3], delays[4]);
    if (delay < 0) {
        return;
    }
    if (spawn_due(b, delay)) {
        b->figure_spawn_delay = 0;
        create_walker(b, id, type, FIGURE_ACTION_ENTERTAINER_CREATED, 0);
    }
}

static int32_t temple_complex_blesses(int32_t god, int32_t upgrade)
{
    const city_player *player = city_player_current();
    return player->has_temple_complex && building_temple_god(player->temple_complex_building_id) == god &&
        building_temple_complex_has_upgrade(player->temple_complex_building_id, upgrade);
}

static int32_t walker_cost(int32_t type, int32_t god, int32_t upgrade, int32_t percentage)
{
    int32_t cost = building_model_table[type].walker_cost;
    if (temple_complex_blesses(god, upgrade)) {
        cost = calc_adjust_with_percentage(cost, percentage);
    }
    return cost;
}

static void spawn_stock_walker(int32_t id, building *b, int32_t type, const uint8_t *delays, int32_t god, int32_t upgrade, int32_t percentage)
{
    mark_problem(b);
    if (walker_alive(b, id, type) || !has_road_access(b)) {
        return;
    }
    seek_labor(id, b, LABOR_SEEKER_HOUSES);
    int32_t delay = spawn_delay(worker_percentage(b), delays[0], delays[1], delays[2], delays[3], delays[4]);
    if (delay < 0 || !spawn_due(b, delay)) {
        return;
    }
    int32_t cost = walker_cost(b->type, god, upgrade, percentage);
    if (cost > b->loads_stored) {
        return;
    }
    b->loads_stored -= (int16_t) cost;
    b->figure_spawn_delay = 0;
    if (type == FIGURE_TEACHER && !settings_data.is_custom_scenario && settings_data.mission_id == tutorial_briefing_tables.tutorial.entries[TUTORIAL_FLAG_SCRIBE].mission &&
        !tutorial_flags_a[TUTORIAL_FLAG_SCRIBE] && tutorial_flags_a[TUTORIAL_FLAG_HOUSES_DONE]) {
        scenario_start_flags[SCENARIO_START_FLAG_EMPIRE_LOCKED] = 0;
        tutorial_set_flag(TUTORIAL_FLAG_SCRIBE);
    }
    create_walker(b, id, type, FIGURE_ACTION_ROAMER_ROAM, 1);
}

static void spawn_tower(int32_t id, building *b)
{
    mark_problem(b);
    if (!has_road_access(b)) {
        return;
    }
    seek_labor(id, b, LABOR_SEEKER_HOUSES);
    if (b->num_workers <= 0) {
        return;
    }
    if (b->figure_id4 == 0 && b->figure_id != 0) {
        int32_t figure_id = figure_create(b->player, FIGURE_BALLISTA, b->x, b->y, 0);
        b->figure_id4 = (int16_t) figure_id;
        figure *f = figure_get(figure_id);
        f->building_id = (int16_t) id;
        f->action_state = FIGURE_ACTION_BALLISTA_READY;
    }
    int32_t figure_id = b->figure_id;
    if (figure_id > 0) {
        figure *f = figure_get(figure_id);
        if (f->state == 0 || (f->type != FIGURE_TOWER_SENTRY && f->type != FIGURE_NONE) || f->building_id != id) {
            b->figure_id = 0;
        }
    }
    if (b->figure_id < 1) {
        building_barracks_tower_sentry_request = 2;
    }
}

static void spawn_temple(int32_t id, building *b)
{
    if (b->prev_part_building_id != 0) {
        return;
    }
    mark_problem(b);
    if (walker_alive(b, id, FIGURE_PRIEST)) {
        return;
    }
    int32_t is_complex = b->type >= BUILDING_TEMPLE_COMPLEX_OSIRIS && b->type <= BUILDING_TEMPLE_COMPLEX_BAST;
    int32_t access = is_complex ? map_has_road_access_temple_complex(b->x, b->y, b->data.temple_complex.orientation) : has_road_access(b);
    if (!access) {
        return;
    }
    seek_labor(id, b, LABOR_SEEKER_HOUSES);
    int32_t pct = worker_percentage(b);
    int32_t delay;
    if (building_model_table[b->type].laborers < 1) {
        delay = 7;
    } else {
        delay = spawn_delay(pct, 3, 7, 10, 15, 20);
        if (delay < 0) {
            return;
        }
    }
    if (!spawn_due(b, delay)) {
        return;
    }
    b->figure_spawn_delay = 0;
    int32_t figure_id = create_walker(b, id, FIGURE_PRIEST, FIGURE_ACTION_ROAMER_ROAM, 0);
    figure *f = figure_get(figure_id);
    int32_t god = building_temple_god(id);
    if (god >= GOD_OSIRIS && god <= GOD_BAST) {
        int32_t *counter = priest_name_counter(god);
        int32_t n = *counter;
        f->name_id = (int16_t) n;
        n++;
        f->name_group = PRIEST_NAME_GROUP_FIRST + god;
        *counter = n;
        if (n >= PRIEST_NAME_COUNT) {
            *counter = 0;
        }
    }
    figure_movement_init_roaming(figure_id);
}

static int32_t bazaar_inventory_index(int32_t resource)
{
    switch (resource) {
        case RESOURCE_GRAIN:
            return 0;
        case RESOURCE_MEAT:
            return 1;
        case RESOURCE_LETTUCE:
            return 2;
        case RESOURCE_CHICKPEAS:
            return 3;
        case RESOURCE_POMEGRANATES:
            return 4;
        case RESOURCE_FIGS:
            return 5;
        case RESOURCE_FISH:
            return 6;
        case RESOURCE_GAMEMEAT:
            return 7;
        case RESOURCE_POTTERY:
            return 9;
        case RESOURCE_BEER:
            return 8;
        case RESOURCE_LINEN:
            return 10;
        case RESOURCE_LUXURY_GOODS:
            return 11;
        default:
            return -1;
    }
}

static void send_buyer(int32_t id, building *b, int32_t second, int32_t *items, uint8_t *routed, int16_t *slot)
{
    has_road_access(b);
    int32_t destination = building_market_get_storage_destination(id, second, items, routed);
    if (destination <= 0) {
        return;
    }
    int32_t figure_id = figure_create(b->player, FIGURE_MARKET_BUYER, map_result_x, map_result_y, 0);
    *slot = (int16_t) figure_id;
    figure *f = figure_get(figure_id);
    f->action_state = FIGURE_ACTION_MARKET_BUYER_TO_STORAGE;
    f->building_id = (int16_t) id;
    f->destination_building_id = (int16_t) destination;
    for (int32_t i = 0; i < 4; i++) {
        f->buyer_resources[i] = items[i];
        f->buyer_storage_ids[i] = 0;
        f->buyer_amounts_bought[i] = 0;
    }
    building *dest = building_get(destination);
    if (map_has_road_access(dest->x, dest->y, dest->size) || map_has_road_access(dest->x, dest->y, 3)) {
        f->destination_x = (int16_t) map_result_x;
        f->destination_y = (int16_t) map_result_y;
    } else {
        f->action_state = FIGURE_ACTION_MARKET_BUYER_RETURNING;
        f->destination_x = f->x;
        f->destination_y = f->y;
    }
}

static void check_buyer(int16_t *slot)
{
    figure *f = figure_get(*slot);
    if (f->state != 1 || (f->type != FIGURE_MARKET_BUYER && f->type != FIGURE_LABOR_SEEKER)) {
        *slot = 0;
    }
}

static void spawn_bazaar(int32_t id, building *b, generate_state *s)
{
    int32_t items[4] = { -1, -1, -1, -1 };
    s->bazaars++;
    uint8_t houses_in_range = building_bazaar_update_image(id);
    mark_problem(b);
    if (!has_road_access(b)) {
        return;
    }
    seek_labor(id, b, LABOR_SEEKER_HOUSES);
    int32_t delay = spawn_delay(worker_percentage(b), 2, 3, 4, 5, 10);
    if (delay < 0) {
        return;
    }
    int32_t goods = 0;
    for (int32_t i = 0; i < city_foods.count; i++) {
        int32_t resource = city_foods.list[i];
        if (resource != RESOURCE_NONE) {
            goods += b->data.market.inventory[bazaar_inventory_index(resource)];
        }
    }
    goods += b->data.market.inventory[9] + b->data.market.inventory[8] + b->data.market.inventory[10] + b->data.market.inventory[11];

    int32_t trader_alive = 0;
    int32_t figure_id = b->figure_id;
    if (figure_id >= 1) {
        figure *f = figure_get(figure_id);
        if (f->state && (f->type == FIGURE_MARKET_TRADER || f->type == FIGURE_NONE) && f->building_id == id) {
            trader_alive = 1;
        } else {
            b->figure_id = 0;
        }
    }
    if (trader_alive || goods == 0) {
        building_bazaar_update_supply(id, 10);
    } else {
        if (!spawn_due(b, delay)) {
            return;
        }
        b->figure_spawn_delay = 0;
        create_walker(b, id, FIGURE_MARKET_TRADER, FIGURE_ACTION_ROAMER_ROAM, 1);
    }

    uint8_t routed = 0;
    if (b->figure_id2 == 0) {
        if (game_time_day % 8 == s->bazaars % 8) {
            send_buyer(id, b, 0, items, &routed, &b->figure_id2);
        }
    } else {
        check_buyer(&b->figure_id2);
    }
    if (b->data.market.second_buyer_id == 0) {
        if (game_time_day % 8 == s->bazaars % 8) {
            send_buyer(id, b, 1, items, &routed, &b->data.market.second_buyer_id);
        }
    } else {
        check_buyer(&b->data.market.second_buyer_id);
    }
    if (houses_in_range) {
        int16_t *slot = &b->data.market.second_trader_id;
        if (*slot == 0 && goods != 0) {
            int32_t trader = figure_create(b->player, FIGURE_MARKET_TRADER, map_result_x, map_result_y, 0);
            *slot = (int16_t) trader;
            figure *f = figure_get(trader);
            f->action_state = FIGURE_ACTION_ROAMER_ROAM;
            f->building_id = (int16_t) id;
            figure_movement_init_roaming(trader);
        } else {
            figure *f = figure_get(*slot);
            if (f->state != 1 || (f->type != FIGURE_MARKET_TRADER && f->type != FIGURE_LABOR_SEEKER)) {
                *slot = 0;
            }
        }
    }
}

static int32_t storage_cart_alive(int16_t *slot, int32_t id)
{
    int32_t figure_id = *slot;
    if (figure_id < 1) {
        return 0;
    }
    figure *f = figure_get(figure_id);
    if (f->state == 0 || (f->type != FIGURE_STORAGE_YARD_CART && f->type != FIGURE_NONE) || f->building_id != id) {
        *slot = 0;
        return 0;
    }
    return 1;
}

static int32_t cart_capacity(building *b, int32_t resource)
{
    if (storage_get(b->storage_id)->resource_state[resource] != 3) {
        return 100;
    }
    switch (resource) {
        case RESOURCE_WEAPONS:
        case RESOURCE_BRICKS:
        case RESOURCE_STONE:
        case RESOURCE_LIMESTONE:
        case RESOURCE_GRANITE:
        case RESOURCE_CHARIOTS:
        case RESOURCE_SANDSTONE:
        case RESOURCE_MARBLE:
            return 100;
        default:
            return 400;
    }
}

static int32_t create_storage_cart(building *b)
{
    int32_t figure_id = figure_create(b->player, FIGURE_STORAGE_YARD_CART, b->road_access_x, b->road_access_y, 4);
    figure *f = figure_get(figure_id);
    f->action_state = FIGURE_ACTION_STORAGE_CART_INITIAL;
    f->resource_id = RESOURCE_NONE;
    return figure_id;
}

static void spawn_storage(int32_t id, building *b, generate_state *s, int32_t is_granary)
{
    s->resource1 = RESOURCE_NONE;
    s->percent1 = 100;
    s->percent2 = 100;
    s->resource2 = RESOURCE_NONE;
    if (is_granary) {
        s->destination1 = 0;
        s->destination2 = 0;
    }
    int32_t first_alive = storage_cart_alive(&b->figure_id, id);
    int32_t second_alive = storage_cart_alive(&b->data.storage_yard.second_cart_id, id);
    mark_problem(b);
    int32_t access;
    if (is_granary) {
        access = map_has_road_access_granary(b->x, b->y);
    } else {
        int32_t part = id;
        for (int32_t i = 0; i < 8; i++) {
            part = building_get(part)->next_part_building_id;
            if (part) {
                building_get(part)->show_on_problem_overlay = b->show_on_problem_overlay;
            }
        }
        access = map_has_road_access(b->x, b->y, b->size) || map_has_road_access(b->x, b->y, 3);
    }
    if (!access) {
        return;
    }
    seek_labor(id, b, LABOR_SEEKER_HOUSES_STORAGE);
    if (first_alive && second_alive) {
        return;
    }
    int32_t saved_x = map_result_x;
    int32_t saved_y = map_result_y;
    building_storage_wanted_resources(id, &s->resource1, &s->resource2, &s->percent1, &s->percent2, &s->destination1, &s->destination2);
    int32_t resource = is_granary ? building_granary_resource_to_send(id, s->resource1) : building_storageyard_resource_to_send(id, s->resource1);
    map_result_x = saved_x;
    map_result_y = saved_y;
    if (resource <= -1) {
        return;
    }
    if (resource != RESOURCE_NONE) {
        if (first_alive) {
            return;
        }
        int32_t figure_id = figure_create(b->player, FIGURE_STORAGE_YARD_CART, b->road_access_x, b->road_access_y, 4);
        figure *f = figure_get(figure_id);
        int32_t capacity = cart_capacity(b, resource);
        f->action_state = FIGURE_ACTION_STORAGE_CART_INITIAL;
        f->resource_id = (uint8_t) resource;
        if (is_granary) {
            int32_t stock = b->data.granary.resource_stored[resource];
            f->resource_amount = (int16_t) (capacity < stock ? capacity : stock);
        } else {
            int32_t stock = building_storageyard_get_amount(id, resource);
            f->resource_amount = (int16_t) (capacity < stock ? capacity : stock);
        }
        f->building_id = (int16_t) id;
        b->figure_id = (int16_t) figure_id;
        if (is_granary) {
            building_granary_remove_resource(city_current_player, id, f->resource_id, f->resource_amount);
        } else {
            building_storageyard_remove_resource(city_current_player, (int16_t) id, f->resource_id, f->resource_amount);
        }
        return;
    }
    int32_t count = storage_count_wanted(s->resource1, s->resource2);
    if (!count) {
        return;
    }
    storage_choose_wanted(s->percent1, s->percent2, count, &s->two_carts, &s->first_is_lower, &s->choose_ok);
    if (!s->choose_ok) {
        return;
    }
    if (!first_alive) {
        if (!s->destination1) {
            return;
        }
        int32_t figure_id = create_storage_cart(b);
        figure *f = figure_get(figure_id);
        b->figure_id = (int16_t) figure_id;
        f->collecting_item_id = (uint8_t) s->resource1;
        f->resource_amount = 0;
        f->building_id = (int16_t) id;
        return;
    }
    if (!s->two_carts) {
        return;
    }
    if (second_alive) {
        return;
    }
    if (s->first_is_lower ? !s->destination1 : !s->destination2) {
        return;
    }
    int32_t figure_id = create_storage_cart(b);
    figure *f = figure_get(figure_id);
    f->collecting_item_id = (uint8_t) (s->first_is_lower ? s->resource1 : s->resource2);
    f->resource_amount = 0;
    b->data.storage_yard.second_cart_id = (int16_t) figure_id;
    f->building_id = (int16_t) id;
}

static void spawn_shipwright(int32_t id, building *b)
{
    mark_problem(b);
    if (!has_road_access(b)) {
        return;
    }
    seek_labor(id, b, LABOR_SEEKER_HOUSES);
    int32_t order = b->data.shipyard.boat_order & BOAT_ORDER_KIND_MASK;
    if (order == 0) {
        int32_t type;
        if (shipwright_boat_ordered_by(id, &type)) {
            b->data.shipyard.boat_order = (uint16_t) (type | BOAT_ORDER_REPAIR);
        } else if (shipwright_boat_needed(&type) && ((type != FIGURE_WARSHIP && type != FIGURE_TRANSPORT_SHIP) || b->loads_stored >= 100)) {
            b->data.shipyard.boat_order = (uint16_t) (type | BOAT_ORDER_FOR_CITY);
        }
        return;
    }
    int32_t building_new;
    if (order == BOAT_ORDER_FOR_CITY) {
        building_new = 1;
    } else if (order == BOAT_ORDER_REPAIR) {
        building_new = 0;
    } else {
        b->data.shipyard.boat_order = 0;
        return;
    }
    int32_t pct = worker_percentage(b);
    int32_t progress;
    if (pct >= 100) {
        progress = 10;
    } else if (pct >= 75) {
        progress = 8;
    } else if (pct >= 50) {
        progress = 6;
    } else if (pct >= 25) {
        progress = 4;
    } else {
        progress = (pct > 0) ? 2 : 0;
    }
    if (!building_new) {
        figure *f = figure_get(b->data.shipyard.boat_order & BOAT_ORDER_TYPE_MASK);
        if (f->state == FIGURE_STATE_DEAD) {
            b->data.shipyard.boat_order = 0;
            return;
        }
        int16_t wood = b->loads_stored;
        if (wood < 1 && !cheat_allow_all) {
            return;
        }
        f->ship_hit_points += progress;
        if (!cheat_allow_all) {
            b->loads_stored = (int16_t) (wood - 1);
        }
        if (f->ship_hit_points >= f->ship_max_hit_points) {
            f->ship_hit_points = f->ship_max_hit_points;
            b->data.shipyard.boat_order = 0;
            f->action_state = 8;
        }
        return;
    }
    int32_t type = b->data.shipyard.boat_order & BOAT_ORDER_TYPE_MASK;
    int32_t destination = shipwright_boat_destination(type);
    if (destination == 0) {
        b->data.shipyard.boat_order = 0;
        b->data.shipyard.progress = 0;
        return;
    }
    if (b->loads_stored < 100 && !cheat_allow_all && type != FIGURE_FISHING_BOAT) {
        int32_t needed;
        progress = 0;
        if (shipwright_boat_needed(&needed) && needed == FIGURE_FISHING_BOAT) {
            b->data.shipyard.boat_order = 0;
            b->data.shipyard.progress = 0;
        }
    }
    b->data.shipyard.progress += (int16_t) progress;
    if (b->data.shipyard.progress < 160) {
        return;
    }
    b->data.shipyard.progress = 0;
    if (!map_water_tile_next_to(b->x, b->y, b->size)) {
        return;
    }
    int32_t figure_id = figure_create(b->player, type, map_result_x, map_result_y, 0);
    figure *f = figure_get(figure_id);
    if (type == FIGURE_FISHING_BOAT) {
        f->action_state = FIGURE_ACTION_FISHING_BOAT_CREATED;
        f->reserved_9 = 0;
        f->ship_hit_points = figure_properties[FIGURE_FISHING_BOAT].max_damage;
        f->ship_max_hit_points = figure_properties[FIGURE_FISHING_BOAT].max_damage;
    } else if (type == FIGURE_TRANSPORT_SHIP) {
        b->loads_stored -= 100;
        f->action_state = FIGURE_ACTION_TRANSPORT_SHIP_WAITING;
        boat_init_transport(figure_id);
    } else {
        b->loads_stored -= 100;
        f->action_state = FIGURE_ACTION_WARSHIP_WAITING;
        boat_init_warship(figure_id);
    }
    f->building_id = (int16_t) destination;
    building_get(destination)->data.dock.docked_boat_id = (int16_t) figure_id;
    b->data.shipyard.boat_order = 0;
}

static void spawn_dock(int32_t id, building *b)
{
    mark_problem(b);
    if (!has_road_access(b)) {
        return;
    }
    seek_labor(id, b, LABOR_SEEKER_HOUSES);
    int32_t pct = worker_percentage(b);
    int32_t max;
    if (pct < 1) {
        max = 0;
    } else if (pct < 50) {
        max = 1;
    } else {
        max = (pct > 74) + 2;
    }
    int32_t count = 0;
    for (int32_t i = 0; i < 3; i++) {
        int32_t figure_id = b->data.dock.docker_ids[i];
        if (figure_id) {
            figure *f = figure_get(figure_id);
            if (f->type == FIGURE_DOCKER && f->building_id == id) {
                count++;
            } else {
                b->data.dock.docker_ids[i] = 0;
            }
        }
    }
    if (count > max) {
        for (int32_t i = 2; i >= 0; i--) {
            if (b->data.dock.docker_ids[i]) {
                figure_get(b->data.dock.docker_ids[i])->state = FIGURE_STATE_DEAD;
                break;
            }
        }
    } else if (count < max) {
        int32_t figure_id = figure_create(b->player, FIGURE_DOCKER, map_result_x, map_result_y, 4);
        figure *f = figure_get(figure_id);
        f->action_state = FIGURE_ACTION_DOCKER_IDLE;
        f->building_id = (int16_t) id;
        for (int32_t i = 0; i < 3; i++) {
            if (b->data.dock.docker_ids[i] == 0) {
                b->data.dock.docker_ids[i] = (int16_t) figure_id;
                break;
            }
        }
    }
}

static void spawn_fishing_wharf(int32_t id, building *b)
{
    mark_problem(b);
    if (b->data.industry.worker_figure_id) {
        figure *f = figure_get(b->data.industry.worker_figure_id);
        if (f->state != 1 || f->type != FIGURE_FISHING_BOAT) {
            b->data.industry.worker_figure_id = 0;
        }
    }
    if (!has_road_access(b)) {
        return;
    }
    seek_labor(id, b, LABOR_SEEKER_HOUSES);
    if (walker_alive(b, id, FIGURE_CART_PUSHER)) {
        return;
    }
    if (b->figure_spawn_delay == 0) {
        return;
    }
    b->figure_spawn_delay = 0;
    b->data.industry.fish_caught = 0;
    city_player_current()->fish_this_month += 100;
    b->output_resource = RESOURCE_FISH;
    int32_t figure_id = figure_create(b->player, FIGURE_CART_PUSHER, map_result_x, map_result_y, 4);
    figure *f = figure_get(figure_id);
    f->action_state = FIGURE_ACTION_CART_INITIAL;
    f->resource_amount = 100;
    if (temple_complex_blesses(GOD_OSIRIS, 2)) {
        f->resource_amount = (int16_t) (f->resource_amount << 1);
    }
    f->resource_id = RESOURCE_FISH;
    b->figure_id = (int16_t) figure_id;
    f->building_id = (int16_t) id;
    f->wait_ticks = CART_START_WAIT_TICKS;
}

static void spawn_ferry(int32_t id, building *b)
{
    if (!has_road_access(b)) {
        return;
    }
    seek_labor(id, b, LABOR_SEEKER_HOUSES);
    int32_t pct = worker_percentage(b);
    int32_t linked = b->linked_building_id;
    building *other = building_get(linked);
    calc_percentage(other->num_workers, building_model_table[other->type].laborers);
    if (linked < 0 || pct == 0) {
        return;
    }
    int32_t delay;
    if (pct >= 100) {
        delay = 3;
    } else if (pct >= 75) {
        delay = 7;
    } else if (pct >= 50) {
        delay = 15;
    } else if (pct >= 25) {
        delay = 29;
    } else if (pct >= 1) {
        delay = -1;
    } else {
        return;
    }
    uint8_t days = ++b->figure_spawn_delay;
    if ((int32_t) days <= delay) {
        return;
    }
    b->figure_spawn_delay = 0;
    if (walker_alive(b, id, FIGURE_FERRY_BOAT)) {
        return;
    }
    if (!map_water_tile_next_to(b->x, b->y, b->size)) {
        return;
    }
    int32_t figure_id = figure_create(b->player, FIGURE_FERRY_BOAT, map_result_x, map_result_y, 0);
    int32_t path = ferry_path_alloc(figure_id);
    b->figure_id = (int16_t) figure_id;
    figure *f = figure_get(figure_id);
    f->ferry_path_index = path;
    f->action_state = FIGURE_ACTION_FERRY_BOAT_CREATED;
    for (int32_t i = 0; i < FERRY_PATH_PASSENGER_SLOTS; i++) {
        ferry_path_records[path].passengers[i] = -1;
    }
    f->building_id = (int16_t) id;
}

static void spawn_wharf(int32_t id, building *b, int32_t boat_type)
{
    mark_problem(b);
    if (b->data.dock.ship_id) {
        figure *f = figure_get(b->data.dock.ship_id);
        if (f->state != 1 || f->type != boat_type) {
            b->data.dock.ship_id = 0;
        }
    }
    if (has_road_access(b) && b->houses_covered < 51) {
        generate_labor_seeker(id);
    }
}

static void spawn_water_supply(int32_t id, building *b)
{
    water_supply_update_image(id);
    mark_problem(b);
    if (walker_alive(b, id, FIGURE_WATER_CARRIER)) {
        return;
    }
    if (!has_road_access(b) || !b->has_water_access) {
        return;
    }
    seek_labor(id, b, LABOR_SEEKER_HOUSES);
    int32_t pct = worker_percentage(b);
    if (map_grids.desirability[b->grid_offset] < 30) {
        pct /= 2;
    }
    int32_t delay = spawn_delay(pct, 1, 3, 7, 15, 29);
    if (delay < 0) {
        return;
    }
    if (spawn_due(b, delay)) {
        b->figure_spawn_delay = 0;
        create_walker(b, id, FIGURE_WATER_CARRIER, FIGURE_ACTION_ROAMER_ROAM, 1);
    }
}

static void spawn_mission_post(int32_t id, building *b)
{
    if (walker_alive(b, id, FIGURE_MISSIONARY)) {
        return;
    }
    city_player *player = city_player_current();
    if (!has_road_access(b) || player->population <= 0) {
        return;
    }
    uint8_t days = (uint8_t) (b->figure_spawn_delay + 1);
    player->mission_post_operational = 1;
    b->figure_spawn_delay = days;
    if (days > 1) {
        b->figure_spawn_delay = 0;
        create_walker(b, id, FIGURE_MISSIONARY, FIGURE_ACTION_ROAMER_ROAM, 1);
    }
}

static void spawn_tax_collector(int32_t id, building *b)
{
    mark_problem(b);
    if (walker_alive(b, id, FIGURE_TAX_COLLECTOR) || !has_road_access(b)) {
        return;
    }
    seek_labor(id, b, LABOR_SEEKER_HOUSES);
    int32_t delay = spawn_delay(worker_percentage(b), 0, 1, 3, 7, 15);
    if (delay < 0) {
        return;
    }
    building *palace = building_get(city_player_current()->palace_building_id);
    int32_t palace_pct = calc_percentage(palace->num_workers, building_model_table[palace->type].laborers);
    if (spawn_due(b, delay)) {
        b->figure_spawn_delay = 0;
        if ((b->type == BUILDING_TAX_COLLECTOR || b->type == BUILDING_TAX_COLLECTOR_UP) && palace_pct > 0) {
            create_walker(b, id, FIGURE_TAX_COLLECTOR, FIGURE_ACTION_SERVICE_CREATED, 0);
        }
    }
}

static void create_native(building *b, int32_t id, int32_t type)
{
    b->figure_spawn_delay = 0;
    create_walker(b, id, type, FIGURE_ACTION_NATIVE_CREATED, 0);
}

static void spawn_native_hut(int32_t id, building *b)
{
    map_grids.images[b->grid_offset] = (map_grids.random[b->grid_offset] & 1) + image_group(IMAGE_GROUP_NATIVE_HUT);
    if (walker_alive(b, id, FIGURE_NATIVE)) {
        return;
    }
    if (b->subtype > 0 && map_native_target_tile(b->x, b->y, b->size)) {
        uint8_t days = ++b->figure_spawn_delay;
        if (days > 4) {
            create_native(b, id, FIGURE_NATIVE);
        }
    }
}

static void spawn_native_meeting(int32_t id, building *b)
{
    map_building_tiles_add(id, b->x, b->y, 2, 2, image_group(IMAGE_GROUP_NATIVE_HUT) + 2, TERRAIN_BUILDING);
    if (city_player_current()->mission_post_operational <= 0) {
        return;
    }
    if (walker_alive(b, id, FIGURE_NATIVE_TRADER)) {
        return;
    }
    if (map_native_target_tile(b->x, b->y, b->size)) {
        uint8_t days = ++b->figure_spawn_delay;
        if (days > 8) {
            create_native(b, id, FIGURE_NATIVE_TRADER);
        }
    }
}

static void spawn_recruiter(int32_t id, building *b)
{
    mark_problem(b);
    if (!has_road_access(b)) {
        return;
    }
    seek_labor(id, b, LABOR_SEEKER_HOUSES_STORAGE);
    int32_t delay = spawn_delay(worker_percentage(b), 8, 12, 16, 32, 48);
    if (delay < 0) {
        return;
    }
    if (spawn_due(b, delay)) {
        b->figure_spawn_delay = 0;
        has_road_access(b);
        if (!building_barracks_create_tower_sentry(id)) {
            building_barracks_create_soldier(id);
        }
    }
}

static void spawn_zoo(int32_t id, building *b)
{
    mark_problem(b);
    if (walker_alive(b, id, FIGURE_ZOOKEEPER) || !has_road_access(b)) {
        return;
    }
    seek_labor(id, b, LABOR_SEEKER_HOUSES);
    int32_t delay = spawn_delay(worker_percentage(b), 7, 15, 30, 50, 80);
    if (delay < 0) {
        return;
    }
    int32_t cost = building_model_table[BUILDING_ZOO].walker_cost;
    if (spawn_due(b, delay) && b->loads_stored >= cost && b->second_input_stock >= cost) {
        b->loads_stored -= (int16_t) cost;
        b->second_input_stock -= (int16_t) cost;
        b->figure_spawn_delay = 0;
        create_walker(b, id, FIGURE_ZOOKEEPER, FIGURE_ACTION_ENTERTAINER_ROAMING, 1);
    }
}

static int32_t create_cart(building *b, int32_t id, int32_t resource, int32_t loads)
{
    int32_t figure_id = figure_create(b->player, FIGURE_CART_PUSHER, map_result_x, map_result_y, 4);
    b->figure_id = (int16_t) figure_id;
    figure *f = figure_get(figure_id);
    f->action_state = FIGURE_ACTION_CART_INITIAL;
    f->resource_id = (uint8_t) resource;
    f->building_id = (int16_t) id;
    f->wait_ticks = CART_START_WAIT_TICKS;
    f->resource_amount = (int16_t) loads;
    return figure_id;
}

static void spawn_industry(int32_t id, building *b, generate_state *s)
{
    mark_problem(b);
    if (!has_road_access(b)) {
        return;
    }
    seek_labor(id, b, LABOR_SEEKER_HOUSES);
    if (walker_alive(b, id, FIGURE_CART_PUSHER)) {
        return;
    }
    int32_t loads;
    if (!building_is_farm(id)) {
        if (!building_industry_has_produced_resource(id)) {
            return;
        }
        loads = 100;
    } else {
        loads = farm_harvest_loads(id);
        if (!loads) {
            return;
        }
        s->farm_harvested = 1;
    }
    building_industry_start_new_production(id);
    int32_t figure_id = figure_create(b->player, FIGURE_CART_PUSHER, map_result_x, map_result_y, 4);
    uint8_t resource = b->output_resource;
    b->figure_id = (int16_t) figure_id;
    figure *f = figure_get(figure_id);
    f->action_state = FIGURE_ACTION_CART_INITIAL;
    f->resource_amount = (int16_t) loads;
    f->resource_id = resource;
    f->building_id = (int16_t) id;
    f->wait_ticks = CART_START_WAIT_TICKS;
    if (s->farm_harvested && b->type == BUILDING_GRAIN_FARM && city_player_current()->mothballed[RESOURCE_STRAW] == 0) {
        int32_t second = figure_create(b->player, FIGURE_CART_PUSHER, map_result_x, map_result_y, 4);
        b->data.industry.second_cart_id = (int16_t) second;
        figure *g = figure_get(second);
        g->action_state = FIGURE_ACTION_CART_INITIAL;
        g->resource_amount = 100;
        g->resource_id = RESOURCE_STRAW;
        g->building_id = (int16_t) id;
        g->wait_ticks = CART_START_WAIT_TICKS;
    }
}

static void send_gatherers(building *b, int32_t id, int32_t type, int32_t action, int32_t max, int32_t wait_mask)
{
    for (;;) {
        int32_t figure_id = figure_create(b->player, type, map_result_x, map_result_y, 0);
        figure *f = figure_get(figure_id);
        f->action_state = (uint8_t) action;
        f->building_id = (int16_t) id;
        random_generate_next();
        uint8_t count = ++b->data.industry.worker_count;
        f->wait_ticks = wait_mask ? (int16_t) (random_generator_state.iv1 & wait_mask) : (int16_t) ((uint32_t) random_generator_state.random1_15bit % 30);
        if (count >= max) {
            break;
        }
    }
}

static void spawn_wood_cutters(int32_t id, building *b)
{
    mark_problem(b);
    if (!has_road_access(b)) {
        return;
    }
    seek_labor(id, b, LABOR_SEEKER_HOUSES);
    if (city_labor_industry_is_mothballed(id)) {
        return;
    }
    int32_t pct = worker_percentage(b);
    int32_t max;
    if (pct >= 100) {
        max = 3;
    } else if (pct >= 50) {
        max = 2;
    } else if (pct >= 1) {
        max = 1;
    } else {
        return;
    }
    if (!walker_alive(b, id, FIGURE_CART_PUSHER) && b->loads_stored > 99) {
        b->loads_stored -= 100;
        b->output_resource = RESOURCE_TIMBER;
        create_cart(b, id, RESOURCE_TIMBER, 100);
    }
    if (500 - b->loads_stored < max * 25) {
        max = (500 - b->loads_stored) / 25;
    }
    if (b->data.industry.worker_count > 5) {
        b->data.industry.worker_count = (uint8_t) building_count_figures_of(id, FIGURE_LUMBERJACK);
    }
    if (b->data.industry.worker_count < max && vegetation_growth_state.trees_reachable) {
        b->figure_spawn_delay = 0;
        if (b->data.industry.worker_count < max) {
            send_gatherers(b, id, FIGURE_LUMBERJACK, FIGURE_ACTION_LUMBERJACK_FIND_TREE, max, 0);
        }
    }
}

static void spawn_hunting_lodge(int32_t id, building *b)
{
    mark_problem(b);
    if (!has_road_access(b)) {
        return;
    }
    seek_labor(id, b, LABOR_SEEKER_HOUSES_STORAGE);
    int32_t pct = worker_percentage(b);
    int32_t max;
    if (pct >= 100) {
        max = 3;
    } else if (pct >= 75) {
        max = 2;
    } else if (pct >= 25) {
        max = 1;
    } else if (pct >= 1) {
        max = 0;
    } else {
        return;
    }
    if (!walker_alive(b, id, FIGURE_CART_PUSHER) && b->loads_stored > 0) {
        int32_t figure_id = figure_create(b->player, FIGURE_CART_PUSHER, map_result_x, map_result_y, 4);
        b->figure_id = (int16_t) figure_id;
        city_player_current()->game_meat_this_month += 100;
        figure *f = figure_get(figure_id);
        f->action_state = FIGURE_ACTION_CART_INITIAL;
        f->resource_id = RESOURCE_GAMEMEAT;
        f->building_id = (int16_t) id;
        f->wait_ticks = CART_START_WAIT_TICKS;
        f->resource_amount = 100;
        if (temple_complex_blesses(GOD_OSIRIS, 2)) {
            f->resource_amount = (int16_t) (f->resource_amount << 1);
        }
        b->loads_stored -= 100;
        b->output_resource = RESOURCE_GAMEMEAT;
    }
    if (500 - b->loads_stored < max * 100) {
        max = (500 - b->loads_stored) / 100;
    }
    if (b->data.industry.worker_count > 5) {
        b->data.industry.worker_count = (uint8_t) building_count_figures_of(id, FIGURE_HUNTER);
    }
    if (b->data.industry.worker_count < max) {
        b->figure_spawn_delay = 0;
        send_gatherers(b, id, FIGURE_HUNTER, FIGURE_ACTION_HUNTER_CREATED, max, 31);
    }
}

static void spawn_reed_gatherer(int32_t id, building *b)
{
    mark_problem(b);
    if (!has_road_access(b)) {
        return;
    }
    seek_labor(id, b, LABOR_SEEKER_HOUSES);
    if (city_labor_industry_is_mothballed(id)) {
        return;
    }
    int32_t pct = worker_percentage(b);
    int32_t max;
    if (pct >= 100) {
        max = 5;
    } else if (pct >= 75) {
        max = 4;
    } else if (pct >= 50) {
        max = 2;
    } else if (pct >= 1) {
        max = 1;
    } else {
        return;
    }
    if (!walker_alive(b, id, FIGURE_CART_PUSHER) && b->loads_stored > 50) {
        int32_t figure_id = figure_create(b->player, FIGURE_CART_PUSHER, map_result_x, map_result_y, 4);
        b->figure_id = (int16_t) figure_id;
        b->loads_stored -= 100;
        figure *f = figure_get(figure_id);
        f->action_state = FIGURE_ACTION_CART_INITIAL;
        f->resource_id = RESOURCE_REEDS;
        f->building_id = (int16_t) id;
        f->wait_ticks = CART_START_WAIT_TICKS;
        f->resource_amount = 100;
    }
    int32_t space = 500 - b->loads_stored;
    if (space < max * 50) {
        max = space / 50;
    }
    if (b->data.industry.worker_count > 5) {
        b->data.industry.worker_count = (uint8_t) building_count_figures_of(id, FIGURE_REED_GATHERER);
    }
    b->figure_spawn_delay = 0;
    if (vegetation_growth_state.reeds_reachable && b->data.industry.worker_count < max) {
        send_gatherers(b, id, FIGURE_REED_GATHERER, FIGURE_ACTION_REED_GATHERER_FIND_REEDS, max, 0);
    }
}

static void count_guild_workers(void)
{
    int32_t highest = building_highest_id;
    for (int32_t id = 1; id <= highest; id++) {
        int32_t type = building_get(id)->type;
        if (type == BUILDING_STONEMASONS_GUILD || type == BUILDING_BRICKLAYERS_GUILD || type == BUILDING_CARPENTERS_GUILD || type == BUILDING_ARTISANS_GUILD) {
            building_get(id)->data.industry.worker_count = 0;
        }
    }
    for (int32_t figure_id = 1; figure_id < MAX_FIGURES; figure_id++) {
        figure *f = figure_get(figure_id);
        if (!f->state) {
            continue;
        }
        int32_t guild;
        switch (f->type) {
            case FIGURE_STONEMASON:
                guild = BUILDING_STONEMASONS_GUILD;
                break;
            case FIGURE_BRICKLAYER:
                guild = BUILDING_BRICKLAYERS_GUILD;
                break;
            case FIGURE_CARPENTER:
                guild = BUILDING_CARPENTERS_GUILD;
                break;
            case FIGURE_TOMB_ARTISAN:
                guild = BUILDING_ARTISANS_GUILD;
                break;
            default:
                continue;
        }
        int32_t building_id = f->building_id;
        if (building_id <= 0 || building_id > highest) {
            continue;
        }
        building *b = building_get(building_id);
        uint8_t count = ++b->data.industry.worker_count;
        if (b->type == guild && count > 4) {
            f->action_state = FIGURE_ACTION_CORPSE;
            f->pyramid_walk_mode = 0;
            f->is_ghost = 1;
        }
    }
}

static void generate_for(int32_t id, building *b, generate_state *s)
{
    switch (b->type) {
        case BUILDING_HOUSE_COMMON_MANOR:
        case BUILDING_HOUSE_SPACIOUS_MANOR:
        case BUILDING_HOUSE_ELEGANT_MANOR:
        case BUILDING_HOUSE_STATELY_MANOR:
        case BUILDING_HOUSE_MODEST_ESTATE:
        case BUILDING_HOUSE_PALATIAL_ESTATE:
            spawn_house_walker(id, b);
            break;
        case BUILDING_BANDSTAND:
            spawn_bandstand(id, b);
            break;
        case BUILDING_BOOTH:
            spawn_booth(id, b);
            break;
        case BUILDING_SENET_HOUSE:
            spawn_senet_house(id, b);
            break;
        case BUILDING_PAVILION:
            spawn_pavilion(id, b);
            break;
        case BUILDING_CONSERVATORY:
            spawn_entertainment_school(id, b, FIGURE_MUSICIAN, delays_service);
            break;
        case BUILDING_DANCE_SCHOOL: {
            static const uint8_t delays[5] = { 5, 10, 20, 35, 60 };
            spawn_entertainment_school(id, b, FIGURE_DANCER, delays);
            break;
        }
        case BUILDING_JUGGLER_SCHOOL:
            spawn_entertainment_school(id, b, FIGURE_JUGGLER, delays_service);
            break;
        case BUILDING_BULLFIGHT_SCHOOL: {
            static const uint8_t delays[5] = { 7, 15, 30, 60, 90 };
            spawn_entertainment_school(id, b, FIGURE_SENET_PLAYER, delays);
            break;
        }
        case BUILDING_APOTHECARY:
            spawn_service_walker(id, b, FIGURE_HERBALIST, 51, delays_service, FIGURE_ACTION_ROAMER_ROAM, 1);
            break;
        case BUILDING_MORTUARY:
            spawn_stock_walker(id, b, FIGURE_EMBALMER, delays_service, GOD_SETH, 1, building_model_table[BUILDING_TEMPLE_COMPLEX_SETH].walker_cost);
            break;
        case BUILDING_DENTIST:
            spawn_service_walker(id, b, FIGURE_DENTIST, 51, delays_service, FIGURE_ACTION_ROAMER_ROAM, 1);
            break;
        case BUILDING_SCRIBAL_SCHOOL:
            spawn_stock_walker(id, b, FIGURE_TEACHER, delays_education, GOD_PTAH, 2, building_model_table[BUILDING_TEMPLE_COMPLEX_PTAH].walker_cost);
            break;
        case BUILDING_LIBRARY:
            spawn_stock_walker(id, b, FIGURE_LIBRARIAN, delays_education, GOD_PTAH, 2, building_model_table[BUILDING_TEMPLE_COMPLEX_PTAH].walker_cost);
            break;
        case BUILDING_POLICE_STATION:
            spawn_service_walker(id, b, FIGURE_CONSTABLE, 101, delays_fast, FIGURE_ACTION_SERVICE_CREATED, 0);
            break;
        case BUILDING_FORT:
            formation_legion_update_recruit_type(id);
            break;
        case BUILDING_MUD_TOWER:
        case BUILDING_BRICK_TOWER:
        case BUILDING_TOWER:
            spawn_tower(id, b);
            break;
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
            spawn_temple(id, b);
            break;
        case BUILDING_BAZAAR:
        case BUILDING_BAZAAR_UP:
            spawn_bazaar(id, b, s);
            break;
        case BUILDING_GRANARY:
        case BUILDING_GRANARY_UP:
            spawn_storage(id, b, s, 1);
            break;
        case BUILDING_STORAGE_YARD:
        case BUILDING_STORAGE_YARD_UP:
            spawn_storage(id, b, s, 0);
            break;
        case BUILDING_SHIPWRIGHT:
            spawn_shipwright(id, b);
            break;
        case BUILDING_DOCK:
        case BUILDING_DOCK_UP:
            spawn_dock(id, b);
            break;
        case BUILDING_FISHING_WHARF:
            spawn_fishing_wharf(id, b);
            break;
        case BUILDING_MISSION_POST:
            spawn_mission_post(id, b);
            break;
        case BUILDING_ARCHITECT_POST:
            spawn_service_walker(id, b, FIGURE_ARCHITECT, 101, delays_fast, FIGURE_ACTION_SERVICE_CREATED, 0);
            break;
        case BUILDING_VILLAGE_PALACE:
        case BUILDING_TOWN_PALACE:
        case BUILDING_TAX_COLLECTOR:
        case BUILDING_TAX_COLLECTOR_UP:
        case BUILDING_VILLAGE_PALACE_UP:
        case BUILDING_TOWN_PALACE_UP:
        case BUILDING_CITY_PALACE:
            spawn_tax_collector(id, b);
            break;
        case BUILDING_NATIVE_HUT:
            spawn_native_hut(id, b);
            break;
        case BUILDING_NATIVE_MEETING:
            spawn_native_meeting(id, b);
            break;
        case BUILDING_WATER_LIFT:
        case BUILDING_MILITARY_ACADEMY:
        case BUILDING_MILITARY_ACADEMY_2:
        case BUILDING_MILITARY_ACADEMY_3:
            mark_problem(b);
            if (has_road_access(b) && b->houses_covered < 101) {
                generate_labor_seeker(id);
            }
            break;
        case BUILDING_NATIVE_CROPS:
            map_native_crops_update(id);
            break;
        case BUILDING_RECRUITER:
            spawn_recruiter(id, b);
            break;
        case BUILDING_BARLEY_FARM:
        case BUILDING_FLAX_FARM:
        case BUILDING_GRAIN_FARM:
        case BUILDING_LETTUCE_FARM:
        case BUILDING_POMEGRANATES_FARM:
        case BUILDING_CHICKPEAS_FARM:
        case BUILDING_STONE_QUARRY:
        case BUILDING_LIMESTONE_QUARRY:
        case BUILDING_CLAY_PIT:
        case BUILDING_BREWERY:
        case BUILDING_WEAVER:
        case BUILDING_WEAPONSMITH:
        case BUILDING_JEWELER:
        case BUILDING_POTTER:
        case BUILDING_GOLD_MINE:
        case BUILDING_GEMSTONE_MINE:
        case BUILDING_CATTLE_RANCH:
        case BUILDING_FIG_FARM:
        case BUILDING_PAPYRUS_MAKER:
        case BUILDING_BRICKWORKS:
        case BUILDING_CHARIOT_MAKER:
        case BUILDING_GRANITE_QUARRY:
        case BUILDING_COPPER_MINE:
        case BUILDING_SANDSTONE_QUARRY:
        case BUILDING_HENNA_FARM:
        case BUILDING_LAMP_MAKER:
        case BUILDING_PAINT_MAKER:
            spawn_industry(id, b, s);
            break;
        case BUILDING_WOOD_CUTTER:
            spawn_wood_cutters(id, b);
            break;
        case BUILDING_HUNTING_LODGE:
            spawn_hunting_lodge(id, b);
            break;
        case BUILDING_FERRY_LANDING:
            spawn_ferry(id, b);
            break;
        case BUILDING_FIREHOUSE:
            spawn_service_walker(id, b, FIGURE_FIREMAN, 101, delays_fast, FIGURE_ACTION_SERVICE_CREATED, 0);
            break;
        case BUILDING_CARPENTERS_GUILD:
            building_carpenters_guild_spawn_figure(id);
            break;
        case BUILDING_BRICKLAYERS_GUILD:
            building_bricklayers_guild_spawn_figure(id);
            break;
        case BUILDING_STONEMASONS_GUILD:
            building_stonemasons_guild_spawn_figure(id);
            break;
        case BUILDING_ARTISANS_GUILD:
            building_artisans_guild_spawn_figure(id);
            break;
        case BUILDING_WATER_SUPPLY:
            spawn_water_supply(id, b);
            break;
        case BUILDING_TRANSPORT_WHARF:
            spawn_wharf(id, b, FIGURE_TRANSPORT_SHIP);
            break;
        case BUILDING_WARSHIP_WHARF:
            spawn_wharf(id, b, FIGURE_WARSHIP);
            break;
        case BUILDING_COURTHOUSE:
            spawn_service_walker(id, b, FIGURE_MAGISTRATE, 101, delays_fast, FIGURE_ACTION_SERVICE_CREATED, 0);
            break;
        case BUILDING_REED_GATHERER:
            spawn_reed_gatherer(id, b);
            break;
        case BUILDING_WORK_CAMP:
            building_work_camp_spawn_figure(&floods_data, id);
            break;
        case BUILDING_PHYSICIAN:
            spawn_service_walker(id, b, FIGURE_PHYSICIAN, 51, delays_service, FIGURE_ACTION_ROAMER_ROAM, 1);
            break;
        case BUILDING_ZOO:
            spawn_zoo(id, b);
            break;
        default:
            break;
    }
}

void building_figure_generate(void)
{
    generate_state s = { 0 };
    s.percent1 = 100;
    s.percent2 = 100;
    house_walker_spawned = 0;
    osiris_double_yield_used = 0;
    if (building_barracks_tower_sentry_request > 0) {
        building_barracks_tower_sentry_request--;
    }
    count_guild_workers();
    for (int32_t id = 1; id <= building_highest_id; id++) {
        building *b = building_get(id);
        if (b->state != BUILDING_STATE_IN_USE) {
            continue;
        }
        if (b->type != BUILDING_STORAGE_ROOM) {
            b->show_on_problem_overlay = 0;
        }
        generate_for(id, b, &s);
    }
    if (osiris_double_yield_used) {
        city_player_current()->osiris_double_farm_yield = 0;
    }
}
