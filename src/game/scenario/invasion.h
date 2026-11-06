// The functions of invasion.c that other files call.

#ifndef SCENARIO_INVASION_H
#define SCENARIO_INVASION_H

#include "scenario/scenario_types.h"

#include <stdbool.h>
#include <stdint.h>

void figure_enemy_ship_init(int32_t figure_id);
void scenario_invasion_landing_message(int32_t event_id, int32_t grid_offset);
int32_t scenario_invasion_start(int32_t nation, int32_t size, int32_t point, int32_t formation_param, int32_t event_id);

#endif
