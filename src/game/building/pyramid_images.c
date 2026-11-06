// Pyramid images and drawing: the image and ramp offsets of a part (its kind turned with the view), the area of
// the complex temple, the pixel offsets of the parts and of the figures on them, and the drawing of a part with
// its ramps and figures.

#include "building/pyramid_images.h"
#include "building/building_data.h"
#include "building/building_image_group.h"
#include "building/building_type.h"
#include "building/monument_internal.h"
#include "building/monument_spots.h"
#include "building/pyramid_level.h"
#include "building/pyramid_preview.h"
#include "figure/figure.h"
#include "game/undo_build.h"
#include "graphics/image_blit_mask.h"
#include "graphics/image_collection.h"
#include "graphics/image_draw.h"
#include "map/building_tiles.h"
#include "map/grid.h"
#include "map/terrain.h"
#include "map/terrain_image_group.h"
#include "widget/city_figure.h"
#include "widget/city_view.h"
#include "widget/city_view_rotate.h"
#include "widget/widget_data.h"

int32_t pyramid_complex_state;

int32_t pyramid_complex_steps;

int32_t pyramid_complex_step;

int32_t pyramid_complex_start_tile;

enum pyramid_image_offset {
    PYRAMID_IMAGES_PER_BLOCK_ROW = 8,
    PYRAMID_IMAGE_CASING_ROWS_END = 11,
    BENT_PYRAMID_IMAGE_CASING_ROWS_END = 5,
    PYRAMID_IMAGE_LAYER_BLOCKS = 95,
    PYRAMID_IMAGE_TOP_INNER = 109,
    PYRAMID_IMAGE_RAMPS = 109,
    PYRAMID_IMAGE_RAMP_BOTTOM_LEFT = 109,
    PYRAMID_IMAGE_RAMP_BOTTOM_RIGHT = 110,
    STEPPED_PYRAMID_IMAGE_RAMP_BOTTOM_RIGHT = 112,
    STEPPED_PYRAMID_IMAGE_RAMP_CORNER_LEFT = 115,
    STEPPED_PYRAMID_IMAGE_RAMP_CORNER_BOTTOM = 119,
    STEPPED_PYRAMID_IMAGE_RAMP_CORNER_RIGHT = 123,
};

enum pyramid_pixel_layout {
    PYRAMID_LAYER_PIXEL_HEIGHT = 90,
    PYRAMID_FIGURE_GROUND_Y = 15,
    PYRAMID_FIGURE_X = 1,
    PYRAMID_FIGURE_Y = 19,
    STEPPED_PYRAMID_FIGURE_SHIFT_X = 5,
    STEPPED_PYRAMID_FIGURE_SHIFT_Y = 8,
};

static int32_t bent_half_layer(int32_t size_variant, int32_t layer)
{
    return pyramid_layout.bent_half_layer[(size_variant * BENT_PYRAMID_LAYER_STRIDE) + layer];
}

static int32_t layer_pixel_height(int32_t size_variant, int32_t layer)
{
    if (scenario_pyramid_style == PYRAMID_STYLE_BENT) {
        return pyramid_layout.bent_pixel_heights[(size_variant * BENT_PYRAMID_LAYER_STRIDE) + layer];
    }
    return layer * PYRAMID_LAYER_PIXEL_HEIGHT;
}

static int32_t rotated_kind(int32_t kind, int32_t view)
{
    if (kind < PYRAMID_PART_EDGE_TOP_LEFT) {
        kind += view;
        if (kind > PYRAMID_PART_CORNER_RIGHT) {
            kind -= PYRAMID_VIEW_COUNT;
        }
    } else if (kind < PYRAMID_PART_INNER) {
        kind += view;
        if (kind > PYRAMID_PART_EDGE_TOP_RIGHT) {
            kind -= PYRAMID_VIEW_COUNT;
        }
    } else if (kind == PYRAMID_PART_MASTABA_MAIN) {
        kind = view + PYRAMID_PART_MASTABA_MAIN;
    }
    return kind;
}

static int16_t ramp_x(int32_t kind, int32_t row)
{
    return (int16_t) ((int16_t) pyramid_ramp_base[kind].x + (int16_t) mastaba_entry.ramp_offsets[kind][row].x);
}

static int16_t ramp_y(int32_t kind, int32_t row)
{
    return (int16_t) ((int16_t) pyramid_ramp_base[kind].y + (int16_t) mastaba_entry.ramp_offsets[kind][row].y);
}

void pyramid_part_update_image(int32_t building_id, int32_t view, int32_t size_variant)
{
    building *b = building_get(building_id);
    building_pyramid *p = &b->data.pyramid;
    p->ramp_link_building_id = 0;
    if (p->stage > PYRAMID_STAGE_CASING) {
        pyramid_part_update_level_images(building_id, size_variant);
        return;
    }
    int32_t part = b->subtype;
    int32_t kind = (scenario_pyramid_style == PYRAMID_STYLE_MASTABA) ? pyramid_layout.mastaba[size_variant][part].kind
                                                                     : pyramid_layout.pyramid[size_variant][part].kind;
    kind = rotated_kind(kind, view);
    int32_t ramp_image;
    int32_t ramp_kind = 0;
    if (scenario_pyramid_style == PYRAMID_STYLE_MASTABA) {
        ramp_image = 0;
    } else {
        ramp_kind = kind;
        if (kind == PYRAMID_PART_EDGE_BOTTOM_LEFT) {
            ramp_image = image_group(IMAGE_GROUP_PYRAMID) + PYRAMID_IMAGE_RAMP_BOTTOM_LEFT;
        } else if (kind == PYRAMID_PART_EDGE_BOTTOM_RIGHT) {
            ramp_image = (scenario_pyramid_style == PYRAMID_STYLE_STEPPED) ? image_group(IMAGE_GROUP_PYRAMID) + STEPPED_PYRAMID_IMAGE_RAMP_BOTTOM_RIGHT
                                                                           : image_group(IMAGE_GROUP_PYRAMID) + PYRAMID_IMAGE_RAMP_BOTTOM_RIGHT;
        } else if (scenario_pyramid_style == PYRAMID_STYLE_STEPPED && kind == PYRAMID_PART_CORNER_LEFT) {
            ramp_image = image_group(IMAGE_GROUP_PYRAMID) + STEPPED_PYRAMID_IMAGE_RAMP_CORNER_LEFT;
        } else if (scenario_pyramid_style == PYRAMID_STYLE_STEPPED && kind == PYRAMID_PART_CORNER_BOTTOM) {
            ramp_image = image_group(IMAGE_GROUP_PYRAMID) + STEPPED_PYRAMID_IMAGE_RAMP_CORNER_BOTTOM;
        } else if (scenario_pyramid_style == PYRAMID_STYLE_STEPPED && kind == PYRAMID_PART_CORNER_RIGHT) {
            ramp_image = image_group(IMAGE_GROUP_PYRAMID) + STEPPED_PYRAMID_IMAGE_RAMP_CORNER_RIGHT;
        } else {
            ramp_image = 0;
        }
    }
    int32_t mark_tiles = 0;
    int32_t image;
    if (p->stage == PYRAMID_STAGE_MAIN_MATERIAL || scenario_pyramid_style == PYRAMID_STYLE_MASTABA) {
        if (p->blocks == 0) {
            if (part < PYRAMID_PART_QUADRANTS && scenario_pyramid_style != PYRAMID_STYLE_MASTABA) {
                image = pyramid_site_image_offsets[view][part] + image_group(IMAGE_GROUP_PYRAMID_LEVEL_TOP);
            } else {
                image = 0;
                mark_tiles = 1;
            }
        } else if (p->layer < p->target_layers || kind == PYRAMID_PART_INNER) {
            image = image_group(IMAGE_GROUP_PYRAMID) + p->blocks + ((map_grids.random[b->grid_offset] & 1) * PYRAMID_BLOCKS_PER_LAYER) + PYRAMID_IMAGE_LAYER_BLOCKS;
        } else if (scenario_pyramid_style == PYRAMID_STYLE_BENT && bent_half_layer(size_variant, p->layer)) {
            image = image_group(IMAGE_GROUP_BENT_PYRAMID) + ((p->blocks - 1) * PYRAMID_IMAGES_PER_BLOCK_ROW) + kind;
        } else if (scenario_pyramid_style == PYRAMID_STYLE_BENT || kind < PYRAMID_PART_INNER) {
            image = image_group(IMAGE_GROUP_PYRAMID) + ((p->blocks - 1) * PYRAMID_IMAGES_PER_BLOCK_ROW) + kind;
        } else {
            image = (kind & 1) + image_group(IMAGE_GROUP_PYRAMID) + PYRAMID_IMAGE_TOP_INNER;
        }
    } else if (scenario_pyramid_style == PYRAMID_STYLE_BENT && bent_half_layer(size_variant, p->layer)) {
        image = image_group(IMAGE_GROUP_BENT_PYRAMID) + ((BENT_PYRAMID_IMAGE_CASING_ROWS_END - p->blocks) * PYRAMID_IMAGES_PER_BLOCK_ROW) + kind;
    } else {
        image = image_group(IMAGE_GROUP_PYRAMID) + ((PYRAMID_IMAGE_CASING_ROWS_END - p->blocks) * PYRAMID_IMAGES_PER_BLOCK_ROW) + kind;
    }

    if ((p->ramp1_height == 0 && p->ramp2_height == 0) || ramp_image == 0) {
        p->ramp_image_id = 0;
    } else {
        int32_t k = ramp_kind;
        if (scenario_pyramid_style == PYRAMID_STYLE_BENT && bent_half_layer(size_variant, p->layer)) {
            if (p->ramp1_height) {
                int32_t row = (p->ramp1_height * 2 - 2) % PYRAMID_BLOCKS_PER_LAYER;
                p->ramp_image_x1 = ramp_x(k, row);
                p->ramp_image_y1 = (int16_t) (mastaba_entry.ramp_offsets[k][row].y / 2 + (int16_t) pyramid_ramp_base[k].y);
            }
            if (p->ramp2_height) {
                int32_t row = (p->ramp2_height * 2 - 2) % PYRAMID_BLOCKS_PER_LAYER;
                p->ramp_image_x2 = ramp_x(k, row);
                p->ramp_image_y2 = (int16_t) (mastaba_entry.ramp_offsets[k][row].y / 2 + (int16_t) pyramid_ramp_base[k].y);
            }
        } else if (scenario_pyramid_style == PYRAMID_STYLE_STEPPED) {
            if (p->ramp1_height) {
                int32_t row = ((p->ramp1_height - 1) % PYRAMID_BLOCKS_PER_LAYER >= 3 ? 3 : 0) + k * PYRAMID_BLOCKS_PER_LAYER;
                p->ramp_image_x1 = (int16_t) stepped_ramp_offsets[row].x;
                p->ramp_image_y1 = (int16_t) stepped_ramp_offsets[row].y;
            }
            if (p->ramp2_height) {
                int32_t row = ((p->ramp2_height - 1) % PYRAMID_BLOCKS_PER_LAYER >= 3 ? 3 : 0) + k * PYRAMID_BLOCKS_PER_LAYER;
                p->ramp_image_x2 = (int16_t) stepped_ramp_offsets[row].x;
                p->ramp_image_y2 = (int16_t) stepped_ramp_offsets[row].y;
            }
        } else {
            if (p->ramp1_height) {
                int32_t row = (p->ramp1_height - 1) % PYRAMID_BLOCKS_PER_LAYER;
                p->ramp_image_x1 = ramp_x(k, row);
                p->ramp_image_y1 = ramp_y(k, row);
            }
            if (p->ramp2_height) {
                int32_t row = (p->ramp2_height - 1) % PYRAMID_BLOCKS_PER_LAYER;
                p->ramp_image_x2 = ramp_x(k, row);
                p->ramp_image_y2 = ramp_y(k, row);
            }
        }
        p->ramp_image_id = (uint32_t) ramp_image;
    }

    if (!mark_tiles) {
        map_building_tiles_add(building_id, b->x, b->y, b->size, b->size, image, TERRAIN_BUILDING);
        return;
    }
    for (int32_t i = 0; i < PYRAMID_PART_QUADRANTS; i++) {
        int32_t tile = b->grid_offset + pyramid_part_tile_offsets[i];
        map_grids.building_ids[tile] = (int16_t) building_id;
        map_grids.terrain[tile] = (map_grids.terrain[tile] & ~TERRAIN_MONUMENT) | TERRAIN_BUILDING;
        pyramid_level_tile_set_image(building_id, tile, (scenario_pyramid_style == PYRAMID_STYLE_MASTABA) ? PYRAMID_STAGE_LEVEL_1 : PYRAMID_STAGE_LEVEL_6, 1);
    }
}

static int32_t complex_tile_blocked(int32_t tile, uint32_t mask)
{
    return (map_grids.terrain[tile + 1] & mask) || (map_grids.terrain[tile + GRID_SIZE] & mask) || (map_grids.terrain[tile + GRID_SIZE + 1] & mask);
}

static int32_t complex_outside(int32_t x, int32_t y)
{
    return map_grid_is_inside_size(map_info.diamond_x_min + x, map_info.diamond_y_min + y, 3) ||
        map_grid_is_inside_size(map_info.diamond_x_min + x + 1, map_info.diamond_y_min + y + 1, 3);
}

static void complex_draw_ghost(int32_t x, int32_t y, int32_t count, int32_t param)
{
    int32_t px, py;
    city_view_tile_to_pixel(x, y, &px, &py);
    for (int32_t i = 0; i < count; i++) {
        image_draw_masked(image_group(IMAGE_GROUP_FLAT_TILE), construction_ghost_tile_offsets[i].x + px, construction_ghost_tile_offsets[i].y + py, param);
    }
}

static void complex_terrain_or(int32_t tile)
{
    map_grids.terrain[tile] |= TERRAIN_MONUMENT;
    map_grids.terrain[tile + 1] |= TERRAIN_MONUMENT;
    map_grids.terrain[tile + GRID_SIZE] |= TERRAIN_MONUMENT;
    map_grids.terrain[tile + GRID_SIZE + 1] |= TERRAIN_MONUMENT;
}

static void complex_terrain_clear(int32_t tile)
{
    uint32_t kept = TERRAIN_KEPT_BY_BUILDING | TERRAIN_WATER;
    map_grids.terrain[tile] &= kept;
    map_grids.terrain[tile + 1] &= kept;
    map_grids.terrain[tile + GRID_SIZE] &= kept;
    map_grids.terrain[tile + GRID_SIZE + 1] &= kept;
}

static void complex_delete_building(int32_t building_id)
{
    game_undo_add_building(building_id);
    routed_building_items_placed++;
    building_get(building_id)->state = BUILDING_STATE_DELETED;
}

void pyramid_complex_area_op(int32_t x, int32_t y, int32_t mode, int32_t param)
{
    pyramid_complex_found = 0;
    pyramid_complex_step = 1;
    pyramid_complex_state = 2;
    uint32_t mask = TERRAIN_NOT_EMPTY & ~(TERRAIN_TREE | TERRAIN_SHRUB);
    uint32_t mask2 = ~(uint32_t) (TERRAIN_WATER | TERRAIN_DEEPWATER | TERRAIN_GROUNDWATER);
    pyramid_complex_area_end = 0;
    pyramid_complex_steps = 0;
    pyramid_complex_free_tile = 0;
    int32_t tile = map_info.grid_start + y * GRID_SIZE + x;
    pyramid_complex_start_tile = tile;
    pyramid_complex_build_type = 0;
    if (mode == PYRAMID_COMPLEX_AREA_FIND_FREE_TILE || mode == PYRAMID_COMPLEX_AREA_MARK_FREE_GROUND || mode == PYRAMID_COMPLEX_AREA_CLEAR ||
        mode == PYRAMID_COMPLEX_AREA_FIND_BUILDING) {
        mask &= ~TERRAIN_BUILDING;
        mask2 &= ~(TERRAIN_BUILDING | TERRAIN_MONUMENT);
    }
    if (complex_outside(x, y) || complex_tile_blocked(tile, mask)) {
        return;
    }
    int32_t count = tile_count_for_size[MONUMENT_GHOST_BLOCK_SIZE];
    switch (mode) {
        case PYRAMID_COMPLEX_AREA_DRAW_GHOST:
            complex_draw_ghost(x, y, count, param);
            break;
        case PYRAMID_COMPLEX_AREA_MARK_GROUND:
            complex_terrain_or(tile);
            map_grids.images[tile] = image_group(IMAGE_GROUP_MAUSOLEUM_WALKWAY);
            map_grids.images[tile + 1] = image_group(IMAGE_GROUP_MAUSOLEUM_WALKWAY);
            map_grids.images[tile + GRID_SIZE] = image_group(IMAGE_GROUP_MAUSOLEUM_WALKWAY);
            map_grids.images[tile + GRID_SIZE + 1] = image_group(IMAGE_GROUP_MAUSOLEUM_WALKWAY);
            break;
        case PYRAMID_COMPLEX_AREA_FIND_FREE_TILE:
            if (map_grids.building_ids[tile] == 0) {
                pyramid_complex_free_tile = tile;
                pyramid_complex_build_type = BUILDING_MORTUARY_TEMPLE;
                return;
            }
            break;
        case PYRAMID_COMPLEX_AREA_MARK_FREE_GROUND:
            if (map_grids.building_ids[tile] == 0) {
                complex_terrain_or(tile);
                map_grids.images[tile] = image_group(IMAGE_GROUP_MAUSOLEUM_WALKWAY);
                map_grids.images[tile + 1] = image_group(IMAGE_GROUP_MAUSOLEUM_WALKWAY);
                map_grids.images[tile + GRID_SIZE] = image_group(IMAGE_GROUP_MAUSOLEUM_WALKWAY);
                map_grids.images[tile + GRID_SIZE + 1] = image_group(IMAGE_GROUP_MAUSOLEUM_WALKWAY);
            }
            break;
        case PYRAMID_COMPLEX_AREA_CLEAR:
            if (map_grids.building_ids[tile]) {
                complex_delete_building(map_grids.building_ids[tile]);
            } else {
                complex_terrain_clear(tile);
            }
            break;
        case PYRAMID_COMPLEX_AREA_FIND_BUILDING:
            if (map_grids.building_ids[tile] == pyramid_complex_search_building) {
                pyramid_complex_found = 1;
                return;
            }
            break;
    }
    tile += pyramid_complex_step * 2;
    x += 2;
    if (map_grid_is_inside_size(map_info.diamond_x_min + x, map_info.diamond_y_min + y, 3)) {
        return;
    }
    for (;;) {
        if (map_grid_is_inside_size(map_info.diamond_x_min + x + 1, map_info.diamond_y_min + y + 1, 3)) {
            return;
        }
        if (complex_tile_blocked(tile, mask)) {
            break;
        }
        switch (mode) {
            case PYRAMID_COMPLEX_AREA_DRAW_GHOST:
                complex_draw_ghost(x, y, count, param);
                break;
            case PYRAMID_COMPLEX_AREA_MARK_GROUND:
                complex_terrain_or(tile);
                break;
            case PYRAMID_COMPLEX_AREA_MARK_FREE_GROUND:
                if (map_grids.building_ids[tile] == 0) {
                    complex_terrain_or(tile);
                }
                break;
            case PYRAMID_COMPLEX_AREA_FIND_FREE_TILE:
                if (map_grids.building_ids[tile] == 0) {
                    pyramid_complex_free_tile = tile;
                    pyramid_complex_build_type = BUILDING_CAUSEWAY;
                    return;
                }
                break;
            case PYRAMID_COMPLEX_AREA_CLEAR:
                if (map_grids.building_ids[tile]) {
                    complex_delete_building(map_grids.building_ids[tile]);
                } else {
                    complex_terrain_clear(tile);
                }
                break;
            case PYRAMID_COMPLEX_AREA_FIND_BUILDING:
                if (map_grids.building_ids[tile] == pyramid_complex_search_building) {
                    pyramid_complex_found = 1;
                    return;
                }
                break;
        }
        pyramid_complex_steps++;
        if (pyramid_complex_steps > 20 || map_info.diamond_y_min + y >= map_info.diamond_y_max) {
            return;
        }
        x += 2;
        tile += pyramid_complex_step * 2;
        if (map_grid_is_inside_size(map_info.diamond_x_min + x, map_info.diamond_y_min + y, 3)) {
            return;
        }
    }
    if (pyramid_complex_steps == 0 || (map_grids.terrain[tile + 1] & mask2) || (map_grids.terrain[tile + GRID_SIZE] & mask) ||
        (map_grids.terrain[tile + GRID_SIZE + 1] & mask2) || !(map_grids.terrain[tile + 1] & TERRAIN_WATER) || !(map_grids.terrain[tile + GRID_SIZE + 1] & TERRAIN_WATER) ||
        complex_outside(x, y)) {
        return;
    }
    switch (mode) {
        case PYRAMID_COMPLEX_AREA_DRAW_GHOST:
            complex_draw_ghost(x, y, tile_count_for_size[MONUMENT_GHOST_BLOCK_SIZE], param);
            break;
        case PYRAMID_COMPLEX_AREA_MARK_GROUND:
            pyramid_complex_area_end = tile;
            complex_terrain_or(tile);
            map_grids.images[tile] = image_group(IMAGE_GROUP_MAUSOLEUM_WALKWAY);
            map_grids.images[tile + GRID_SIZE] = image_group(IMAGE_GROUP_MAUSOLEUM_WALKWAY);
            return;
        case PYRAMID_COMPLEX_AREA_FIND_FREE_TILE:
            if (map_grids.building_ids[tile] == 0) {
                pyramid_complex_free_tile = tile;
                pyramid_complex_build_type = BUILDING_VALLEY_TEMPLE;
                return;
            }
            break;
        case PYRAMID_COMPLEX_AREA_MARK_FREE_GROUND:
            if (map_grids.building_ids[tile] == 0) {
                pyramid_complex_area_end = tile;
                complex_terrain_or(tile);
                map_grids.images[tile] = image_group(IMAGE_GROUP_MAUSOLEUM_WALKWAY);
                map_grids.images[tile + GRID_SIZE] = image_group(IMAGE_GROUP_MAUSOLEUM_WALKWAY);
                return;
            }
            break;
        case PYRAMID_COMPLEX_AREA_CLEAR:
            pyramid_complex_area_end = tile;
            if (map_grids.building_ids[tile]) {
                complex_delete_building(map_grids.building_ids[tile]);
            } else {
                complex_terrain_clear(tile);
            }
            return;
        case PYRAMID_COMPLEX_AREA_FIND_BUILDING:
            if (map_grids.building_ids[tile] == pyramid_complex_search_building) {
                pyramid_complex_found = 1;
                return;
            }
            break;
    }
    pyramid_complex_area_end = tile;
}

void city_view_tile_to_pixel(int32_t x, int32_t y, int32_t *px, int32_t *py)
{
    int32_t value = city_view.lookup[(city_view.camera_row * VIEW_COLS) + city_view.camera_col];
    int32_t row, column;
    if (value < VIEW_LOOKUP_EMPTY) {
        column = value % GRID_SIZE;
        row = value / GRID_SIZE;
    } else {
        view_tile_from_screen(city_view.camera_row, city_view.camera_col * 2 + 1, &row, &column);
    }
    y = map_info.diamond_y_min - row + y;
    x = map_info.diamond_x_min - column + x;
    switch (map_info.orientation) {
        case VIEW_ORIENTATION_NORTH:
            *px = (x - y) * (city_view.tile_half_width + 1) + city_view.origin_x;
            *py = (x + y - 1) * city_view.tile_half_height + city_view.origin_y;
            break;
        case VIEW_ORIENTATION_EAST:
            *px = (x + y) * (city_view.tile_half_width + 1) + city_view.origin_x;
            *py = (y - x - 1) * city_view.tile_half_height + city_view.origin_y;
            break;
        case VIEW_ORIENTATION_SOUTH:
            *px = (y - x) * (city_view.tile_half_width + 1) + city_view.origin_x;
            *py = city_view.origin_y - (x + y + 1) * city_view.tile_half_height;
            break;
        case VIEW_ORIENTATION_WEST:
            *px = city_view.origin_x - (x + y) * (city_view.tile_half_width + 1);
            *py = (x - y - 1) * city_view.tile_half_height + city_view.origin_y;
            break;
    }
}

void pyramid_part_pixel_offset(int32_t building_id, int32_t height, int32_t *px, int32_t *py)
{
    if (height < 1) {
        *px = 0;
        *py = PYRAMID_FIGURE_GROUND_Y;
        return;
    }
    building *b = building_get(building_id);
    int32_t layer = (height - 1) / PYRAMID_BLOCKS_PER_LAYER;
    int32_t layer_height = layer_pixel_height(b->data.pyramid.size_variant, layer);
    *px = city_view.tile_half_width + 1;
    *py = -(city_view.tile_half_height + layer_height);
    int32_t kind = (scenario_pyramid_style == PYRAMID_STYLE_MASTABA) ? pyramid_layout.mastaba[b->data.pyramid.size_variant][b->subtype].kind
                                                                     : pyramid_layout.pyramid[b->data.pyramid.size_variant][b->subtype].kind;
    int32_t view = map_info.orientation / 2;
    if (kind < PYRAMID_PART_EDGE_TOP_LEFT) {
        kind += view;
        if (kind >= PYRAMID_PART_EDGE_TOP_LEFT) {
            kind -= PYRAMID_VIEW_COUNT;
        }
    } else if (kind < PYRAMID_PART_INNER) {
        kind += view;
        if (kind >= PYRAMID_PART_INNER) {
            kind -= PYRAMID_VIEW_COUNT;
        }
    }
    if (kind < PYRAMID_PART_MASTABA_MAIN) {
        if (b->data.pyramid.layer < b->data.pyramid.target_layers) {
            kind = PYRAMID_PART_INNER;
        }
        int32_t step = (height - 1) % PYRAMID_BLOCKS_PER_LAYER;
        if (scenario_pyramid_style == PYRAMID_STYLE_STEPPED) {
            if (b->data.pyramid.ramp_image_id) {
                *px += stepped_ramp_offsets[step + kind * PYRAMID_BLOCKS_PER_LAYER].x_with_ramp;
                *py += stepped_ramp_offsets[step + kind * PYRAMID_BLOCKS_PER_LAYER].y_with_ramp;
            } else {
                *px += stepped_ramp_offsets[step + kind * PYRAMID_BLOCKS_PER_LAYER].x;
                *py += stepped_ramp_offsets[step + kind * PYRAMID_BLOCKS_PER_LAYER].y;
            }
            *px += STEPPED_PYRAMID_FIGURE_SHIFT_X;
            *py += STEPPED_PYRAMID_FIGURE_SHIFT_Y;
            *px += PYRAMID_FIGURE_X;
            *py += PYRAMID_FIGURE_Y;
            return;
        }
        *px += mastaba_entry.ramp_offsets[kind][step].x;
        *py += mastaba_entry.ramp_offsets[kind][step].y;
        if (b->data.pyramid.ramp_image_id) {
            *px += pyramid_ramp_pixel_offsets[kind].x;
            *py += pyramid_ramp_pixel_offsets[kind].y;
        }
    }
    *px += PYRAMID_FIGURE_X;
    *py += PYRAMID_FIGURE_Y;
}

static void part_anchor_tile(int32_t building_id, int32_t *x, int32_t *y)
{
    building *b = building_get(building_id);
    *x = b->x;
    *y = b->y;
    switch (city_view_orientation) {
        case VIEW_ORIENTATION_NORTH:
            (*y)++;
            break;
        case VIEW_ORIENTATION_SOUTH:
            (*x)++;
            break;
        case VIEW_ORIENTATION_WEST:
            (*x)++;
            (*y)++;
            break;
    }
}

void pyramid_figure_get_pixel_offset(int32_t figure_id, int32_t *out_x, int32_t *out_y)
{
    figure *f = figure_get(figure_id);
    int32_t prev = f->pyramid_work.previous_part;
    int32_t prev_height = f->pyramid_work.previous_height;
    int32_t part = f->pyramid_work.part;
    int32_t height = f->pyramid_work.height;
    int32_t ax, ay;
    if (prev == 0 || prev == part) {
        building *main = building_get(monument_main_part_id(part));
        city_view_tile_to_pixel(main->road_access_x, main->road_access_y, &ax, &ay);
    } else {
        pyramid_part_pixel_offset(prev, prev_height, &ax, &ay);
        int32_t tx, ty, px, py;
        part_anchor_tile(prev, &tx, &ty);
        city_view_tile_to_pixel(tx, ty, &px, &py);
        ax += px;
        ay += py;
    }
    int32_t tx, ty, ox, oy, px, py;
    part_anchor_tile(part, &tx, &ty);
    pyramid_part_pixel_offset(part, height, &ox, &oy);
    city_view_tile_to_pixel(tx, ty, &px, &py);
    int32_t bx = ox + px;
    int32_t by = oy + py;
    int16_t progress = f->pyramid_work.progress;
    int16_t total = f->pyramid_work.total;
    if (progress < total) {
        *out_x = ((bx - ax) * progress) / total + ax;
        *out_y = ((by - ay) * progress) / total + ay;
    } else {
        *out_x = bx;
        *out_y = by;
    }
}
static void ramp_sprites(int32_t subtype, int32_t size_variant, int32_t height1, int32_t height2, int32_t *sprite1, int32_t *sprite2)
{
    *sprite1 = 0;
    *sprite2 = 0;
    if (scenario_pyramid_style != PYRAMID_STYLE_STEPPED) {
        return;
    }
    int32_t kind = pyramid_layout.pyramid[size_variant][subtype].kind;
    if (kind >= PYRAMID_PART_EDGE_TOP_LEFT) {
        *sprite1 = (height1 - 1) % 3;
        *sprite2 = (height2 - 1) % 3;
        return;
    }
    switch ((uint8_t) height1 % PYRAMID_BLOCKS_PER_LAYER) {
        case 2:
            *sprite1 = 1;
            break;
        case 4:
            *sprite1 = 2;
            break;
        case 5:
            *sprite1 = 3;
            break;
        default:
            break;
    }
    switch ((uint8_t) height2 % PYRAMID_BLOCKS_PER_LAYER) {
        case 2:
            *sprite2 = 1;
            break;
        case 4:
            *sprite2 = 2;
            break;
        case 5:
            *sprite2 = 3;
            break;
        default:
            *sprite1 = 0;
            *sprite2 = 0;
            break;
    }
}

static void draw_one_ramp(int32_t image, int32_t offset_x, int32_t offset_y, int32_t base_x, int32_t base_y)
{
    int32_t shift_y = 0;
    int32_t shift_x = 0;
    if (scenario_pyramid_style == PYRAMID_STYLE_STEPPED) {
        int32_t index = (image - image_group(IMAGE_GROUP_PYRAMID)) - PYRAMID_IMAGE_RAMPS;
        shift_x = stepped_ramp_shift[index].x;
        shift_y = stepped_ramp_shift[index].y;
    }
    monument_draw_image(image, (int16_t) offset_x + shift_x + base_x, (int16_t) offset_y + shift_y + base_y);
}

static void draw_part_ramps(building *part, int32_t size_variant, int32_t base_x, int32_t base_y)
{
    building_pyramid *p = &part->data.pyramid;
    if (p->ramp_image_id == 0) {
        return;
    }
    int32_t sprite1, sprite2;
    ramp_sprites(part->subtype, size_variant, p->ramp1_height, p->ramp2_height, &sprite1, &sprite2);
    if (p->ramp1_height != 0) {
        draw_one_ramp((int32_t) p->ramp_image_id + sprite1, p->ramp_image_x1, p->ramp_image_y1, base_x, base_y);
    }
    if (p->ramp2_height != 0) {
        draw_one_ramp((int32_t) p->ramp_image_id + sprite2, p->ramp_image_x2, p->ramp_image_y2, base_x, base_y);
    }
}

static void place_part_figures(int32_t grid_offset, int32_t mark, int32_t x, int32_t y, int32_t *last_tile)
{
    for (int32_t i = 0; i < PYRAMID_PART_QUADRANTS; i++) {
        int32_t tile = grid_offset + pyramid_part_figure_tiles[i];
        if (last_tile) {
            *last_tile = tile;
        }
        for (int32_t id = map_grids.figure_ids[tile]; id > 0; id = city_figures.records[id].next_figure_id_on_same_tile) {
            int32_t pixel = city_figures.records[id].pyramid_walk_mode;
            if (mark == 3) {
                if (pixel == 3) {
                    city_draw_figure(id, x, y, 0);
                }
            } else if (mark == 1) {
                if (pixel != 0) {
                    city_draw_figure(id, x, y, 0);
                }
            } else if (pixel > 0 && pixel != 3) {
                city_draw_figure(id, x, y, 0);
            }
        }
    }
}

static void store_ramp_link(building *neighbor, int32_t building_id, int32_t draw_x, int32_t draw_y, int32_t x, int32_t y)
{
    building_pyramid *link = &neighbor->data.pyramid;
    link->ramp_link_building_id = (uint32_t) building_id;
    link->ramp_link_draw_x = draw_x;
    link->ramp_link_draw_y = draw_y;
    link->ramp_link_x = x;
    link->ramp_link_y = y;
}

void pyramid_part_draw(int32_t image, int32_t x, int32_t y, int32_t building_id)
{
    building *b = building_get(building_id);
    building_pyramid *p = &b->data.pyramid;
    int32_t height = layer_pixel_height(p->size_variant, p->layer);
    int32_t draw_x = x;
    int32_t draw_y = y;
    int32_t bare = p->stage == PYRAMID_STAGE_MAIN_MATERIAL && p->layer == 0 && p->blocks == 0 && scenario_pyramid_style != PYRAMID_STYLE_MASTABA &&
        b->subtype >= PYRAMID_PART_QUADRANTS;
    if (p->stage < PYRAMID_STAGE_SITE_PREPARATION && !bare) {
        draw_x = city_view.tile_half_width + 1 + x;
        draw_y = (y - city_view.tile_half_height) - height;
    }
    int32_t linked_here = 0;
    if (p->ramp1_height >= 7) {
        int32_t special = 0;
        if (scenario_pyramid_style == PYRAMID_STYLE_STEPPED) {
            special = ((int32_t) p->ramp1_height - 1) % PYRAMID_BLOCKS_PER_LAYER < 3;
        } else {
            special = (uint8_t) p->ramp1_height % PYRAMID_BLOCKS_PER_LAYER == 1;
        }
        if (special) {
            int32_t kind = pyramid_layout.pyramid[p->size_variant][b->subtype].kind;
            int32_t view = map_info.orientation / 2;
            if (kind < PYRAMID_PART_EDGE_TOP_LEFT) {
                kind += view;
                if (kind > PYRAMID_PART_CORNER_RIGHT) {
                    kind -= PYRAMID_VIEW_COUNT;
                }
            } else if (kind < PYRAMID_PART_INNER) {
                kind += view;
                if (kind > PYRAMID_PART_EDGE_TOP_RIGHT) {
                    kind -= PYRAMID_VIEW_COUNT;
                }
            }
            int32_t attach = (scenario_pyramid_style == PYRAMID_STYLE_STEPPED)
                ? kind != PYRAMID_PART_CORNER_TOP && kind != PYRAMID_PART_EDGE_TOP_LEFT && kind != PYRAMID_PART_EDGE_TOP_RIGHT
                : kind == PYRAMID_PART_EDGE_BOTTOM_LEFT || kind == PYRAMID_PART_EDGE_BOTTOM_RIGHT;
            if (attach) {
                int32_t delta = pyramid_neighbour_offsets[view][kind == PYRAMID_PART_CORNER_BOTTOM ? 0 : 1];
                int32_t neighbor_id = map_grids.building_ids[b->grid_offset + delta];
                building *neighbor = building_get(neighbor_id);
                if (neighbor_id != 0 && neighbor->type == BUILDING_PYRAMID) {
                    store_ramp_link(neighbor, building_id, draw_x, draw_y, x, y);
                    linked_here = 1;
                }
            }
        }
    }
    int32_t last_tile = 0;
    int32_t figures_placed = 0;
    int32_t link = (int32_t) p->ramp_link_building_id;
    if (!linked_here && p->ramp_image_id == 0 && link == 0) {
        place_part_figures(b->grid_offset, 3, x, y, &last_tile);
        figures_placed = 1;
    }
    if (height != 0) {
        if (scenario_pyramid_style == PYRAMID_STYLE_MASTABA && p->size_variant > 1 && mastaba_part_decor[b->subtype] != 0 && p->layer != 0) {
            int32_t decor = image_group(IMAGE_GROUP_PYRAMID) + PYRAMID_IMAGE_LAYER_BLOCKS + PYRAMID_BLOCKS_PER_LAYER + ((map_grids.random[last_tile] & 1) * PYRAMID_BLOCKS_PER_LAYER);
            monument_draw_image(decor, draw_x, draw_y + height);
            image_draw_isometric_top(decor, draw_x, draw_y + height);
        }
        monument_draw_image(image, draw_x, draw_y);
    }
    image_draw_isometric_top(image, draw_x, draw_y);
    if (link != 0) {
        building *other = building_get(link);
        draw_part_ramps(other, p->size_variant, p->ramp_link_draw_x, p->ramp_link_draw_y);
        place_part_figures(other->grid_offset, 1, p->ramp_link_x, p->ramp_link_y, 0);
    }
    if (linked_here) {
        return;
    }
    draw_part_ramps(b, p->size_variant, draw_x, draw_y);
    if (!figures_placed) {
        place_part_figures(b->grid_offset, 3, x, y, 0);
    }
    place_part_figures(b->grid_offset, 2, x, y, 0);
}
