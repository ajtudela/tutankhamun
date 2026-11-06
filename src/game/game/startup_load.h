// The start-up steps of game/startup_load.c.

#ifndef GAME_STARTUP_LOAD_H
#define GAME_STARTUP_LOAD_H

#include <stdint.h>

enum game_setup_result {
    GAME_SETUP_IMAGES_CHANGED = -2,
    GAME_SETUP_OK = 0,
    GAME_SETUP_NO_MEMORY = 1,
    GAME_SETUP_NO_DATA_FILE = 2,
    GAME_SETUP_NO_EMPIRE_FILE = 3,
};

int32_t settings_read_ini(void);
void settings_start_campaign(int32_t campaign_id);
int32_t settings_load(void);
void settings_load_default_names(void);
int32_t game_setup(void);
void game_state_init(void);
void sidebar_init_positions(void);
void platform_measure_speed(void);

#endif
