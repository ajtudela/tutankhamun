// Removal of special buildings: the gatehouse's tiles and its burning into rubble, the temple complex's tiles per
// orientation, the legion of a deleted fort, the obelisk, and the tile removal of the scripted monuments and royal
// tombs (rock or groundwater given back to their tiles).

#include "building/destroy_paths.h"
#include "building/abu_simbel.h"
#include "building/building_data.h"
#include "building/building_delete.h"
#include "building/building_figure_helpers.h"
#include "building/building_image_group.h"
#include "building/building_maintenance.h"
#include "building/building_type.h"
#include "building/gatehouse_place.h"
#include "building/lighthouse.h"
#include "building/monument.h"
#include "building/temple_complex.h"
#include "city/resource_type.h"
#include "core/calc.h"
#include "editor/editor_terrain_tiles.h"
#include "figure/figure.h"
#include "figure/formation.h"
#include "graphics/image_collection.h"
#include "map/building_tiles.h"
#include "map/grid.h"
#include "map/map_data.h"
#include "map/terrain.h"
#include "map/terrain_image_group.h"
#include "map/tiles.h"
#include "widget/widget_data.h"
#include <stdint.h>
#include <string.h>

static void gatehouse_part_tile(int32_t orientation, const gatehouse_part *part, int32_t x, int32_t y, int32_t *out_x, int32_t *out_y)
{
    if (orientation == GATEHOUSE_ORIENTATION_AS_RECORDED) {
        *out_x = x + part->dx;
        *out_y = y + part->dy;
    } else if (orientation == GATEHOUSE_ORIENTATION_SWAPPED) {
        *out_x = x + part->dy;
        *out_y = y + part->dx;
    } else {
        *out_x = x;
        *out_y = y;
    }
}

void building_gatehouse_remove_tiles(int32_t building_id)
{
    const building *b = building_get(building_id);
    int32_t x = b->x;
    int32_t y = b->y;
    for (int32_t i = 0; i < GATEHOUSE_PART_COUNT; i++) {
        const gatehouse_part *part = &gatehouse_parts.parts[i];
        int32_t tx;
        int32_t ty;
        gatehouse_part_tile(b->data.oriented.orientation, part, x, y, &tx, &ty);
        building_tiles_size_y = part->size;
        building_tiles_size_x = part->size;
        map_building_tiles_remove(building_id, tx, ty);
    }
}

static void start_ruin_fire(building *b)
{
    b->fire_duration = (int16_t) ((b->house_figure_generation_delay & BURNING_RUIN_FIRE_DAYS_MASK) + 1);
    b->figure_id4 = 0;
    b->fire_proof = 1;
}

static void cover_part_with_rubble(const gatehouse_part *part, int32_t x, int32_t y)
{
    for (int32_t n = 0; n < part->size; n++) {
        int32_t id = building_create(city_current_player, BUILDING_BURNING_RUIN, x + gatehouse_rubble_offsets[n * 2], y + gatehouse_rubble_offsets[(n * 2) + 1],
            0, 0);
        building *rubble = building_get(id);
        int32_t variant = map_grids.random[rubble->grid_offset] & (BURNING_RUIN_VARIANTS - 1);
        int32_t image = image_group(IMAGE_GROUP_RUINS) + (variant * BURNING_RUIN_IMAGE_STEP);
        map_building_tiles_add(id, rubble->x, rubble->y, 1, 1, image, TERRAIN_BUILDING);
        start_ruin_fire(rubble);
    }
}

void building_gatehouse_destroy_by_fire(int32_t building_id)
{
    building *b = building_get(building_id);
    undo_available = 0;
    b->fire_risk = 0;
    b->damage_risk = 0;
    b->output_resource = RESOURCE_NONE;
    b->distance_from_entry = 0;
    building_clear_related_data(building_id);
    building_gatehouse_remove_tiles(building_id);
    b->type = BUILDING_BURNING_RUIN;
    b->figure_id4 = 0;
    b->fire_duration = (int16_t) ((b->house_figure_generation_delay & BURNING_RUIN_FIRE_DAYS_MASK) + 1);
    b->fire_proof = 1;
    b->size = 1;
    int32_t rotation = b->data.oriented.orientation + map_info.orientation;
    if (rotation >= DIRECTION_COUNT) {
        rotation -= DIRECTION_COUNT;
    }
    const int32_t *parts = &gatehouse_orientation_parts[rotation * GATEHOUSE_PART_COUNT];
    for (int32_t i = 0; i < GATEHOUSE_PART_COUNT; i++) {
        const gatehouse_part *part = &gatehouse_parts.parts[parts[i]];
        int32_t x;
        int32_t y;
        gatehouse_part_tile(rotation, part, b->x, b->y, &x, &y);
        cover_part_with_rubble(part, x, y);
    }
}

static void temple_complex_tile_at(int32_t orientation, int32_t x0, int32_t y0, int32_t row, int32_t column, int32_t *x, int32_t *y)
{
    switch (orientation) {
        case 0:
            *x = x0 + column;
            *y = y0 + row;
            break;
        case 2:
            *x = x0 - row;
            *y = y0 - column;
            break;
        case 4:
            *x = x0 - column;
            *y = y0 - row;
            break;
        default:
            *x = x0 + row;
            *y = y0 + column;
            break;
    }
}

static void temple_complex_site_end(int32_t orientation, int32_t x0, int32_t y0, int32_t *x_end, int32_t *y_end)
{
    temple_complex_tile_at(orientation, x0, y0, TEMPLE_COMPLEX_SITE_ROWS, TEMPLE_COMPLEX_SITE_COLUMNS, x_end, y_end);
}

static void free_temple_complex_tile(int32_t offset, int32_t x, int32_t y)
{
    map_grids.bitfields[offset] &= DESTROYED_TILE_BITFIELD_KEEP;
    map_grids.building_ids[offset] = 0;
    map_grids.building_damage[offset] = 0;
    map_grids.edge[offset] = (uint8_t) ((map_grids.edge[offset] & DESTROYED_TILE_EDGE_KEEP) | MAP_EDGE_DRAW_TILE);
    uint32_t terrain = map_grids.terrain[offset] & ~TERRAIN_BUILDING;
    map_grids.terrain[offset] = terrain;
    map_grids.sprite[offset] = 0;
    map_grids.canals[offset] = 0;
    if ((terrain & TERRAIN_WATER) && !(terrain & TERRAIN_FLOODPLAIN)) {
        map_grids.terrain[offset] = terrain & TERRAIN_WATER;
        map_tiles_set_water_image(x, y, 1);
    } else {
        map_grids.images[offset] = (map_grids.random[offset] & (LEVELED_GROUND_IMAGES - 1)) + image_group(IMAGE_GROUP_LEVELED_GROUND);
        map_grids.terrain[offset] = terrain & (TERRAIN_KEPT_BY_BUILDING | TERRAIN_WATER);
    }
}

void building_temple_complex_remove_tiles(int32_t building_id)
{
    const building *b = building_get(building_id);
    int32_t orientation = b->data.oriented.orientation;
    int32_t half = orientation / 2;
    int32_t x0 = b->x + temple_complex_origin[half * 2];
    int32_t y0 = b->y + temple_complex_origin[(half * 2) + 1];
    int32_t x_end;
    int32_t y_end;
    temple_complex_site_end(orientation, x0, y0, &x_end, &y_end);
    const int32_t *tile = temple_complex_tiles;
    for (int32_t row = 0; row < TEMPLE_COMPLEX_SITE_ROWS; row++) {
        for (int32_t column = 0; column < TEMPLE_COMPLEX_SITE_COLUMNS; column++, tile += TEMPLE_DECOR_RECORD_INTS) {
            int32_t x;
            int32_t y;
            temple_complex_tile_at(orientation, x0, y0, row, column, &x, &y);
            int32_t offset = map_grid_offset(x, y);
            if (tile[0] == 0) {
                continue;
            }
            if (building_id != 0) {
                int32_t id = map_grids.building_ids[offset];
                if (id != building_id && building_main(id) != building_id) {
                    continue;
                }
                if (b->type != BUILDING_BURNING_RUIN) {
                    map_grids.rubble_building_type[offset] = (uint8_t) b->type;
                }
            }
            free_temple_complex_tile(offset, x, y);
        }
    }
    map_tiles_update_region_roads(x0, y0, x_end, y_end);
    map_tiles_update_region_empty_land(x0, y0, x_end, y_end, 0, 0);
    map_tiles_update_region_meadow(x0, y0, x_end, y_end);
    map_tiles_update_region_floodplain(x0, y0, x_end, y_end);
    map_tiles_update_region_water(x0, y0, x_end, y_end, 1);
    map_tiles_update_region_marshland(x0, y0, x_end, y_end);
    map_tiles_update_region_rubble(x0, y0, x_end, y_end);
    window_redraw_request = 1;
}

void formation_legion_delete_for_fort(int32_t formation_id)
{
    if (formation_id <= 0 || formation_get(formation_id)->in_use == 0) {
        return;
    }
    formation *legion = formation_get(formation_id);
    int32_t standard = legion->standard_figure_id;
    if (standard != 0) {
        figure_delete(standard);
    }
    memset(legion, 0, sizeof(formation));
    formation_calculate_legion_totals();
}

void obelisk_on_delete(int32_t building_id)
{
    monument_unregister(building_id);
}

static void remove_tiles_singly(const monument_part_desc *parts, int32_t building_id, int32_t abu_simbel)
{
    const building *b = building_get(building_id);
    const monument_part_desc *part = &parts[b->subtype];
    int32_t x = b->x;
    int32_t y0 = b->y;
    for (int32_t i = 0; i < part->size; i++, x++) {
        int32_t y = y0;
        for (int32_t j = 0; j < part->size; j++, y++) {
            building_tiles_size_x = 1;
            building_tiles_size_y = 1;
            map_building_tiles_remove(building_id, x, y);
            if (abu_simbel) {
                if (part->kind != ABU_SIMBEL_PART_KIND_ON_OPEN_GROUND) {
                    editor_tile_set_rock_kind(x, y, MONUMENT_ROCK_TERRAIN);
                }
            } else if (part->kind == LIGHTHOUSE_PART_TOWER) {
                editor_tile_set_rock_kind(x, y, TERRAIN_ROCK);
            }
        }
    }
}

void abu_simbel_remove_tiles(abu_simbel_object_t *self, int32_t building_id)
{
    remove_tiles_singly(self->parts, building_id, 1);
}

void lighthouse_remove_tiles(lighthouse_object_t *self, int32_t building_id)
{
    remove_tiles_singly(self->parts, building_id, 0);
}

void library_remove_tiles(library_object_t *self, int32_t building_id)
{
    const building *b = building_get(building_id);
    const monument_part_desc *part = &self->parts[b->subtype];
    int32_t x = b->x;
    int32_t y = b->y;
    building_tiles_size_y = part->size;
    building_tiles_size_x = part->size;
    map_building_tiles_remove(building_id, x, y);
    for (int32_t i = 0; i < part->size; i++) {
        for (int32_t j = 0; j < part->size; j++) {
            map_building_tiles_remove(building_id, x + i, y + j);
        }
    }
}

void caesareum_remove_tiles(caesareum_object_t *self, int32_t building_id)
{
    const building *b = building_get(building_id);
    const monument_part_desc *part = &self->parts[b->subtype];
    int32_t x = b->x;
    int32_t y = b->y;
    building_tiles_size_y = 1;
    building_tiles_size_x = 1;
    for (int32_t i = 0; i < part->size; i++) {
        for (int32_t j = 0; j < part->size; j++) {
            map_building_tiles_remove(building_id, x + i, y + j);
        }
    }
}

void royal_tomb_clear_tiles(tomb_object *self, int32_t building_id, const monument_part_desc *descs, int32_t count)
{
    (void) count;
    const building *b = building_get(building_id);
    int32_t part = b->subtype;
    int32_t size = descs[part].size;
    uint32_t terrain = (part != self->last_part) ? MONUMENT_ROCK_TERRAIN : TERRAIN_GROUNDWATER;
    for (int32_t i = 0; i < size; i++) {
        int32_t offset = map_grid_offset(b->x + i, b->y);
        for (int32_t j = 0; j < size; j++, offset += GRID_SIZE) {
            map_grids.building_ids[offset] = 0;
            map_grids.terrain[offset] = 0;
            map_grids.bitfields[offset] &= DESTROYED_TILE_BITFIELD_KEEP;
            map_grids.canals[offset] = 0;
            map_grids.building_damage[offset] = 0;
            map_grids.edge[offset] = (uint8_t) ((map_grids.edge[offset] & DESTROYED_TILE_EDGE_KEEP) | MAP_EDGE_DRAW_TILE);
            map_grids.sprite[offset] = 0;
            map_grids.terrain[offset] = terrain;
        }
    }
}

void small_royal_tomb_clear_tiles(tomb_object *self, int32_t building_id)
{
    royal_tomb_clear_tiles(self, building_id, royal_tomb_record_of(self)->small.descs, SMALL_TOMB_PARTS);
}

void medium_royal_tomb_clear_tiles(tomb_object *self, int32_t building_id)
{
    royal_tomb_clear_tiles(self, building_id, royal_tomb_record_of(self)->medium.descs, MEDIUM_TOMB_PARTS);
}

void large_royal_tomb_clear_tiles(tomb_object *self, int32_t building_id)
{
    royal_tomb_clear_tiles(self, building_id, royal_tomb_record_of(self)->large.descs, LARGE_TOMB_PARTS);
}

void grand_royal_tomb_clear_tiles(tomb_object *self, int32_t building_id)
{
    royal_tomb_clear_tiles(self, building_id, royal_tomb_record_of(self)->grand.descs, GRAND_TOMB_PARTS);
}
