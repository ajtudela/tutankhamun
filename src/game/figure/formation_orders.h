// The legions' orders and the military queries of figure/formation_orders.c, and the rules of an order.

#ifndef FIGURE_FORMATION_ORDERS_H
#define FIGURE_FORMATION_ORDERS_H

#include <stdbool.h>
#include <stdint.h>

enum legion_order {
    LEGION_ATTACK_NEAR_TILES = 9,
    LEGION_MORALE_WARNING = 20,
    LEGION_ORDER_LOW_MORALE_CHANGE = 10,
};

int32_t figure_soldier_refuses_orders(int32_t figure_id);
int32_t formation_legion_move_to(int32_t x, int32_t y, int32_t formation_id, bool quiet);
int32_t formation_legion_attack_near(int32_t x, int32_t y, int32_t formation_id, int32_t target);
int32_t formation_legion_return_to_fort(int32_t x, int32_t y, int32_t formation_id);
void formation_legion_to_standard(int32_t formation_id, int32_t action);
int32_t formation_legion_at_building(int32_t grid_offset);
int32_t figure_fighter_at_grid_offset(int32_t grid_offset);
int32_t figure_warship_by_index(int32_t index);
void figure_warship_return_to_dock(int32_t figure_id);
int32_t formation_any_legion_with_soldiers(void);
int32_t formation_for_legion(int32_t index);
int32_t formation_grid_offset_for_invasion(int32_t value);

#endif
