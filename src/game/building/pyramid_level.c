// Site preparation of the pyramid family: laborers level the 2x2 tiles of every part through stages 2 to 11,
// quadrant by quadrant, with the level images; and the god speed-up that finishes pyramid work without workers.
//
// The reference part's position is read twice for two of the distance metrics.

#include "building/pyramid_level.h"
#include "building/building_data.h"
#include "building/building_image_group.h"
#include "building/building_type.h"
#include "building/monument.h"
#include "building/monument_internal.h"
#include "building/monument_object.h"
#include "building/monument_spots.h"
#include "building/pyramid.h"
#include "building/pyramid_blocks.h"
#include "building/pyramid_images.h"
#include "building/pyramid_parts.h"
#include "figure/figure.h"
#include "figure/figure_action.h"
#include "figure/figure_core.h"
#include "figure/figure_type.h"
#include "game/game_data.h"
#include "graphics/image_collection.h"
#include "map/grid.h"
#include "map/routing_terrain.h"
#include "map/terrain.h"
#include "map/terrain_image_group.h"
#include "map/tiles.h"
#include "map/tiles_road.h"
#include <stdbool.h>

int32_t pyramid_level_images_preset[1];

static int32_t part_count(int32_t size)
{
    return (scenario_pyramid_style == PYRAMID_STYLE_MASTABA) ? mastaba_part_count[size] : pyramid_part_count[size];
}

static const pyramid_preset_list *preset_lists(void)
{
    return (scenario_pyramid_style == PYRAMID_STYLE_MASTABA) ? mastaba_preset_parts : pyramid_preset_parts.lists;
}

static int32_t square_distance(int32_t dx, int32_t dy)
{
    return dx * dx + dy * dy;
}

static void set_leveled_ground(int32_t grid_offset)
{
    map_grids.terrain[grid_offset] = TERRAIN_MONUMENT | TERRAIN_BUILDING;
    map_grids.images[grid_offset] = (map_grids.random[grid_offset] & 7) + image_group(IMAGE_GROUP_LEVELED_GROUND);
}

int32_t pyramid_level_stage_next(int32_t stage)
{
    stage++;
    if (stage == PYRAMID_STAGE_PRESET) {
        stage = PYRAMID_STAGE_LEVELED;
    }
    return stage;
}

bool pyramid_find_level_tile(int32_t main_id, int32_t *x_out, int32_t *y_out, int32_t check_workers)
{
    const building *main = building_get(main_id);
    int32_t size = main->data.pyramid.size_variant;
    const building *ref = building_get(main_id + pyramid_level_reference_part[size]);
    int32_t stage_limit = (scenario_pyramid_style != PYRAMID_STYLE_MASTABA) ? PYRAMID_STAGE_LEVEL_6 : PYRAMID_STAGE_LEVEL_1;
    int32_t best_id = 0;
    int32_t best_stage = 0;
    int32_t best_metric = PYRAMID_LEVEL_NO_METRIC;
    int32_t skipped_busy = 0;
    for (int32_t id = main_id; id < main_id + part_count(size); id++) {
        const building *part = building_get(id);
        if (check_workers && part->data.pyramid.worker_figure_id > 0 && pyramid_part_has_active_worker(id)) {
            skipped_busy = 1;
            continue;
        }
        int32_t stage = part->data.pyramid.stage;
        if (stage == PYRAMID_STAGE_PRESET) {
            stage = PYRAMID_STAGE_LEVELED;
        }
        int32_t mask = part->data.pyramid.prep_mask;
        if (mask == PYRAMID_PREP_ALL_QUADRANTS) {
            mask = 0;
            stage = pyramid_level_stage_next(stage);
        }
        if (stage > stage_limit) {
            continue;
        }
        int32_t part_metric = PYRAMID_LEVEL_NO_METRIC;
        int32_t part_x = 0;
        int32_t part_y = 0;
        for (int32_t q = 0; q < PYRAMID_PART_QUADRANTS; q++) {
            if (mask & (1 << q)) {
                continue;
            }
            int32_t x = part->x + pyramid_level_quadrant_dx[q];
            int32_t y = part->y + pyramid_level_quadrant_dy[q];
            int32_t metric;
            switch (pyramid_level_stage_metric[stage]) {
                case PYRAMID_LEVEL_NEAR_MAIN_PART:
                    metric = square_distance(x - main->x, y - main->y);
                    break;
                case PYRAMID_LEVEL_NEAR_REFERENCE_PART:
                    metric = square_distance(x - ref->x, y - ref->y);
                    break;
                case PYRAMID_LEVEL_FAR_FROM_REFERENCE_PART:
                    metric = PYRAMID_LEVEL_FAR_METRIC_BASE - ((y - ref->y) * (y - ref->y)) - ((x - ref->x) * (x - ref->x));
                    break;
                default:
                    metric = PYRAMID_LEVEL_NO_METRIC;
                    break;
            }
            if (metric < part_metric) {
                part_metric = metric;
                part_x = x;
                part_y = y;
            }
        }
        if (best_id == 0 || stage < best_stage || (stage == best_stage && part_metric < best_metric)) {
            best_id = id;
            best_metric = part_metric;
            *x_out = part_x;
            *y_out = part_y;
            best_stage = stage;
        }
    }
    if (best_id != 0) {
        return 1;
    }
    int32_t main_levelled = (main->data.pyramid.stage == PYRAMID_STAGE_LEVEL_6) && (main->data.pyramid.prep_mask == PYRAMID_PREP_ALL_QUADRANTS);
    if (skipped_busy || scenario_pyramid_style == PYRAMID_STYLE_MASTABA || !main_levelled) {
        return 0;
    }
    if (check_workers) {
        for (int32_t i = 0; i < PYRAMID_PART_QUADRANTS; i++) {
            if (building_get(main_id + i)->data.pyramid.worker_figure_id > 0 && pyramid_part_has_active_worker(main_id + i)) {
                return 0;
            }
        }
    }
    *x_out = main->x + 1;
    *y_out = main->y + 1;
    return 1;
}

int32_t pyramid_level_tile_done(int32_t x, int32_t y)
{
    int32_t grid_offset = map_info.grid_start + y * GRID_SIZE + x;
    int32_t id = map_grids.building_ids[grid_offset];
    building *b = building_get(id);
    int32_t bit = 0;
    for (int32_t q = 0; q < PYRAMID_PART_QUADRANTS; q++) {
        if (b->grid_offset + pyramid_part_quadrant_offsets[q] == grid_offset) {
            bit = 1 << q;
            break;
        }
    }
    if (b->data.pyramid.prep_mask == PYRAMID_PREP_ALL_QUADRANTS) {
        if (b->data.pyramid.stage < PYRAMID_STAGE_LEVEL_6) {
            b->data.pyramid.stage = (uint8_t) pyramid_level_stage_next(b->data.pyramid.stage);
            if (b->data.pyramid.stage == PYRAMID_STAGE_LEVEL_1) {
                bit = PYRAMID_PREP_ALL_QUADRANTS;
            }
            b->data.pyramid.prep_mask = (uint8_t) bit;
        } else {
            if (b->subtype != 0) {
                return 0;
            }
            int32_t stage = b->data.pyramid.stage + 1;
            if (stage <= PYRAMID_STAGE_LEVEL_TOP) {
                for (int32_t i = 0; i < PYRAMID_PART_QUADRANTS; i++) {
                    building *part = building_get(id + i);
                    part->data.pyramid.stage = (uint8_t) stage;
                    part->data.pyramid.prep_mask = PYRAMID_PREP_ALL_QUADRANTS;
                }
                pyramid_update_all_part_images(id);
                pyramid_check_site_prepared(id);
                return id;
            }
        }
    } else {
        if (b->data.pyramid.stage == PYRAMID_STAGE_LEVEL_1) {
            bit = PYRAMID_PREP_ALL_QUADRANTS;
        }
        b->data.pyramid.prep_mask |= (uint8_t) bit;
        if (b->data.pyramid.stage == PYRAMID_STAGE_PRESET) {
            b->data.pyramid.stage = PYRAMID_STAGE_LEVELED;
            for (int32_t q = 0; q < PYRAMID_PART_QUADRANTS; q++) {
                set_leveled_ground(b->grid_offset + pyramid_part_quadrant_offsets[q]);
            }
        }
    }
    if (b->data.pyramid.stage <= PYRAMID_STAGE_LEVELED) {
        set_leveled_ground(grid_offset);
    }
    pyramid_check_site_prepared(id - b->subtype);
    pyramid_update_all_part_images(id - b->subtype);
    return id;
}

bool pyramid_level_one_tile_instant(int32_t main_id)
{
    int32_t x;
    int32_t y;
    if (pyramid_find_level_tile(main_id, &x, &y, 0)) {
        pyramid_level_tile_done(x, y);
        return 1;
    }
    return 0;
}

void pyramid_speedup_tick(void)
{
    int32_t orientation = map_info.orientation / 2;
    for (int32_t id = 1; id < MAX_BUILDINGS; id++) {
        building *b = building_get(id);
        if (b->state != BUILDING_STATE_IN_USE || b->type != BUILDING_PYRAMID) {
            continue;
        }
        if (b->data.pyramid.stage > PYRAMID_STAGE_CASING) {
            if (!pyramid_level_one_tile_instant(id)) {
                pyramid_start_block_phase(id);
            }
            map_routing_update_land_citizen();
        } else {
            pyramid_find_frontier_parts(id);
            pyramid_sort_frontier_parts();
            if (pyramid_frontier_count > 0) {
                int32_t part_id = pyramid_frontier_parts[0];
                pyramid_part_add_block_instant(part_id);
                if (pyramid_part_needs_ramp(part_id)) {
                    pyramid_part_build_ramp(part_id);
                }
                const building *part_main = building_get(monument_main_part_id(part_id));
                pyramid_part_update_image(part_id, orientation, part_main->data.pyramid.size_variant);
            } else {
                int32_t size = b->data.pyramid.size_variant;
                if (pyramid_has_complex_temple(size)) {
                    int32_t offset = pyramid_complex_temple_offset[size] + b->grid_offset - map_info.grid_start;
                    pyramid_complex_area_op(offset % GRID_SIZE, offset / GRID_SIZE, PYRAMID_COMPLEX_AREA_FIND_FREE_TILE, 0);
                    if (pyramid_complex_free_tile != 0) {
                        int32_t x = (pyramid_complex_free_tile - map_info.grid_start) % GRID_SIZE;
                        int32_t y = (pyramid_complex_free_tile - map_info.grid_start) / GRID_SIZE;
                        int32_t new_id = building_create(city_current_player, pyramid_complex_build_type, x, y, 0, 0);
                        building_construction_add_to_map(city_current_player, pyramid_complex_build_type, new_id, x, y, 2, 1, 0);
                        map_tiles_update_region_canals(0, 0, map_info.grid_width - 1, map_info.grid_height - 1, 0);
                        map_routing_update_land_citizen();
                        map_routing_update_land_noncitizen();
                    }
                }
            }
        }
        id += part_count(b->data.pyramid.size_variant) - 1;
    }
}

void pyramid_speedup_start(int32_t multiplier)
{
    int32_t shortest = 999999;
    for (int32_t id = 1; id < building_highest_id; id++) {
        const building *b = building_get(id);
        if (b->state != BUILDING_STATE_IN_USE || b->type != BUILDING_PYRAMID) {
            continue;
        }
        int32_t steps = 999999;
        int32_t end_id = id + part_count(b->data.pyramid.size_variant);
        for (int32_t figure_id = 1; figure_id < MAX_FIGURES; figure_id++) {
            figure *f = figure_get(figure_id);
            if (!f->state) {
                continue;
            }
            int32_t target = f->destination_building_id;
            if (f->type == FIGURE_LABORER) {
                if (target < id) {
                    continue;
                }
            } else if (f->type != FIGURE_STONEMASON && f->type != FIGURE_CARPENTER && f->type != FIGURE_BRICKLAYER && f->type != FIGURE_SLED_PULLER) {
                continue;
            }
            if (target < end_id) {
                f->action_state = FIGURE_ACTION_CORPSE;
                map_figure_delete(figure_id);
            }
        }
        int32_t size = b->data.pyramid.size_variant;
        if (b->data.pyramid.stage > PYRAMID_STAGE_CASING) {
            pyramid_progress info;
            pyramid_get_progress_info(id, &info);
            steps = info.prep_total - info.prep_done + 1;
            int32_t speedup_steps = (part_count(size) * multiplier) * 2;
            if (speedup_steps < steps) {
                steps = speedup_steps;
            }
        } else if (!pyramid_blocks_finished(id)) {
            steps = part_count(size) * multiplier;
        }
        if (steps < shortest) {
            shortest = steps;
        }
        id += part_count(b->data.pyramid.size_variant) - 1;
    }
    if (shortest != 999999) {
        pyramid_speedup_active = 1;
        pyramid_speedup_countdown = shortest;
    }
}

int32_t pyramid_tile_effective_stage(int32_t grid_offset)
{
    int32_t id = map_grids.building_ids[grid_offset];
    if (id == 0) {
        return -1;
    }
    const building *b = building_get(id);
    if (b->type != BUILDING_PYRAMID) {
        return -1;
    }
    int32_t bit;
    switch (grid_offset - b->grid_offset) {
        case 0:
            bit = 1;
            break;
        case 1:
            bit = 2;
            break;
        case GRID_SIZE:
            bit = 4;
            break;
        case GRID_SIZE + 1:
            bit = 8;
            break;
        default:
            return PYRAMID_STAGE_SITE_PREPARATION;
    }
    int32_t stage = b->data.pyramid.stage;
    if (stage == PYRAMID_STAGE_PRESET) {
        const pyramid_preset_list *list = &preset_lists()[b->data.pyramid.size_variant];
        for (int32_t i = 0; i < list->count; i++) {
            if (b->subtype == list->parts[i].part) {
                return (grid_offset == list->parts[i].grid_offset + b->grid_offset) + PYRAMID_STAGE_SITE_PREPARATION;
            }
        }
        return PYRAMID_STAGE_SITE_PREPARATION;
    }
    if (b->data.pyramid.prep_mask & bit) {
        return stage;
    }
    if (stage - 1 == PYRAMID_STAGE_PRESET) {
        return PYRAMID_STAGE_SITE_PREPARATION;
    }
    return stage - 1;
}

bool pyramid_level_tile_set_image(int32_t building_id, int32_t grid_offset, int32_t stage, bool redraw)
{
    (void) building_id;
    const int32_t *images;
    int32_t base;
    int32_t variants = 1;
    int32_t mask = PYRAMID_LEVEL_ALL_NEIGHBOURS;
    switch (stage) {
        case PYRAMID_STAGE_PRESET:
            images = pyramid_level_images_preset;
            base = image_group(IMAGE_GROUP_MAUSOLEUM_WALKWAY);
            mask = 0;
            break;
        case PYRAMID_STAGE_LEVELED:
            if (redraw) {
                map_grids.images[grid_offset] = 0;
                int32_t x = (grid_offset - map_info.grid_start) % GRID_SIZE;
                int32_t y = (grid_offset - map_info.grid_start) / GRID_SIZE;
                map_tiles_update_region_trees(x, y, x, y);
                map_tiles_update_region_empty_land(x, y, x, y, 0, 1);
                map_tiles_update_region_meadow(x, y, x, y);
            }
            return redraw;
        case PYRAMID_STAGE_LEVEL_1:
            images = pyramid_level_images_stage5;
            base = image_group(IMAGE_GROUP_PYRAMID_LEVEL_STAGE_5);
            variants = PYRAMID_LEVEL_STAGE_1_VARIANTS;
            break;
        case PYRAMID_STAGE_LEVEL_2:
        case PYRAMID_STAGE_LEVEL_4:
            images = pyramid_level_images_edges;
            base = image_group(IMAGE_GROUP_PYRAMID_LEVEL_STAGE_6_8);
            mask = PYRAMID_LEVEL_EDGE_NEIGHBOURS;
            break;
        case PYRAMID_STAGE_LEVEL_3:
            images = pyramid_level_images_edges;
            base = image_group(IMAGE_GROUP_PYRAMID_LEVEL_STAGE_7);
            mask = PYRAMID_LEVEL_EDGE_NEIGHBOURS;
            break;
        case PYRAMID_STAGE_LEVEL_5:
            images = pyramid_level_images_edges;
            base = image_group(IMAGE_GROUP_PYRAMID_LEVEL_STAGE_9);
            mask = PYRAMID_LEVEL_EDGE_NEIGHBOURS;
            break;
        case PYRAMID_STAGE_LEVEL_6:
            images = pyramid_level_images_edges;
            base = image_group(IMAGE_GROUP_PYRAMID_LEVEL_STAGE_10);
            mask = PYRAMID_LEVEL_EDGE_NEIGHBOURS;
            break;
        default:
            return 0;
    }
    int32_t orientation = map_info.orientation;
    int32_t neighbours = 0;
    for (int32_t i = 0; i < PYRAMID_LEVEL_NEIGHBOURS; i++) {
        int32_t s = pyramid_tile_effective_stage(pyramid_level_neighbour_offsets[(i + orientation) & 7] + grid_offset);
        if ((stage == PYRAMID_STAGE_LEVEL_1) ? ((s >= PYRAMID_STAGE_LEVEL_1) || (s == PYRAMID_STAGE_MAIN_MATERIAL)) : (s >= 0)) {
            neighbours |= 1 << i;
        }
    }
    neighbours &= mask;
    int32_t image_id = images[neighbours] + base;
    map_grids.images[grid_offset] = image_id;
    if (neighbours == PYRAMID_LEVEL_ALL_NEIGHBOURS && variants > 1) {
        map_grids.images[grid_offset] = map_grids.random[grid_offset] % variants + image_id;
    }
    return 1;
}

void pyramid_part_update_level_images(int32_t building_id, int32_t size)
{
    const pyramid_preset_list *lists = preset_lists();
    const building *b = building_get(building_id);
    switch (b->data.pyramid.stage) {
        case PYRAMID_STAGE_PRESET: {
            const pyramid_preset_list *list = &lists[size];
            for (int32_t i = 0; i < list->count; i++) {
                if (b->subtype == list->parts[i].part) {
                    int32_t offset = list->parts[i].grid_offset + b->grid_offset;
                    map_grids.images[offset] = image_group(IMAGE_GROUP_MAUSOLEUM_WALKWAY);
                    if (map_grids.random[offset] & 1) {
                        map_grids.bitfields[offset] |= 0x20;
                    }
                    return;
                }
            }
            break;
        }
        case PYRAMID_STAGE_LEVELED:
            for (int32_t q = 0; q < PYRAMID_PART_QUADRANTS; q++) {
                if (b->data.pyramid.prep_mask & (1 << q)) {
                    map_grids.terrain[b->grid_offset + pyramid_part_tile_offsets[q]] &= ~(TERRAIN_TREE | TERRAIN_SHRUB | TERRAIN_GROUNDWATER);
                }
            }
            break;
        case PYRAMID_STAGE_LEVEL_1:
        case PYRAMID_STAGE_LEVEL_2:
        case PYRAMID_STAGE_LEVEL_3:
        case PYRAMID_STAGE_LEVEL_4:
        case PYRAMID_STAGE_LEVEL_5:
        case PYRAMID_STAGE_LEVEL_6:
            for (int32_t q = 0; q < PYRAMID_PART_QUADRANTS; q++) {
                int32_t stage = b->data.pyramid.stage;
                if (!(b->data.pyramid.prep_mask & (1 << q))) {
                    stage--;
                }
                pyramid_level_tile_set_image(building_id, pyramid_part_tile_offsets[q] + b->grid_offset, stage, 1);
            }
            break;
        case PYRAMID_STAGE_LEVEL_TOP:
            map_building_tiles_add(building_id, b->x, b->y, b->size, b->size,
                pyramid_site_image_offsets[map_info.orientation / 2][b->subtype] + image_group(IMAGE_GROUP_PYRAMID_LEVEL_TOP), TERRAIN_BUILDING);
            break;
        default:
            break;
    }
}

bool pyramid_laborer_take_level_tile(int32_t figure_id)
{
    figure *f = figure_get(figure_id);
    int32_t target_id = f->destination_building_id;
    const building *target = building_get(target_id);
    if (target->type != BUILDING_PYRAMID || target->data.pyramid.stage <= PYRAMID_STAGE_CASING) {
        return 0;
    }
    int32_t x;
    int32_t y;
    if (!pyramid_find_level_tile(target_id - target->subtype, &x, &y, 1)) {
        return 0;
    }
    f->destination_x = (int16_t) x;
    f->destination_y = (int16_t) y;
    figure_movement_set_cross_country_direction(figure_id, f->cross_country.x, f->cross_country.y, f->destination_x * 15, (int16_t) y * 15, 0);
    f->roam_length = 0;
    int32_t part_id = map_grids.building_ids[map_info.grid_start + y * GRID_SIZE + x];
    f->destination_building_id = (int16_t) part_id;
    building_get(part_id)->data.pyramid.worker_figure_id = (int16_t) figure_id;
    return 1;
}

int32_t pyramid_level_tile_work_time(int32_t grid_offset)
{
    int32_t stage = pyramid_tile_effective_stage(grid_offset);
    if ((stage >= PYRAMID_STAGE_SITE_PREPARATION) && (stage <= 12)) {
        return PYRAMID_LEVEL_TILE_WORK_TICKS;
    }
    return 0;
}

bool pyramid_laborer_level_work(int32_t figure_id)
{
    figure *f = figure_get(figure_id);
    int32_t grid_offset = map_info.grid_start + f->x + f->y * GRID_SIZE;
    int32_t id = map_grids.building_ids[grid_offset];
    if (id == 0) {
        return 0;
    }
    int32_t work_time = pyramid_level_tile_work_time(grid_offset);
    f->wait_ticks++;
    if (f->wait_ticks >= work_time) {
        pyramid_level_tile_done(f->destination_x, f->destination_y);
        building_get(id)->data.pyramid.worker_figure_id = 0;
        return 0;
    }
    return 1;
}

int32_t pyramid_speedup_level(void)
{
    if (!settings_data.pyramid_speedup) {
        return 0;
    }
    int32_t best_size = -1;
    int32_t best_id = 0;
    for (int32_t id = 1; id <= building_highest_id; id++) {
        const building *b = building_get(id);
        if (b->state == BUILDING_STATE_IN_USE && b->type == BUILDING_PYRAMID && b->data.pyramid.size_variant > best_size) {
            best_id = id;
            best_size = b->data.pyramid.size_variant;
        }
    }
    if (best_size < 0) {
        return 0;
    }
    pyramid_progress info;
    pyramid_get_progress_info(best_id, &info);
    if (info.percent < 50) {
        return 2;
    }
    if (info.percent < 75) {
        return 1;
    }
    return 0;
}
