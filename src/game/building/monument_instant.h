// Monuments of earlier missions completed at once (building/monument_instant.c).

#ifndef BUILDING_MONUMENT_INSTANT_H
#define BUILDING_MONUMENT_INSTANT_H

#include "building/monument_object.h"
#include <stdint.h>

void mausoleum_update_all_images(int32_t building_id);
void mausoleum_complete_instant(int32_t building_id);
void library_set_completed(library_object_t *self, int32_t building_id);
void lighthouse_complete_instant(lighthouse_object_t *self, int32_t building_id);

#endif
