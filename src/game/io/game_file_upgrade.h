// The functions of game_file_upgrade.c the loaders call: the upgrades by version, the fixes of every load, the route
// path after loading, the dock queues of the save and the chunk converters.

#ifndef IO_GAME_FILE_UPGRADE_H
#define IO_GAME_FILE_UPGRADE_H

#include "empire/empire_route.h"

#include <stdint.h>

void game_file_upgrade_empire_object_states(void);
void game_file_upgrade_clear_formation_field(void);
void game_file_upgrade_clear_message_fields(void);
void game_file_upgrade_copy_player_block(void);
void game_file_upgrade_widen_player_counts(void);
void game_file_upgrade_widen_player_counts2(void);
void game_file_upgrade_empire_object_flags(void);
void game_file_upgrade_message_text_ids(void);
void game_file_upgrade_clear_figure_fields(void);
void game_file_assign_figure_name_groups(void);
void game_file_upgrade_obelisk_work_spots(void);
void game_file_reset_mission_records(void);
void game_file_upgrade_figure_targets(void);
void game_file_upgrade_formation_figures(void);
void game_file_upgrade_event_fields(void);
void game_file_upgrade_monument_fields(void);
void game_file_upgrade_clear_formation_flags(void);
void game_file_upgrade_building_storage_flags(void);
void game_file_reset_player_fields(void);
void game_file_upgrade_clear_building_field(void);
void game_file_upgrade_clear_player_fields(void);
void game_file_upgrade_scenario_events(void);
void game_file_upgrade_building_orientation(void);
char game_file_build_empire_route_path(empire_route *route);
void game_file_transfer_records(char load);
void game_file_update_granary_stock(void);
void game_file_update_festival_square(void);
void game_file_convert_scenario_events(void *element, int32_t has_data);
void game_file_convert_message(void *element, int32_t has_data);

#endif
