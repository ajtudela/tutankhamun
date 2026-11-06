// Image contexts: the 8 neighbours of the tile at the tile cursor are reduced to values in map_adjacent_tiles and
// matched against a table of neighbour patterns, which gives the image group offset and the variant of the tile's
// image (plain, reversed and floodplain matching). Also the neighbour count of a tile for a terrain bit.

#include "map/image_context.h"

#include "map/tiles_internal.h"

#include <stdint.h>

static const int32_t neighbour_delta[DIRECTION_COUNT] = { -GRID_SIZE, 1 - GRID_SIZE, 1, GRID_SIZE + 1, GRID_SIZE, GRID_SIZE - 1, -1, -GRID_SIZE - 1 };

static uint32_t neighbour_terrain(int32_t i)
{
    return map_grids.terrain[map_tile_cursor_offset + neighbour_delta[i]];
}

void map_image_context_set_tiles(uint32_t terrain_mask)
{
    for (int32_t i = 0; i < DIRECTION_COUNT; i++) {
        map_adjacent_tiles[i] = (neighbour_terrain(i) & terrain_mask) != 0;
    }
}

void map_image_context_set_tiles_all(uint32_t terrain_mask)
{
    for (int32_t i = 0; i < DIRECTION_COUNT; i++) {
        map_adjacent_tiles[i] = (neighbour_terrain(i) & terrain_mask) == terrain_mask;
    }
}

void map_image_context_set_tiles_without(uint32_t terrain_mask, uint32_t not_mask)
{
    for (int32_t i = 0; i < DIRECTION_COUNT; i++) {
        uint32_t terrain = neighbour_terrain(i);
        map_adjacent_tiles[i] = (terrain & terrain_mask) && !(terrain & not_mask);
    }
}

void map_image_context_or_tiles(uint32_t terrain_mask, int32_t bit)
{
    int32_t shift = ((uint8_t) bit) & 0x1F;
    for (int32_t i = 0; i < DIRECTION_COUNT; i++) {
        map_adjacent_tiles[i] |= ((neighbour_terrain(i) & terrain_mask) != 0) << shift;
    }
}

void map_image_context_set_tiles_fault(void)
{
    for (int32_t i = 0; i < DIRECTION_COUNT; i++) {
        int32_t offset = map_tile_cursor_offset + neighbour_delta[i];
        map_adjacent_tiles[i] = (map_grids.terrain[offset] & TERRAIN_ROCK) && (map_grids.bitfields[offset] & MAP_BITFIELD_PLAZA_OR_FAULT);
    }
}

static int32_t context_matches(const terrain_image_context *context)
{
    for (int32_t i = 0; i < DIRECTION_COUNT; i++) {
        int32_t pattern = context->tiles[i];
        if (pattern == CONTEXT_TILE_ANY) {
            continue;
        }
        if ((pattern != CONTEXT_TILE_NO) != (map_adjacent_tiles[i] != 0)) {
            return 0;
        }
    }
    return 1;
}

static void use_context(terrain_image_context *context, int32_t random, int32_t use_random)
{
    context_group_offset = context->offset_for_orientation[map_info.orientation / 2];
    context_canal_offset = context->canal_offset;
    if (use_random) {
        context_item_offset = random % context->max_item_offset;
    } else {
        context->current_item_offset++;
        if (context->current_item_offset >= context->max_item_offset) {
            context->current_item_offset = 0;
        }
        context_item_offset = context->current_item_offset;
    }
}

int32_t map_image_context_match(terrain_image_context *contexts, int32_t count, int32_t random, int32_t reverse)
{
    if (!(char) reverse) {
        for (int32_t i = 0; i < count; i++) {
            if (context_matches(&contexts[i])) {
                use_context(&contexts[i], random, random >= 0);
                return i + 1;
            }
        }
    } else {
        for (int32_t i = 0; i < count; i++) {
            terrain_image_context *context = &contexts[count - 1 - i];
            if (context_matches(context)) {
                use_context(context, random, random != 0);
                return i + 1;
            }
        }
    }
    return 0;
}

int32_t map_image_context_match_floodsystem(terrain_image_context *contexts, int32_t count)
{
    static const uint8_t group_for_corners[16] = { 0, 1, 3, 2, 5, 1, 4, 2, 6, 7, 1, 1, 5, 1, 1, 1 };
    uint32_t corners = 0;
    for (int32_t i = 0; i < DIRECTION_COUNT; i += 2) {
        if (map_adjacent_tiles[i + 1] & FLOODSYSTEM_TILE_WATER) {
            corners |= (1u << (((i + 2) >> 1) & 3)) | (1u << (i >> 1));
        }
        if (map_adjacent_tiles[i] & FLOODSYSTEM_TILE_WATER) {
            corners |= 1u << (i >> 1);
        }
    }
    int32_t group = corners ? group_for_corners[corners] : 0;
    contexts += group * count;
    for (int32_t i = 0; i < count; i++) {
        terrain_image_context *context = &contexts[i];
        int32_t matches = 1;
        for (int32_t t = 0; t < DIRECTION_COUNT; t++) {
            int32_t pattern = context->tiles[t];
            if (pattern != CONTEXT_TILE_ANY && (pattern != CONTEXT_TILE_NO) != ((map_adjacent_tiles[t] & FLOODSYSTEM_TILE_EDGE) != 0)) {
                matches = 0;
                break;
            }
        }
        if (matches) {
            use_context(context, 0, 0);
            return (group * count) + i + 1;
        }
    }
    return 0;
}

void map_image_context_reset(terrain_image_context *contexts, int32_t count)
{
    for (int32_t i = 0; i < count; i++) {
        contexts[i].current_item_offset = 0;
    }
}

void map_image_context_invert_tiles(void)
{
    for (int32_t i = 0; i < DIRECTION_COUNT; i++) {
        map_adjacent_tiles[i] = map_adjacent_tiles[i] == 0;
    }
}

void map_image_context_normalize_tiles(void)
{
    for (int32_t i = 0; i < DIRECTION_COUNT; i++) {
        map_adjacent_tiles[i] = map_adjacent_tiles[i] != 0;
    }
}

int32_t map_image_context_count_tiles(void)
{
    int32_t count = 0;
    for (int32_t i = 0; i < DIRECTION_COUNT; i++) {
        if (map_adjacent_tiles[i]) {
            count++;
        }
    }
    return count;
}

int32_t map_terrain_count_neighbours_with(uint32_t terrain_mask, int32_t offset)
{
    int8_t count = 0;
    for (int32_t i = 0; i < DIRECTION_COUNT; i++) {
        if (map_grids.terrain[offset + neighbour_delta[i]] & terrain_mask) {
            count++;
        }
    }
    return count;
}
