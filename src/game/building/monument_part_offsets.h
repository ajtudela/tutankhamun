// The pixel offsets of the monument parts (building/monument_part_offsets.c).

#ifndef BUILDING_MONUMENT_PART_OFFSETS_H
#define BUILDING_MONUMENT_PART_OFFSETS_H

#include "building/monument_object.h"
#include "building/royal_tomb.h"
#include <stdint.h>

void abu_simbel_rotate_part_descs(abu_simbel_object_t *self);
void library_rotate_part_descs(library_object_t *self);
void caesareum_rotate_part_descs(caesareum_object_t *self);
void lighthouse_rotate_parts(lighthouse_object_t *self);
void small_royal_tomb_rotate_part_descs(tomb_object *self);
void medium_royal_tomb_rotate_parts(tomb_object *self);
void large_royal_tomb_rotate_parts(tomb_object *self);
void grand_royal_tomb_rotate_parts(tomb_object *self);
void sun_temple_init_part_offsets(void);
void mausoleum_init_part_offsets(void);

#endif
