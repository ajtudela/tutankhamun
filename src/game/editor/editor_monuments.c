// The scenario editor's monuments window, gods settings window and trade prices window, with the monument checks and
// the int16 numeric input. Monuments: three slots of the scenario's monument era, emptied when the era or the
// empire's materials do not allow them, kept sorted; burial provisions are listed for the monuments that need them.
//
// The trade prices reset walks two words past the list, so the last price record is also copied into the last city
// culture record.

#include "editor/editor_monuments.h"
#include "building/building_data.h"
#include "building/monument.h"
#include "building/monument_data.h"
#include "city/gods.h"
#include "city/ratings.h"
#include "city/resource_type.h"
#include "core/crt_compat.h"
#include "editor/editor_data.h"
#include "editor/editor_dialogs.h"
#include "editor/editor_text.h"
#include "empire/empire_data.h"
#include "empire/empire_helpers.h"
#include "graphics/image_collection.h"
#include "text/lang.h"
#include "window/advisors_monuments.h"
#include "window/input_dialogs.h"
#include "window/options_windows.h"
#include "window/popup_dialog.h"
#include "window/window_data.h"
#include "window/window_id.h"
#include "window/window_image_group.h"
#include <stdint.h>

enum monument_window_layout {
    MONUMENT_SLOT_BUTTON_FIRST = 0,
    MONUMENT_PROVISION_BUTTON_FIRST = 3,
    MONUMENT_ERA_BUTTON = 18,
    MONUMENT_ERA_FOCUS = 19,
    MONUMENT_PROVISION_FOCUS_FIRST = 4,
    MONUMENT_PANEL_WIDTH_BLOCKS = 40,
    MONUMENT_PANEL_HEIGHT_BLOCKS = 28,
    MONUMENT_HINT_Y = 422,
    MONUMENT_PANEL_WIDTH = 608,
    MONUMENT_TEXT_X = 4,
    MONUMENT_TEXT_Y = 4,
    MONUMENT_RATING_X = 10,
    MONUMENT_RATING_Y = 166,
    MONUMENT_RATING_WIDTH = 270,
    MONUMENT_RATING_HEIGHT = 100,
    MONUMENT_RATING_TEXT_Y = 190,
    MONUMENT_RATING_VALUE_Y = 214,
    MONUMENT_UNITS_X = 290,
    MONUMENT_UNITS_Y = 400,
    MONUMENT_PROVISION_Y = 29,
    MONUMENT_PROVISION_ROW_HEIGHT = 25,
    MONUMENT_PROVISION_ICON_Y = -4,
    MONUMENT_PROVISION_ICON_LEFT_X = 350,
    MONUMENT_PROVISION_ICON_RIGHT_X = 600,
    MONUMENT_PROVISION_NAME_X = 450,
    MONUMENT_PROVISION_AMOUNT_X = 385,
    MONUMENT_PROVISION_MAX = 3200,
    MONUMENT_PROVISION_KINDS_MAX = 5,
    MONUMENT_PROVISION_INPUT_X = 300,
    MONUMENT_PROVISION_INPUT_Y = 200,
};

enum monument_select_list_position {
    MONUMENT_LIST_640_X = 110,
    MONUMENT_LIST_640_Y = 140,
    MONUMENT_LIST_800_X = 190,
    MONUMENT_LIST_800_Y = 200,
    MONUMENT_LIST_X = 302,
    MONUMENT_LIST_Y = 284,
    MONUMENT_LIST_COLUMNS = 2,
    MONUMENT_ERA_LIST_ROWS = 3,
};

enum gods_window_layout {
    GODS_STATUS_BUTTON_FIRST = 0,
    GODS_TEMPLE_COMPLEX_BUTTON_FIRST = 10,
    GODS_TEMPLE_COMPLEX_FOCUS_FIRST = 11,
    GODS_PANEL_WIDTH_BLOCKS = 21,
    GODS_PANEL_HEIGHT_BLOCKS = 14,
    GODS_PANEL_WIDTH = 336,
    GODS_TITLE_X = 10,
    GODS_TITLE_Y = 10,
    GODS_HINT_Y = 200,
    GODS_HEADER_Y = 46,
    GODS_NAME_X = 20,
    GODS_RANK_X = 132,
    GODS_TEMPLE_COMPLEX_X = 197,
    GODS_ROW_Y = 70,
    GODS_ROW_HEIGHT = 24,
    GODS_TEXT_X = 4,
    GODS_TEXT_Y = 4,
};

enum trade_prices_layout {
    TRADE_PRICE_LIST_DRAWN = 34,
    TRADE_PRICE_LIST_RESET = 36,
    TRADE_PRICES_RESET_BUTTON = 66,
    TRADE_PRICES_RESET_FOCUS = 67,
    TRADE_PRICES_FIRST_FOCUS = 2,
    TRADE_PRICES_PANEL_X = -19,
    TRADE_PRICES_PANEL_WIDTH_BLOCKS = 43,
    TRADE_PRICES_PANEL_HEIGHT_BLOCKS = 16,
    TRADE_PRICES_PANEL_WIDTH = 608,
    TRADE_PRICES_TEXT_X = -9,
    TRADE_PRICES_TITLE_Y = 9,
    TRADE_PRICES_BUYS_Y = 84,
    TRADE_PRICES_SELLS_Y = 109,
    TRADE_PRICES_SECOND_BUYS_Y = 159,
    TRADE_PRICES_SECOND_SELLS_Y = 184,
    TRADE_PRICES_HINT_Y = 209,
    TRADE_PRICES_COLUMN_WIDTH = 35,
    TRADE_PRICES_ICON_X = 40,
    TRADE_PRICES_ICON_Y = 50,
    TRADE_PRICES_ICON_LINE_HEIGHT = 78,
    TRADE_PRICES_BUTTON_X = 34,
    TRADE_PRICES_LINE_HEIGHT = 75,
    TRADE_PRICES_BUYS_BUTTON_Y = 80,
    TRADE_PRICES_SELLS_BUTTON_Y = 105,
    TRADE_PRICES_BUTTON_WIDTH = 35,
    TRADE_PRICES_BUTTON_HEIGHT = 20,
    TRADE_PRICES_TEXT_Y = 5,
    TRADE_PRICES_NUMBER_WIDTH = 30,
    TRADE_PRICES_RESET_TEXT_X = 5,
    TRADE_PRICES_RESET_TEXT_Y = 8,
    TRADE_PRICES_INPUT_X = 45,
    TRADE_PRICES_INPUT_Y = 45,
};

enum trade_price_kind {
    TRADE_PRICE_BUYS = 0,
    TRADE_PRICE_SELLS = 1,
    TRADE_PRICE_KINDS = 2,
};

int32_t monument_def_goods_available(int32_t id)
{
    const monument_def *def = &monument_defs[id];
    for (int32_t i = 0; i < 3; i++) {
        int32_t resource = def->resources[i];
        if (resource != RESOURCE_NONE && !empire_resource_available(resource)) {
            return 0;
        }
    }
    return 1;
}

static int32_t monument_of_every_era(const monument_def *def)
{
    if (def->kind != MONUMENT_DEF_KIND_OTHER) {
        return 0;
    }
    return def->subkind == MONUMENT_DEF_SPHINX || def->subkind == MONUMENT_DEF_MAUSOLEUM || def->subkind == MONUMENT_DEF_OBELISK ||
        def->subkind == MONUMENT_DEF_SUN_TEMPLE;
}

int32_t monument_def_allowed(int32_t era, const monument_def *def)
{
    int32_t subkind = def->subkind;
    int32_t not_pyramid = def->kind != MONUMENT_DEF_KIND_PYRAMID_FAMILY;
    if (era == MONUMENT_ERA_NONE) {
        return 0;
    }
    if (monument_of_every_era(def)) {
        return 1;
    }
    if (era == MONUMENT_ERA_ALEXANDRIA) {
        return not_pyramid && (subkind == MONUMENT_DEF_LIBRARY || subkind == MONUMENT_DEF_CAESAREUM || subkind == MONUMENT_DEF_LIGHTHOUSE);
    }
    if (era == MONUMENT_ERA_VALLEY_OF_THE_KINGS) {
        return not_pyramid && (subkind >= MONUMENT_DEF_SMALL_ROYAL_TOMB && subkind <= MONUMENT_DEF_GRAND_ROYAL_TOMB);
    }
    if (era == MONUMENT_ERA_ABU_SIMBEL) {
        return not_pyramid && subkind == MONUMENT_DEF_ABU_SIMBEL;
    }
    if (era == MONUMENT_ERA_PYRAMIDS) {
        return def->kind == MONUMENT_DEF_KIND_PYRAMID_FAMILY;
    }
    return 1;
}

static int32_t monument_choosable(int32_t id)
{
    return monument_def_goods_available(id) && monument_def_allowed(scenario_data.animation_set, &monument_defs[id]);
}

int editor_monument_ids_compare(const void *a, const void *b)
{
    int16_t id_a = *(const int16_t *) a;
    int16_t id_b = *(const int16_t *) b;
    if (id_b < id_a) {
        return -1;
    }
    return id_a < id_b;
}

static void monument_slots_check(void)
{
    for (int32_t slot = 0; slot < MONUMENT_SLOTS; slot++) {
        if (!monument_choosable(scenario_data.monument_def_ids[slot])) {
            scenario_data.monument_def_ids[slot] = 0;
        }
    }
}

static int32_t monument_slots_need_burial_provisions(void)
{
    int32_t needed = 0;
    for (int32_t slot = 0; slot < MONUMENT_SLOTS; slot++) {
        if (monument_burial_provisions[scenario_data.monument_def_ids[slot]] != 0) {
            needed = 1;
        }
    }
    return needed;
}

static void draw_monument_slots(void)
{
    for (int32_t slot = 0; slot < MONUMENT_SLOTS; slot++) {
        const generic_button *button = &buttons_special_orders.editor_monuments[MONUMENT_SLOT_BUTTON_FIRST + slot];
        button_border_draw_rect(&button->x, focus_button_id == (slot + 1), 0, 0);
        lang_text_draw_centered_in_rect(
            LANG_GROUP_MONUMENT_NAMES, scenario_data.monument_def_ids[slot], button, image_group(IMAGE_GROUP_FONT_BLACK), 0, MONUMENT_TEXT_X, MONUMENT_TEXT_Y, ui_origin_x, ui_origin_y);
    }
}

static void draw_monument_rating(void)
{
    button_border_draw_ui(
        ui_origin_x + MONUMENT_RATING_X, ui_origin_y + MONUMENT_RATING_Y, MONUMENT_RATING_WIDTH, MONUMENT_RATING_HEIGHT, 0, ui_origin_x, ui_origin_y);
    lang_text_draw_centered_ui(LANG_GROUP_MONUMENTS_ADVISOR, MONUMENTS_TEXT_MONUMENT_RATING, ui_origin_x + MONUMENT_RATING_X,
        ui_origin_y + MONUMENT_RATING_TEXT_Y, MONUMENT_RATING_WIDTH, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    int32_t goal = (int32_t) (int64_t) city_ratings_calculate_monument(1);
    text_draw_number_centered(
        goal, NUMBER_PREFIX_NONE, empty_text, ui_origin_x + MONUMENT_RATING_X, ui_origin_y + MONUMENT_RATING_VALUE_Y, MONUMENT_RATING_WIDTH, image_group(IMAGE_GROUP_FONT_LARGE), 0);
    scenario_data.win_monument_enabled = goal != 0;
    scenario_data.win_monument = goal;
}

static void draw_monument_era(void)
{
    const generic_button *button = &buttons_special_orders.editor_monuments[MONUMENT_ERA_BUTTON];
    button_border_draw_rect(&button->x, focus_button_id == MONUMENT_ERA_FOCUS, 0, 0);
    lang_text_draw_centered_in_rect(
        LANG_GROUP_MONUMENT_ERAS, scenario_data.animation_set, button, image_group(IMAGE_GROUP_FONT_BLACK), 0, MONUMENT_TEXT_X, MONUMENT_TEXT_Y, ui_origin_x, ui_origin_y);
}

static void draw_burial_provision(int32_t row, int32_t x0, int32_t y)
{
    int32_t resource = burial_provision_resources[row];
    image_draw(image_group(IMAGE_GROUP_RESOURCE_ICONS) + resource, x0 + MONUMENT_PROVISION_ICON_LEFT_X, y + MONUMENT_PROVISION_ICON_Y);
    image_draw(image_group(IMAGE_GROUP_RESOURCE_ICONS) + resource, x0 + MONUMENT_PROVISION_ICON_RIGHT_X, y + MONUMENT_PROVISION_ICON_Y);
    lang_text_draw_ui(LANG_GROUP_RESOURCE_NAMES, resource, x0 + MONUMENT_PROVISION_NAME_X, y, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    const generic_button *button = &buttons_special_orders.editor_monuments[MONUMENT_PROVISION_BUTTON_FIRST + row];
    button_border_draw_rect(&button->x, focus_button_id == (MONUMENT_PROVISION_FOCUS_FIRST + row), 0, 0);
    if (empire_resource_available(resource)) {
        if (scenario_data.provisions_needed[resource] > MONUMENT_PROVISION_MAX) {
            scenario_data.provisions_needed[resource] = MONUMENT_PROVISION_MAX;
        }
        text_draw_number_ui(
            scenario_data.provisions_needed[resource], NUMBER_PREFIX_NONE, postfix_none, x0 + MONUMENT_PROVISION_AMOUNT_X, y, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    } else {
        lang_text_draw_ui(LANG_GROUP_YES_NO, LANG_NOT_AVAILABLE, x0 + MONUMENT_PROVISION_AMOUNT_X, y, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
        scenario_data.provisions_needed[resource] = 0;
    }
}

void window_editor_monuments_draw(void)
{
    int32_t x0 = ui_origin_x;
    int32_t y0 = ui_origin_y;
    outer_panel_draw(x0, y0, MONUMENT_PANEL_WIDTH_BLOCKS, MONUMENT_PANEL_HEIGHT_BLOCKS);
    lang_text_draw_centered_ui(
        LANG_GROUP_WINDOW_HINTS, WINDOW_HINT_RIGHT_CLICK_TO_CLOSE, x0, y0 + MONUMENT_HINT_Y, MONUMENT_PANEL_WIDTH, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    monument_slots_check();
    crt_qsort(scenario_data.monument_def_ids, MONUMENT_SLOTS, sizeof(scenario_data.monument_def_ids[0]), editor_monument_ids_compare);
    draw_monument_slots();
    draw_monument_rating();
    draw_monument_era();
    if (!monument_slots_need_burial_provisions()) {
        for (int32_t resource = 0; resource < RESOURCE_MAX; resource++) {
            scenario_data.provisions_needed[resource] = 0;
        }
        return;
    }
    lang_text_draw_ui(
        LANG_GROUP_EDITOR, EDITOR_TEXT_CARTLOAD_IS_100_UNITS, x0 + MONUMENT_UNITS_X, y0 + MONUMENT_UNITS_Y, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    int32_t y = y0 + MONUMENT_PROVISION_Y;
    for (int32_t row = 0; row < BURIAL_PROVISION_ROWS; row++) {
        draw_burial_provision(row, x0, y);
        y += MONUMENT_PROVISION_ROW_HEIGHT;
    }
}

void editor_open_monuments(void)
{
    window_current_id = WINDOW_EDITOR_MONUMENTS;
    window_redraw_request = 1;
}

static int32_t monument_choice_blocked(const monument_def *def, int32_t other_id)
{
    const monument_def *other = &monument_defs[other_id];
    if (other->kind != def->kind) {
        return 0;
    }
    int32_t one_kind_only = other->kind == MONUMENT_DEF_KIND_PYRAMID_FAMILY ||
        (other->kind == MONUMENT_DEF_KIND_OTHER && other->subkind < MONUMENT_DEF_LIGHTHOUSE);
    if (one_kind_only && other->subkind != def->subkind) {
        return 1;
    }
    return other->kind == MONUMENT_DEF_KIND_OTHER && (other->subkind == MONUMENT_DEF_SPHINX || other->subkind == MONUMENT_DEF_MAUSOLEUM);
}

static int32_t monument_checked_against_others(const monument_def *def)
{
    return !(def->kind == MONUMENT_DEF_KIND_OTHER && def->subkind >= MONUMENT_DEF_LIGHTHOUSE);
}

static void monument_select_list_position(int32_t *x, int32_t *y)
{
    if (settings_data.display_mode == DISPLAY_MODE_640X480) {
        *x = MONUMENT_LIST_640_X;
        *y = MONUMENT_LIST_640_Y;
    } else if (settings_data.display_mode == DISPLAY_MODE_800X600) {
        *x = MONUMENT_LIST_800_X;
        *y = MONUMENT_LIST_800_Y;
    } else {
        *x = MONUMENT_LIST_X;
        *y = MONUMENT_LIST_Y;
    }
}

void editor_monument_select(void)
{
    int32_t other_a = scenario_data.monument_def_ids[(button_parameter == 0) ? 1 : 0];
    int32_t other_b = scenario_data.monument_def_ids[(button_parameter == 2) ? 1 : 2];
    int32_t count = 1;
    select_list.items[0] = 0;
    for (int32_t id = 1; id < monument_defs_count(); id++) {
        if (!monument_choosable(id)) {
            continue;
        }
        const monument_def *def = &monument_defs[id];
        if (monument_checked_against_others(def) && (monument_choice_blocked(def, other_a) || monument_choice_blocked(def, other_b))) {
            continue;
        }
        select_list.items[count] = id;
        count++;
    }
    int32_t x;
    int32_t y;
    monument_select_list_position(&x, &y);
    editor_select_list_show(x, y, count, LANG_GROUP_MONUMENT_NAMES, 0, 0, &scenario_data.monument_def_ids[button_parameter], 0, 0, INPUT_TARGET_INT16, 1, 0, 1,
        (count + 1) / MONUMENT_LIST_COLUMNS, 0);
}

void editor_monument_animation_set(void)
{
    int32_t x;
    int32_t y;
    monument_select_list_position(&x, &y);
    editor_select_list_show(
        x, y, MONUMENT_ERA_COUNT, LANG_GROUP_MONUMENT_ERAS, 0, &scenario_data.animation_set, 0, 0, 0, INPUT_TARGET_INT8, 0, 0, 0, MONUMENT_ERA_LIST_ROWS, 0);
}

void editor_monument_goods_amount(void)
{
    if (!empire_resource_available(button_parameter)) {
        window_popup_dialog_show(POPUP_BURIAL_PROVISION_NOT_AVAILABLE, button_none, POPUP_BUTTONS_OK);
        window_redraw_request = 1;
        return;
    }
    if (monument_count_needed_resources(button_parameter) >= MONUMENT_PROVISION_KINDS_MAX) {
        window_popup_dialog_show(POPUP_TOO_MANY_BURIAL_PROVISIONS, button_none, POPUP_BUTTONS_OK);
        window_redraw_request = 1;
        return;
    }
    numeric_input_show(MONUMENT_PROVISION_INPUT_X, MONUMENT_PROVISION_INPUT_Y, INPUT_TARGET_INT32, 0, 0, &scenario_data.provisions_needed[button_parameter], 0);
    window_redraw_request = 1;
}

static void draw_god_row(int32_t god, int32_t x0, int32_t y)
{
    lang_text_draw_ui(LANG_GROUP_GOD_NAMES, god, x0 + GODS_NAME_X, y, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
    const generic_button *status = &buttons_special_orders.editor_gods[GODS_STATUS_BUTTON_FIRST + god];
    button_border_draw_rect(&status->x, focus_button_id == (god + 1), 0, 0);
    lang_text_draw_centered_in_rect(LANG_GROUP_GOD_STATUS, scenario_data.god_status[god], status, image_group(IMAGE_GROUP_FONT_BLACK), 0, GODS_TEXT_X, GODS_TEXT_Y, ui_origin_x, ui_origin_y);
    const generic_button *temple_complex = &buttons_special_orders.editor_gods[GODS_TEMPLE_COMPLEX_BUTTON_FIRST + god];
    button_border_draw_rect(&temple_complex->x, focus_button_id == (god + GODS_TEMPLE_COMPLEX_FOCUS_FIRST), 0, 0);
    lang_text_draw_centered_in_rect(
        LANG_GROUP_YES_NO, scenario_data.temple_complexes_allowed[god], temple_complex, image_group(IMAGE_GROUP_FONT_BLACK), 0, GODS_TEXT_X, GODS_TEXT_Y, ui_origin_x, ui_origin_y);
}

void window_editor_gods_draw(void)
{
    int32_t x0 = ui_origin_x;
    int32_t y0 = ui_origin_y;
    outer_panel_draw(x0, y0, GODS_PANEL_WIDTH_BLOCKS, GODS_PANEL_HEIGHT_BLOCKS);
    lang_text_draw_ui(LANG_GROUP_EDITOR, EDITOR_TEXT_GODS_SETTINGS, x0 + GODS_TITLE_X, y0 + GODS_TITLE_Y, image_group(IMAGE_GROUP_FONT_LARGE), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_centered_ui(
        LANG_GROUP_WINDOW_HINTS, WINDOW_HINT_RIGHT_CLICK_TO_CLOSE, x0, y0 + GODS_HINT_Y, GODS_PANEL_WIDTH, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_ui(LANG_GROUP_EDITOR, EDITOR_TEXT_NAME, x0 + GODS_NAME_X, y0 + GODS_HEADER_Y, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_ui(LANG_GROUP_EDITOR, EDITOR_TEXT_RANK, x0 + GODS_RANK_X, y0 + GODS_HEADER_Y, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_ui(LANG_GROUP_EDITOR, EDITOR_TEXT_TEMPLE_COMPLEX, x0 + GODS_TEMPLE_COMPLEX_X, y0 + GODS_HEADER_Y, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    int32_t y = y0 + GODS_ROW_Y;
    for (int32_t god = 0; god < GOD_COUNT; god++) {
        draw_god_row(god, x0, y);
        y += GODS_ROW_HEIGHT;
    }
}

void editor_gods_toggle_temple_complex(void)
{
    int32_t god = button_parameter;
    if (scenario_data.god_status[god] == GOD_STATUS_NONE) {
        scenario_data.temple_complexes_allowed[god] = 0;
    } else if (scenario_data.temple_complexes_allowed[god] == 1) {
        scenario_data.temple_complexes_allowed[god] = 0;
    } else if (scenario_data.temple_complexes_allowed[god] == 0) {
        scenario_data.temple_complexes_allowed[god] = 1;
    }
    window_redraw_request = 1;
}

void editor_gods_next_status(void)
{
    int32_t god = button_parameter;
    scenario_data.god_status[god]++;
    if (scenario_data.god_status[god] > GOD_STATUS_PATRON) {
        scenario_data.god_status[god] = GOD_STATUS_NONE;
    }
    if (scenario_data.god_status[god] == GOD_STATUS_NONE) {
        scenario_data.temple_complexes_allowed[god] = 0;
    }
    window_redraw_request = 1;
}

static int32_t trade_price_shown(int32_t resource)
{
    return resource != RESOURCE_NONE && resource != RESOURCE_UNUSED && resource >= 0 && resource < RESOURCE_MAX;
}

static int32_t resource_traded(int32_t resource)
{
    return empire_any_city_sells_resource(resource) != 0 || empire_any_city_buys_resource(resource) != 0;
}

static void draw_trade_price_column(int32_t resource, int32_t column, int32_t focus, int32_t px, int32_t y0)
{
    int32_t second_line = resource > RESOURCE_LINEN;
    int32_t x = (column * TRADE_PRICES_COLUMN_WIDTH) + px;
    image_draw(
        image_group(IMAGE_GROUP_RESOURCE_ICONS) + resource, x + TRADE_PRICES_ICON_X, y0 + (second_line * TRADE_PRICES_ICON_LINE_HEIGHT) + TRADE_PRICES_ICON_Y);
    int32_t y = y0 + (second_line * TRADE_PRICES_LINE_HEIGHT);
    x += TRADE_PRICES_BUTTON_X;
    button_border_draw_ui(
        x, y + TRADE_PRICES_BUYS_BUTTON_Y, TRADE_PRICES_BUTTON_WIDTH, TRADE_PRICES_BUTTON_HEIGHT, focus_button_id == (focus - 1), ui_origin_x, ui_origin_y);
    button_border_draw_ui(
        x, y + TRADE_PRICES_SELLS_BUTTON_Y, TRADE_PRICES_BUTTON_WIDTH, TRADE_PRICES_BUTTON_HEIGHT, focus_button_id == focus, ui_origin_x, ui_origin_y);
    int32_t buys_y = y + TRADE_PRICES_BUYS_BUTTON_Y + TRADE_PRICES_TEXT_Y;
    int32_t sells_y = y + TRADE_PRICES_SELLS_BUTTON_Y + TRADE_PRICES_TEXT_Y;
    if (!resource_traded(resource)) {
        lang_text_draw_ui(LANG_GROUP_YES_NO, LANG_NOT_AVAILABLE, x, buys_y, image_group(IMAGE_GROUP_FONT_SMALL), 0, ui_origin_x, ui_origin_y);
        lang_text_draw_ui(LANG_GROUP_YES_NO, LANG_NOT_AVAILABLE, x, sells_y, image_group(IMAGE_GROUP_FONT_SMALL), 0, ui_origin_x, ui_origin_y);
    } else {
        const int32_t *prices = &trade_prices[resource * TRADE_PRICE_KINDS];
        text_draw_number_centered(prices[TRADE_PRICE_BUYS], NUMBER_PREFIX_NONE, postfix_none, x, buys_y, TRADE_PRICES_NUMBER_WIDTH, image_group(IMAGE_GROUP_FONT_SMALL), 0);
        text_draw_number_centered(prices[TRADE_PRICE_SELLS], NUMBER_PREFIX_NONE, postfix_none, x, sells_y, TRADE_PRICES_NUMBER_WIDTH, image_group(IMAGE_GROUP_FONT_SMALL), 0);
    }
}

void window_editor_trade_prices_draw(void)
{
    int32_t y0 = ui_origin_y;
    int32_t px = ui_origin_x + TRADE_PRICES_PANEL_X;
    outer_panel_draw(px, y0, TRADE_PRICES_PANEL_WIDTH_BLOCKS, TRADE_PRICES_PANEL_HEIGHT_BLOCKS);
    int32_t tx = ui_origin_x + TRADE_PRICES_TEXT_X;
    lang_text_draw_ui(LANG_GROUP_EDITOR, EDITOR_TEXT_SET_KINGDOM_PRICES, tx, y0 + TRADE_PRICES_TITLE_Y, image_group(IMAGE_GROUP_FONT_LARGE), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_ui(LANG_GROUP_EDITOR, EDITOR_TEXT_BUYS, tx, y0 + TRADE_PRICES_BUYS_Y, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_ui(LANG_GROUP_EDITOR, EDITOR_TEXT_SELLS, tx, y0 + TRADE_PRICES_SELLS_Y, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_ui(LANG_GROUP_EDITOR, EDITOR_TEXT_BUYS, tx, y0 + TRADE_PRICES_SECOND_BUYS_Y, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_ui(LANG_GROUP_EDITOR, EDITOR_TEXT_SELLS, tx, y0 + TRADE_PRICES_SECOND_SELLS_Y, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    int32_t column = 1;
    int32_t focus = TRADE_PRICES_FIRST_FOCUS;
    for (int32_t entry = 1; entry < TRADE_PRICE_LIST_DRAWN; entry++, focus += 2) {
        int32_t resource = trade_goods_list.list.resources[entry];
        if (!trade_price_shown(resource)) {
            continue;
        }
        if (resource == RESOURCE_GEMS) {
            column = 1;
        }
        draw_trade_price_column(resource, column, focus, px, y0);
        column++;
    }
    const generic_button *reset = &build_menu_buttons.editor_trade_prices[TRADE_PRICES_RESET_BUTTON];
    button_border_draw_rect(&reset->x, focus_button_id == TRADE_PRICES_RESET_FOCUS, 0, 0);
    lang_text_draw_rect_ui(
        LANG_GROUP_EDITOR, EDITOR_TEXT_RESET_PRICES, &reset->x, image_group(IMAGE_GROUP_FONT_BLACK), 0, TRADE_PRICES_RESET_TEXT_X, TRADE_PRICES_RESET_TEXT_Y, ui_origin_x, ui_origin_y);
    lang_text_draw_centered_ui(LANG_GROUP_WINDOW_HINTS, WINDOW_HINT_RIGHT_CLICK_TO_CONTINUE, px, y0 + TRADE_PRICES_HINT_Y, TRADE_PRICES_PANEL_WIDTH, image_group(IMAGE_GROUP_FONT_BLACK),
        0, ui_origin_x, ui_origin_y);
}

void editor_trade_price_button(void)
{
    int32_t resource = trade_goods_list.list.resources[button_parameter2];
    if (!resource_traded(resource)) {
        window_popup_dialog_show(POPUP_CANNOT_CHANGE_PRICE, button_none, POPUP_BUTTONS_OK);
        window_redraw_request = 1;
        return;
    }
    numeric_input_show(ui_origin_x + TRADE_PRICES_INPUT_X, ui_origin_y + TRADE_PRICES_INPUT_Y, INPUT_TARGET_INT32, 0, 0,
        &trade_prices[button_parameter + (resource * TRADE_PRICE_KINDS)], 0);
    window_redraw_request = 1;
}

static int32_t *trade_price_record(int32_t *prices, int32_t *record_before, int32_t resource)
{
    return (resource < 0) ? record_before : &prices[resource * TRADE_PRICE_KINDS];
}

void editor_trade_prices_reset(void)
{
    int32_t *last_culture = (int32_t *) &empire_culture_table[EMPIRE_CITY_NAMES - 1];
    int32_t *last_price = &trade_prices[(RESOURCE_MAX - 1) * TRADE_PRICE_KINDS];
    for (int32_t entry = 0; entry < TRADE_PRICE_LIST_RESET; entry++) {
        int32_t resource = trade_goods_list.walked[entry];
        int32_t *target = trade_price_record(trade_prices, last_culture, resource);
        const int32_t *source = trade_price_record(trade_default_prices, last_price, resource);
        for (int32_t kind = TRADE_PRICE_BUYS; kind < TRADE_PRICE_KINDS; kind++) {
            target[kind] = source[kind];
        }
    }
}

void numeric_input_show_int16(int32_t x, int32_t y, int16_t *target, input_dialog_callback on_ok)
{
    numeric_input_target16 = target;
    numeric_input_x = x;
    int32_t window = window_current_id;
    numeric_input_y = y;
    numeric_input_width = INPUT_TARGET_INT16;
    numeric_input_editor_use = NUMERIC_INPUT_EDITOR_NONE;
    numeric_input_target8 = 0;
    numeric_input_target32 = 0;
    numeric_input_on_ok = on_ok;
    numeric_input_return_window.word = window;
    numeric_input_value = 0;
    numeric_input_digits = 0;
    window_current_id = WINDOW_NUMERIC_INPUT;
}
