// The advisors window (window 7): opening an advisor from the sidebar, the hotkeys or the advisor bar (in a tutorial
// mission only the advisors it makes available), the city figures the advisors show, computed again on each opening,
// the background with the advisor bar, the foreground and the advisors' help buttons.
//
// The advisor bar is not complete:
// - the tutorial flags enable 13 of the bar's 14 buttons;
// - the fleet advisor has no button of its own and shows the military advisor's button pressed.

#include "window/advisors.h"
#include "building/building_stocks.h"
#include "city/culture.h"
#include "city/finance.h"
#include "city/finance_estimates.h"
#include "city/kingdom.h"
#include "city/labor.h"
#include "city/message.h"
#include "city/ratings.h"
#include "figure/action.h"
#include "figure/formation.h"
#include "game/game_data.h"
#include "game/game_internal.h"
#include "graphics/button.h"
#include "graphics/graphics_data.h"
#include "graphics/image_buttons.h"
#include "graphics/image_collection.h"
#include "graphics/image_draw.h"
#include "graphics/label_animation_slots.h"
#include "widget/widget_data.h"
#include "window/advisor/chief.h"
#include "window/advisor/education.h"
#include "window/advisor/entertainment.h"
#include "window/advisor/financial.h"
#include "window/advisor/health.h"
#include "window/advisor/labor.h"
#include "window/advisor/military.h"
#include "window/advisor/population.h"
#include "window/advisor/ratings.h"
#include "window/advisor/religion.h"
#include "window/advisor/trade.h"
#include "window/advisors_fleet.h"
#include "window/advisors_kingdom.h"
#include "window/advisors_monuments.h"
#include "window/message_dialog.h"
#include "window/window_id.h"
#include <stdbool.h>
#include <stdint.h>

enum advisors_layout {
    ADVISORS_PICTURE_WIDTH = 1024,
    ADVISORS_PICTURE_HEIGHT = 768,
    ADVISOR_BAR_Y = 432,
};

enum advisor_bar {
    ADVISOR_BAR_TUTORIAL_BUTTONS = 13,
    ADVISOR_BAR_IMAGES_PER_BUTTON = 4,
    ADVISOR_BAR_PRESSED_IMAGE = 2,
};

int32_t advisor_help_messages[ADVISOR_COUNT] = {
    [ADVISOR_LABOR] = 20,
    [ADVISOR_MILITARY] = 21,
    [ADVISOR_FLEET] = 21,
    [ADVISOR_KINGDOM] = 22,
    [ADVISOR_RATINGS] = 23,
    [ADVISOR_TRADE] = 24,
    [ADVISOR_POPULATION] = 25,
    [ADVISOR_HEALTH] = 26,
    [ADVISOR_EDUCATION] = 27,
    [ADVISOR_ENTERTAINMENT] = 28,
    [ADVISOR_RELIGION] = 29,
    [ADVISOR_FINANCIAL] = 30,
    [ADVISOR_CHIEF] = 31,
    [ADVISOR_MONUMENTS] = 373,
};

void advisor_button_help(void)
{
    if (advisor_shown <= ADVISOR_NONE || advisor_shown > ADVISOR_MONUMENTS) {
        return;
    }
    window_message_dialog_show(advisor_help_messages[advisor_shown], 1, 0);
    window_redraw_request = 1;
}

void button_advisor_bar(void)
{
    int32_t advisor = button_parameter;
    if (advisor == ADVISOR_NONE) {
        window_current_id = WINDOW_CITY;
        window_redraw_request = 1;
        return;
    }
    if (settings_data.is_custom_scenario != 0 || tutorial_flags_b[advisor] != 0) {
        advisor_shown = advisor;
        settings_data.last_advisor = advisor;
        window_redraw_request = 1;
    }
}

void window_advisors_draw_background(void)
{
    image_view_x = (ADVISORS_PICTURE_WIDTH - g_screen_width) / 2;
    image_view_width = g_screen_width;
    image_view_y = (ADVISORS_PICTURE_HEIGHT - g_screen_height) / 2;
    image_view_height = g_screen_height;
    if (window_redraw_request == 1) {
        image_draw(image_group(IMAGE_GROUP_ADVISOR_BACKGROUND), 0, 0);
    }
    int32_t x = ui_origin_x;
    int32_t y = ui_origin_y + ADVISOR_BAR_Y;
    image_draw(image_group(IMAGE_GROUP_ADVISOR_BAR_BACKGROUND), x, y);
    image_button *buttons = advisor_bar_buttons;
    for (int32_t i = 0; i < ADVISOR_BAR_TUTORIAL_BUTTONS; i++) {
        buttons[i].enabled = tutorial_flags_b[buttons[i].parameter1] != 0;
    }
    image_buttons_draw(x, y, buttons, ADVISOR_BAR_BUTTONS);
    int32_t advisor = advisor_shown;
    int32_t pressed = (advisor >= ADVISOR_FLEET) ? (advisor - 2) : (advisor - 1);
    int32_t pressed_image = image_group(IMAGE_GROUP_ADVISOR_BAR) + (pressed * ADVISOR_BAR_IMAGES_PER_BUTTON) + ADVISOR_BAR_PRESSED_IMAGE;
    image_draw(pressed_image, buttons[pressed].x + x, buttons[pressed].y + y);
}

void window_advisors_draw_advisor(void)
{
    window_advisors_draw_background();
    switch (advisor_shown) {
        case ADVISOR_LABOR:
            window_advisor_labor_draw_background();
            break;
        case ADVISOR_MILITARY:
            window_advisor_military_draw_background();
            break;
        case ADVISOR_FLEET:
            window_advisor_fleet_draw_background();
            break;
        case ADVISOR_KINGDOM:
            window_advisor_kingdom_draw_background();
            break;
        case ADVISOR_RATINGS:
            window_advisor_ratings_draw_background();
            break;
        case ADVISOR_TRADE:
            window_advisor_trade_draw_background();
            break;
        case ADVISOR_POPULATION:
            window_advisor_population_draw_background();
            break;
        case ADVISOR_HEALTH:
            window_advisor_health_draw_background();
            break;
        case ADVISOR_EDUCATION:
            window_advisor_education_draw_background();
            break;
        case ADVISOR_ENTERTAINMENT:
            window_advisor_entertainment_draw_background();
            break;
        case ADVISOR_RELIGION:
            window_advisor_religion_draw_background();
            break;
        case ADVISOR_FINANCIAL:
            window_advisor_financial_draw_background();
            break;
        case ADVISOR_CHIEF:
            window_advisor_chief_draw_background();
            break;
        case ADVISOR_MONUMENTS:
            window_advisor_monuments_draw_background();
            break;
        default:
            break;
    }
}

void city_victory_update_win_houses(void)
{
    city_houses_of_win_level = city_victory_count_houses_of_level(scenario_data.win_houses_level);
}

void window_advisors_init(void)
{
    city_labor_allocate_workers_to_categories();
    city_labor_set_building_worker_weight();
    city_labor_allocate_workers_to_buildings();
    city_finance_estimate_taxes();
    city_finance_estimate_wages();
    city_finance_estimate_interest();
    city_finance_estimate_salary();
    city_finance_calculate_totals();
    city_finance_calculate_tribute();
    city_ratings_advisor_reasons();
    city_culture_update_coverage();
    city_resource_calculate_food_stocks_and_supply();
    formation_calculate_figures();
    advisor_military_count_ships();
    city_ratings_update_culture_advisor();
    city_ratings_prosperity_explanation();
    city_ratings_kingdom_explanation();
    city_victory_update_win_houses();
}

static void advisors_open(int32_t advisor)
{
    window_current_id = WINDOW_ADVISORS;
    advisor_shown = advisor;
    city_player_current()->trade_prices_mode = 0;
    window_advisors_init();
    window_redraw_request = 1;
    label_animation[LABEL_SLOT_ADVISOR_OPEN] = ADVISOR_OPEN_DONE;
}

void window_advisors_show_checked(void)
{
    int32_t advisor = button_parameter;
    if (settings_data.is_custom_scenario == 0 && tutorial_flags_b[advisor] == 0) {
        if (window_current_id == WINDOW_MESSAGE_DIALOG) {
            window_message_dialog_close();
            window_current_id = WINDOW_CITY;
        }
        city_warning_show(WARNING_NOT_AVAILABLE_IN_THIS_ASSIGNMENT);
        return;
    }
    settings_data.last_advisor = advisor;
    advisors_open(advisor);
}

static int32_t next_advisor(int32_t advisor)
{
    advisor++;
    return (advisor >= ADVISOR_COUNT) ? ADVISOR_LABOR : advisor;
}

bool tutorial_advisor_find_available(int32_t *advisor)
{
    int32_t start = *advisor;
    if (tutorial_flags_b[start] != 0) {
        return true;
    }
    for (*advisor = next_advisor(start); *advisor != start; *advisor = next_advisor(*advisor)) {
        if (tutorial_flags_b[*advisor] != 0) {
            return true;
        }
    }
    return false;
}

void sidebar_button_advisors(void)
{
    if (settings_data.is_custom_scenario == 0 && !tutorial_advisor_find_available(&settings_data.last_advisor)) {
        city_warning_show(WARNING_NOT_AVAILABLE_IN_THIS_ASSIGNMENT);
        return;
    }
    advisors_open(settings_data.last_advisor);
}

void window_advisors_draw_foreground(void)
{
    if (label_animation[LABEL_SLOT_ADVISOR_OPEN] <= ADVISOR_OPEN_LAST_STEP) {
        label_animation[LABEL_SLOT_ADVISOR_OPEN] = ADVISOR_OPEN_DONE;
        window_redraw_request = 1;
    }
}
