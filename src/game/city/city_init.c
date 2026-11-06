// The city data of a new scenario: both player records cleared, the defaults of the human player and of its gods,
// and the city messages reset.

#include "city/city_init.h"

#include "building/building_data.h"
#include "city/city_data.h"
#include "city/city_internal.h"
#include "city/finance_estimates.h"
#include "city/gods.h"
#include "city/kingdom.h"
#include "city/message.h"
#include "city/resource_type.h"
#include "core/calc.h"

#include <string.h>

int32_t new_city_rules[NEW_CITY_RULE_COUNT] = {
    [NEW_CITY_RULE_HEALTH] = 50,
    [NEW_CITY_RULE_SENTIMENT] = 60,
    [NEW_CITY_RULE_KINGDOM_WAGES] = 30,
    [NEW_CITY_RULE_WAGES] = 30,
    [NEW_CITY_RULE_TAX_PERCENTAGE] = 9,
};

void city_gods_init(void)
{
    city_player *player = city_player_get(CITY_PLAYER_HUMAN);
    for (int32_t god = 0; god < GOD_COUNT; god++) {
        player->god_target_mood[god] = GOD_MOOD_NEUTRAL;
        player->god_mood[god] = GOD_MOOD_NEUTRAL;
        player->god_wrath_bolts[god] = 0;
        player->god_happy_bolts[god] = 0;
        player->god_extra_state_1[god] = 0;
        player->god_extra_state_2[god] = 0;
        player->god_extra_state_3[god] = 0;
        player->god_months_since_festival[god] = 0;
    }
    player->ra_trade_bonus_months = GOD_EFFECT_NONE;
    player->ra_trade_penalty_months = GOD_EFFECT_NONE;
    player->ra_trade_minor_penalty_months = GOD_EFFECT_NONE;
    player->ra_exports_bonus_months = GOD_EFFECT_NONE;
    player->ra_no_traders_months = GOD_EFFECT_NONE;
    player->gods_angry_message_delay = 0;
}

void city_data_init(void)
{
    memset(city_players, 0, sizeof(city_players));
    city_player_faction[0].word = 0;
    city_player *player = city_player_get(CITY_PLAYER_HUMAN);
    player->health_target = new_city_rules[NEW_CITY_RULE_HEALTH];
    player->health = new_city_rules[NEW_CITY_RULE_HEALTH];
    player->sentiment = new_city_rules[NEW_CITY_RULE_SENTIMENT];
    player->wages_kingdom = new_city_rules[NEW_CITY_RULE_KINGDOM_WAGES];
    player->wages = new_city_rules[NEW_CITY_RULE_WAGES];
    player->food_consumption_state = 3;
    player->tax_percentage = new_city_rules[NEW_CITY_RULE_TAX_PERCENTAGE];
    player->caravan_import_resource = RESOURCE_GRAIN;
    player->caravan_backup_import_resource = RESOURCE_GRAIN;
    player->monthly_population_next_index = 0;
    player->monthly_population_count = 0;
    player->months_since_festival = 1;
    player->festival_selected_size = FESTIVAL_SMALL;
    for (int32_t gift = GIFT_MODEST; gift < GIFT_SIZES; gift++) {
        player->gift_cost[gift] = 0;
    }
    city_gods_init();
}

void city_message_init_scenario(void)
{
    for (int32_t i = 0; i < MAX_MESSAGES; i++) {
        city_messages[i].type = 0;
        city_messages[i].phrases[0] = MESSAGE_PHRASES_END;
    }
    memset(city_message_population_shown, 0, POPULATION_SHOWN_FIRST_CLEAR);
    city_message_queue_delay = 0;
    for (int32_t i = 0; i < MESSAGE_CATEGORY_COUNT; i++) {
        city_message_count[i] = 0;
        city_message_delay[i] = 0;
        city_message_queue[i] = 0;
    }
    memset(city_message_population_shown + POPULATION_SHOWN_FIRST_CLEAR, 0, sizeof(city_message_population_shown) - POPULATION_SHOWN_FIRST_CLEAR);
    city_message_next_sequence = 0;
    city_message_total = 0;
    city_message_current_id = -1;
    city_message_shown_index = 0;
    city_message_scroll_position = 0;
    city_message_list_scroll = 0;
    city_message_posted_once = 1;
    message_fire_last_time = 0;
    message_collapse_last_time = 0;
    city_message_init_problem_areas();
    city_message_count_problem = 0;
}
