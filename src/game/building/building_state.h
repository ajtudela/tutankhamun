// Building counters and state (building/building_state.c): counts, removal of deleted buildings, subtypes and
// the clearing at the start of a game.

#ifndef BUILDING_BUILDING_STATE_H
#define BUILDING_BUILDING_STATE_H

#include <stdint.h>

void building_clear_all(void);
void building_counters_clear(void);
int32_t building_count_warship_wharves(void);
int32_t building_get_subtype(int32_t building_id);
void building_update_money_values(void);

void building_update_state(void);
void building_update_highest_id(void);
void building_count_update(void);
void building_reset_tick47_counters(void);

#endif
