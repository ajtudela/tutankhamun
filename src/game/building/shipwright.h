// The shipwrights (building/shipwright.c): repair slots, boat orders and the setup of new boats.

#ifndef BUILDING_SHIPWRIGHT_H
#define BUILDING_SHIPWRIGHT_H

#include <stdbool.h>
#include <stdint.h>

enum shipwright_boat_order {
    BOAT_ORDER_TYPE_MASK = 0x0FFF,
    BOAT_ORDER_FOR_CITY = 0x1000,
    BOAT_ORDER_REPAIR = 0x2000,
    BOAT_ORDER_KIND_MASK = 0xF000,
};

enum warship_ram_damage {
    WARSHIP_FAST_RAM_DAMAGE_PERCENT = 20,
};

void building_repair_slots_update(void);
int32_t ferry_path_alloc(int32_t figure_id);
int32_t shipwright_boat_destination(int32_t boat_type);
bool shipwright_boat_needed(int32_t *type);
bool shipwright_boat_ordered_by(int32_t building_id, int32_t *type);
void boat_init_transport(int32_t figure_id);
void boat_init_warship(int32_t figure_id);

#endif
