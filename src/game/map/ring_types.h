// The records of the ring tables (map/ring.c): a ring tile as its (x, y) step from the centre, the table of ring tile
// counts and first offsets, and the grid offsets of the rings by distance and by building size.

#ifndef MAP_RING_TYPES_H
#define MAP_RING_TYPES_H

#include <stdint.h>

enum ring_distance {
    RING_DISTANCE_MAX = 12,
    RING_AROUND_BUILDING_MAX = 6,
    RING_DISTANCE_FLOODPLAIN_SHARE = 4,
};

enum ring_building_size {
    RING_BUILDING_SIZE_SMALLEST = 2,
    RING_BUILDING_SIZE_LARGEST = 6,
    RING_BUILDING_SIZES = 5,
    RING_BUILDING_TILES = 1200,
    RING_UNUSED_TILES = 168,
};

typedef struct {
    int8_t x;
    int8_t y;
} ring_tile;

typedef struct {
    ring_tile distance12_tiles[96];
    ring_tile distance11_tiles[88];
    ring_tile distance10_tiles[80];
    ring_tile distance9_tiles[72];
    ring_tile distance8_tiles[64];
    ring_tile distance7_tiles[56];
    ring_tile distance6_tiles[48];
    ring_tile distance5_tiles[40];
    ring_tile distance4_tiles[32];
    ring_tile distance3_tiles[24];
    ring_tile distance2_tiles[16];
    ring_tile distance1_tiles[8];
    int16_t size6_offsets[288];
    int16_t size5_offsets[264];
    int16_t size4_offsets[240];
    int16_t size3_offsets[216];
    int16_t size2_offsets[192];
} ring_offset_tables;

typedef struct {
    int16_t tile_count[RING_DISTANCE_MAX + 1];
    int16_t building_tile_count[RING_BUILDING_SIZES][RING_AROUND_BUILDING_MAX];
    int16_t tile_start[RING_DISTANCE_MAX];
    int16_t building_tile_start[RING_BUILDING_SIZES][RING_AROUND_BUILDING_MAX];
    ring_tile building_tiles[RING_BUILDING_TILES];
    ring_tile unused_tiles[RING_UNUSED_TILES];
} ring_tile_table;

#endif
