// Shared by the derived map layers that the tick updates (desirability, water supply, road networks, service visits,
// vegetation): their common includes and the grid fill and AND helpers.

#ifndef MAP_MAP_LAYERS_H
#define MAP_MAP_LAYERS_H

#include "building/building.h"
#include "map/grid.h"
#include "map/road_network.h"
#include "map/water_supply.h"

#include <stdint.h>
#include "map/terrain.h"

void memand_grid_u32(uint32_t *grid, int32_t count, uint32_t mask);

#endif
