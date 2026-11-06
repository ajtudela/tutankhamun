// The trade advisor: its draws (the resources with their trade state, stock and orders), its buttons (a resource
// opens the resource settings, the prices toggle, exit), the scroll arrows of its resource list (13 rows) and the
// toggle of its list mode (1 <-> 0x10).

#include "window/advisor/trade.h"
#include "city/city_internal.h"
#include "city/resource_type.h"
#include "city/resources.h"
#include "empire/empire.h"
#include "empire/empire_helpers.h"
#include "empire/trade_status.h"
#include "game/game_internal.h"
#include "graphics/arrow_buttons.h"
#include "graphics/graphics_primitives.h"
#include "graphics/image_collection.h"
#include "graphics/image_draw.h"
#include "graphics/label_animation_slots.h"
#include "graphics/panel_draw.h"
#include "graphics/text_draw.h"
#include "graphics/ui_layout.h"
#include "window/advisors.h"
#include "window/empire_window.h"
#include "window/window_id.h"
#include "window/window_image_group.h"
#include <stdint.h>

enum trade_advisor_button {
    TRADE_ADVISOR_LUXURY_BUTTON = 2,
};

enum trade_price_field {
    TRADE_PRICE_BUY = 0,
    TRADE_PRICE_SELL = 1,
    TRADE_PRICE_FIELDS = 2,
};

static inline int32_t trade_buy_price(int32_t resource)
{
    return trade_prices[resource * TRADE_PRICE_FIELDS + TRADE_PRICE_BUY];
}

static inline int32_t trade_sell_price(int32_t resource)
{
    return trade_prices[resource * TRADE_PRICE_FIELDS + TRADE_PRICE_SELL];
}

static int32_t resource_is_fractional(int32_t resource)
{
    switch (resource) {
        case RESOURCE_WEAPONS:
        case RESOURCE_STONE:
        case RESOURCE_LIMESTONE:
        case RESOURCE_GRANITE:
        case RESOURCE_CHARIOTS:
        case RESOURCE_SANDSTONE:
        case RESOURCE_MARBLE:
        case RESOURCE_DEBEN:
            return 1;
        default:
            return 0;
    }
}

static int32_t scale_amount_for_display(int32_t amount, int32_t resource)
{
    if (resource_is_fractional(resource)) {
        return amount / 100;
    }
    return amount;
}

static int32_t prices_mode_unit_text(int32_t resource)
{
    switch (resource) {
        case RESOURCE_STONE:
        case RESOURCE_LIMESTONE:
        case RESOURCE_GRANITE:
        case RESOURCE_SANDSTONE:
        case RESOURCE_MARBLE:
            return 49;
        case RESOURCE_WEAPONS:
            return 47;
        default:
            return (resource != RESOURCE_CHARIOTS ? 2 : 0) + 48;
    }
}

static void draw_status_text(int32_t group, int32_t index, int32_t x, int32_t y, int32_t font)
{
    lang_text_draw_ui(group, index, x, y, font, 0, ui_origin_x, ui_origin_y);
}

void window_advisor_trade_draw_background(void)
{
    int32_t anim;

    city_resource_determine_available();
    anim = 5 - label_animation[LABEL_SLOT_ADVISOR_OPEN];
    advisor_x = ui_origin_x;
    advisor_y = ui_origin_y;
    advisor_height_blocks = 27;

    if (anim >= 5) {
        return;
    }
    if (anim > 0) {
        graphics_draw_rect(anim * 16 + ui_origin_x, ui_origin_y + anim * 8, (anim * -2 + 40) * 16, (anim * -2 + 27) * 16, 0);
        return;
    }

    outer_panel_draw(ui_origin_x, ui_origin_y, 40, 27);
    image_draw(image_group(IMAGE_GROUP_ADVISOR_ICONS) + 4, advisor_x + 10, advisor_y + 10);
    lang_text_draw_ui(54, 0, advisor_x + 60, advisor_y + 12, image_group(IMAGE_GROUP_FONT_LARGE), 0, ui_origin_x, ui_origin_y);

    if (city_player_current()->trade_prices_mode == 0) {
        lang_text_draw_ui(54, 1, advisor_x + 100, advisor_y + 38, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    } else {
        lang_text_draw_ui(54, 22, advisor_x + 175, advisor_y + 38, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
        lang_text_draw_ui(54, 23, advisor_x + 275, advisor_y + 38, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    }
}

void window_advisor_trade_draw_scrollbar(void)
{
    int32_t max_scroll;
    int32_t scroll;
    int32_t percent;
    int32_t y;

    max_scroll = city_resources.count - 15;
    if (max_scroll <= 0) {
        return;
    }

    scroll = city_player_current()->resource_list_scroll;
    if (scroll < 2) {
        percent = 0;
    } else if (city_resources.count < scroll + 14) {
        percent = 100;
    } else {
        percent = calc_percentage(scroll - 1, max_scroll);
    }
    y = calc_adjust_with_percentage(256, percent);
    image_draw(image_group(IMAGE_GROUP_PANEL_BUTTON) + 39, advisor_x + 597, advisor_y + 78 + y);
}

static void draw_none_trade_status(int32_t resource, int32_t row_y)
{
    int32_t text_id;
    int32_t font;

    if (empire_can_import_resource(resource) != 0) {
        if (empire_can_export_resource(resource) != 0) {
            text_id = 33;
            font = image_group(IMAGE_GROUP_FONT_WHITE);
            draw_status_text(54, text_id, advisor_x + 324, row_y, font);
            return;
        }
    }
    if (empire_can_import_resource(resource) != 0) {
        text_id = 31;
        font = image_group(IMAGE_GROUP_FONT_WHITE);
        draw_status_text(54, text_id, advisor_x + 324, row_y, font);
        return;
    }
    if (empire_can_export_resource(resource) != 0) {
        text_id = 32;
        font = image_group(IMAGE_GROUP_FONT_WHITE);
        draw_status_text(54, text_id, advisor_x + 324, row_y, font);
        return;
    }
    if (empire_can_export_resource_potentially(resource) != 0) {
        if (empire_can_import_resource_potentially(resource) != 0) {
            text_id = 36;
            font = image_group(IMAGE_GROUP_FONT_GREEN);
            draw_status_text(54, text_id, advisor_x + 324, row_y, font);
            return;
        }
    }
    if (empire_can_import_resource_potentially(resource) != 0) {
        text_id = 34;
        font = image_group(IMAGE_GROUP_FONT_GREEN);
        draw_status_text(54, text_id, advisor_x + 324, row_y, font);
        return;
    }
    if (empire_can_export_resource_potentially(resource) != 0) {
        text_id = 35;
        font = image_group(IMAGE_GROUP_FONT_GREEN);
        draw_status_text(54, text_id, advisor_x + 324, row_y, font);
    }
}

static void draw_luxury_kind_or_button(int32_t kind_count, int32_t *kinds, int32_t row_index, int32_t row_y)
{
    int32_t i;

    if (kind_count == 1) {
        for (i = 0; i < 16; i++) {
            if (kinds[i] != 0) {
                draw_status_text(23, i + 39, g_text_width + 324 + advisor_x, row_y, image_group(IMAGE_GROUP_FONT_WHITE));
                return;
            }
        }
        return;
    }
    if (kind_count != 0) {
        int32_t y_base = row_index * 22;
        trade_advisor_buttons[TRADE_ADVISOR_LUXURY_BUTTON].y = (int16_t) (y_base + 56);
        trade_advisor_buttons[TRADE_ADVISOR_LUXURY_BUTTON].y_end = (int16_t) (y_base + 76);
        draw_status_text(54, 45, g_text_width + 324 + advisor_x, row_y, image_group(IMAGE_GROUP_FONT_WHITE));
    }
}

static int32_t resource_uses_amount_phrase(int32_t resource)
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

static void draw_trade_row_normal(int32_t resource, int32_t row_index, int32_t row_y)
{
    int32_t stock = city_player_current()->storage_stored[resource];
    int16_t status;

    stock = scale_amount_for_display(stock, resource);
    text_draw_number_centered(stock, NUMBER_PREFIX_NONE, postfix_none, advisor_x + 140, row_y, 100, image_group(IMAGE_GROUP_FONT_WHITE), 0);

    if (city_player_current()->stockpiled[resource] != 0) {
        draw_status_text(54, 3, advisor_x + 232, row_y, image_group(IMAGE_GROUP_FONT_WHITE));
    }

    status = city_player_current()->trade_status[resource];
    if (status == 0) {
        draw_none_trade_status(resource, row_y);
        return;
    }
    if (status == 1) {
        g_text_width = 0;
        if (resource == RESOURCE_LUXURY_GOODS) {
            int32_t kinds[16];
            int32_t kind_count;
            int32_t i;

            for (i = 0; i < 16; i++) {
                kinds[i] = 0;
            }
            g_text_width = 0;
            kind_count = empire_count_luxury_goods_kinds(kinds);
            draw_status_text(54, 5, advisor_x + 324, row_y, image_group(IMAGE_GROUP_FONT_WHITE));
            text_draw_number_ui((int32_t) city_player_current()->trading_amount[19] * 100, NUMBER_PREFIX_NONE, postfix_none, g_text_width + 324 + advisor_x, row_y,
                image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
            draw_luxury_kind_or_button(kind_count, kinds, row_index, row_y);
            return;
        }
        draw_status_text(54, 5, advisor_x + 324, row_y, image_group(IMAGE_GROUP_FONT_WHITE));
        {
            int32_t amount = (int32_t) city_player_current()->trading_amount[resource] * 100;
            amount = scale_amount_for_display(amount, resource);
            if (resource_uses_amount_phrase(resource)) {
                lang_text_draw_amount(8, 56, amount, g_text_width + 324 + advisor_x, row_y, image_group(IMAGE_GROUP_FONT_WHITE), 0);
            } else {
                text_draw_number_ui(amount, NUMBER_PREFIX_NONE, postfix_none, g_text_width + 324 + advisor_x, row_y, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
            }
        }
        return;
    }
    if (status == 2) {
        int32_t amount;

        g_text_width = 0;
        draw_status_text(54, 6, advisor_x + 324, row_y, image_group(IMAGE_GROUP_FONT_WHITE));
        amount = (int32_t) city_player_current()->trading_amount[resource] * 100;
        amount = scale_amount_for_display(amount, resource);
        text_draw_number_ui(amount, NUMBER_PREFIX_NONE, postfix_none, g_text_width + 324 + advisor_x, row_y, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
        return;
    }
    if (status == 3) {
        if (resource != RESOURCE_LUXURY_GOODS) {
            draw_status_text(54, 37, advisor_x + 324, row_y, image_group(IMAGE_GROUP_FONT_WHITE));
            return;
        }
        {
            int32_t kinds[16];
            int32_t kind_count;
            int32_t i;

            for (i = 0; i < 16; i++) {
                kinds[i] = 0;
            }
            g_text_width = 0;
            kind_count = empire_count_luxury_goods_kinds(kinds);
            draw_status_text(54, 37, advisor_x + 324, row_y, image_group(IMAGE_GROUP_FONT_WHITE));
            draw_luxury_kind_or_button(kind_count, kinds, row_index, row_y);
        }
        return;
    }
    draw_status_text(54, 38, advisor_x + 324, row_y, image_group(IMAGE_GROUP_FONT_WHITE));
}

static void draw_trade_row_prices(int32_t resource, int32_t row_y)
{
    int32_t unit_text;

    text_draw_number_centered(trade_buy_price(resource), NUMBER_PREFIX_NONE, postfix_none, advisor_x + 175, row_y, 100, image_group(IMAGE_GROUP_FONT_WHITE), 0);
    text_draw_number_centered(trade_sell_price(resource), NUMBER_PREFIX_NONE, postfix_none, advisor_x + 275, row_y, 100, image_group(IMAGE_GROUP_FONT_WHITE), 0);
    unit_text = prices_mode_unit_text(resource);
    draw_status_text(54, unit_text, advisor_x + 425, row_y, image_group(IMAGE_GROUP_FONT_WHITE));
}

void window_advisor_trade_draw_foreground(void)
{
    int32_t scroll;
    int32_t row_y;
    int32_t row_index;

    if ((city_resources.count - city_player_current()->resource_list_scroll) + 1 < 15) {
        city_player_current()->resource_list_scroll = city_resources.count - 14;
    }
    if (city_player_current()->resource_list_scroll < 1) {
        city_player_current()->resource_list_scroll = 1;
    }
    scroll = city_player_current()->resource_list_scroll;

    if (label_animation[LABEL_SLOT_ADVISOR_OPEN] <= 5) {
        return;
    }

    inner_panel_draw_ui(advisor_x + 16, advisor_y + 52, 36, 21, ui_origin_x, ui_origin_y);

    if (city_resources.count > 14) {
        arrow_buttons_get_focus(advisor_x + 573, advisor_y + 52, trade_advisor_scroll_up, 1, 10);
        arrow_buttons_handle_mouse(advisor_x + 573, advisor_y + 52, trade_advisor_scroll_up, 1);
        arrow_buttons_draw(advisor_x + 573, advisor_y + 52, trade_advisor_scroll_up, 1);
        arrow_buttons_get_focus(advisor_x + 597, advisor_y + 364, trade_advisor_scroll_down, 1, 10);
        arrow_buttons_handle_mouse(advisor_x + 597, advisor_y + 364, trade_advisor_scroll_down, 1);
        arrow_buttons_draw(advisor_x + 597, advisor_y + 364, trade_advisor_scroll_down, 1);
        inner_panel_draw_ui(advisor_x + 593, advisor_y + 76, 2, 18, ui_origin_x, ui_origin_y);
    }

    trade_advisor_buttons[TRADE_ADVISOR_LUXURY_BUTTON].y = 0;
    trade_advisor_buttons[TRADE_ADVISOR_LUXURY_BUTTON].y_end = 0;
    row_y = 0;
    row_index = 0;

    while (1) {
        int32_t resource = city_resources.list[scroll - 1];
        int32_t name_font;
        int32_t text_y = row_y + 61 + advisor_y;

        image_draw(image_group(IMAGE_GROUP_RESOURCE_ICONS) + resource, advisor_x + 22, row_y + 57 + advisor_y);

        if (row_index == focus_button_id - 4) {
            button_border_draw_ui(advisor_x + 20, row_y + 54 + advisor_y, 570, 24, 1, ui_origin_x, ui_origin_y);
        }

        name_font = city_player_current()->mothballed[resource] == 1 ? image_group(IMAGE_GROUP_FONT_RED) : image_group(IMAGE_GROUP_FONT_WHITE);
        lang_text_draw_ui(23, resource, advisor_x + 44, text_y, name_font, 0, ui_origin_x, ui_origin_y);

        if (city_player_current()->trade_prices_mode == 0) {
            draw_trade_row_normal(resource, row_index, text_y);
        } else {
            draw_trade_row_prices(resource, text_y);
        }

        scroll++;
        if (scroll > city_resources.count) {
            break;
        }
        row_y += 22;
        row_index++;
        if (row_y >= 330) {
            break;
        }
    }

    button_border_draw_ui(advisor_x + 398, advisor_y + 396, 200, 24, focus_button_id == 1, ui_origin_x, ui_origin_y);
    lang_text_draw_centered_ui(
        54, city_player_current()->trade_prices_mode == 0 ? 2 : 46, advisor_x + 400, advisor_y + 402, 200, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);

    button_border_draw_ui(advisor_x + 98, advisor_y + 396, 200, 24, focus_button_id == 2, ui_origin_x, ui_origin_y);
    lang_text_draw_centered_ui(54, 30, advisor_x + 100, advisor_y + 402, 200, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);

    window_advisor_trade_draw_scrollbar();
}

void city_toggle_resource_settings_step(void)
{
    int32_t *value = &city_player_current()->resource_settings_step;
    if (*value == 1) {
        *value = 16;
    } else if (*value == 16) {
        *value = 1;
    }
    window_redraw_request = 1;
}

void city_list_scroll_arrow(void)
{
    int32_t *scroll = &city_player_current()->resource_list_scroll;
    if (button_parameter == 0) {
        *scroll = *scroll - 1;
        if (*scroll <= 0) {
            *scroll = 1;
        }
        window_redraw_request = 1;
        return;
    }
    *scroll = *scroll + 1;
    if (*scroll + 13 > city_resources.count) {
        *scroll = city_resources.count - 13;
    }
    window_redraw_request = 1;
}

void button_trade_resource(void)
{
    int32_t resource = city_resources.list[(city_player_current()->resource_list_scroll + button_parameter) - 1];
    trade_resource_index = resource;
    city_trade_status_update(resource);
    window_current_id = WINDOW_RESOURCE_SETTINGS;
    window_redraw_request = 1;
}

void button_trade_prices_mode_toggle(void)
{
    window_redraw_request = 1;
    uint8_t *flag = &city_player_current()->trade_prices_mode;
    *flag = *flag == 0;
}

void button_trade_exit(void)
{
    button_empire_open();
    window_redraw_request = 1;
}
