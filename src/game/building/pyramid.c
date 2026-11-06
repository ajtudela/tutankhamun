// The pyramid family (all five styles, the mastaba included): the scenario's pyramid style (a true pyramid when
// there is none) and whether a part still needs its ramp.

#include "building/pyramid.h"
#include "building/building.h"
#include "building/monument.h"
#include "building/monument_data.h"
#include "window/window_data.h"
#include <stdint.h>

int32_t scenario_get_pyramid_style(void)
{
    for (int32_t i = 0; i < MONUMENT_SLOTS; i++) {
        int32_t def_id = scenario_data.monument_def_ids[i];
        if (def_id != 0 && monument_defs[def_id].kind == MONUMENT_DEF_KIND_PYRAMID_FAMILY) {
            return monument_defs[def_id].subkind;
        }
    }
    return PYRAMID_STYLE_PYRAMID;
}

static int32_t part_height(const building *part)
{
    return part->data.pyramid.blocks + (part->data.pyramid.layer * PYRAMID_BLOCKS_PER_LAYER);
}

int32_t pyramid_part_needs_ramp(int32_t building_id)
{
    if (scenario_pyramid_style == PYRAMID_STYLE_MASTABA) {
        return PYRAMID_RAMP_NONE;
    }
    building *part = building_get(building_id);
    building *main = building_get(building_id - part->subtype);
    if (part->data.pyramid.stage == PYRAMID_STAGE_CASING && main->data.pyramid.stage == PYRAMID_STAGE_CASING) {
        return PYRAMID_RAMP_NONE;
    }
    int32_t size = part->data.pyramid.size_variant;
    const pyramid_ramp *ramp = pyramid_ramp_table[size];
    for (int32_t i = 0; i < pyramid_ramp_count[size]; i++, ramp++) {
        if (ramp->part != part->subtype) {
            continue;
        }
        int32_t height = part_height(part);
        if (ramp->ramp1_height > 0 && ramp->ramp1_height <= height && part->data.pyramid.ramp1_height == 0) {
            return PYRAMID_RAMP_FIRST;
        }
        if (ramp->ramp2_height < 1 || height < ramp->ramp2_height || part->data.pyramid.ramp2_height != 0) {
            return PYRAMID_RAMP_NONE;
        }
        return PYRAMID_RAMP_SECOND;
    }
    return PYRAMID_RAMP_NONE;
}
