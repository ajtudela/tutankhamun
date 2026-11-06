// The records a won mission keeps for the next one: the savings and the best legion of each kind, for the mission
// just finished and for the one being started (record_savings).

#ifndef GAME_MISSION_RECORDS_TYPES_H
#define GAME_MISSION_RECORDS_TYPES_H

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

enum mission_legion_kind {
    MISSION_LEGION_INFANTRY = 0,
    MISSION_LEGION_ARCHERS = 1,
    MISSION_LEGION_CHARIOTEERS = 2,
    MISSION_LEGION_KINDS = 3,
    MISSION_LEGION_NONE = 0xFF,
};

typedef struct {
    uint8_t legion_id;
    uint8_t reserved_1[3];
    int32_t experience;
    uint8_t soldiers;
    uint8_t reserved_2[3];
} mission_legion_record;

typedef struct {
    int32_t savings;
    mission_legion_record legions[MISSION_LEGION_KINDS];
} mission_carry_records;

typedef struct {
    mission_carry_records finished;
    mission_carry_records started;
} mission_records_state;

static_assert(sizeof(mission_carry_records) == 40, "mission_carry_records is a player file record of 40 bytes");
static_assert(sizeof(mission_records_state) == 80, "mission_records_state is a player file record of 80 bytes");
static_assert(offsetof(mission_records_state, started) == 0x28, "mission_records_state is the player file's layout");

#endif
