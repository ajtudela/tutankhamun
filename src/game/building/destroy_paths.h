// Removal of special buildings (building/destroy_paths.c) and the tile bits a cleared tile keeps.

#ifndef BUILDING_DESTROY_PATHS_H
#define BUILDING_DESTROY_PATHS_H

#include "building/monument_object.h"
#include "building/monument_tables.h"
#include "building/royal_tomb.h"
#include "map/terrain.h"
#include <stdint.h>

enum destroyed_tile {
    DESTROYED_TILE_BITFIELD_KEEP = 0xE0,
    DESTROYED_TILE_EDGE_KEEP = 0xC0,
    LEVELED_GROUND_IMAGES = 8,
    MONUMENT_ROCK_TERRAIN = TERRAIN_ROCK | TERRAIN_ROCK_NEAR_MONUMENT,
};

enum gatehouse_orientation {
    GATEHOUSE_ORIENTATION_AS_RECORDED = 0,
    GATEHOUSE_ORIENTATION_SWAPPED = 1,
};

void abu_simbel_remove_tiles(abu_simbel_object_t *self, int32_t building_id);
void caesareum_remove_tiles(caesareum_object_t *self, int32_t building_id);
void grand_royal_tomb_clear_tiles(tomb_object *self, int32_t building_id);
void large_royal_tomb_clear_tiles(tomb_object *self, int32_t building_id);
void library_remove_tiles(library_object_t *self, int32_t building_id);
void lighthouse_remove_tiles(lighthouse_object_t *self, int32_t building_id);
void medium_royal_tomb_clear_tiles(tomb_object *self, int32_t building_id);
void royal_tomb_clear_tiles(tomb_object *self, int32_t building_id, const monument_part_desc *descs, int32_t count);
void small_royal_tomb_clear_tiles(tomb_object *self, int32_t building_id);
void building_gatehouse_destroy_by_fire(int32_t building_id);
void building_gatehouse_remove_tiles(int32_t building_id);
void building_temple_complex_remove_tiles(int32_t building_id);
void formation_legion_delete_for_fort(int32_t formation_id);
void obelisk_on_delete(int32_t building_id);

#endif
