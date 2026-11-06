// Building maintenance (building/building_maintenance.c): access checks, fire, collapse, rubble, the fire
// direction and the criminals of unhappy houses.

#ifndef BUILDING_BUILDING_MAINTENANCE_H
#define BUILDING_BUILDING_MAINTENANCE_H

#include "building/building_types.h"
#include <stdbool.h>
#include <stdint.h>

enum burning_ruin {
    BURNING_RUIN_VARIANTS = 4,
    BURNING_RUIN_IMAGE_STEP = 9,
    BURNING_RUIN_FIRE_DAYS_MASK = 7,
};

bool figure_criminal_tomb_robbers_possible(void);
void building_destroy_by_fire(int32_t building_id, int32_t plague);
void building_destroy_linked_parts(int32_t building_id, int32_t on_fire);
void building_destroy_multipart_by_fire(int32_t building_id);
void building_gatehouse_set_rubble(int32_t building_id);
void building_maintenance_destroy_last_placed(void);
void building_maintenance_update_fire_direction(void);
void figure_generate_criminal(int32_t building_id, int32_t tomb_robber);
void figure_generate_criminals(void);
void building_maintenance_check_kingdom_access(void);
void building_maintenance_update_burning_ruins(void);
void building_maintenance_check_fire_collapse(void);
int32_t building_is_checked_for_fire(const building *b, int32_t id);
int32_t building_burns_whole(int32_t type);

#endif
