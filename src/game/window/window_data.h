// The tables of window_data.c, the initial data of the windows: their buttons, menus, layouts and texts. The empire
// editor's button tables are fields of build_menu_buttons.

#ifndef WINDOW_WINDOW_DATA_H
#define WINDOW_WINDOW_DATA_H

#include "map/construction_terrain.h"
#include "window/input_dialogs.h"
#include "building/monument.h"
#include "building/monument_object_types.h"
#include "building/monument_progress_types.h"
#include "city/resource_list_types.h"
#include "editor/editor_new_scenario.h"
#include "empire/empire_types.h"
#include "figure/formation_types.h"
#include "game/mission_records_types.h"
#include "game/player_record.h"
#include "game/settings_types.h"
#include "game/tutorial.h"
#include "graphics/image_collection_types.h"
#include "input/mouse.h"
#include "scenario/campaign.h"
#include "scenario/scenario_event_types.h"
#include "scenario/scenario_types.h"
#include "video/bink.h"
#include "window/window_button_types.h"
#include "window/window_state_types.h"
#include <stdint.h>

extern image_button buttons_expanded_extra[2];

extern single_image_button buttons_collapsed_expand;

extern image_button buttons_expanded_build[16];

extern image_button buttons_collapsed_build[12];

extern sidebar_editor_button_table buttons_editor;

extern image_button buttons_expanded_top[2];

extern image_button rotate_button_x[1];

extern image_button file_dialog_buttons[4];

extern image_button map_dialog_buttons[4];

extern image_button_triple building_info_help_close_buttons;

extern image_button advisor_help_buttons[2];

extern image_button festival_panel_buttons[4];

extern image_button hold_festival_buttons[4];

extern single_image_button empire_help_button;

extern single_image_button empire_exit_button;

extern single_image_button empire_advisors_button;

extern image_button city_intro_buttons[2];

extern single_image_button message_back;

extern single_image_button message_close;

extern message_scroll_button message_scroll_up;

extern message_scroll_button message_scroll_down;

extern single_image_button message_help;

extern single_image_button message_go_to_problem;

extern single_image_button message_list_help;

extern single_image_button message_labor_advisor;

extern single_image_button message_military_advisor[2];

extern single_image_button message_imperial_advisor[2];

extern single_image_button message_trade_advisor;

extern single_image_button message_population_advisor;

extern single_image_button message_health_advisor[3];

extern single_image_button message_religion_advisor[4];

extern single_image_button message_list_close;

extern single_image_button message_list_scroll_up;

extern single_image_button message_list_scroll_down;

extern image_button popup_buttons[2];

extern image_button difficulty_arrow_buttons[2];

extern storage_order_button_table order_buttons;

typedef struct {
    image_button mission_selection_buttons[2];
    generic_button mission_buttons[11];
} mission_selection_table_group;
extern mission_selection_table_group mission_selection_tables;

typedef struct {
    image_button player_selection_image_buttons[2];
    generic_button player_row_buttons[16];
} player_selection_table_group;
extern player_selection_table_group player_selection_tables;

extern char setup_mp3_path[24];

extern const char save_file_pattern[8];

extern const char file_mode_read[4];

extern const char path_separator[4];

extern const char save_file_extension[4];


extern uint8_t cheats_disabled;

extern int32_t legion_cycle_last;

extern const char str_err_reinit[36];

extern const char str_ok_not_active[20];

extern char mission_briefing_speech_files[2400];

extern char mission_briefing_speech_files_classic[2400];

extern char mission_victory_speech_files[2400];

extern char mission_victory_speech_files_classic[2404];

extern generic_button main_menu_buttons[5];

extern generic_button game_menu_buttons[5];

extern uint32_t main_menu_button_labels[5];

extern const char str_replay_save[20];

extern const char str_video_win_big[32];

extern const char str_video_win_small[32];



extern char postfix_none[4];

extern menu_button_tables build_menu_buttons;

extern name_list_button_table name_list_buttons;

extern single_generic_button buttons_delete_read;

extern message_list_button_tables buttons_rows;

extern festival_button_tables festival_square_button;

extern mansion_button_tables mansion_salary_button;

extern generic_button chief_advisor_rows[CHIEF_ADVISOR_ROWS];

extern chief_nile_and_god_buttons row_nile2;

extern single_generic_button buttons_house_ventures;

extern single_generic_button buttons_granary_orders;

extern single_generic_button buttons_bazaar_orders;

extern single_generic_button buttons_storage_orders;

extern family_name_list_button_table family_name_list_button;

extern arrow_button storage_orders_scroll_down[1];

extern arrow_button storage_orders_scroll_up[1];

extern arrow_button trade_advisor_scroll_down[1];

extern arrow_button trade_advisor_scroll_up[1];

extern single_generic_button buttons_back_orders;

extern single_generic_button buttons_storage_back;

extern figure_list_button_table buttons_figure_list;

extern figure_phrase_button_tables buttons_figure_phrase;

extern generic_button select_list_extra_buttons[2];

extern editor_allowed_building_button_table editor_allowed_buildings_buttons;

extern generic_button editor_win_criteria_buttons[18];

extern victory_dialog_button_table victory_dialog_buttons;

extern defeat_dialog_button_tables defeat_dialog_buttons;

extern order_button_row_table buttons_legion;

extern single_generic_button buttons_legion_return;

extern order_button_row_table buttons_boat;

extern ship_order_button_tables buttons_ship;

extern special_order_button_tables buttons_special_orders;

extern arrow_button empire_editor_arrow_buttons[7];

extern arrow_button advisor_labor_arrow_buttons[2];

extern arrow_button advisor_finance_tax_arrow_buttons[2];

extern arrow_button donate_arrow_buttons[2];

extern arrow_button speed_options_arrow_buttons[4];

extern arrow_button sound_options_arrow_buttons[8];

extern arrow_button editor_scenario_image_arrow_buttons[2];

extern difficulty_arrow_table difficulty_arrows;

extern menu_bar_entry menu_bar_city[MENU_BAR_ENTRIES];

extern menu_bar_entry menu_bar_editor[MENU_BAR_ENTRIES];

enum build_menu_table_index {
    BUILD_MENU_TYPE_FIRST = 1,
    BUILD_MENU_SUB_POSITIONS_FIRST = 1185,
    OVERLAY_MENU_OVERLAYS_FIRST = 1247,
    OVERLAY_SUBMENU_OVERLAYS_FIRST = 1257,
    BUILD_MENU_TABLE_ENTRIES = 1607,
};

extern int32_t build_menu_types[BUILD_MENU_TABLE_ENTRIES];

extern int32_t empire_map_width;

extern int32_t empire_map_height;

extern int32_t empire_border_x;

extern int32_t empire_border_y;

extern int32_t empire_border_bottom;

extern int32_t empire_hovered_event_marker;

extern high_score_button_table high_scores_buttons;

extern const char str_highscore_file[24];

extern phrase_button_table phrase_buttons;

extern char money_format[4];

extern char pct_postfix[4];

extern int32_t route_point_selected;

extern int32_t boat_order_image_offsets[5];

extern void *tutorial_buttons[15];

extern const char player_file_pattern[12];

extern const int32_t large_label_frames[30];

extern generic_button message_box_buttons[3];

extern char money_postfix[4];

extern char wages_rome_format[4];

extern char unemployment_format[4];

extern char db_format[4];

extern generic_button trade_advisor_buttons[18];

extern resource_settings_button_tables resource_settings_buttons;

extern arrow_button resource_settings_export_arrows[2];

extern arrow_button resource_settings_import_arrows[2];

extern image_button resource_settings_image_buttons[2];


extern int32_t overlay_menu_hover_time;

extern int32_t saved_scroll_speed;

extern file_list file_list_saves;

extern int32_t scenario_current_request;

extern int32_t display_mode_confirm_time;

extern int32_t advisor_labor_priority_max;

extern int32_t labor_focus;

extern int32_t trade_resource_index;

extern int32_t empire_goods_list_slot;

extern int32_t construction_place_ok;

extern int32_t needs_groundwater;

extern int32_t needs_road;

extern int32_t needs_wall;

extern int32_t needs_marsh;

extern int32_t needs_water_near;

extern int32_t needs_trees;

extern int32_t needs_farm_land;

extern int32_t construction_start_grid_offset;

extern int32_t construction_end_y;

extern int32_t construction_start_y;

extern int32_t construction_end_x;

extern int32_t construction_start_x;

extern int32_t construction_in_progress;

extern int32_t construction_building_type;

extern int32_t file_dialog_scroll_layout;

extern int32_t file_dialog_selected;

extern int32_t file_dialog_scroll;

extern int32_t file_dialog_scroll_total;

extern int32_t name_list_choice;

extern int32_t debug_event_kind;

extern int32_t debug_event_level;

extern int32_t debug_event_text;

extern int32_t cheat_allow_all;

extern int32_t editor_empire_file_number;

extern int32_t editor_empire_map_number;

extern int32_t editor_new_map_size;

extern int32_t editor_herd_point_number;

extern int32_t editor_landing_point_number;

extern int32_t editor_fishing_point_number;

extern int32_t editor_invasion_point_number;

extern int32_t editor_tool_variant;

extern int32_t editor_tool_kind;

extern int32_t editor_brush_size;

extern int32_t editor_map_preview;

extern int32_t editor_active;

extern int32_t city_population_graph_order;

extern int32_t advisor_shown;

extern terrain_test construction_near_terrain_test;

extern input_dialog_callback numeric_input_on_ok;

extern input_dialog_callback select_list_on_choose;

extern int32_t select_list_extra_text;

extern int32_t select_list_extra_on;

extern uint8_t construction_hover_locked;

extern uint32_t game_options_flags;

extern uint8_t overlay_show_all;

extern video_player *video_player_ptr;

extern int32_t build_menu_item_counts[12];

extern int32_t mission_select_mode;

extern int32_t ui_panel_y;

extern int32_t ui_panel_x;

extern int32_t intermezzo_next_window;

extern int32_t intermezzo_end_ms;

extern mission_records_state record_savings;

extern player_data campaign_object;

extern campaign_mission_choice_slot campaign_mission_choice;

extern int32_t mission_briefing_started;

extern int32_t mission_selected;

extern int32_t campaign_period_reached;

extern uint8_t game_is_replay;

extern const char empty_text[4];

extern int32_t cheat_focus;

extern map_list map_file_list;

extern int32_t arrow_button_focus_id;

extern int32_t image_button_focus_id;

extern int32_t focus_button_id_tertiary;

extern int32_t focus_button_id_secondary;

extern int32_t focus_button_id;

extern int32_t overlay_submenu_index;

extern int32_t overlay_submenu_count;

extern int32_t overlay_submenu_open;


extern int32_t build_menu_y_offset;

extern int32_t build_menu_group;

extern int32_t build_menu_item_count;

extern popup_state popup;

extern int32_t menu_enabled[1140];

extern int32_t empire_scroll_y;

extern int32_t empire_scroll_x;

extern int32_t empire_ui_mode;

extern scenario_record scenario_data;

extern int32_t empire_selected_object;

extern int32_t empire_selected_city;

extern scenario_events_t scenario_events;

extern int32_t formation_selected;

extern formation_table formation_array;

extern int32_t selected_transport_id;

extern int32_t selected_warship_id;

extern int32_t selected_transport_active;

extern int32_t selected_warship_active;

extern char high_score_previous_name[32];

extern int32_t hs_result;

extern mission_record_table highscores;

extern int32_t hs_scroll;

extern int32_t hs_count;

extern int32_t tooltip_window_id;

extern int32_t dialog_history_count;

extern int32_t dialog_history_scroll[200];

extern int32_t dialog_history_id[200];

extern int32_t dialog_text_id;

extern int32_t dialog_scroll_drag_y;

extern int32_t dialog_scroll_dragging;

extern int32_t rich_scroll_pos;

extern int32_t dialog_scroll_max;

extern int32_t rich_line_count;

extern int32_t rich_viewport_lines;

extern int32_t dialog_background;

extern int32_t dialog_text_height_blocks;

extern int32_t dialog_text_width_blocks;

extern int32_t dialog_y_text;

extern int32_t dialog_x_text;

extern int32_t dialog_x;

extern int32_t rich_text_link_font;

extern int32_t rich_text_normal_font;

extern int32_t dialog_param2;

extern int32_t dialog_param1;

extern int32_t dialog_month;

extern int32_t dialog_year;

extern int32_t dialog_prev_window;

extern int32_t dialog_show_video;

extern uint8_t menu_can_start;

extern uint8_t menu_period_open;

extern uint8_t menu_has_saves;

extern uint8_t menu_can_continue;

extern int32_t button_parameter2;

extern int32_t button_parameter;


extern uint8_t file_dialog_error_frames;

extern window_return_slot file_dialog_return_window;

extern uint8_t file_dialog_type;

extern uint8_t popup_answer;

extern uint8_t game_quit_request;

extern int32_t image_view_y;

extern int32_t image_view_x;

extern int32_t image_view_height;

extern int32_t image_view_width;


extern int32_t g_word_chars;

extern int32_t g_text_width;

extern uint8_t capture_insert_mode;

extern int32_t keyboard_field_index;

extern int32_t keyboard_accepted;

extern char scenario_map_name[65];

extern char file_dialog_last_name[68];

extern char file_dialog_name[6568];

extern char file_extension[660];

extern int32_t display_width;


extern int32_t ui_origin_y;

extern int32_t ui_origin_x;

extern uint8_t window_ui_redraw;

extern uint8_t window_redraw_request;


extern uint8_t window_current_id;

extern int32_t window_input_delay_frames;

extern settings_file settings_data;

extern cursor_handle_table cursor_handles;

extern void *cursor_current;

extern int32_t app_paused;

extern int32_t app_active;

extern void *platform_main_window;

extern int32_t display_mode;


extern int32_t key_scroll_right;

extern int32_t key_scroll_left;

extern int32_t key_scroll_down;

extern int32_t key_scroll_up;

extern uint32_t city_scroll_last_time;


extern int32_t inner_width;

extern int32_t inner_y;

extern int32_t inner_x;

extern int32_t thumb_y;

extern int32_t thumb_dragged;


extern uint8_t mission_lost_can_replay;

extern uint8_t city_lost_by_population;

extern int32_t month_axis_end_year;

extern int32_t month_axis_end_month;

extern int32_t month_axis_start_year;

extern int32_t month_axis_start_month;

extern int32_t city_houses_of_win_level;

extern int32_t bi_progress_tomb[8];

extern mausoleum_info_progress bi_progress_mausoleum;

extern scripted_monument_info_progress bi_progress_lighthouse;

extern scripted_monument_info_progress bi_progress_caesareum;

extern abu_simbel_info_progress bi_progress_abu_simbel;

extern int32_t bi_progress_sun_temple[14];

extern pyramid_progress bi_progress_pyramid;

extern int32_t bi_storage_state;

extern int32_t building_info_orders_shown;

extern int32_t building_info_warship_id;

extern int32_t building_info_transport_id;

extern int32_t building_info_formation_id;

extern int32_t info_has_academy;

extern int32_t house_worst_desirability_building;

extern int32_t info_figure_index;

extern int32_t building_info_advisor_buttons[15];

extern int32_t info_figure_ids[7];

extern int32_t info_figure_count;

extern int32_t building_info_details_shown;

extern int32_t info_terrain_kind;

extern int32_t building_info_mode;

extern int32_t info_help_id;

extern building_info_details bi_road_access_block;

extern int32_t building_info_building_id;

extern int32_t building_info_h;

extern int32_t info_w_blocks;

extern int32_t building_info_y;

extern int32_t building_info_x;

extern int32_t info_first_draw;

extern file_list player_file_list;


extern int32_t plist_scroll;

extern int32_t plist_selected;

extern uint8_t message_box_answered;

extern int32_t message_box_button;

extern int32_t message_box_button_count;

extern empire_event_marker_state empire_markers;


extern int32_t victory_y;

extern int32_t victory_x;

extern city_resource_list city_foods;

extern city_resource_list city_resources;

extern int32_t advisor_height_blocks;


extern int32_t advisor_y;

extern int32_t advisor_x;

extern int32_t label_animation[16];

extern image_collection_object image_collection;

#endif
