// The gatehouse and temple complex tables: the four gatehouse parts (road allowed, size, offset), the oracle and
// altar offsets per turn of the temple complex, its part image groups and the part step per direction.

#ifndef BUILDING_GATEHOUSE_TYPES_H
#define BUILDING_GATEHOUSE_TYPES_H

#include <stdint.h>

enum gatehouse_layout {
    GATEHOUSE_PART_COUNT = 4,
    TEMPLE_COMPLEX_TURNS = 4,
    TEMPLE_COMPLEX_PART_GROUPS = 48,
    TEMPLE_COMPLEX_PART_DIRECTIONS = 8,
};

typedef struct gatehouse_part {
    int32_t on_road;
    int32_t size;
    int32_t dy;
    int32_t dx;
} gatehouse_part;

typedef struct temple_tile_offset {
    int32_t x;
    int32_t y;
} temple_tile_offset;

typedef struct gatehouse_part_tables {
    gatehouse_part parts[GATEHOUSE_PART_COUNT];
    temple_tile_offset oracle_tiles[TEMPLE_COMPLEX_TURNS];
    temple_tile_offset altar_tiles[TEMPLE_COMPLEX_TURNS];
    int32_t oracle_pixels[TEMPLE_COMPLEX_TURNS];
    int32_t altar_pixels[TEMPLE_COMPLEX_TURNS];
    int32_t part_groups[TEMPLE_COMPLEX_PART_GROUPS];
    temple_tile_offset part_steps[TEMPLE_COMPLEX_PART_DIRECTIONS];
} gatehouse_part_tables;

#endif
