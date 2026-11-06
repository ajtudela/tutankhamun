// Entries of the city's top menus: File > New game, Options > Cities - Egyptian / Classical, Help > Help, About, Mouse
// help and Warnings, and the right-click help of the top menu's info fields (funds, population, date). New game resets
// the city's construction state and goes back to the menu the game came from.

#include "window/top_menu_entries.h"
#include "building/building_type.h"
#include "editor/editor_data.h"
#include "game/game_data.h"
#include "io/game_file_upgrade.h"
#include "map/map_data.h"
#include "sound/sound_data.h"
#include "sound/sound_lib.h"
#include "widget/widget_data.h"
#include "window/free_play.h"
#include "window/main_menu.h"
#include "window/message_dialog.h"
#include "window/mission_flow.h"
#include "window/player_selection.h"
#include "window/window_data.h"
#include "window/window_id.h"
#include <stdint.h>

void menu_file_new_game(void)
{
    game_file_reset_mission_records();
    map_sweep_countdown = 0;
    city_frogs_countdown = 0;
    pyramid_speedup_countdown = 0;
    city_hailstorm_countdown = 0;
    construction_building_type = BUILDING_NONE;
    undo_available = 0;
    city_overlay = 0;
    city_overlay_previous = 0;
    if (editor_active == 1) {
        window_current_id = WINDOW_FRONT_MENU;
        window_redraw_request = 1;
    } else if (free_play_running()) {
        free_play_leave();
    } else {
        switch (game_is_replay) {
            case GAME_PLAY_CUSTOM_SCENARIO:
            case GAME_PLAY_CAMPAIGN_FROM_PERIOD:
            case GAME_PLAY_MISSION_REPLAY:
                window_main_menu_show();
                break;
            case GAME_PLAY_CAMPAIGN:
                window_player_selection_show();
                player_selection_button_proceed();
                break;
            default:
                break;
        }
    }
    if ((settings_data.sound_music_enabled != 0) && (sound_digital_ok != 0)) {
        sound_lib_play_stream(&sound_manager, setup_mp3_path, SOUND_STREAM_MUSIC, 0);
    }
}

void options_toggle_city_names(void)
{
    if (settings_data.classical_names != 0) {
        settings_data.classical_names = 0;
        g_menu_items[MENU_ITEM_OPTIONS_CITY_NAMES].text_number = OPTIONS_MENU_CITIES_EGYPTIAN;
        popup.menu_dropdown_keep_open = 1;
        return;
    }
    g_menu_items[MENU_ITEM_OPTIONS_CITY_NAMES].text_number = OPTIONS_MENU_CITIES_CLASSICAL;
    settings_data.classical_names = 1;
    popup.menu_dropdown_keep_open = 1;
}

void help_menu_show_help(void)
{
    window_message_dialog_show(MESSAGE_DIALOG_HELP, 0, 0);
}

void help_menu_show_about(void)
{
    window_message_dialog_show(MESSAGE_DIALOG_ABOUT, 0, 0);
}

void help_menu_toggle_tooltips(void)
{
    if (settings_data.tooltips == TOOLTIPS_FULL) {
        settings_data.tooltips = TOOLTIPS_NONE;
        g_menu_items[MENU_ITEM_HELP_TOOLTIPS].text_number = HELP_MENU_MOUSE_HELP_OFF;
        g_menu_items[MENU_ITEM_EDITOR_HELP_TOOLTIPS].text_number = HELP_MENU_MOUSE_HELP_OFF;
        popup.menu_dropdown_keep_open = 1;
        return;
    }
    if (settings_data.tooltips == TOOLTIPS_NONE) {
        settings_data.tooltips = TOOLTIPS_SOME;
        g_menu_items[MENU_ITEM_HELP_TOOLTIPS].text_number = HELP_MENU_MOUSE_HELP_SOME;
        g_menu_items[MENU_ITEM_EDITOR_HELP_TOOLTIPS].text_number = HELP_MENU_MOUSE_HELP_SOME;
        popup.menu_dropdown_keep_open = 1;
        return;
    }
    settings_data.tooltips = TOOLTIPS_FULL;
    g_menu_items[MENU_ITEM_HELP_TOOLTIPS].text_number = HELP_MENU_MOUSE_HELP_FULL;
    g_menu_items[MENU_ITEM_EDITOR_HELP_TOOLTIPS].text_number = HELP_MENU_MOUSE_HELP_FULL;
    popup.menu_dropdown_keep_open = 1;
}

void help_menu_toggle_warnings(void)
{
    if (settings_data.warnings != 0) {
        settings_data.warnings = 0;
        g_menu_items[MENU_ITEM_HELP_WARNINGS].text_number = HELP_MENU_WARNINGS_OFF;
        popup.menu_dropdown_keep_open = 1;
        return;
    }
    g_menu_items[MENU_ITEM_HELP_WARNINGS].text_number = HELP_MENU_WARNINGS_ON;
    settings_data.warnings = 1;
    popup.menu_dropdown_keep_open = 1;
}

void construction_right_click_help(void)
{
    if (top_menu_focus_info_id == TOP_MENU_INFO_FUNDS) {
        window_message_dialog_show(MESSAGE_DIALOG_TOP_FUNDS, 0, 0);
    } else if (top_menu_focus_info_id == TOP_MENU_INFO_POPULATION) {
        window_message_dialog_show(MESSAGE_DIALOG_TOP_POPULATION, 0, 0);
    } else if (top_menu_focus_info_id == TOP_MENU_INFO_DATE) {
        window_message_dialog_show(MESSAGE_DIALOG_TOP_DATE, 0, 0);
    }
    window_redraw_request = 1;
}
