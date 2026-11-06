// Access tiles of the royal tombs and the sun temple: the tile next to a part where a worker or a tomb robber
// goes, by the offset of the monument's table for the part.
//
// Parts outside the tables and the sun temple's side rows are handled unevenly:
// - a part outside the table gets its own tile in the small tomb and the sun temple, and nothing in the others;
// - the sun temple's side rows are entered from the inner side, right for even parts and left for odd ones.

#include "building/monument_access.h"
#include "building/building.h"
#include "building/monument_data.h"
#include "building/sun_temple.h"
#include <stdbool.h>

enum part_outside_table {
    OUTSIDE_TABLE_UNCHANGED,
    OUTSIDE_TABLE_OWN_TILE,
};

static bool set_access_tile(enum monument_access_kind kind, int32_t index, enum part_outside_table outside, int32_t building_id, int32_t *x, int32_t *y)
{
    const monument_access_table *table = &monument_access_tables[kind];
    const building *b = building_get(building_id);
    if (index >= 0 && index < table->count) {
        *x = b->x + table->offsets[index].dx;
        *y = b->y + table->offsets[index].dy;
    } else if (outside == OUTSIDE_TABLE_OWN_TILE) {
        *x = b->x;
        *y = b->y;
    }
    return 1;
}

bool small_royal_tomb_get_access_tile(tomb_object *self, int32_t building_id, int32_t *x, int32_t *y)
{
    int32_t part = building_get(building_id)->subtype;
    return set_access_tile(MONUMENT_ACCESS_SMALL_ROYAL_TOMB, part, OUTSIDE_TABLE_OWN_TILE, building_id, x, y);
}

bool medium_royal_tomb_get_entry_tile(tomb_object *self, int32_t building_id, int32_t *x, int32_t *y)
{
    int32_t part = building_get(building_id)->subtype;
    return set_access_tile(MONUMENT_ACCESS_MEDIUM_ROYAL_TOMB, part, OUTSIDE_TABLE_UNCHANGED, building_id, x, y);
}

bool large_royal_tomb_get_entry_tile(tomb_object *self, int32_t building_id, int32_t *x, int32_t *y)
{
    int32_t part = building_get(building_id)->subtype;
    return set_access_tile(MONUMENT_ACCESS_LARGE_ROYAL_TOMB, part, OUTSIDE_TABLE_UNCHANGED, building_id, x, y);
}

bool grand_royal_tomb_get_entry_tile(tomb_object *self, int32_t building_id, int32_t *x, int32_t *y)
{
    int32_t part = building_get(building_id)->subtype;
    return set_access_tile(MONUMENT_ACCESS_GRAND_ROYAL_TOMB, part, OUTSIDE_TABLE_UNCHANGED, building_id, x, y);
}

bool sun_temple_get_access_tile(int32_t building_id, int32_t *x, int32_t *y)
{
    const building *b = building_get(building_id);
    int32_t part = b->subtype;
    int32_t kind = sun_temple_part_descs[part].kind;
    if (kind == SUN_TEMPLE_PART_SIDE_ROW) {
        int32_t inner_side = (part % 2 == 0) ? 1 : -1;
        *x = b->x + inner_side;
        *y = b->y;
        return 1;
    }
    return set_access_tile(MONUMENT_ACCESS_SUN_TEMPLE, kind, OUTSIDE_TABLE_OWN_TILE, building_id, x, y);
}
