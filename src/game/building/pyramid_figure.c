// The construction figures on pyramids: walking up the ramps, climbing from part to part (without routes, 15
// steps per tile of distance) and pairing the laborers who bring blocks with the masons waiting for them.
//
// The search for the next entry of a table reads into the next table at the end of one.

#include "building/pyramid_figure.h"
#include "building/building_type.h"
#include "building/monument_internal.h"
#include "building/pyramid.h"
#include "building/pyramid_blocks.h"
#include "building/pyramid_parts.h"
#include "building/building_data.h"
#include "figure/figure.h"
#include "figure/figure_action.h"
#include "figure/figure_type.h"
#include "figure/movement_internal.h"
#include "map/grid.h"
#include "core/calc.h"
#include <math.h>
#include <stdlib.h>

enum {
    PYRAMID_WALK_NONE = 0,
    PYRAMID_WALK_START = 1,
    PYRAMID_WALK_ALONG_X = 2,
    PYRAMID_WALK_ALONG_Y = 3,
    PYRAMID_WALK_ON_LAYER = 4,
};

enum { PYRAMID_STEPS_PER_TILE = 15, PYRAMID_NO_FRONTIER_INDEX = 999, PYRAMID_LABORER_NEAR_TILES = 7 };


static int32_t part_height(const building *part)
{
    return part->data.pyramid.blocks + part->data.pyramid.layer * PYRAMID_BLOCKS_PER_LAYER;
}

static int32_t part_count(int32_t size_variant)
{
    return (scenario_pyramid_style == PYRAMID_STYLE_MASTABA) ? mastaba_part_count[size_variant] : pyramid_part_count[size_variant];
}

static const pyramid_entry_point *entry_points(int32_t size_variant, int32_t *count)
{
    if (scenario_pyramid_style == PYRAMID_STYLE_MASTABA) {
        *count = MASTABA_ENTRY_POINT_COUNT;
        return mastaba_entry.entry_points[size_variant];
    }
    *count = PYRAMID_ENTRY_POINT_COUNT;
    return pyramid_entry_points[size_variant];
}

static int32_t rotated_tile_kind(const building *part)
{
    int32_t kind;
    if (scenario_pyramid_style == PYRAMID_STYLE_MASTABA) {
        kind = pyramid_layout.mastaba[part->data.pyramid.size_variant][part->subtype].kind;
    } else {
        kind = pyramid_layout.pyramid[part->data.pyramid.size_variant][part->subtype].kind;
    }
    int32_t rotation = map_info.orientation / 2;
    if (kind < PYRAMID_PART_EDGE_TOP_LEFT) {
        kind += rotation;
        if (kind >= PYRAMID_PART_EDGE_TOP_LEFT) {
            kind -= PYRAMID_VIEW_COUNT;
        }
    } else if (kind < PYRAMID_PART_INNER) {
        kind += rotation;
        if (kind >= PYRAMID_PART_INNER) {
            kind -= PYRAMID_VIEW_COUNT;
        }
    }
    return kind;
}

static void neighbour_part_tile(int32_t grid_offset, int32_t rotation, int32_t main_id, int32_t last_id, int32_t *x, int32_t *y)
{
    const int32_t *offsets = pyramid_neighbour_offsets[rotation];
    for (int32_t i = 0; i < 3; i++) {
        int32_t id = map_grids.building_ids[grid_offset + offsets[i]];
        if (id >= main_id && id <= last_id) {
            const building *part = building_get(id);
            *x = part->x;
            *y = part->y;
            return;
        }
    }
}

static int32_t part_mason_is_working(const building *part)
{
    int32_t worker_id = part->data.pyramid.worker_figure_id;
    if (worker_id <= 0) {
        return 0;
    }
    const figure *worker = figure_get(worker_id);
    return (worker->type == FIGURE_STONEMASON && worker->action_state == FIGURE_ACTION_STONEMASON_WORK) ||
        (worker->type == FIGURE_BRICKLAYER && worker->action_state == FIGURE_ACTION_BRICKLAYER_WORK);
}

static int32_t mason_waits_for_block(const figure *mason)
{
    return (mason->type == FIGURE_STONEMASON && mason->action_state == FIGURE_ACTION_STONEMASON_WAIT_FOR_BLOCK) ||
        (mason->type == FIGURE_BRICKLAYER && mason->action_state == FIGURE_ACTION_BRICKLAYER_WAIT_FOR_BLOCK);
}

void pyramid_figure_set_walk_target(int32_t figure_id, int32_t building_id, int32_t height, int32_t set_walk_mode)
{
    figure *f = figure_get(figure_id);
    const building *from_part = building_get(f->pyramid_work.previous_part);
    f->pyramid_work.previous_part = f->pyramid_work.part;
    f->pyramid_work.previous_height = f->pyramid_work.height;
    f->pyramid_work.part = (int16_t) building_id;
    f->pyramid_work.height = (int16_t) height;

    const building *part = building_get(building_id);
    f->destination_x = part->x;
    f->destination_y = part->y;
    f->destination_grid_offset = part->grid_offset;
    int32_t dx = abs(f->destination_x - f->x);
    int32_t dy = abs(f->destination_y - f->y);
    f->pyramid_work.progress = 0;
    f->pyramid_work.total = (int16_t) ((int32_t) sqrt((double) ((dx * dx) + (dy * dy))) * PYRAMID_STEPS_PER_TILE);
    f->direction = (int8_t) calc_missile_shooter_direction(from_part->x, from_part->y, part->x, part->y);
    if (!set_walk_mode) {
        return;
    }
    switch (rotated_tile_kind(part)) {
        case PYRAMID_PART_CORNER_TOP:
        case PYRAMID_PART_CORNER_LEFT:
        case PYRAMID_PART_EDGE_TOP_LEFT:
        case PYRAMID_PART_EDGE_TOP_RIGHT:
            f->pyramid_walk_mode = PYRAMID_WALK_ALONG_Y;
            break;
        default:
            f->pyramid_walk_mode = PYRAMID_WALK_ALONG_X;
            break;
    }
}

enum { RAMP_FAIL, RAMP_ON_LAYER, RAMP_WALK_TO_ENTRY, RAMP_HANDLED };

static void stop_pyramid_walk(figure *f, uint8_t fail_action)
{
    f->action_state = fail_action;
    f->pyramid_walk_mode = PYRAMID_WALK_NONE;
}

static void walk_on_layer(figure *f, uint8_t next_action)
{
    f->pyramid_walk_mode = PYRAMID_WALK_ON_LAYER;
    f->action_state = next_action;
}

static int32_t worker_has_ramp_part(int32_t figure_id, int32_t main_id, const pyramid_entry_point *entries, int32_t count)
{
    for (int32_t i = 0; i < count; i++) {
        int32_t id = main_id + entries[i].part;
        if (building_get(id)->data.pyramid.worker_figure_id == (int16_t) figure_id && pyramid_part_needs_ramp(id)) {
            return 1;
        }
    }
    return 0;
}

static int32_t ramp_next_step(int32_t figure_id, int32_t part_id, int32_t mode, const pyramid_entry_point *entries, int32_t count, int32_t *index)
{
    figure *f = figure_get(figure_id);
    const building *part = building_get(part_id);
    int32_t main_id = part_id - part->subtype;
    if (mode == 1) {
        if (part->data.pyramid.worker_figure_id == (int16_t) figure_id) {
            return pyramid_part_needs_ramp(part_id) ? RAMP_ON_LAYER : RAMP_FAIL;
        }
        if (!worker_has_ramp_part(figure_id, main_id, entries, count)) {
            return RAMP_FAIL;
        }
    }

    const building *first_frontier = building_get(0);
    if (f->pyramid_walk_mode == PYRAMID_WALK_START) {
        pyramid_find_frontier_parts(main_id);
        if (pyramid_frontier_count < 1) {
            return RAMP_FAIL;
        }
        if (f->pyramid_work.height >= part_height(building_get(pyramid_frontier_parts[0]))) {
            return RAMP_ON_LAYER;
        }
        *index = 0;
        return RAMP_WALK_TO_ENTRY;
    }
    if (mode == 0 || mode == 2) {
        pyramid_find_frontier_parts(main_id);
        if (pyramid_frontier_count < 1) {
            return RAMP_FAIL;
        }
        first_frontier = building_get(pyramid_frontier_parts[0]);
        if (f->pyramid_work.height >= part_height(first_frontier)) {
            return RAMP_ON_LAYER;
        }
    }
    int32_t i;
    for (i = 0; i < count; i++) {
        if (entries[i].part == part->subtype && entries[i].height == f->pyramid_work.height) {
            break;
        }
    }
    if (i >= count) {
        return RAMP_FAIL;
    }
    i++;
    if (entries[i].part >= 0) {
        *index = i;
        return RAMP_WALK_TO_ENTRY;
    }
    if (mode == 1) {
        return RAMP_FAIL;
    }
    if (mode == 0 && f->type == FIGURE_STONEMASON && first_frontier->data.pyramid.stage == PYRAMID_STAGE_CASING) {
        f->action_state = FIGURE_ACTION_STONEMASON_NEXT_CASING_PART;
        pyramid_figure_set_walk_target(figure_id, pyramid_next_on_ring(f->pyramid_work.part), f->pyramid_work.height, 0);
        return RAMP_HANDLED;
    }
    return RAMP_ON_LAYER;
}

static void walk_to_entry(int32_t figure_id, int32_t part_id, int32_t mode, const pyramid_entry_point *entry)
{
    figure *f = figure_get(figure_id);
    int32_t next_id = entry->part - building_get(part_id)->subtype + part_id;
    if (mode == 2 && pyramid_part_needs_ramp(next_id)) {
        f->animation_frame = 0;
        return;
    }
    int32_t same_part = next_id == f->pyramid_work.part;
    pyramid_figure_set_walk_target(figure_id, next_id, entry->height, 1);
    if (same_part) {
        f->pyramid_work.progress = f->pyramid_work.total;
    }
}

void pyramid_figure_walk_on_ramp(int32_t figure_id, int32_t mode, uint8_t fail_action, uint8_t next_action)
{
    figure *f = figure_get(figure_id);
    int32_t part_id = map_grids.building_ids[f->destination_grid_offset];
    const building *part = building_get(part_id);
    if (part->type != BUILDING_PYRAMID) {
        figure_get(figure_current_id)->state = FIGURE_STATE_DEAD;
        return;
    }
    if (pyramid_figure_climb_step(figure_id, 1) != 1) {
        return;
    }
    int32_t count;
    const pyramid_entry_point *entries = entry_points(part->data.pyramid.size_variant, &count);
    map_figure_delete(figure_id);
    f->x = f->destination_x;
    f->y = f->destination_y;
    f->grid_offset = f->destination_grid_offset;
    map_figure_add(figure_id);

    int32_t index = 0;
    switch (ramp_next_step(figure_id, part_id, mode, entries, count, &index)) {
        case RAMP_WALK_TO_ENTRY:
            walk_to_entry(figure_id, part_id, mode, &entries[index]);
            break;
        case RAMP_ON_LAYER:
            walk_on_layer(f, next_action);
            break;
        case RAMP_FAIL:
            stop_pyramid_walk(f, fail_action);
            break;
        default:
            break;
    }
}

static int32_t claim_next_part(int32_t figure_id, int32_t set_height, uint8_t done_action, uint8_t next_action)
{
    figure *f = figure_get(figure_id);
    int32_t part_id = map_grids.building_ids[f->grid_offset];
    const building *part = building_get(part_id);
    if (part->type != BUILDING_PYRAMID) {
        return 0;
    }
    int32_t main_id = part_id - part->subtype;
    pyramid_find_frontier_parts(main_id);
    if (pyramid_frontier_count < 1) {
        return 0;
    }
    const building *first = building_get(pyramid_frontier_parts[0]);
    int32_t first_main_id = pyramid_frontier_parts[0] - first->subtype;
    int32_t height = part_height(first);
    if (set_height) {
        f->pyramid_work.height = (int16_t) height;
    }
    if (height != f->pyramid_work.height) {
        return 0;
    }

    int32_t count;
    const pyramid_entry_point *entries = entry_points(building_get(main_id)->data.pyramid.size_variant, &count);
    int32_t ref_id = 0;
    for (int32_t i = 0; i < count; i++) {
        ref_id = first_main_id + entries[i].part;
        if (entries[i].height >= height) {
            break;
        }
    }
    const building *ref = building_get(ref_id);
    int32_t ref_x = ref->x;
    int32_t ref_y = ref->y;

    int32_t best_id = 0;
    int32_t best_distance = 0;
    int32_t best_pair_waiting = 0;
    int32_t best_has_pair = 0;
    for (int32_t i = 0; i < pyramid_frontier_count; i++) {
        int32_t id = pyramid_frontier_parts[i];
        int32_t needs_main_material = pyramid_part_needs_main_material(id);
        switch (scenario_pyramid_style) {
            case PYRAMID_STYLE_BENT:
            case PYRAMID_STYLE_STEPPED:
            case PYRAMID_STYLE_PYRAMID:
            case PYRAMID_STYLE_MASTABA:
                break;
            case PYRAMID_STYLE_MUDBRICK:
                if (f->type == FIGURE_BRICKLAYER ? !needs_main_material : needs_main_material) {
                    continue;
                }
                break;
            default:
                continue;
        }
        const building *b = building_get(id);
        int32_t dx = ref_x - b->x;
        int32_t dy = ref_y - b->y;
        int32_t distance = (dx * dx) + (dy * dy);
        uint8_t is_first;
        int32_t pair_id = pyramid_frontier_pair_part(id, &is_first);
        int32_t has_pair;
        if (pair_id > 0) {
            const building *pair = building_get(pair_id);
            int32_t pdx = ref_x - pair->x;
            int32_t pdy = ref_y - pair->y;
            int32_t pair_distance = (pdx * pdx) + (pdy * pdy);
            if (distance > pair_distance) {
                pair_distance = distance;
            }
            distance = pair_distance * 2;
            has_pair = 1;
        } else {
            distance *= 2;
            has_pair = 0;
        }
        int32_t pair_waiting = pyramid_pair_waiting_mason(id) > 0;
        if (best_id != 0) {
            int32_t best_worker = building_get(best_id)->data.pyramid.worker_figure_id;
            int32_t worker = b->data.pyramid.worker_figure_id;
            int32_t best_taken = best_worker > 0 && best_worker != figure_id;
            int32_t free = worker == 0 || worker == figure_id;
            if (!best_taken || !free) {
                if (!best_taken && !free) {
                    continue;
                }
                if (distance < best_distance) {
                    continue;
                }
                if (distance == best_distance && !(pair_waiting && !best_pair_waiting) && !(has_pair && !best_has_pair)) {
                    continue;
                }
            }
        }
        best_pair_waiting = pair_waiting;
        best_id = id;
        best_distance = distance;
        best_has_pair = has_pair;
    }
    if (best_id == 0) {
        return 0;
    }
    building *best = building_get(best_id);
    if (best->data.pyramid.worker_figure_id != 0 && best->data.pyramid.worker_figure_id != figure_id && pyramid_part_has_active_worker(best_id)) {
        return 0;
    }
    if (best->data.pyramid.stage == PYRAMID_STAGE_CASING) {
        best->data.pyramid.worker_figure_id = 0;
        f->action_state = done_action;
        return 1;
    }
    best->data.pyramid.worker_figure_id = (int16_t) figure_id;
    f->pyramid_walk_mode = PYRAMID_WALK_ON_LAYER;
    f->action_state = next_action;
    pyramid_figure_set_walk_target(figure_id, best_id, f->pyramid_work.previous_height, 0);
    return 1;
}

void pyramid_figure_find_next_part(int32_t figure_id, int32_t set_height, uint8_t fail_action, uint8_t done_action, uint8_t next_action)
{
    if (!claim_next_part(figure_id, set_height, done_action, next_action)) {
        stop_pyramid_walk(figure_get(figure_id), fail_action);
    }
}

void pyramid_figure_start_climb(int32_t figure_id)
{
    figure *f = figure_get(figure_id);
    f->pyramid_walk_mode = PYRAMID_WALK_START;
    int16_t part_id = map_grids.building_ids[f->destination_grid_offset];
    f->pyramid_work.previous_part = part_id;
    f->pyramid_work.previous_height = 0;
    f->pyramid_work.part = part_id;
    f->pyramid_work.height = 0;
    f->pyramid_work.progress = 1;
    f->pyramid_work.total = PYRAMID_STEPS_PER_TILE;
}

int32_t pyramid_figure_climb_step(int32_t figure_id, int32_t step)
{
    figure *f = figure_get(figure_id);
    if (f->pyramid_work.total < 1) {
        return 1;
    }
    int32_t reached = 0;
    f->pyramid_work.progress = (int16_t) (f->pyramid_work.progress + step);
    if (f->pyramid_work.progress >= f->pyramid_work.total) {
        reached = 1;
        f->pyramid_work.progress = f->pyramid_work.total;
    }
    int32_t prev_id = f->pyramid_work.previous_part;
    int32_t target_id = f->pyramid_work.part;
    const building *prev = building_get(prev_id);
    const building *target = building_get(target_id);
    if (prev_id != 0) {
        f->direction = (int8_t) calc_missile_shooter_direction(prev->x, prev->y, target->x, target->y);
    } else {
        f->direction = 0;
    }
    int32_t rotation = map_info.orientation / 2;
    int32_t x, y, grid_offset;
    if (prev_id == 0) {
        x = target->x;
        y = target->y;
        grid_offset = map_info.grid_start + y * GRID_SIZE + x;
    } else if (f->pyramid_walk_mode == PYRAMID_WALK_ON_LAYER) {
        int32_t main_id = prev_id - prev->subtype;
        int32_t last_id = part_count(prev->data.pyramid.size_variant) + main_id - 1;
        int32_t progress = f->pyramid_work.progress;
        int32_t total = f->pyramid_work.total;
        int32_t tile_y = ((target->y - prev->y) * progress) / total + prev->y;
        int32_t tile_x = ((target->x - prev->x) * progress) / total + prev->x;
        const building *b = building_get(map_grids.building_ids[map_info.grid_start + tile_y * GRID_SIZE + tile_x]);
        x = b->x;
        y = b->y;
        neighbour_part_tile(b->grid_offset, rotation, main_id, last_id, &x, &y);
        grid_offset = map_info.grid_start + y * GRID_SIZE + x;
    } else {
        int32_t kind = rotated_tile_kind(prev);
        int32_t corner = 0;
        int32_t facing = (int8_t) figure_get(figure_current_id)->direction - map_info.orientation + 1;
        if (facing < 0) {
            facing += 8;
        }
        int32_t facing_back = facing <= 1 || facing >= 6;
        const building *at;
        switch (kind) {
            case PYRAMID_PART_CORNER_TOP:
            case PYRAMID_PART_EDGE_TOP_RIGHT:
                corner = 1;
                at = prev;
                break;
            case PYRAMID_PART_CORNER_LEFT:
            case PYRAMID_PART_CORNER_RIGHT:
                at = target;
                break;
            case PYRAMID_PART_EDGE_BOTTOM_LEFT:
                at = facing_back ? prev : target;
                break;
            default:
                at = prev;
                break;
        }
        x = at->x;
        y = at->y;
        grid_offset = at->grid_offset;
        if (corner) {
            int32_t main_id = prev_id - prev->subtype;
            int32_t last_id = part_count(prev->data.pyramid.size_variant) + main_id - 1;
            neighbour_part_tile(prev->grid_offset, (rotation - 2) & 3, main_id, last_id, &x, &y);
            grid_offset = map_info.grid_start + y * GRID_SIZE + x;
        }
    }
    int32_t moved = f->x != x || f->y != y;
    if (moved) {
        map_figure_delete(figure_id);
    }
    f->x = (int16_t) x;
    f->y = (int16_t) y;
    f->grid_offset = grid_offset;
    if (moved) {
        map_figure_add(figure_id);
    }
    return reached;
}

void pyramid_stonemason_arrive_part(int32_t figure_id)
{
    figure *f = figure_get(figure_id);
    if (f->pyramid_walk_mode < 1) {
        f->action_state = FIGURE_ACTION_STONEMASON_RETURN;
        return;
    }
    if (pyramid_figure_climb_step(figure_id, 1) != 1) {
        return;
    }
    pyramid_figure_snap_to_part(figure_id);
    int32_t part_id = f->pyramid_work.part;
    pyramid_find_frontier_parts(monument_main_part_id(part_id));
    for (int32_t i = 0; i < pyramid_frontier_count; i++) {
        int32_t id = pyramid_frontier_parts[i];
        building *part = building_get(id);
        if (id == part_id && !part_mason_is_working(part)) {
            f->action_state = FIGURE_ACTION_STONEMASON_WORK;
            f->wait_ticks = 0;
            part->data.pyramid.worker_figure_id = (int16_t) figure_id;
            return;
        }
    }
    int32_t search_id = part_id;
    for (;;) {
        for (int32_t i = 0; i < pyramid_frontier_count; i++) {
            int32_t id = pyramid_frontier_parts[i];
            building *part = building_get(id);
            if (id == search_id && !part_mason_is_working(part)) {
                part->data.pyramid.worker_figure_id = (int16_t) figure_id;
                pyramid_figure_set_walk_target(figure_id, pyramid_next_on_ring(part_id), f->pyramid_work.height, 1);
                return;
            }
        }
        search_id = pyramid_next_on_ring(search_id);
        if (search_id == part_id) {
            f->action_state = FIGURE_ACTION_STONEMASON_RETURN;
            f->pyramid_walk_mode = PYRAMID_WALK_NONE;
            return;
        }
    }
}

void pyramid_figure_snap_to_part(int32_t figure_id)
{
    figure *f = figure_get(figure_id);
    map_figure_delete(figure_id);
    const figure *current = figure_get(figure_current_id);
    f->x = current->destination_x;
    f->y = current->destination_y;
    f->grid_offset = current->destination_grid_offset;
    map_figure_add(figure_id);
    f->pyramid_work.height = (int16_t) part_height(building_get(f->pyramid_work.part));
    f->wait_ticks = 0;
}

void pyramid_laborer_deliver_block(int32_t mason_figure_id)
{
    int32_t part_id = map_grids.building_ids[figure_get(mason_figure_id)->destination_grid_offset];
    pyramid_find_frontier_parts(monument_main_part_id(part_id));
    int32_t mason_id = pyramid_pair_waiting_mason(part_id);
    if (mason_id <= 0) {
        return;
    }
    figure *mason = figure_get(mason_id);
    if (mason->type == FIGURE_BRICKLAYER) {
        mason->action_state = FIGURE_ACTION_BRICKLAYER_WORK;
    } else if (mason->type == FIGURE_STONEMASON) {
        mason->action_state = FIGURE_ACTION_STONEMASON_WORK;
    }
    mason->wait_ticks = 0;
}

static uint8_t laborer_action(const figure *laborer)
{
    uint8_t action = laborer->action_state;
    if (action == FIGURE_ACTION_ON_FERRY || action == FIGURE_ACTION_WAIT_FOR_FERRY) {
        action = laborer->action_state_before_attack;
    }
    return action;
}

int32_t pyramid_find_mason_waiting_for_block(int32_t building_id, int32_t clear_partner)
{
    pyramid_find_frontier_parts(monument_main_part_id(building_id));
    pyramid_sort_frontier_parts();
    for (int32_t i = 0; i < pyramid_frontier_count; i++) {
        int32_t id = pyramid_frontier_parts[i];
        if (pyramid_part_is_pair_second(id)) {
            continue;
        }
        building *part = building_get(id);
        int32_t mason_id = part->data.pyramid.worker_figure_id;
        if (mason_id == 0) {
            continue;
        }
        if (!pyramid_part_has_active_worker(id)) {
            if (part->data.pyramid.worker_figure_id != 0) {
                part->data.pyramid.worker_figure_id = 0;
            }
            continue;
        }
        figure *mason = figure_get(mason_id);
        if (!mason_waits_for_block(mason)) {
            continue;
        }
        int32_t has_laborer = 0;
        int32_t laborer_on_way = 0;
        int32_t laborer_id = mason->leading_figure_id;
        if (laborer_id != 0 && pyramid_laborer_brings_block_to(laborer_id, mason_id)) {
            has_laborer = 1;
            uint8_t action = laborer_action(figure_get(laborer_id));
            if (action == FIGURE_ACTION_LABORER_CLIMB_RAMP || action == FIGURE_ACTION_LABORER_TO_MASON || action == FIGURE_ACTION_LABORER_WALK_TO_MASON ||
                action == FIGURE_ACTION_LABORER_DELIVERED || action == FIGURE_ACTION_LABORER_TO_PYRAMID || action == FIGURE_ACTION_LABORER_SLED_AT_RAMP) {
                laborer_on_way = 1;
            }
        }
        if (pyramid_pair_waiting_mason(id) == 0) {
            continue;
        }
        if (!has_laborer) {
            return mason_id;
        }
        if (!laborer_on_way && clear_partner) {
            mason->leading_figure_id = 0;
            return mason_id;
        }
    }
    return 0;
}

int32_t pyramid_laborer_can_enter_ramp(int32_t figure_id)
{
    const figure *f = figure_get(figure_id);
    if (f->routing_path_id <= 0) {
        return 1;
    }
    int32_t part_id = map_grids.building_ids[f->destination_grid_offset];
    const building *part = building_get(part_id);
    if (part_id == 0 || part->type != BUILDING_PYRAMID) {
        return 1;
    }
    int32_t main_id = part_id - part->subtype;
    int32_t remaining = f->routing_path_length - f->routing_path_current_tile;
    int32_t size = building_get(main_id)->data.pyramid.size_variant;
    for (int32_t id = main_id; id < part_count(size) + main_id; id++) {
        const figure *mason = figure_get(building_get(id)->data.pyramid.worker_figure_id);
        if (!mason_waits_for_block(mason)) {
            continue;
        }
        int32_t laborer_id = mason->leading_figure_id;
        if (laborer_id == 0) {
            continue;
        }
        const figure *laborer = figure_get(laborer_id);
        if (laborer->type == FIGURE_LABORER && laborer->action_state == FIGURE_ACTION_LABORER_TO_PYRAMID && laborer->routing_path_id > 0 &&
            abs(laborer->routing_path_length - laborer->routing_path_current_tile - remaining) < PYRAMID_LABORER_NEAR_TILES) {
            return 0;
        }
    }
    return 1;
}

void pyramid_laborer_pair_with_mason(int32_t figure_id)
{
    figure *f = figure_get(figure_id);
    int32_t part_id = map_grids.building_ids[f->destination_grid_offset];
    const building *part = building_get(part_id);
    if (part_id == 0 || part->type != BUILDING_PYRAMID) {
        return;
    }
    int32_t main_id = part_id - part->subtype;
    int32_t size = building_get(main_id)->data.pyramid.size_variant;
    figure *mason = 0;
    if (f->leading_figure_id != 0) {
        mason = figure_get(f->leading_figure_id);
        int32_t mason_part_id = map_grids.building_ids[mason->destination_grid_offset];
        if (mason_part_id == 0 || !pyramid_part_has_active_worker(mason_part_id)) {
            mason = 0;
        }
    }
    if (!mason) {
        int32_t mason_id = pyramid_find_mason_waiting_for_block(part_id, 1);
        if (mason_id == 0) {
            return;
        }
        f->leading_figure_id = (int16_t) mason_id;
        mason = figure_get(mason_id);
        mason->leading_figure_id = (int16_t) figure_id;
    }
    int32_t mason_part_id = map_grids.building_ids[mason->destination_grid_offset];
    pyramid_find_frontier_parts(main_id);
    pyramid_sort_frontier_parts();
    int32_t mason_index;
    for (mason_index = 0; mason_index < pyramid_frontier_count; mason_index++) {
        if (pyramid_frontier_parts[mason_index] == mason_part_id) {
            break;
        }
    }
    if (mason_index >= pyramid_frontier_count || mason_index == 0) {
        return;
    }
    int32_t best_index = mason_index;
    int32_t best_mason_id = 0;
    int32_t best_laborer_id = 0;
    for (int32_t id = main_id; id < part_count(size) + main_id; id++) {
        int32_t other_mason_id = building_get(id)->data.pyramid.worker_figure_id;
        if (other_mason_id == 0 || pyramid_part_is_pair_second(id)) {
            continue;
        }
        figure *other_mason = figure_get(other_mason_id);
        if (!mason_waits_for_block(other_mason)) {
            continue;
        }
        int32_t other_laborer_id = other_mason->leading_figure_id;
        if (other_laborer_id == figure_id) {
            continue;
        }
        const figure *other_laborer = figure_get(other_laborer_id);
        if (other_laborer_id != 0) {
            if (other_laborer->leading_figure_id != other_mason_id) {
                other_mason->leading_figure_id = 0;
                other_laborer_id = 0;
            } else if (other_laborer->type != FIGURE_LABORER ||
                (other_laborer->action_state != FIGURE_ACTION_LABORER_TO_STORAGE_YARD && other_laborer->action_state != FIGURE_ACTION_LABORER_AT_STORAGE_YARD &&
                    other_laborer->action_state != FIGURE_ACTION_LABORER_SLED_AT_RAMP && other_laborer->action_state != FIGURE_ACTION_LABORER_TO_PYRAMID)) {
                continue;
            }
        }
        int32_t index = PYRAMID_NO_FRONTIER_INDEX;
        for (int32_t i = 0; i < pyramid_frontier_count; i++) {
            if (pyramid_frontier_parts[i] == id) {
                index = i;
                break;
            }
        }
        if (index < best_index && other_laborer_id != 0 && f->collecting_item_id == other_laborer->collecting_item_id) {
            best_index = index;
            best_mason_id = other_mason_id;
            best_laborer_id = other_laborer_id;
        }
    }
    if (best_index == mason_index) {
        return;
    }
    int32_t old_mason_id = f->leading_figure_id;
    figure_get(best_mason_id)->leading_figure_id = (int16_t) figure_id;
    f->leading_figure_id = (int16_t) best_mason_id;
    if (best_laborer_id != 0) {
        figure_get(old_mason_id)->leading_figure_id = (int16_t) best_laborer_id;
        figure_get(best_laborer_id)->leading_figure_id = (int16_t) old_mason_id;
    }
}
