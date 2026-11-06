// Pyramid part helpers (building/pyramid_parts.c): rings, pairs, frontier order and image refresh.

#ifndef BUILDING_PYRAMID_PARTS_H
#define BUILDING_PYRAMID_PARTS_H

#include "building/pyramid_refresh_types.h"
#include <stdbool.h>
#include <stdint.h>

void pyramid_update_all_part_images(int32_t building_id);
int32_t pyramid_next_on_ring(int32_t building_id);
int32_t pyramid_frontier_pair_part(int32_t building_id, uint8_t *is_first);
bool pyramid_laborer_brings_block_to(int32_t laborer_id, int32_t worker_id);
bool pyramid_part_is_pair_second(int32_t building_id);
int32_t pyramid_pair_waiting_mason(int32_t building_id);
int32_t pyramid_part_pair(int32_t building_id);
void pyramids_update_all_images(void);
void pyramid_complex_queue_refresh(int32_t building_id);
void pyramid_complex_process_refresh(void);
int pyramid_frontier_compare(const void *a, const void *b);

#endif
