// Figure phrases (the counterpart of Julius' figure/phrase.c): what a figure says in the figure info window, its speech
/// sound, and the window's city problem button. Most types say a phrase of their own by their state; the other walkers
// talk about the city's biggest problem, picked by weight. The speech sounds per type come from a config file.
//
// The phrase choice depends on these details:
// - the problem pick masks random1_7bit with the sum of the weights instead of taking it modulo;
// - the problem button clears the weight at the signed phrase id, -1 and 8 included (bytes next to the weights);
// - min_max_seen is a byte, so the phrases for values of 500 and more never show;
// - a figure type past the speech table says nothing.

#include "figure/figure_phrase.h"
#include "building/building.h"
#include "building/building_data.h"
#include "building/building_figure_helpers.h"
#include "building/building_type.h"
#include "building/temple_complex.h"
#include "city/gods.h"
#include "city/labor.h"
#include "city/sentiment.h"
#include "core/calc.h"
#include "empire/empire_data.h"
#include "figure/animals.h"
#include "figure/figure.h"
#include "figure/figure_action.h"
#include "figure/figure_data.h"
#include "figure/figure_type.h"
#include "figure/laborer.h"
#include "figure/trade.h"
#include "map/map_data.h"
#include "scenario/event_manager.h"
#include "scenario/scenario_climate.h"
#include "scenario/scenario_internal.h"
#include "sound/music.h"
#include "sound/sound_play.h"
#include "window/window_data.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

int32_t phrase_sound_id;

int32_t figure_speech_sounds[FIGURE_TYPE_COUNT] = {
    [FIGURE_NONE] = FIGURE_SPEECH_NONE,
    [FIGURE_IMMIGRANT] = 202,
    [FIGURE_EMIGRANT] = 203,
    [FIGURE_HOMELESS] = 204,
    [FIGURE_CART_PUSHER] = 222,
    [FIGURE_LABOR_SEEKER] = 213,
    [FIGURE_EXPLOSION] = FIGURE_SPEECH_NONE,
    [FIGURE_TAX_COLLECTOR] = 245,
    [FIGURE_ARCHITECT] = 243,
    [FIGURE_STORAGE_YARD_CART] = 222,
    [FIGURE_FIREMAN] = 242,
    [FIGURE_ARCHER] = FIGURE_SPEECH_NONE,
    [FIGURE_INFANTRY] = FIGURE_SPEECH_NONE,
    [FIGURE_CHARIOTEER] = FIGURE_SPEECH_NONE,
    [FIGURE_STANDARD_BEARER] = FIGURE_SPEECH_NONE,
    [FIGURE_JUGGLER] = 226,
    [FIGURE_MUSICIAN] = 227,
    [FIGURE_DANCER] = 228,
    [FIGURE_SENET_PLAYER] = 229,
    [FIGURE_TRADE_CARAVAN] = 225,
    [FIGURE_TRADE_SHIP] = 224,
    [FIGURE_TRADE_CARAVAN_DONKEY] = 225,
    [FIGURE_PROTESTER] = FIGURE_SPEECH_NONE,
    [FIGURE_CRIMINAL] = 205,
    [FIGURE_TOMB_ROBBER] = 248,
    [FIGURE_FISHING_BOAT] = 211,
    [FIGURE_MARKET_TRADER] = 220,
    [FIGURE_PRIEST] = FIGURE_SPEECH_BY_TEMPLE_GOD,
    [FIGURE_BUBBLES] = FIGURE_SPEECH_NONE,
    [FIGURE_TEACHER] = 235,
    [FIGURE_LIBRARIAN] = 236,
    [FIGURE_DENTIST] = 238,
    [FIGURE_PHYSICIAN] = 239,
    [FIGURE_HERBALIST] = 241,
    [FIGURE_EMBALMER] = 240,
    [FIGURE_WORKER] = 212,
    [FIGURE_MAP_FLAG] = FIGURE_SPEECH_NONE,
    [FIGURE_FLOTSAM] = FIGURE_SPEECH_NONE,
    [FIGURE_DOCKER] = 222,
    [FIGURE_MARKET_BUYER] = 219,
    [FIGURE_PATRICIAN] = 207,
    [FIGURE_NATIVE] = FIGURE_SPEECH_NONE,
    [FIGURE_TOWER_SENTRY] = 252,
    [FIGURE_ENEMY_ARCHER] = FIGURE_SPEECH_NONE,
    [FIGURE_ENEMY_INFANTRY] = FIGURE_SPEECH_NONE,
    [FIGURE_ENEMY_CHARIOT] = FIGURE_SPEECH_NONE,
    [FIGURE_ENEMY_CAMEL] = FIGURE_SPEECH_NONE,
    [FIGURE_ENEMY_ELEPHANT] = FIGURE_SPEECH_NONE,
    [FIGURE_ENEMY_BARBARIAN_CHARIOT] = FIGURE_SPEECH_NONE,
    [FIGURE_ENEMY_BARBARIAN_LIGHT] = FIGURE_SPEECH_NONE,
    [FIGURE_ENEMY_BARBARIAN_HEAVY] = FIGURE_SPEECH_NONE,
    [FIGURE_ENEMY_BARBARIAN_ARCHER] = FIGURE_SPEECH_NONE,
    [FIGURE_ENEMY_BARBARIAN_MOUNTED] = FIGURE_SPEECH_NONE,
    [FIGURE_ENEMY_BARBARIAN_HEAVY_2] = FIGURE_SPEECH_NONE,
    [FIGURE_ENEMY_EGYPTIAN_INFANTRY] = FIGURE_SPEECH_NONE,
    [FIGURE_ENEMY_EGYPTIAN_ARCHER] = FIGURE_SPEECH_NONE,
    [FIGURE_ENEMY_EGYPTIAN_CHARIOT] = FIGURE_SPEECH_NONE,
    [FIGURE_KINGDOM_SOLDIER] = FIGURE_SPEECH_NONE,
    [FIGURE_NATIVE_TRADER] = FIGURE_SPEECH_NONE,
    [FIGURE_ARROW] = FIGURE_SPEECH_NONE,
    [FIGURE_JAVELIN] = FIGURE_SPEECH_NONE,
    [FIGURE_BOLT] = FIGURE_SPEECH_NONE,
    [FIGURE_BALLISTA] = FIGURE_SPEECH_NONE,
    [FIGURE_CREATURE] = FIGURE_SPEECH_NONE,
    [FIGURE_MISSIONARY] = FIGURE_SPEECH_NONE,
    [FIGURE_SEAGULLS] = FIGURE_SPEECH_NONE,
    [FIGURE_DELIVERY_BOY] = 221,
    [FIGURE_SHIPWRECK] = FIGURE_SPEECH_NONE,
    [FIGURE_BIRDS] = FIGURE_SPEECH_NONE,
    [FIGURE_OSTRICH] = FIGURE_SPEECH_NONE,
    [FIGURE_ANTELOPE] = FIGURE_SPEECH_NONE,
    [FIGURE_SPEAR] = FIGURE_SPEECH_NONE,
    [FIGURE_CHARIOT_RACER] = FIGURE_SPEECH_NONE,
    [FIGURE_HUNTER] = FIGURE_SPEECH_BY_CLIMATE,
    [FIGURE_HUNTER_SPEAR] = FIGURE_SPEECH_NONE,
    [FIGURE_LUMBERJACK] = 214,
    [FIGURE_FERRY_BOAT] = FIGURE_SPEECH_NONE,
    [FIGURE_TRANSPORT_SHIP] = 250,
    [FIGURE_WARSHIP] = 249,
    [FIGURE_CARPENTER] = 216,
    [FIGURE_BRICKLAYER] = 217,
    [FIGURE_STONEMASON] = 218,
    [FIGURE_CROCODILE] = FIGURE_SPEECH_NONE,
    [FIGURE_HYENA] = FIGURE_SPEECH_NONE,
    [FIGURE_HIPPO] = FIGURE_SPEECH_NONE,
    [FIGURE_LABORER] = 212,
    [FIGURE_SLED] = FIGURE_SPEECH_NONE,
    [FIGURE_WATER_CARRIER] = 237,
    [FIGURE_CONSTABLE] = 244,
    [FIGURE_MAGISTRATE] = 246,
    [FIGURE_REED_GATHERER] = 215,
    [FIGURE_FESTIVAL_PRIEST] = FIGURE_SPEECH_BY_SOUND_TYPE,
    [FIGURE_ENEMY_TRANSPORT_SHIP] = FIGURE_SPEECH_NONE,
    [FIGURE_ENEMY_WARSHIP] = FIGURE_SPEECH_NONE,
    [FIGURE_FUNERAL_WALKER] = FIGURE_SPEECH_NONE,
    [FIGURE_FISHING_SPOT] = FIGURE_SPEECH_NONE,
    [FIGURE_SLED_PULLER] = 212,
    [FIGURE_SHOWMAN] = FIGURE_SPEECH_NONE,
    [FIGURE_PLAGUED_CITIZEN] = 206,
    [FIGURE_BEDOUIN] = FIGURE_SPEECH_NONE,
    [FIGURE_ENEMY_EGYPTIAN_WARSHIP] = FIGURE_SPEECH_NONE,
    [FIGURE_ENEMY_EGYPTIAN_TRANSPORT_SHIP] = FIGURE_SPEECH_NONE,
    [FIGURE_ASP] = FIGURE_SPEECH_NONE,
    [FIGURE_LION] = FIGURE_SPEECH_NONE,
    [FIGURE_SCORPION] = FIGURE_SPEECH_NONE,
    [FIGURE_ZOOKEEPER] = 309,
    [FIGURE_FROG] = FIGURE_SPEECH_NONE,
    [FIGURE_LOCUST] = FIGURE_SPEECH_NONE,
    [FIGURE_TOMB_ARTISAN] = 247,
    [FIGURE_MUMMY] = FIGURE_SPEECH_NONE,
};

static int32_t trader_traded_nothing(int32_t trader_id)
{
    const empire_trader *trader = &empire_map_routes.traders[trader_id];
    return trader->sold_amount == 0 && trader->bought_amount == 0;
}

static int32_t figure_speech_sound(int32_t type)
{
    return ((uint32_t) type < FIGURE_TYPE_COUNT) ? figure_speech_sounds[type] : FIGURE_SPEECH_NONE;
}

static int32_t any_event_running(void)
{
    for (scenario_event *e = scenario_invasion_next(&scenario_events, 0, 0); e; e = scenario_invasion_next(&scenario_events, e, 0)) {
        if (scenario_event_is_running(e)) {
            return 1;
        }
    }
    return 0;
}

int32_t figure_phrase_city_weights(uint8_t *weights, uint8_t *none)
{
    city_player *player = city_player_current();
    memset(weights, 0, CITY_PROBLEM_COUNT);
    int32_t flagged_event = 0;
    int32_t event_running = 0;
    int32_t weight = 0;
    int32_t total = 0;
    for (scenario_event *e = scenario_invasion_next(&scenario_events, 0, 0); e; e = scenario_invasion_next(&scenario_events, e, 0)) {
        uint32_t flags = e->flags;
        if ((flags >> 4) & 1) {
            if ((flags >> 2) & 1) {
                flagged_event = 1;
            }
            break;
        }
    }
    event_running = any_event_running();
    for (int32_t i = 0; i < CITY_PROBLEM_COUNT; i++) {
        int32_t value;
        switch (i) {
            case CITY_PROBLEM_HEALTH:
                value = player->health;
                if (value < 20) {
                    weight = 8;
                    *none = 0;
                } else if (value < 30) {
                    weight = 6;
                    *none = 0;
                } else if (value < 40) {
                    weight = 4;
                    *none = 0;
                } else {
                    weight = 1;
                }
                break;
            case CITY_PROBLEM_FOOD:
                value = player->food_supply_months;
                if (value < 1) {
                    weight = 6;
                    *none = 0;
                } else if (value < 2) {
                    weight = 4;
                    *none = 0;
                } else {
                    weight = 1;
                }
                break;
            case CITY_PROBLEM_INVASION:
                if (!event_running) {
                    weight = 1;
                } else if (formation_num_legions == 0 && player->total_warships == 0) {
                    weight = 8;
                    *none = 0;
                }
                break;
            case CITY_PROBLEM_WORKER_SHORTAGE: {
                int32_t unemployed = 0;
                int32_t workers = 0;
                for (int32_t c = 0; c < LABOR_CATEGORY_COUNT; c++) {
                    unemployed += player->labor_categories[c].workers_allocated;
                }
                for (int32_t c = 0; c < LABOR_CATEGORY_COUNT; c++) {
                    workers += player->labor_categories[c].workers_needed;
                }
                if (workers == 0) {
                    weight = 1;
                    break;
                }
                value = 100 - ((unemployed * 100) / workers);
                if (value > 20) {
                    weight = 8;
                    *none = 0;
                } else if (value > 10) {
                    weight = 6;
                    *none = 0;
                } else if (value > 5) {
                    weight = 4;
                    *none = 0;
                } else {
                    weight = 1;
                }
                break;
            }
            case CITY_PROBLEM_GODS: {
                int32_t lowest = 100;
                for (int32_t c = 0; c < GOD_COUNT; c++) {
                    int32_t v = player->god_mood[c];
                    if (v < lowest) {
                        lowest = v;
                    }
                }
                if (lowest <= 20) {
                    weight = 6;
                    *none = 0;
                } else if (lowest <= 30) {
                    weight = 4;
                    *none = 0;
                } else {
                    weight = 1;
                }
                break;
            }
            case CITY_PROBLEM_KINGDOM:
                if (flagged_event) {
                    weight = 0;
                    break;
                }
                value = player->rating_kingdom;
                if (value <= 0) {
                    weight = 8;
                    *none = 0;
                } else if (value < 20) {
                    weight = 6;
                    *none = 0;
                } else if (value < 40) {
                    weight = 4;
                    *none = 0;
                } else {
                    weight = 1;
                }
                break;
            case CITY_PROBLEM_UNEMPLOYMENT: {
                int32_t population = player->population;
                if (population == 0) {
                    weight = 1;
                    break;
                }
                value = (player->workers_unemployed * 100) / population;
                if (value > 17) {
                    weight = 8;
                    *none = 0;
                } else if (value > 10) {
                    weight = 6;
                    *none = 0;
                } else if (value > 5) {
                    weight = 4;
                    *none = 0;
                } else {
                    weight = 1;
                }
                break;
            }
            case CITY_PROBLEM_ENTERTAINMENT:
                value = player->average_entertainment;
                if (value <= 0) {
                    weight = 6;
                    *none = 0;
                } else if (value < 20) {
                    weight = 4;
                    *none = 0;
                } else {
                    weight = 1;
                }
                break;
        }
        total += weight;
        weights[i] = (uint8_t) weight;
    }
    if (weights[CITY_PROBLEM_WORKER_SHORTAGE] == 1) {
        if (weights[CITY_PROBLEM_UNEMPLOYMENT] >= 1) {
            weights[CITY_PROBLEM_WORKER_SHORTAGE] = 0;
            return total - 1;
        }
    } else if (weights[CITY_PROBLEM_WORKER_SHORTAGE] > 1 && weights[CITY_PROBLEM_UNEMPLOYMENT] == 1) {
        weights[CITY_PROBLEM_UNEMPLOYMENT] = 0;
        total--;
    }
    return total;
}

int32_t figure_phrase_pick_city_problem(void)
{
    uint8_t none = 1;
    int32_t total = figure_phrase_city_weights(phrase_weight_block.phrase_weights, &none);
    int32_t sum = 0;
    for (int32_t i = 0; i < CITY_PROBLEM_COUNT; i++) {
        sum += (phrase_weight_block.phrase_weights[i] * 100) / total;
    }
    int32_t r = random_generator_state.random1_7bit & sum;
    if (none) {
        return CITY_PROBLEM_NONE;
    }
    int32_t acc = 0;
    int32_t i;
    for (i = 0; i < CITY_PROBLEM_COUNT; i++) {
        acc += (phrase_weight_block.phrase_weights[i] * 100) / total;
        if (r <= acc) {
            break;
        }
    }
    if (i == CITY_PROBLEM_COUNT && r > acc) {
        i = CITY_PROBLEM_COUNT - 1;
    }
    return (phrase_weight_block.phrase_weights[i] == 1) ? CITY_PROBLEM_NOTHING_TO_COMPLAIN : i;
}

int32_t figure_trade_ship_dock_activity(int32_t figure_id)
{
    building *dock = building_get(figure_get(figure_id)->destination_building_id);
    if (dock->state != BUILDING_STATE_IN_USE) {
        return TRADE_SHIP_BUYING;
    }
    int16_t type = dock->type;
    if (type != BUILDING_DOCK && type != BUILDING_DOCK_UP) {
        return TRADE_SHIP_BUYING;
    }
    for (int32_t i = 0; i < BUILDING_DOCK_DOCKERS; i++) {
        int32_t docker = dock->data.dock.docker_ids[i];
        if (docker == 0) {
            continue;
        }
        const figure *docker_figure = figure_get(docker);
        if (docker_figure->state != FIGURE_STATE_ALIVE) {
            continue;
        }
        switch (docker_figure->action_state) {
            case FIGURE_ACTION_DOCKER_IMPORT_QUEUE:
            case FIGURE_ACTION_DOCKER_IMPORT_TO_STORAGEYARD:
            case FIGURE_ACTION_DOCKER_IMPORT_AT_STORAGEYARD:
            case FIGURE_ACTION_DOCKER_RETURNING:
                return TRADE_SHIP_BUYING;
            case FIGURE_ACTION_DOCKER_EXPORT_QUEUE:
            case FIGURE_ACTION_DOCKER_EXPORT_TO_STORAGEYARD:
            case FIGURE_ACTION_DOCKER_EXPORT_RETURNING:
            case FIGURE_ACTION_DOCKER_EXPORT_AT_STORAGEYARD:
                return TRADE_SHIP_SELLING;
        }
    }
    return TRADE_SHIP_NONE;
}

static int32_t phrase_max_distance(const figure *f)
{
    return calc_maximum_distance(f->destination_x, f->destination_y, f->source_x, f->source_y);
}

static int32_t phrase_next_sequence(figure *f, int32_t limit, int32_t restart)
{
    uint8_t seq = (uint8_t) (f->phrase_sequence_exact + 1);
    f->phrase_sequence_exact = seq;
    if (seq >= limit) {
        f->phrase_sequence_exact = (uint8_t) restart;
    }
    return f->phrase_sequence_exact;
}

static const int32_t problem_groups[CITY_PROBLEM_COUNT] = {
    [CITY_PROBLEM_HEALTH] = PHRASE_GROUP_HEALTH,
    [CITY_PROBLEM_FOOD] = PHRASE_GROUP_CITY_LIFE,
    [CITY_PROBLEM_INVASION] = PHRASE_GROUP_MILITARY,
    [CITY_PROBLEM_WORKER_SHORTAGE] = PHRASE_GROUP_LABOR,
    [CITY_PROBLEM_GODS] = PHRASE_GROUP_RELIGION,
    [CITY_PROBLEM_KINGDOM] = PHRASE_GROUP_KINGDOM,
    [CITY_PROBLEM_UNEMPLOYMENT] = PHRASE_GROUP_LABOR,
    [CITY_PROBLEM_ENTERTAINMENT] = PHRASE_GROUP_ENTERTAINMENT,
};

static void phrase_say(figure *f, int32_t phrase)
{
    figure_phrase_id = phrase;
    f->phrase_id = (uint8_t) figure_phrase_id;
}

static void phrase_say_in_group(figure *f, int32_t group, int32_t phrase)
{
    f->phrase_group = (uint8_t) group;
    phrase_say(f, phrase);
}

void figure_phrase_determine(int32_t figure_id)
{
    if (figure_id < 1) {
        return;
    }
    figure *f = figure_get(figure_id);
    city_player *player = city_player_current();
    f->phrase_id = 0;
    figure_phrase_id = FIGURE_PHRASE_NONE;
    int32_t event_running = any_event_running();
    f->phrase_group = PHRASE_GROUP_OWN;
    int32_t type = f->type;
    int32_t action = f->action_state;
    switch (type) {
        case FIGURE_IMMIGRANT:
            figure_phrase_id = phrase_next_sequence(f, 3, 0);
            break;
        case FIGURE_EMIGRANT:
            switch (player->sentiment_low_mood_cause) {
                case LOW_MOOD_CAUSE_LACK_OF_FOOD:
                    phrase_say_in_group(f, PHRASE_GROUP_LABOR, 1);
                    return;
                case LOW_MOOD_CAUSE_LACK_OF_JOBS:
                    phrase_say_in_group(f, PHRASE_GROUP_LABOR, 0);
                    return;
                case LOW_MOOD_CAUSE_HIGH_TAXES:
                    phrase_say_in_group(f, PHRASE_GROUP_TAXES, 2);
                    return;
                default:
                    phrase_say_in_group(f, PHRASE_GROUP_LABOR, 3);
                    return;
            }
        case FIGURE_HOMELESS:
            figure_phrase_id = phrase_next_sequence(f, 2, 0);
            f->phrase_group = PHRASE_GROUP_CITY_LIFE;
            break;
        case FIGURE_CART_PUSHER:
            if (action == FIGURE_ACTION_CART_INITIAL) {
                if (f->min_max_seen == 2) {
                    phrase_say_in_group(f, PHRASE_GROUP_LABOR, 0);
                    return;
                }
                if (f->min_max_seen == 1) {
                    phrase_say(f, 1);
                    return;
                }
            } else if (action == FIGURE_ACTION_CART_TO_STORAGEYARD || action == FIGURE_ACTION_CART_TO_GRANARY || action == FIGURE_ACTION_CART_TO_WORKSHOP) {
                if (phrase_max_distance(f) >= FIGURE_PHRASE_FAR_DISTANCE) {
                    phrase_say(f, 2);
                    return;
                }
            }
            break;
        case FIGURE_LABOR_SEEKER:
            if (player->unemployment_percentage > 5) {
                phrase_say(f, 10);
                return;
            }
            if (player->workers_needed > 5) {
                phrase_say(f, 11);
                return;
            }
            break;
        case FIGURE_TAX_COLLECTOR: {
            int32_t a = player->taxed_nobles + player->taxed_plebs;
            int32_t total = player->untaxed_nobles + player->untaxed_plebs + a;
            if (((a * 100) / total) < 50) {
                figure_phrase_id = 10;
                f->phrase_group = PHRASE_GROUP_TAXES;
            } else if (f->min_max_seen >= 7) {
                figure_phrase_id = 11;
                f->phrase_group = PHRASE_GROUP_TAXES;
            } else if (f->min_max_seen >= 1) {
                figure_phrase_id = 12;
                f->phrase_group = PHRASE_GROUP_TAXES;
            } else {
                f->phrase_group = PHRASE_GROUP_TAXES;
            }
            break;
        }
        case FIGURE_ARCHITECT:
            phrase_say(f, ((int32_t) f->min_max_seen >= 600) + 10);
            return;
        case FIGURE_STORAGE_YARD_CART:
            if (action == FIGURE_ACTION_STORAGE_CART_INITIAL) {
                if (f->min_max_seen == 2) {
                    phrase_say(f, 1);
                    return;
                }
            } else if (action == FIGURE_ACTION_STORAGE_CART_DELIVERING) {
                if (phrase_max_distance(f) >= FIGURE_PHRASE_FAR_DISTANCE) {
                    phrase_say(f, 2);
                    return;
                }
            }
            break;
        case FIGURE_FIREMAN:
            phrase_next_sequence(f, 13, 0);
            if (action == FIGURE_ACTION_FIREMAN_AT_FIRE) {
                figure_phrase_id = f->phrase_sequence_exact;
            }
            break;
        case FIGURE_JUGGLER:
        case FIGURE_MUSICIAN:
        case FIGURE_DANCER:
        case FIGURE_SENET_PLAYER:
        case FIGURE_TEACHER:
        case FIGURE_LIBRARIAN:
        case FIGURE_DENTIST:
        case FIGURE_PATRICIAN:
        case FIGURE_WATER_CARRIER:
        case FIGURE_ZOOKEEPER:
            break;
        case FIGURE_TRADE_CARAVAN:
        case FIGURE_TRADE_CARAVAN_DONKEY: {
            uint8_t seq = (uint8_t) (f->phrase_sequence_exact + 1);
            f->phrase_sequence_exact = seq;
            if (seq > 2) {
                f->phrase_sequence_exact = 1;
            }
            figure_phrase_id = f->phrase_sequence_exact + 1;
            if (action == FIGURE_ACTION_TRADE_CARAVAN_LEAVING) {
                int32_t trader = f->trader_id;
                if (trader_traded_nothing(trader)) {
                    figure_phrase_id = 0;
                }
            } else if (action == FIGURE_ACTION_TRADE_CARAVAN_TRADING) {
                if (figure_trade_caravan_can_buy(figure_id, city_current_player, f->destination_building_id, f->empire_city_id)) {
                    figure_phrase_id = 4;
                } else if (figure_trade_caravan_can_sell(figure_id, city_current_player, f->destination_building_id, f->empire_city_id)) {
                    figure_phrase_id = 3;
                }
            }
            f->phrase_group = PHRASE_GROUP_TRADE;
            break;
        }
        case FIGURE_TRADE_SHIP:
            if (action == FIGURE_ACTION_TRADE_SHIP_LEAVING) {
                int32_t trader = f->trader_id;
                figure_phrase_id = trader_traded_nothing(trader) ? 2 : 4;
            } else if (action == FIGURE_ACTION_TRADE_SHIP_DOCKED) {
                int32_t activity = figure_trade_ship_dock_activity(figure_id);
                figure_phrase_id = (activity == TRADE_SHIP_BUYING) ? 1 : ((activity != TRADE_SHIP_SELLING) ? 2 : 0);
            } else {
                figure_phrase_id = 3;
            }
            f->phrase_group = PHRASE_GROUP_TRADE;
            break;
        case FIGURE_CRIMINAL:
            figure_phrase_id = phrase_next_sequence(f, 4, 0);
            f->phrase_group = PHRASE_GROUP_CITY_LIFE;
            break;
        case FIGURE_TOMB_ROBBER:
            figure_phrase_id = phrase_next_sequence(f, 2, 0);
            f->phrase_group = PHRASE_GROUP_MONUMENTS;
            break;
        case FIGURE_FISHING_BOAT:
            switch (action) {
                case FIGURE_ACTION_FISHING_BOAT_CREATED:
                case FIGURE_ACTION_FISHING_BOAT_AT_WHARF:
                    phrase_say(f, 2);
                    return;
                case FIGURE_ACTION_FISHING_BOAT_GOING_TO_FISH:
                case FIGURE_ACTION_FISHING_BOAT_FISHING:
                    phrase_say(f, 0);
                    return;
                case FIGURE_ACTION_FISHING_BOAT_GOING_TO_WHARF:
                case FIGURE_ACTION_FISHING_BOAT_RETURNING_WITH_FISH:
                    phrase_say(f, 1);
                    return;
            }
            break;
        case FIGURE_MARKET_TRADER:
            f->phrase_group = PHRASE_GROUP_TRADE;
            figure_phrase_id = action != FIGURE_ACTION_ROAMER_RETURN;
            break;
        case FIGURE_PRIEST:
            switch (building_temple_god(f->building_id)) {
                case GOD_OSIRIS:
                    if (floods_data.settings.quality < 60 && floods_data.has_floodplain != 0) {
                        phrase_say_in_group(f, PHRASE_GROUP_CITY_LIFE, 11);
                        return;
                    }
                    break;
                case GOD_RA:
                    if (player->rating_kingdom < 20) {
                        phrase_say_in_group(f, PHRASE_GROUP_KINGDOM, 11);
                        return;
                    }
                    break;
                case GOD_PTAH:
                    if (player->workers_needed > 10) {
                        phrase_say_in_group(f, PHRASE_GROUP_LABOR, 11);
                        return;
                    }
                    break;
                case GOD_SETH:
                    if (event_running) {
                        phrase_say_in_group(f, PHRASE_GROUP_MILITARY, 11);
                        return;
                    }
                    break;
                case GOD_BAST:
                    if (building_temple_complex_has_upgrade(f->building_id, TEMPLE_COMPLEX_ORACLE)) {
                        phrase_say_in_group(f, PHRASE_GROUP_HEALTH, 13);
                        return;
                    }
                    if (player->sentiment < 60) {
                        phrase_say_in_group(f, PHRASE_GROUP_CITY_LIFE, 11);
                        return;
                    }
                    if (player->average_entertainment < 60) {
                        phrase_say_in_group(f, PHRASE_GROUP_ENTERTAINMENT, 12);
                        return;
                    }
                    break;
            }
            break;
        case FIGURE_PHYSICIAN:
            if (player->health < 40) {
                phrase_say_in_group(f, PHRASE_GROUP_HEALTH, 10);
                return;
            }
            break;
        case FIGURE_HERBALIST:
            f->phrase_group = PHRASE_GROUP_HEALTH;
            figure_phrase_id = ((int32_t) f->min_max_seen > 800) + 1;
            break;
        case FIGURE_EMBALMER:
            if (player->health < 30) {
                phrase_say_in_group(f, PHRASE_GROUP_HEALTH, 10);
                return;
            }
            break;
        case FIGURE_DOCKER:
            if (action == FIGURE_ACTION_DOCKER_IMPORT_TO_STORAGEYARD || action == FIGURE_ACTION_DOCKER_EXPORT_TO_STORAGEYARD) {
                if (phrase_max_distance(f) >= FIGURE_PHRASE_FAR_DISTANCE) {
                    figure_phrase_id = 2;
                }
                f->phrase_group = PHRASE_GROUP_TRADE;
            }
            break;
        case FIGURE_MARKET_BUYER:
            if (action == FIGURE_ACTION_MARKET_BUYER_TO_STORAGE) {
                phrase_say_in_group(f, PHRASE_GROUP_TRADE, 10);
                return;
            }
            if (action == FIGURE_ACTION_MARKET_BUYER_RETURNING) {
                phrase_say_in_group(f, PHRASE_GROUP_TRADE, 11);
                return;
            }
            break;
        case FIGURE_TOWER_SENTRY: {
            int32_t value = player->enemies;
            if (value == 0) {
                phrase_say_in_group(f, PHRASE_GROUP_MILITARY, 0);
                return;
            }
            if (event_running) {
                phrase_say_in_group(f, PHRASE_GROUP_MILITARY, 1);
                return;
            }
            if (value <= 10) {
                phrase_say_in_group(f, PHRASE_GROUP_MILITARY, 2);
                return;
            }
            if (value <= 30) {
                phrase_say_in_group(f, PHRASE_GROUP_MILITARY, 3);
                return;
            }
            break;
        }
        case FIGURE_DELIVERY_BOY:
            figure_phrase_id = phrase_next_sequence(f, 3, 0);
            f->phrase_group = PHRASE_GROUP_TRADE;
            break;
        case FIGURE_HUNTER:
            phrase_say(f, action == FIGURE_ACTION_HUNTER_RETURN_WITH_PREY);
            return;
        case FIGURE_LUMBERJACK:
        case FIGURE_REED_GATHERER:
            f->phrase_group = PHRASE_GROUP_TRADE;
            figure_phrase_id = (action == FIGURE_ACTION_LUMBERJACK_RETURN_HOME);
            break;
        case FIGURE_TRANSPORT_SHIP:
            if (player->enemies != 0 && action == FIGURE_ACTION_TRANSPORT_SHIP_EVADING) {
                figure_phrase_id = 0;
            } else if (player->enemy_warships != 0) {
                figure_phrase_id = 1;
            } else {
                figure_phrase_id = event_running ? 2 : 3;
            }
            f->phrase_group = PHRASE_GROUP_SHIPS;
            break;
        case FIGURE_WARSHIP:
            if (player->enemies != 0 && action == FIGURE_ACTION_WARSHIP_HUNTING) {
                figure_phrase_id = 0;
            } else if (player->enemies != 0 && action == FIGURE_ACTION_WARSHIP_EVADING) {
                figure_phrase_id = 1;
            } else if (player->enemy_warships != 0 && action != FIGURE_ACTION_WARSHIP_HUNTING && action != FIGURE_ACTION_WARSHIP_EVADING) {
                figure_phrase_id = 2;
            } else {
                figure_phrase_id = event_running ? 3 : 4;
            }
            f->phrase_group = PHRASE_GROUP_SHIPS;
            break;
        case FIGURE_CARPENTER:
        case FIGURE_STONEMASON:
        case FIGURE_TOMB_ARTISAN:
            f->phrase_group = PHRASE_GROUP_MONUMENTS;
            figure_phrase_id = (action != FIGURE_ACTION_CARPENTER_GO_TO_SITE);
            break;
        case FIGURE_BRICKLAYER:
            f->phrase_group = PHRASE_GROUP_MONUMENTS;
            figure_phrase_id = action != FIGURE_ACTION_BRICKLAYER_GO_TO_SITE;
            break;
        case FIGURE_LABORER: {
            int32_t working = 0;
            switch (action) {
                case FIGURE_ACTION_LABORER_TO_STORAGE_YARD:
                case FIGURE_ACTION_LABORER_AT_STORAGE_YARD:
                case FIGURE_ACTION_LABORER_TO_PYRAMID:
                case FIGURE_ACTION_LABORER_CLIMB_RAMP:
                case FIGURE_ACTION_LABORER_TO_MASON:
                case FIGURE_ACTION_LABORER_WALK_TO_MASON:
                case FIGURE_ACTION_LABORER_TO_LEVEL_TILE:
                case FIGURE_ACTION_LABORER_LEVEL_WORK:
                case FIGURE_ACTION_LABORER_SLED_TO_SUN_TEMPLE:
                case FIGURE_ACTION_LABORER_SLED_TO_MAUSOLEUM:
                case FIGURE_ACTION_LABORER_SLED_AT_RAMP:
                    working = 1;
                    break;
            }
            if (laborer_farm_work_waiting(&floods_data)) {
                phrase_say(f, 11);
                return;
            }
            if (working) {
                phrase_say_in_group(f, PHRASE_GROUP_MONUMENTS, 12);
                return;
            }
            break;
        }
        case FIGURE_CONSTABLE:
            phrase_next_sequence(f, 4, 0);
            if (action == FIGURE_ACTION_ATTACK) {
                int32_t t1 = figure_get(f->target_figure_id)->type;
                int32_t t2 = figure_get(f->targeted_by_figure_id)->type;
                if (t1 == FIGURE_CRIMINAL || t2 == FIGURE_CRIMINAL || t1 == FIGURE_PROTESTER || t2 == FIGURE_PROTESTER) {
                    phrase_say_in_group(f, PHRASE_GROUP_CITY_LIFE, 13);
                    return;
                }
                figure_phrase_id = f->phrase_sequence_exact + 14;
                break;
            }
            if ((int32_t) f->min_max_seen >= 500) {
                phrase_say_in_group(f, PHRASE_GROUP_CITY_LIFE, 12);
                return;
            }
            if (f->min_max_seen < 100) {
                phrase_say(f, 10);
                return;
            }
            phrase_say_in_group(f, PHRASE_GROUP_CITY_LIFE, 11);
            return;
        case FIGURE_MAGISTRATE:
            if (player->monument_completion_flag != 0) {
                phrase_say(f, 11);
                return;
            }
            if ((int32_t) f->min_max_seen >= 500) {
                phrase_say_in_group(f, PHRASE_GROUP_CITY_LIFE, 14);
                return;
            }
            if (f->min_max_seen >= 100) {
                phrase_say_in_group(f, PHRASE_GROUP_CITY_LIFE, 13);
                return;
            }
            phrase_say(f, 12);
            return;
        case FIGURE_FESTIVAL_PRIEST:
            phrase_say_in_group(f, PHRASE_GROUP_RELIGION, 10);
            return;
        case FIGURE_SLED_PULLER:
            phrase_say_in_group(f, PHRASE_GROUP_MONUMENTS, 12);
            return;
        case FIGURE_PLAGUED_CITIZEN:
            phrase_say_in_group(f, PHRASE_GROUP_HEALTH, 0);
            return;
        default:
            f->phrase_id = (uint8_t) FIGURE_PHRASE_NONE;
            figure_phrase_id = FIGURE_PHRASE_NONE;
            return;
    }
    if (figure_phrase_id == FIGURE_PHRASE_NONE) {
        switch (f->type) {
            case FIGURE_LABOR_SEEKER:
            case FIGURE_TAX_COLLECTOR:
            case FIGURE_FIREMAN:
            case FIGURE_JUGGLER:
            case FIGURE_MUSICIAN:
            case FIGURE_DANCER:
            case FIGURE_SENET_PLAYER:
            case FIGURE_PRIEST:
            case FIGURE_TEACHER:
            case FIGURE_LIBRARIAN:
            case FIGURE_DENTIST:
            case FIGURE_PHYSICIAN:
            case FIGURE_EMBALMER:
            case FIGURE_WORKER:
            case FIGURE_MARKET_BUYER:
            case FIGURE_PATRICIAN:
            case FIGURE_WATER_CARRIER:
            case FIGURE_CONSTABLE:
            case FIGURE_MAGISTRATE:
            case FIGURE_ZOOKEEPER: {
                int32_t problem = figure_phrase_pick_city_problem();
                figure_phrase_id = problem;
                if (problem == CITY_PROBLEM_NONE || problem == CITY_PROBLEM_NOTHING_TO_COMPLAIN) {
                    phrase_say_in_group(f, 13, problem);
                    return;
                }
                phrase_say_in_group(f, problem_groups[problem], problem);
                return;
            }
        }
    }
    f->phrase_id = (uint8_t) figure_phrase_id;
}

static int32_t phrase_god_sound(const figure *f)
{
    int32_t god = building_temple_god(f->building_id);
    return ((uint32_t) god < GOD_COUNT) ? (FIGURE_SPEECH_PRIEST_OSIRIS + god) : FIGURE_SPEECH_NONE;
}

static int32_t phrase_climate_sound(void)
{
    switch (climate_prey(scenario_data.climate_type)->figure_type) {
        case FIGURE_BIRDS:
            return FIGURE_SPEECH_HUNTER_BIRDS;
        case FIGURE_OSTRICH:
            return FIGURE_SPEECH_HUNTER_OSTRICH;
        case FIGURE_ANTELOPE:
            return FIGURE_SPEECH_HUNTER_ANTELOPE;
    }
    return FIGURE_SPEECH_NONE;
}

void figure_phrase_play(void)
{
    int32_t figure_id = info_figure_ids[info_figure_index];
    if (figure_id < 1) {
        return;
    }
    figure *f = figure_get(figure_id);
    int32_t sound = -1;
    switch (f->type) {
        case FIGURE_HYENA:
            sound = 0;
            break;
        case FIGURE_LION:
            sound = 1;
            break;
        case FIGURE_SCORPION:
            sound = 2;
            break;
        case FIGURE_ASP:
            sound = 3;
            break;
        case FIGURE_OSTRICH:
            sound = 4;
            break;
        case FIGURE_ANTELOPE:
            sound = 5;
            break;
        case FIGURE_HIPPO:
            sound = 6;
            break;
        case FIGURE_CROCODILE:
            sound = 7;
            break;
        case FIGURE_BIRDS:
            sound = 8;
            break;
    }
    if (sound >= 0) {
        sound_figure_play(figure_id, sound, 0);
    }
    int32_t id = figure_speech_sound(f->type);
    phrase_sound_id = id;
    if (id == FIGURE_SPEECH_BY_CLIMATE) {
        int32_t s = phrase_climate_sound();
        id = s >= 0 ? s : FIGURE_SPEECH_BY_CLIMATE;
    } else {
        int32_t god_sound = -1;
        if (id == FIGURE_SPEECH_BY_TEMPLE_GOD) {
            god_sound = phrase_god_sound(f);
            id = god_sound >= 0 ? god_sound : phrase_sound_id;
        }
        if (god_sound < 0) {
            if (id == FIGURE_SPEECH_BY_SOUND_TYPE) {
                id = figure_speech_sound(f->resource_id);
                phrase_sound_id = id;
                if (id == FIGURE_SPEECH_BY_TEMPLE_GOD) {
                    int32_t s = phrase_god_sound(f);
                    id = s >= 0 ? s : phrase_sound_id;
                }
            }
            if (id == FIGURE_SPEECH_NONE) {
                return;
            }
        }
    }
    id -= FIGURE_SPEECH_FIRST;
    phrase_sound_id = id;
    if (id >= 0) {
        figure_phrase_id = (int8_t) f->phrase_id;
        sound_speech_play_phrase(id, figure_phrase_id);
    }
}

int32_t figure_phrase_sound_id(int32_t figure_id)
{
    figure *f = figure_get(figure_id);
    int32_t id = figure_speech_sound(f->type);
    if (id > 0) {
        return id;
    }
    if (id == FIGURE_SPEECH_BY_SOUND_TYPE) {
        id = figure_speech_sound(f->resource_id);
        switch (id) {
            case FIGURE_SPEECH_BY_TEMPLE_GOD:
                return phrase_god_sound(f);
            case FIGURE_SPEECH_BY_SOUND_TYPE:
            case FIGURE_SPEECH_BY_CLIMATE:
            case FIGURE_SPEECH_NONE:
                return FIGURE_SPEECH_NONE;
        }
        return id;
    }
    if (id == FIGURE_SPEECH_BY_CLIMATE) {
        return phrase_climate_sound();
    }
    if (id == FIGURE_SPEECH_BY_TEMPLE_GOD) {
        return phrase_god_sound(f);
    }
    return FIGURE_SPEECH_NONE;
}

static void phrase_weight_clear(int32_t phrase)
{
    uint8_t *block = (uint8_t *) &phrase_weight_block;
    block[offsetof(phrase_weight_record, phrase_weights) + phrase] = 0;
}

void window_figure_info_button_city_problem(void)
{
    int32_t figure_id = info_figure_ids[info_figure_index];
    figure *f = figure_get(figure_id);
    int32_t phrase = (int8_t) f->phrase_id;
    if (phrase != CITY_PROBLEM_NONE) {
        phrase_weight_clear(phrase);
        uint8_t best = 0;
        for (uint8_t i = 0; i < CITY_PROBLEM_COUNT; i++) {
            if (phrase_weight_block.phrase_weights[i] > best) {
                best = i;
            }
        }
        if (phrase_weight_block.phrase_weights[best] > 1) {
            phrase_weight_block.phrase_weights[best] = 0;
            f->phrase_id = best;
            figure_phrase_id = best;
            if (best != CITY_PROBLEM_NONE && best != CITY_PROBLEM_NOTHING_TO_COMPLAIN) {
                window_redraw_request = 1;
                f->phrase_group = (uint8_t) problem_groups[best];
                return;
            }
            f->phrase_group = PHRASE_GROUP_CITY_LIFE;
        }
    }
    window_redraw_request = 1;
}
