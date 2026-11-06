// The built-in material needs of the monuments not yet placed: per monument definition the main material and its
// amount, and the casing's limestone. config/monument_unplaced_needs.json can override them.

#include "building/monument_spots.h"

monument_unplaced_need monument_unplaced_needs[MONUMENT_DEFINITIONS] = {
    [0] = { RESOURCE_NONE, 0, 0 },
    [1] = { RESOURCE_STONE, 4800, 19200 },
    [2] = { RESOURCE_STONE, 21600, 43200 },
    [3] = { RESOURCE_BRICKS, 4800, 19200 },
    [4] = { RESOURCE_BRICKS, 21600, 43200 },
    [5] = { RESOURCE_BRICKS, 67200, 76800 },
    [6] = { RESOURCE_BRICKS, 144000, 120000 },
    [7] = { RESOURCE_BRICKS, 264000, 160000 },
    [8] = { RESOURCE_STONE, 4800, 0 },
    [9] = { RESOURCE_STONE, 21600, 0 },
    [10] = { RESOURCE_STONE, 67200, 0 },
    [11] = { RESOURCE_STONE, 144000, 0 },
    [12] = { RESOURCE_STONE, 264000, 0 },
    [13] = { RESOURCE_STONE, 4800, 19200 },
    [14] = { RESOURCE_STONE, 21600, 43200 },
    [15] = { RESOURCE_STONE, 67200, 76800 },
    [16] = { RESOURCE_STONE, 144000, 120000 },
    [17] = { RESOURCE_STONE, 264000, 160000 },
    [18] = { RESOURCE_BRICKS, 10800, 0 },
    [19] = { RESOURCE_BRICKS, 24000, 0 },
    [20] = { RESOURCE_BRICKS, 42000, 0 },
    [21] = { RESOURCE_NONE, 0, 0 },
    [22] = { RESOURCE_GRANITE, 10000, 0 },
    [23] = { RESOURCE_GRANITE, 20000, 0 },
    [24] = { RESOURCE_SANDSTONE, 38000, 0 },
    [25] = { RESOURCE_SANDSTONE, 49600, 0 },
    [26] = { RESOURCE_SANDSTONE, 49600, 0 },
    [27] = { RESOURCE_SANDSTONE, 49600, 0 },
};
