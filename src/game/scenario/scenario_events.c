// The scenario event processors, called once a month for every event: requests, gifts, invasions (warnings, then
// the attack), city status changes, messages, trade blockades, route limit steps, wage and price changes, the flood
// made better or worse, the common disasters and the kingdom rating change; and the start of follow-up events as
// clones of their template.
//
// A price decrease does nothing, not even the follow-up event, while the sell price is 0.

#include "building/building_storage.h"
#include "building/building_type.h"
#include "building/temple_complex.h"
#include "city/city_disasters.h"
#include "city/legion_requests.h"
#include "city/message.h"
#include "city/ratings.h"
#include "city/resource_type.h"
#include "editor/editor_data.h"
#include "empire/empire.h"
#include "empire/trade_status.h"
#include "figure/enemy.h"
#include "flood/flood_internal.h"
#include "game/game_data.h"
#include "game/tutorial.h"
#include "map/map_data.h"
#include "scenario/invasion.h"
#include "scenario/requests.h"
#include "scenario/scenario_internal.h"
#include "scenario/scenario_rules.h"
#include "window/window_data.h"

#include <string.h>

enum scenario_event_rules {
    TRADE_PROBLEM_MONTHS = 48,
    TRADE_LIMIT_LOW = 15,
    TRADE_LIMIT_MEDIUM = 25,
    TRADE_LIMIT_HIGH = 40,
    WAGES_MAX = 45,
    WAGES_MIN = 5,
    PRICE_DECREASE_FLOOR_BUY = 2,
    INVASION_BRIBE_KINGDOM = 15,
    REQUEST_REMINDER_MONTHS = 6,
    REQUEST_OVERDUE_MONTHS = 24,
    REQUEST_IGNORED_KINGDOM_PENALTY = 3,
    REQUEST_PHRASE_BLOCK = 18,
    TROOPS_RETURN_EARLY_MONTHS = 2,
    FLOOD_BETTER = 100,
};

static int32_t *trade_route_limit(int32_t route, int32_t resource)
{
    return &trade_route_limits[route][resource];
}

static int32_t *trade_price_buy(int32_t resource)
{
    return &trade_prices[resource * 2];
}

static int32_t *trade_price_sell(int32_t resource)
{
    return &trade_prices[(resource * 2) + 1];
}

static void chain_action(scenario_event *e, int32_t id_offset_value, int32_t action)
{
    if (scenario_event_is_chain_target(e, id_offset_value)) {
        int32_t phrase = scenario_event_reason_phrase(e, action);
        scenario_event_chain(e, id_offset_value, phrase);
    }
}

static void chain_completed(scenario_event *e)
{
    chain_action(e, e->on_completed, 0);
}

static void post_popup(int32_t type, int32_t param1, int32_t param2, int16_t *phrases)
{
    city_message_use_popup = 1;
    city_message_post(type, param1, param2, phrases, 0);
}

void scenario_event_process_reputation_increase(scenario_event *e)
{
    if (!scenario_event_is_due(e, 0)) {
        return;
    }
    scenario_event *parent = scenario_event_get(e->parent_id);
    int32_t param = e->id;
    int16_t p[EVENT_PHRASES];
    scenario_event_init_phrases(e, p);
    if (parent) {
        param = parent->id;
    }
    post_popup(MESSAGE_TEMPLATE_GENERAL, param, 0, p);
    city_ratings_change_kingdom(e->amount.value);
    e->flags |= EVENT_FLAG_FIRED;
    e->is_active = 0;
    chain_completed(e);
}

void scenario_event_process_sea_trade_problem(scenario_event *e)
{
    if (!scenario_event_is_due(e, 0)) {
        return;
    }
    e->flags |= EVENT_FLAG_FIRED;
    city_player *player = city_player_current();
    if (player->num_sea_routes > 0) {
        player->sea_trade_problem_duration = TRADE_PROBLEM_MONTHS;
        city_message_use_popup = 1;
        int16_t p[EVENT_PHRASES];
        scenario_event_init_phrases(e, p);
        city_message_post(MESSAGE_SEA_TRADE_PROBLEM, 0, 0, p, 0);
    }
    chain_completed(e);
}

void scenario_event_process_land_trade_problem(scenario_event *e)
{
    if (!scenario_event_is_due(e, 0)) {
        return;
    }
    e->flags |= EVENT_FLAG_FIRED;
    city_player *player = city_player_current();
    if (player->num_land_routes > 0) {
        player->land_trade_problem_duration = TRADE_PROBLEM_MONTHS;
        city_message_use_popup = 1;
        int16_t p[EVENT_PHRASES];
        scenario_event_init_phrases(e, p);
        city_message_post(MESSAGE_LAND_TRADE_PROBLEM, 0, 0, p, 0);
    }
    chain_completed(e);
}

void scenario_event_process_wage_increase(scenario_event *e)
{
    if (!scenario_event_is_due(e, 0)) {
        return;
    }
    e->flags |= EVENT_FLAG_FIRED;
    int32_t base = city_temple_complex_wage_bonus();
    city_player *player = city_player_current();
    int32_t wages = player->wages_kingdom + e->amount.value;
    if (base + wages <= WAGES_MAX) {
        player->wages_kingdom = wages;
        int16_t p[EVENT_PHRASES];
        scenario_event_init_phrases(e, p);
        post_popup(MESSAGE_WAGES_RAISED, 0, 0, p);
    }
    chain_completed(e);
}

void scenario_event_process_wage_decrease(scenario_event *e)
{
    if (!scenario_event_is_due(e, 0)) {
        return;
    }
    e->flags |= EVENT_FLAG_FIRED;
    int32_t base = city_temple_complex_wage_bonus();
    city_player *player = city_player_current();
    int32_t wages = player->wages_kingdom - e->amount.value;
    if (base + wages >= WAGES_MIN) {
        player->wages_kingdom = wages;
        city_message_use_popup = 1;
        int16_t p[EVENT_PHRASES];
        scenario_event_init_phrases(e, p);
        city_message_post(MESSAGE_WAGES_LOWERED, 0, 0, p, 0);
    }
    chain_completed(e);
}

static int32_t contaminated_water_health_loss(int32_t health)
{
    if (health > 80) {
        return -50;
    }
    return (health > 60) ? -40 : -25;
}

void scenario_event_process_contaminated_water(scenario_event *e)
{
    if (!scenario_event_is_due(e, 0)) {
        return;
    }
    e->flags |= EVENT_FLAG_FIRED;
    city_health_change(contaminated_water_health_loss(city_player_current()->health));
    city_message_use_popup = 1;
    int16_t p[EVENT_PHRASES];
    scenario_event_init_phrases(e, p);
    city_message_post(MESSAGE_CONTAMINATED_WATER, 0, 0, p, 0);
    chain_completed(e);
}

void scenario_event_process_clay_pit_flood(scenario_event *e)
{
    if (!scenario_event_is_due(e, 0)) {
        return;
    }
    e->flags |= EVENT_FLAG_FIRED;
    int32_t grid_offset = building_destroy_first_of_type(BUILDING_CLAY_PIT);
    if (grid_offset) {
        post_popup(MESSAGE_CLAY_PIT_FLOOD, 0, grid_offset, NULL);
    }
    chain_completed(e);
}

void scenario_event_process_locusts(scenario_event *e)
{
    if (!scenario_event_is_due(e, 0)) {
        return;
    }
    e->flags |= EVENT_FLAG_FIRED;
    city_locusts_start();
    post_popup(MESSAGE_LOCUSTS, 0, 0, NULL);
    chain_completed(e);
}

static int32_t event_message_type(const scenario_event *e)
{
    switch (e->subtype) {
        case EVENT_MESSAGE_CITY_SAVED:
            return MESSAGE_CITY_SAVED;
        case EVENT_MESSAGE_BATTLE_WON:
            return MESSAGE_DISTANT_BATTLE_WON;
        default:
            return MESSAGE_TEMPLATE_GENERAL;
    }
}

void scenario_event_process_message(scenario_event *e)
{
    if (!scenario_event_is_due(e, 0)) {
        return;
    }
    e->flags |= EVENT_FLAG_FIRED;
    city_message_use_popup = 1;
    int16_t p[EVENT_PHRASES];
    if (e->subtype < EVENT_MESSAGE_SUBTYPES) {
        p[EVENT_PHRASE_SECOND] = (int16_t) (146 + (4 * e->subtype));
        p[EVENT_PHRASE_FIRST] = (int16_t) (147 + (4 * e->subtype));
        p[EVENT_PHRASE_TEXT] = (int16_t) scenario_event_message_text(e);
    } else {
        p[EVENT_PHRASE_FIRST] = -1;
        p[EVENT_PHRASE_SECOND] = -1;
        p[EVENT_PHRASE_TEXT] = -1;
    }
    scenario_event_fill_phrases(e, p);
    city_message_post(event_message_type(e), 0, 0, p, 0);
    chain_completed(e);
}

void scenario_event_process_flood_quality(scenario_event *e, int32_t better)
{
    if (editor_active == 1 || !scenario_event_is_due(e, 0)) {
        return;
    }
    e->flags |= EVENT_FLAG_FIRED;
    int16_t p[EVENT_PHRASES];
    int32_t type;
    if (better == 0) {
        floods_worsen_next_quality(&floods_data);
        scenario_event_init_phrases(e, p);
        type = MESSAGE_NO_INUNDATION;
    } else {
        floods_improve_next_quality(&floods_data);
        scenario_event_init_phrases(e, p);
        type = MESSAGE_PERFECT_INUNDATION;
    }
    post_popup(type, 0, 0, p);
    chain_completed(e);
}

static int32_t trade_limit_raised(int32_t limit)
{
    if (limit >= TRADE_LIMIT_MEDIUM) {
        return TRADE_LIMIT_HIGH;
    }
    return (limit < TRADE_LIMIT_LOW) ? TRADE_LIMIT_LOW : TRADE_LIMIT_MEDIUM;
}

static int32_t trade_limit_lowered(int32_t limit)
{
    if (limit > TRADE_LIMIT_MEDIUM) {
        return TRADE_LIMIT_MEDIUM;
    }
    return (limit <= TRADE_LIMIT_LOW) ? 0 : TRADE_LIMIT_LOW;
}

void scenario_event_process_demand_increase(scenario_event *e)
{
    if (!scenario_event_is_due(e, 0)) {
        return;
    }
    int32_t resource = e->item.value;
    e->flags |= EVENT_FLAG_FIRED;
    int32_t route = e->route.value;
    int32_t *limit = trade_route_limit(route, resource);
    if (*limit < TRADE_LIMIT_HIGH) {
        *limit = trade_limit_raised(*limit);
        if (empire_city_is_trade_route_open(route)) {
            int16_t p[EVENT_PHRASES];
            scenario_event_init_phrases(e, p);
            post_popup(MESSAGE_TRADE_INCREASES, empire_city_get_for_trade_route(route), resource, p);
        }
    }
    chain_completed(e);
}

void scenario_event_process_demand_decrease(scenario_event *e)
{
    if (!scenario_event_is_due(e, 0)) {
        return;
    }
    int32_t resource = e->item.value;
    e->flags |= EVENT_FLAG_FIRED;
    int32_t route = e->route.value;
    int32_t *limit = trade_route_limit(route, resource);
    if (*limit > 0) {
        *limit = trade_limit_lowered(*limit);
        city_trade_status_update(resource);
        if (empire_city_is_trade_route_open(route)) {
            int16_t p[EVENT_PHRASES];
            scenario_event_init_phrases(e, p);
            post_popup(MESSAGE_TRADE_DECREASES, empire_city_get_for_trade_route(route), resource, p);
        }
    }
    chain_completed(e);
}

void scenario_event_process_price_increase(scenario_event *e)
{
    if (!scenario_event_is_due(e, 0)) {
        return;
    }
    int32_t amount = e->amount.value;
    e->flags |= EVENT_FLAG_FIRED;
    int32_t resource = e->item.value;
    *trade_price_buy(resource) += amount;
    *trade_price_sell(resource) += amount;
    int16_t p[EVENT_PHRASES];
    scenario_event_init_phrases(e, p);
    post_popup(MESSAGE_PRICE_INCREASE, amount, resource, p);
    chain_completed(e);
}

void scenario_event_process_price_decrease(scenario_event *e)
{
    if (!scenario_event_is_due(e, 0)) {
        return;
    }
    int32_t resource = e->item.value;
    int32_t amount = e->amount.value;
    e->flags |= EVENT_FLAG_FIRED;
    int32_t *sell = trade_price_sell(resource);
    int32_t *buy = trade_price_buy(resource);
    if (*sell <= 0) {
        return;
    }
    if (*sell > amount) {
        *buy -= amount;
        *sell -= amount;
    } else {
        *buy = PRICE_DECREASE_FLOOR_BUY;
        *sell = 0;
    }
    int16_t p[EVENT_PHRASES];
    scenario_event_init_phrases(e, p);
    post_popup(MESSAGE_PRICE_DECREASE, amount, resource, p);
    chain_completed(e);
}

static int32_t city_status_new_type(int32_t subtype, int32_t type)
{
    switch (subtype) {
        case CITY_STATUS_FELL_TO_ENEMY:
            if (type == EMPIRE_CITY_EGYPTIAN_TRADING) {
                return EMPIRE_CITY_FOREIGN_TRADING;
            }
            return (type == EMPIRE_CITY_EGYPTIAN) ? EMPIRE_CITY_FOREIGN : -1;
        case CITY_STATUS_CONQUERED:
            if (type == EMPIRE_CITY_FOREIGN_TRADING) {
                return EMPIRE_CITY_EGYPTIAN_TRADING;
            }
            return (type == EMPIRE_CITY_FOREIGN) ? EMPIRE_CITY_EGYPTIAN : -1;
        case CITY_STATUS_NEW_TRADE_ROUTE:
            if (type == EMPIRE_CITY_FOREIGN) {
                return EMPIRE_CITY_FOREIGN_TRADING;
            }
            if (type == EMPIRE_CITY_EGYPTIAN) {
                return EMPIRE_CITY_EGYPTIAN_TRADING;
            }
            return (type == EMPIRE_CITY_PHARAOH) ? EMPIRE_CITY_PHARAOH_TRADING : -1;
        case CITY_STATUS_LOST_TRADE_ROUTE:
        case CITY_STATUS_UNDER_SIEGE:
            if (type == EMPIRE_CITY_FOREIGN_TRADING) {
                return EMPIRE_CITY_FOREIGN;
            }
            if (type == EMPIRE_CITY_EGYPTIAN_TRADING) {
                return EMPIRE_CITY_EGYPTIAN;
            }
            return (type == EMPIRE_CITY_PHARAOH_TRADING) ? EMPIRE_CITY_PHARAOH : -1;
    }
    return -1;
}

static int32_t city_status_message_type(int32_t subtype)
{
    switch (subtype) {
        case CITY_STATUS_FELL_TO_ENEMY:
            return MESSAGE_CITY_FELL_TO_ENEMY;
        case CITY_STATUS_CONQUERED:
            return MESSAGE_CITY_CONQUERED;
        default:
            return MESSAGE_TEMPLATE_GENERAL;
    }
}

static int32_t empire_city_type_trades(int32_t type)
{
    return empire_city_type_is_trading(type) || type == EMPIRE_CITY_OURS;
}

static void empire_city_set_type(int32_t city_id, int32_t object, int32_t type, int32_t closes_route)
{
    empire_city *city = empire_city_edit(city_id);
    empire_object *map_object = &empire_map_objects[object];
    map_object->city_type = (uint8_t) type;
    map_object->image_id = (uint16_t) empire_object_city_image(object);
    city->type = map_object->city_type;
    if (closes_route) {
        city->is_open = 0;
    }
    for (int32_t r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
        city->sells[r] = 0;
        city->buys[r] = 0;
        if (empire_city_type_trades(city->type)) {
            if (empire_object_city_sells_resource(object, r)) {
                city->sells[r] = 1;
            }
            if (empire_object_city_buys_resource(object, r)) {
                city->buys[r] = 1;
            }
        }
        city_trade_status_update(r);
    }
}

void scenario_event_process_city_status_change(scenario_event *e)
{
    if (!scenario_event_is_due(e, 0)) {
        return;
    }
    e->flags |= EVENT_FLAG_FIRED;
    int32_t city_id = empire_city_get_for_trade_route(e->route.value);
    if (city_id != 0) {
        int32_t object = empire_city_get(city_id)->map_object;
        int32_t new_type = city_status_new_type(e->subtype, empire_map_objects[object].city_type);
        if (new_type >= 0) {
            int16_t p[EVENT_PHRASES];
            p[EVENT_PHRASE_SECOND] = (int16_t) (126 + (4 * e->subtype));
            p[EVENT_PHRASE_FIRST] = (int16_t) (0x7F + (4 * e->subtype));
            p[EVENT_PHRASE_TEXT] = (int16_t) scenario_event_message_text(e);
            scenario_event_fill_phrases(e, p);
            int32_t closes_route = e->subtype == CITY_STATUS_LOST_TRADE_ROUTE || e->subtype == CITY_STATUS_UNDER_SIEGE;
            empire_city_set_type(city_id, object, new_type, closes_route);
            post_popup(city_status_message_type(e->subtype), 0, 0, p);
        }
    }
    chain_completed(e);
}

static void gift_phrase(scenario_event *e, int16_t *p, int32_t room, int32_t amount, int32_t base_half, int32_t base_less)
{
    int32_t pharaoh = e->sender_faction != 0;
    if (room >= amount) {
        p[EVENT_PHRASE_BUTTONS] = GIFT_BUTTON_ACCEPT | GIFT_BUTTON_REFUSE;
        p[EVENT_PHRASE_FIRST] = (int16_t) (((e->item.value == RESOURCE_DEBEN) ? 269 : 267) - pharaoh);
    } else if ((amount / 2) <= room) {
        p[EVENT_PHRASE_BUTTONS] = GIFT_BUTTON_ACCEPT | GIFT_BUTTON_LATER | GIFT_BUTTON_REFUSE;
        p[EVENT_PHRASE_FIRST] = (int16_t) (base_half - pharaoh);
    } else {
        p[EVENT_PHRASE_BUTTONS] = 0;
        p[EVENT_PHRASE_FIRST] = (int16_t) (base_less - pharaoh);
    }
}

static int32_t gift_room(scenario_event *e)
{
    if (e->item.value == RESOURCE_DEBEN) {
        return e->amount.value;
    }
    return building_storage_room_for(e->item.value) / 100;
}

static int32_t event_starts_this_month(const scenario_event *e)
{
    return !(e->flags & (EVENT_FLAG_CHAIN_ONLY | EVENT_FLAG_BY_FAVOUR)) && game_time_year == e->year.value + (int16_t) scenario_data.start.word &&
        game_time_month == e->month;
}

static void gift_offer(scenario_event *e, int16_t *p, int32_t room, int32_t base_half, int32_t base_less, int32_t offered, int32_t not_offered)
{
    gift_phrase(e, p, room, e->amount.value, base_half, base_less);
    post_popup(MESSAGE_TEMPLATE_GIFT, e->id, 0, p);
    e->state = (int16_t) ((p[EVENT_PHRASE_BUTTONS] != 0) ? offered : not_offered);
}

static void gift_finish(scenario_event *e, int16_t *p)
{
    city_message_use_popup = 1;
    int32_t pharaoh = e->sender_faction != 0;
    switch (e->state) {
        case GIFT_STATE_ACCEPTED:
            if (e->item.value == RESOURCE_DEBEN) {
                city_player *player = city_player_current();
                p[EVENT_PHRASE_DELIVERED_AMOUNT] = e->amount.value;
                player->treasury += e->amount.value;
                player->donated_this_year += e->amount.value;
                p[EVENT_PHRASE_FIRST] = (int16_t) (281 - pharaoh);
            } else {
                int16_t stored = (int16_t) building_storage_store_anywhere(e->item.value, e->amount.value * 100, 0);
                p[EVENT_PHRASE_FIRST] = (int16_t) (279 - pharaoh);
                p[EVENT_PHRASE_DELIVERED_AMOUNT] = (int16_t) (stored / 100);
            }
            city_message_post(MESSAGE_TEMPLATE_GIFT, e->id, 0, p, 0);
            chain_action(e, e->on_completed, 0);
            break;
        case GIFT_STATE_REFUSED:
            p[EVENT_PHRASE_FIRST] = (int16_t) (285 - pharaoh);
            city_message_post(MESSAGE_TEMPLATE_GIFT, e->id, 0, p, 0);
            chain_action(e, e->on_refusal, 1);
            break;
        case GIFT_STATE_TOO_LATE:
            p[EVENT_PHRASE_FIRST] = (int16_t) (277 - pharaoh);
            city_message_post(MESSAGE_TEMPLATE_GIFT, e->id, 0, p, 0);
            chain_action(e, e->on_too_late, 2);
            break;
    }
    e->state = GIFT_STATE_DONE;
    e->is_active = 0;
    e->flags |= EVENT_FLAG_FIRED;
}

void scenario_event_process_gift_from_pharaoh(scenario_event *e)
{
    if (e->state == GIFT_STATE_DONE) {
        return;
    }
    int16_t p[EVENT_PHRASES];
    p[EVENT_PHRASE_SECOND] = (e->sender_faction != 0) ? 264 : 265;
    p[EVENT_PHRASE_TEXT] = (int16_t) scenario_event_message_text(e);
    scenario_event_fill_phrases(e, p);
    int16_t state = e->state;
    if (state == GIFT_STATE_ACCEPTED || state == GIFT_STATE_REFUSED || state == GIFT_STATE_TOO_LATE) {
        gift_finish(e, p);
        return;
    }
    if (!e->is_active) {
        if (!event_starts_this_month(e)) {
            return;
        }
        int32_t amount = e->amount.value;
        e->is_active = 1;
        int32_t room = gift_room(e);
        if (amount > room && (amount / 2) > room) {
            e->months_left = 1;
        }
        gift_offer(e, p, room, 271, 273, GIFT_STATE_OFFERED, GIFT_STATE_OFFER_AGAIN);
        return;
    }
    if (--e->months_left > 0) {
        return;
    }
    int32_t room = gift_room(e);
    if (e->state == GIFT_STATE_OFFER_AGAIN) {
        gift_offer(e, p, room, 271, 273, GIFT_STATE_OFFERED_AGAIN, GIFT_STATE_OFFER_LAST);
    } else if (e->state == GIFT_STATE_OFFER_LAST) {
        gift_offer(e, p, room, 275, 277, GIFT_STATE_OFFERED_LAST, GIFT_STATE_TOO_LATE);
    }
}

static void invasion_phrases(scenario_event *e, int16_t *p)
{
    switch (e->item.value) {
        case EVENT_INVADER_ENEMY:
            p[EVENT_PHRASE_SECOND] = 182;
            p[EVENT_PHRASE_TEXT] = 190;
            break;
        case EVENT_INVADER_EGYPT:
            p[EVENT_PHRASE_SECOND] = 172;
            p[EVENT_PHRASE_TEXT] = (int16_t) ((scenario_event_parent_by_favour(e) != 0) + 180);
            break;
        case EVENT_INVADER_PHARAOH:
            p[EVENT_PHRASE_SECOND] = 162;
            p[EVENT_PHRASE_TEXT] = (int16_t) ((scenario_event_parent_by_favour(e) != 0) + 170);
            break;
        case EVENT_INVADER_BEDOUINS:
            p[EVENT_PHRASE_SECOND] = 0xFF;
            p[EVENT_PHRASE_TEXT] = 263;
            break;
    }
    scenario_event_fill_phrases(e, p);
}

static int32_t invasion_nation(int32_t invader, int16_t *phrase)
{
    switch (invader) {
        case EVENT_INVADER_ENEMY:
            *phrase = 188;
            return scenario_data.enemy_type;
        case EVENT_INVADER_EGYPT:
            *phrase = 178;
            return ENEMY_EGYPTIAN;
        case EVENT_INVADER_PHARAOH:
            *phrase = 168;
            return ENEMY_EGYPTIAN;
        case EVENT_INVADER_BEDOUINS:
            *phrase = 261;
            return ENEMY_BEDOUIN;
        default:
            *phrase = 0;
            return -1;
    }
}

static void invasion_attack(scenario_event *e, int16_t *p)
{
    int32_t started = 0;
    int32_t invasion_id = 0;
    int16_t phrase;
    int32_t nation = invasion_nation(e->item.value, &phrase);
    if (nation >= 0) {
        invasion_id = scenario_invasion_start(nation, e->amount.value, e->route.value, e->attack_target, e->id);
        if (invasion_id) {
            city_message_use_popup = 1;
            started = 1;
            if (e->item.value == EVENT_INVADER_ENEMY) {
                scenario_event_reason_phrase(e, 0);
            }
            p[EVENT_PHRASE_FIRST] = phrase;
        }
    }
    e->flags |= EVENT_FLAG_FIRED;
    if (started) {
        post_popup(MESSAGE_ATTACK_ALERT, invasion_last_id, invasion_id, p);
    }
    int32_t next_phrase = scenario_event_reason_phrase(e, 0);
    scenario_event_chain(e, e->on_completed, next_phrase);
}

static int32_t invasion_warning_for(scenario_event *e, int32_t months)
{
    for (int32_t w = 1; w < INVASION_WARNINGS; w++) {
        if (invasion_warnings[w].months_left == months) {
            return w;
        }
    }
    if (!scenario_event_is_due(e, 1) || months <= 1) {
        return -1;
    }
    return 0;
}

void scenario_event_process_invasion(scenario_event *e)
{
    if (e->flags & EVENT_FLAG_FIRED) {
        return;
    }
    int16_t p[EVENT_PHRASES];
    invasion_phrases(e, p);
    if (!scenario_event_is_running(e)) {
        return;
    }
    if (scenario_event_parent_by_favour(e) && city_player_current()->rating_kingdom >= INVASION_BRIBE_KINGDOM) {
        post_popup(MESSAGE_INVASION_CALLED_OFF, 0, 0, NULL);
        scenario_invasion_called_off(e);
        return;
    }
    int32_t months = scenario_event_months_left(e);
    if (months == 0) {
        invasion_attack(e, p);
        return;
    }
    int32_t w = invasion_warning_for(e, months);
    if (w < 0) {
        return;
    }
    const invasion_warning *warning = &invasion_warnings[w];
    int32_t type = 0;
    int32_t invader = e->item.value - EVENT_INVADER_ENEMY;
    if ((uint32_t) invader < EVENT_INVADERS) {
        p[EVENT_PHRASE_FIRST] = (int16_t) warning->phrase[invader];
        type = warning->message[invader];
    }
    uint8_t shown = (uint8_t) (1 << w);
    if (shown & e->invasion_warnings_shown) {
        return;
    }
    e->invasion_warnings_shown |= shown;
    city_message_use_popup = scenario_event_is_due(e, 1) != 0;
    city_message_post(type, 0, 0, p, 0);
}

static int32_t request_phrase(scenario_event *e, int32_t base, int16_t *phrase)
{
    static const int8_t block[REQUEST_SUBTYPES] = {
        [REQUEST_SUBTYPE_GENERIC] = 0,
        [REQUEST_SUBTYPE_TROOPS] = -1,
        [REQUEST_SUBTYPE_DISTANT_BATTLE] = -1,
        [REQUEST_SUBTYPE_FESTIVAL] = 1,
        [REQUEST_SUBTYPE_CONSTRUCTION] = 2,
        [REQUEST_SUBTYPE_FAMINE] = 3,
        [REQUEST_SUBTYPE_CITY_IN_TROUBLE] = 4,
    };
    if (e->subtype >= REQUEST_SUBTYPES || block[e->subtype] < 0) {
        return 0;
    }
    *phrase = (int16_t) (base - (e->sender_faction != 0) + (REQUEST_PHRASE_BLOCK * block[e->subtype]));
    return 1;
}

static int32_t request_message_type(scenario_event *e)
{
    switch (e->subtype) {
        case REQUEST_SUBTYPE_FESTIVAL:
            return MESSAGE_FESTIVAL_REQUEST;
        case REQUEST_SUBTYPE_CONSTRUCTION:
            return MESSAGE_CONSTRUCTION_REQUEST;
        case REQUEST_SUBTYPE_FAMINE:
            return MESSAGE_FAMINE_REQUEST;
        default:
            return MESSAGE_TEMPLATE_REQUEST;
    }
}

static void request_check_compliance(scenario_event *e, int16_t *p)
{
    if (e->can_comply_shown) {
        return;
    }
    bool in_trouble = e->subtype == REQUEST_SUBTYPE_CITY_IN_TROUBLE;
    const city_player *player = city_player_current();
    if (e->item.value == RESOURCE_DEBEN) {
        if (player->treasury < e->amount.value) {
            return;
        }
        e->can_comply_shown = 1;
        city_message_use_popup = 1;
        p[EVENT_PHRASE_MONTHS_TO_COMPLY] = e->months_left;
        p[EVENT_PHRASE_NONE] = p[EVENT_PHRASE_SECOND] = p[EVENT_PHRASE_FIRST] = -1;
        city_message_post(MESSAGE_COMPLIANCE_NOW_POSSIBLE, e->id, 0, p, in_trouble);
    } else {
        int32_t needed = e->amount.value * 100;
        if (player->storage_stored[e->item.value] < needed) {
            return;
        }
        e->can_comply_shown = 1;
        city_message_use_popup = 1;
        p[EVENT_PHRASE_MONTHS_TO_COMPLY] = e->months_left;
        scenario_current_request = e->id;
        p[EVENT_PHRASE_NONE] = p[EVENT_PHRASE_SECOND] = p[EVENT_PHRASE_FIRST] = -1;
        city_message_post(MESSAGE_STORAGE_YARDS_READY_TO_FULFILL_REQUEST, e->id, 0, p, in_trouble);
    }
}

static void request_dispatched(scenario_event *e)
{
    if (--e->months_left > 0) {
        return;
    }
    city_message_use_popup = 1;
    if (e->state == REQUEST_STATE_DISPATCHED) {
        chain_action(e, e->on_completed, 0);
    } else {
        chain_action(e, e->on_too_late, 2);
    }
    e->state = REQUEST_STATE_FINISHED;
    e->is_active = 0;
    e->flags |= EVENT_FLAG_FIRED;
}

static void request_active(scenario_event *e, int16_t *p, int32_t type)
{
    int16_t months = --e->months_left;
    if (e->state == REQUEST_STATE_NORMAL) {
        if (months == REQUEST_REMINDER_MONTHS) {
            city_message_use_popup = 1;
            p[EVENT_PHRASE_MONTHS_TO_COMPLY] = months;
            request_phrase(e, 41, &p[EVENT_PHRASE_FIRST]);
            city_message_post(type, e->id, 0, p, 0);
        } else if (months <= 0) {
            city_message_use_popup = 1;
            request_phrase(e, 43, &p[EVENT_PHRASE_FIRST]);
            e->months_left = REQUEST_OVERDUE_MONTHS;
            p[EVENT_PHRASE_MONTHS_TO_COMPLY] = REQUEST_OVERDUE_MONTHS;
            e->state = REQUEST_STATE_OVERDUE;
            city_message_post(MESSAGE_TEMPLATE_REQUEST, e->id, 0, p, 0);
            city_ratings_change_kingdom(-REQUEST_IGNORED_KINGDOM_PENALTY);
            city_player_current()->kingdom_ignored_request_penalty = REQUEST_IGNORED_KINGDOM_PENALTY;
        }
    } else if (e->state == REQUEST_STATE_OVERDUE && months <= 0) {
        chain_action(e, e->on_refusal, 1);
        e->state = REQUEST_STATE_IGNORED;
        e->is_active = 0;
    }
    request_check_compliance(e, p);
}

static int32_t request_is_pottery_tutorial(const scenario_event *e)
{
    return tutorial_flags_a[TUTORIAL_FLAG_POTTERY_REQUEST] == 0 && settings_data.mission_id == tutorial_briefing_tables.tutorial.entries[TUTORIAL_FLAG_POTTERY_REQUEST].mission && e->type == EVENT_TYPE_REQUEST &&
        e->item.value == RESOURCE_POTTERY && settings_data.is_custom_scenario == 0;
}

static void request_start(scenario_event *e, int16_t *p, int32_t type)
{
    e->is_active = 1;
    const city_player *player = city_player_current();
    if (e->item.value == RESOURCE_DEBEN) {
        if (player->treasury >= e->amount.value) {
            e->can_comply_shown = 1;
        }
    } else if (player->storage_stored[e->item.value] >= e->amount.value * 100) {
        e->can_comply_shown = 1;
    }
    e->months_left = e->months_initial;
    city_message_use_popup = 1;
    request_phrase(e, 39, &p[EVENT_PHRASE_FIRST]);
    city_message_post(type, e->id, 0, p, 0);
    if (request_is_pottery_tutorial(e)) {
        tutorial_set_flag(TUTORIAL_FLAG_POTTERY_REQUEST);
    }
}

void scenario_event_process_request(scenario_event *e)
{
    if (e->item.value == RESOURCE_TROOPS) {
        scenario_event_process_troops_request(e);
        return;
    }
    if (e->state == REQUEST_STATE_FINISHED || e->state == REQUEST_STATE_IGNORED) {
        return;
    }
    int32_t type = request_message_type(e);
    int16_t p[EVENT_PHRASES];
    request_phrase(e, 37, &p[EVENT_PHRASE_SECOND]);
    p[EVENT_PHRASE_TEXT] = (int16_t) scenario_event_message_text(e);
    scenario_event_fill_phrases(e, p);
    if (e->state == REQUEST_STATE_DISPATCHED || e->state == REQUEST_STATE_DISPATCHED_LATE) {
        request_dispatched(e);
    } else if (e->is_active) {
        request_active(e, p, type);
    } else if (event_starts_this_month(e)) {
        request_start(e, p, type);
    }
}

static void troops_request_months_up(scenario_event *e)
{
    city_message_use_popup = 1;
    if (e->state == REQUEST_STATE_NORMAL) {
        chain_action(e, e->on_refusal, 1);
        e->state = REQUEST_STATE_FINISHED;
        e->is_active = 0;
        return;
    }
    if (e->state != REQUEST_STATE_RECEIVED) {
        e->state = REQUEST_STATE_FINISHED;
        e->is_active = 0;
        return;
    }
    int16_t left;
    if (e->months_left > TROOPS_RETURN_EARLY_MONTHS) {
        chain_action(e, e->on_too_late, 2);
        e->state = REQUEST_STATE_TROOPS_RETURN_TOO_LATE;
        left = (int16_t) (scenario_event_travel_months(e) - e->months_left);
    } else if (scenario_troops_request_dispatched(e->id)) {
        chain_action(e, e->on_completed, 0);
        e->state = REQUEST_STATE_TROOPS_RETURN;
        left = (int16_t) scenario_event_travel_months(e);
    } else {
        chain_action(e, e->on_defeat, 3);
        e->state = REQUEST_STATE_FINISHED;
        e->is_active = 0;
        return;
    }
    e->months_left = left;
    if (left < 1) {
        e->months_left = 1;
    }
}

static void troops_request_waiting(scenario_event *e, int16_t *p, int32_t type, int32_t months)
{
    int16_t block = (e->subtype != REQUEST_SUBTYPE_TROOPS) ? REQUEST_PHRASE_BLOCK : 0;
    int32_t pharaoh = e->sender_faction != 0;
    p[EVENT_PHRASE_MONTHS_TO_COMPLY] = (int16_t) scenario_event_months_margin(e);
    if (!e->is_active) {
        if (!event_starts_this_month(e)) {
            return;
        }
        e->is_active = 1;
        e->months_left = e->months_initial;
        city_message_use_popup = 1;
        p[EVENT_PHRASE_FIRST] = (int16_t) (block + (pharaoh ? 2 : 3));
        city_message_post(type, e->id, 0, p, 0);
        return;
    }
    int32_t travel = scenario_event_travel_months(e);
    if (travel <= 0) {
        travel = EVENT_TRAVEL_MONTHS_DEFAULT;
    }
    int32_t ahead = months - travel;
    if (ahead == e->months_margin) {
        p[EVENT_PHRASE_FIRST] = (int16_t) (block + (pharaoh ? 4 : 5));
    } else if (ahead == e->coin) {
        p[EVENT_PHRASE_FIRST] = (int16_t) (block + (pharaoh ? 6 : 7));
    } else {
        return;
    }
    city_message_post(MESSAGE_TEMPLATE_REQUEST, e->id, 0, p, 0);
}

static void troops_request_returning(scenario_event *e)
{
    if (--e->months_left >= 1) {
        return;
    }
    int32_t type = (e->state == REQUEST_STATE_TROOPS_RETURN) ? MESSAGE_TROOPS_RETURN : MESSAGE_TROOPS_RETURN_TOO_LATE;
    post_popup(type, 0, city_player_current()->exit_point_grid_offset, NULL);
    formation_legions_return_from_request(e->id);
    e->state = REQUEST_STATE_FINISHED;
    e->is_active = 0;
}

void scenario_event_process_troops_request(scenario_event *e)
{
    if (e->state == REQUEST_STATE_FINISHED) {
        return;
    }
    int32_t city_asks = e->subtype == REQUEST_SUBTYPE_TROOPS;
    int16_t p[EVENT_PHRASES];
    if (e->sender_faction != 0) {
        p[EVENT_PHRASE_SECOND] = (int16_t) (city_asks ? 0 : REQUEST_PHRASE_BLOCK);
    } else {
        p[EVENT_PHRASE_SECOND] = (int16_t) (city_asks ? 1 : (REQUEST_PHRASE_BLOCK + 1));
    }
    p[EVENT_PHRASE_TEXT] = (int16_t) scenario_event_message_text(e);
    int32_t type = city_asks ? MESSAGE_TRADE_CITY_UNDER_SIEGE : MESSAGE_DISTANT_BATTLE_REQUEST;
    scenario_event_fill_phrases(e, p);
    int32_t months = scenario_event_months_left(e);
    if (months == 0) {
        troops_request_months_up(e);
        return;
    }
    switch (e->state) {
        case REQUEST_STATE_NORMAL:
            troops_request_waiting(e, p, type, months);
            break;
        case REQUEST_STATE_RECEIVED:
            if (--e->months_left < 1) {
                e->months_left = 1;
            }
            break;
        case REQUEST_STATE_TROOPS_RETURN:
        case REQUEST_STATE_TROOPS_RETURN_TOO_LATE:
            troops_request_returning(e);
            break;
    }
}

static void inherit_value(scenario_value *child, int16_t parent_value)
{
    int16_t lo;
    int16_t hi;
    if (child->fixed < 0) {
        lo = child->min;
        hi = child->max;
    } else if (scenario_value_choice_count(child) == 1) {
        lo = hi = child->fixed;
    } else {
        lo = hi = -1;
    }
    if (parent_value >= lo && parent_value <= hi) {
        child->value = parent_value;
    }
}

void scenario_event_chain(scenario_event *e, int32_t id, int32_t phrase)
{
    scenario_event *template_event = scenario_event_get(id);
    if (!template_event) {
        return;
    }
    scenario_event *child = scenario_event_add(&scenario_events, 0);
    if (!child) {
        return;
    }
    int16_t child_id = child->id;
    memcpy(child, template_event, sizeof(scenario_event));
    child->id = child_id;
    scenario_event_reset(child);
    child->flags = EVENT_FLAG_CLONE;
    int32_t child_uses_city = scenario_event_uses_trade_city(child);
    if (child_uses_city == scenario_event_uses_trade_city(e)) {
        inherit_value(&child->route, e->route.value);
    }
    if (scenario_event_type_uses_resource(child) && scenario_event_type_uses_resource(e) && scenario_event_resource_in_range(child, e->item.value)) {
        inherit_value(&child->item, e->item.value);
    }
    child->parent_phrase = (int16_t) phrase;
    child->parent_id = e->id;
    if (scenario_event_uses_trade_city(e)) {
        child->parent_city = (uint8_t) e->route.value;
    } else {
        child->parent_city = 0;
    }
    int32_t delay = child->year.value;
    child->parent_amount = e->amount.value;
    child->parent_item = e->item.value;
    child->parent_phrase_var = e->phrase_var;
    int32_t month = (game_time_month + delay) % 12;
    int32_t years = delay / 12;
    child->month = (uint8_t) month;
    if ((month & 0xFF) < game_time_month) {
        years++;
    }
    child->year.value = (int16_t) (game_time_year - scenario_data.start.word + years);
    if (delay == 0 && !scenario_event_processed_later(&scenario_events, child->id)) {
        if (child->type == EVENT_TYPE_INVASION) {
            scenario_event_process_invasion(child);
        } else {
            scenario_event_process(child);
        }
    }
}

void scenario_event_process(scenario_event *e)
{
    switch (e->type) {
        case EVENT_TYPE_REQUEST:
            scenario_event_process_request(e);
            break;
        case EVENT_TYPE_REVOLT:
        case EVENT_TYPE_CHANGE_PHARAOH:
            break;
        case EVENT_TYPE_SEA_TRADE_PROBLEM:
            scenario_event_process_sea_trade_problem(e);
            break;
        case EVENT_TYPE_LAND_TRADE_PROBLEM:
            scenario_event_process_land_trade_problem(e);
            break;
        case EVENT_TYPE_WAGE_INCREASE:
            scenario_event_process_wage_increase(e);
            break;
        case EVENT_TYPE_WAGE_DECREASE:
            scenario_event_process_wage_decrease(e);
            break;
        case EVENT_TYPE_CONTAMINATED_WATER:
            scenario_event_process_contaminated_water(e);
            break;
        case EVENT_TYPE_GOLD_MINE_COLLAPSE:
            scenario_event_process_gold_mine_collapse(e);
            break;
        case EVENT_TYPE_CLAY_PIT_FLOOD:
            scenario_event_process_clay_pit_flood(e);
            break;
        case EVENT_TYPE_DEMAND_INCREASE:
            scenario_event_process_demand_increase(e);
            break;
        case EVENT_TYPE_DEMAND_DECREASE:
            scenario_event_process_demand_decrease(e);
            break;
        case EVENT_TYPE_PRICE_INCREASE:
            scenario_event_process_price_increase(e);
            break;
        case EVENT_TYPE_PRICE_DECREASE:
            scenario_event_process_price_decrease(e);
            break;
        case EVENT_TYPE_REPUTATION_INCREASE:
            scenario_event_process_reputation_increase(e);
            break;
        case EVENT_TYPE_REPUTATION_DECREASE:
            scenario_event_process_reputation_decrease(e);
            break;
        case EVENT_TYPE_CITY_STATUS_CHANGE:
            scenario_event_process_city_status_change(e);
            break;
        case EVENT_TYPE_MESSAGE:
            scenario_event_process_message(e);
            break;
        case EVENT_TYPE_FLOOD_WORSE:
            scenario_event_process_flood_quality(e, 0);
            break;
        case EVENT_TYPE_FLOOD_BETTER:
            scenario_event_process_flood_quality(e, FLOOD_BETTER);
            break;
        case EVENT_TYPE_GIFT_FROM_PHARAOH:
            scenario_event_process_gift_from_pharaoh(e);
            break;
        case EVENT_TYPE_LOCUSTS:
            scenario_event_process_locusts(e);
            break;
        case EVENT_TYPE_FROGS:
            scenario_event_process_frogs(e);
            break;
        case EVENT_TYPE_HAILSTORM:
            scenario_event_process_hailstorm(e);
            break;
        case EVENT_TYPE_BLOOD_RIVER:
            scenario_event_process_blood_river(e);
            break;
        case EVENT_TYPE_CRIME_WAVE:
            scenario_event_process_crime_wave(e);
            break;
        case EVENT_TYPE_MUMMY:
            scenario_event_process_mummies(e);
            break;
    }
}

void scenario_invasion_called_off(scenario_event *e)
{
    e->is_active = 0;
    e->flags |= EVENT_FLAG_FIRED;
}
