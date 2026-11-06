// Monument checks of the build tool: the mausoleum and sun temple sites, whether the city has a finished sun
// temple, and the preset parts of a pyramid with the ground of its complex temple.

#include "building/monument_checks.h"
#include "building/building.h"
#include "building/building_data.h"
#include "building/building_type.h"
#include "building/mausoleum.h"
#include "building/monument.h"
#include "building/monument_object.h"
#include "building/pyramid_images.h"
#include "building/sun_temple.h"
#include "map/building_tiles.h"
#include "map/grid.h"
#include "map/map_data.h"
#include "map/terrain.h"
#include <stdbool.h>
#include <stdint.h>

static int32_t part_square_free(int32_t x, int32_t y, int32_t size)
{
    map_tile_cursor_offset = map_info.grid_start + y * GRID_SIZE + x;
    for (int32_t row = 0; row < size; row++) {
        for (int32_t column = 0; column < size; column++) {
            if (map_grid_is_inside_size(map_info.diamond_x_min + column + x, map_info.diamond_y_min + row + y, 3)) {
                return 0;
            }
            if (map_grids.terrain[building_square_tile_offset(row, column) + map_tile_cursor_offset] & TERRAIN_BLOCKS_MONUMENT_SITE) {
                return 0;
            }
        }
    }
    return 1;
}

bool mausoleum_can_place(int32_t grid_offset)
{
    int32_t rel = grid_offset - map_info.grid_start;
    int32_t x = rel % GRID_SIZE;
    int32_t y = rel / GRID_SIZE;
    for (int32_t part = 0; part < MAUSOLEUM_PARTS; part++) {
        monument_part_desc desc;
        mausoleum_get_part_desc(&desc, part, map_info.orientation, map_info.orientation);
        if (!part_square_free(desc.dx + x, desc.dy + y, desc.size)) {
            return 0;
        }
    }
    return 1;
}

bool sun_temple_tile_is_free(int32_t grid_offset)
{
    int32_t rel = grid_offset - map_info.grid_start;
    int32_t x = rel % GRID_SIZE;
    int32_t y = rel / GRID_SIZE;
    for (int32_t part = 0; part < SUN_TEMPLE_PARTS; part++) {
        const monument_part_desc *desc = &sun_temple_part_descs[part];
        if (!part_square_free(desc->dx + x, desc->dy + y, desc->size)) {
            return 0;
        }
    }
    return 1;
}

bool sun_temple_any_completed(void)
{
    for (int32_t id = 1; id < MAX_BUILDINGS; id++) {
        if (city_buildings.records[id].state == BUILDING_STATE_IN_USE && city_buildings.records[id].type == BUILDING_SUN_TEMPLE) {
            if (sun_temple_get_progress(id, 0) == MONUMENT_PROGRESS_COMPLETE) {
                return 1;
            }
            id += SUN_TEMPLE_PARTS - 1;
        }
    }
    return 0;
}

void pyramid_init_preset_parts(int32_t building_id)
{
    int32_t size = city_buildings.records[building_id].data.pyramid.size_variant;
    int32_t view = map_info.orientation / 2;
    const pyramid_preset_list *lists = (scenario_pyramid_style == PYRAMID_STYLE_MASTABA) ? mastaba_preset_parts : pyramid_preset_parts.lists;
    for (int32_t i = 0; i < lists[size].count; i++) {
        int32_t part_id = lists[size].parts[i].part + building_id;
        building *part = &city_buildings.records[part_id];
        part->data.pyramid.stage = PYRAMID_STAGE_PRESET;
        part->data.pyramid.prep_mask = 0;
        pyramid_part_update_image(part_id, view, size);
    }
    if (pyramid_has_complex_temple(size)) {
        int32_t offset = pyramid_complex_temple_offset[size] + city_buildings.records[building_id].grid_offset - map_info.grid_start;
        pyramid_complex_area_op(offset % GRID_SIZE, offset / GRID_SIZE, PYRAMID_COMPLEX_AREA_MARK_GROUND, 0);
    }
}
