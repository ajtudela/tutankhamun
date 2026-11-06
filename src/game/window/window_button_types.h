// The records of the windows' button tables (window/window_data.c).
//
// A list of buttons ends with a zero word or with blank records; lists that lie next to
// each other and are read across stay fields of one record.
//
// The readers depend on that layout:
// - the image button input reads the type of the record after a list's last button;
// - the message dialog enables its scroll buttons with one 32-bit store over their four
//   flag bytes (state_word);
// - the editor sidebar counts 15 records (the last one blank), the editor settings 12
//   (two blank) and the trade price dialog 69 from its 67 buttons (into the end word and
//   the empire trade route button that follow).

#ifndef WINDOW_WINDOW_BUTTON_TYPES_H
#define WINDOW_WINDOW_BUTTON_TYPES_H

#include "graphics/button.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>

enum chief_advisor_table {
    CHIEF_ADVISOR_ROWS = 12,
};

typedef struct {
    int16_t placement[8];
    button_handler handlers[2];
    uint32_t flags;
    int32_t parameters[2];
} image_button_state_word;

typedef union {
    image_button buttons[1];
    image_button_state_word state_word;
} image_button_with_state_word;

typedef struct {
    image_button_with_state_word button;
    uint32_t list_end;
} message_scroll_button;

typedef struct {
    image_button buttons[1];
    uint32_t list_end;
} single_image_button;

typedef struct {
    image_button buttons[3];
    uint32_t list_end;
} image_button_triple;

typedef struct {
    image_button buttons[15];
    uint32_t list_end;
} sidebar_editor_button_table;

typedef struct {
    image_button limit_buttons[60];
    image_button blank_buttons[12];
    arrow_button editor_terrain_arrows[3];
} storage_order_button_table;

typedef struct {
    generic_button buttons[1];
    uint32_t list_end;
} single_generic_button;

typedef struct {
    generic_button buttons[43];
    uint32_t list_end;
} name_list_button_table;

typedef struct {
    generic_button rows[23];
    uint32_t rows_end;
    generic_button labor_advisor[9];
    uint32_t labor_advisor_end;
    generic_button labor_priority[10];
    generic_button military_advisor[20];
    generic_button fleet_advisor[20];
    generic_button population_advisor[2];
    generic_button religion_festival[1];
    uint32_t list_end;
} message_list_button_tables;

typedef struct {
    generic_button festival_square[1];
    uint32_t festival_square_end;
    generic_button festival_gods[8];
    generic_button hold_festival_gods[8];
    generic_button kingdom_advisor[8];
} festival_button_tables;

typedef struct {
    generic_button mansion_salary[1];
    uint32_t mansion_salary_end;
    generic_button salaries[12];
    generic_button donations[7];
    uint32_t donations_end;
    generic_button gifts[5];
    uint32_t gifts_end;
    generic_button burial_provisions[3];
    uint32_t burial_provisions_end;
    arrow_button burial_provision_arrows[2];
    generic_button monuments_advisor[8];
} mansion_button_tables;

typedef struct {
    generic_button chief_nile_row[1];
    uint32_t chief_nile_row_end;
    generic_button religion_gods[5];
    uint32_t list_end;
} chief_nile_and_god_buttons;

typedef struct {
    generic_button buttons[1];
    uint32_t list_end;
    arrow_button unused_arrows[2];
} family_name_list_button_table;

typedef struct {
    generic_button buttons[7];
    uint32_t list_end;
} figure_list_button_table;

typedef struct {
    generic_button phrase[1];
    uint32_t phrase_end;
    generic_button editor_settings[12];
    generic_button editor_flood[3];
    uint32_t list_end;
} figure_phrase_button_tables;

typedef struct {
    generic_button buttons[47];
    uint32_t list_end;
} editor_allowed_building_button_table;

typedef struct {
    generic_button buttons[3];
    uint32_t list_end;
} victory_dialog_button_table;

typedef struct {
    generic_button menu[1];
    uint32_t menu_end;
    generic_button choices[5];
    uint32_t list_end;
} defeat_dialog_button_tables;

typedef struct {
    generic_button buttons[5];
    uint32_t list_end;
} order_button_row_table;

typedef struct {
    generic_button orders[5];
    uint32_t orders_end;
    generic_button display_mode_confirm[1];
    uint32_t list_end;
} ship_order_button_tables;

typedef struct {
    generic_button resources[30];
    generic_button blank_resources[6];
    generic_button sidebar_problem[1];
    uint32_t sidebar_problem_end;
    generic_button editor_gods[20];
    generic_button editor_monuments[19];
    uint32_t editor_monuments_end;
    arrow_button editor_temple_complex_arrows[20];
} special_order_button_tables;

typedef struct {
    single_generic_button industry;
    single_generic_button trade;
    single_generic_button import_toggle;
    single_generic_button export_toggle;
    single_generic_button stockpile;
} resource_settings_button_tables;

typedef struct {
    arrow_button arrows[2];
    uint32_t unused[6];
} difficulty_arrow_table;

typedef struct {
    image_button buttons[2];
    uint32_t unused[23];
} high_score_button_table;

typedef struct {
    generic_button buttons[3];
    int32_t building_info_tooltips[15];
} phrase_button_table;

typedef struct {
    generic_button build_menu[30];
    generic_button overlay_menu[10];
    generic_button overlay_submenu[10];
    generic_button display_options[5];
    uint32_t display_options_end;
    generic_button game_options[14];
    generic_button speed_options[2];
    generic_button sound_options[6];
    generic_button file_dialog_rows[12];
    generic_button empire_editor_modes[8];
    generic_button empire_editor_general[1];
    uint32_t empire_editor_general_end;
    generic_button editor_start_year[3];
    uint32_t editor_start_year_end;
    generic_button empire_editor_ornament_kinds[3];
    uint32_t empire_editor_ornament_kinds_end;
    generic_button empire_editor_routes[4];
    generic_button editor_trade_amounts[16];
    generic_button editor_trade_prices[67];
    uint32_t editor_trade_prices_end;
    generic_button empire_trade_route[1];
    uint32_t empire_trade_route_end;
    generic_button empire_editor_city[7];
    uint32_t empire_editor_city_end;
    generic_button empire_editor_battle[2];
    generic_button empire_editor_our_city_sold[14];
    generic_button empire_editor_trade_city_amounts[16];
    generic_button numeric_input[12];
} menu_button_tables;

#if !defined(_WIN64) && !defined(__x86_64__)
static_assert(sizeof(image_button_state_word) == sizeof(image_button), "image_button_state_word is an image_button seen as words");
static_assert(offsetof(image_button_state_word, flags) == offsetof(image_button, enabled), "image_button_state_word's flags are image_button's enabled byte");
#endif

#endif
