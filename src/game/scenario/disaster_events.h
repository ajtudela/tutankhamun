// The disaster event processors of scenario/disaster_events.c and the earthquake's tiles.

#ifndef SCENARIO_DISASTER_EVENTS_H
#define SCENARIO_DISASTER_EVENTS_H

#include "scenario/scenario_event_types.h"

#include <stdbool.h>
#include <stdint.h>

void scenario_event_process_reputation_decrease(scenario_event *e);
void scenario_event_process_gold_mine_collapse(scenario_event *e);
void scenario_event_process_frogs(scenario_event *e);
void scenario_event_process_hailstorm(scenario_event *e);
void scenario_event_process_blood_river(scenario_event *e);
void scenario_event_process_crime_wave(scenario_event *e);
void scenario_event_process_mummies(scenario_event *e);
bool scenario_earthquake_tile_free(int32_t x, int32_t y);
void scenario_earthquake_destroy_tile(int32_t x, int32_t y);
bool scenario_earthquake_event_process(scenario_event *e);

#endif
