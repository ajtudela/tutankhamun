// The tables of editor/editor_data.c.

#ifndef EDITOR_EDITOR_DATA_H
#define EDITOR_EDITOR_DATA_H

#include "editor/editor_new_scenario.h"
#include "editor/empire_editor_objects.h"
#include "empire/empire_types.h"
#include "graphics/button.h"
#include "graphics/menu.h"
#include "window/input_dialogs.h"
#include "window/window_state_types.h"

#include <stdint.h>

extern const char map_file_pattern[12];

enum menu_item_index {
    MENU_ITEM_OPTIONS_CITY_NAMES = 10,
    MENU_ITEM_OPTIONS_AUTOSAVE = 12,
    MENU_ITEM_OPTIONS_PYRAMID_SPEEDUP = 13,
    MENU_ITEM_HELP_TOOLTIPS = 15,
    MENU_ITEM_HELP_WARNINGS = 16,
    MENU_ITEM_EDITOR_HELP_TOOLTIPS = 19,
};

extern menu_item g_menu_items[MENU_ITEMS];

extern int32_t empire_editor_route_dragging;

extern int32_t route_current;

typedef struct {
    image_button arrows[2];
    generic_button rows[14];
} event_list_buttons_layout;

extern event_list_buttons_layout event_list_buttons[1];

enum editor_event_type_button_count {
    EDITOR_EVENT_TYPE_BUTTONS = 30,
};

extern generic_button editor_event_type_buttons[EDITOR_EVENT_TYPE_BUTTONS];

extern int32_t empire_goods_list_previous;

extern int32_t select_list_column_blocks;

extern uint8_t select_list_heading;

extern select_list_state select_list;

extern int32_t select_list_text_base;

extern int32_t select_list_group;

extern int32_t select_list_count;

extern int32_t select_list_y;

extern int32_t select_list_rows;

extern int32_t select_list_x;

extern window_return_slot select_list_return_window;

extern int32_t *numeric_input_target32;

extern int16_t *numeric_input_target16;

extern int8_t *numeric_input_target8;

extern int32_t numeric_input_editor_use;

extern int32_t numeric_input_width;

extern int32_t numeric_input_y;

extern int32_t numeric_input_x;

extern int32_t numeric_input_digits;

extern int32_t numeric_input_value;

extern window_return_slot numeric_input_return_window;

extern select_list_button_table select_list_buttons;

extern int16_t invasion_last_id;

extern int32_t empire_editor_object_count;

extern int32_t g_empire_selected_object;

extern int32_t empire_editor_image_kind;

extern int32_t empire_editor_image;

extern empire_map_headers empire_header;

extern int32_t construction_start_elevation;

extern uint8_t map_pyramid_redraw_pending;

extern int32_t arrow_repeat_count;

extern pharaohmap_info empire_map_region_infos[EMPIRE_MAP_REGIONS];

extern empire_map_region_position empire_map_region_positions[EMPIRE_MAP_REGIONS];

extern new_map_size new_map_sizes[NEW_MAP_SIZES];

extern const char pharaohmaps_bitmap_file[20];

#endif
