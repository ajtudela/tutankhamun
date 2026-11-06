// The royal tombs' placement ghost, called by the four tombs' placement methods with their part table.

#ifndef WIDGET_CONSTRUCTION_GHOST_ROYAL_TOMB_H
#define WIDGET_CONSTRUCTION_GHOST_ROYAL_TOMB_H

#include "building/monument_tables.h"
#include "building/royal_tomb.h"
#include <stdint.h>

void royal_tomb_draw_placement(tomb_object *self, const monument_part_desc *descs, int32_t count);

#endif
