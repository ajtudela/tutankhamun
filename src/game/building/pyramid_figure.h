// The pyramid workers' walk from part to part (building/pyramid_figure.c).

#ifndef BUILDING_PYRAMID_FIGURE_H
#define BUILDING_PYRAMID_FIGURE_H

#include <stdint.h>

void pyramid_figure_find_next_part(int32_t figure_id, int32_t set_height, uint8_t fail_action, uint8_t done_action, uint8_t next_action);
void pyramid_figure_set_walk_target(int32_t figure_id, int32_t building_id, int32_t height, int32_t set_walk_mode);
int32_t pyramid_figure_climb_step(int32_t figure_id, int32_t step);
int32_t pyramid_laborer_can_enter_ramp(int32_t figure_id);
void pyramid_figure_snap_to_part(int32_t figure_id);
int32_t pyramid_find_mason_waiting_for_block(int32_t building_id, int32_t clear_partner);
void pyramid_figure_walk_on_ramp(int32_t figure_id, int32_t mode, uint8_t fail_action, uint8_t next_action);
void pyramid_figure_start_climb(int32_t figure_id);
void pyramid_stonemason_arrive_part(int32_t figure_id);
void pyramid_laborer_deliver_block(int32_t mason_figure_id);
void pyramid_laborer_pair_with_mason(int32_t figure_id);

#endif
