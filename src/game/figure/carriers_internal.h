// Shared by figure/carriers.c, boats.c and trade.c: the direction of a figure relative to the view and the resources
// carried on a sled-like cart, which are drawn from the other side.

#ifndef FIGURE_CARRIERS_INTERNAL_H
#define FIGURE_CARRIERS_INTERNAL_H

#include "building/building_data.h"
#include "city/resource_type.h"
#include "figure/figure_data.h"
#include "map/grid.h"
#include <stdint.h>

static inline int32_t carrier_image_direction(int32_t direction)
{
    int32_t dir = direction - map_info.orientation;
    figure_image_direction = dir;
    if (dir < 0) {
        dir += DIRECTION_COUNT;
        figure_image_direction = dir;
    }
    return dir;
}

static inline int32_t carrier_is_big_resource(int32_t resource)
{
    switch (resource) {
        case RESOURCE_BRICKS:
        case RESOURCE_STONE:
        case RESOURCE_LIMESTONE:
        case RESOURCE_GRANITE:
        case RESOURCE_SANDSTONE:
        case RESOURCE_MARBLE:
            return 1;
    }
    return 0;
}

#endif
