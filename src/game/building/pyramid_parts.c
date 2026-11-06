// Pyramid part helpers: the next part along a layer's ring, the part pairs worked as one block, the frontier
// order and pair parts, whether a laborer brings a block to a worker, and the image refresh of a pyramid, of all
// pyramids and of a queue of complex areas to mark again.

#include "building/pyramid_parts.h"
#include "building/building_data.h"
#include "building/building_type.h"
#include "building/monument.h"
#include "building/monument_data.h"
#include "building/monument_internal.h"
#include "building/pyramid_images.h"
#include "figure/figure.h"
#include "figure/figure_action.h"
#include "figure/figure_type.h"
#include "map/grid.h"
#include <stdbool.h>

int32_t pyramid_complex_refresh_count;

static int32_t pyramid_part_count_of(int32_t size_variant)
{
    return (scenario_pyramid_style == PYRAMID_STYLE_MASTABA) ? mastaba_part_count[size_variant] : pyramid_part_count[size_variant];
}

static const pyramid_paired_parts *pair_table(int32_t size_variant)
{
    return (scenario_pyramid_style == PYRAMID_STYLE_MASTABA) ? mastaba_part_pair_table[size_variant] : pyramid_part_pair_table[size_variant];
}

int32_t pyramid_next_on_ring(int32_t building_id)
{
    int32_t part = building_get(building_id)->subtype;
    int32_t main_id = building_id - part;
    int32_t ring = 0;
    while (part < pyramid_part_rings[ring].down_first || part > pyramid_part_rings[ring].up_last) {
        ring++;
        if (ring >= PYRAMID_RING_COUNT) {
            return building_id;
        }
    }
    const pyramid_part_ring *r = &pyramid_part_rings[ring];
    if (part >= r->down_first && part <= r->down_last) {
        return (part == r->down_first) ? r->down_last + main_id + 1 : building_id - 1;
    }
    if (part >= r->up_first && part <= r->up_last) {
        return (part == r->up_last) ? r->up_first + main_id - 1 : building_id + 1;
    }
    return (part & 1) ? building_id - 2 : building_id + 2;
}

static int32_t distance_from_origin(int32_t building_id)
{
    building *b = building_get(building_id);
    int32_t dx = pyramid_sort_origin_x - b->x;
    int32_t dy = pyramid_complex_refresh_queue.sort.sort_origin_y - b->y;
    return dy * dy + dx * dx;
}

int pyramid_frontier_compare(const void *a, const void *b)
{
    int32_t part_a = *(const int32_t *) a;
    int32_t part_b = *(const int32_t *) b;
    int32_t da = distance_from_origin(part_a);
    int32_t pair = pyramid_part_pair(part_a);
    if (pair > 0) {
        int32_t d = distance_from_origin(pair);
        if (da <= d) {
            da = d;
        }
    }
    int32_t db = distance_from_origin(part_b);
    pair = pyramid_part_pair(part_b);
    if (pair > 0) {
        int32_t d = distance_from_origin(pair);
        if (db <= d) {
            db = d;
        }
    }
    return db - da;
}

static int32_t laborer_carries_block(int32_t action)
{
    switch (action) {
        case FIGURE_ACTION_LABORER_TO_STORAGE_YARD:
        case FIGURE_ACTION_LABORER_AT_STORAGE_YARD:
        case FIGURE_ACTION_LABORER_TO_PYRAMID:
        case FIGURE_ACTION_LABORER_SLED_AT_RAMP:
        case FIGURE_ACTION_LABORER_CLIMB_RAMP:
        case FIGURE_ACTION_LABORER_TO_MASON:
        case FIGURE_ACTION_LABORER_DELIVERED:
        case FIGURE_ACTION_LABORER_WALK_TO_MASON:
            return 1;
        default:
            return 0;
    }
}

bool pyramid_laborer_brings_block_to(int32_t laborer_id, int32_t worker_id)
{
    figure *f = figure_get(laborer_id);
    if (f->state == FIGURE_STATE_NONE || f->type != FIGURE_LABORER) {
        return 0;
    }
    int32_t action = f->action_state;
    if (action == FIGURE_ACTION_ON_FERRY || action == FIGURE_ACTION_WAIT_FOR_FERRY) {
        action = f->action_state_before_attack;
    }
    if (laborer_carries_block(action)) {
        return f->leading_figure_id == worker_id;
    }
    return 0;
}

int32_t pyramid_part_pair(int32_t building_id)
{
    building *b = building_get(building_id);
    const pyramid_paired_parts *table = pair_table(b->data.pyramid.size_variant);
    int32_t part = b->subtype;
    int32_t main_id = building_id - part;
    for (int32_t i = 0; table[i].first >= 0; i++) {
        if (table[i].second == part) {
            return table[i].first + main_id;
        }
        if (table[i].first == part) {
            return table[i].second + main_id;
        }
    }
    return -1;
}

int32_t pyramid_frontier_pair_part(int32_t building_id, uint8_t *is_first)
{
    building *b = building_get(building_id);
    *is_first = 0;
    const pyramid_paired_parts *table = pair_table(b->data.pyramid.size_variant);
    if (table[0].first < 0) {
        return -1;
    }
    for (int32_t i = 0;; i++) {
        if (table[i].second == b->subtype) {
            for (int32_t j = 0; j < pyramid_frontier_count; j++) {
                int32_t id = pyramid_frontier_parts[j];
                int32_t subtype = building_get(id)->subtype;
                if (subtype == table[i].first) {
                    *is_first = 1;
                    return id;
                }
            }
            return -1;
        }
        if (table[i].first == b->subtype) {
            for (int32_t j = 0; j < pyramid_frontier_count; j++) {
                int32_t id = pyramid_frontier_parts[j];
                if (building_get(id)->subtype == table[i].second) {
                    return id;
                }
            }
            return -1;
        }
        if (table[i + 1].first < 0) {
            return -1;
        }
    }
}

bool pyramid_part_is_pair_second(int32_t building_id)
{
    uint8_t is_first;
    if (pyramid_frontier_pair_part(building_id, &is_first) < 1) {
        return 0;
    }
    return is_first != 0;
}

int32_t pyramid_pair_waiting_mason(int32_t building_id)
{
    uint8_t is_first;
    int32_t pair = pyramid_frontier_pair_part(building_id, &is_first);
    if (pair < 1) {
        return -1;
    }
    int32_t worker_id = building_get(pair)->data.pyramid.worker_figure_id;
    if (worker_id == 0) {
        return 0;
    }
    figure *f = figure_get(worker_id);
    if (f->type == FIGURE_BRICKLAYER) {
        return (f->action_state == FIGURE_ACTION_BRICKLAYER_WAIT_FOR_BLOCK) ? worker_id : 0;
    }
    if (f->type == FIGURE_STONEMASON) {
        return (f->action_state == FIGURE_ACTION_STONEMASON_WAIT_FOR_BLOCK) ? worker_id : 0;
    }
    return 0;
}

static void refresh_complex_area(int32_t main_id, int32_t size_variant)
{
    int32_t offset = pyramid_complex_temple_offset[size_variant] + building_get(main_id)->grid_offset - map_info.grid_start;
    pyramid_complex_area_op(offset % GRID_SIZE, offset / GRID_SIZE, PYRAMID_COMPLEX_AREA_MARK_FREE_GROUND, 0);
}

void pyramid_complex_queue_refresh(int32_t building_id)
{
    building *b = building_get(building_id);
    if (b->type != BUILDING_PYRAMID) {
        return;
    }
    for (int32_t i = 0; i < pyramid_complex_refresh_count; i++) {
        if (pyramid_complex_refresh_queue.queue[i] == building_id - b->subtype) {
            return;
        }
    }
    if (pyramid_complex_refresh_count < PYRAMID_COMPLEX_REFRESH_QUEUE) {
        pyramid_complex_refresh_queue.queue[pyramid_complex_refresh_count] = building_id - b->subtype;
        pyramid_complex_refresh_count++;
    }
}

void pyramid_complex_process_refresh(void)
{
    for (int32_t i = 0; i < pyramid_complex_refresh_count; i++) {
        int32_t main_id = pyramid_complex_refresh_queue.queue[i];
        int32_t size = building_get(main_id)->data.pyramid.size_variant;
        if (pyramid_has_complex_temple(size)) {
            refresh_complex_area(main_id, size);
        }
    }
    pyramid_complex_refresh_count = 0;
}

void pyramid_update_all_part_images(int32_t building_id)
{
    int32_t size = building_get(building_id)->data.pyramid.size_variant;
    int32_t view = map_info.orientation / 2;
    for (int32_t id = building_id; id < pyramid_part_count_of(size) + building_id; id++) {
        pyramid_part_update_image(id, view, size);
    }
    if (pyramid_has_complex_temple(size)) {
        refresh_complex_area(building_id, size);
    }
}

void pyramids_update_all_images(void)
{
    for (int32_t id = 1; id < MAX_BUILDINGS; id++) {
        building *b = building_get(id);
        if (b->state != BUILDING_STATE_UNUSED && b->type == BUILDING_PYRAMID) {
            pyramid_update_all_part_images(id);
            id += pyramid_part_count_of(b->data.pyramid.size_variant) - 1;
        }
    }
}
