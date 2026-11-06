// The functions of editor_menu.c that other files call: the menu bar and sidebar buttons, the map file dialog's
// buttons and the checks of the map's points before saving.

#ifndef EDITOR_EDITOR_MENU_H
#define EDITOR_EDITOR_MENU_H

#include <stdbool.h>
#include <stdint.h>

void editor_open_win_criteria(void);
void editor_open_flood(void);
void editor_open_allowed_buildings(void);
void editor_open_settings(void);
void editor_open_event_list(void);
void editor_open_gods(void);
void editor_clear_prey_points(void);
void editor_clear_predator_points(void);
void editor_clear_fishing_points(void);
void editor_clear_invasion_landings(void);
void editor_clear_invasion_points(void);
void editor_menu_save_map(void);
void editor_menu_new_map(void);
void editor_menu_open_map(void);
void editor_sidebar_tool(void);
void editor_sidebar_preview(void);
void editor_sidebar_enemy(void);
void editor_sidebar_climate(void);
void editor_sidebar_soldier_images(void);
void editor_button_empire(void);
void editor_map_refresh(void);
void building_info_legion_exit(void);
void editor_map_dialog_button(void);
void help_menu_editor_guide(void);
bool editor_fishing_points_valid(void);
bool editor_terrain_walkable(int32_t terrain);
bool editor_entry_exit_valid(void);
bool editor_river_points_valid(void);
bool editor_invasion_points_valid(void);
bool editor_sea_invasion_points_valid(void);
bool editor_food_kinds_valid(void);
bool map_editor_points_terrain_valid(void);

#endif
