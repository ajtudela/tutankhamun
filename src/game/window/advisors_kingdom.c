// The kingdom advisor and its dialogs: the kingdom rating, the waiting requests (troops by land or sea, goods and
// deben with the months left), savings, salary, donation and gift, the answer to a request, and window 0x4C, the
// burial provisions sent to a royal tomb. Building stones are kept in hundredths and shown in loads; sending the
// provisions that meet the victory goals marks every royal tomb whose main part is complete as finished.

#include "window/advisors_kingdom.h"
#include "building/building.h"
#include "building/building_storage.h"
#include "building/building_type.h"
#include "building/monument_object.h"
#include "building/royal_tomb.h"
#include "city/city_data.h"
#include "city/finance.h"
#include "city/finance_estimates.h"
#include "city/kingdom.h"
#include "city/population_extra.h"
#include "city/ratings.h"
#include "city/resource_type.h"
#include "empire/empire.h"
#include "game/game_internal.h"
#include "graphics/graphics_primitives.h"
#include "graphics/image_collection.h"
#include "graphics/image_draw.h"
#include "graphics/label_animation_slots.h"
#include "graphics/panel_draw.h"
#include "graphics/text_draw.h"
#include "graphics/ui_layout.h"
#include "input/input_data.h"
#include "input/mouse.h"
#include "io/game_file_internal.h"
#include "scenario/event_helpers.h"
#include "scenario/event_manager.h"
#include "scenario/requests.h"
#include "scenario/scenario_internal.h"
#include "text/lang.h"
#include "window/advisors.h"
#include "window/advisors_fleet.h"
#include "window/advisors_monuments.h"
#include "window/popup_dialog.h"
#include "window/window_id.h"
#include "window/window_image_group.h"
#include <stdint.h>

enum kingdom_layout {
    KINGDOM_WIDTH_BLOCKS = 40,
    KINGDOM_HEIGHT_BLOCKS = 27,
    KINGDOM_BLOCK = 16,
    REQUEST_ROWS = 5,
    REQUEST_ROW_HEIGHT = 42,
    REQUEST_ROWS_Y = 96,
    REQUEST_BUTTON_X = 38,
    REQUEST_BUTTON_WIDTH = 560,
    REQUEST_BUTTON_HEIGHT = 40,
    DIALOG_BUTTON_HEIGHT = 20,
};

enum kingdom_button {
    KINGDOM_BUTTON_GIVE_TO_CITY = 1,
    KINGDOM_BUTTON_SALARY = 2,
    KINGDOM_BUTTON_SEND_GIFT = 3,
    KINGDOM_BUTTON_FIRST_REQUEST = 4,
    SALARY_BUTTON_CANCEL = 1,
    SALARY_BUTTON_FIRST_RANK = 2,
    DONATE_BUTTON_CANCEL = 1,
    DONATE_BUTTON_GIVE = 2,
    DONATE_BUTTON_FIRST_AMOUNT = 3,
    GIFT_BUTTON_FIRST_SIZE = 1,
    GIFT_BUTTON_SEND = 4,
    GIFT_BUTTON_CANCEL = 5,
    PROVISIONS_BUTTON_ALL = 1,
    PROVISIONS_BUTTON_SEND = 2,
    PROVISIONS_BUTTON_BACK = 3,
    PROVISIONS_ARROW_MORE = 10,
    PROVISIONS_ARROW_LESS = 11,
};

enum kingdom_amounts {
    PLAYER_RANKS = 11,
    GIFTS_PER_SIZE = 4,
    DONATION_AMOUNTS = 5,
    DONATION_ARROW_STEP = 10,
    SMALL_FORCE_BELOW = 46,
    LARGE_FORCE_FROM = 89,
    RESOURCE_ICON_DEBEN = 21,
    RESOURCE_UNIT_NAMES = 54,
    STORAGE_UNITS_PER_LOAD = 100,
    TOMB_PROGRESS_DONE = 100,
};

static const int32_t donation_amounts[DONATION_AMOUNTS] = { 0, 500, 2000, 5000, 1000000 };

static int32_t resource_in_hundredths(int32_t resource)
{
    switch (resource) {
        case RESOURCE_STONE:
        case RESOURCE_LIMESTONE:
        case RESOURCE_GRANITE:
        case RESOURCE_SANDSTONE:
        case RESOURCE_MARBLE:
            return 1;
        default:
            return 0;
    }
}

static int32_t resource_counted_singly(int32_t resource)
{
    return resource == RESOURCE_CHARIOTS || resource == RESOURCE_WEAPONS;
}

static int32_t requested_amount_shown(int32_t resource, int32_t amount)
{
    if (!resource_in_hundredths(resource) && !resource_counted_singly(resource) && resource != RESOURCE_DEBEN) {
        return amount * STORAGE_UNITS_PER_LOAD;
    }
    return amount;
}

static void draw_kingdom_text(int32_t text, int32_t x, int32_t y, int32_t font)
{
    lang_text_draw_ui(LANG_GROUP_KINGDOM_ADVISOR, text, x, y, font, 0, ui_origin_x, ui_origin_y);
}

static void draw_number(int32_t value, const char *postfix, int32_t x, int32_t y, int32_t font)
{
    text_draw_number_ui(value, NUMBER_PREFIX_NONE, postfix, x, y, font, 0, ui_origin_x, ui_origin_y);
}

static void draw_troop_request_row(scenario_event *e, int32_t row_y)
{
    int32_t city = empire_city_get_for_trade_route(e->route.value);
    int32_t amount = e->amount.value;
    int32_t months = scenario_event_months_margin(e);
    button_border_draw_ui(advisor_x + REQUEST_BUTTON_X, row_y, REQUEST_BUTTON_WIDTH, REQUEST_BUTTON_HEIGHT, 0, ui_origin_x, ui_origin_y);
    int32_t route = e->route.value;
    int32_t by_sea = 0;
    if (route >= 0 && route < EMPIRE_ROUTES && empire_map_routes.routes[route].kind == EMPIRE_ROUTE_KIND_SEA) {
        by_sea = 1;
    }
    image_draw(image_group(IMAGE_GROUP_TROOP_REQUEST_ICONS) + by_sea, advisor_x + 50, row_y + 10);
    g_text_width = 0;
    draw_kingdom_text(by_sea ? KINGDOM_TEXT_DISPATCH_WATERBORNE_TROOPS_TO : KINGDOM_TEXT_DISPATCH_TROOPS_TO, advisor_x + 80, row_y + 6, image_group(IMAGE_GROUP_FONT_WHITE));
    int32_t city_names = (settings_data.classical_names != 0) ? LANG_GROUP_CITY_NAMES_CLASSICAL : LANG_GROUP_CITY_NAMES;
    lang_text_draw_ui(city_names, empire_city_get(city)->name_id, g_text_width + advisor_x + 80, row_y + 6, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
    g_text_width = 0;
    int32_t force_text;
    if (amount < SMALL_FORCE_BELOW) {
        force_text = KINGDOM_TEXT_SMALL_FORCE_ATTACKS_IN;
    } else if (amount < LARGE_FORCE_FROM) {
        force_text = KINGDOM_TEXT_AVERAGE_FORCE_ATTACKS_IN;
    } else {
        force_text = KINGDOM_TEXT_LARGE_FORCE_ATTACKS_IN;
    }
    draw_kingdom_text(force_text, advisor_x + 80, row_y + 24, image_group(IMAGE_GROUP_FONT_WHITE));
    lang_text_draw_amount(LANG_GROUP_AMOUNTS, AMOUNT_TEXT_MONTH, months, g_text_width + advisor_x + 80, row_y + 24, image_group(IMAGE_GROUP_FONT_WHITE), 0);
}

static void draw_goods_request_row(const city_player *player, scenario_event *e, int32_t row_y)
{
    int32_t resource = e->item.value;
    int32_t amount = requested_amount_shown(resource, e->amount.value);
    button_border_draw_ui(advisor_x + REQUEST_BUTTON_X, row_y, REQUEST_BUTTON_WIDTH, REQUEST_BUTTON_HEIGHT, 0, ui_origin_x, ui_origin_y);
    int32_t icon = (resource == RESOURCE_DEBEN) ? (image_group(IMAGE_GROUP_RESOURCE_ICONS) + RESOURCE_ICON_DEBEN) : (image_group(IMAGE_GROUP_RESOURCE_ICONS) + resource);
    image_draw(icon, advisor_x + 40, row_y + 4);
    g_text_width = 0;
    draw_number(amount, postfix_none, advisor_x + 66, row_y + 6, image_group(IMAGE_GROUP_FONT_WHITE));
    int32_t name = resource;
    if (resource_in_hundredths(resource) || (resource_counted_singly(resource) && amount > 1)) {
        name = resource + RESOURCE_UNIT_NAMES;
    }
    lang_text_draw_ui(LANG_GROUP_RESOURCE_NAMES, name, g_text_width + advisor_x + 66, row_y + 6, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_amount(LANG_GROUP_AMOUNTS, AMOUNT_TEXT_MONTH, e->months_left, advisor_x + 310, row_y + 6, image_group(IMAGE_GROUP_FONT_WHITE), 0);
    lang_text_draw_ui(
        LANG_GROUP_MESSAGE_BUTTONS, MESSAGE_BUTTON_TEXT_TO_COMPLY, g_text_width + advisor_x + 310, row_y + 6, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
    g_text_width = 0;
    int32_t have;
    int32_t have_text;
    if (resource == RESOURCE_DEBEN) {
        have = player->treasury;
        have_text = KINGDOM_TEXT_IN_CITY_TREASURY;
    } else {
        have = player->storage_stored[resource];
        if (resource_in_hundredths(resource) || resource_counted_singly(resource)) {
            have /= STORAGE_UNITS_PER_LOAD;
        }
        have_text = KINGDOM_TEXT_IN_STORAGE_YARDS;
    }
    draw_number(have, postfix_none, advisor_x + 40, row_y + 24, image_group(IMAGE_GROUP_FONT_WHITE));
    draw_kingdom_text(have_text, g_text_width + advisor_x + 40, row_y + 24, image_group(IMAGE_GROUP_FONT_WHITE));
    int32_t status_text = (have < amount) ? KINGDOM_TEXT_UNABLE_TO_FULFILL : KINGDOM_TEXT_CLICK_TO_DISPATCH;
    draw_kingdom_text(status_text, g_text_width + advisor_x + 80, row_y + 24, image_group(IMAGE_GROUP_FONT_WHITE));
}

static void draw_advisor_frame(int32_t frame)
{
    graphics_draw_rect((frame * KINGDOM_BLOCK) + ui_origin_x, ui_origin_y + (frame * 8), (KINGDOM_WIDTH_BLOCKS - (frame * 2)) * KINGDOM_BLOCK,
        (KINGDOM_HEIGHT_BLOCKS - (frame * 2)) * KINGDOM_BLOCK, 0);
}

void window_advisor_kingdom_draw_background(void)
{
    city_kingdom_gift_costs_update();
    int32_t frame = ADVISOR_OPEN_LAST_STEP - label_animation[LABEL_SLOT_ADVISOR_OPEN];
    advisor_x = ui_origin_x;
    advisor_y = ui_origin_y;
    advisor_height_blocks = KINGDOM_HEIGHT_BLOCKS;
    if (frame >= ADVISOR_OPEN_LAST_STEP) {
        return;
    }
    if (frame > 0) {
        draw_advisor_frame(frame);
        return;
    }
    const city_player *player = city_player_current();
    outer_panel_draw(ui_origin_x, ui_origin_y, KINGDOM_WIDTH_BLOCKS, KINGDOM_HEIGHT_BLOCKS);
    image_draw(image_group(IMAGE_GROUP_ADVISOR_ICONS) + 2, advisor_x + 10, advisor_y + 10);
    g_text_width = 0;
    draw_kingdom_text(KINGDOM_TEXT_POLITICAL_OVERSEER_FOR, advisor_x + 60, advisor_y + 12, image_group(IMAGE_GROUP_FONT_LARGE));
    text_draw((const uint8_t *) message_token_player_name(), g_text_width + advisor_x + 60, advisor_y + 12, image_group(IMAGE_GROUP_FONT_LARGE), 0);
    g_text_width = 0;
    draw_kingdom_text(KINGDOM_TEXT_RATING, advisor_x + 60, advisor_y + 44, image_group(IMAGE_GROUP_FONT_BLACK));
    draw_number(player->rating_kingdom, postfix_none, g_text_width + advisor_x + 60, advisor_y + 44, image_group(IMAGE_GROUP_FONT_BLACK));
    lang_text_draw_multiline_ui(LANG_GROUP_KINGDOM_ADVISOR, KINGDOM_TEXT_RATING_OPINION_FIRST + (player->rating_kingdom / 5), advisor_x + 60, advisor_y + 60,
        544, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    inner_panel_draw_ui(advisor_x + 32, advisor_y + 90, 36, 14, ui_origin_x, ui_origin_y);
    int32_t rows = 0;
    for (scenario_event *e = scenario_request_next_active(&scenario_events, NULL); e != NULL;
        e = scenario_request_next_active(&scenario_events, e)) {
        if (scenario_request_is_waiting(e) == 0) {
            continue;
        }
        if (rows >= REQUEST_ROWS) {
            break;
        }
        int32_t row_y = advisor_y + REQUEST_ROWS_Y + (rows * REQUEST_ROW_HEIGHT);
        if (e->item.value == RESOURCE_TROOPS) {
            draw_troop_request_row(e, row_y);
        } else {
            draw_goods_request_row(player, e, row_y);
        }
        rows++;
    }
    if (rows != 0) {
        return;
    }
    lang_text_draw_multiline_ui(
        LANG_GROUP_KINGDOM_ADVISOR, KINGDOM_TEXT_NO_REQUESTS, advisor_x + 64, advisor_y + 160, 512, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
}

void window_advisor_kingdom_draw_foreground(void)
{
    if (label_animation[LABEL_SLOT_ADVISOR_OPEN] <= ADVISOR_OPEN_LAST_STEP) {
        return;
    }
    const city_player *player = city_player_current();
    inner_panel_draw_ui(advisor_x + 64, advisor_y + 324, 32, 6, ui_origin_x, ui_origin_y);
    lang_text_draw_ui(LANG_GROUP_PLAYER_RANKS, player->player_rank, advisor_x + 72, advisor_y + 338, image_group(IMAGE_GROUP_FONT_LARGE_B), 0, ui_origin_x, ui_origin_y);
    g_text_width = 0;
    draw_kingdom_text(KINGDOM_TEXT_SAVINGS_OF, advisor_x + 72, advisor_y + 372, image_group(IMAGE_GROUP_FONT_WHITE));
    draw_number(player->personal_savings, money_format, g_text_width + advisor_x + 80, advisor_y + 372, image_group(IMAGE_GROUP_FONT_WHITE));
    g_text_width = 0;
    draw_number(player->savings_lost, postfix_none, advisor_x + 250, advisor_y + 372, image_group(IMAGE_GROUP_FONT_WHITE));
    draw_kingdom_text(KINGDOM_TEXT_DEBENS_LOST_TO_THEFT, g_text_width + advisor_x + 250, advisor_y + 372, image_group(IMAGE_GROUP_FONT_WHITE));
    button_border_draw_ui(
        advisor_x + 320, advisor_y + 352, 250, DIALOG_BUTTON_HEIGHT, focus_button_id == KINGDOM_BUTTON_GIVE_TO_CITY, ui_origin_x, ui_origin_y);
    lang_text_draw_centered_ui(
        LANG_GROUP_KINGDOM_ADVISOR, KINGDOM_TEXT_GIVE_TO_CITY, advisor_x + 320, advisor_y + 357, 250, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
    if (player->mansion_placed == 0) {
        draw_kingdom_text(KINGDOM_TEXT_NEED_MANSION, advisor_x + 100, advisor_y + 398, image_group(IMAGE_GROUP_FONT_WHITE));
    } else {
        button_border_draw_ui(
            advisor_x + 70, advisor_y + 393, 500, DIALOG_BUTTON_HEIGHT, focus_button_id == KINGDOM_BUTTON_SALARY, ui_origin_x, ui_origin_y);
        g_text_width = 0;
        draw_kingdom_text(KINGDOM_TEXT_SALARY_OF_RANK_FIRST + player->salary_rank, advisor_x + 120, advisor_y + 398, image_group(IMAGE_GROUP_FONT_WHITE));
        draw_number(player->salary_amount, postfix_none, g_text_width + advisor_x + 120, advisor_y + 398, image_group(IMAGE_GROUP_FONT_WHITE));
        draw_kingdom_text(KINGDOM_TEXT_DEBEN_PER_MONTH, g_text_width + advisor_x + 120, advisor_y + 398, image_group(IMAGE_GROUP_FONT_WHITE));
    }
    button_border_draw_ui(
        advisor_x + 320, advisor_y + 330, 250, DIALOG_BUTTON_HEIGHT, focus_button_id == KINGDOM_BUTTON_SEND_GIFT, ui_origin_x, ui_origin_y);
    lang_text_draw_centered_ui(
        LANG_GROUP_KINGDOM_ADVISOR, KINGDOM_TEXT_SEND_GIFT, advisor_x + 320, advisor_y + 335, 250, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
    for (int32_t i = 0; i < REQUEST_ROWS; i++) {
        int32_t focus = focus_button_id == (KINGDOM_BUTTON_FIRST_REQUEST + i);
        if (scenario_request_status_button(i) != SCENARIO_REQUEST_NONE) {
            button_border_draw_ui(advisor_x + REQUEST_BUTTON_X, advisor_y + REQUEST_ROWS_Y + (i * REQUEST_ROW_HEIGHT), REQUEST_BUTTON_WIDTH,
                REQUEST_BUTTON_HEIGHT, focus, ui_origin_x, ui_origin_y);
        }
    }
}

void window_set_salary_draw(void)
{
    const city_player *player = city_player_current();
    outer_panel_draw(advisor_x + 128, advisor_y + 32, 24, 25);
    image_draw(image_group(IMAGE_GROUP_RESOURCE_ICONS) + RESOURCE_ICON_DEBEN, advisor_x + 144, advisor_y + 48);
    lang_text_draw_centered_ui(
        LANG_GROUP_KINGDOM_ADVISOR, KINGDOM_TEXT_SET_SALARY_LEVEL, advisor_x + 144, advisor_y + 48, 368, image_group(IMAGE_GROUP_FONT_LARGE), 0, ui_origin_x, ui_origin_y);
    inner_panel_draw_ui(advisor_x + 144, advisor_y + 80, 22, 15, ui_origin_x, ui_origin_y);
    int32_t dy = 0;
    for (int32_t rank = 0; rank < PLAYER_RANKS; rank++) {
        g_text_width = 0;
        int32_t font = (focus_button_id == (SALARY_BUTTON_FIRST_RANK + rank)) ? image_group(IMAGE_GROUP_FONT_RED) : image_group(IMAGE_GROUP_FONT_WHITE);
        draw_kingdom_text(KINGDOM_TEXT_SALARY_OF_RANK_FIRST + rank, advisor_x + 176, dy + advisor_y + 90, font);
        draw_number(salary_by_rank[rank], (const char *) money_format, g_text_width + advisor_x + 176, dy + advisor_y + 90, font);
        dy += 20;
    }
    int32_t text;
    if (player->mission_continue_months_left > 0) {
        text = KINGDOM_TEXT_SALARY_FORBIDDEN;
    } else if (player->salary_rank > player->player_rank) {
        text = KINGDOM_TEXT_SALARY_ABOVE_RANK;
    } else {
        text = KINGDOM_TEXT_FREE_TO_SET_SALARY;
    }
    lang_text_draw_multiline_ui(LANG_GROUP_KINGDOM_ADVISOR, text, advisor_x + 152, advisor_y + 336, 336, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    button_border_draw_ui(advisor_x + 240, advisor_y + 395, 160, DIALOG_BUTTON_HEIGHT, focus_button_id == SALARY_BUTTON_CANCEL, ui_origin_x, ui_origin_y);
    lang_text_draw_centered_ui(LANG_GROUP_WINDOW_HINTS, WINDOW_HINT_CANCEL, advisor_x + 176, advisor_y + 400, 288, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
}

void window_donate_to_city_draw_background(void)
{
    outer_panel_draw(advisor_x + 64, advisor_y + 160, 32, 10);
    image_draw(image_group(IMAGE_GROUP_RESOURCE_ICONS) + RESOURCE_ICON_DEBEN, advisor_x + 96, advisor_y + 176);
    lang_text_draw_centered_ui(
        LANG_GROUP_KINGDOM_ADVISOR, KINGDOM_TEXT_GIVE_MONEY_TO_CITY, advisor_x + 80, advisor_y + 176, 496, image_group(IMAGE_GROUP_FONT_LARGE), 0, ui_origin_x, ui_origin_y);
    inner_panel_draw_ui(advisor_x + 112, advisor_y + 208, 26, 4, ui_origin_x, ui_origin_y);
    for (int32_t i = 0; i < DONATION_AMOUNTS - 1; i++) {
        text_draw_number_centered(donation_amounts[i], NUMBER_PREFIX_NONE, postfix_none, advisor_x + 124 + (i * 80), advisor_y + 221, 64, image_group(IMAGE_GROUP_FONT_WHITE), 0);
    }
    lang_text_draw_centered_ui(LANG_GROUP_KINGDOM_ADVISOR, KINGDOM_TEXT_ALL, advisor_x + 444, advisor_y + 221, 64, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
    draw_kingdom_text(KINGDOM_TEXT_DONATION_IS, advisor_x + 128, advisor_y + 248, image_group(IMAGE_GROUP_FONT_WHITE));
    draw_number(city_player_current()->donate_amount, postfix_none, advisor_x + 316, advisor_y + 248, image_group(IMAGE_GROUP_FONT_WHITE));
    lang_text_draw_centered_ui(LANG_GROUP_WINDOW_HINTS, WINDOW_HINT_CANCEL, advisor_x + 336, advisor_y + 288, 160, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_centered_ui(
        LANG_GROUP_KINGDOM_ADVISOR, KINGDOM_TEXT_GIVE_MONEY, advisor_x + 144, advisor_y + 288, 160, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
}

void window_donate_to_city_draw_foreground(void)
{
    for (int32_t i = 0; i < DONATION_AMOUNTS; i++) {
        button_border_draw_ui(advisor_x + 128 + (i * 80), advisor_y + 216, 64, DIALOG_BUTTON_HEIGHT, focus_button_id == (DONATE_BUTTON_FIRST_AMOUNT + i),
            ui_origin_x, ui_origin_y);
    }
    button_border_draw_ui(advisor_x + 336, advisor_y + 283, 160, DIALOG_BUTTON_HEIGHT, focus_button_id == DONATE_BUTTON_CANCEL, ui_origin_x, ui_origin_y);
    button_border_draw_ui(advisor_x + 144, advisor_y + 283, 160, DIALOG_BUTTON_HEIGHT, focus_button_id == DONATE_BUTTON_GIVE, ui_origin_x, ui_origin_y);
}

void window_send_gift_draw_background(void)
{
    outer_panel_draw(advisor_x + 96, advisor_y + 144, 30, 15);
    image_draw(image_group(IMAGE_GROUP_RESOURCE_ICONS) + RESOURCE_ICON_DEBEN, advisor_x + 128, advisor_y + 160);
    lang_text_draw_centered_ui(
        LANG_GROUP_KINGDOM_ADVISOR, KINGDOM_TEXT_GIVE_TO_EGYPT, advisor_x + 128, advisor_y + 160, 432, image_group(IMAGE_GROUP_FONT_LARGE), 0, ui_origin_x, ui_origin_y);
    inner_panel_draw_ui(advisor_x + 112, advisor_y + 208, 28, 5, ui_origin_x, ui_origin_y);
    g_text_width = 0;
    lang_text_draw_ui(LANG_GROUP_KINGDOM_ADVISOR, KINGDOM_TEXT_TIME_SINCE_LAST_GIFT, advisor_x + 144, advisor_y + 304, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_amount(
        LANG_GROUP_AMOUNTS, AMOUNT_TEXT_MONTH, city_player_current()->months_since_gift, g_text_width + advisor_x + 144, advisor_y + 304, image_group(IMAGE_GROUP_FONT_BLACK), 0);
    lang_text_draw_centered_ui(LANG_GROUP_WINDOW_HINTS, WINDOW_HINT_CANCEL, advisor_x + 400, advisor_y + 341, 160, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
}

static void draw_gift_row(const city_player *player, int32_t size)
{
    int32_t y = advisor_y + 218 + (size * 20);
    draw_kingdom_text(KINGDOM_TEXT_GIFT_SIZE_FIRST + size, advisor_x + 128, y, image_group(IMAGE_GROUP_FONT_WHITE));
    int32_t font = (focus_button_id == (GIFT_BUTTON_FIRST_SIZE + size)) ? image_group(IMAGE_GROUP_FONT_RED) : image_group(IMAGE_GROUP_FONT_WHITE);
    g_text_width = 0;
    draw_kingdom_text(KINGDOM_TEXT_MODEST_GIFT_FIRST + (size * GIFTS_PER_SIZE) + player->gift_id[size], advisor_x + 224, y, font);
    draw_number(player->gift_cost[size], money_postfix, g_text_width + advisor_x + 224, y, font);
}

void window_send_gift_draw_foreground(void)
{
    const city_player *player = city_player_current();
    if (player->personal_savings < player->gift_cost[0]) {
        lang_text_draw_multiline_ui(
            LANG_GROUP_KINGDOM_ADVISOR, KINGDOM_TEXT_NOT_ENOUGH_SAVINGS, advisor_x + 160, advisor_y + 224, 352, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
    } else {
        draw_gift_row(player, 0);
    }
    for (int32_t size = 1; size < GIFT_SIZES; size++) {
        if (player->gift_cost[size] <= player->personal_savings) {
            draw_gift_row(player, size);
        }
    }
    if (player->gift_cost[0] <= player->personal_savings) {
        lang_text_draw_centered_ui(LANG_GROUP_KINGDOM_ADVISOR, KINGDOM_TEXT_DISPATCH_GIFT_FIRST + player->selected_gift_size, advisor_x + 128, advisor_y + 341,
            240, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
        button_border_draw_ui(advisor_x + 128, advisor_y + 336, 240, DIALOG_BUTTON_HEIGHT, focus_button_id == GIFT_BUTTON_SEND, ui_origin_x, ui_origin_y);
    }
    button_border_draw_ui(advisor_x + 400, advisor_y + 336, 160, DIALOG_BUTTON_HEIGHT, focus_button_id == GIFT_BUTTON_CANCEL, ui_origin_x, ui_origin_y);
}

void request_popup_answer(void)
{
    if (popup_answer == 1) {
        scenario_request_dispatch(scenario_current_request);
    }
}

void window_go_burial_provisions(void)
{
    window_current_id = WINDOW_BURIAL_PROVISIONS;
    window_redraw_request = 1;
}

static void clamp_donation(city_player *player)
{
    if (player->donate_amount > player->personal_savings) {
        player->donate_amount = player->personal_savings;
    }
    if (player->donate_amount < 0) {
        player->donate_amount = 0;
    }
}

void advisor_donation_arrow(void)
{
    city_player *player = city_player_current();
    if (button_parameter == 1) {
        player->donate_amount -= DONATION_ARROW_STEP;
    } else if (button_parameter == 0) {
        player->donate_amount += DONATION_ARROW_STEP;
    }
    clamp_donation(player);
    window_redraw_request = 1;
}

void button_donate_to_city(void)
{
    city_player *player = city_player_current();
    window_current_id = WINDOW_DONATE_TO_CITY;
    if (player->donate_amount > player->personal_savings) {
        player->donate_amount = player->personal_savings;
    }
    window_redraw_request = 1;
}

void button_set_salary(void)
{
    if (city_player_current()->mansion_placed == 0) {
        return;
    }
    if (button_parameter == 0) {
        window_current_id = WINDOW_SET_SALARY;
        window_redraw_request = 1;
        return;
    }
    advisor_x = ui_origin_x;
    advisor_y = ui_origin_y;
    mouse_clear_clicks();
    window_current_id = WINDOW_SALARY_DIALOG;
    window_redraw_request = 1;
}

void button_send_gift(void)
{
    city_player *player = city_player_current();
    window_current_id = WINDOW_SEND_GIFT;
    if (player->selected_gift_size == 2 && player->gift_cost[2] > player->personal_savings) {
        player->selected_gift_size = 1;
    }
    if (player->selected_gift_size == 1 && player->gift_cost[1] > player->personal_savings) {
        player->selected_gift_size = 0;
    }
    window_redraw_request = 1;
}

static void request_popup(int32_t text)
{
    window_popup_dialog_show(text, button_none, 0);
}

void button_request(void)
{
    int32_t event_id;
    int32_t status = scenario_request_status(button_parameter, &event_id);
    if (status == SCENARIO_REQUEST_NONE) {
        return;
    }
    city_player_current()->empire_service_legions = 0;
    switch (status) {
        case SCENARIO_REQUEST_NO_WATERBORNE_COMPANIES:
            request_popup(POPUP_NO_WATERBORNE_COMPANIES);
            return;
        case SCENARIO_REQUEST_NO_WATERBORNE_IN_SERVICE:
            request_popup(POPUP_NO_WATERBORNE_COMPANIES_IN_SERVICE);
            return;
        case SCENARIO_REQUEST_NO_COMPANIES:
            request_popup(POPUP_NO_COMPANIES);
            return;
        case SCENARIO_REQUEST_NO_COMPANIES_IN_SERVICE:
            request_popup(POPUP_NO_COMPANIES_IN_SERVICE);
            return;
        case SCENARIO_REQUEST_TROOPS:
            scenario_current_request = event_id;
            window_popup_dialog_show(POPUP_DISPATCH_RELIEF_FORCE, request_popup_answer, 2);
            return;
        case SCENARIO_REQUEST_SHORT:
            request_popup(POPUP_NOT_ENOUGH_FOR_REQUEST);
            return;
        default:
            break;
    }
    scenario_current_request = status - 1;
    window_popup_dialog_show(POPUP_DISPATCH_GOODS, request_popup_answer, 2);
    window_redraw_request = 1;
}

void button_salary_cancel(void)
{
    uint8_t window = window_current_id;
    if (window == WINDOW_SET_SALARY) {
        window_current_id = WINDOW_ADVISORS;
        window_redraw_request = 1;
        return;
    }
    if (window == WINDOW_SALARY_DIALOG) {
        window_current_id = WINDOW_BUILDING_INFO;
    }
    window_redraw_request = 1;
}

void button_salary_rank(void)
{
    city_player *player = city_player_current();
    if (player->mission_continue_months_left > 0) {
        return;
    }
    int32_t rank = button_parameter;
    player->salary_rank = rank;
    player->salary_amount = salary_by_rank[rank];
    city_finance_estimate_salary();
    city_ratings_kingdom_explanation();
    uint8_t window = window_current_id;
    window_redraw_request = 1;
    window_current_id = (window == WINDOW_SET_SALARY) ? WINDOW_ADVISORS : WINDOW_BUILDING_INFO;
}

void button_donate_give(void)
{
    city_player *player = city_player_current();
    player->treasury += player->donate_amount;
    player->donated_this_year += player->donate_amount;
    player->personal_savings -= player->donate_amount;
    city_finance_calculate_totals();
    city_finance_calculate_tribute();
    window_current_id = WINDOW_ADVISORS;
    window_redraw_request = 1;
}

void button_donate_amount(void)
{
    city_player *player = city_player_current();
    if (button_parameter >= 0 && button_parameter < DONATION_AMOUNTS) {
        player->donate_amount = donation_amounts[button_parameter];
    }
    clamp_donation(player);
    window_redraw_request = 1;
}

void button_gift_select(void)
{
    city_player *player = city_player_current();
    int32_t gift = button_parameter;
    if (gift >= 1 && gift <= GIFT_SIZES && player->gift_cost[gift - 1] > player->personal_savings) {
        return;
    }
    player->selected_gift_size = gift - 1;
    window_redraw_request = 1;
}

void button_gift_send(void)
{
    const city_player *player = city_player_current();
    if (player->gift_cost[0] > player->personal_savings) {
        return;
    }
    city_kingdom_send_gift_to_pharaoh();
    window_current_id = WINDOW_ADVISORS;
    window_redraw_request = 1;
}

void window_burial_provisions_draw_background(void)
{
    int32_t resource = city_player_current()->request_resource;
    outer_panel_draw(advisor_x + 96, advisor_y + 144, 30, 15);
    image_draw(image_group(IMAGE_GROUP_RESOURCE_ICONS) + resource, advisor_x + 128, advisor_y + 160);
    lang_text_draw_centered_ui(LANG_GROUP_MONUMENTS_ADVISOR, MONUMENTS_TEXT_DISPATCH_BURIAL_PROVISIONS, advisor_x + 128, advisor_y + 160, 432, image_group(IMAGE_GROUP_FONT_LARGE), 0,
        ui_origin_x, ui_origin_y);
    inner_panel_draw_ui(advisor_x + 112, advisor_y + 208, 28, 5, ui_origin_x, ui_origin_y);
    lang_text_draw_centered_ui(LANG_GROUP_WINDOW_HINTS, WINDOW_HINT_CANCEL, advisor_x + 400, advisor_y + 341, 160, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
}

static void draw_provisions_text(int32_t text, int32_t x, int32_t y, int32_t font)
{
    lang_text_draw_ui(LANG_GROUP_MONUMENTS_ADVISOR, text, x, y, font, 0, ui_origin_x, ui_origin_y);
}

void window_burial_provisions_draw_foreground(void)
{
    const city_player *player = city_player_current();
    int32_t amount = requested_amount_shown(player->request_resource, player->request_amount);
    g_text_width = 0;
    draw_provisions_text(MONUMENTS_TEXT_AMOUNT_TO_DISPATCH, advisor_x + 128, advisor_y + 213, image_group(IMAGE_GROUP_FONT_WHITE));
    text_draw_number_ui(amount, NUMBER_PREFIX_NONE, empty_text, advisor_x + 128 + g_text_width, advisor_y + 213, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
    draw_provisions_text(MONUMENTS_TEXT_ALL, advisor_x + 338, advisor_y + 213, (focus_button_id == PROVISIONS_BUTTON_ALL) ? image_group(IMAGE_GROUP_FONT_RED) : image_group(IMAGE_GROUP_FONT_WHITE));
    draw_provisions_text(
        MONUMENTS_TEXT_INCREASE_AMOUNT, advisor_x + 338, advisor_y + 238, (arrow_button_focus_id == PROVISIONS_ARROW_MORE) ? image_group(IMAGE_GROUP_FONT_RED) : image_group(IMAGE_GROUP_FONT_WHITE));
    draw_provisions_text(
        MONUMENTS_TEXT_DECREASE_AMOUNT, advisor_x + 338, advisor_y + 263, (arrow_button_focus_id == PROVISIONS_ARROW_LESS) ? image_group(IMAGE_GROUP_FONT_RED) : image_group(IMAGE_GROUP_FONT_WHITE));
    lang_text_draw_centered_ui(
        LANG_GROUP_MONUMENTS_ADVISOR, MONUMENTS_TEXT_CLICK_TO_DISPATCH, advisor_x + 128, advisor_y + 341, 240, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    button_border_draw_ui(advisor_x + 128, advisor_y + 336, 240, DIALOG_BUTTON_HEIGHT, focus_button_id == PROVISIONS_BUTTON_SEND, ui_origin_x, ui_origin_y);
    button_border_draw_ui(advisor_x + 400, advisor_y + 336, 160, DIALOG_BUTTON_HEIGHT, focus_button_id == PROVISIONS_BUTTON_BACK, ui_origin_x, ui_origin_y);
}

static void royal_tombs_complete_finished(void)
{
    for (int32_t id = 1; id <= building_highest_id; id++) {
        building *b = building_get(id);
        tomb_object *object = royal_tomb_object_for_type(b->type);
        if (!object) {
            continue;
        }
        if (b->subtype == 0) {
            if (royal_tomb_progress(b->type, id, NULL) != TOMB_PROGRESS_DONE) {
                continue;
            }
            b->data.royal_tomb.completed = 1;
        }
        royal_tomb_update_image(object, id);
    }
    map_tiles_update_all();
}

void burial_provisions_send(void)
{
    int32_t player_id = city_current_player;
    const city_player *player = city_player_current();
    int32_t resource = player->request_resource;
    int32_t amount = player->request_amount;
    scenario_data.provisions_sent[resource] += amount;
    building_storageyards_remove_resource(player_id, resource, amount * STORAGE_UNITS_PER_LOAD);
    if (city_victory_goals_reached() != 0) {
        royal_tombs_complete_finished();
    }
    window_current_id = WINDOW_ADVISORS;
    window_redraw_request = 1;
}

void burial_provisions_back(void)
{
    window_current_id = WINDOW_ADVISORS;
    window_redraw_request = 1;
    const city_player *player = city_player_current();
    scenario_data.provisions_sent[player->request_resource] = player->request_available;
}

static int32_t burial_provisions_limit(int32_t resource)
{
    int32_t open = scenario_data.provisions_needed[resource] - scenario_data.provisions_sent[resource];
    int32_t stock = city_player_current()->storage_stored[resource] / STORAGE_UNITS_PER_LOAD;
    return (open < stock) ? open : stock;
}

void burial_provisions_all(void)
{
    city_player *player = city_player_current();
    player->request_amount = burial_provisions_limit(player->request_resource);
    window_redraw_request = 1;
}

void burial_provisions_more(void)
{
    city_player *player = city_player_current();
    int32_t amount = player->request_amount + 1;
    player->request_amount = amount;
    int32_t limit = burial_provisions_limit(player->request_resource);
    player->request_amount = (amount < limit) ? amount : limit;
    window_redraw_request = 1;
}

void burial_provisions_less(void)
{
    city_player *player = city_player_current();
    if (mouse_left_went_down != 0 && player->request_amount > 0) {
        player->request_amount--;
    }
    window_redraw_request = 1;
}
