// The size of the map grid: 228 x 228 tiles, and the offset of a tile x, y from another.

#ifndef MAP_GRID_SIZE_H
#define MAP_GRID_SIZE_H

#define GRID_SIZE 228
#define GRID_CELLS (GRID_SIZE * GRID_SIZE)
#define GRID_OFFSET(x, y) ((y) * GRID_SIZE + (x))

#endif
