// Image contexts (map/image_context.c): one entry of a context table (the pattern of the 8 neighbours, the group
// offset per view orientation, the canal offset and the variants) and the functions that fill and match the
// neighbours.

#ifndef MAP_IMAGE_CONTEXT_H
#define MAP_IMAGE_CONTEXT_H

#include "map/grid.h"

#include <stdint.h>

enum context_tile_pattern {
    CONTEXT_TILE_NO = 0,
    CONTEXT_TILE_YES = 1,
    CONTEXT_TILE_ANY = 2,
};

enum floodsystem_tile_bit {
    FLOODSYSTEM_TILE_EDGE = 0x1,
    FLOODSYSTEM_TILE_WATER = 0x2,
};

typedef struct {
    uint8_t tiles[DIRECTION_COUNT];
    uint8_t offset_for_orientation[4];
    uint8_t canal_offset;
    uint8_t max_item_offset;
    uint8_t current_item_offset;
    uint8_t unused;
} terrain_image_context;

void map_image_context_set_tiles(uint32_t terrain_mask);
void map_image_context_set_tiles_all(uint32_t terrain_mask);
void map_image_context_set_tiles_without(uint32_t terrain_mask, uint32_t not_mask);
void map_image_context_or_tiles(uint32_t terrain_mask, int32_t bit);
void map_image_context_set_tiles_fault(void);
int32_t map_image_context_match(terrain_image_context *contexts, int32_t count, int32_t random, int32_t reverse);
int32_t map_image_context_match_floodsystem(terrain_image_context *contexts, int32_t count);
void map_image_context_reset(terrain_image_context *contexts, int32_t count);
void map_image_context_invert_tiles(void);
void map_image_context_normalize_tiles(void);
int32_t map_image_context_count_tiles(void);
int32_t map_terrain_count_neighbours_with(uint32_t terrain_mask, int32_t offset);

#include "map/image_context_data.h"

#endif
