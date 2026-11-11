// The draw of the open window: the background (when a redraw is requested or forced; under a tooltip too) and the
// foreground, one branch per window id with the buttons the window draws over its panels. The front menu starts the
// menu music when nothing plays.

#include "window/window_draw.h"
#include "building/building.h"
#include "building/building_type.h"
#include "city/kingdom.h"
#include "city/message.h"
#include "city/resource_type.h"
#include "city/resources.h"
#include "core/calc.h"
#include "editor/editor_allowed_buildings.h"
#include "editor/editor_data.h"
#include "editor/editor_event_draw.h"
#include "editor/editor_event_list.h"
#include "editor/editor_map_options.h"
#include "editor/editor_monuments.h"
#include "editor/editor_scenario_settings.h"
#include "editor/editor_sidebar.h"
#include "editor/editor_win_criteria.h"
#include "editor/empire_editor_draw.h"
#include "editor/empire_editor_objects.h"
#include "empire/empire.h"
#include "empire/empire_types.h"
#include "figure/action.h"
#include "game/game_data.h"
#include "game/game_internal.h"
#include "graphics/arrow_buttons.h"
#include "graphics/generic_buttons.h"
#include "graphics/graphics_data.h"
#include "graphics/image_buttons.h"
#include "graphics/label_animation_slots.h"
#include "graphics/layout_buttons.h"
#include "input/input_data.h"
#include "sound/sound_data.h"
#include "sound/sound_lib.h"
#include "video/bink.h"
#include "video/smacker.h"
#include "widget/city_building_ghost.h"
#include "widget/city_draw_view.h"
#include "widget/city_hud.h"
#include "widget/debug_info.h"
#include "widget/menu_bar.h"
#include "widget/screen_layout.h"
#include "widget/sidebar.h"
#include "widget/sidebar_buttons.h"
#include "widget/tooltip.h"
#include "widget/touch_construction.h"
#include "widget/widget_data.h"
#include "window/advisor/labor.h"
#include "window/advisor/military.h"
#include "window/advisor/population.h"
#include "window/advisor/ratings.h"
#include "window/advisor/religion.h"
#include "window/advisor/trade.h"
#include "window/advisor_comments.h"
#include "window/advisors.h"
#include "window/advisors_fleet.h"
#include "window/advisors_kingdom.h"
#include "window/advisors_monuments.h"
#include "window/build_menu.h"
#include "window/building_info.h"
#include "window/building_info_advisors.h"
#include "window/building_info_religion.h"
#include "window/building_info_storage.h"
#include "window/empire_window.h"
#include "window/file_dialog.h"
#include "window/high_scores.h"
#include "window/input_dialogs.h"
#include "window/intermezzo.h"
#include "window/loading_window.h"
#include "window/main_menu.h"
#include "window/map_selection.h"
#include "window/message_dialog.h"
#include "window/message_list.h"
#include "window/mission_flow.h"
#include "window/name_list.h"
#include "window/options_windows.h"
#include "window/overlay_menu.h"
#include "window/player_selection.h"
#include "window/popup_dialog.h"
#include "window/resource_settings.h"
#include "window/report_bug.h"
#include "window/victory_dialog.h"
#include "window/window_id.h"
#include <stdint.h>


enum window_draw_layout {
    BLOCK_SIZE = 16,
    HELP_CLOSE_BUTTONS = 2,
    BUILDING_INFO_BUTTONS_RAISE = 40,
    ORDERS_EMPTY_BUTTONS_Y = 432,
    ORDERS_LIST_TOP_BLOCKS = 7,
    ORDERS_BUTTONS_RAISE = 8,
    CITY_OVERLAY_TEXT_DX = 11,
    EDITOR_PREVIEW_ARROWS_DX = 12,
    EDITOR_PREVIEW_ARROWS_DY = 40,
    OPTIONS_ARROWS_SHOWN_AFTER = 20,
    SOUND_OPTIONS_ARROWS_DX = 208,
    SOUND_OPTIONS_ARROWS_DY = 60,
    SOUND_OPTIONS_ARROW_BUTTONS = 8,
    SPEED_OPTIONS_ARROWS_DX = 128,
    SPEED_OPTIONS_ARROWS_DY = 40,
    SPEED_OPTIONS_ARROW_BUTTONS = 4,
    EMPIRE_HELP_BUTTON_X = 20,
    EMPIRE_EXIT_BUTTON_LEFT_OF_RIGHT = 44,
    EMPIRE_BOTTOM_BUTTONS_RAISE = 44,
    EMPIRE_ADVISORS_BUTTON_X = 24,
    EMPIRE_ADVISORS_BUTTON_RAISE = 110,
    EMPIRE_EDITOR_ARROWS_X = 223,
    EMPIRE_EDITOR_ARROWS_RAISE = 114,
    EMPIRE_EDITOR_ARROW_BUTTONS = 5,
    FESTIVAL_BUTTONS = 4,
    HOLD_FESTIVAL_BUTTONS_X = 30,
    HOLD_FESTIVAL_BUTTONS_Y = 30,
    BRIEFING_ACCEPT_DX = 564,
    BRIEFING_ACCEPT_DY = 410,
    MISSION_CHOICE_ACCEPT_DX = 580,
    BRIEFING_DIFFICULTY_DX = 110,
    BRIEFING_DIFFICULTY_DY = 416,
    BRIEFING_BACK_DX = 10,
    BRIEFING_BACK_DY = 412,
    NAME_INPUT_ACCEPT_DX = 464,
    NAME_INPUT_ACCEPT_DY = 249,
    NAME_INPUT_LIST_BUTTON_DX = 352,
    NAME_INPUT_LIST_BUTTON_DY = 246,
    NAME_LIST_640X480_X = 110,
    NAME_LIST_640X480_Y = 30,
    NAME_LIST_800X600_X = 190,
    NAME_LIST_800X600_Y = 90,
    NAME_LIST_1024X768_X = 302,
    NAME_LIST_1024X768_Y = 174,
};

void window_draw_background_dispatch(int32_t force)
{
    uint32_t window_id;

    if (force == 0) {
        if (window_redraw_request == 0) {
            return;
        }
    } else {
        window_redraw_request = 1;
    }

    if (tooltip_text_id != 0) {
        window_id = (uint32_t) tooltip_window_id;
    } else {
        window_id = window_current_id;
    }

    switch (window_id) {
        case WINDOW_FRONT_MENU:
            menu_background_draw();
            break;
        case WINDOW_CITY:
        case WINDOW_TOP_MENU:
        case WINDOW_EDITOR_START_YEAR:
        case WINDOW_EDITOR_EVENT:
        case WINDOW_EDITOR_FLOOD:
        case WINDOW_EDITOR_ALLOWED_BUILDINGS:
        case WINDOW_EDITOR_WIN_CRITERIA:
        case WINDOW_CITY_LEGION_SELECTED:
        case WINDOW_CITY_TRANSPORT_SELECTED:
        case WINDOW_CITY_WARSHIP_SELECTED:
        case WINDOW_EDITOR_SETTINGS:
        case WINDOW_EDITOR_EVENT_LIST:
        case WINDOW_EDITOR_GODS:
        case WINDOW_CITY_LEGION_ORDER:
        case WINDOW_EDITOR_MONUMENTS:
            widget_sidebar_draw_background();
            break;
        case WINDOW_INTERMEZZO:
            window_intermezzo_draw();
            break;
        case WINDOW_MESSAGE_DIALOG:
            window_message_dialog_draw_background();
            break;
        case WINDOW_BUILD_MENU:
            window_build_menu_draw_background();
            break;
        case WINDOW_ADVISORS:
            window_advisors_draw_advisor();
            break;
        case WINDOW_OVERLAY_MENU:
            overlay_menu_draw_background();
            break;
        case WINDOW_BUILDING_INFO:
            widget_sidebar_draw_background();
            window_building_info_draw_background();
            break;
        case WINDOW_FILE_DIALOG:
            window_file_dialog_draw();
            break;
        case WINDOW_SELECT_LIST:
            window_select_list_draw(select_list_x, select_list_y, select_list_rows);
            break;
        case WINDOW_MAP_FILE_DIALOG:
            window_map_file_dialog_draw();
            break;
        case WINDOW_MESSAGE_LIST:
            widget_sidebar_draw_background();
            window_message_list_draw_background();
            break;
        case WINDOW_EMPIRE:
            window_empire_draw_background();
            break;
        case WINDOW_EDITOR_EMPIRE:
            empire_editor_draw_background();
            break;
        case WINDOW_MISSION_SELECTION:
            window_mission_selection_draw_background();
            break;
        case WINDOW_TOOLTIP:
            tooltip_draw_text_box();
            break;
        case WINDOW_CITY_INTRO:
            window_city_intro_draw_background();
            break;
        case WINDOW_RESOURCE_SETTINGS:
            window_advisors_draw_background();
            window_advisor_trade_draw_background();
            window_advisor_trade_draw_foreground();
            window_resource_settings_draw_foreground();
            break;
        case WINDOW_LABOR_PRIORITY:
            window_advisors_draw_background();
            window_advisor_labor_draw_background();
            window_advisor_labor_draw_foreground();
            window_labor_priority_draw_background();
            break;
        case WINDOW_FESTIVAL_DIALOG:
            window_advisors_draw_background();
            window_festival_dialog_draw_background();
            break;
        case WINDOW_SET_SALARY:
            window_advisors_draw_background();
        case WINDOW_SALARY_DIALOG:
            window_set_salary_draw();
            break;
        case WINDOW_DONATE_TO_CITY:
            window_advisors_draw_background();
            window_donate_to_city_draw_background();
            break;
        case WINDOW_SEND_GIFT:
            window_advisors_draw_background();
            window_send_gift_draw_background();
            break;
        case WINDOW_PLAYER_SELECTION:
            window_player_selection_draw_background();
            break;
        case WINDOW_MAIN_MENU:
            window_main_menu_draw_background();
            break;
        case WINDOW_MISSION_END:
            window_victory_draw();
            break;
        case WINDOW_MISSION_CHOICE:
            window_mission_choice_draw();
            break;
        case WINDOW_BRIEFING:
        case WINDOW_BRIEFING_2:
            window_briefing_draw();
            break;
        case WINDOW_NAME_INPUT:
            player_selection_draw_background();
            break;
        case WINDOW_END_PICTURE_1:
            window_end_picture_1_draw();
            break;
        case WINDOW_END_PICTURE_2:
            window_end_picture_2_draw();
            break;
        case WINDOW_END_PICTURE_3:
            window_end_picture_3_draw();
            break;
        case WINDOW_END_PICTURE_FINAL:
            window_end_picture_final_draw();
            break;
        case WINDOW_LOGO:
            logo_window_draw();
            break;
        case WINDOW_MAP_SELECTION:
            window_map_selection_draw();
            break;
        case WINDOW_DISPLAY_MODE_CONFIRM:
            window_3e_draw_background();
            break;
        case WINDOW_POPUP_DIALOG:
            window_popup_dialog_draw();
            break;
        case WINDOW_VICTORY:
            victory_dialog_draw_background();
            break;
        case WINDOW_BURIAL_PROVISIONS:
            window_advisors_draw_background();
            window_burial_provisions_draw_background();
            break;
        case WINDOW_ADVISOR_COMMENT:
            window_advisor_comment_draw();
            break;
        case WINDOW_ADVISOR_TEXT:
            window_god_description_draw();
            break;
        case WINDOW_NAME_LIST:
            window_name_list_draw();
            break;
        case WINDOW_HOLD_FESTIVAL:
            window_hold_festival_draw_background();
            break;
        case WINDOW_HIGH_SCORES:
            window_high_scores_draw_background();
            break;
        case WINDOW_LOADING:
            window_loading_draw_background();
            break;
        default:
            break;
    }
    window_redraw_request = 0;
}

static void foreground_finish(void)
{
    window_ui_redraw = 0;
    if (game_fast_forward_year != 0) {
        window_fast_forward_note_draw();
    }
}

static void foreground_draw_buttons(void *buttons, int32_t count)
{
    window_ui_redraw = 1;
    image_buttons_draw(ui_origin_x, ui_origin_y, buttons, count);
}

static void draw_building_info_buttons(void)
{
    int32_t buttons_y = (building_info_h * BLOCK_SIZE) - BUILDING_INFO_BUTTONS_RAISE + building_info_y;
    image_buttons_draw(building_info_x, buttons_y, building_info_help_close_buttons.buttons, HELP_CLOSE_BUTTONS);
    if (building_info_mode == BUILDING_INFO_MODE_BUILDING) {
        window_building_draw_advisor_buttons();
    }
}

static int32_t bazaar_goods_available(void)
{
    int32_t goods = 0;
    for (int32_t *resource = city_resources.list; resource < city_resources.list + RESOURCE_MAX; resource++) {
        if (resource_is_bazaar_good(*resource) != 0) {
            goods++;
        }
    }
    return goods;
}

static int32_t building_orders_resource_count(void)
{
    int16_t type = building_info_building()->type;
    if (type == BUILDING_STORAGE_YARD || type == BUILDING_STORAGE_YARD_UP) {
        return building_info_orders_rows();
    }
    if (type == BUILDING_BAZAAR || type == BUILDING_BAZAAR_UP) {
        return bazaar_goods_available();
    }
    return city_foods.count;
}

static void draw_building_orders_buttons(void)
{
    int32_t resource_count = building_orders_resource_count();
    if (resource_count == 0) {
        image_buttons_draw(building_info_x, ORDERS_EMPTY_BUTTONS_Y, building_info_help_close_buttons.buttons, HELP_CLOSE_BUTTONS);
        return;
    }
    int32_t list_blocks = calc_divide_round_up(resource_count * 4, 3);
    int32_t buttons_y = ((list_blocks + ORDERS_LIST_TOP_BLOCKS) * BLOCK_SIZE) - ORDERS_BUTTONS_RAISE;
    image_buttons_draw(building_info_x, buttons_y, building_info_help_close_buttons.buttons, HELP_CLOSE_BUTTONS);
}

static int32_t empire_editor_shows_arrows(void)
{
    if (empire_ui_mode == EMPIRE_EDITOR_MODE_ADD_OBJECT) {
        return empire_editor_image_kind == EMPIRE_OBJECT_ORNAMENT;
    }
    return empire_ui_mode == EMPIRE_EDITOR_MODE_EDIT_OBJECTS && empire_map_objects[g_empire_selected_object].type == EMPIRE_OBJECT_ORNAMENT;
}

static void draw_advisor_foreground(int32_t advisor)
{
    switch (advisor) {
        case ADVISOR_LABOR:
            arrow_buttons_draw(advisor_x, advisor_y, advisor_labor_arrow_buttons, 2);
            window_advisor_labor_draw_foreground();
            break;
        case ADVISOR_MILITARY:
            window_advisor_military_draw_foreground();
            break;
        case ADVISOR_FLEET:
            window_advisor_fleet_draw_foreground();
            break;
        case ADVISOR_KINGDOM:
            window_advisor_kingdom_draw_foreground();
            break;
        case ADVISOR_RATINGS:
            window_advisor_ratings_draw_foreground();
            break;
        case ADVISOR_TRADE:
            window_advisor_trade_draw_foreground();
            break;
        case ADVISOR_POPULATION:
            window_advisor_population_draw_foreground();
            break;
        case ADVISOR_RELIGION:
            window_advisor_religion_draw_foreground();
            break;
        case ADVISOR_FINANCIAL:
            arrow_buttons_draw(advisor_x, advisor_y, advisor_finance_tax_arrow_buttons, 2);
            break;
        case ADVISOR_MONUMENTS:
            window_advisor_monuments_draw_foreground();
            break;
        default:
            break;
    }
}

void window_draw_foreground_dispatch(void)
{
    int32_t count;
    void *buttons;
    int32_t focus;
    int32_t handled;
    int32_t order;
    int32_t x;
    int32_t y;

    if (window_current_id == WINDOW_FRONT_MENU) {
        if (sound_digital_ok != 0 && sound_lib_stream_is_playing(&sound_manager, SOUND_STREAM_MUSIC) == 0 && settings_data.sound_music_enabled != 0) {
            sound_lib_play_stream(&sound_manager, setup_mp3_path, SOUND_STREAM_MUSIC, 0);
        }
        front_menu_draw();
        foreground_finish();
        return;
    }

    if (window_current_id == WINDOW_MISSION_SELECTION) {
        window_mission_selection_draw();
        count = mission_selection_button_count();
        if (count == 0) {
            foreground_finish();
            return;
        }
        foreground_draw_buttons(mission_selection_tables.mission_selection_buttons, count);
        foreground_finish();
        return;
    }

    if (window_current_id == WINDOW_INTERMEZZO) {
        foreground_finish();
        return;
    }
    if (window_current_id == WINDOW_POPUP_DIALOG) {
        window_popup_dialog_draw_foreground();
        foreground_finish();
        return;
    }

    if (window_current_id == WINDOW_CITY) {
        if (editor_active == 0) {
            widget_city_draw_current_view();
            widget_sidebar_draw_buttons();
            widget_sidebar_draw_overlay_text(0, city_view_width_expanded + CITY_OVERLAY_TEXT_DX);
            widget_sidebar_draw_minimap(0);
            window_city_draw_paused_and_time_left();
            city_building_ghost_draw_construction_cost();
            debug_draw_city_info();
            debug_draw_evolve_info();
            debug_draw_building_info();
            debug_draw_figure_info();
            touch_construction_draw();
            city_message_process_queue();
            debug_frame_update();
        } else {
            if (editor_map_preview == 0) {
                widget_city_draw_current_view();
            } else {
                editor_options_panel_draw();
                arrow_buttons_draw(ui_origin_x + EDITOR_PREVIEW_ARROWS_DX, ui_origin_y + EDITOR_PREVIEW_ARROWS_DY, editor_scenario_image_arrow_buttons, 2);
            }
            widget_sidebar_draw_minimap(0);
            editor_sidebar_draw_text();
            debug_draw_building_info();
        }
        foreground_finish();
        return;
    }

    if (window_current_id == WINDOW_TOP_MENU) {
        widget_city_draw_current_view();
        menu_dropdown_draw();
        foreground_finish();
        return;
    }
    if (window_current_id == WINDOW_MESSAGE_LIST) {
        window_message_list_draw_messages();
        foreground_finish();
        return;
    }
    if (window_current_id == WINDOW_BUILD_MENU) {
        widget_city_draw_current_view();
        window_build_menu_draw_foreground();
        foreground_finish();
        return;
    }
    if (window_current_id == WINDOW_OVERLAY_MENU) {
        widget_city_draw_current_view();
        window_overlay_menu_draw_foreground();
        foreground_finish();
        return;
    }
    if (window_current_id == WINDOW_MESSAGE_DIALOG) {
        window_message_dialog_draw_foreground();
        foreground_finish();
        return;
    }
    if (window_current_id == WINDOW_MESSAGE_VIDEO) {
        video_draw();
        if (dialog_show_video != 0) {
            window_message_dialog_draw_foreground();
        }
        foreground_finish();
        return;
    }
    if (window_current_id == WINDOW_VIDEO) {
        if (video_player_ptr != 0) {
            video_bink_blit(video_player_ptr);
        }
        foreground_finish();
        return;
    }
    if (window_current_id == WINDOW_TOOLTIP_HOVER) {
        foreground_finish();
        return;
    }
    if (window_current_id == WINDOW_TOOLTIP) {
        if (tooltip_window_id == WINDOW_CITY) {
            widget_city_draw_current_view();
        }
        tooltip_draw_text_box();
        foreground_finish();
        return;
    }

    if (window_current_id == WINDOW_BUILDING_INFO) {
        window_ui_redraw = 1;
        window_building_info_draw_foreground();
        if (building_info_orders_shown == 0) {
            draw_building_info_buttons();
        } else {
            draw_building_orders_buttons();
        }
        foreground_finish();
        return;
    }

    if (window_current_id == WINDOW_SIDEBAR_SLIDE) {
        widget_sidebar_slide();
        foreground_finish();
        return;
    }
    if (window_current_id == WINDOW_DISPLAY_OPTIONS) {
        window_display_options_draw();
        foreground_finish();
        return;
    }
    if (window_current_id == WINDOW_GAME_OPTIONS) {
        window_game_options_draw();
        foreground_finish();
        return;
    }
    if (window_current_id == WINDOW_DIFFICULTY_OPTIONS) {
        window_difficulty_options_draw();
        foreground_finish();
        return;
    }
    if (window_current_id == WINDOW_SOUND_OPTIONS) {
        window_sound_options_draw();
        if (label_animation[LABEL_SLOT_SOUND_OPTIONS] > OPTIONS_ARROWS_SHOWN_AFTER) {
            x = ui_origin_x + SOUND_OPTIONS_ARROWS_DX;
            y = ui_origin_y + SOUND_OPTIONS_ARROWS_DY;
            arrow_buttons_draw(x, y, sound_options_arrow_buttons, SOUND_OPTIONS_ARROW_BUTTONS);
        }
        foreground_finish();
        return;
    }
    if (window_current_id == WINDOW_SPEED_OPTIONS) {
        window_speed_options_draw();
        if (label_animation[LABEL_SLOT_SPEED_OPTIONS] > OPTIONS_ARROWS_SHOWN_AFTER) {
            x = ui_origin_x + SPEED_OPTIONS_ARROWS_DX;
            y = ui_origin_y + SPEED_OPTIONS_ARROWS_DY;
            arrow_buttons_draw(x, y, speed_options_arrow_buttons, SPEED_OPTIONS_ARROW_BUTTONS);
        }
        foreground_finish();
        return;
    }
    if (window_current_id == WINDOW_FILE_DIALOG) {
        window_file_dialog_draw();
        foreground_finish();
        return;
    }
    if (window_current_id == WINDOW_MAP_FILE_DIALOG) {
        window_map_file_dialog_draw();
        foreground_finish();
        return;
    }
    if (window_current_id == WINDOW_EDITOR_START_YEAR) {
        window_editor_start_year_draw();
        foreground_finish();
        return;
    }
    if (window_current_id == WINDOW_EDITOR_FLOOD) {
        window_editor_flood_draw();
        foreground_finish();
        return;
    }
    if (window_current_id == WINDOW_EDITOR_EVENT) {
        window_event_editor_draw();
        foreground_finish();
        return;
    }
    if (window_current_id == WINDOW_EDITOR_ALLOWED_BUILDINGS) {
        window_editor_allowed_buildings_draw();
        foreground_finish();
        return;
    }
    if (window_current_id == WINDOW_EDITOR_EVENT_LIST) {
        window_event_list_draw();
        foreground_finish();
        return;
    }
    if (window_current_id == WINDOW_EDITOR_GODS) {
        window_editor_gods_draw();
        foreground_finish();
        return;
    }
    if (window_current_id == WINDOW_EDITOR_MONUMENTS) {
        window_editor_monuments_draw();
        foreground_finish();
        return;
    }
    if (window_current_id == WINDOW_EDITOR_WIN_CRITERIA) {
        window_editor_win_criteria_draw();
        foreground_finish();
        return;
    }
    if (window_current_id == WINDOW_EDITOR_SETTINGS) {
        window_editor_settings_draw();
        foreground_finish();
        return;
    }
    if (window_current_id == WINDOW_EDITOR_TRADE_AMOUNTS) {
        window_editor_trade_amounts_draw();
        foreground_finish();
        return;
    }
    if (window_current_id == WINDOW_EDITOR_TRADE_PRICES) {
        window_editor_trade_prices_draw();
        foreground_finish();
        return;
    }

    if (window_current_id == WINDOW_EMPIRE) {
        window_ui_redraw = 1;
        window_empire_draw_map();
        window_empire_draw_background();
        window_empire_draw_city_name();
        image_buttons_draw(EMPIRE_HELP_BUTTON_X, g_screen_height - EMPIRE_BOTTOM_BUTTONS_RAISE, empire_help_button.buttons, 1);
        image_buttons_draw(g_screen_width - EMPIRE_EXIT_BUTTON_LEFT_OF_RIGHT, g_screen_height - EMPIRE_BOTTOM_BUTTONS_RAISE, empire_exit_button.buttons, 1);
        image_buttons_draw(EMPIRE_ADVISORS_BUTTON_X, g_screen_height - EMPIRE_ADVISORS_BUTTON_RAISE, empire_advisors_button.buttons, 1);
        foreground_finish();
        return;
    }

    if (window_current_id == WINDOW_EDITOR_EMPIRE) {
        window_empire_draw_map();
        window_empire_draw_city_name();
        empire_editor_draw_mode_buttons();
        empire_editor_draw_info();
        if (empire_editor_shows_arrows()) {
            y = g_screen_height - EMPIRE_EDITOR_ARROWS_RAISE;
            arrow_buttons_draw(EMPIRE_EDITOR_ARROWS_X, y, empire_editor_arrow_buttons, EMPIRE_EDITOR_ARROW_BUTTONS);
        }
        foreground_finish();
        return;
    }

    if (window_current_id == WINDOW_CITY_INTRO) {
        foreground_draw_buttons(city_intro_buttons, 2);
        foreground_finish();
        return;
    }

    if (window_current_id == WINDOW_NUMERIC_INPUT) {
        window_numeric_input_draw(numeric_input_x, numeric_input_y);
        foreground_finish();
        return;
    }
    if (window_current_id == WINDOW_SELECT_LIST || window_current_id == WINDOW_EDITOR_SELECT_LIST) {
        window_select_list_draw(select_list_x, select_list_y, select_list_rows);
        foreground_finish();
        return;
    }

    if (window_current_id == WINDOW_ADVISORS) {
        window_ui_redraw = 1;
        window_advisors_draw_foreground();
        image_buttons_draw(advisor_x, ((advisor_height_blocks - 2) * BLOCK_SIZE) + advisor_y, advisor_help_buttons, 1);
        draw_advisor_foreground(advisor_shown);
        foreground_finish();
        return;
    }

    if (window_current_id == WINDOW_RESOURCE_SETTINGS) {
        window_ui_redraw = 1;
        window_resource_settings_draw_foreground();
        image_buttons_draw(advisor_x, advisor_y, resource_settings_image_buttons, 2);
        order = city_player_current()->trade_status[trade_resource_index];
        if (order == TRADE_STATUS_EXPORT) {
            arrow_buttons_draw(advisor_x, advisor_y, resource_settings_export_arrows, 2);
        }
        if (order == TRADE_STATUS_IMPORT) {
            arrow_buttons_draw(advisor_x, advisor_y, resource_settings_import_arrows, 2);
        }
        foreground_finish();
        return;
    }

    if (window_current_id == WINDOW_LABOR_PRIORITY) {
        window_labor_priority_draw_foreground();
        foreground_finish();
        return;
    }
    if (window_current_id == WINDOW_FESTIVAL_DIALOG) {
        window_ui_redraw = 1;
        window_festival_dialog_draw_foreground();
        image_buttons_draw(advisor_x, advisor_y, festival_panel_buttons, FESTIVAL_BUTTONS);
        foreground_finish();
        return;
    }
    if (window_current_id == WINDOW_HOLD_FESTIVAL) {
        window_ui_redraw = 1;
        window_hold_festival_draw_foreground();
        image_buttons_draw(HOLD_FESTIVAL_BUTTONS_X, HOLD_FESTIVAL_BUTTONS_Y, hold_festival_buttons, FESTIVAL_BUTTONS);
        foreground_finish();
        return;
    }
    if (window_current_id == WINDOW_SET_SALARY || window_current_id == WINDOW_SALARY_DIALOG) {
        window_ui_redraw = 1;
        window_set_salary_draw();
        foreground_finish();
        return;
    }
    if (window_current_id == WINDOW_DONATE_TO_CITY) {
        window_ui_redraw = 1;
        window_donate_to_city_draw_foreground();
        arrow_buttons_draw(advisor_x, advisor_y, donate_arrow_buttons, 2);
        foreground_finish();
        return;
    }
    if (window_current_id == WINDOW_SEND_GIFT) {
        window_ui_redraw = 1;
        window_send_gift_draw_foreground();
        foreground_finish();
        return;
    }
    if (window_current_id == WINDOW_BURIAL_PROVISIONS) {
        window_ui_redraw = 1;
        window_burial_provisions_draw_foreground();
        foreground_finish();
        return;
    }
    if (window_current_id == WINDOW_VICTORY) {
        victory_dialog_draw_foreground();
        foreground_finish();
        return;
    }
    if (window_current_id == WINDOW_MISSION_END) {
        if (city_victory_state != VICTORY_STATE_WON) {
            victory_dialog_draw_foreground();
        }
        foreground_finish();
        return;
    }
    if (window_current_id == WINDOW_BRIEFING) {
        window_ui_redraw = 1;
        mission_briefing_draw_scroll();
        image_buttons_draw(ui_panel_x + BRIEFING_ACCEPT_DX, ui_panel_y + BRIEFING_ACCEPT_DY, &tutorial_briefing_tables.tutorial.briefing_accept_button, 1);
        image_buttons_draw(ui_panel_x + BRIEFING_DIFFICULTY_DX, ui_panel_y + BRIEFING_DIFFICULTY_DY, difficulty_arrow_buttons, 2);
        if (campaign_has_choices(&campaign_object.campaign) != 0) {
            image_buttons_draw(ui_panel_x + BRIEFING_BACK_DX, ui_panel_y + BRIEFING_BACK_DY, &tutorial_briefing_tables.briefing.back_button, 1);
        }
        foreground_finish();
        return;
    }
    if (window_current_id == WINDOW_MISSION_CHOICE) {
        window_ui_redraw = 1;
        if (campaign_mission_choice.choice != 0) {
            image_buttons_draw(ui_panel_x + MISSION_CHOICE_ACCEPT_DX, ui_panel_y + BRIEFING_ACCEPT_DY, &tutorial_briefing_tables.tutorial.briefing_accept_button, 1);
        }
        mission_choice_draw_markers();
        foreground_finish();
        return;
    }
    if (window_current_id == WINDOW_BRIEFING_2) {
        window_ui_redraw = 1;
        mission_briefing_draw_scroll();
        image_buttons_draw(ui_panel_x + BRIEFING_ACCEPT_DX, ui_panel_y + BRIEFING_ACCEPT_DY, &tutorial_briefing_tables.tutorial.briefing_accept_button, 1);
        image_buttons_draw(ui_panel_x + BRIEFING_DIFFICULTY_DX, ui_panel_y + BRIEFING_DIFFICULTY_DY, difficulty_arrow_buttons, 2);
        foreground_finish();
        return;
    }
    if (window_current_id == WINDOW_MAP_SELECTION) {
        window_ui_redraw = 1;
        count = map_selection_image_button_count();
        buttons = map_selection_row_buttons();
        image_buttons_draw(ui_origin_x, ui_origin_y, buttons, count);
        if (map_selection_has_image_buttons() != 0) {
            buttons = map_selection_image_buttons();
            image_buttons_draw(ui_origin_x, ui_origin_y, buttons, 2);
        }
        map_selection_draw_list();
        foreground_finish();
        return;
    }
    if (window_current_id == WINDOW_PLAYER_SELECTION) {
        window_ui_redraw = 1;
        count = player_selection_image_button_count();
        image_buttons_draw(ui_origin_x, ui_origin_y, player_selection_tables.player_selection_image_buttons, count);
        window_player_selection_draw_foreground();
        foreground_finish();
        return;
    }
    if (window_current_id == WINDOW_MAIN_MENU) {
        window_main_menu_draw_foreground();
        foreground_finish();
        return;
    }
    if (window_current_id == WINDOW_DISPLAY_MODE_CONFIRM) {
        window_3e_draw_foreground();
        foreground_finish();
        return;
    }

    if (window_current_id == WINDOW_NAME_INPUT) {
        window_ui_redraw = 1;
        window_name_input_draw_foreground();
        image_buttons_draw(ui_origin_x + NAME_INPUT_ACCEPT_DX, ui_origin_y + NAME_INPUT_ACCEPT_DY, &tutorial_briefing_tables.tutorial.briefing_accept_button, 1);
        x = ui_origin_x + NAME_INPUT_LIST_BUTTON_DX;
        y = ui_origin_y + NAME_INPUT_LIST_BUTTON_DY;
        focus = generic_buttons_get_focus(x, y, family_name_list_button.buttons, 1);
        focus_button_id = ui_layout_buttons(x, y, family_name_list_button.buttons, 1, focus);
        handled = generic_buttons_handle_mouse(x, y, family_name_list_button.buttons, 1);
        if (handled != 0) {
            return;
        }
        if (mouse_right_went_up != 0) {
            mouse_right_went_up = 0;
            window_player_selection_show();
            return;
        }
        foreground_finish();
        return;
    }

    if (window_current_id == WINDOW_NAME_LIST) {
        if (!screen_layout_is_standard_size()) {
            x = ui_origin_x + NAME_LIST_640X480_X;
            y = ui_origin_y + NAME_LIST_640X480_Y;
        } else if (settings_data.display_mode == DISPLAY_MODE_640X480) {
            x = NAME_LIST_640X480_X;
            y = NAME_LIST_640X480_Y;
        } else if (settings_data.display_mode == DISPLAY_MODE_800X600) {
            x = NAME_LIST_800X600_X;
            y = NAME_LIST_800X600_Y;
        } else {
            x = NAME_LIST_1024X768_X;
            y = NAME_LIST_1024X768_Y;
        }
        focus = generic_buttons_get_focus(x, y, name_list_buttons.buttons, NAME_LIST_BUTTONS);
        focus_button_id = ui_layout_buttons(x, y, name_list_buttons.buttons, NAME_LIST_BUTTONS, focus);
        handled = generic_buttons_handle_mouse(x, y, name_list_buttons.buttons, NAME_LIST_BUTTONS);
        if (handled != 0) {
            return;
        }
        window_name_list_draw();
        foreground_finish();
        return;
    }

    if (window_current_id == WINDOW_CITY_LEGION_SELECTED || window_current_id == WINDOW_CITY_LEGION_ORDER ||
        window_current_id == WINDOW_CITY_TRANSPORT_SELECTED || window_current_id == WINDOW_CITY_WARSHIP_SELECTED) {
        widget_city_draw_current_view();
        widget_sidebar_draw_minimap(0);
        window_city_draw_paused_and_time_left();
        foreground_finish();
        return;
    }
    if (window_current_id == WINDOW_HIGH_SCORES) {
        window_high_scores_draw_background();
        image_buttons_draw(ui_origin_x, ui_origin_y, high_scores_buttons.buttons, 2);
        foreground_finish();
        return;
    }
    if (window_current_id == WINDOW_CHEAT_INPUT) {
        window_cheat_input_draw();
        foreground_finish();
        return;
    }
    if (window_current_id == WINDOW_REPORT_BUG) {
        report_bug_draw();
        foreground_finish();
        return;
    }

    foreground_finish();
}
