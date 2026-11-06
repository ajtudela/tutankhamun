// The player's record (save\<player name>.dat): the campaign object (mission links and names, the replay's saved
// game, the best result of each mission) followed by the positions of the monuments kept for the next missions;
// records of older versions end before them.

#ifndef GAME_PLAYER_RECORD_H
#define GAME_PLAYER_RECORD_H

#include "scenario/campaign.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

enum player_data_layout {
    PLAYER_DATA_OLD_SIZE = 19052,
};

typedef struct {
    int16_t small_tomb_x;
    int16_t small_tomb_y;
    int16_t medium_tomb_x;
    int16_t medium_tomb_y;
    int16_t large_tomb_x;
    int16_t large_tomb_y;
    uint16_t mausoleum_x_and_orientation;
    int16_t mausoleum_y;
    int16_t library_x;
    int16_t library_y;
    int16_t lighthouse_x;
    int16_t lighthouse_y;
} kept_monument_positions;

static_assert(sizeof(kept_monument_positions) == 0x18, "kept_monument_positions is a player file record of 24 bytes");

typedef struct {
    campaign campaign;
    kept_monument_positions monuments;
} player_data;

static_assert(sizeof(player_data) == 0x4A9C, "player_data is a player file record of 19100 bytes");

const char *player_record_path(void);
int32_t player_record_mission(int32_t mission, int32_t monuments, int32_t score, int32_t months);
bool player_data_write_file(const void *self, const char *file);

#endif
