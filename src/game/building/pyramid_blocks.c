// Block construction of the pyramid family: which part gets the next block, the main material and casing blocks
// laid by stonemasons and bricklayers, the ramps built by carpenters, the temple of the complex, and whether the
// city can supply what the next block needs.
//
// Two things in the block work are not what they seem:
// - the bricklayer's done and next actions are equal, so a bricklayer never goes on;
// - the god speed-up builds the complex temple inline instead of calling its function.

#include "building/pyramid_blocks.h"
#include "building/building_type.h"
#include "building/monument.h"
#include "building/monument_internal.h"
#include "building/monument_object.h"
#include "building/monument_spots.h"
#include "building/pyramid.h"
#include "building/pyramid_figure.h"
#include "building/pyramid_images.h"
#include "building/pyramid_level.h"
#include "building/pyramid_parts.h"
#include "city/city_player.h"
#include "city/resource_type.h"
#include "core/crt_compat.h"
#include "figure/figure.h"
#include "figure/figure_action.h"
#include "figure/figure_core.h"
#include "figure/figure_type.h"
#include "game/undo_build.h"
#include "map/grid.h"
#include "map/routing_terrain.h"
#include "map/tiles_road.h"
#include <stdbool.h>

static int32_t part_height(const building *part)
{
    return part->data.pyramid.blocks + part->data.pyramid.layer * PYRAMID_BLOCKS_PER_LAYER;
}

static int32_t layer_capacity(const building *part, int32_t size, int32_t is_main)
{
    if (scenario_pyramid_style == PYRAMID_STYLE_BENT) {
        return pyramid_layout.bent_half_layer[(size * BENT_PYRAMID_LAYER_STRIDE) + part->data.pyramid.layer] ? 3 : 6;
    }
    if (scenario_pyramid_style == PYRAMID_STYLE_MASTABA && is_main) {
        return 1;
    }
    return 6;
}

static void add_block(building *part, int32_t size, int32_t is_main)
{
    building_pyramid *p = &part->data.pyramid;
    if (p->stage == 0) {
        int32_t blocks = ++p->blocks;
        int32_t capacity = layer_capacity(part, size, is_main);
        if (blocks >= capacity) {
            if (p->layer < p->target_layers) {
                if (blocks > capacity) {
                    p->layer++;
                    p->blocks = 1;
                }
            } else {
                p->stage = 1;
                p->blocks = (uint8_t) (capacity < 1 ? 1 : capacity);
            }
        }
    } else {
        p->blocks--;
        if (p->ramp1_height > p->blocks) {
            p->ramp1_height = 0;
        }
        if (p->ramp2_height > p->blocks) {
            p->ramp2_height = 0;
        }
    }
}

static void clear_all_ramps(int32_t main_id, int32_t size)
{
    for (int32_t i = 0; i < pyramid_part_count_of_size(size); i++) {
        building_pyramid *p = &building_get(main_id + i)->data.pyramid;
        p->ramp1_height = 0;
        p->ramp2_height = 0;
    }
}

int32_t pyramid_part_has_active_worker(int32_t building_id)
{
    building *part = building_get(building_id);
    int32_t figure_id = part->data.pyramid.worker_figure_id;
    if (figure_id == 0) {
        return 0;
    }
    figure *f = figure_get(figure_id);
    if (f->state == 0) {
        part->data.pyramid.worker_figure_id = 0;
        return 0;
    }
    int32_t action = f->action_state;
    if (action == FIGURE_ACTION_ON_FERRY || action == FIGURE_ACTION_WAIT_FOR_FERRY) {
        action = f->action_state_before_attack;
    }
    switch (f->type) {
        case FIGURE_CARPENTER:
            return action == FIGURE_ACTION_CARPENTER_GO_TO_SITE || action == FIGURE_ACTION_CARPENTER_CLIMB_RAMP || action == FIGURE_ACTION_CARPENTER_WORK;
        case FIGURE_BRICKLAYER:
            return action == FIGURE_ACTION_BRICKLAYER_GO_TO_SITE || action == FIGURE_ACTION_BRICKLAYER_CLIMB_RAMP ||
                action == FIGURE_ACTION_BRICKLAYER_WALK_TO_PART || action == FIGURE_ACTION_BRICKLAYER_WORK || action == FIGURE_ACTION_BRICKLAYER_FIND_PART ||
                action == FIGURE_ACTION_BRICKLAYER_WAIT_FOR_BLOCK;
        case FIGURE_STONEMASON:
            if (pyramid_part_needs_ramp(building_id) == 0) {
                if (action == FIGURE_ACTION_STONEMASON_GO_TO_SITE || action == FIGURE_ACTION_STONEMASON_CLIMB_RAMP ||
                    action == FIGURE_ACTION_STONEMASON_WALK_TO_PART || action == FIGURE_ACTION_STONEMASON_WORK ||
                    action == FIGURE_ACTION_STONEMASON_FIND_PART || action == FIGURE_ACTION_STONEMASON_NEXT_CASING_PART) {
                    return 1;
                }
                if (action != FIGURE_ACTION_STONEMASON_WAIT_FOR_BLOCK) {
                    return 0;
                }
                if (map_grids.building_ids[f->grid_offset] == building_id) {
                    return 1;
                }
            }
            part->data.pyramid.worker_figure_id = 0;
            return 0;
        case FIGURE_LABORER:
            return action == FIGURE_ACTION_LABORER_TO_STORAGE_YARD || action == FIGURE_ACTION_LABORER_AT_STORAGE_YARD ||
                action == FIGURE_ACTION_LABORER_TO_PYRAMID || action == FIGURE_ACTION_LABORER_SLED_AT_RAMP || action == FIGURE_ACTION_LABORER_CLIMB_RAMP ||
                action == FIGURE_ACTION_LABORER_TO_MASON || action == FIGURE_ACTION_LABORER_WALK_TO_MASON || action == FIGURE_ACTION_LABORER_TO_LEVEL_TILE ||
                action == FIGURE_ACTION_LABORER_DELIVERED || action == FIGURE_ACTION_LABORER_LEVEL_WORK;
        default:
            return 0;
    }
}

void pyramid_part_place_block(int32_t building_id, int32_t figure_id)
{
    figure *f = figure_get(figure_id);
    uint8_t done_action;
    uint8_t next_action;
    uint8_t climb_action;
    if (f->type == FIGURE_STONEMASON) {
        done_action = FIGURE_ACTION_STONEMASON_RETURN;
        next_action = FIGURE_ACTION_STONEMASON_NEXT_CASING_PART;
        climb_action = FIGURE_ACTION_STONEMASON_WALK_TO_PART;
    } else if (f->type == FIGURE_BRICKLAYER) {
        climb_action = FIGURE_ACTION_BRICKLAYER_WALK_TO_PART;
        done_action = FIGURE_ACTION_BRICKLAYER_RETURN;
        next_action = FIGURE_ACTION_BRICKLAYER_RETURN;
    } else {
        climb_action = 0;
        next_action = 0;
        done_action = 0;
    }
    building *part = building_get(building_id);
    int32_t main_id = building_id - part->subtype;
    building *main = building_get(main_id);
    int32_t size = main->data.pyramid.size_variant;
    if (main->monument_remaining_blocks < 1) {
        f->action_state = done_action;
        f->pyramid_walk_mode = 0;
        return;
    }
    if (part->data.pyramid.worker_figure_id != figure_id && pyramid_part_has_active_worker(building_id)) {
        if (next_action == done_action) {
            f->action_state = done_action;
            f->pyramid_walk_mode = 0;
        } else if (part->data.pyramid.stage != 1) {
            pyramid_figure_find_next_part(figure_id, 1, done_action, next_action, climb_action);
        } else {
            f->action_state = next_action;
            pyramid_figure_set_walk_target(figure_id, pyramid_next_on_ring(f->pyramid_work.part), f->pyramid_work.height, 0);
        }
        return;
    }
    int32_t ticks = (part->data.pyramid.stage == 0) ? pyramid_block_ticks_a + pyramid_block_ticks_b : pyramid_casing_ticks;
    if (++f->wait_ticks < ticks) {
        return;
    }
    part->data.pyramid.worker_figure_id = 0;
    main->monument_remaining_blocks--;
    f->wait_ticks = 0;
    add_block(part, size, main_id == building_id);
    int32_t orientation = map_info.orientation / 2;
    pyramid_part_update_image(building_id, orientation, size);
    if (main->monument_remaining_blocks == 0) {
        pyramid_remove_entry_tile(main_id, 0);
    }
    if (scenario_pyramid_style == PYRAMID_STYLE_STEPPED && main->monument_remaining_blocks == 0) {
        for (int32_t i = 0; i < pyramid_part_count_of_size(size); i++) {
            building_pyramid *p = &building_get(main_id + i)->data.pyramid;
            if (p->ramp1_height != 0 || p->ramp2_height != 0) {
                p->ramp1_height = 0;
                p->ramp2_height = 0;
                pyramid_part_update_image(main_id + i, orientation, size);
            }
        }
        return;
    }
    if (part->data.pyramid.stage == 1 && building_get(monument_main_part_id(building_id))->data.pyramid.stage >= 1) {
        if (next_action == 0) {
            f->action_state = done_action;
            f->pyramid_walk_mode = 0;
            return;
        }
        f->action_state = next_action;
        pyramid_figure_set_walk_target(figure_id, pyramid_next_on_ring(f->pyramid_work.part), f->pyramid_work.height, 1);
        return;
    }
    pyramid_figure_find_next_part(figure_id, 0, done_action, next_action, climb_action);
}

void pyramid_part_add_block_instant(int32_t building_id)
{
    building *part = building_get(building_id);
    int32_t main_id = building_id - part->subtype;
    building *main = building_get(main_id);
    int32_t size = main->data.pyramid.size_variant;
    main->monument_remaining_blocks--;
    add_block(part, size, main_id == building_id);
    if (main->monument_remaining_blocks == 0) {
        pyramid_remove_entry_tile(main_id, 0);
    }
    if (scenario_pyramid_style == PYRAMID_STYLE_STEPPED && main->monument_remaining_blocks == 0) {
        clear_all_ramps(main_id, size);
    }
}

void pyramid_part_build_ramp(int32_t building_id)
{
    if (scenario_pyramid_style == PYRAMID_STYLE_MASTABA) {
        return;
    }
    building *part = building_get(building_id);
    int32_t size = part->data.pyramid.size_variant;
    if (pyramid_part_needs_ramp(building_id) == 0) {
        return;
    }
    const pyramid_ramp *ramp = pyramid_ramp_table[size];
    for (int32_t i = 0; i < pyramid_ramp_count[size]; i++, ramp++) {
        if (part->subtype != ramp->part) {
            continue;
        }
        int32_t height = 0;
        int32_t link = 0;
        int32_t is_first = 0;
        if (ramp->ramp1_height > 0 && ramp->ramp1_height <= part_height(part) && part->data.pyramid.ramp1_height == 0) {
            height = ramp->ramp1_height;
            link = ramp->ramp1_linked_part;
            part->data.pyramid.ramp1_height = (uint8_t) height;
            is_first = 1;
        } else if (ramp->ramp2_height > 0 && ramp->ramp2_height <= part_height(part) && part->data.pyramid.ramp2_height == 0) {
            height = ramp->ramp2_height;
            link = ramp->ramp2_linked_part;
            part->data.pyramid.ramp2_height = (uint8_t) height;
        }
        if (height == 0) {
            continue;
        }
        int32_t orientation = map_info.orientation / 2;
        pyramid_part_update_image(building_id, orientation, size);
        if (scenario_pyramid_style != PYRAMID_STYLE_STEPPED || link == 0) {
            return;
        }
        int32_t linked_id = link + building_id - part->subtype;
        building *linked = building_get(linked_id);
        if (is_first) {
            linked->data.pyramid.ramp1_height = (uint8_t) height;
        } else {
            linked->data.pyramid.ramp2_height = (uint8_t) height;
        }
        pyramid_part_update_image(linked_id, orientation, size);
        return;
    }
}

void pyramid_carpenter_work_tick(int32_t building_id, int32_t figure_id)
{
    figure *f = figure_get(figure_id);
    building *part = building_get(building_id);
    if (pyramid_part_needs_ramp(building_id) == 0 || (part->data.pyramid.worker_figure_id != figure_id && pyramid_part_has_active_worker(building_id))) {
        f->action_state = FIGURE_ACTION_CARPENTER_RETURN;
        f->pyramid_walk_mode = 0;
        return;
    }
    if (++f->wait_ticks >= 30) {
        pyramid_part_build_ramp(building_id);
        f->action_state = FIGURE_ACTION_CARPENTER_RETURN;
        part->data.pyramid.worker_figure_id = 0;
        f->pyramid_walk_mode = 0;
    }
}

static int32_t frontier_order(const building_pyramid *part, const building_pyramid *best)
{
    if (part->stage != best->stage) {
        return (part->stage != 0) ? 1 : -1;
    }
    if (part->stage == 0) {
        if (part->layer != best->layer) {
            return (part->layer < best->layer) ? -1 : 1;
        }
        return (part->blocks < best->blocks) ? -1 : part->blocks != best->blocks;
    }
    if (part->layer != best->layer) {
        return (part->layer > best->layer) ? -1 : 1;
    }
    return (part->blocks > best->blocks) ? -1 : part->blocks != best->blocks;
}

void pyramid_find_frontier_parts(int32_t building_id)
{
    pyramid_frontier_count = 0;
    int32_t size = building_get(building_id)->data.pyramid.size_variant;
    int32_t main_id = monument_main_part_id(building_id);
    int32_t best = 0;
    for (int32_t id = main_id; id < main_id + pyramid_part_count_of_size(size); id++) {
        const building_pyramid *p = &building_get(id)->data.pyramid;
        if (p->stage != 0 && (p->blocks == 0 || scenario_pyramid_style == PYRAMID_STYLE_STEPPED || scenario_pyramid_style == PYRAMID_STYLE_MASTABA)) {
            continue;
        }
        int32_t order = best ? frontier_order(p, &building_get(best)->data.pyramid) : -1;
        if (order > 0) {
            continue;
        }
        if (order < 0) {
            pyramid_frontier_count = 0;
            best = id;
        }
        pyramid_frontier_parts[pyramid_frontier_count++] = id;
    }
}

void pyramid_sort_frontier_parts(void)
{
    if (pyramid_frontier_count < 2) {
        return;
    }
    int32_t first = pyramid_frontier_parts[0];
    const building *part = building_get(first);
    int32_t main_id = first - part->subtype;
    int32_t height = part_height(part);
    int32_t count;
    const pyramid_entry_point *entry;
    if (scenario_pyramid_style == PYRAMID_STYLE_MASTABA) {
        count = MASTABA_ENTRY_POINT_COUNT;
        entry = mastaba_entry.entry_points[part->data.pyramid.size_variant];
    } else {
        count = PYRAMID_ENTRY_POINT_COUNT;
        entry = pyramid_entry_points[part->data.pyramid.size_variant];
    }
    int32_t origin_id = first;
    for (int32_t i = 0; i < count; i++, entry++) {
        origin_id = entry->part + main_id;
        if (entry->height >= height) {
            break;
        }
    }
    const building *origin = building_get(origin_id);
    pyramid_sort_origin_x = origin->x;
    pyramid_complex_refresh_queue.sort.sort_origin_y = origin->y;
    crt_qsort(pyramid_frontier_parts, pyramid_frontier_count, sizeof(pyramid_frontier_parts[0]), pyramid_frontier_compare);
}

int32_t pyramid_part_needs_main_material(int32_t building_id)
{
    const building *part = building_get(building_id);
    const building_pyramid *p = &part->data.pyramid;
    if (p->layer >= p->target_layers) {
        return 0;
    }
    int32_t capacity = layer_capacity(part, p->size_variant, part->subtype == 0);
    return p->blocks != capacity || p->layer + 1 != p->target_layers;
}

int32_t pyramid_find_block_task(int32_t figure_id, int32_t is_mason)
{
    figure *f = figure_get(figure_id);
    for (int32_t id = 1; id < MAX_BUILDINGS; id++) {
        building *main = building_get(id);
        if (main->state != 1 || main->type != BUILDING_PYRAMID) {
            continue;
        }
        if (main->monument_remaining_blocks != 0) {
            pyramid_find_frontier_parts(id);
            for (int32_t i = 0; i < pyramid_frontier_count; i++) {
                int32_t part_id = pyramid_frontier_parts[i];
                building *part = building_get(part_id);
                if (pyramid_part_needs_ramp(part_id) != 0) {
                    continue;
                }
                if (part->data.pyramid.worker_figure_id != 0) {
                    if (!pyramid_part_has_active_worker(part_id)) {
                        part->data.pyramid.worker_figure_id = 0;
                    }
                    if (part->data.pyramid.worker_figure_id != 0) {
                        continue;
                    }
                }
                int32_t main_material = pyramid_part_needs_main_material(part_id);
                switch (scenario_pyramid_style) {
                    case PYRAMID_STYLE_BENT:
                    case PYRAMID_STYLE_STEPPED:
                    case PYRAMID_STYLE_PYRAMID:
                    case PYRAMID_STYLE_MASTABA:
                        break;
                    case PYRAMID_STYLE_MUDBRICK:
                        if (is_mason == 0 ? !main_material : main_material) {
                            continue;
                        }
                        break;
                    default:
                        continue;
                }
                part->data.pyramid.worker_figure_id = (int16_t) figure_id;
                int32_t size = main->data.pyramid.size_variant;
                const building *entry;
                if (scenario_pyramid_style == PYRAMID_STYLE_MASTABA) {
                    entry = building_get(id + mastaba_entry.entry_points[size][0].part);
                    f->destination_x = entry->x;
                    f->destination_y = entry->y + 2;
                } else {
                    entry = building_get(id + pyramid_ramp_table[size][0].part);
                    f->destination_x = entry->road_access_x;
                    f->destination_y = entry->road_access_y;
                }
                f->destination_grid_offset = entry->grid_offset;
                f->roam_length = 0;
                return part_id;
            }
        }
        int32_t size = main->data.pyramid.size_variant;
        if (is_mason == 1 && main->data.pyramid.stage < PYRAMID_STAGE_SITE_PREPARATION && pyramid_has_complex_temple(size)) {
            int32_t offset = pyramid_complex_temple_offset[size] + main->grid_offset - map_info.grid_start;
            pyramid_complex_area_op(offset % GRID_SIZE, offset / GRID_SIZE, PYRAMID_COMPLEX_AREA_FIND_FREE_TILE, 0);
            int32_t tile = pyramid_complex_free_tile;
            if (tile != 0) {
                int32_t taken = 0;
                for (int32_t other = 1; other < MAX_FIGURES; other++) {
                    const figure *g = figure_get(other);
                    if (other != figure_id && g->state != 0 && g->type == f->type && g->destination_grid_offset == tile && g->destination_building_id == id) {
                        taken = 1;
                        break;
                    }
                }
                if (!taken) {
                    int32_t local = tile - map_info.grid_start;
                    f->destination_x = (int16_t) (local % GRID_SIZE);
                    f->destination_y = (int16_t) (local / GRID_SIZE);
                    f->destination_grid_offset = tile;
                    f->roam_length = 0;
                    f->destination_building_id = (int16_t) id;
                    return id;
                }
            }
        }
        id += pyramid_part_count_of_size(main->data.pyramid.size_variant) - 1;
    }
    return 0;
}

int32_t pyramid_find_task_stonemason(int32_t figure_id)
{
    if (scenario_pyramid_style == PYRAMID_STYLE_MASTABA) {
        return 0;
    }
    return pyramid_find_block_task(figure_id, 1);
}

int32_t pyramid_find_task_bricklayer(int32_t figure_id)
{
    return pyramid_find_block_task(figure_id, 0);
}

int32_t pyramid_find_ramp_task(int32_t figure_id)
{
    if (scenario_pyramid_style == PYRAMID_STYLE_MASTABA) {
        return 0;
    }
    for (int32_t id = 1; id < MAX_BUILDINGS; id++) {
        const building *main = building_get(id);
        if (main->state != 1 || main->type != BUILDING_PYRAMID) {
            continue;
        }
        int32_t size = main->data.pyramid.size_variant;
        for (int32_t i = 0; i < pyramid_ramp_count[size]; i++) {
            int32_t part_id = pyramid_ramp_table[size][i].part + id;
            if (pyramid_part_needs_ramp(part_id) == 0) {
                continue;
            }
            building *part = building_get(part_id);
            if (part->data.pyramid.worker_figure_id == 0 || !pyramid_part_has_active_worker(part_id)) {
                part->data.pyramid.worker_figure_id = (int16_t) figure_id;
                return pyramid_set_figure_destination_entry(figure_id, part_id);
            }
            break;
        }
        id += pyramid_part_count_of_size(main->data.pyramid.size_variant) - 1;
    }
    return 0;
}

void pyramid_check_site_prepared(int32_t building_id)
{
    int32_t size = building_get(building_id)->data.pyramid.size_variant;
    for (int32_t i = 0; i < pyramid_part_count_of_size(size); i++) {
        const building_pyramid *p = &building_get(building_id + i)->data.pyramid;
        if (scenario_pyramid_style == PYRAMID_STYLE_MASTABA) {
            if (p->stage < PYRAMID_STAGE_LEVEL_1 || p->prep_mask < PYRAMID_PREP_ALL_QUADRANTS) {
                return;
            }
        } else if (i < 4) {
            if (p->stage < PYRAMID_STAGE_LEVEL_TOP) {
                return;
            }
        } else if (p->stage < PYRAMID_STAGE_LEVEL_6 || p->prep_mask < PYRAMID_PREP_ALL_QUADRANTS) {
            return;
        }
    }
    pyramid_start_block_phase(building_id);
}

void pyramid_start_block_phase(int32_t building_id)
{
    int32_t size = building_get(building_id)->data.pyramid.size_variant;
    for (int32_t i = 0; i < pyramid_part_count_of_size(size); i++) {
        building_get(building_id + i)->data.pyramid.stage = 0;
    }
    pyramid_update_all_part_images(building_id);
}

int32_t pyramid_complex_temple_free_tile(const building *main, int32_t size)
{
    int32_t offset = pyramid_complex_temple_offset[size] + main->grid_offset - map_info.grid_start;
    pyramid_complex_area_op(offset % GRID_SIZE, offset / GRID_SIZE, PYRAMID_COMPLEX_AREA_FIND_FREE_TILE, 0);
    return pyramid_complex_free_tile;
}

bool pyramid_blocks_finished(int32_t building_id)
{
    const building *main = building_get(building_id);
    if (main->data.pyramid.stage > 1) {
        return 0;
    }
    pyramid_find_frontier_parts(building_id);
    if (pyramid_frontier_count != 0) {
        return 0;
    }
    int32_t size = main->data.pyramid.size_variant;
    if (pyramid_has_complex_temple(size) && pyramid_complex_temple_free_tile(main, size) != 0) {
        return 0;
    }
    return 1;
}

int32_t pyramid_is_complete(int32_t building_id)
{
    const building *b = building_get(building_id);
    if (b->type != BUILDING_PYRAMID) {
        return 0;
    }
    const building *main = building_get(building_id - b->subtype);
    int32_t size = main->data.pyramid.size_variant;
    if (main->monument_remaining_blocks >= 1) {
        return 0;
    }
    if (pyramid_has_complex_temple(size)) {
        return pyramid_complex_temple_free_tile(main, size) == 0;
    }
    return 1;
}

int32_t pyramid_has_ramp_work(int32_t building_id)
{
    if (scenario_pyramid_style == PYRAMID_STYLE_MASTABA) {
        return 0;
    }
    int32_t size = building_get(building_id)->data.pyramid.size_variant;
    for (int32_t i = 0; i < pyramid_ramp_count[size]; i++) {
        if (pyramid_part_needs_ramp(pyramid_ramp_table[size][i].part + building_id) != 0) {
            return 1;
        }
    }
    return 0;
}

static void build_complex_temple(int32_t building_id, int32_t figure_id, uint8_t next_action)
{
    figure *f = figure_get(figure_id);
    const building *main = building_get(building_id);
    int32_t size = main->data.pyramid.size_variant;
    if (!pyramid_has_complex_temple(size)) {
        f->action_state = next_action;
        return;
    }
    int32_t tile = pyramid_complex_temple_free_tile(main, size);
    if (tile != f->destination_grid_offset || tile == 0) {
        f->action_state = next_action;
        return;
    }
    if (++f->wait_ticks < 200) {
        return;
    }
    int32_t x = (tile - map_info.grid_start) % GRID_SIZE;
    int32_t y = (tile - map_info.grid_start) / GRID_SIZE;
    int32_t new_id = building_create(city_current_player, pyramid_complex_build_type, x, y, 0, 0);
    building_construction_add_to_map(city_current_player, pyramid_complex_build_type, new_id, x, y, 2, 1, 0);
    map_tiles_update_region_canals(0, 0, map_info.grid_width - 1, map_info.grid_height - 1, 0);
    const building *b = building_get(new_id);
    int32_t s = b->size;
    map_routing_update_land_citizen_rect(b->x - s, b->y - s, b->x + s, b->y + s, 1);
    map_routing_update_land_noncitizen_rect(b->x - s, b->y - s, b->x + s, b->y + s);
    f->action_state = next_action;
    building_construction_add_to_map(city_current_player, pyramid_complex_build_type, new_id, x, y, 2, 0, 0);
}

void pyramid_build_complex_temple(int32_t building_id, int32_t figure_id, uint8_t next_action)
{
    build_complex_temple(building_id, figure_id, next_action);
}

bool monument_resource_available(int32_t resource, int32_t amount)
{
    const city_player *player = city_player_current();
    if (amount > player->storage_stored[resource]) {
        return 1;
    }
    if (amount <= 0) {
        return 0;
    }
    return player->stockpiled[resource] != 0;
}

static bool next_block_material_available(int32_t def, const pyramid_progress *progress, int32_t main_id)
{
    for (int32_t k = 0; k < MONUMENT_DEF_RESOURCES; k++) {
        int32_t resource = monument_defs[def].resources[k];
        if (resource == RESOURCE_NONE || resource == RESOURCE_TIMBER || !monument_needs_resource(main_id, resource)) {
            continue;
        }
        int32_t amount = (resource == RESOURCE_LIMESTONE) ? progress->material_casing : progress->material_main;
        if (monument_resource_available(resource, amount)) {
            return 1;
        }
    }
    return 0;
}

bool pyramid_can_build_next(int32_t def, pyramid_progress progress, int32_t main_id)
{
    return next_block_material_available(def, &progress, main_id);
}
