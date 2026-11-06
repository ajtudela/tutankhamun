// The city drawn without an overlay: the footprint pass (ambient sounds near the view's centre, the water
// animation, every tile's footprint, early tops next to pyramid ramps and tall figures) and the top pass
// (building tops, monument parts and work spots, figures, animations, workshop stock).
//
// Two things the passes do that look accidental:
// - the figure loops take their row and column back from city_view after drawing a figure;
// - the third gatehouse type draws its temple complex annex.

#include "widget/city_without_overlay.h"
#include "building/abu_simbel.h"
#include "building/animation_offset.h"
#include "building/building.h"
#include "building/building_data.h"
#include "building/building_image_group.h"
#include "building/building_industry.h"
#include "building/building_type.h"
#include "building/monument_work_spots.h"
#include "building/obelisk.h"
#include "building/pyramid_images.h"
#include "building/royal_tomb.h"
#include "building/sphinx.h"
#include "building/sun_temple.h"
#include "building/temple_complex.h"
#include "figure/figure.h"
#include "figure/figure_action.h"
#include "figure/figure_type.h"
#include "game/game_data.h"
#include "graphics/image_blit.h"
#include "graphics/image_collection.h"
#include "graphics/image_draw.h"
#include "map/farm.h"
#include "map/grid.h"
#include "map/terrain.h"
#include "map/terrain_image_group.h"
#include "map/tiles.h"
#include "sound/ambient.h"
#include "sound/city_play.h"
#include "sound/city_view.h"
#include "widget/city_building_anims.h"
#include "widget/city_building_sprites.h"
#include "widget/city_figure.h"
#include "widget/city_view.h"
#include "widget/city_view_rotate.h"
#include "widget/city_with_overlay.h"
#include "widget/construction_ghost.h"
#include "widget/widget_data.h"
#include "widget/widget_image_group.h"
#include "window/window_data.h"
#include <stdint.h>

enum city_walk {
    WALK_ROWS_BEFORE = 8,
    WALK_COLS_BEFORE = 4,
    WALK_COLS_EXTRA = 6,
    WALK_FOOTPRINT_ROWS_EXTRA = 14,
    WALK_TOP_HALF_TILES = 9,
    WALK_TILE_GAP_X = 2,
    WALK_TOP_ROWS_BELOW = 4,
    WALK_PYRAMID_ROWS_BELOW = 32,
    WALK_ABU_SIMBEL_ROWS_BELOW = 80,
    AMBIENT_CENTRE_REACH = 3,
};

enum city_draw_tile {
    FOOTPRINT_SIZE_MASK = 0xF,
    FOOTPRINT_LARGEST_SIZE = 5,
    BITFIELD_TERRAIN_OVERLAY = 0x10,
    PYRAMID_RAMP_HIGHEST = 3,
    TOMB_GROUND_RAISED_FIRST = 48,
    TOMB_GROUND_RAISED_END = 72,
    TOMB_GROUND_RAISE = 150,
    HOUSE_SICK_FLAG_A = 1,
    HOUSE_SICK_FLAG_B = 2,
};

enum city_draw_water {
    WATER_SHALLOW_STEP = 1,
    WATER_SHALLOW_SPAN = 6,
    WATER_DEEP_STEP = 15,
    WATER_DEEP_SPAN = 90,
    WATER_FRAMES = 6,
};

enum fort_flag {
    FORT_FLAG_SUBTYPE_A = 11,
    FORT_FLAG_SUBTYPE_B = 12,
    FORT_FLAG_SUBTYPE_C = 13,
    FORT_FLAG_IMAGE_A = 2,
    FORT_FLAG_IMAGE_B = 4,
    FORT_FLAG_IMAGE_C = 3,
};

enum monument_spot_part {
    ABU_SIMBEL_FRONT_PART = 0,
    ABU_SIMBEL_BACK_PART = 1,
    SPHINX_FRONT_PART_A = 0,
    SPHINX_FRONT_PART_B = 3,
    SPHINX_BACK_PART_A = 2,
    SPHINX_BACK_PART_B = 5,
    LIBRARY_PART_NORTH = 6,
    LIBRARY_PART_EAST = 5,
    LIBRARY_PART_SOUTH = 7,
    LIBRARY_PART_WEST = 8,
    MONUMENT_MAIN_PART = 0,
};

static int16_t image_frames(int32_t image_id)
{
    return image_record_animation_frames(image_id < image_collection.image_count ? image_id : 0);
}

static int32_t row_start_x(int32_t odd_row)
{
    int32_t x = city_view.origin_x;
    if (odd_row) {
        x = (city_view.origin_x - city_view.tile_half_width) - 1;
    }
    return x - ((WALK_TILE_GAP_X + city_view.tile_width) * WALK_COLS_BEFORE);
}

static int32_t pyramid_ramp_shown(int32_t building_id)
{
    const building *b = building_get(building_id);
    return b->type == BUILDING_PYRAMID && b->data.pyramid.blocks != 0 && b->data.pyramid.ramp_image_id != 0 &&
        b->data.pyramid.ramp1_height <= PYRAMID_RAMP_HIGHEST;
}

static int32_t figure_is_tall(const figure *f)
{
    switch (f->type) {
        case FIGURE_CHARIOTEER:
        case FIGURE_ENEMY_CHARIOT:
        case FIGURE_ENEMY_BARBARIAN_HEAVY:
        case FIGURE_ENEMY_EGYPTIAN_CHARIOT:
        case FIGURE_KINGDOM_SOLDIER:
            return 1;
        default:
            return 0;
    }
}

static int32_t tile_covers_top(int32_t grid_offset)
{
    if (grid_offset >= VIEW_LOOKUP_EMPTY) {
        return 0;
    }
    if (pyramid_ramp_shown(map_grids.building_ids[grid_offset])) {
        return 1;
    }
    for (int32_t figure_id = map_grids.figure_ids[grid_offset]; figure_id > 0;) {
        const figure *f = figure_get(figure_id);
        if (f->is_ghost == 0 && figure_is_tall(f)) {
            return 1;
        }
        figure_id = f->next_figure_id_on_same_tile;
    }
    return 0;
}

static int32_t neighbours_cover_top(int32_t odd_row)
{
    int32_t index = (city_view.row * VIEW_COLS) + city_view.col;
    int32_t first = odd_row ? city_view.lookup[index - VIEW_COLS - 1] : city_view.lookup[index - VIEW_COLS];
    int32_t second = odd_row ? city_view.lookup[index - VIEW_COLS] : city_view.lookup[index - VIEW_COLS + 1];
    return tile_covers_top(first) || tile_covers_top(second);
}

static void score_terrain_ambient(uint32_t terrain)
{
    int32_t stream;
    if ((terrain & TERRAIN_WATER) != 0 && (terrain & TERRAIN_DIKE) == 0) {
        stream = AMBIENT_WATER;
    } else if ((terrain & TERRAIN_ROCK) != 0) {
        stream = AMBIENT_ROCK;
    } else if ((terrain & TERRAIN_MEADOW) != 0) {
        stream = AMBIENT_GRASSLAND;
    } else if ((terrain & (TERRAIN_FLOODPLAIN | TERRAIN_MARSH)) != 0) {
        stream = AMBIENT_FLOODPLAIN;
    } else if ((terrain & (TERRAIN_TREE | TERRAIN_SHRUB)) != 0) {
        stream = AMBIENT_VEGETATION;
    } else if ((terrain & TERRAIN_GROUNDWATER) != 0) {
        stream = AMBIENT_GRASSLAND;
    } else {
        stream = AMBIENT_DESERT;
    }
    ambient_scores[stream]++;
}

static void score_building_ambient(int32_t type)
{
    int32_t category = building_type_sound_category(type);
    if (category == AMBIENT_HOUSING || category == AMBIENT_INDUSTRIAL || category == AMBIENT_MONUMENT) {
        ambient_scores[category]++;
    }
}

static int32_t advance_water_image(int32_t image_id, uint32_t terrain, int32_t sweep_passed)
{
    if ((terrain & TERRAIN_WATER) == 0 || city_view.advance_water == 0) {
        return image_id;
    }
    int32_t first;
    int32_t step;
    int32_t span;
    if ((terrain & TERRAIN_DEEPWATER) == 0) {
        first = image_group(IMAGE_GROUP_WATER_SHALLOW);
        step = WATER_SHALLOW_STEP;
        span = WATER_SHALLOW_SPAN;
    } else {
        first = sweep_passed ? image_group(IMAGE_GROUP_WATER_DEEP_SWEPT) : image_group(IMAGE_GROUP_WATER_DEEP);
        step = WATER_DEEP_STEP;
        span = WATER_DEEP_SPAN;
    }
    int32_t last = first - 1 + (step * WATER_FRAMES);
    if (image_id > last || image_id < first) {
        return image_id;
    }
    int32_t next = image_id + step;
    if (next > last) {
        next -= span;
    }
    g_last_image_id = next;
    map_grids.images[city_view.grid_offset] = next;
    return next;
}

static int32_t is_raised_tomb_ground(int32_t image_id)
{
    return image_id >= image_group(IMAGE_GROUP_ROYAL_TOMB_GROUND) + TOMB_GROUND_RAISED_FIRST && image_id < image_group(IMAGE_GROUP_ROYAL_TOMB_GROUND) + TOMB_GROUND_RAISED_END;
}

static void draw_footprint_tile(int32_t x, int32_t y, int32_t odd_row, int32_t near_centre, int32_t sweep_passed)
{
    int32_t building_id = map_grids.building_ids[city_view.grid_offset];
    g_last_image_id = map_grids.images[city_view.grid_offset];
    uint32_t terrain = map_grids.terrain[city_view.grid_offset];
    uint8_t bitfields = map_grids.bitfields[city_view.grid_offset];
    if (building_id == 0) {
        if (near_centre) {
            score_terrain_ambient(terrain);
        }
    } else {
        const building *b = building_get(building_id);
        if (b->is_deleted != 0) {
            city_view.color_mask = GHOST_COLOR_BLOCKED;
        }
        if (near_centre) {
            score_building_ambient(b->type);
        }
    }
    if ((terrain & TERRAIN_GARDEN) != 0) {
        building_get(0)->type = BUILDING_GARDENS;
        sound_city_mark_building_view(0, 2);
    }
    if ((bitfields & BITFIELD_TERRAIN_OVERLAY) != 0) {
        g_last_image_id = image_group(IMAGE_GROUP_TERRAIN_OVERLAY);
    }
    int32_t size = bitfields & FOOTPRINT_SIZE_MASK;
    if (size == 0) {
        int32_t image_id = advance_water_image(g_last_image_id, terrain, sweep_passed);
        if (!is_raised_tomb_ground(image_id)) {
            image_draw(image_id, x, y);
            image_id = g_last_image_id;
        }
        if ((map_grids.terrain[city_view.grid_offset] & TERRAIN_NOT_CLEAR) == 0 && neighbours_cover_top(odd_row)) {
            image_draw_isometric_top(image_id, x, y);
        }
    } else {
        int32_t dx;
        int32_t dy;
        if (city_view_footprint_offset(size, &dx, &dy)) {
            image_draw(g_last_image_id, x + dx, y - dy);
        }
    }
    city_view.color_mask = 0;
}

static void draw_empty_view_tile(int32_t x, int32_t y)
{
    if (g_scenario_climate == 1) {
        g_last_image_id = 0;
    } else {
        g_last_image_id = city_view.grid_offset - VIEW_LOOKUP_EMPTY;
    }
    image_draw(g_last_image_id, x, y);
}

void city_without_overlay_draw_footprint(void)
{
    sound_ambient_clear_scores();
    int32_t centre_col = (city_view.width_tiles + WALK_COLS_EXTRA) / 2;
    int32_t centre_row = (city_view.height_tiles + WALK_FOOTPRINT_ROWS_EXTRA) / 2;
    int32_t y = city_view.origin_y - (city_view.tile_half_height * WALK_TOP_HALF_TILES);
    int32_t odd_row = 0;
    int32_t view_row_index = city_view.camera_row - WALK_ROWS_BEFORE;
    city_view.row = view_row_index;
    for (int32_t row = 0; row < city_view.height_tiles + WALK_FOOTPRINT_ROWS_EXTRA; row++) {
        int32_t x = row_start_x(odd_row);
        int32_t row_odd = odd_row;
        odd_row = !odd_row;
        city_view.col = city_view.camera_col - WALK_COLS_BEFORE;
        city_view.row = view_row_index;
        for (int32_t col = 0; col <= city_view.width_tiles + WALK_COLS_EXTRA; col++) {
            if (city_view.col >= 0 && city_view.col < VIEW_COLS && view_row_index >= 0 && view_row_index < VIEW_ROWS) {
                city_view.grid_offset = city_view.lookup[(view_row_index * VIEW_COLS) + city_view.col];
                int32_t sweep_passed = (uint8_t) map_sweep_has_passed(city_view.col);
                if (city_view.grid_offset == construction_start_grid_offset) {
                    city_view.selected_pixel_x = y;
                    city_view.selected_pixel_y = x;
                }
                if (city_view.grid_offset < VIEW_LOOKUP_EMPTY) {
                    if ((map_grids.edge[city_view.grid_offset] & MAP_EDGE_DRAW_TILE) != 0) {
                        int32_t near_centre = col > centre_col - AMBIENT_CENTRE_REACH && col < centre_col + AMBIENT_CENTRE_REACH &&
                            row > centre_row - AMBIENT_CENTRE_REACH && row < centre_row + AMBIENT_CENTRE_REACH;
                        draw_footprint_tile(x, y, row_odd, near_centre, sweep_passed);
                    }
                } else {
                    draw_empty_view_tile(x, y);
                }
                view_row_index = city_view.row;
            }
            city_view.col++;
            x += WALK_TILE_GAP_X + city_view.tile_width;
        }
        y += city_view.tile_half_height;
        view_row_index++;
        city_view.row = view_row_index;
    }
}

static int32_t view_is(int32_t first, int32_t second)
{
    return map_info.orientation == first || map_info.orientation == second;
}

static void draw_abu_simbel_top(int32_t building_id, int32_t size, int32_t odd_row, int32_t x, int32_t y)
{
    int32_t top_x = x;
    int32_t top_y = y;
    int32_t draw_top;
    if (size == 0) {
        draw_top = (map_grids.terrain[city_view.grid_offset] & TERRAIN_NOT_CLEAR) != 0 || !neighbours_cover_top(odd_row);
    } else {
        int32_t dx;
        int32_t dy;
        draw_top = city_view_footprint_offset(size, &dx, &dy);
        top_x = x + dx;
        top_y = y - dy;
    }
    if (draw_top) {
        image_draw_isometric_top(g_last_image_id, top_x, top_y);
    }
    int32_t part = building_get(building_id)->subtype;
    if ((view_is(VIEW_ORIENTATION_NORTH, VIEW_ORIENTATION_SOUTH) && part == ABU_SIMBEL_FRONT_PART) ||
        (view_is(VIEW_ORIENTATION_EAST, VIEW_ORIENTATION_WEST) && part == ABU_SIMBEL_BACK_PART)) {
        abu_simbel_draw_built_parts(&abu_simbel_object, building_id, x, y);
        abu_simbel_draw_spots(x, y);
    }
}

static void draw_house_sick_flags(const building *b, int32_t x, int32_t y)
{
    const building_anim_offset *offset = &building_anim_offsets[BUILDING_ANIM_HOUSE_SICK_FLAG];
    if (b->house_size != 0) {
        if (b->data.house.is_sick == HOUSE_SICK_FLAG_A) {
            image_draw(image_group(IMAGE_GROUP_FARM_FLAG), x + offset->dx, y + offset->dy);
        }
        if (b->house_size != 0 && b->data.house.is_sick == HOUSE_SICK_FLAG_B) {
            image_draw(image_group(IMAGE_GROUP_FARM_FLAG_LATE), x + offset->dx, y + offset->dy);
        }
    }
}

static void draw_building_top_image(int32_t building_id, int32_t size, int32_t odd_row, int32_t x, int32_t y)
{
    if (size == 0) {
        if ((map_grids.terrain[city_view.grid_offset] & TERRAIN_NOT_CLEAR) != 0 || !neighbours_cover_top(odd_row)) {
            image_draw_isometric_top(g_last_image_id, x, y);
        }
    } else if (size == 2) {
        image_draw_isometric_top(g_last_image_id, city_view.tile_width + 2 + x, y - city_view.tile_height);
        if (building_is_farm(building_id) != 0) {
            map_building_tiles_add_farm(building_id, city_view.tile_width + 2 + x, y - city_view.tile_height, 1);
        }
    } else {
        int32_t dx;
        int32_t dy;
        if (city_view_footprint_offset(size, &dx, &dy)) {
            image_draw_isometric_top(g_last_image_id, x + dx, y - dy);
        }
    }
    draw_house_sick_flags(building_get(building_id), x, y);
}

static int32_t sphinx_spots_shown(int32_t part)
{
    if (view_is(VIEW_ORIENTATION_SOUTH, VIEW_ORIENTATION_EAST)) {
        return part == SPHINX_FRONT_PART_A || part == SPHINX_FRONT_PART_B;
    }
    if (view_is(VIEW_ORIENTATION_NORTH, VIEW_ORIENTATION_WEST)) {
        return part == SPHINX_BACK_PART_A || part == SPHINX_BACK_PART_B;
    }
    return 0;
}

static int32_t library_spots_shown(int32_t part)
{
    switch (map_info.orientation) {
        case VIEW_ORIENTATION_NORTH:
            return part == LIBRARY_PART_NORTH;
        case VIEW_ORIENTATION_EAST:
            return part == LIBRARY_PART_EAST;
        case VIEW_ORIENTATION_SOUTH:
            return part == LIBRARY_PART_SOUTH;
        case VIEW_ORIENTATION_WEST:
            return part == LIBRARY_PART_WEST;
        default:
            return 0;
    }
}

static int32_t is_royal_tomb(int32_t type)
{
    return type == BUILDING_SMALL_ROYAL_TOMB || type == BUILDING_MEDIUM_ROYAL_TOMB || type == BUILDING_GRAND_ROYAL_TOMB || type == BUILDING_LARGE_ROYAL_TOMB;
}

static void draw_monument_spots(int32_t building_id, int32_t x, int32_t y)
{
    const building *b = building_get(building_id);
    if (b->type == BUILDING_SPHINX && sphinx_spots_shown(b->subtype)) {
        sphinx_draw_spots(x, y);
    }
    if (b->type == BUILDING_SMALL_OBELISK || b->type == BUILDING_LARGE_OBELISK) {
        obelisk_draw_overlay(building_id, x, y);
    }
    if (b->type == BUILDING_SUN_TEMPLE && b->subtype == MONUMENT_MAIN_PART) {
        sun_temple_draw_stage_blocks(building_id, x, y);
        sun_temple_draw_spots(x, y);
    }
    if (b->type == BUILDING_ALEXANDRIA_LIBRARY && library_spots_shown(b->subtype)) {
        library_draw_spots(x, y);
    }
    if (b->type == BUILDING_CAESAREUM && b->subtype == MONUMENT_MAIN_PART) {
        caesareum_draw_spots(x, y);
    }
    if (is_royal_tomb(b->type) && b->data.royal_tomb.can_take_workers != 0 &&
        figure_get(b->data.royal_tomb.worker_figure_id)->direction == DIRECTION_AT_DESTINATION) {
        royal_tomb_draw_spots(x, y, building_id);
    }
    if (b->type == BUILDING_LIGHTHOUSE && b->subtype == MONUMENT_MAIN_PART) {
        lighthouse_draw_spots(x, y);
    }
}

static int32_t draw_building_top(int32_t odd_row, int32_t x, int32_t y, int32_t rows_below)
{
    int32_t building_id = map_grids.building_ids[city_view.grid_offset];
    const building *b = building_get(building_id);
    if (building_id != 0 && b->is_deleted != 0) {
        city_view.color_mask = GHOST_COLOR_BLOCKED;
    }
    uint8_t bitfields = map_grids.bitfields[city_view.grid_offset];
    g_last_image_id = map_grids.images[city_view.grid_offset];
    int32_t size = bitfields & FOOTPRINT_SIZE_MASK;
    int32_t type = b->type;
    if (type == BUILDING_PYRAMID) {
        rows_below = WALK_PYRAMID_ROWS_BELOW;
        pyramid_part_draw(g_last_image_id, x, y, building_id);
    } else if (type == BUILDING_ABU_SIMBEL) {
        rows_below = WALK_ABU_SIMBEL_ROWS_BELOW;
        draw_abu_simbel_top(building_id, size, odd_row, x, y);
    } else if (is_raised_tomb_ground(g_last_image_id)) {
        image_draw(g_last_image_id, x, y - TOMB_GROUND_RAISE);
        image_draw_isometric_top(g_last_image_id, x, y - TOMB_GROUND_RAISE);
    } else {
        draw_building_top_image(building_id, size, odd_row, x, y);
    }
    draw_monument_spots(building_id, x, y);
    city_view.color_mask = 0;
    return rows_below;
}

static int32_t top_shown(int32_t grid_offset)
{
    if ((city_overlay != OVERLAY_HIDE_CLIFFS && overlay_show_all == 0) ||
        (map_grids.terrain[grid_offset] & (TERRAIN_ROCK | TERRAIN_ROCK_NEAR_MONUMENT)) != (TERRAIN_ROCK | TERRAIN_ROCK_NEAR_MONUMENT)) {
        return (map_grids.edge[grid_offset] & MAP_EDGE_DRAW_TILE) != 0;
    }
    return 0;
}

static int32_t draw_building_tops(int32_t odd_row, int32_t y, int32_t rows_below)
{
    int32_t x = row_start_x(odd_row);
    city_view.col = city_view.camera_col - WALK_COLS_BEFORE;
    for (int32_t col = 0; col <= city_view.width_tiles + WALK_COLS_EXTRA; col++) {
        if (city_view.col >= 0 && city_view.col < VIEW_COLS) {
            city_view.grid_offset = city_view.lookup[(city_view.row * VIEW_COLS) + city_view.col];
            if (city_view.grid_offset < VIEW_LOOKUP_EMPTY && top_shown(city_view.grid_offset)) {
                rows_below = draw_building_top(odd_row, x, y, rows_below);
            }
        }
        city_view.col++;
        x += WALK_TILE_GAP_X + city_view.tile_width;
    }
    return rows_below;
}

static void draw_figures_not_clear(int32_t odd_row, int32_t y, int32_t selected_figure_id)
{
    int32_t x = row_start_x(odd_row);
    int32_t row = city_view.row;
    int32_t col = city_view.camera_col - WALK_COLS_BEFORE;
    for (int32_t col_number = 0; col_number <= city_view.width_tiles + WALK_COLS_EXTRA; col_number++) {
        if (col >= 0 && col < VIEW_COLS) {
            city_view.grid_offset = city_view.lookup[(row * VIEW_COLS) + col];
            if (city_view.grid_offset < VIEW_LOOKUP_EMPTY && (map_grids.terrain[city_view.grid_offset] & TERRAIN_NOT_CLEAR) != 0) {
                int32_t figure_id = map_grids.figure_ids[city_view.grid_offset];
                city_view.col = col;
                while (figure_id > 0) {
                    const figure *f = figure_get(figure_id);
                    if (f->is_ghost == 0 && f->pyramid_walk_mode == 0) {
                        city_draw_figure(figure_id, x, y, selected_figure_id);
                        row = city_view.row;
                        col = city_view.col;
                    }
                    figure_id = f->next_figure_id_on_same_tile;
                }
            }
        }
        col++;
        x += WALK_TILE_GAP_X + city_view.tile_width;
    }
}

static void draw_figures_clear(int32_t odd_row, int32_t y, int32_t selected_figure_id)
{
    int32_t x = row_start_x(odd_row);
    int32_t row = city_view.row;
    int32_t col = city_view.camera_col - WALK_COLS_BEFORE;
    city_view.col = col;
    for (int32_t col_number = 0; col_number <= city_view.width_tiles + WALK_COLS_EXTRA; col_number++) {
        if (col >= 0 && col < VIEW_COLS) {
            city_view.grid_offset = city_view.lookup[(row * VIEW_COLS) + col];
            if (city_view.grid_offset < VIEW_LOOKUP_EMPTY && (map_grids.terrain[city_view.grid_offset] & TERRAIN_NOT_CLEAR) == 0) {
                int32_t figure_id = map_grids.figure_ids[city_view.grid_offset];
                city_view.col = col;
                while (figure_id > 0) {
                    const figure *f = figure_get(figure_id);
                    if (f->is_ghost == 0 && f->pyramid_walk_mode == 0) {
                        city_draw_figure(figure_id, x, y, selected_figure_id);
                        col = city_view.col;
                        row = city_view.row;
                    }
                    figure_id = f->next_figure_id_on_same_tile;
                }
            }
        }
        x += WALK_TILE_GAP_X + city_view.tile_width;
        col++;
        city_view.col = col;
    }
}

static void draw_fort_flag(int32_t subtype, int32_t x, int32_t y)
{
    const building_anim_offset *offset = &building_anim_offsets[BUILDING_ANIM_FORT_FLAG];
    if (subtype == FORT_FLAG_SUBTYPE_C) {
        image_draw(image_group(IMAGE_GROUP_FORT) + FORT_FLAG_IMAGE_C, x + offset->dx, y + offset->dy);
    } else if (subtype == FORT_FLAG_SUBTYPE_B) {
        image_draw(image_group(IMAGE_GROUP_FORT) + FORT_FLAG_IMAGE_B, x + offset->dx, y + offset->dy);
    } else if (subtype == FORT_FLAG_SUBTYPE_A) {
        image_draw(image_group(IMAGE_GROUP_FORT) + FORT_FLAG_IMAGE_A, x + offset->dx, y + offset->dy);
    }
}

static int32_t is_gatehouse(int32_t type)
{
    return type == BUILDING_MUD_GATEHOUSE || type == BUILDING_BRICK_GATEHOUSE || type == BUILDING_GATEHOUSE || type == BUILDING_MUD_GATEHOUSE_UP ||
        type == BUILDING_BRICK_GATEHOUSE_UP;
}

static void draw_still_building(int32_t x, int32_t y)
{
    int32_t building_id = map_grids.building_ids[city_view.grid_offset];
    const building *b = building_get(building_id);
    if (b->type == BUILDING_FORT) {
        if ((map_grids.edge[city_view.grid_offset] & MAP_EDGE_DRAW_TILE) != 0) {
            draw_fort_flag(b->subtype, x, y);
        }
    } else if (!is_gatehouse(b->type)) {
        if (b->type == BUILDING_GATEHOUSE_UP) {
            if ((map_grids.edge[city_view.grid_offset] & MAP_EDGE_DRAW_TILE) != 0) {
                temple_complex_draw_annex(building_id);
            }
        } else if (b->type == BUILDING_GRANARY || b->type == BUILDING_GRANARY_UP) {
            draw_granary_stores(building_id, city_view.grid_offset, x, y);
        }
    }
}

static void draw_animated_building(int32_t image_id, int32_t x, int32_t y)
{
    int32_t building_id = map_grids.building_ids[city_view.grid_offset];
    const building *b = building_get(building_id);
    if (building_id != 0 && b->is_deleted != 0) {
        city_view.color_mask = GHOST_COLOR_BLOCKED;
    }
    if (b->type == BUILDING_GRANARY || b->type == BUILDING_GRANARY_UP) {
        draw_granary_stores(building_id, city_view.grid_offset, x, y);
    }
    if (b->type == BUILDING_STORAGE_YARD || b->type == BUILDING_STORAGE_YARD_UP) {
        building_draw_storage_yard_roof(image_group(IMAGE_GROUP_STORAGE_YARD) + STORAGE_YARD_ROOF_IMAGE, x, y);
    }
    int32_t frame = building_animation_offset(image_id, city_view.grid_offset);
    if (frame >= 1) {
        if (image_frames(image_id) < frame) {
            frame = image_frames(image_id);
        }
        int32_t size = map_grids.bitfields[city_view.grid_offset] & FOOTPRINT_SIZE_MASK;
        if (building_is_floodplain_farm(building_id) == 0) {
            building_draw_type_anim(b, image_id, frame, size, x, y);
        }
    }
    city_view.color_mask = 0;
}

static void draw_animations(int32_t odd_row, int32_t y)
{
    int32_t x = row_start_x(odd_row);
    city_view.col = city_view.camera_col - WALK_COLS_BEFORE;
    for (int32_t col = 0; col <= city_view.width_tiles + WALK_COLS_EXTRA; col++, city_view.col++, x += city_view.tile_width + WALK_TILE_GAP_X) {
        if (city_view.col < 0 || city_view.col >= VIEW_COLS) {
            continue;
        }
        city_view.grid_offset = city_view.lookup[(city_view.row * VIEW_COLS) + city_view.col];
        if (city_view.grid_offset >= VIEW_LOOKUP_EMPTY) {
            continue;
        }
        if ((map_grids.terrain[city_view.grid_offset] & (TERRAIN_WATER | TERRAIN_ROAD)) == (TERRAIN_WATER | TERRAIN_ROAD) && map_grids.sprite[city_view.grid_offset] != 0) {
            image_draw((map_grids.sprite[city_view.grid_offset] - 1) + image_group(IMAGE_GROUP_BRIDGE), x, y - city_view.tile_height);
            continue;
        }
        int32_t image_id = map_grids.images[city_view.grid_offset];
        if (image_frames(image_id) == 0) {
            draw_still_building(x, y);
        } else if ((map_grids.edge[city_view.grid_offset] & MAP_EDGE_DRAW_TILE) == 0) {
            continue;
        } else {
            draw_animated_building(image_id, x, y);
        }
        if ((map_grids.edge[city_view.grid_offset] & MAP_EDGE_DRAW_TILE) != 0) {
            draw_workshop_raw_material_storage(map_grids.building_ids[city_view.grid_offset], x, y);
        }
    }
}

void city_without_overlay_draw_top(int32_t selected_figure_id)
{
    int32_t y = city_view.origin_y - (city_view.tile_half_height * WALK_TOP_HALF_TILES);
    int32_t rows_below = WALK_TOP_ROWS_BELOW;
    int32_t odd_row = 0;
    city_view.row = city_view.camera_row - WALK_ROWS_BEFORE;
    for (int32_t row = 0; row < city_view.height_tiles + WALK_FOOTPRINT_ROWS_EXTRA + rows_below; row++) {
        if (city_view.row >= 0 && city_view.row < VIEW_ROWS) {
            rows_below = draw_building_tops(odd_row, y, rows_below);
            draw_figures_not_clear(odd_row, y, selected_figure_id);
            draw_animations(odd_row, y);
            draw_figures_clear(odd_row, y, selected_figure_id);
        }
        odd_row = !odd_row;
        y += city_view.tile_half_height;
        city_view.row++;
    }
}
