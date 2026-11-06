// The functions of game_file_relayout.c that other files call.

#ifndef IO_GAME_FILE_RELAYOUT_H
#define IO_GAME_FILE_RELAYOUT_H

#include <stdint.h>

void game_file_upgrade_farm_fields(void);
void game_file_relayout_empire_cities(void);
void game_file_relayout_players(void);
void game_file_relayout_buildings(void);
void game_file_relayout_storages(void);
void farm_update_map(void);

#endif
