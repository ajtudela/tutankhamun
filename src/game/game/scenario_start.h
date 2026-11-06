// The functions of scenario_start.c that other files call.

#ifndef GAME_SCENARIO_START_H
#define GAME_SCENARIO_START_H

#include <stdbool.h>
#include <stdint.h>

void game_file_start_scenario(const char *name, bool replay);
void scenario_start_custom(const char *path);
int32_t scenario_map_exists(char *name);
void figure_action_profile_init(void);
void scenario_load_map(void);
void figure_create_fishing_points(void);
void traders_clear(void);
void figure_init_scenario(void);
void scenario_invasion_amounts_clear(void);
void figure_name_init(void);

#endif
