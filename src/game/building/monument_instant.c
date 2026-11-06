// Monuments of earlier missions placed again at the start of a mission and completed at once: the mausoleum, the
// library and the lighthouse, marked complete and kept from an earlier mission, then their images.
//
// The mausoleum's parts and the lighthouse get main part flag 3 (elsewhere the game sets 0 or 1), and the lighthouse's
// work flags 1.

#include "building/monument_instant.h"
#include "building/building.h"
#include "building/building_data.h"
#include "building/building_type.h"
#include "building/library.h"
#include "building/lighthouse.h"
#include "building/mausoleum.h"
#include "building/monument.h"
#include "building/monument_data.h"
#include "building/monument_tables.h"
#include <stdint.h>

void mausoleum_update_all_images(int32_t building_id)
{
    const building *b = &city_buildings.records[building_id];
    if (b->type != BUILDING_MAUSOLEUM) {
        return;
    }
    int32_t main_id = building_id - b->subtype;
    for (int32_t i = 0; i < MAUSOLEUM_PARTS; i++) {
        mausoleum_update_image(main_id + i);
    }
}

void mausoleum_complete_instant(int32_t building_id)
{
    int32_t main_id = building_id - city_buildings.records[building_id].subtype;
    for (int32_t i = 0; i < MAUSOLEUM_PARTS; i++) {
        building *part = &city_buildings.records[main_id + i];
        int32_t is_block = (mausoleum_part_descs[i].kind & 0xFF) < MAUSOLEUM_BLOCK_KINDS;
        part->data.mausoleum.stage = is_block ? MAUSOLEUM_BLOCK_FULL : MAUSOLEUM_GROUND_FINISHED;
        part->data.mausoleum.work_flags = MAUSOLEUM_MAIN_FLAGS_ALL;
        part->data.mausoleum.is_main_part = 3;
        part->data.mausoleum.completion_shown = 1;
        part->created_sequence = MONUMENT_KEPT_FROM_EARLIER_MISSION;
    }
    mausoleum_update_all_images(building_id);
}

void library_set_completed(library_object_t *self, int32_t building_id)
{
    int32_t end = building_id + LIBRARY_PARTS;
    for (int32_t id = building_id; id < end; id++) {
        building *part = &city_buildings.records[id];
        part->data.monument.stage = LIBRARY_STAGE_COMPLETE;
        part->data.monument.is_main_part = 1;
        part->data.monument.completion_shown = 1;
        part->created_sequence = MONUMENT_KEPT_FROM_EARLIER_MISSION;
        self->base.vtable->update_image(&self->base, id);
    }
}

void lighthouse_complete_instant(lighthouse_object_t *self, int32_t building_id)
{
    building *part = &city_buildings.records[building_id];
    part->data.monument.stage = LIGHTHOUSE_STAGE_COMPLETE;
    part->data.monument.work_flags = MONUMENT_WORK_COMPLETE;
    part->data.monument.is_main_part = 3;
    part->data.monument.completion_shown = 1;
    part->created_sequence = MONUMENT_KEPT_FROM_EARLIER_MISSION;
    self->base.vtable->update_image(&self->base, building_id);
}
