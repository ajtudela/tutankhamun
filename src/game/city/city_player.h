// The ranks of a player and the access to the player records: city_player_get(player) and city_player_current().

#ifndef CITY_CITY_PLAYER_H
#define CITY_CITY_PLAYER_H

#include "building/building_data.h"
#include "city/city_data.h"
#include "city/city_player_types.h"

enum player_rank {
    PLAYER_RANK_PHARAOH = 10,
};

static inline city_player *city_player_get(int32_t player)
{
    return &city_players[player];
}

static inline city_player *city_player_current(void)
{
    return city_player_get(city_current_player);
}

#endif
