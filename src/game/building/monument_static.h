// The part descriptors and the royal tomb constructors of building/monument_static.c.

#ifndef BUILDING_MONUMENT_STATIC_H
#define BUILDING_MONUMENT_STATIC_H

#include "building/monument_object.h"
#include <stdint.h>

void royal_tomb_object_base_init(tomb_object *self);
void small_royal_tomb_object_init(tomb_object *self);
void medium_royal_tomb_object_init(tomb_object *self);
void large_royal_tomb_object_init(tomb_object *self);
void grand_royal_tomb_object_init(tomb_object *self);

#endif
