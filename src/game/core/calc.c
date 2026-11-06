// Arithmetic and grid helpers: percentages, rounded-up division, byte and mask fills of grids, and the
// directions and image angle of a missile flying between two tiles.
//
// calc_divide_round_up adds 1 for any remainder, negative values included, and gives 0 for a divisor of 0;
// calc_missile_angle uses a rough pi (3.1429) and truncates the angle.

#include "core/calc.h"

#include <math.h>
#include <stddef.h>
#include <stdint.h>

enum calc_axis_ratio {
    AXIS_RATIO_DIAGONAL = 100,
    AXIS_RATIO_STRAIGHT = 500,
    AXIS_RATIO_HALF_STEP = 200,
    AXIS_RATIO_SHOOTER_STRAIGHT = 400,
};

static const double missile_angle_pi = 3.1429;
static const double missile_angle_turn = 6.2858;
static const double missile_angle_start = 5.500075;
static const double missile_angle_steps_per_radian = 5.090839670368132;

int32_t calc_adjust_with_percentage(int32_t value, int32_t percentage)
{
    return (int32_t) ((uint32_t) value * (uint32_t) percentage) / 100;
}

int32_t calc_percentage(int32_t value, int32_t total)
{
    if (total == 0) {
        return 0;
    }
    return (int32_t) ((uint32_t) value * 100u) / total;
}

int32_t calc_divide_round_up(int32_t value, int32_t divisor)
{
    if (divisor == 0) {
        return 0;
    }
    int32_t result = value / divisor;
    if ((value % divisor) != 0) {
        result++;
    }
    return result;
}

void memand_grid_u32(uint32_t *grid, int32_t count, uint32_t mask)
{
    for (int32_t i = 0; i < count; i++) {
        grid[i] &= mask;
    }
}

static int32_t axis_ratio(int32_t x_from, int32_t y_from, int32_t x_to, int32_t y_to)
{
    int32_t dx = (x_from > x_to) ? (x_from - x_to) : (x_to - x_from);
    int32_t dy = (y_from > y_to) ? (y_from - y_to) : (y_to - y_from);
    if (dx > dy) {
        return calc_percentage(dx, dy);
    }
    if (dx < dy) {
        return -calc_percentage(dy, dx);
    }
    return AXIS_RATIO_DIAGONAL;
}

int32_t calc_missile_direction(int32_t x_from, int32_t y_from, int32_t x_to, int32_t y_to)
{
    int32_t ratio = axis_ratio(x_from, y_from, x_to, y_to);
    if (x_from == x_to) {
        if (y_from > y_to) {
            return 0;
        }
        if (y_from < y_to) {
            return 8;
        }
    }
    if (x_from > x_to) {
        if (y_from == y_to) {
            return 12;
        }
        if (y_from > y_to) {
            if (ratio >= AXIS_RATIO_STRAIGHT) {
                return 12;
            }
            if (ratio >= AXIS_RATIO_HALF_STEP) {
                return 13;
            }
            if (ratio > -AXIS_RATIO_HALF_STEP) {
                return 14;
            }
            return (ratio <= -AXIS_RATIO_STRAIGHT) ? 0 : 15;
        }
        if (ratio >= AXIS_RATIO_STRAIGHT) {
            return 12;
        }
        if (ratio >= AXIS_RATIO_HALF_STEP) {
            return 11;
        }
        if (ratio > -AXIS_RATIO_HALF_STEP) {
            return 10;
        }
        return (ratio > -AXIS_RATIO_STRAIGHT) ? 9 : 8;
    }
    if (x_from >= x_to) {
        return 0;
    }
    if (y_from == y_to) {
        return 4;
    }
    if (y_from > y_to) {
        if (ratio >= AXIS_RATIO_STRAIGHT) {
            return 4;
        }
        if (ratio >= AXIS_RATIO_HALF_STEP) {
            return 3;
        }
        if (ratio > -AXIS_RATIO_HALF_STEP) {
            return 2;
        }
        return (ratio > -AXIS_RATIO_STRAIGHT) ? 1 : 0;
    }
    if (ratio >= AXIS_RATIO_STRAIGHT) {
        return 4;
    }
    if (ratio >= AXIS_RATIO_HALF_STEP) {
        return 5;
    }
    if (ratio > -AXIS_RATIO_HALF_STEP) {
        return 6;
    }
    return (ratio <= -AXIS_RATIO_STRAIGHT) ? 8 : 7;
}

int32_t calc_missile_shooter_direction(int32_t x_from, int32_t y_from, int32_t x_to, int32_t y_to)
{
    int32_t ratio = axis_ratio(x_from, y_from, x_to, y_to);
    if (x_from == x_to) {
        if (y_from > y_to) {
            return 0;
        }
        if (y_from < y_to) {
            return 4;
        }
    }
    if (x_from > x_to) {
        if (y_from == y_to) {
            return 6;
        }
        if (y_from > y_to) {
            if (ratio >= AXIS_RATIO_SHOOTER_STRAIGHT) {
                return 6;
            }
            return (ratio <= -AXIS_RATIO_SHOOTER_STRAIGHT) ? 0 : 7;
        }
        if (ratio >= AXIS_RATIO_SHOOTER_STRAIGHT) {
            return 6;
        }
        return (ratio > -AXIS_RATIO_SHOOTER_STRAIGHT) ? 5 : 4;
    }
    if (x_from >= x_to) {
        return 0;
    }
    if (y_from == y_to) {
        return 2;
    }
    if (y_from > y_to) {
        if (ratio >= AXIS_RATIO_SHOOTER_STRAIGHT) {
            return 2;
        }
        return (ratio > -AXIS_RATIO_SHOOTER_STRAIGHT) ? 1 : 0;
    }
    if (ratio >= AXIS_RATIO_SHOOTER_STRAIGHT) {
        return 2;
    }
    return (ratio <= -AXIS_RATIO_SHOOTER_STRAIGHT) ? 4 : 3;
}

static inline int32_t ftol(double value)
{
    return (int32_t) (int64_t) value;
}

uint8_t calc_missile_angle(int32_t x_from, int32_t y_from, int32_t x_to, int32_t y_to)
{
    double dx = (double) (x_to - x_from);
    double dy = (double) (y_from - y_to);
    double distance = sqrt((dy * dy) + (dx * dx));
    if (distance == 0.0) {
        return 0;
    }
    double angle = asin(dy / distance);
    if (dx <= 0.0) {
        angle = missile_angle_pi - angle;
    }
    angle -= missile_angle_start;
    while (angle < 0.0) {
        angle += missile_angle_turn;
    }
    return (uint8_t) ftol((missile_angle_turn - angle) * missile_angle_steps_per_radian);
}
