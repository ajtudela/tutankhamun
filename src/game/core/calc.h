// Calculation helpers (core/calc.c): percentages, missile directions and filling a grid.

#ifndef CORE_CALC_H
#define CORE_CALC_H

#include <stdint.h>

extern int32_t distance_dx;
extern int32_t distance_dy;

int32_t calc_adjust_with_percentage(int32_t value, int32_t percentage);
int32_t calc_percentage(int32_t value, int32_t total);
int32_t calc_divide_round_up(int32_t value, int32_t divisor);

static inline int32_t calc_maximum_distance(int32_t x1, int32_t y1, int32_t x2, int32_t y2)
{
    int32_t dx = (x1 > x2) ? (x1 - x2) : (x2 - x1);
    int32_t dy = (y1 > y2) ? (y1 - y2) : (y2 - y1);
    distance_dx = dx;
    distance_dy = dy;
    return (dx >= dy) ? dx : dy;
}

void memand_grid_u32(uint32_t *grid, int32_t count, uint32_t mask);
int32_t calc_missile_shooter_direction(int32_t x_from, int32_t y_from, int32_t x_to, int32_t y_to);
int32_t calc_missile_direction(int32_t x_from, int32_t y_from, int32_t x_to, int32_t y_to);
uint8_t calc_missile_angle(int32_t x_from, int32_t y_from, int32_t x_to, int32_t y_to);

#endif
