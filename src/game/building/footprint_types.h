// The footprint offset tables: per view the grid offsets of a 6x6 footprint's tiles in drawing order, then per
// footprint size the pixel offsets of its tiles.

#ifndef BUILDING_FOOTPRINT_TYPES_H
#define BUILDING_FOOTPRINT_TYPES_H

#include <stdint.h>

enum footprint_offset_counts {
    FOOTPRINT_VIEWS = 4,
    FOOTPRINT_VIEW_TILES = 36,
};

typedef struct footprint_offset_tables {
    int32_t grid_offsets[FOOTPRINT_VIEWS][FOOTPRINT_VIEW_TILES];
    int32_t size2_pixels[2 * 4];
    int32_t size3_pixels[2 * 9];
    int32_t size4_pixels[2 * 16];
    int32_t size5_pixels[2 * 25];
    int32_t size6_pixels[2 * 36];
} footprint_offset_tables;

#endif
