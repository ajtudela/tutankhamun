// Undo of a build (game/undo_build.c): the buildings of the last placement.

#ifndef GAME_UNDO_BUILD_H
#define GAME_UNDO_BUILD_H

#include <stdint.h>

int32_t game_undo_add_building(int32_t building_id);
void game_undo_finish_build(void);
void game_undo_start_build(void);
void map_images_restore_free_tiles(void);
void game_undo_perform(void);
int32_t game_undo_backup_map(void);
void game_undo_restore_building(int32_t id);
void game_undo_restore_houses(void);

#endif
