// The built-in data of the scenario editor: its tools, image contexts, menus and the event editor's buttons.
// editor/editor_data.h declares them.

#include "editor/editor_data.h"
#include "editor/editor_event_buttons.h"
#include "editor/editor_event_list.h"
#include "editor/editor_map_options.h"
#include "editor/editor_menu.h"
#include "figure/action.h"
#include "window/advisors.h"
#include "window/file_dialog_buttons.h"
#include "window/mission_flow.h"
#include "window/options_buttons.h"
#include "window/top_menu_entries.h"

#include <assert.h>

const char map_file_pattern[12] = "maps\\*.map";

menu_item g_menu_items[MENU_ITEMS] = { { 0, 1, menu_file_new_game, 0, 0 }, { 20, 2, mission_replay, 0, 0 },
    { 40, 3, file_menu_load_game, 0, 0 }, { 60, 4, file_menu_save_game, 0, 0 },
    { 80, 6, file_menu_delete_game, 0, 0 }, { 100, 5, file_menu_exit, 0, 0 },
    { 0, 1, window_display_options_show, 0, 0 }, { 20, 2, window_sound_options_show, 0, 0 },
    { 40, 3, window_speed_options_show, 0, 0 }, { 60, 11, window_game_options_show, 0, 0 },
    { 80, 7, options_toggle_city_names, 0, 0 }, { 100, 6, window_difficulty_options_show, 0, 0 },
    { 120, 9, options_toggle_autosave, 0, 0 }, { 140, 15, options_toggle_pyramid_speedup, 0, 0 },
    { 0, 1, help_menu_show_help, 0, 0 }, { 20, 2, help_menu_toggle_tooltips, 0, 0 },
    { 40, 5, help_menu_toggle_warnings, 0, 0 }, { 60, 7, help_menu_show_about, 0, 0 },
    { 0, 1, help_menu_show_help, 0, 0 }, { 20, 2, help_menu_toggle_tooltips, 0, 0 },
    { 40, 8, help_menu_editor_guide, 0, 0 }, { 0, 1, window_advisors_show_checked, 0, 1 },
    { 20, 2, window_advisors_show_checked, 0, 2 }, { 40, 3, window_advisors_show_checked, 0, 4 },
    { 60, 4, window_advisors_show_checked, 0, 5 }, { 80, 5, window_advisors_show_checked, 0, 6 },
    { 100, 6, window_advisors_show_checked, 0, 7 }, { 120, 7, window_advisors_show_checked, 0, 8 },
    { 140, 8, window_advisors_show_checked, 0, 9 }, { 160, 9, window_advisors_show_checked, 0, 10 },
    { 180, 10, window_advisors_show_checked, 0, 11 }, { 200, 11, window_advisors_show_checked, 0, 12 },
    { 220, 12, window_advisors_show_checked, 0, 13 }, { 240, 13, window_advisors_show_checked, 0, 14 },
    { 0, 1, editor_menu_new_map, 0, 0 }, { 20, 2, editor_menu_open_map, 0, 0 },
    { 40, 3, editor_menu_save_map, 0, 0 }, { 60, 4, editor_menu_exit_builder, 0, 0 },
    { 0, 1, editor_clear_predator_points, 0, 0 }, { 20, 2, editor_clear_fishing_points, 0, 0 },
    { 40, 3, editor_clear_invasion_points, 0, 0 }, { 60, 4, editor_clear_invasion_landings, 0, 0 },
    { 80, 5, editor_clear_prey_points, 0, 0 }, { 100, 10, editor_map_refresh, 0, 0 } };

int32_t empire_editor_route_dragging = 1;

int32_t route_current = 1;

event_list_buttons_layout event_list_buttons[1] = {
    { .arrows = { { 508, 46, 34, 34, 4, 365, 8, 0, event_list_arrow, button_none, 1, 0, 0, 0, 0, 0 },
          { 508, 300, 34, 34, 4, 365, 12, 0, event_list_arrow, button_none, 1, 0, 0, 0, 1, 0 } },
        .rows = { { 32, 46, 500, 66, event_list_row, button_none, 0, 0, 0, 3, 0, 0 },
            { 32, 70, 500, 90, event_list_row, button_none, 0, 0, 0, 3, 1, 0 },
            { 32, 94, 500, 114, event_list_row, button_none, 0, 0, 0, 3, 2, 0 },
            { 32, 118, 500, 138, event_list_row, button_none, 0, 0, 0, 3, 3, 0 },
            { 32, 142, 500, 162, event_list_row, button_none, 0, 0, 0, 3, 4, 0 },
            { 32, 166, 500, 186, event_list_row, button_none, 0, 0, 0, 3, 5, 0 },
            { 32, 190, 500, 210, event_list_row, button_none, 0, 0, 0, 3, 6, 0 },
            { 32, 214, 500, 234, event_list_row, button_none, 0, 0, 0, 3, 7, 0 },
            { 32, 238, 500, 258, event_list_row, button_none, 0, 0, 0, 3, 8, 0 },
            { 32, 262, 500, 282, event_list_row, button_none, 0, 0, 0, 3, 9, 0 },
            { 32, 286, 500, 306, event_list_row, button_none, 0, 0, 0, 3, 10, 0 },
            { 32, 310, 500, 330, event_list_row, button_none, 0, 0, 0, 3, 11, 0 },
            { 100, 358, 250, 378, event_list_new, button_none, 0, 0, 0, 1, 1, 0 },
            { 260, 358, 410, 378, event_list_delete, button_none, 0, 0, 0, 1, 1, 0 } } }
};

generic_button editor_event_type_buttons[EDITOR_EVENT_TYPE_BUTTONS] = {
    { 258, 20, 428, 45, event_type_button, button_none, 0, 0, 0, 1, 0, 0 },
    { 64, 60, 114, 85, event_month_button, button_none, 0, 0, 0, 3, 0, 0 },
    { 40, 125, 140, 167, event_year_button, button_none, 0, 0, 0, 3, 0, 0 },
    { 40, 195, 140, 237, event_year_button, button_none, 0, 0, 0, 3, 1, 0 },
    { 180, 72, 310, 97, event_item_button, button_none, 0, 0, 0, 3, 0, 0 },
    { 320, 72, 450, 97, event_item_button, button_none, 0, 0, 0, 3, 1, 0 },
    { 460, 72, 590, 97, event_item_button, button_none, 0, 0, 0, 3, 2, 0 },
    { 412, 142, 482, 167, event_amount_button, button_none, 0, 0, 0, 3, 0, 0 },
    { 522, 142, 592, 167, event_amount_button, button_none, 0, 0, 0, 3, 1, 0 },
    { 632, 142, 702, 167, event_amount_button, button_none, 0, 0, 0, 3, 2, 0 },
    { 270, 100, 370, 125, event_route_button, button_none, 0, 0, 0, 3, 0, 0 },
    { 420, 100, 520, 125, event_route_button, button_none, 0, 0, 0, 3, 1, 0 },
    { 580, 100, 680, 125, event_route_button, button_none, 0, 0, 0, 3, 2, 0 },
    { 290, 244, 330, 269, event_link_button, button_none, 0, 0, 0, 3, 0, 0 },
    { 290, 276, 330, 301, event_link_button, button_none, 0, 0, 0, 3, 1, 0 },
    { 290, 308, 330, 333, event_link_button, button_none, 0, 0, 0, 3, 2, 0 },
    { 290, 340, 330, 365, event_link_button, button_none, 0, 0, 0, 3, 3, 0 },
    { 430, 20, 590, 45, event_trigger_button, button_none, 0, 0, 0, 1, 0, 0 },
    { 170, 180, 270, 205, event_months_button, button_none, 0, 0, 0, 3, 0, 0 },
    { 170, 212, 370, 237, event_attack_target_button, button_none, 0, 0, 0, 1, 0, 0 },
    { 166, 20, 258, 45, event_sender_button, button_none, 0, 0, 0, 1, 0, 0 },
    { 412, 180, 482, 205, event_invader_button, button_none, 0, 0, 0, 3, 0, 0 },
    { 522, 180, 592, 205, event_invader_button, button_none, 0, 0, 0, 3, 1, 0 },
    { 632, 180, 702, 205, event_invader_button, button_none, 0, 0, 0, 3, 2, 0 },
    { 188, 46, 428, 71, event_subtype_button, button_none, 0, 0, 0, 3, 0, 0 },
    { 472, 212, 592, 237, event_phrase_button, button_none, 0, 0, 0, 3, 0, 0 },
    { 340, 244, 400, 269, event_reason_completed_button, button_none, 0, 0, 0, 3, 0, 0 },
    { 340, 276, 400, 301, event_reason_refused_button, button_none, 0, 0, 0, 3, 0, 0 },
    { 340, 308, 400, 333, event_reason_too_late_button, button_none, 0, 0, 0, 3, 0, 0 },
    { 340, 340, 400, 365, event_reason_defeat_button, button_none, 0, 0, 0, 3, 0, 0 },
};

int32_t empire_goods_list_previous;

int32_t select_list_column_blocks;

uint8_t select_list_heading;

select_list_state select_list;

int32_t select_list_text_base;

int32_t select_list_group;

int32_t select_list_count;

int32_t select_list_y;

int32_t select_list_rows;

int32_t select_list_x;

window_return_slot select_list_return_window;

int32_t *numeric_input_target32;

int16_t *numeric_input_target16;

int8_t *numeric_input_target8;

int32_t numeric_input_editor_use;

int32_t numeric_input_width;

int32_t numeric_input_y;

int32_t numeric_input_x;

int32_t numeric_input_digits;

int32_t numeric_input_value;

window_return_slot numeric_input_return_window;

select_list_button_table select_list_buttons;

int16_t invasion_last_id;

int32_t empire_editor_object_count;

int32_t g_empire_selected_object;

int32_t empire_editor_image_kind;

int32_t empire_editor_image;

static_assert(sizeof(empire_map_headers) == 1280, "empire_header is the savegame and scenario empire headers of 1280 bytes");
empire_map_headers empire_header;

int32_t construction_start_elevation;

uint8_t map_pyramid_redraw_pending;

int32_t arrow_repeat_count;

pharaohmap_info empire_map_region_infos[EMPIRE_MAP_REGIONS] = {
    { 0, 0, 0, 0, 0, 0, 40, 40, 21526, 188, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 1, 0, 0, 0, 0, 0, 40, 40, 21526, 188, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 2, 0, 0, 0, 0, 0, 60, 60, 19236, 168, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 3, 0, 0, 0, 0, 0, 60, 60, 19236, 168, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 4, 0, 0, 0, 0, 0, 80, 80, 16946, 148, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 5, 0, 0, 0, 0, 0, 80, 80, 16946, 148, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 6, 0, 0, 0, 0, 0, 80, 80, 16946, 148, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 7, 0, 0, 0, 0, 0, 80, 80, 16946, 148, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 8, 0, 0, 0, 0, 0, 100, 100, 14656, 128, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 9, 0, 0, 0, 0, 0, 100, 100, 14656, 128, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 10, 0, 0, 0, 0, 0, 100, 100, 14656, 128, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 11, 0, 0, 0, 0, 0, 100, 100, 14656, 128, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 12, 0, 0, 0, 0, 0, 100, 100, 14656, 128, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 13, 0, 0, 0, 0, 0, 100, 100, 14656, 128, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 14, 0, 0, 0, 0, 0, 100, 100, 14656, 128, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 15, 0, 0, 0, 0, 0, 120, 120, 12366, 108, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 16, 0, 0, 0, 0, 0, 120, 120, 12366, 108, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 17, 0, 0, 0, 0, 0, 120, 120, 12366, 108, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 18, 0, 0, 0, 0, 0, 120, 120, 12366, 108, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 19, 0, 0, 0, 0, 0, 120, 120, 12366, 108, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 20, 0, 0, 0, 0, 0, 120, 120, 12366, 108, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 21, 0, 0, 0, 0, 0, 120, 120, 12366, 108, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 22, 0, 0, 0, 0, 0, 160, 160, 7786, 68, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 23, 0, 0, 0, 0, 0, 160, 160, 7786, 68, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 24, 0, 0, 0, 0, 0, 160, 160, 7786, 68, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 25, 0, 0, 0, 0, 0, 160, 160, 7786, 68, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 26, 0, 0, 0, 0, 0, 160, 160, 7786, 68, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 27, 0, 0, 0, 0, 0, 160, 160, 7786, 68, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 28, 0, 0, 0, 0, 0, 160, 160, 7786, 68, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 29, 0, 0, 0, 0, 0, 160, 160, 7786, 68, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 30, 0, 0, 0, 0, 0, 160, 160, 7786, 68, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 31, 0, 0, 0, 0, 0, 160, 160, 7786, 68, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 32, 0, 0, 0, 0, 0, 160, 160, 7786, 68, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 33, 0, 0, 0, 0, 0, 160, 160, 7786, 68, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

empire_map_region_position empire_map_region_positions[EMPIRE_MAP_REGIONS] = {
    { 2, 20 }, { 86, 20 }, { 179, 20 }, { 303, 20 }, { 426, 20 }, { 576, 20 }, { 740, 20 }, { 902, 20 },
    { 4, 164 }, { 143, 164 }, { 280, 164 }, { 417, 164 }, { 566, 164 }, { 703, 164 }, { 848, 164 }, { 4, 334 },
    { 131, 334 }, { 260, 334 }, { 387, 334 }, { 516, 334 }, { 643, 334 }, { 771, 334 }, { 4, 540 }, { 168, 540 },
    { 331, 540 }, { 496, 540 }, { 660, 540 }, { 824, 540 }, { 4, 770 }, { 168, 770 }, { 331, 770 }, { 496, 770 },
    { 660, 770 }, { 824, 770 }
};

new_map_size new_map_sizes[NEW_MAP_SIZES] = {
    { 56, 56, 19694, 172 },
    { 84, 84, 16488, 144 },
    { 112, 112, 13282, 116 },
    { 140, 140, 10076, 88 },
    { 170, 170, 6641, 58 },
    { 226, 226, 229, 2 }
};

const char pharaohmaps_bitmap_file[20] = "Pharaohmaps.bmp";
