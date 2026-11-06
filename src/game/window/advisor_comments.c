// The chief advisor's comments: window 0x4F shows the comment on one topic of his rows, a text chosen from the city's
// state by the thresholds of advisor_comment_rules, and keeps in the city record the advisor it sends the player to;
// window 0x50 shows a god's description over the religion advisor.
//
// A few comments can never or only oddly appear:
// - the food stocks comment never shows text 101, nor the employment comment text 92 (minor labor shortage);
// - the military comment's "no forces" text needs a negative soldier count;
// - the kingdom comment stops counting at five waiting requests.

#include "window/advisor_comments.h"
#include "city/finance_estimates.h"
#include "city/food_supply.h"
#include "city/gods.h"
#include "city/resource_type.h"
#include "city/sentiment.h"
#include "flood/flood_internal.h"
#include "game/game_data.h"
#include "game/game_internal.h"
#include "graphics/graphics_primitives.h"
#include "graphics/image_buttons.h"
#include "graphics/image_collection.h"
#include "graphics/panel_draw.h"
#include "graphics/ui_layout.h"
#include "map/map_data.h"
#include "scenario/event_helpers.h"
#include "scenario/event_manager.h"
#include "scenario/requests.h"
#include "text/lang.h"
#include "window/advisors.h"
#include "window/window_id.h"
#include "window/window_image_group.h"
#include <stdint.h>

enum advisor_comment_layout {
    COMMENT_GREY_X = 32,
    COMMENT_GREY_Y = 60,
    COMMENT_GREY_WIDTH = 576,
    COMMENT_GREY_HEIGHT = 272,
    COMMENT_PANEL_X = 16,
    COMMENT_PANEL_Y = 144,
    COMMENT_PANEL_WIDTH_BLOCKS = 38,
    COMMENT_PANEL_BLOCK = 16,
    COMMENT_TEXT_X = 32,
    COMMENT_TEXT_Y = 160,
    COMMENT_TEXT_WIDTH = 612,
    COMMENT_HINT_WIDTH = 608,
    COMMENT_HINT_ABOVE_BOTTOM = 24,
    COMMENT_TUTORIAL_BUTTON_X = 32,
    COMMENT_TUTORIAL_BUTTON_Y = 272,
    GOD_DESCRIPTION_PANEL_BLOCKS = 13,
    COMMENT_PANEL_BLOCKS = 11,
    GOD_DESCRIPTION_TEXT_FIRST = 32,
};

enum troop_request_state {
    TROOP_REQUEST_ABROAD = 5,
    TROOP_REQUEST_RETURNING = 6,
    TROOP_REQUEST_RETURNING_LATE = 7,
};

int32_t advisor_comment_rules[ADVISOR_COMMENT_RULE_COUNT] = {
    [ADVISOR_COMMENT_RULE_SENTIMENT_LOW] = 50,
    [ADVISOR_COMMENT_RULE_SENTIMENT_FAIR] = 100,
    [ADVISOR_COMMENT_RULE_MIGRATION_MAX_ARMED_FIGURES] = 3,
    [ADVISOR_COMMENT_RULE_MIGRATION_NEWCOMERS_STRONG] = 5,
    [ADVISOR_COMMENT_RULE_MIGRATION_PERCENTAGE_STRONG] = 80,
    [ADVISOR_COMMENT_RULE_EMPLOYMENT_BALANCED_UNEMPLOYMENT] = 5,
    [ADVISOR_COMMENT_RULE_EMPLOYMENT_BALANCED_WORKERS_NEEDED] = 5,
    [ADVISOR_COMMENT_RULE_UNEMPLOYMENT_URGENT] = 17,
    [ADVISOR_COMMENT_RULE_UNEMPLOYMENT_SERIOUS] = 10,
    [ADVISOR_COMMENT_RULE_UNEMPLOYMENT_IMPORTANT] = 5,
    [ADVISOR_COMMENT_RULE_LABOR_SHORTAGE_URGENT] = 20,
    [ADVISOR_COMMENT_RULE_LABOR_SHORTAGE_SERIOUS] = 10,
    [ADVISOR_COMMENT_RULE_FOOD_STOCKS_LOW_PERCENT] = 50,
    [ADVISOR_COMMENT_RULE_FOOD_SUPPLY_PLENTY_PERCENT] = 150,
    [ADVISOR_COMMENT_RULE_FOOD_SUPPLY_ENOUGH_PERCENT] = 110,
    [ADVISOR_COMMENT_RULE_FOOD_SUPPLY_BARELY_ENOUGH_PERCENT] = 95,
    [ADVISOR_COMMENT_RULE_FOOD_SUPPLY_SLIGHTLY_SHORT_PERCENT] = 85,
    [ADVISOR_COMMENT_RULE_FOOD_SUPPLY_SHORT_PERCENT] = 70,
    [ADVISOR_COMMENT_RULE_ANGRY_GOD_MOOD] = 40,
    [ADVISOR_COMMENT_RULE_VERY_ANGRY_GOD_MOOD] = 20,
    [ADVISOR_COMMENT_RULE_HAPPY_GOD_MOOD] = 60,
    [ADVISOR_COMMENT_RULE_THEFTS_MANY] = 10,
    [ADVISOR_COMMENT_RULE_THEFTS_SEVERAL] = 5,
    [ADVISOR_COMMENT_RULE_THEFTS_SOME] = 2,
    [ADVISOR_COMMENT_RULE_REQUESTS_COUNTED] = 5,
    [ADVISOR_COMMENT_RULE_REQUESTS_SOME_MAX] = 2,
    [ADVISOR_COMMENT_RULE_REQUESTS_SEVERAL_MAX] = 4,
};

static void draw_comment_panel(int32_t group, int32_t text, int32_t height_blocks)
{
    int32_t panel_bottom = advisor_y + COMMENT_PANEL_Y + (height_blocks * COMMENT_PANEL_BLOCK);
    graphics_grey_rect(advisor_x + COMMENT_GREY_X, advisor_y + COMMENT_GREY_Y, COMMENT_GREY_WIDTH, COMMENT_GREY_HEIGHT, 0);
    outer_panel_draw(advisor_x + COMMENT_PANEL_X, advisor_y + COMMENT_PANEL_Y, COMMENT_PANEL_WIDTH_BLOCKS, height_blocks);
    lang_text_draw_multiline_ui(
        group, text, advisor_x + COMMENT_TEXT_X, advisor_y + COMMENT_TEXT_Y, COMMENT_TEXT_WIDTH, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_centered_ui(LANG_GROUP_WINDOW_HINTS, WINDOW_HINT_RIGHT_CLICK_TO_CONTINUE, advisor_x + COMMENT_PANEL_X,
        panel_bottom - COMMENT_HINT_ABOVE_BOTTOM, COMMENT_HINT_WIDTH, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
}

void window_go_god_description(void)
{
    window_current_id = WINDOW_ADVISOR_TEXT;
    window_redraw_request = 1;
}

void window_god_description_draw(void)
{
    int32_t text = button_parameter + GOD_DESCRIPTION_TEXT_FIRST;
    if (text < 0) {
        return;
    }
    draw_comment_panel(LANG_GROUP_RELIGION_ADVISOR, text, GOD_DESCRIPTION_PANEL_BLOCKS);
}

static int32_t sentiment_comment(const city_player *player, int32_t *advisor)
{
    switch (player->sentiment_low_mood_cause) {
        case LOW_MOOD_CAUSE_LACK_OF_FOOD:
        case LOW_MOOD_CAUSE_SLUMS:
            *advisor = ADVISOR_POPULATION;
            break;
        case LOW_MOOD_CAUSE_LACK_OF_JOBS:
        case LOW_MOOD_CAUSE_LOW_WAGES:
            *advisor = ADVISOR_LABOR;
            break;
        case LOW_MOOD_CAUSE_HIGH_TAXES:
            *advisor = ADVISOR_FINANCIAL;
            break;
    }
    if (player->sentiment < advisor_comment_rules[ADVISOR_COMMENT_RULE_SENTIMENT_LOW]) {
        return CHIEF_TEXT_LOW_SENTIMENT_CAUSE_FIRST + player->sentiment_low_mood_cause;
    }
    if (player->sentiment < advisor_comment_rules[ADVISOR_COMMENT_RULE_SENTIMENT_FAIR]) {
        *advisor = ADVISOR_NONE;
        return CHIEF_TEXT_LOW_SENTIMENT;
    }
    return -1;
}

static int32_t migration_comment(const city_player *player, int32_t *advisor)
{
    int32_t cause = player->no_immigration_cause;
    switch (cause) {
        case NO_IMMIGRATION_LOW_WAGES:
        case NO_IMMIGRATION_NO_JOBS:
            *advisor = ADVISOR_LABOR;
            break;
        case NO_IMMIGRATION_NO_FOOD:
        case NO_IMMIGRATION_HIGH_TAXES:
            *advisor = ADVISOR_POPULATION;
            break;
    }
    int32_t has_room = (player->refused_immigrants_today == 0) && (player->room_in_houses > 0);
    if (player->migration_direction == CITY_MIGRATION_IMMIGRATION) {
        if ((player->kingdom_soldiers + player->enemies) > advisor_comment_rules[ADVISOR_COMMENT_RULE_MIGRATION_MAX_ARMED_FIGURES]) {
            return -1;
        }
        if (player->migration_newcomers >= advisor_comment_rules[ADVISOR_COMMENT_RULE_MIGRATION_NEWCOMERS_STRONG]) {
            *advisor = ADVISOR_NONE;
            return CHIEF_TEXT_IMMIGRATION_RAISE_SENTIMENT;
        }
        if (!has_room) {
            *advisor = ADVISOR_POPULATION;
            return CHIEF_TEXT_IMMIGRATION_BUILD_HOUSING;
        }
        if (player->migration_percentage >= advisor_comment_rules[ADVISOR_COMMENT_RULE_MIGRATION_PERCENTAGE_STRONG]) {
            *advisor = ADVISOR_NONE;
            return CHIEF_TEXT_IMMIGRATION_RAISE_SENTIMENT;
        }
        return CHIEF_TEXT_IMMIGRATION_CAUSE_FIRST + cause;
    }
    if (player->migration_direction == CITY_MIGRATION_NONE) {
        if (has_room) {
            return CHIEF_TEXT_STAY_CAUSE_FIRST + cause;
        }
        *advisor = ADVISOR_POPULATION;
        return CHIEF_TEXT_MIGRATION_BUILD_HOUSING;
    }
    *advisor = ADVISOR_NONE;
    return CHIEF_TEXT_MIGRATION_BALANCE_WEALTH;
}

static int32_t employment_comment(const city_player *player)
{
    int32_t unemployment = player->unemployment_percentage;
    int32_t needed = player->workers_needed;
    if (unemployment <= advisor_comment_rules[ADVISOR_COMMENT_RULE_EMPLOYMENT_BALANCED_UNEMPLOYMENT] &&
        needed <= advisor_comment_rules[ADVISOR_COMMENT_RULE_EMPLOYMENT_BALANCED_WORKERS_NEEDED]) {
        return CHIEF_TEXT_EMPLOYMENT_BALANCED;
    }
    if (unemployment > advisor_comment_rules[ADVISOR_COMMENT_RULE_UNEMPLOYMENT_URGENT]) {
        return CHIEF_TEXT_UNEMPLOYMENT_URGENT;
    }
    if (unemployment > advisor_comment_rules[ADVISOR_COMMENT_RULE_UNEMPLOYMENT_SERIOUS]) {
        return CHIEF_TEXT_UNEMPLOYMENT_SERIOUS;
    }
    if (unemployment > advisor_comment_rules[ADVISOR_COMMENT_RULE_UNEMPLOYMENT_IMPORTANT]) {
        return CHIEF_TEXT_UNEMPLOYMENT_IMPORTANT;
    }
    if (unemployment > 0) {
        return CHIEF_TEXT_UNEMPLOYMENT_TOLERABLE;
    }
    if (needed > advisor_comment_rules[ADVISOR_COMMENT_RULE_LABOR_SHORTAGE_URGENT]) {
        return CHIEF_TEXT_LABOR_SHORTAGE_URGENT;
    }
    if (needed > advisor_comment_rules[ADVISOR_COMMENT_RULE_LABOR_SHORTAGE_SERIOUS]) {
        return CHIEF_TEXT_LABOR_SHORTAGE_SERIOUS;
    }
    return CHIEF_TEXT_LABOR_SHORTAGE_IMPORTANT;
}

static int32_t food_stocks_comment(const city_player *player)
{
    if (player->food_supply_months <= 0) {
        return CHIEF_TEXT_FOOD_STOCKS_EMPTY;
    }
    if (((player->food_stored_total * 100) / player->food_needed_per_month) < advisor_comment_rules[ADVISOR_COMMENT_RULE_FOOD_STOCKS_LOW_PERCENT]) {
        return CHIEF_TEXT_FOOD_STOCKS_LOW;
    }
    return CHIEF_TEXT_FOOD_STOCKS_ENOUGH;
}

static int32_t food_consumption_comment(const city_player *player)
{
    if (player->population == 0) {
        return CHIEF_TEXT_FOOD_SUPPLY_FAR_SHORT;
    }
    int32_t percentage = (city_resource_food_supply_percentage() * 100) / player->population;
    if (percentage > advisor_comment_rules[ADVISOR_COMMENT_RULE_FOOD_SUPPLY_PLENTY_PERCENT]) {
        return CHIEF_TEXT_FOOD_SUPPLY_PLENTY;
    }
    if (percentage > advisor_comment_rules[ADVISOR_COMMENT_RULE_FOOD_SUPPLY_ENOUGH_PERCENT]) {
        return CHIEF_TEXT_FOOD_SUPPLY_ENOUGH;
    }
    if (percentage > advisor_comment_rules[ADVISOR_COMMENT_RULE_FOOD_SUPPLY_BARELY_ENOUGH_PERCENT]) {
        return CHIEF_TEXT_FOOD_SUPPLY_BARELY_ENOUGH;
    }
    if (percentage > advisor_comment_rules[ADVISOR_COMMENT_RULE_FOOD_SUPPLY_SLIGHTLY_SHORT_PERCENT]) {
        return CHIEF_TEXT_FOOD_SUPPLY_SLIGHTLY_SHORT;
    }
    if (percentage > advisor_comment_rules[ADVISOR_COMMENT_RULE_FOOD_SUPPLY_SHORT_PERCENT]) {
        return CHIEF_TEXT_FOOD_SUPPLY_SHORT;
    }
    return CHIEF_TEXT_FOOD_SUPPLY_FAR_SHORT;
}

static int32_t angry_gods_text(int32_t text, int32_t lowest_mood)
{
    return (lowest_mood < advisor_comment_rules[ADVISOR_COMMENT_RULE_VERY_ANGRY_GOD_MOOD]) ? (text + 1) : text;
}

static int32_t religion_comment(const city_player *player)
{
    int32_t angry = 0;
    int32_t happy = 0;
    int32_t lowest = 100;
    for (int32_t god = 0; god < GOD_COUNT; god++) {
        int8_t mood = player->god_target_mood[god];
        if (mood < advisor_comment_rules[ADVISOR_COMMENT_RULE_ANGRY_GOD_MOOD]) {
            angry++;
        }
        if (mood < lowest) {
            lowest = mood;
        }
        if (mood > advisor_comment_rules[ADVISOR_COMMENT_RULE_HAPPY_GOD_MOOD]) {
            happy++;
        }
    }
    if (angry > 3) {
        return angry_gods_text(CHIEF_TEXT_GODS_SEVERAL_ANGRY, lowest);
    }
    if (angry == 3) {
        return angry_gods_text(CHIEF_TEXT_GODS_THREE_ANGRY, lowest);
    }
    if (angry == 2) {
        return angry_gods_text(CHIEF_TEXT_GODS_TWO_ANGRY, lowest);
    }
    if (angry == 1) {
        return angry_gods_text(CHIEF_TEXT_GODS_ONE_ANGRY, lowest);
    }
    if (happy == 0) {
        return CHIEF_TEXT_GODS_APPEASED;
    }
    if (happy == 1) {
        return CHIEF_TEXT_GODS_ONE_HAPPY;
    }
    if (happy == 2) {
        return CHIEF_TEXT_GODS_TWO_HAPPY;
    }
    return -1;
}

static int32_t finances_comment(const city_player *player)
{
    int32_t uncollected = player->yearly_uncollected_nobles + player->yearly_uncollected_plebs;
    int32_t collected = player->yearly_collected_nobles + player->yearly_collected_plebs;
    if (uncollected != 0 && (collected / uncollected) < 1) {
        return CHIEF_TEXT_TAXES_UNCOLLECTED;
    }
    if (player->treasury > player->balance_last_year) {
        return CHIEF_TEXT_TREASURY_RISING;
    }
    if (player->treasury < player->balance_last_year) {
        return CHIEF_TEXT_TREASURY_FALLING;
    }
    return CHIEF_TEXT_TREASURY_STEADY;
}

static int32_t crime_comment(const city_player *player)
{
    int32_t thefts = player->crimes_this_month;
    if (thefts >= advisor_comment_rules[ADVISOR_COMMENT_RULE_THEFTS_MANY]) {
        return CHIEF_TEXT_THEFTS_MANY;
    }
    if (thefts == 0) {
        return CHIEF_TEXT_THEFTS_NONE;
    }
    if (thefts < advisor_comment_rules[ADVISOR_COMMENT_RULE_THEFTS_SOME]) {
        return CHIEF_TEXT_THEFTS_FEW;
    }
    if (thefts < advisor_comment_rules[ADVISOR_COMMENT_RULE_THEFTS_SEVERAL]) {
        return CHIEF_TEXT_THEFTS_SOME;
    }
    return CHIEF_TEXT_THEFTS_SEVERAL;
}

static int32_t military_comment(const city_player *player)
{
    uint8_t troops_requested = 0;
    uint8_t troops_away = 0;
    for (scenario_event *request = scenario_request_next_active(&scenario_events, 0); request != 0;
        request = scenario_request_next_active(&scenario_events, request)) {
        if (request->item.value != RESOURCE_TROOPS) {
            continue;
        }
        if (scenario_request_is_waiting(request)) {
            troops_requested = 1;
        }
        int16_t state = request->state;
        if (state == TROOP_REQUEST_ABROAD || state == TROOP_REQUEST_RETURNING || state == TROOP_REQUEST_RETURNING_LATE) {
            troops_away = 1;
        }
    }
    if (player->kingdom_soldiers != 0) {
        return CHIEF_TEXT_KINGDOM_SOLDIERS_AT_CITY;
    }
    if (player->enemies != 0) {
        return CHIEF_TEXT_ENEMIES_AT_CITY;
    }
    if (scenario_invasion_running() != 0) {
        return CHIEF_TEXT_ENEMY_ADVANCING;
    }
    if (troops_away) {
        return CHIEF_TEXT_TROOPS_ABROAD;
    }
    if (troops_requested) {
        return CHIEF_TEXT_TROOPS_REQUESTED;
    }
    return (player->soldiers >= 0) ? CHIEF_TEXT_MILITARY_WATCHING : CHIEF_TEXT_MILITARY_NO_FORCES;
}

static int32_t kingdom_comment(void)
{
    int32_t count = 0;
    for (scenario_event *request = scenario_request_next_active(&scenario_events, 0); request != 0;
        request = scenario_request_next_active(&scenario_events, request)) {
        if (!scenario_request_is_waiting(request)) {
            continue;
        }
        if (count >= advisor_comment_rules[ADVISOR_COMMENT_RULE_REQUESTS_COUNTED]) {
            break;
        }
        if (request->item.value != 0) {
            count++;
        }
    }
    if (count == 0) {
        return CHIEF_TEXT_REQUESTS_NONE;
    }
    if (count <= advisor_comment_rules[ADVISOR_COMMENT_RULE_REQUESTS_SOME_MAX]) {
        return CHIEF_TEXT_REQUESTS_SOME;
    }
    if (count <= advisor_comment_rules[ADVISOR_COMMENT_RULE_REQUESTS_SEVERAL_MAX]) {
        return CHIEF_TEXT_REQUESTS_SEVERAL;
    }
    return CHIEF_TEXT_REQUESTS_MANY;
}

static int32_t nilometer_comment(void)
{
    return CHIEF_TEXT_FLOOD_FORECAST_FIRST + ((floodplain_settings_get_quality(&floods_data.settings) + 19) / 20);
}

void window_advisor_comment_draw(void)
{
    city_player *player = city_player_current();
    int32_t text = -1;
    int32_t advisor = ADVISOR_NONE;
    switch (button_parameter) {
        case ADVISOR_COMMENT_SENTIMENT:
            text = sentiment_comment(player, &advisor);
            break;
        case ADVISOR_COMMENT_MIGRATION:
            text = migration_comment(player, &advisor);
            break;
        case ADVISOR_COMMENT_EMPLOYMENT:
            advisor = ADVISOR_LABOR;
            text = employment_comment(player);
            break;
        case ADVISOR_COMMENT_FOOD_STOCKS:
            advisor = ADVISOR_POPULATION;
            text = food_stocks_comment(player);
            break;
        case ADVISOR_COMMENT_FOOD_CONSUMPTION:
            advisor = ADVISOR_POPULATION;
            text = food_consumption_comment(player);
            break;
        case ADVISOR_COMMENT_HEALTH:
            advisor = ADVISOR_HEALTH;
            text = CHIEF_TEXT_HEALTH_FIRST + (player->health / 10);
            break;
        case ADVISOR_COMMENT_RELIGION:
            advisor = ADVISOR_RELIGION;
            text = religion_comment(player);
            break;
        case ADVISOR_COMMENT_FINANCES:
            advisor = ADVISOR_FINANCIAL;
            text = finances_comment(player);
            break;
        case ADVISOR_COMMENT_CRIME:
            advisor = ADVISOR_FINANCIAL;
            text = crime_comment(player);
            break;
        case ADVISOR_COMMENT_MILITARY:
            advisor = ADVISOR_MILITARY;
            text = military_comment(player);
            break;
        case ADVISOR_COMMENT_KINGDOM:
            advisor = ADVISOR_KINGDOM;
            text = kingdom_comment();
            break;
        case ADVISOR_COMMENT_NILOMETER:
            advisor = ADVISOR_POPULATION;
            text = nilometer_comment();
            break;
        default:
            break;
    }
    player->advisor_comment = advisor;
    if (text < 0) {
        return;
    }
    draw_comment_panel(LANG_GROUP_CHIEF_ADVISOR, text, COMMENT_PANEL_BLOCKS);
    if (tutorial_flags_b[advisor] != 0) {
        image_buttons_draw(advisor_x + COMMENT_TUTORIAL_BUTTON_X, advisor_y + COMMENT_TUTORIAL_BUTTON_Y, tutorial_buttons[advisor], 1);
    }
}
