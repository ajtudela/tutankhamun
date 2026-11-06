// The city window's top strip: funds, population and date at the resolution's positions, the menu bar, the
// rotate button, and which field is under the mouse. It is redrawn when a value changed or when forced; screen
// sizes other than the standard ones take the widest standard layout that fits, moved to the right edge.

#include "widget/top_menu.h"
#include "city/city_player.h"
#include "game/game_data.h"
#include "game/game_internal.h"
#include "graphics/graphics_data.h"
#include "graphics/image_collection.h"
#include "graphics/text_draw.h"
#include "graphics/ui_layout.h"
#include "input/input_data.h"
#include "text/lang.h"
#include "widget/menu_bar.h"
#include "widget/screen_layout.h"
#include "widget/widget_data.h"
#include "window/top_menu_entries.h"
#include "window/window_id.h"
#include "window/window_image_group.h"
#include <stdint.h>

int32_t g_drawn_month;

int32_t g_drawn_treasury;

int32_t g_drawn_population;

int32_t g_offset_date;

int32_t g_offset_population;

int32_t g_offset_funds;

enum top_menu_layout {
    TOP_MENU_TEXT_Y = 5,
    TOP_MENU_MAP_NAME_X = 260,
    TOP_MENU_MAP_NAME_Y = 4,
    TOP_MENU_INFO_TOP = 3,
    TOP_MENU_INFO_BOTTOM = 18,
    TOP_MENU_INFO_WIDTH = 128,
    TOP_MENU_EDITOR_BUTTON_SCREEN_PARTS = 5,
    TOP_MENU_BUTTON_GAP_640 = 10,
    TOP_MENU_BUTTON_GAP = 47,
};

enum top_menu_color {
    TOP_MENU_COLOR_DEFAULT = 0,
    TOP_MENU_COLOR_POPULATION = 0xFFFF,
};

typedef struct top_menu_values_layout {
    int32_t screen_width;
    int32_t offset_funds;
    int32_t offset_population;
    int32_t offset_date;
    int32_t funds_text_x;
    int32_t funds_number_x;
    int32_t population_text_x;
    int32_t population_number_x;
    int32_t month_x;
    int32_t year_x;
    int32_t year_kind;
    int32_t rotate_button_gap;
} top_menu_values_layout;

enum {
    TOP_MENU_VALUES_LAYOUTS = 3,
};

static const top_menu_values_layout top_menu_values_layouts[TOP_MENU_VALUES_LAYOUTS] = {
    { 1024, 493, 637, 885, 495, 501, 645, 651, 883, 883, 0, TOP_MENU_BUTTON_GAP },
    { 800, 338, 458, 652, 343, 339, 470, 466, 655, 655, 0, TOP_MENU_BUTTON_GAP },
    { 640, 242, 367, 485, 250, 245, 375, 367, 515, 504, 2, TOP_MENU_BUTTON_GAP_640 },
};

static const top_menu_values_layout *values_layout_of_screen(int32_t screen_width)
{
    for (int32_t i = 0; i < TOP_MENU_VALUES_LAYOUTS - 1; i++) {
        if (screen_width >= top_menu_values_layouts[i].screen_width) {
            return &top_menu_values_layouts[i];
        }
    }
    return &top_menu_values_layouts[TOP_MENU_VALUES_LAYOUTS - 1];
}

static void draw_values_of_screen_layout(const city_player *player, int32_t treasury_font, int32_t font)
{
    const top_menu_values_layout *layout = values_layout_of_screen(g_screen_width);
    int32_t dx = g_screen_width - layout->screen_width;
    g_offset_funds = layout->offset_funds + dx;
    g_offset_population = layout->offset_population + dx;
    g_offset_date = layout->offset_date + dx;
    lang_text_draw_ui(LANG_GROUP_TOP_MENU, TOP_MENU_TEXT_FUNDS, layout->funds_text_x + dx, TOP_MENU_TEXT_Y, treasury_font, TOP_MENU_COLOR_DEFAULT, ui_origin_x,
        ui_origin_y);
    text_draw_number_ui(player->treasury, NUMBER_PREFIX_NONE, postfix_none, g_text_width + layout->funds_number_x + dx, TOP_MENU_TEXT_Y, treasury_font,
        TOP_MENU_COLOR_DEFAULT, ui_origin_x, ui_origin_y);
    g_text_width = 0;
    lang_text_draw_ui(LANG_GROUP_TOP_MENU, TOP_MENU_TEXT_POPULATION, layout->population_text_x + dx, TOP_MENU_TEXT_Y, font, TOP_MENU_COLOR_POPULATION,
        ui_origin_x, ui_origin_y);
    text_draw_number_ui(player->population, NUMBER_PREFIX_NONE, postfix_none, g_text_width + layout->population_number_x + dx, TOP_MENU_TEXT_Y, font,
        TOP_MENU_COLOR_POPULATION, ui_origin_x, ui_origin_y);
    g_text_width = 0;
    lang_text_draw_ui(LANG_GROUP_MONTHS_SHORT, game_time_month, layout->month_x + dx, TOP_MENU_TEXT_Y, font, TOP_MENU_COLOR_DEFAULT, ui_origin_x, ui_origin_y);
    lang_text_draw_year(layout->year_kind, game_time_year, g_text_width + layout->year_x + dx, TOP_MENU_TEXT_Y, font, TOP_MENU_COLOR_DEFAULT);
    rotate_button_x[0].x = (int16_t) (g_offset_date - layout->rotate_button_gap);
    menu_bar_draw_side_button();
}

static int32_t window_hides_top_menu(uint32_t window)
{
    if (window >= WINDOW_MISSION_END) {
        return 1;
    }
    switch (window) {
        case WINDOW_FRONT_MENU:
        case WINDOW_INTERMEZZO:
        case WINDOW_MESSAGE_DIALOG:
        case WINDOW_ADVISORS:
        case WINDOW_FILE_DIALOG:
        case WINDOW_NUMERIC_INPUT:
        case WINDOW_SELECT_LIST:
        case WINDOW_MAP_FILE_DIALOG:
        case WINDOW_MESSAGE_LIST:
        case WINDOW_TOOLTIP_HOVER:
        case WINDOW_MESSAGE_VIDEO:
        case WINDOW_EMPIRE:
        case WINDOW_EDITOR_EMPIRE:
        case WINDOW_MISSION_SELECTION:
        case WINDOW_CITY_INTRO:
        case WINDOW_RESOURCE_SETTINGS:
        case WINDOW_LABOR_PRIORITY:
        case WINDOW_FESTIVAL_DIALOG:
        case WINDOW_SET_SALARY:
        case WINDOW_DONATE_TO_CITY:
        case WINDOW_SEND_GIFT:
        case WINDOW_PLAYER_SELECTION:
        case WINDOW_MAIN_MENU:
            return 1;
        default:
            return 0;
    }
}

void widget_top_menu_draw(int32_t force)
{
    if (editor_active == 1 && window_current_id == WINDOW_CITY) {
        text_draw((const uint8_t *) scenario_map_name, TOP_MENU_MAP_NAME_X, TOP_MENU_MAP_NAME_Y, image_group(IMAGE_GROUP_FONT_WHITE), TOP_MENU_COLOR_DEFAULT);
    }
    if (window_hides_top_menu(window_current_id)) {
        return;
    }

    const city_player *player = city_player_current();
    int32_t need_draw = 1;
    if (player->population == g_drawn_population) {
        need_draw = force;
    }
    if (player->treasury != g_drawn_treasury) {
        need_draw = 1;
    }
    if (game_time_month == g_drawn_month && need_draw == 0) {
        if (g_top_menu_force < 1) {
            return;
        }
        if (g_top_menu_force == 1 || g_top_menu_force == 2) {
            top_menu_draw_panel_strip();
        }
    } else {
        g_top_menu_force = 1;
        top_menu_draw_panel_strip();
    }

    g_top_menu_force = 0;
    g_drawn_population = player->population;
    g_drawn_treasury = player->treasury;

    if (editor_active == 1) {
        menu_bar_draw(menu_bar_editor, 4);
        rotate_button_x[0].x = (int16_t) (g_screen_width - (g_screen_width / TOP_MENU_EDITOR_BUTTON_SCREEN_PARTS));
        menu_bar_draw_side_button();
        return;
    }

    menu_bar_draw(menu_bar_city, 4);
    g_text_width = 0;
    int32_t treasury_font = image_group(IMAGE_GROUP_FONT_BLACK);
    if (player->treasury < 0) {
        treasury_font = image_group(IMAGE_GROUP_FONT_RED);
    }
    int32_t font = image_group(IMAGE_GROUP_FONT_BLACK);
    if (!screen_layout_is_standard_size()) {
        draw_values_of_screen_layout(player, treasury_font, font);
        return;
    }
    if (g_screen_width == 640) {
        g_offset_funds = 242;
        g_offset_population = 367;
        g_offset_date = 485;
        lang_text_draw_ui(LANG_GROUP_TOP_MENU, TOP_MENU_TEXT_FUNDS, 250, TOP_MENU_TEXT_Y, treasury_font, TOP_MENU_COLOR_DEFAULT, ui_origin_x, ui_origin_y);
        text_draw_number_ui(player->treasury, NUMBER_PREFIX_NONE, postfix_none, g_text_width + 245, TOP_MENU_TEXT_Y, treasury_font, TOP_MENU_COLOR_DEFAULT,
            ui_origin_x, ui_origin_y);
        g_text_width = 0;
        lang_text_draw_ui(LANG_GROUP_TOP_MENU, TOP_MENU_TEXT_POPULATION, 375, TOP_MENU_TEXT_Y, font, TOP_MENU_COLOR_POPULATION, ui_origin_x, ui_origin_y);
        text_draw_number_ui(player->population, NUMBER_PREFIX_NONE, postfix_none, g_text_width + 367, TOP_MENU_TEXT_Y, font, TOP_MENU_COLOR_POPULATION,
            ui_origin_x, ui_origin_y);
        g_text_width = 0;
        lang_text_draw_ui(LANG_GROUP_MONTHS_SHORT, game_time_month, 515, TOP_MENU_TEXT_Y, font, TOP_MENU_COLOR_DEFAULT, ui_origin_x, ui_origin_y);
        lang_text_draw_year(2, game_time_year, g_text_width + 504, TOP_MENU_TEXT_Y, font, TOP_MENU_COLOR_DEFAULT);
    } else if (g_screen_width == 800) {
        g_offset_funds = 338;
        g_offset_population = 458;
        g_offset_date = 652;
        lang_text_draw_ui(LANG_GROUP_TOP_MENU, TOP_MENU_TEXT_FUNDS, 343, TOP_MENU_TEXT_Y, treasury_font, TOP_MENU_COLOR_DEFAULT, ui_origin_x, ui_origin_y);
        text_draw_number_ui(player->treasury, NUMBER_PREFIX_NONE, postfix_none, g_text_width + 339, TOP_MENU_TEXT_Y, treasury_font, TOP_MENU_COLOR_DEFAULT,
            ui_origin_x, ui_origin_y);
        g_text_width = 0;
        lang_text_draw_ui(LANG_GROUP_TOP_MENU, TOP_MENU_TEXT_POPULATION, 470, TOP_MENU_TEXT_Y, font, TOP_MENU_COLOR_POPULATION, ui_origin_x, ui_origin_y);
        text_draw_number_ui(player->population, NUMBER_PREFIX_NONE, postfix_none, g_text_width + 466, TOP_MENU_TEXT_Y, font, TOP_MENU_COLOR_POPULATION,
            ui_origin_x, ui_origin_y);
        g_text_width = 0;
        lang_text_draw_ui(LANG_GROUP_MONTHS_SHORT, game_time_month, 655, TOP_MENU_TEXT_Y, font, TOP_MENU_COLOR_DEFAULT, ui_origin_x, ui_origin_y);
        lang_text_draw_year(0, game_time_year, g_text_width + 655, TOP_MENU_TEXT_Y, font, TOP_MENU_COLOR_DEFAULT);
    } else if (g_screen_width == 1024) {
        g_offset_funds = 493;
        g_offset_population = 637;
        g_offset_date = 885;
        lang_text_draw_ui(LANG_GROUP_TOP_MENU, TOP_MENU_TEXT_FUNDS, 495, TOP_MENU_TEXT_Y, treasury_font, TOP_MENU_COLOR_DEFAULT, ui_origin_x, ui_origin_y);
        text_draw_number_ui(player->treasury, NUMBER_PREFIX_NONE, postfix_none, g_text_width + 501, TOP_MENU_TEXT_Y, treasury_font, TOP_MENU_COLOR_DEFAULT,
            ui_origin_x, ui_origin_y);
        g_text_width = 0;
        lang_text_draw_ui(LANG_GROUP_TOP_MENU, TOP_MENU_TEXT_POPULATION, 645, TOP_MENU_TEXT_Y, font, TOP_MENU_COLOR_POPULATION, ui_origin_x, ui_origin_y);
        text_draw_number_ui(player->population, NUMBER_PREFIX_NONE, postfix_none, g_text_width + 651, TOP_MENU_TEXT_Y, font, TOP_MENU_COLOR_POPULATION,
            ui_origin_x, ui_origin_y);
        g_text_width = 0;
        lang_text_draw_ui(LANG_GROUP_MONTHS_SHORT, game_time_month, 883, TOP_MENU_TEXT_Y, font, TOP_MENU_COLOR_DEFAULT, ui_origin_x, ui_origin_y);
        lang_text_draw_year(0, game_time_year, g_text_width + 883, TOP_MENU_TEXT_Y, font, TOP_MENU_COLOR_DEFAULT);
    }

    if (g_screen_width == 640) {
        rotate_button_x[0].x = (int16_t) (g_offset_date - TOP_MENU_BUTTON_GAP_640);
    } else {
        rotate_button_x[0].x = (int16_t) (g_offset_date - TOP_MENU_BUTTON_GAP);
    }
    menu_bar_draw_side_button();
}

void city_top_menu_hover_update(void)
{
    top_menu_focus_info_id = TOP_MENU_INFO_NONE;
    if (TOP_MENU_INFO_TOP < mouse_y && mouse_y < TOP_MENU_INFO_BOTTOM) {
        if (g_offset_funds < mouse_x && mouse_x < g_offset_funds + TOP_MENU_INFO_WIDTH) {
            top_menu_focus_info_id = TOP_MENU_INFO_FUNDS;
        }
        if (g_offset_population < mouse_x && mouse_x < g_offset_population + TOP_MENU_INFO_WIDTH) {
            top_menu_focus_info_id = TOP_MENU_INFO_POPULATION;
        }
        if (g_offset_date < mouse_x && mouse_x < g_offset_date + TOP_MENU_INFO_WIDTH) {
            top_menu_focus_info_id = TOP_MENU_INFO_DATE;
        }
    }
}
