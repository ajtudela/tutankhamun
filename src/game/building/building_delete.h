// Deleting a building and clearing what it owns.

#ifndef BUILDING_BUILDING_DELETE_H
#define BUILDING_BUILDING_DELETE_H

#include <stdint.h>

void building_delete(int32_t building_id);
void building_clear_related_data(int32_t building_id);

#endif
