// The input of a frame by window id: the focus cleared, the frame's mouse position kept, then the open window's
// buttons and the right click that closes it; windows not in the dispatch get no input. The city windows of a selected
// legion or ship keep the build tools and scrolling. Also the cancel of the construction tool (a right click on the
// city, Esc, or after placing some buildings), which reports whether there was anything to cancel.

#include "window/window_input.h"
#include "building/building_type.h"
#include "building/construction_ui.h"
#include "building/ferry_landing.h"
#include "city/kingdom.h"
#include "city/message.h"
#include "editor/editor_data.h"
#include "editor/editor_event_list.h"
#include "editor/editor_map_options.h"
#include "editor/editor_menu.h"
#include "empire/empire.h"
#include "empire/empire_helpers.h"
#include "empire/empire_types.h"
#include "figure/action.h"
#include "game/game_data.h"
#include "game/game_internal.h"
#include "game/tutorial.h"
#include "graphics/arrow_buttons.h"
#include "graphics/generic_buttons.h"
#include "graphics/graphics_data.h"
#include "graphics/image_buttons.h"
#include "graphics/layout_buttons.h"
#include "input/input_data.h"
#include "platform/log.h"
#include "platform/platform_time.h"
#include "platform/platform_win.h"
#include "video/bink.h"
#include "video/smacker.h"
#include "video/video_data.h"
#include "widget/city_selection.h"
#include "widget/city_view.h"
#include "widget/menu_bar.h"
#include "widget/minimap.h"
#include "widget/sidebar_buttons.h"
#include "widget/top_menu.h"
#include "widget/widget_data.h"
#include "window/advisors.h"
#include "window/build_menu.h"
#include "window/building_info.h"
#include "window/building_info_military.h"
#include "window/building_info_ships.h"
#include "window/display_resolution_list.h"
#include "window/empire_scroll.h"
#include "window/empire_window.h"
#include "window/file_dialog_buttons.h"
#include "window/free_play.h"
#include "window/high_scores.h"
#include "window/intermezzo.h"
#include "window/main_menu.h"
#include "window/main_menu_links.h"
#include "window/map_selection.h"
#include "window/message_dialog.h"
#include "window/message_list.h"
#include "window/mission_flow.h"
#include "window/options_buttons.h"
#include "window/overlay_menu.h"
#include "window/player_selection.h"
#include "window/popup_dialog.h"
#include "window/resource_settings.h"
#include "window/text_input.h"
#include "window/top_menu_entries.h"
#include "window/victory_dialog.h"
#include "window/window_id.h"
#include <stdint.h>
#include <string.h>

const char credits_video_file[16] = "smk\\credits.smk";

const char intro_video_file[16] = "smk\\intro.smk";

const char logo_video_file[16] = "smk\\logo.smk";

generic_button editor_map_options_buttons[9] = { { 200, 36, 450, 66, editor_open_settings, button_none, 0, 0, 0, 3, 1, 0 },
    { 200, 76, 450, 106, editor_sidebar_climate, button_none, 0, 0, 0, 3, 1, 0 },
    { 200, 116, 450, 146, editor_open_event_list, button_none, 0, 0, 0, 3, 3, 0 },
    { 200, 156, 450, 186, editor_sidebar_enemy, button_none, 0, 0, 0, 3, 3, 0 },
    { 200, 196, 450, 226, editor_open_gods, button_none, 0, 0, 0, 3, 3, 0 },
    { 200, 236, 450, 266, editor_open_allowed_buildings, button_none, 0, 0, 0, 3, 3, 0 },
    { 200, 276, 450, 306, editor_open_win_criteria, button_none, 0, 0, 0, 3, 3, 0 },
    { 200, 316, 450, 346, editor_open_flood, button_none, 0, 0, 0, 3, 3, 0 },
    { 5, 156, 180, 186, editor_sidebar_soldier_images, button_none, 0, 0, 0, 3, 3, 0 } };

static void window_go(int32_t id)
{
    window_current_id = (uint8_t) id;
    window_redraw_request = 1;
}
static int32_t generic_buttons(int32_t x, int32_t y, void *buttons, int32_t count, int32_t *focus)
{
    int32_t hover = generic_buttons_get_focus(x, y, buttons, count);
    *focus = ui_layout_buttons(x, y, buttons, count, hover);
    return generic_buttons_handle_mouse(x, y, buttons, count);
}
static int32_t image_buttons(int32_t x, int32_t y, void *buttons, int32_t count, int32_t tooltip_base)
{
    image_buttons_set_tooltip(x, y, buttons, count, tooltip_base);
    return image_buttons_handle_mouse(x, y, buttons, count);
}
static void dialog_with_buttons(int32_t back, int32_t dx, int32_t dy, void *buttons, int32_t count)
{
    if (mouse_right_went_down != 0) {
        window_go(back);
        return;
    }
    generic_buttons(ui_origin_x + dx, ui_origin_y + dy, buttons, count, &focus_button_id);
}

typedef void (*on_click_fn)(void);
static int32_t city_overlay_window(on_click_fn on_click)
{
    city_view_mouse_select();
    if (g_sidebar_collapsed == 0) {
        build_sidebar_hover_update();
        if (minimap_sidebar_click() != 0) {
            return 1;
        }
    }
    city_view_scroll_direction(scroll_get_direction());
    if (mouse_right_went_up == 0) {
        city_view_mouse_select();
        if (mouse_left_went_down != 0) {
            on_click();
        }
        return 1;
    }
    return 0;
}
static void input_front_menu(void)
{
    if (window_free_play_handle_input()) {
        return;
    }
    if (main_menu_links_handle_mouse()) {
        return;
    }
    generic_buttons(ui_origin_x, ui_origin_y, main_menu_buttons, 5, &focus_button_id);
}
static void input_mission_selection(void)
{
    int32_t count = mission_selection_button_count();
    if (count > 0 && image_buttons_handle_mouse(ui_origin_x, ui_origin_y, mission_selection_tables.mission_selection_buttons, count) != 0) {
        return;
    }
    if (generic_buttons(ui_origin_x, ui_origin_y, mission_selection_tables.mission_buttons, 11, &focus_button_id) != 0) {
        return;
    }
    if (mouse_right_went_up == 0) {
        return;
    }
    if (mission_select_mode != 1 && campaign_period_reached != 0) {
        return;
    }
    window_main_menu_show();
}
static void input_video(const char *file, int32_t width, int32_t height, int32_t next_window)
{
    video_system_mode = VIDEO_SYSTEM_MODE_FULL_SCREEN;
    video_start(file, (g_screen_width - width) / 2, (g_screen_height - height) / 2, next_window);
}
static void input_high_score_record(void)
{
    highscore_record_mission();
    window_intermezzo_show(INTERMEZZO_MISSION_WON, WINDOW_MISSION_END, INTERMEZZO_DURATION_MS);
}
static void input_victory_dialog(void)
{
    int32_t count;
    if (settings_data.mission_id >= 2 || settings_data.is_custom_scenario != 0 || settings_data.campaign != 0) {
        count = 3;
    } else {
        count = 1;
    }
    generic_buttons(victory_x, victory_y, victory_dialog_buttons.buttons, count, &focus_button_id);
}
static void input_city_construction(void)
{
    city_view_mouse_select();
    construction_place_ok = 0;
    if (map_info.selected_grid == 0) {
        construction_place_ok = 0;
        return;
    }
    if (mouse_left_went_down != 0) {
        if (city_select_legion() != 0) {
            return;
        }
        if (city_select_transport() != 0) {
            return;
        }
        if (city_select_warship() != 0) {
            return;
        }
        building_construction_start();
        return;
    }
    if (mouse_left_down != 0) {
        widget_city_build_move();
        return;
    }
    if (mouse_left_went_up != 0) {
        widget_city_build_end();
        return;
    }
    if (mouse_right_went_up == 0) {
        construction_place_ok = 0;
        return;
    }
    if (construction_cancel() != 0) {
        return;
    }
    window_building_info_init();
}
static void input_city(void)
{
    int32_t handled;

    if (editor_active == 1) {
        top_menu_focus_menu_id = menu_bar_focus(menu_bar_editor, 4);
        handled = menu_bar_handle_mouse(menu_bar_editor, 4);
    } else {
        top_menu_focus_menu_id = menu_bar_focus(menu_bar_city, 4);
        handled = menu_bar_handle_mouse(menu_bar_city, 4);
    }
    if (handled != 0) {
        return;
    }
    if (editor_active == 0) {
        tutorial_update_mission_messages();
    }
    city_view_scroll_direction(scroll_get_direction());
    if (editor_active == 1) {
        int32_t count;
        if (editor_map_preview < 1) {
            image_buttons_set_tooltip(city_view_width_expanded, 30, buttons_editor.buttons, 15, 1);
        } else {
            int32_t hover = generic_buttons_get_focus(ui_origin_x + 12, ui_origin_y + 40, editor_map_options_buttons, 8);
            focus_button_id = ui_layout_buttons(ui_origin_x + 12, ui_origin_y + 40, editor_map_options_buttons, 8, hover);
        }
        build_sidebar_hover_update();
        if (minimap_sidebar_click() != 0) {
            return;
        }
        if (rotate_button_handle_mouse() != 0) {
            return;
        }
        if (editor_map_preview == 0) {
            count = 15;
        } else {
            if (mouse_right_went_down != 0) {
                editor_map_preview = 0;
                window_redraw_request = 1;
                return;
            }
            keyboard_capture_set(5);
            count = 2;
            if (generic_buttons(ui_origin_x + 12, ui_origin_y + 40, editor_map_options_buttons, 9, &focus_button_id) != 0) {
                return;
            }
            if (arrow_buttons_handle_mouse(ui_origin_x + 12, ui_origin_y + 40, editor_scenario_image_arrow_buttons, 2) != 0) {
                return;
            }
        }
        if (image_buttons_handle_mouse(city_view_width_expanded, 24, buttons_editor.buttons, count) != 0) {
            window_ui_redraw = 1;
            return;
        }
        if (editor_map_preview == 1) {
            return;
        }
    } else if (g_sidebar_collapsed == 0) {
        build_sidebar_hover_update();
        if (minimap_sidebar_click() != 0) {
            return;
        }
        if (rotate_button_handle_mouse() != 0) {
            return;
        }
        city_top_menu_hover_update();
        if (top_menu_focus_info_id != 0 && mouse_right_went_up != 0) {
            construction_right_click_help();
            return;
        }
        if (image_buttons(city_view_width_expanded, 30, buttons_expanded_extra, 2, 10) != 0) {
            window_ui_redraw = 1;
            return;
        }
        image_buttons_set_tooltip(city_view_width_expanded, 30, buttons_expanded_build, 16, 30);
        if (generic_buttons_handle_mouse(city_view_width_expanded, 30, buttons_special_orders.sidebar_problem, 1) != 0) {
            return;
        }
        if (image_buttons_handle_mouse(city_view_width_expanded, 30, buttons_expanded_build, 16) != 0) {
            window_ui_redraw = 1;
            return;
        }
        if (image_buttons(city_view_width_expanded, 30, buttons_expanded_top, 2, 50) != 0) {
            window_ui_redraw = 1;
            return;
        }
    } else if (g_sidebar_collapsed == 1) {
        if (rotate_button_handle_mouse() != 0) {
            return;
        }
        city_top_menu_hover_update();
        if (top_menu_focus_info_id != 0 && mouse_right_went_up != 0) {
            construction_right_click_help();
            return;
        }
        if (image_buttons(city_view_width_collapsed, 24, buttons_collapsed_expand.buttons, 1, 10) != 0) {
            window_ui_redraw = 1;
            return;
        }
        if (image_buttons(city_view_width_collapsed, 24, buttons_collapsed_build, 12, 30) != 0) {
            window_ui_redraw = 1;
            return;
        }
    }
    input_city_construction();
}
static void input_tooltip_hover(void)
{
    if (editor_active == 1) {
        return;
    }
    if (g_sidebar_collapsed != 0) {
        return;
    }
    generic_buttons_handle_mouse(city_view_width_expanded, 24, buttons_special_orders.sidebar_problem, 1);
}
static void input_build_menu(void)
{
    int32_t x;
    int32_t y;
    void *buttons;
    int32_t count;

    if (mouse_right_went_up != 0) {
        window_go(WINDOW_CITY);
        return;
    }
    if (build_menu_handle_input() != 0) {
        return;
    }
    if (editor_active == 1 && image_buttons_handle_mouse(city_view_width_expanded, 30, buttons_editor.buttons, editor_map_preview != 0 ? 2 : 15) != 0) {
        window_ui_redraw = 1;
        return;
    }
    if (g_sidebar_collapsed == 0) {
        if (generic_buttons_handle_mouse(city_view_width_expanded, 30, buttons_special_orders.sidebar_problem, 1) != 0) {
            return;
        }
        count = 16;
        buttons = buttons_expanded_build;
        y = 30;
        x = city_view_width_expanded;
    } else {
        if (g_sidebar_collapsed != 1) {
            return;
        }
        count = 12;
        buttons = buttons_collapsed_build;
        y = 24;
        x = city_view_width_collapsed;
    }
    if (image_buttons_handle_mouse(x, y, buttons, count) == 0) {
        return;
    }
    window_ui_redraw = 1;
}
static void input_overlay_menu(void)
{
    if (mouse_right_went_down != 0) {
        window_go(WINDOW_CITY);
        return;
    }
    if (generic_buttons(city_view_width_expanded - 170, build_menu_y_offset + 32, build_menu_buttons.overlay_menu, 10, &focus_button_id) != 0) {
        return;
    }
    overlay_menu_handle_submenu();
    if (overlay_submenu_open == 0) {
        return;
    }
    generic_buttons(city_view_width_expanded - 348, build_menu_y_offset + 32 + overlay_submenu_index * 24, build_menu_buttons.overlay_submenu, overlay_submenu_count,
        &focus_button_id_secondary);
}
static void input_message_video(void)
{
    if (dialog_show_video != 0) {
        window_message_dialog_handle_input();
        return;
    }
    if (mouse_right_went_up != 0) {
        video_stop();
    }
    if (mouse_left_went_up == 0) {
        return;
    }
    video_stop();
}
static void input_video_window(void)
{
    if (dialog_show_video != 0) {
        window_message_dialog_handle_input();
        return;
    }
    if (video_player_ptr == 0) {
        return;
    }
    if (mouse_right_went_up != 0) {
        video_bink_end(video_player_ptr);
        return;
    }
    if (mouse_left_went_up == 0) {
        return;
    }
    video_bink_end(video_player_ptr);
}
static void input_top_menu(void)
{
    int32_t handled;
    if (editor_active == 1) {
        handled = menu_dropdown_handle_mouse(menu_bar_editor, 4);
    } else {
        handled = menu_dropdown_handle_mouse(menu_bar_city, 4);
    }
    if (handled != 0) {
        return;
    }
    if (mouse_right_went_up == 0) {
        return;
    }
    menu_bar_close();
}
static void input_dialog_arrows(void *buttons, int32_t count, int32_t ax, int32_t ay, void *arrows, int32_t arrow_count)
{
    if (mouse_right_went_up != 0) {
        window_go(WINDOW_CITY);
        return;
    }
    if (generic_buttons(ui_origin_x, ui_origin_y, buttons, count, &focus_button_id) != 0) {
        return;
    }
    if (arrows) {
        arrow_buttons_handle_mouse(ui_origin_x + ax, ui_origin_y + ay, arrows, arrow_count);
    }
}
static void input_difficulty(void)
{
    if (mouse_right_went_up == 0) {
        arrow_buttons_handle_mouse(ui_origin_x + 288, ui_origin_y + 112, difficulty_arrows.arrows, 2);
        return;
    }
    window_current_id = WINDOW_CITY;
    window_redraw_request = 1;
    city_player *player = city_player_current();
    if (player->difficulty <= settings_data.difficulty) {
        return;
    }
    player->difficulty = settings_data.difficulty;
}

typedef void (*on_accept_fn)(void);
static void input_text_dialog(int32_t slot, on_accept_fn on_accept, int32_t back_on_right_up, void *image_buttons_array)
{
    keyboard_capture_set(slot);
    if (keyboard_accepted != 0) {
        keyboard_accepted = 0;
        button_parameter = 1;
        on_accept();
    }
    if (back_on_right_up) {
        if (mouse_right_went_up != 0) {
            mouse_right_went_up = 0;
            window_go(file_dialog_return_window.id);
            return;
        }
    } else if (mouse_right_went_down != 0) {
        window_go(WINDOW_CITY);
        return;
    }
    if (generic_buttons(ui_origin_x, ui_origin_y, build_menu_buttons.file_dialog_rows, 12, &focus_button_id) != 0) {
        return;
    }
    if (image_buttons(ui_origin_x, ui_origin_y, image_buttons_array, 4, 10) != 0) {
        return;
    }
    file_dialog_handle_scrollbar();
}
static void input_empire(void)
{
    empire_scroll_map(scroll_get_direction());
    if (image_buttons(20, g_screen_height - 44, empire_help_button.buttons, 1, 10) != 0) {
        return;
    }
    if (image_buttons(g_screen_width - 44, g_screen_height - 44, empire_exit_button.buttons, 1, 11) != 0) {
        return;
    }
    if (image_buttons(24, g_screen_height - 110, empire_advisors_button.buttons, 1, 12) != 0) {
        return;
    }
    if (mouse_left_went_down != 0 && mouse_x >= empire_border_x && mouse_x < display_width - (empire_border_x * 2) && mouse_y >= empire_border_y &&
        mouse_y < screen_height - empire_border_bottom - empire_border_y) {
        empire_map_object_at_mouse(&empire_selected_object, &empire_hovered_event_marker);
        window_redraw_request = 1;
    }
    if (mouse_right_went_down != 0) {
        empire_selected_object = 0;
        empire_hovered_event_marker = -1;
        window_redraw_request = 1;
    } else if (empire_selected_object != 0 && empire_map_objects[empire_selected_object - 1].type == EMPIRE_OBJECT_CITY) {
        empire_selected_city = empire_city_get_for_object(empire_selected_object - 1);
        const empire_city *city = &empire_cities[empire_selected_city];
        if (empire_city_type_is_trading(city->type) && city->is_open == 0) {
            if (generic_buttons((g_screen_width - 500) / 2, g_screen_height - 105, build_menu_buttons.empire_trade_route, 1, &focus_button_id_secondary) != 0) {
                return;
            }
        }
    }
    if (mouse_right_went_up == 0) {
        return;
    }
    button_empire_exit();
}
static void input_advisors(void)
{
    int32_t x;
    int32_t y;
    void *buttons;
    int32_t count;

    y = advisor_y + 432;
    image_buttons_set_tooltip(ui_origin_x, y, advisor_bar_buttons, ADVISOR_BAR_BUTTONS, 10);
    if (image_buttons_handle_mouse(ui_origin_x, y, advisor_bar_buttons, ADVISOR_BAR_BUTTONS) != 0) {
        return;
    }
    if (image_buttons(advisor_x, (advisor_height_blocks - 2) * 16 + advisor_y, advisor_help_buttons, 1, 25) != 0) {
        return;
    }
    if (mouse_right_went_up != 0) {
        window_go(WINDOW_CITY);
        city_player_current()->trade_prices_mode = 0;
        return;
    }
    x = advisor_x;
    y = advisor_y;
    switch (advisor_shown) {
        case ADVISOR_LABOR:
            if (generic_buttons(x, y, buttons_rows.labor_advisor, 9, &focus_button_id) != 0) {
                return;
            }
            arrow_buttons_get_focus(x, y, advisor_labor_arrow_buttons, 2, 10);
            arrow_buttons_handle_mouse(advisor_x, advisor_y, advisor_labor_arrow_buttons, 2);
            return;
        case ADVISOR_MILITARY:
            buttons = buttons_rows.military_advisor;
            count = 20;
            break;
        case ADVISOR_FLEET:
            generic_buttons(x, y, buttons_rows.fleet_advisor, 20, &focus_button_id);
            return;
        case ADVISOR_KINGDOM:
            generic_buttons(x, y, festival_square_button.kingdom_advisor, 8, &focus_button_id);
            return;
        case ADVISOR_RATINGS:
            buttons = ratings_advisor_buttons;
            count = 4;
            break;
        case ADVISOR_TRADE: {
            int32_t n = city_resources.count;
            if (n > 15) {
                n = 15;
            }
            int32_t hover = generic_buttons_get_focus(x, y, trade_advisor_buttons, n + 3);
            focus_button_id = ui_layout_buttons(x, y, trade_advisor_buttons, n + 3, hover);
            n = city_resources.count;
            if (n > 15) {
                n = 15;
            }
            if (generic_buttons_handle_mouse(advisor_x, advisor_y, trade_advisor_buttons, n + 3) != 0) {
                return;
            }
            window_advisor_trade_handle_scrollbar();
            return;
        }
        case ADVISOR_POPULATION:
            generic_buttons(x, y, buttons_rows.population_advisor, 2, &focus_button_id);
            return;
        case ADVISOR_RELIGION: {
            int32_t hover;
            if (generic_buttons(x, y, buttons_rows.religion_festival, 1, &focus_button_id) != 0) {
                return;
            }
            hover = generic_buttons_get_focus(advisor_x, advisor_y, row_nile2.religion_gods, 5);
            ui_layout_buttons(advisor_x, advisor_y, row_nile2.religion_gods, 5, hover);
            generic_buttons_handle_mouse(advisor_x, advisor_y, row_nile2.religion_gods, 5);
            return;
        }
        case ADVISOR_FINANCIAL:
            arrow_buttons_get_focus(x, y, advisor_finance_tax_arrow_buttons, 2, 10);
            arrow_buttons_handle_mouse(advisor_x, advisor_y, advisor_finance_tax_arrow_buttons, 2);
            return;
        case ADVISOR_CHIEF:
            generic_buttons(x, y, chief_advisor_rows, CHIEF_ADVISOR_ROWS, &focus_button_id);
            return;
        case ADVISOR_MONUMENTS:
            buttons = mansion_salary_button.monuments_advisor;
            count = 8;
            break;
        default:
            return;
    }
    {
        int32_t hover = generic_buttons_get_focus(x, y, buttons, count);
        focus_button_id = ui_layout_buttons(x, y, buttons, count, hover);
    }
    generic_buttons_handle_mouse(advisor_x, advisor_y, buttons, count);
}
static void input_trade_resource(void)
{
    int32_t handled;

    focus_button_id = 0;
    focus_button_id_secondary = 0;
    focus_button_id_tertiary = 0;
    if (mouse_right_went_up != 0) {
        window_go(WINDOW_ADVISORS);
        return;
    }
    if (image_buttons(advisor_x, advisor_y, resource_settings_image_buttons, 2, 10) != 0) {
        return;
    }
    int32_t order = city_player_current()->trade_status[trade_resource_index];
    if (order == TRADE_STATUS_EXPORT) {
        arrow_buttons_get_focus(advisor_x, advisor_y, resource_settings_export_arrows, 2, 10);
        handled = arrow_buttons_handle_mouse(advisor_x, advisor_y, resource_settings_export_arrows, 2);
        if (handled != 0) {
            return;
        }
    } else if (order == TRADE_STATUS_IMPORT) {
        arrow_buttons_get_focus(advisor_x, advisor_y, resource_settings_import_arrows, 2, 10);
        handled = arrow_buttons_handle_mouse(advisor_x, advisor_y, resource_settings_import_arrows, 2);
        if (handled != 0) {
            return;
        }
    }
    if (generic_buttons(advisor_x, advisor_y, resource_settings_buttons.industry.buttons, 1, &focus_button_id) != 0) {
        return;
    }
    if (generic_buttons(advisor_x, advisor_y, resource_settings_buttons.import_toggle.buttons, 1, &focus_button_id_secondary) != 0) {
        return;
    }
    if (focus_button_id_secondary == 0) {
        if (generic_buttons(advisor_x, advisor_y, resource_settings_buttons.export_toggle.buttons, 1, &focus_button_id_secondary) != 0) {
            return;
        }
        if (focus_button_id_secondary != 0) {
            focus_button_id_secondary = focus_button_id_secondary + 1;
        }
    }
    generic_buttons(advisor_x, advisor_y, resource_settings_buttons.stockpile.buttons, 1, &focus_button_id_tertiary);
}
static int32_t advisor_dialog(int32_t back, void *buttons, int32_t count)
{
    if (mouse_right_went_up != 0) {
        window_go(back);
        return 1;
    }
    return generic_buttons(advisor_x, advisor_y, buttons, count, &focus_button_id);
}
static void input_display_mode_confirm(void)
{
    if (generic_buttons(ui_origin_x + 80, ui_origin_y + 64, buttons_ship.display_mode_confirm, 1, &focus_button_id) != 0) {
        return;
    }
    if ((int32_t) platform_time_ms() <= display_mode_confirm_time + 10000) {
        return;
    }
    display_check_timeout();
}
static void input_briefing(int32_t window)
{
    if (window == WINDOW_MISSION_CHOICE) {
        if (campaign_mission_choice.choice != 0 && image_buttons_handle_mouse(ui_panel_x + 580, ui_panel_y + 410, &tutorial_briefing_tables.tutorial.briefing_accept_button, 1) != 0) {
            return;
        }
        mission_choice_click();
        return;
    }
    if (image_buttons_handle_mouse(ui_panel_x + 564, ui_panel_y + 410, &tutorial_briefing_tables.tutorial.briefing_accept_button, 1) != 0) {
        return;
    }
    if (window == WINDOW_BRIEFING) {
        if (image_buttons_handle_mouse(ui_panel_x + 110, ui_panel_y + 416, difficulty_arrow_buttons, 2) != 0) {
            return;
        }
        if (campaign_has_choices(&campaign_object.campaign) != 0 && image_buttons_handle_mouse(ui_panel_x + 10, ui_panel_y + 412, &tutorial_briefing_tables.briefing.back_button, 1) != 0) {
            return;
        }
    } else if (image_buttons_handle_mouse(ui_panel_x + 110, ui_panel_y + 416, difficulty_arrow_buttons, 2) != 0) {
        return;
    }
    mission_briefing_handle_scroll();
}
static void input_map_selection(void)
{
    if (map_selection_handle_scrollbar() != 0) {
        return;
    }
    {
        int32_t count = map_selection_image_button_count();
        void *buttons = map_selection_row_buttons();
        if (image_buttons_handle_mouse(ui_origin_x, ui_origin_y, buttons, count) != 0) {
            return;
        }
    }
    if (map_selection_has_image_buttons() != 0) {
        void *buttons = map_selection_image_buttons();
        if (image_buttons_handle_mouse(ui_origin_x, ui_origin_y, buttons, 2) != 0) {
            return;
        }
    }
    if (map_file_list.campaign == 1) {
        void *buttons = map_selection_toggle_buttons();
        int32_t hover = generic_buttons_get_focus(ui_origin_x + 360, ui_origin_y + 410, buttons, 1);
        buttons = map_selection_toggle_buttons();
        focus_button_id_secondary = ui_layout_buttons(ui_origin_x + 360, ui_origin_y + 410, buttons, 1, hover);
        buttons = map_selection_toggle_buttons();
        if (generic_buttons_handle_mouse(ui_origin_x + 350, ui_origin_y + 410, buttons, 1) != 0) {
            return;
        }
    }
    {
        int32_t count = map_selection_button_count();
        void *buttons = map_selection_buttons();
        int32_t hover = generic_buttons_get_focus(ui_origin_x, ui_origin_y, buttons, count);
        count = map_selection_button_count();
        buttons = map_selection_buttons();
        focus_button_id = ui_layout_buttons(ui_origin_x, ui_origin_y, buttons, count, hover);
        count = map_selection_button_count();
        buttons = map_selection_buttons();
        if (generic_buttons_handle_mouse(ui_origin_x, ui_origin_y, buttons, count) != 0) {
            return;
        }
    }
    if (mouse_right_went_up == 0) {
        return;
    }
    map_selection_back();
}
static void input_mission_end(void)
{
    if (city_victory_state == VICTORY_STATE_WON) {
        if (mouse_right_went_up == 0) {
            return;
        }
        mission_end_advance();
        return;
    }
    {
        int32_t count = city_defeat_options();
        void *buttons = defeat_buttons();
        int32_t hover = generic_buttons_get_focus(victory_x, victory_y - 112, buttons, count);
        count = city_defeat_options();
        buttons = defeat_buttons();
        focus_button_id = ui_layout_buttons(victory_x, victory_y - 112, buttons, count, hover);
        count = city_defeat_options();
        buttons = defeat_buttons();
        generic_buttons_handle_mouse(victory_x, victory_y - 112, buttons, count);
    }
}
static void input_player_selection(void)
{
    if (player_selection_handle_scrollbar() != 0) {
        return;
    }
    if (image_buttons_handle_mouse(ui_origin_x, ui_origin_y, player_selection_tables.player_selection_image_buttons, player_selection_image_button_count()) !=
        0) {
        return;
    }
    if (generic_buttons(ui_origin_x, ui_origin_y, player_selection_tables.player_row_buttons, 16, &focus_button_id) != 0) {
        return;
    }
    if (mouse_right_went_up == 0) {
        return;
    }
    editor_menu_exit_builder();
}
static void input_main_menu(void)
{
    if (generic_buttons(ui_origin_x, ui_origin_y, game_menu_buttons, 5, &focus_button_id) != 0) {
        return;
    }
    if (mouse_right_went_up == 0) {
        return;
    }
    main_menu_button_back();
}
static void input_name_input(void)
{
    keyboard_capture_set(1);
    if (image_buttons_handle_mouse(ui_origin_x + 464, ui_origin_y + 249, &tutorial_briefing_tables.tutorial.briefing_accept_button, 1) != 0) {
        return;
    }
    if (keyboard_accepted == 0) {
        return;
    }
    mission_button_accept();
}
static void input_tutorial_advisor(void)
{
    int32_t step;
    if (mouse_right_went_up != 0) {
        window_go(WINDOW_ADVISORS);
        return;
    }
    step = city_player_current()->advisor_comment;
    if (tutorial_flags_b[step] == 0) {
        return;
    }
    image_buttons_set_tooltip(advisor_x + 32, advisor_y + 272, tutorial_buttons[step], 1, step + 40);
    image_buttons_handle_mouse(advisor_x + 32, advisor_y + 272, tutorial_buttons[step], 1);
}
static void input_high_scores(void)
{
    if (mouse_right_went_up != 0) {
        window_go(WINDOW_FRONT_MENU);
        return;
    }
    if (high_scores_handle_scrollbar() != 0) {
        return;
    }
    image_buttons_handle_mouse(ui_origin_x, ui_origin_y, high_scores_buttons.buttons, 2);
}
void window_handle_input_dispatch(void)
{
    arrow_button_focus_id = 0;
    keyboard_field_index = 0;
    image_button_focus_id = 0;
    top_menu_focus_menu_id = 0;
    mouse_x_ui = mouse_x;
    mouse_y_ui = mouse_y;
    if (mouse_drag_state == 2) {
        mouse_drag_state = 0;
        window_redraw_request = 1;
    }

    switch (window_current_id) {
        case WINDOW_FRONT_MENU:
            input_front_menu();
            return;
        case WINDOW_MISSION_SELECTION:
            input_mission_selection();
            return;
        case WINDOW_INTERMEZZO:
            window_intermezzo_handle();
            return;
        case WINDOW_POPUP_DIALOG:
            window_popup_dialog_handle();
            return;
        case WINDOW_LOGO_VIDEO:
            input_video(logo_video_file, 500, 292, WINDOW_INTRO_VIDEO);
            return;
        case WINDOW_INTRO_VIDEO:
            input_video(intro_video_file, 500, 292, WINDOW_CREDITS_VIDEO);
            return;
        case WINDOW_CREDITS_VIDEO:
            input_video(credits_video_file, 640, 480, WINDOW_LOGO);
            return;
        case WINDOW_VICTORY_VIDEO:
            input_high_score_record();
            return;
        case WINDOW_VICTORY:
            input_victory_dialog();
            return;
        case WINDOW_CITY:
            input_city();
            return;
        case WINDOW_TOOLTIP_HOVER:
            input_tooltip_hover();
            return;
        case WINDOW_BUILD_MENU:
            input_build_menu();
            return;
        case WINDOW_OVERLAY_MENU:
            input_overlay_menu();
            return;
        case WINDOW_MESSAGE_DIALOG:
            window_message_dialog_handle_input();
            return;
        case WINDOW_MESSAGE_VIDEO:
            input_message_video();
            return;
        case WINDOW_VIDEO:
            input_video_window();
            return;
        case WINDOW_BUILDING_INFO:
            window_building_info_handle_input();
            return;
        case WINDOW_TOP_MENU:
            input_top_menu();
            return;
        case WINDOW_MESSAGE_LIST:
            window_message_list_handle_input();
            return;
        case WINDOW_SIDEBAR_SLIDE:
            city_view_scroll_direction(scroll_get_direction());
            return;
        case WINDOW_DISPLAY_OPTIONS:
            if (display_resolution_list_allowed()) {
                input_dialog_arrows(display_resolution_list_buttons(), display_resolution_list_button_count(), 0, 0, 0, 0);
                return;
            }
            input_dialog_arrows(build_menu_buttons.display_options, 5, 0, 0, 0, 0);
            return;
        case WINDOW_GAME_OPTIONS:
            input_dialog_arrows(build_menu_buttons.game_options, 14, 0, 0, 0, 0);
            return;
        case WINDOW_DIFFICULTY_OPTIONS:
            input_difficulty();
            return;
        case WINDOW_SOUND_OPTIONS:
            input_dialog_arrows(build_menu_buttons.sound_options, 6, 208, 60, sound_options_arrow_buttons, 8);
            return;
        case WINDOW_SPEED_OPTIONS:
            input_dialog_arrows(build_menu_buttons.speed_options, 2, 0x80, 40, speed_options_arrow_buttons, 4);
            return;
        case WINDOW_FILE_DIALOG:
            input_text_dialog(2, file_dialog_button, 1, file_dialog_buttons);
            return;
        case WINDOW_MAP_FILE_DIALOG:
            input_text_dialog(4, editor_map_dialog_button, 0, map_dialog_buttons);
            return;
        case WINDOW_EDITOR_START_YEAR:
            dialog_with_buttons(WINDOW_EDITOR_SETTINGS, 160, 44, build_menu_buttons.editor_start_year, 2);
            return;
        case WINDOW_EDITOR_FLOOD:
            dialog_with_buttons(WINDOW_CITY, 0, 80, buttons_figure_phrase.editor_flood, 3);
            return;
        case WINDOW_EDITOR_EVENT:
            if (mouse_right_went_down != 0) {
                event_editor_back();
                return;
            }
            generic_buttons(ui_origin_x, ui_origin_y, editor_event_type_buttons, 30, &focus_button_id);
            return;
        case WINDOW_EDITOR_ALLOWED_BUILDINGS:
            dialog_with_buttons(WINDOW_CITY, 16, 32, editor_allowed_buildings_buttons.buttons, 47);
            return;
        case WINDOW_EDITOR_GODS:
            dialog_with_buttons(WINDOW_CITY, 0, 0, buttons_special_orders.editor_gods, 20);
            return;
        case WINDOW_EDITOR_MONUMENTS:
            dialog_with_buttons(WINDOW_EDITOR_WIN_CRITERIA, 0, 0, buttons_special_orders.editor_monuments, 19);
            return;
        case WINDOW_EDITOR_EVENT_LIST:
            if (mouse_right_went_down != 0) {
                window_go(WINDOW_CITY);
                return;
            }
            if (generic_buttons(ui_origin_x, ui_origin_y, event_list_buttons[0].rows, 14, &focus_button_id) != 0) {
                return;
            }
            if (image_buttons(ui_origin_x, ui_origin_y, event_list_buttons, 2, 10) != 0) {
                return;
            }
            event_list_scrollbar();
            return;
        case WINDOW_EDITOR_WIN_CRITERIA:
            dialog_with_buttons(WINDOW_CITY, 16, 32, editor_win_criteria_buttons, 18);
            return;
        case WINDOW_EDITOR_SETTINGS:
            dialog_with_buttons(WINDOW_CITY, 12, 12, buttons_figure_phrase.editor_settings, 12);
            return;
        case WINDOW_EDITOR_TRADE_AMOUNTS:
            dialog_with_buttons(WINDOW_EDITOR_EMPIRE, 16, 32, build_menu_buttons.editor_trade_amounts, 16);
            return;
        case WINDOW_EDITOR_TRADE_PRICES:
            dialog_with_buttons(WINDOW_EDITOR_EMPIRE, 16, 0, build_menu_buttons.editor_trade_prices, 69);
            return;
        case WINDOW_EDITOR_RESOURCE_WARNING:
            if (mouse_right_went_down != 0) {
                window_go(WINDOW_EDITOR_EMPIRE);
            }
            return;
        case WINDOW_EMPIRE:
            input_empire();
            return;
        case WINDOW_EDITOR_EMPIRE:
            window_input_empire_editor();
            return;
        case WINDOW_CITY_INTRO:
            image_buttons(ui_origin_x, ui_origin_y, city_intro_buttons, 2, 10);
            return;
        case WINDOW_NUMERIC_INPUT:
            if (mouse_right_went_down != 0) {
                window_go(numeric_input_return_window.id);
                return;
            }
            generic_buttons(numeric_input_x, numeric_input_y, build_menu_buttons.numeric_input, 12, &focus_button_id);
            return;
        case WINDOW_SELECT_LIST:
        case WINDOW_EDITOR_SELECT_LIST:
            if (mouse_right_went_down != 0) {
                select_list_extra_on = 0;
                window_go(select_list_return_window.id);
                return;
            }
            generic_buttons(select_list_x, select_list_y, &select_list_buttons, select_list_count, &focus_button_id);
            return;
        case WINDOW_ADVISORS:
            input_advisors();
            return;
        case WINDOW_RESOURCE_SETTINGS:
            input_trade_resource();
            return;
        case WINDOW_LABOR_PRIORITY:
            advisor_dialog(WINDOW_ADVISORS, buttons_rows.labor_priority, advisor_labor_priority_max + 1);
            return;
        case WINDOW_FESTIVAL_DIALOG:
            if (mouse_right_went_up != 0) {
                window_go(WINDOW_ADVISORS);
                return;
            }
            if (image_buttons(advisor_x, advisor_y, festival_panel_buttons, 4, 10) != 0) {
                return;
            }
            generic_buttons(advisor_x, advisor_y, festival_square_button.festival_gods, 8, &focus_button_id);
            return;
        case WINDOW_HOLD_FESTIVAL:
            if (mouse_right_went_up != 0) {
                window_go(WINDOW_BUILDING_INFO);
                return;
            }
            if (image_buttons(30, 30, hold_festival_buttons, 4, 10) != 0) {
                return;
            }
            generic_buttons(30, 30, festival_square_button.hold_festival_gods, 8, &focus_button_id);
            return;
        case WINDOW_SET_SALARY:
            advisor_dialog(WINDOW_ADVISORS, mansion_salary_button.salaries, 12);
            return;
        case WINDOW_SALARY_DIALOG:
            advisor_dialog(WINDOW_BUILDING_INFO, mansion_salary_button.salaries, 12);
            return;
        case WINDOW_DONATE_TO_CITY:
            if (advisor_dialog(WINDOW_ADVISORS, mansion_salary_button.donations, 7) != 0) {
                return;
            }
            arrow_buttons_get_focus(advisor_x, advisor_y, donate_arrow_buttons, 2, 10);
            arrow_buttons_handle_mouse(advisor_x, advisor_y, donate_arrow_buttons, 2);
            return;
        case WINDOW_SEND_GIFT:
            advisor_dialog(WINDOW_ADVISORS, mansion_salary_button.gifts, 5);
            return;
        case WINDOW_BURIAL_PROVISIONS:
            if (advisor_dialog(WINDOW_ADVISORS, mansion_salary_button.burial_provisions, 3) != 0) {
                return;
            }
            arrow_buttons_get_focus(advisor_x + 304, advisor_y + 233, mansion_salary_button.burial_provision_arrows, 2, 10);
            arrow_buttons_handle_mouse(advisor_x + 304, advisor_y + 233, mansion_salary_button.burial_provision_arrows, 2);
            arrow_buttons_draw(advisor_x + 304, advisor_y + 233, mansion_salary_button.burial_provision_arrows, 2);
            return;
        case WINDOW_MISSION_END:
            input_mission_end();
            return;
        case WINDOW_BRIEFING:
        case WINDOW_MISSION_CHOICE:
        case WINDOW_BRIEFING_2:
            input_briefing(window_current_id);
            return;
        case WINDOW_MAP_SELECTION:
            input_map_selection();
            return;
        case WINDOW_PLAYER_SELECTION:
            input_player_selection();
            return;
        case WINDOW_MAIN_MENU:
            input_main_menu();
            return;
        case WINDOW_DISPLAY_MODE_CONFIRM:
            input_display_mode_confirm();
            return;
        case WINDOW_NAME_INPUT:
            input_name_input();
            return;
        case WINDOW_NAME_LIST:
            if (mouse_right_went_up == 0) {
                return;
            }
            keyboard_accepted = 0;
            window_go(WINDOW_NAME_INPUT);
            return;
        case WINDOW_LOGO:
            if (mouse_right_went_up == 0 && mouse_left_went_up == 0) {
                return;
            }
            window_go(WINDOW_FRONT_MENU);
            return;
        case WINDOW_END_PICTURE_1:
            if (mouse_right_went_up == 0 && mouse_left_went_down == 0) {
                return;
            }
            window_go(WINDOW_END_PICTURE_2);
            return;
        case WINDOW_END_PICTURE_2:
        case WINDOW_END_PICTURE_3:
            if (mouse_right_went_up == 0 && mouse_left_went_down == 0) {
                return;
            }
            game_quit_request = GAME_QUIT_REQUESTED;
            window_redraw_request = 1;
            return;
        case WINDOW_END_PICTURE_FINAL:
            if (mouse_right_went_up == 0 && mouse_left_went_down == 0) {
                return;
            }
            window_go(WINDOW_FRONT_MENU);
            return;
        case WINDOW_CITY_LEGION_SELECTED:
            if (city_overlay_window(legion_target_handle_input) != 0) {
                return;
            }
            break;
        case WINDOW_CITY_LEGION_ORDER:
            if (city_overlay_window(legion_fort_click) != 0) {
                return;
            }
            break;
        case WINDOW_CITY_TRANSPORT_SELECTED:
            if (city_overlay_window(transport_target_click) != 0) {
                return;
            }
            window_current_id = WINDOW_CITY;
            selected_transport_active = 0;
            city_warning_clear_all();
            window_redraw_request = 1;
            return;
        case WINDOW_CITY_WARSHIP_SELECTED:
            if (city_overlay_window(warship_target_click) != 0) {
                return;
            }
            window_current_id = WINDOW_CITY;
            selected_warship_active = 0;
            city_warning_clear_all();
            window_redraw_request = 1;
            return;
        case WINDOW_ADVISOR_COMMENT:
            input_tutorial_advisor();
            return;
        case WINDOW_ADVISOR_TEXT:
            if (mouse_right_went_up != 0) {
                window_go(WINDOW_ADVISORS);
            }
            return;
        case WINDOW_HIGH_SCORES:
            input_high_scores();
            return;
        case WINDOW_MESSAGE_BOX:
            generic_buttons(0, 0, message_box_buttons, message_box_button_count, &focus_button_id);
            return;
        case WINDOW_CHEAT_INPUT:
            window_cheat_input_handle();
            return;
        default:
            return;
    }
    window_current_id = WINDOW_CITY;
    city_warning_clear_all();
    window_redraw_request = 1;
}

int32_t construction_cancel(void)
{
    int32_t cancelled = 0;
    if (window_current_id != WINDOW_CITY) {
        cancelled = 1;
    }
    if (construction_building_type != BUILDING_NONE) {
        cancelled = 1;
    }
    window_current_id = WINDOW_CITY;
    construction_building_type = BUILDING_NONE;
    construction_in_progress = 0;
    construction_end_x = 0;
    construction_start_x = 0;
    construction_end_y = 0;
    construction_start_y = 0;
    if (construction_ferry_state == FERRY_PLACEMENT_FIRST_PLACED) {
        construction_ferry_cancel(construction_ferry_first);
    }
    if (construction_ferry_state == FERRY_PLACEMENT_FIRST_EXISTING) {
        construction_ferry_state = FERRY_PLACEMENT_NONE;
    }
    window_redraw_request = 1;
    return cancelled;
}
