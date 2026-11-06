// Access to the finished and the started mission records.

#ifndef GAME_MISSION_RECORDS_H
#define GAME_MISSION_RECORDS_H

#include "game/mission_records_types.h"
#include "window/window_data.h"

#include <stdint.h>

static inline mission_carry_records *mission_finished_records(void)
{
    return &record_savings.finished;
}

static inline mission_carry_records *mission_started_records(void)
{
    return &record_savings.started;
}

#endif
