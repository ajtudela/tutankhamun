// The monuments' placement ghosts (methods of the scripted monument objects) and the pyramid preview update.

#ifndef WIDGET_CONSTRUCTION_GHOST_MONUMENTS_H
#define WIDGET_CONSTRUCTION_GHOST_MONUMENTS_H

#include "building/monument_object_types.h"
#include <stdint.h>

void abu_simbel_draw_placement(monument_object *self);
void caesareum_draw_placement(monument_object *self);
void lighthouse_draw_placement(monument_object *self);
void small_royal_tomb_draw_placement(tomb_object *self);
void medium_royal_tomb_draw_placement(tomb_object *self);
void large_royal_tomb_draw_placement(tomb_object *self);
void grand_royal_tomb_draw_placement(tomb_object *self);
void sun_temple_draw_placement(void);
void mausoleum_draw_placement(void);
void pyramid_draw_construction_preview(void);
void pyramid_preview_update(void);

#endif
