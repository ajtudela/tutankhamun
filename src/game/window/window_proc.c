// The window procedure: mouse messages set the position and button bits mouse_update reads; key messages run at once
// (text input, Shift / Ctrl, scrolling and paging, bookmarks F1-F4, display F5-F9, F12 fog, Esc), PLATFORM_MSG_CHAR runs the
// hotkeys; the rest is Win32 activation, painting and sizing. While the Bink player or the message box runs, the
// messages the game does not need go to DefWindowProc.
//
// In a transport's building info the order keys write the transport into building_info_warship_id.

#include "window/window_proc.h"
#include "building/building.h"
#include "building/building_type.h"
#include "building/royal_tomb.h"
#include "city/gods.h"
#include "city/message.h"
#include "editor/editor_menu.h"
#include "figure/figure_type.h"
#include "figure/formation.h"
#include "game/cheats.h"
#include "game/game_data.h"
#include "game/game_internal.h"
#include "game/player_record.h"
#include "graphics/graphics_data.h"
#include "graphics/graphics_display.h"
#include "graphics/graphics_present.h"
#include "input/input_data.h"
#include "input/mouse.h"
#include "map/grid.h"
#include "platform/platform_data.h"
#include "platform/platform_dialog.h"
#include "platform/platform_input.h"
#include "platform/platform_message.h"
#include "platform/platform_win.h"
#include "platform/platform_window.h"
#include "platform/status_log.h"
#include "sound/sound_data.h"
#include "sound/sound_lib.h"
#include "text/lang.h"
#include "text/text_lookup.h"
#include "video/bink.h"
#include "video/smacker.h"
#include "widget/city_view_rotate.h"
#include "widget/city_with_overlay.h"
#include "widget/widget_data.h"
#include "window/advisor/trade.h"
#include "window/advisors.h"
#include "window/building_info.h"
#include "window/building_info_military.h"
#include "window/building_info_ships.h"
#include "window/building_info_storage.h"
#include "window/building_info_storage_orders.h"
#include "window/building_menu.h"
#include "window/display_resolution_list.h"
#include "window/empire_editor_input.h"
#include "window/file_dialog_buttons.h"
#include "window/hotkeys.h"
#include "window/input_dialogs.h"
#include "window/main_menu.h"
#include "window/map_selection.h"
#include "window/message_box.h"
#include "window/options_buttons.h"
#include "window/options_windows.h"
#include "window/player_selection.h"
#include "window/popup_dialog.h"
#include "window/text_input.h"
#include "window/window_id.h"
#include "window/window_input.h"
#include <stdint.h>
#include <string.h>

int32_t app_minimized;

enum window_proc_keys {
    LIST_PAGE_LINES = 12,
    MAP_LIST_PAGE_LINES = 15,
    EMPIRE_NUDGE_STEP = 1,
    EMPIRE_NUDGE_CTRL_STEP = 8,
    DEBUG_EVENT_TEXT_LAST = 4,
    DEBUG_EVENT_LEVEL_FIRST = 1,
    DEBUG_EVENT_LEVEL_LAST = 3,
    DEBUG_EVENT_KIND_LAST = 4,
    DEBUG_GRID_OVERLAY_LAST = 27,
    NUMERIC_INPUT_CHEAT_DIGIT = 3,
};

static int32_t empire_nudge_step(void)
{
    return (key_ctrl != 0) ? EMPIRE_NUDGE_CTRL_STEP : EMPIRE_NUDGE_STEP;
}

static int32_t building_info_shows_storage(void)
{
    int16_t type = building_info_building()->type;
    return type == BUILDING_STORAGE_YARD || type == BUILDING_STORAGE_YARD_UP || type == BUILDING_STORAGE_ROOM;
}

static void key_page(int32_t down)
{
    switch (window_current_id) {
        case WINDOW_CITY:
            button_parameter = down ? ROTATION_BUTTON_RIGHT : ROTATION_BUTTON_LEFT;
            rotation_button_click();
            break;
        case WINDOW_FILE_DIALOG:
        case WINDOW_MAP_FILE_DIALOG:
            button_parameter = down;
            for (int32_t i = LIST_PAGE_LINES; i != 0; i--) {
                file_dialog_button_scroll();
            }
            window_redraw_request = 1;
            break;
        case WINDOW_MAP_SELECTION:
            button_parameter = down;
            for (int32_t i = MAP_LIST_PAGE_LINES; i != 0; i--) {
                map_selection_scroll();
            }
            window_redraw_request = 1;
            break;
        case WINDOW_PLAYER_SELECTION:
            button_parameter = down;
            for (int32_t i = LIST_PAGE_LINES; i != 0; i--) {
                player_selection_button_scroll();
            }
            window_redraw_request = 1;
            break;
        case WINDOW_ADVISORS:
            if (advisor_shown == ADVISOR_TRADE) {
                button_parameter = down;
                for (int32_t i = LIST_PAGE_LINES; i != 0; i--) {
                    city_list_scroll_arrow();
                }
                window_redraw_request = 1;
            }
            break;
        case WINDOW_BUILDING_INFO:
            if (building_info_shows_storage()) {
                button_parameter = down;
                for (int32_t i = 0; i < storage_orders_rows(); i++) {
                    building_orders_scroll_arrow();
                }
                window_redraw_request = 1;
            }
            break;
        default:
            break;
    }
}

static void key_line(int32_t down)
{
    int32_t *flag = down ? &key_scroll_down : &key_scroll_up;

    switch (window_current_id) {
        case WINDOW_ADVISORS:
            if (advisor_shown == ADVISOR_TRADE) {
                button_parameter = down;
                city_list_scroll_arrow();
            }
            break;
        case WINDOW_BUILDING_INFO:
            if (building_info_shows_storage()) {
                button_parameter = down;
                if (building_info_orders_shown != 0) {
                    building_orders_scroll_arrow();
                } else {
                    building_info_toggle_dc();
                }
            }
            break;
        case WINDOW_FILE_DIALOG:
        case WINDOW_MAP_FILE_DIALOG:
            button_parameter = down;
            file_dialog_button_scroll();
            break;
        case WINDOW_MAP_SELECTION:
            button_parameter = down;
            map_selection_scroll();
            break;
        case WINDOW_PLAYER_SELECTION:
            button_parameter = down;
            player_selection_button_scroll();
            break;
        default:
            if (down) {
                keyboard_right();
                empire_editor_nudge_object(DIRECTION_BOTTOM, empire_nudge_step());
            } else {
                keyboard_left();
                empire_editor_nudge_object(DIRECTION_TOP, empire_nudge_step());
            }
            break;
    }
    *flag = 1;
}

static void key_display_mode(int32_t mode)
{
    uint8_t window = window_current_id;
    if (window == WINDOW_SELECT_LIST || window == WINDOW_EDITOR_SELECT_LIST || window == WINDOW_NUMERIC_INPUT ||
        (mode == DISPLAY_MODE_640X480 && window == WINDOW_EDITOR_EMPIRE)) {
        return;
    }
    display_resolution_take_request();
    g_skip_refresh = 1;
    graphics_display_shutdown(1);
    platform_set_display_mode(mode);
    graphics_screen_resize();
    city_warning_clear_all();
    graphics_begin();
    g_skip_refresh = 0;
    display_mode_confirm_open();
    window_redraw_request = 1;
}

static void centre_window(void *hwnd)
{
    int32_t width;
    int32_t height;
    platform_window_outer_size(hwnd, display_width, screen_height, &width, &height);
    platform_window_move(hwnd, (desktop_width - g_screen_width) / 2, (desktop_height - g_screen_height) / 2, width, height);
}

static int32_t key_down_ends_message(void *hwnd, uintptr_t key)
{
    switch (key) {
        case PLATFORM_KEY_BACKSPACE:
            keyboard_backspace();
            break;
        case PLATFORM_KEY_RETURN:
            if (window_current_id == WINDOW_NUMERIC_INPUT) {
                numeric_input_ok();
            } else {
                keyboard_delete();
            }
            keyboard_return();
            break;
        case PLATFORM_KEY_CONTROL:
            key_ctrl = 1;
            break;
        case PLATFORM_KEY_ESCAPE: {
            video_stop();
            const char *text = lang_get_string(LANG_GROUP_POPUP, POPUP_QUIT_GAME);
            const char *title = lang_get_string(LANG_GROUP_POPUP, POPUP_QUIT_GAME + 1);
            int32_t answer = window_message_box_show(title, text, PLATFORM_BOX_YES_NO);
            mouse_buttons = 0;
            if (answer == PLATFORM_ANSWER_YES) {
                game_quit_request = GAME_QUIT_REQUESTED;
                window_redraw_request = 1;
            }
            return 1;
        }
        case PLATFORM_KEY_PAGE_UP:
            key_page(0);
            break;
        case PLATFORM_KEY_PAGE_DOWN:
            key_page(1);
            break;
        case PLATFORM_KEY_END:
            keyboard_end();
            break;
        case PLATFORM_KEY_HOME:
            keyboard_home();
            if (window_current_id == WINDOW_CITY) {
                button_parameter = ROTATION_BUTTON_RESET;
                rotation_button_click();
            }
            break;
        case PLATFORM_KEY_LEFT:
            keyboard_left();
            empire_editor_nudge_object(DIRECTION_LEFT, empire_nudge_step());
            key_scroll_left = 1;
            break;
        case PLATFORM_KEY_UP:
            key_line(0);
            break;
        case PLATFORM_KEY_RIGHT:
            keyboard_right();
            empire_editor_nudge_object(DIRECTION_RIGHT, empire_nudge_step());
            key_scroll_right = 1;
            break;
        case PLATFORM_KEY_DOWN:
            key_line(1);
            break;
        case PLATFORM_KEY_INSERT:
            keyboard_insert();
            break;
        case PLATFORM_KEY_DELETE:
            if (window_current_id == WINDOW_NUMERIC_INPUT) {
                numeric_input_cancel();
            } else {
                keyboard_delete();
            }
            break;
        case 'A':
        case 'a':
            if (cheats_disabled == 0) {
                if (key_ctrl != 0 && window_current_id == WINDOW_CITY) {
                    cheat_allow_all ^= 1;
                }
                building_menu_update();
                sidebar_build_buttons_update();
                if (key_ctrl != 0 && settings_data.player_name[0] != 0 && window_current_id != WINDOW_NAME_INPUT) {
                    campaign_mark_all_played(&campaign_object.campaign);
                    player_data_write_file(&campaign_object, player_record_path());
                }
            }
            break;
        case 'G':
        case 'g':
            if (cheats_disabled == 0 && key_ctrl != 0 && window_current_id == WINDOW_CITY) {
                debug_event_text++;
                if (debug_event_text > DEBUG_EVENT_TEXT_LAST) {
                    debug_event_text = 0;
                }
            }
            break;
        case 'H':
        case 'h':
            if (cheats_disabled == 0 && key_ctrl != 0 && window_current_id == WINDOW_CITY) {
                debug_event_level++;
                if (debug_event_level > DEBUG_EVENT_LEVEL_LAST) {
                    debug_event_level = DEBUG_EVENT_LEVEL_FIRST;
                }
            }
            break;
        case 'J':
        case 'j':
            if (cheats_disabled == 0 && key_ctrl != 0 && window_current_id == WINDOW_CITY) {
                debug_event_kind++;
                if (debug_event_kind > DEBUG_EVENT_KIND_LAST) {
                    debug_event_kind = 0;
                }
            }
            break;
        case 'K':
        case 'k':
            if (cheats_disabled == 0 && key_ctrl != 0 && window_current_id == WINDOW_CITY && debug_event_kind != 0) {
                city_gods_perform_blessing_or_curse(debug_event_text, debug_event_kind);
            }
            break;
        case 'b':
            if (cheats_disabled == 0) {
                settings_data.debug_grid_overlay--;
                if (settings_data.debug_grid_overlay < 0) {
                    settings_data.debug_grid_overlay = DEBUG_GRID_OVERLAY_LAST;
                }
            }
            break;
        case 'c':
            if (window_current_id == WINDOW_NUMERIC_INPUT && cheats_disabled != 0) {
                button_parameter = NUMERIC_INPUT_CHEAT_DIGIT;
                numeric_input_digit();
            }
            break;
        case PLATFORM_KEY_F1:
        case PLATFORM_KEY_F2:
        case PLATFORM_KEY_F3:
        case PLATFORM_KEY_F4:
            if (key_ctrl != 0) {
                hotkey_set_bookmark((int32_t) key - PLATFORM_KEY_F1);
            } else {
                hotkey_go_to_bookmark((int32_t) key - PLATFORM_KEY_F1);
            }
            break;
        case PLATFORM_KEY_F5:
            if (settings_data.fullscreen == 0) {
                centre_window(hwnd);
            }
            break;
        case PLATFORM_KEY_F6:
            g_skip_refresh = 1;
            settings_data.fullscreen = settings_data.fullscreen == 0;
            graphics_display_shutdown(1);
            graphics_begin();
            g_skip_refresh = 0;
            platform_window_raise(platform_main_window);
            window_redraw_request = 1;
            break;
        case PLATFORM_KEY_F7:
            key_display_mode(DISPLAY_MODE_640X480);
            break;
        case PLATFORM_KEY_F8:
            key_display_mode(DISPLAY_MODE_800X600);
            break;
        case PLATFORM_KEY_F9:
            key_display_mode(DISPLAY_MODE_1024X768);
            break;
        case PLATFORM_KEY_F12:
            g_fog_disabled = g_fog_disabled == 0;
            break;
        default:
            break;
    }
    return 0;
}

typedef void (*button_handler)(void);

static void with_button_parameter(int32_t value, button_handler fn)
{
    int32_t saved = button_parameter;
    button_parameter = value;
    fn();
    button_parameter = saved;
}

static int32_t legion_may_charge(const formation *legion)
{
    return legion->figure_type == FIGURE_CHARIOTEER && legion->has_move_order == 0;
}

static void char_selected_legion(uintptr_t c, int32_t formation_id)
{
    formation *legion = formation_get(formation_id);
    switch (c) {
        case 'C':
        case 'c':
            if (legion_may_charge(legion)) {
                window_current_id = WINDOW_CITY_LEGION_ORDER;
            }
            break;
        case 'F':
        case 'f':
            building_info_formation_id = formation_id;
            legion_return_button();
            break;
        case 'L':
        case 'l':
            if (legion->figure_type != FIGURE_CHARIOTEER) {
                legion->layout = (legion->alternate_layout != 0) ? FORMATION_SINGLE_LINE_2 : FORMATION_SINGLE_LINE_1;
            }
            break;
        case 'M':
        case 'm':
            legion->prev_layout = legion->layout;
            legion->layout = FORMATION_MOP_UP;
            break;
        case 'N':
        case 'n':
            legion->layout = FORMATION_CHARGE;
            break;
        case 'R':
        case 'r':
            button_parameter2 = 1;
            legion_order_button_toggle();
            break;
        case 'T':
        case 't':
            legion->layout = (legion->alternate_layout != 0) ? FORMATION_DOUBLE_LINE_2 : FORMATION_DOUBLE_LINE_1;
            break;
        default:
            break;
    }
}

static void char_transport_ship_orders(uintptr_t c)
{
    switch (c) {
        case 'E':
        case 'e':
            with_button_parameter(1, transport_ship_button);
            break;
        case 'H':
        case 'h':
            with_button_parameter(0, transport_ship_button);
            break;
        case 'K':
        case 'k':
            with_button_parameter(2, transport_ship_button);
            break;
        case 'R':
        case 'r':
            with_button_parameter(3, transport_ship_button);
            break;
        case 'W':
        case 'w':
            with_button_parameter(4, transport_ship_order_button);
            break;
        default:
            break;
    }
}

static void char_warship_orders(uintptr_t c)
{
    switch (c) {
        case 'A':
        case 'a':
            with_button_parameter(2, warship_button);
            break;
        case 'H':
        case 'h':
            with_button_parameter(0, warship_button);
            break;
        case 'N':
        case 'n':
            with_button_parameter(1, warship_button);
            break;
        case 'R':
        case 'r':
            with_button_parameter(3, warship_button);
            break;
        case 'W':
        case 'w':
            with_button_parameter(4, warship_order_button);
            break;
        default:
            break;
    }
}

static void char_legion_info(uintptr_t c)
{
    const formation *legion = formation_get(formation_selected);
    switch (c) {
        case 'C':
        case 'c':
            if (legion_may_charge(legion)) {
                button_parameter = 1;
                legion_order_button();
            }
            break;
        case 'F':
        case 'f':
            legion_return_button();
            break;
        case 'L':
        case 'l':
            if (legion->figure_type != FIGURE_CHARIOTEER) {
                button_parameter = 1;
                legion_order_button();
            }
            break;
        case 'M':
        case 'm':
            button_parameter = 3;
            legion_order_button();
            if (legion->layout == FORMATION_MOP_UP) {
                window_current_id = WINDOW_CITY;
            }
            break;
        case 'N':
        case 'n':
            button_parameter = 2;
            legion_order_button();
            break;
        case 'R':
        case 'r':
            legion_order_button_toggle();
            break;
        case 'T':
        case 't':
            button_parameter = 0;
            legion_order_button();
            break;
        default:
            break;
    }
}

static void char_hotkeys(uintptr_t c, uint8_t window)
{
    if (c == ' ') {
        int32_t swap = 1;
        if (window == WINDOW_CITY_LEGION_ORDER || window == WINDOW_CITY_TRANSPORT_SELECTED || window == WINDOW_CITY_WARSHIP_SELECTED) {
            window_current_id = WINDOW_CITY;
        } else if (window != WINDOW_CITY) {
            swap = 0;
        }
        if (swap) {
            int32_t previous = city_overlay_previous;
            city_overlay_previous = city_overlay;
            city_overlay = previous;
            window_redraw_request = 1;
            royal_tombs_update_all_images();
        }
    }
    if (editor_active != 0) {
        if (c == 'H' || c == 'h') {
            overlay_show_all = overlay_show_all == 0;
        }
        return;
    }
    if (c == 'F' || c == 'f') {
        hotkey_toggle_overlay(OVERLAY_FIRE);
    }
    if (c == 'D' || c == 'd') {
        hotkey_toggle_overlay(OVERLAY_DAMAGE);
    }
    if (c == 'C' || c == 'c') {
        hotkey_toggle_overlay(OVERLAY_CRIME);
    }
    if (c == 'T' || c == 't') {
        hotkey_toggle_overlay(OVERLAY_PROBLEMS);
    }
    if (c == 'W' || c == 'w') {
        hotkey_toggle_overlay(OVERLAY_WATER);
    }
    if (c == 'H' || c == 'h') {
        hotkey_toggle_overlay(OVERLAY_HIDE_CLIFFS);
    }
    if (c == 'X' || c == 'x') {
        hotkey_toggle_overlay(OVERLAY_TAX_INCOME);
    }
    if (c == 'Y' || c == 'y') {
        hotkey_toggle_overlay(OVERLAY_MALARIA);
    }
    if (c == 'Z' || c == 'z') {
        hotkey_toggle_overlay(OVERLAY_DISEASE);
    }
    switch (c) {
        case '`':
            hotkey_show_advisor(ADVISOR_LABOR);
            break;
        case '1':
            hotkey_show_advisor(ADVISOR_MILITARY);
            break;
        case '2':
            hotkey_show_advisor(ADVISOR_KINGDOM);
            break;
        case '3':
            hotkey_show_advisor(ADVISOR_RATINGS);
            break;
        case '4':
            hotkey_show_advisor(ADVISOR_TRADE);
            break;
        case '5':
            hotkey_show_advisor(ADVISOR_POPULATION);
            break;
        case '6':
            hotkey_show_advisor(ADVISOR_HEALTH);
            break;
        case '7':
            hotkey_show_advisor(ADVISOR_EDUCATION);
            break;
        case '8':
            hotkey_show_advisor(ADVISOR_ENTERTAINMENT);
            break;
        case '9':
            hotkey_show_advisor(ADVISOR_RELIGION);
            break;
        case '0':
            hotkey_show_advisor(ADVISOR_FINANCIAL);
            break;
        case '-':
            hotkey_show_advisor(ADVISOR_CHIEF);
            break;
        case '=':
            hotkey_show_advisor(ADVISOR_MONUMENTS);
            break;
        case 'L':
        case 'l':
            hotkey_cycle_legion();
            break;
        default:
            break;
    }
}

static void char_message(uintptr_t c)
{
    uint8_t window;

    keyboard_character((uint8_t) c);
    if (c == '[') {
        hotkey_change_game_speed(1);
        window = window_current_id;
    } else if (c == ']') {
        hotkey_change_game_speed(0);
        window = window_current_id;
    } else {
        if (c == 'P' || c == 'p') {
            uint8_t saved = window_current_id;
            if (saved == WINDOW_CITY_LEGION_SELECTED || saved == WINDOW_CITY_LEGION_ORDER || saved == WINDOW_CITY_TRANSPORT_SELECTED ||
                saved == WINDOW_CITY_WARSHIP_SELECTED) {
                window_current_id = WINDOW_CITY;
            }
            if (window_current_id == WINDOW_CITY && editor_active == 0) {
                settings_data.paused = settings_data.paused == 0;
                city_warning_clear_all();
            }
            window_current_id = saved;
        }
        window = window_current_id;
        if ((c == 'M' || c == 'm') && editor_active == 0) {
            editor_wide_bounds = 0;
            construction_hover_locked = construction_hover_locked == 0;
        }
    }

    if (window == WINDOW_CITY_LEGION_SELECTED) {
        char_selected_legion(c, formation_selected);
        return;
    }
    if (window == WINDOW_CITY_LEGION_ORDER) {
        if (formation_selected != 0) {
            window_current_id = WINDOW_CITY_LEGION_SELECTED;
            char_selected_legion(c, formation_selected);
            return;
        }
    } else if (window == WINDOW_CITY_TRANSPORT_SELECTED) {
        if (selected_transport_active != 0) {
            building_info_transport_id = selected_transport_id;
            char_transport_ship_orders(c);
            return;
        }
    } else if (window == WINDOW_CITY_WARSHIP_SELECTED) {
        if (selected_warship_active != 0) {
            building_info_warship_id = selected_warship_id;
            char_warship_orders(c);
            return;
        }
    } else if (window == WINDOW_BUILDING_INFO) {
        if (building_info_mode == BUILDING_INFO_MODE_LEGION) {
            char_legion_info(c);
            return;
        }
        if (building_info_mode == BUILDING_INFO_MODE_WARSHIP) {
            building_info_warship_id = selected_warship_id;
            char_warship_orders(c);
            return;
        }
        if (building_info_mode == BUILDING_INFO_MODE_TRANSPORT_SHIP) {
            building_info_warship_id = selected_transport_id;
            char_transport_ship_orders(c);
            return;
        }
    } else if (window == WINDOW_NUMERIC_INPUT) {
        if (c >= '0' && c <= '9') {
            button_parameter = (int32_t) c - '0';
            numeric_input_digit();
        }
        return;
    }
    char_hotkeys(c, window);
}

static int32_t message_box_needs(uint32_t message)
{
    switch (message) {
        case PLATFORM_MSG_SET_CURSOR:
        case PLATFORM_MSG_PAINT:
        case PLATFORM_MSG_MOUSE_MOVE:
        case PLATFORM_MSG_LEFT_DOWN:
        case PLATFORM_MSG_LEFT_UP:
        case PLATFORM_MSG_LEFT_DOUBLE_CLICK:
        case PLATFORM_MSG_RIGHT_DOWN:
        case PLATFORM_MSG_RIGHT_UP:
        case PLATFORM_MSG_RIGHT_DOUBLE_CLICK:
            return 1;
        default:
            return 0;
    }
}

static void mouse_set_position(intptr_t lparam)
{
    mouse_window_x = (int16_t) lparam;
    mouse_window_y = (int16_t) ((uint32_t) lparam >> 16);
}

intptr_t window_proc(void *hwnd, uint32_t message, uintptr_t wparam, intptr_t lparam)
{
    const video_player *player = video_player_ptr;
    if (player != 0 && player->done == 0) {
        return platform_message_default(hwnd, message, wparam, lparam);
    }
    if (window_current_id == WINDOW_MESSAGE_BOX && !message_box_needs(message)) {
        return platform_message_default(hwnd, message, wparam, lparam);
    }
    game_cheat_handle_key((int32_t) message, (int32_t) wparam);

    switch (message) {
        case PLATFORM_MSG_DESTROY:
            video_shutdown();
            platform_main_window = 0;
            graphics_display_shutdown(1);
            platform_message_post_quit(0);
            break;
        case PLATFORM_MSG_MOVE:
        case PLATFORM_MSG_SIZE: {
            platform_rect *client_rect = &window_screen_rect;
            if (platform_window_is_minimized(hwnd)) {
                platform_pause();
            } else {
                platform_resume();
                app_active = 1;
            }
            if (settings_data.fullscreen != 0) {
                int32_t height = platform_desktop_height();
                int32_t width = platform_desktop_width();
                client_rect->left = 0;
                client_rect->top = 0;
                client_rect->right = width;
                client_rect->bottom = height;
            } else {
                platform_window_screen_rect(hwnd, client_rect);
            }
            graphics_refresh_screen(1);
            window_redraw_request = 1;
            g_force_refresh = 1;
            break;
        }
        case PLATFORM_MSG_ACTIVATE:
            if ((wparam >> 16) != 0) {
                sound_lib_stop_stream(&sound_manager, SOUND_STREAM_MUSIC);
                sound_lib_stop_stream(&sound_manager, SOUND_STREAM_CITY);
                sound_lib_stop_stream(&sound_manager, SOUND_STREAM_SPEECH);
                app_minimized = 1;
            } else if (app_minimized != 0 && settings_data.fullscreen != 0) {
                platform_window_raise(platform_main_window);
                app_minimized = 0;
            }
            break;
        case PLATFORM_MSG_PAINT: {
            platform_window_validate(hwnd);
            graphics_refresh_screen(1);
            g_force_refresh = 1;
            window_redraw_request = 1;
            break;
        }
        case PLATFORM_MSG_CLOSE:
            game_quit_request = GAME_QUIT_CLOSING;
            break;
        case PLATFORM_MSG_ACTIVATE_APP: {
            const char *status;
            if (wparam != 0 && platform_window_is_foreground(hwnd) && !platform_window_is_minimized(hwnd)) {
                app_active = 1;
                status = "OK :Active.  ";
            } else {
                app_active = 0;
                status = str_ok_not_active;
            }
            log_info_text(status, 0);
            if (app_paused != 0 && app_active != 0) {
                if (platform_restore_surfaces() != 0) {
                    platform_resume();
                } else if (platform_window_is_foreground(hwnd)) {
                    log_info_text(str_err_reinit, 0);
                    if (graphics_begin() != 0) {
                        platform_resume();
                    }
                }
            }
            window_redraw_request = 1;
            g_force_refresh = 1;
            break;
        }
        case PLATFORM_MSG_DISPLAY_CHANGE:
            desktop_width = (int32_t) (lparam & 0xFFFF);
            desktop_height = (int32_t) ((uint32_t) lparam >> 16);
            break;
        case PLATFORM_MSG_KEY_DOWN:
            if (key_down_ends_message(hwnd, wparam)) {
                return 0;
            }
            break;
        case PLATFORM_MSG_KEY_UP:
            if (wparam == PLATFORM_KEY_CONTROL) {
                key_ctrl = 0;
            }
            break;
        case PLATFORM_MSG_CHAR:
            char_message(wparam);
            break;
        case PLATFORM_MSG_SYS_KEY_DOWN:
            switch (wparam) {
                case 'D':
                case 'd':
                    editor_wide_bounds = editor_wide_bounds == 0;
                    break;
                case 'X':
                    platform_message_post(hwnd, PLATFORM_MSG_KEY_DOWN, PLATFORM_KEY_ESCAPE, 0);
                    break;
                case 'Z':
                case 'z':
                    editor_map_refresh();
                    break;
                default:
                    break;
            }
            break;
        case PLATFORM_MSG_MOUSE_MOVE:
            mouse_set_position(lparam);
            break;
        case PLATFORM_MSG_LEFT_DOWN:
            mouse_left_clicked = 1;
            mouse_buttons |= MOUSE_BUTTON_LEFT;
            mouse_set_position(lparam);
            break;
        case PLATFORM_MSG_LEFT_UP:
            mouse_buttons &= ~MOUSE_BUTTON_LEFT;
            mouse_set_position(lparam);
            break;
        case PLATFORM_MSG_RIGHT_DOWN:
            mouse_buttons |= MOUSE_BUTTON_RIGHT;
            mouse_set_position(lparam);
            break;
        case PLATFORM_MSG_RIGHT_UP:
            mouse_buttons &= ~MOUSE_BUTTON_RIGHT;
            mouse_set_position(lparam);
            break;
        default:
            break;
    }
    return platform_message_default(hwnd, message, wparam, lparam);
}
