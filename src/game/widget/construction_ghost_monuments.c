// The monuments' placement ghosts: the mausoleum and the sun temple drawn part by part green or red by the
// site check, the pyramid or mastaba with its parts, temple area and ramp, and the scripted monuments' ghost
// methods that pass their part table to the base class or to the royal tombs' ghost. Also the pyramid preview
// that follows the cursor.

#include "widget/construction_ghost_monuments.h"
#include "building/building_data.h"
#include "building/building_type.h"
#include "building/mausoleum.h"
#include "building/monument.h"
#include "building/monument_checks.h"
#include "building/monument_data.h"
#include "building/monument_object.h"
#include "building/monument_place.h"
#include "building/monument_tables.h"
#include "building/pyramid_images.h"
#include "building/pyramid_preview.h"
#include "building/royal_tomb.h"
#include "building/sun_temple.h"
#include "city/city_player.h"
#include "graphics/image_collection.h"
#include "map/grid.h"
#include "map/terrain.h"
#include "map/terrain_image_group.h"
#include "widget/city_view_rotate.h"
#include "widget/construction_ghost.h"
#include "widget/construction_ghost_royal_tomb.h"
#include "widget/widget_data.h"
#include <stdint.h>

enum monument_ghost {
    PYRAMID_MAP_EDGE = 3,
};

static void draw_part(int32_t size, int32_t x, int32_t y, int32_t half_width_plus_one, int32_t half_height, int32_t color)
{
    int32_t tiles = ghost_tiles_of_size(size);
    if (size > 1) {
        int32_t steps = size - 1;
        if (map_info.orientation == VIEW_ORIENTATION_EAST) {
            x += steps * half_width_plus_one;
            y -= steps * half_height;
        } else if (map_info.orientation == VIEW_ORIENTATION_SOUTH) {
            y += (steps * half_height) * -2;
        } else if (map_info.orientation == VIEW_ORIENTATION_WEST) {
            x += -(steps * half_width_plus_one);
            y -= steps * half_height;
        }
    }
    for (int32_t i = 0; i < tiles; i++) {
        ghost_image_draw_masked(image_group(IMAGE_GROUP_FLAT_TILE), ghost_flat_tile_dx(i) + x, ghost_flat_tile_dy(i) - city_view.tile_half_height + y, color);
    }
}

void mausoleum_draw_placement(void)
{
    int32_t color = mausoleum_can_place(map_info.hover_offset) ? GHOST_COLOR_FREE : GHOST_COLOR_BLOCKED;
    int32_t view = map_info.orientation / VIEW_ORIENTATION_STEP;
    int32_t px, py;
    city_view_tile_to_pixel(map_info.hover_x, map_info.hover_y, &px, &py);
    int32_t half_width_plus_one = city_view.tile_half_width + 1;
    int32_t half_height = city_view.tile_half_height;
    for (int32_t part = 0; part < MAUSOLEUM_PARTS; part++) {
        monument_part_desc desc;
        mausoleum_get_part_desc(&desc, part, map_info.orientation, map_info.orientation);
        draw_part(desc.size, desc.view_x[view] + px, desc.view_y[view] + city_view.tile_half_height + py, half_width_plus_one, half_height, color);
    }
}

void sun_temple_draw_placement(void)
{
    int32_t color = sun_temple_tile_is_free(map_info.hover_offset) ? GHOST_COLOR_FREE : GHOST_COLOR_BLOCKED;
    int32_t view = map_info.orientation / VIEW_ORIENTATION_STEP;
    int32_t px, py;
    city_view_tile_to_pixel(map_info.hover_x, map_info.hover_y, &px, &py);
    int32_t half_width_plus_one = city_view.tile_half_width + 1;
    int32_t half_height = city_view.tile_half_height;
    for (int32_t part = 0; part < SUN_TEMPLE_PARTS; part++) {
        const monument_part_desc *desc = &sun_temple_part_descs[part];
        draw_part(desc->size, desc->view_x[view] + px, city_view.tile_half_height + py + desc->view_y[view], half_width_plus_one, half_height, color);
    }
}

static int32_t is_mastaba(void)
{
    return scenario_pyramid_style == PYRAMID_STYLE_MASTABA;
}

static int32_t pyramid_part_total(int32_t size)
{
    return is_mastaba() ? mastaba_part_count[size] : pyramid_part_count[size];
}

static void pyramid_part_place(int32_t size, int32_t view, int32_t part, int32_t *origin, int32_t *tx, int32_t *ty)
{
    if (is_mastaba()) {
        int32_t index = pyramid_layout.mastaba_tiles[size][view][part].offset_index;
        *origin = mastaba_part_xy[index].x + (mastaba_part_xy[index].y * GRID_SIZE);
        *tx = pyramid_layout.mastaba_tiles[size][view][part].x;
        *ty = pyramid_layout.mastaba_tiles[size][view][part].y;
    } else {
        *origin = pyramid_part_grid_offsets[pyramid_layout.pyramid_tiles[size][view][part].offset_index];
        *tx = pyramid_layout.pyramid_tiles[size][view][part].x;
        *ty = pyramid_layout.pyramid_tiles[size][view][part].y;
    }
}

static void draw_pyramid_part(int32_t origin, int32_t x, int32_t y, int32_t view, int32_t tiles, int32_t broke)
{
    int32_t origin_x = origin % GRID_SIZE;
    int32_t origin_y = origin / GRID_SIZE;
    for (int32_t i = 0; i < tiles; i++) {
        uint32_t blocked = map_grids.terrain[ghost_view_tile_offset(view, i) + origin] & TERRAIN_BLOCKS_MONUMENT_SITE;
        int32_t color;
        if (map_grid_is_inside_size(origin_x, origin_y, PYRAMID_MAP_EDGE) || blocked) {
            color = GHOST_COLOR_BLOCKED;
        } else {
            color = broke ? GHOST_COLOR_BLOCKED : GHOST_COLOR_FREE;
        }
        ghost_image_draw_masked(image_group(IMAGE_GROUP_FLAT_TILE), ghost_flat_tile_dx(i) + x, ghost_flat_tile_dy(i) + y, color);
    }
}

static void draw_blocked_ramps(void)
{
    int32_t ramps, ramp_offset;
    if (is_mastaba()) {
        ramps = mastaba_ramp_count[pyramid_layout.size_being_placed];
        ramp_offset = mastaba_ramp_offset[pyramid_layout.size_being_placed];
    } else {
        ramps = pyramid_ramp_tile_count[pyramid_layout.size_being_placed];
        ramp_offset = pyramid_ramp_offset[pyramid_layout.size_being_placed];
    }
    for (int32_t i = 0; i < ramps; i++) {
        int32_t offset = map_info.hover_offset + ramp_offset + i;
        if (map_grids.terrain[offset] & TERRAIN_BLOCKS_MONUMENT_RAMP) {
            int32_t rx, ry;
            city_view_tile_to_pixel((offset % GRID_SIZE) - map_info.diamond_x_min, (offset / GRID_SIZE) - map_info.diamond_y_min, &rx, &ry);
            ghost_image_draw_masked(image_group(IMAGE_GROUP_FLAT_TILE), rx, ry, GHOST_COLOR_BLOCKED);
        }
    }
}

void pyramid_draw_construction_preview(void)
{
    int32_t broke = (city_player_current()->treasury <= GHOST_TREASURY_BROKE);
    int32_t reason;
    uint8_t fits = pyramid_can_place(map_info.hover_offset, pyramid_layout.size_being_placed, &reason);
    int32_t color = fits ? GHOST_COLOR_FREE : GHOST_COLOR_BLOCKED;
    int32_t view = map_info.orientation / VIEW_ORIENTATION_STEP;
    int32_t px, py;
    city_view_tile_to_pixel(map_info.hover_x, map_info.hover_y, &px, &py);
    int32_t tiles = ghost_tiles_of_size(PYRAMID_PART_SIZE);
    for (int32_t part = 0; part < pyramid_part_total(pyramid_layout.size_being_placed); part++) {
        int32_t origin, tx, ty;
        pyramid_part_place(pyramid_layout.size_being_placed, view, part, &origin, &tx, &ty);
        draw_pyramid_part(origin + map_info.hover_offset, (tx * (city_view.tile_width + 2)) + px, (ty * city_view.tile_height) + py, view, tiles, broke);
    }
    if (pyramid_has_complex_temple(pyramid_layout.size_being_placed)) {
        int32_t offset = pyramid_complex_temple_offset[pyramid_layout.size_being_placed] + map_info.hover_offset - map_info.grid_start;
        pyramid_complex_area_op(offset % GRID_SIZE, offset / GRID_SIZE, PYRAMID_COMPLEX_AREA_DRAW_GHOST, color);
    }
    draw_blocked_ramps();
}

void abu_simbel_draw_placement(monument_object *self)
{
    abu_simbel_object_t *abu_simbel = (abu_simbel_object_t *) self;
    monument_object_draw_placement(&abu_simbel->base, abu_simbel->parts, ABU_SIMBEL_PARTS);
}

void caesareum_draw_placement(monument_object *self)
{
    caesareum_object_t *caesareum = (caesareum_object_t *) self;
    monument_object_draw_placement(&caesareum->base, caesareum->parts, CAESAREUM_PARTS);
}

void lighthouse_draw_placement(monument_object *self)
{
    lighthouse_object_t *lighthouse = (lighthouse_object_t *) self;
    monument_object_draw_placement(&lighthouse->base, lighthouse->parts, LIGHTHOUSE_PARTS);
}

void small_royal_tomb_draw_placement(tomb_object *self)
{
    royal_tomb_draw_placement(self, royal_tomb_record_of(self)->small.descs, SMALL_TOMB_PARTS);
}

void medium_royal_tomb_draw_placement(tomb_object *self)
{
    royal_tomb_draw_placement(self, royal_tomb_record_of(self)->medium.descs, MEDIUM_TOMB_PARTS);
}

void large_royal_tomb_draw_placement(tomb_object *self)
{
    royal_tomb_draw_placement(self, royal_tomb_record_of(self)->large.descs, LARGE_TOMB_PARTS);
}

void grand_royal_tomb_draw_placement(tomb_object *self)
{
    royal_tomb_draw_placement(self, royal_tomb_record_of(self)->grand.descs, GRAND_TOMB_PARTS);
}

void pyramid_preview_update(void)
{
    if (construction_building_type == BUILDING_PYRAMID) {
        pyramid_preview_find_place();
    }
}
