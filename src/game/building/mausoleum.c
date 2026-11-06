// The mausoleum: 116 parts placed from a descriptor table, in plain C functions without an object.
//
// Placement and orientation, the work (four main blocks built sub-tile by sub-tile by stonemasons and finished
// by carpenters, ground parts prepared by laborers who also bring sandstone), the workers' slots, progress, the
// part images by view and orientation, and moving parts and their workers when the view turns.
//
// A waiting stonemason's moved laborer is the partner of figure_current_id, not of the stonemason; a part whose worker
// slot is active with no worker in it returns progress 0, the report left unfilled.

#include "building/mausoleum.h"
#include "building/building_data.h"
#include "building/building_image_group.h"
#include "building/building_type.h"
#include "building/monument.h"
#include "building/monument_object.h"
#include "building/monument_place.h"
#include "building/monument_spots.h"
#include "city/resource_type.h"
#include "figure/figure.h"
#include "figure/figure_action.h"
#include "figure/figure_core.h"
#include "figure/figure_type.h"
#include "graphics/image_collection.h"
#include "map/building_tiles.h"
#include "map/grid.h"
#include "map/map_data.h"
#include "map/terrain.h"
#include "map/terrain_image_group.h"
#include "map/tiles.h"
#include <stdbool.h>

void mausoleum_get_part_desc(monument_part_desc *out, int32_t part, int32_t view, int32_t orientation)
{
    *out = mausoleum_part_descs[part];
    if (orientation == 0 || orientation == 4) {
        if (view == 4 || view == 2) {
            out->dx = MAUSOLEUM_BOX_WIDTH - out->dx - out->size;
            out->dy = MAUSOLEUM_BOX_DEPTH - out->dy - out->size;
            out->grid_delta = out->dx + out->dy * GRID_SIZE;
        }
        return;
    }
    if (view == 4 || view == 6) {
        out->dx = MAUSOLEUM_BOX_WIDTH - out->dx - out->size;
        out->dy = MAUSOLEUM_BOX_DEPTH - out->dy - out->size;
    }
    int32_t x = out->dx;
    out->dx = out->dy;
    out->dy = x;
    out->grid_delta = out->dx + out->dy * GRID_SIZE;
}

static int32_t count_subtiles(int32_t mask)
{
    int32_t n = 0;
    for (int32_t i = 0; i < MAUSOLEUM_BLOCK_SUBTILES; i++) {
        if (mask & (1 << i)) {
            n++;
        }
    }
    return n;
}

static int32_t block_index(int32_t kind_word)
{
    switch (kind_word & MAUSOLEUM_KIND_BLOCK_MASK) {
        case MAUSOLEUM_KIND_BLOCK_0:
            return 0;
        case MAUSOLEUM_KIND_BLOCK_1:
            return 1;
        case MAUSOLEUM_KIND_BLOCK_2:
            return 2;
        case MAUSOLEUM_KIND_BLOCK_3:
            return 3;
        default:
            return -1;
    }
}

static int32_t pair_index(int32_t kind_word)
{
    switch (kind_word & MAUSOLEUM_KIND_PAIR_MASK) {
        case MAUSOLEUM_KIND_PAIR_0:
            return 0;
        case MAUSOLEUM_KIND_PAIR_1:
            return 1;
        case MAUSOLEUM_KIND_PAIR_2:
            return 2;
        case MAUSOLEUM_KIND_PAIR_3:
            return 3;
        default:
            return -1;
    }
}

static int32_t block_finished_flag(int32_t block)
{
    return MAUSOLEUM_BLOCK_FINISHED_FIRST << block;
}

int32_t mausoleum_get_progress(int32_t building_id, mausoleum_info_progress *out)
{
    int32_t main_id = monument_main_part_id(building_id);
    uint32_t flags = building_get(main_id)->data.mausoleum.work_flags;
    int32_t low_done = flags & MAUSOLEUM_BLOCKS_LOW_DONE;
    int32_t full = (flags & MAUSOLEUM_BLOCKS_FULL) != 0;
    int32_t status = MAUSOLEUM_STATUS_SITE;
    int32_t blocks_left = 0;
    if (low_done) {
        for (int32_t block = 0; block < MAUSOLEUM_BLOCK_KINDS; block++) {
            if (!(flags & block_finished_flag(block))) {
                blocks_left++;
            }
        }
    }
    int32_t stage0 = 0, below2 = 0, missing = 0, material = 0;
    int32_t masons = 0, waiting = 0, laborers = 0, carpenters = 0;
    for (int32_t i = 0; i < MAUSOLEUM_PARTS; i++) {
        int32_t part_id = main_id + i;
        building *part = building_get(part_id);
        if (mausoleum_part_has_active_worker(part_id)) {
            int32_t worker_id = part->data.mausoleum.worker_figure_id;
            if (worker_id == 0) {
                return 0;
            }
            figure *f = figure_get(worker_id);
            if (f->type == FIGURE_CARPENTER) {
                carpenters++;
            } else if (f->type == FIGURE_STONEMASON) {
                if (f->action_state == FIGURE_ACTION_STONEMASON_WAIT_FOR_BLOCK) {
                    waiting++;
                }
                masons++;
            } else if (f->type == FIGURE_LABORER) {
                laborers++;
            }
        }
        int32_t kind = mausoleum_part_descs[part->subtype].kind & MAUSOLEUM_KIND_MASK;
        if (kind < MAUSOLEUM_BLOCK_KINDS) {
            int32_t done = count_subtiles(part->data.mausoleum.stage);
            missing += MAUSOLEUM_BLOCK_SUBTILES - done;
            material += (MAUSOLEUM_BLOCK_SUBTILES - done) * MONUMENT_MATERIAL_PER_LOAD;
            if (done) {
                status = MAUSOLEUM_STATUS_STARTED;
            }
        } else if (kind <= 12 && kind != 11) {
            if (part->data.mausoleum.stage == MAUSOLEUM_GROUND_UNTOUCHED) {
                stage0++;
                below2++;
            } else if (part->data.mausoleum.stage == MAUSOLEUM_GROUND_LEVELED) {
                below2++;
            }
        }
    }
    if (stage0 == 0 && status == MAUSOLEUM_STATUS_SITE) {
        status = MAUSOLEUM_STATUS_STARTED;
    }
    if (full || low_done) {
        status = MAUSOLEUM_STATUS_BLOCKS_RISING;
    }
    int32_t percent = ((MAUSOLEUM_WORK_UNITS - below2 - missing - stage0) * MONUMENT_PROGRESS_COMPLETE) / MAUSOLEUM_WORK_UNITS;
    if (out) {
        out->status = status;
        out->parts_at_stage_0 = stage0;
        out->missing_subtiles = missing;
        out->parts_below_stage_2 = below2;
        out->stonemasons = masons;
        out->stonemasons_waiting = waiting;
        out->laborers = laborers;
        out->material = material;
        out->percent = percent;
        out->material_total = (MAUSOLEUM_BLOCK_KINDS * MAUSOLEUM_BLOCK_SUBTILES) * MONUMENT_MATERIAL_PER_LOAD;
        out->carpenters = carpenters;
        out->blocks_left = blocks_left;
    }
    return percent;
}

static void init_part(int32_t building_id, int32_t subtype, const monument_part_desc *desc, int32_t city_slot, int32_t x, int32_t y, int32_t orientation)
{
    building *b = building_get(building_id);
    b->subtype = (int16_t) subtype;
    b->size = (uint8_t) desc->size;
    b->data.mausoleum.city_slot = city_slot;
    b->data.mausoleum.stage = 0;
    b->data.mausoleum.is_main_part = subtype == 0;
    b->data.mausoleum.worker_figure_id = 0;
    b->data.mausoleum.origin_x = x;
    b->data.mausoleum.origin_y = y;
    b->data.mausoleum.orientation = orientation;
    mausoleum_update_image(building_id);
    game_undo_add_building(building_id);
}

void mausoleum_place(int32_t x, int32_t y, int32_t building_id, bool no_slot)
{
    int32_t city_slot = 0;
    int32_t orientation;
    if (!no_slot) {
        city_slot = monument_register(building_id);
        orientation = map_info.orientation;
    } else {
        orientation = campaign_object.monuments.mausoleum_x_and_orientation >> MAUSOLEUM_SAVED_ORIENTATION_SHIFT;
    }
    monument_part_desc desc;
    mausoleum_get_part_desc(&desc, 0, map_info.orientation, orientation);
    building *main = building_get(building_id);
    main->x = (int16_t) (desc.dx + x);
    main->y = (int16_t) (desc.dy + y);
    main->grid_offset = map_info.grid_start + (desc.dy + y) * GRID_SIZE + desc.dx + x;
    main->prev_part_building_id = 0;
    init_part(building_id, 0, &desc, city_slot, x, y, orientation);
    int32_t prev_id = building_id;
    for (int32_t i = 1; i < MAUSOLEUM_PARTS; i++) {
        mausoleum_get_part_desc(&desc, i, map_info.orientation, orientation);
        int32_t id = building_create(city_current_player, BUILDING_MAUSOLEUM, desc.dx + x, desc.dy + y, 0, prev_id + 1);
        building *b = building_get(id);
        b->prev_part_building_id = (int16_t) prev_id;
        b->next_part_building_id = 0;
        building_get(prev_id)->next_part_building_id = (int16_t) id;
        init_part(id, i, &desc, city_slot, x, y, orientation);
        prev_id = id;
    }
}

void mausoleum_update_image(int32_t building_id)
{
    building *b = building_get(building_id);
    monument_part_desc desc;
    mausoleum_get_part_desc(&desc, b->subtype, map_info.orientation, b->data.mausoleum.orientation);
    int32_t flags;
    int32_t image = mausoleum_get_image(building_id, &flags);
    int32_t size = desc.size;
    if (!(flags & MONUMENT_PART_IMAGE_PER_TILE)) {
        map_building_tiles_add(building_id, b->x, b->y, size, size, image, TERRAIN_BUILDING);
        return;
    }
    int32_t x = b->x;
    int32_t y = b->y;
    map_tile_cursor_offset = map_info.grid_start + y * GRID_SIZE + x;
    for (int32_t row = 0; row < size; row++) {
        for (int32_t col = 0; col < size; col++) {
            int32_t tile = building_square_tile_offset(row, col) + map_tile_cursor_offset;
            uint32_t terrain = map_grids.terrain[tile];
            map_grids.building_ids[tile] = (int16_t) building_id;
            map_grids.bitfields[tile] &= (uint8_t) ~(MAP_BITFIELD_SIZE | MAP_BITFIELD_OVERLAY);
            terrain |= TERRAIN_MONUMENT;
            map_grids.edge[tile] |= MAP_EDGE_DRAW_TILE;
            map_grids.terrain[tile] = terrain;
            if (image == 0) {
                map_grids.images[tile] = 0;
                continue;
            }
            map_grids.images[tile] = (image == image_group(IMAGE_GROUP_LEVELED_GROUND)) ? (map_grids.random[tile] % ALTERNATE_GRASS_IMAGES) + image : image;
            if (image == image_group(IMAGE_GROUP_MAUSOLEUM_WALKWAY_ALEXANDRIA) || image == image_group(IMAGE_GROUP_MAUSOLEUM_WALKWAY)) {
                map_grids.terrain[tile] = terrain & ~MAUSOLEUM_TERRAIN_CLEARED_ON_WALKWAY;
            } else {
                map_grids.terrain[tile] = terrain & ~MAUSOLEUM_TERRAIN_CLEARED;
            }
        }
    }
    if (image == 0) {
        int32_t x1 = size + x - 1;
        int32_t y1 = y + size - 1;
        map_tiles_update_region_trees(x, y, x1, y1);
        map_tiles_update_region_empty_land(x, y, x1, y1, 0, 1);
        map_tiles_update_region_meadow(x, y, x1, y1);
    }
}

static int32_t next_subtile(int32_t kind_word, int32_t mask)
{
    int32_t pair = pair_index(kind_word);
    if (pair < 0) {
        return 0;
    }
    int32_t first = 1 << (pair * 2);
    const int32_t order[4] = { first, first << 1, first << 8, first << 9 };
    for (int32_t i = 0; i < 4; i++) {
        if (!(mask & order[i])) {
            return order[i];
        }
    }
    return 0;
}

static void release_worker(building *b, figure *f, uint8_t done_action)
{
    b->data.mausoleum.worker_figure_id = 0;
    f->action_state = done_action;
}

static void laborer_tick(int32_t building_id, building *b, figure *f, int32_t kind_word, uint8_t done_action)
{
    int32_t stage = b->data.mausoleum.stage;
    if (stage != MAUSOLEUM_GROUND_UNTOUCHED && stage != MAUSOLEUM_GROUND_LEVELED) {
        if (stage < MAUSOLEUM_GROUND_UNTOUCHED) {
            b->data.mausoleum.stage = MAUSOLEUM_GROUND_UNTOUCHED;
        } else if (stage > MAUSOLEUM_GROUND_FINISHED) {
            b->data.mausoleum.stage = MAUSOLEUM_GROUND_FINISHED;
        }
        release_worker(b, f, done_action);
        return;
    }
    f->wait_ticks++;
    if (f->wait_ticks < MAUSOLEUM_WORK_TICKS) {
        return;
    }
    stage++;
    b->data.mausoleum.stage = stage;
    int32_t kind = kind_word & MAUSOLEUM_KIND_MASK;
    if (kind == 10 || kind == 12) {
        building_get(building_id + 1)->data.mausoleum.stage = stage;
        mausoleum_update_image(building_id + 1);
    }
    mausoleum_update_image(building_id);
    int32_t main_id = building_id - b->subtype;
    int32_t started = 1, done = 1;
    for (int32_t i = 0; i < MAUSOLEUM_PARTS; i++) {
        if ((mausoleum_part_descs[i].kind & MAUSOLEUM_KIND_MASK) >= MAUSOLEUM_KIND_GROUND_FIRST) {
            int32_t s = building_get(main_id + i)->data.mausoleum.stage;
            if (s == MAUSOLEUM_GROUND_UNTOUCHED) {
                started = 0;
            }
            if (s < MAUSOLEUM_GROUND_FINISHED) {
                done = 0;
            }
        }
    }
    building *main = building_get(main_id);
    if (started) {
        main->data.mausoleum.work_flags |= MAUSOLEUM_GROUND_STARTED;
    }
    if (done) {
        main->data.mausoleum.work_flags |= MAUSOLEUM_GROUND_DONE;
    }
    release_worker(b, f, done_action);
}

void mausoleum_figure_work_tick(int32_t building_id, int32_t figure_id, uint8_t done_action)
{
    building *b = building_get(building_id);
    int32_t kind_word = mausoleum_part_descs[b->subtype].kind;
    figure *f = figure_get(figure_id);
    if (f->type == FIGURE_LABORER) {
        laborer_tick(building_id, b, f, kind_word, done_action);
        return;
    }
    int32_t is_carpenter = 0;
    if (f->type == FIGURE_CARPENTER) {
        is_carpenter = 1;
    } else if (f->type != FIGURE_STONEMASON) {
        f->action_state = done_action;
        return;
    }
    int32_t block = block_index(kind_word);
    if (block < 0) {
        release_worker(b, f, done_action);
        return;
    }
    int32_t block_id = building_id - b->subtype + block;
    if (block_id == 0) {
        release_worker(b, f, done_action);
        return;
    }
    building *block_part = building_get(block_id);
    int32_t mask = block_part->data.mausoleum.stage;
    int32_t bit = next_subtile(kind_word, mask);
    if (is_carpenter) {
        bit = block_finished_flag(block);
    }
    if (bit == 0) {
        release_worker(b, f, done_action);
        return;
    }
    if (f->wait_ticks < 0) {
        f->wait_ticks = 0;
    }
    f->wait_ticks++;
    if (f->wait_ticks < MAUSOLEUM_WORK_TICKS) {
        return;
    }
    building *main = building_get(building_id - b->subtype);
    if (is_carpenter) {
        b->data.mausoleum.worker_figure_id = 0;
        f->action_state = done_action;
        main->data.mausoleum.work_flags |= bit;
        mausoleum_update_image(block_id);
        return;
    }
    block_part->data.mausoleum.stage = mask | bit;
    mausoleum_update_image(block_id);
    int32_t full = 1, low_done = 1;
    for (int32_t i = 0; i < MAUSOLEUM_BLOCK_KINDS; i++) {
        int32_t s = building_get(building_id - b->subtype + i)->data.mausoleum.stage;
        if (s < MAUSOLEUM_BLOCK_FULL) {
            full = 0;
        }
        if ((s & MAUSOLEUM_BLOCK_LOW_DONE_MASK) != MAUSOLEUM_BLOCK_LOW_DONE_MASK) {
            low_done = 0;
        }
    }
    if (full) {
        main->data.mausoleum.work_flags |= MAUSOLEUM_BLOCKS_FULL;
    }
    if (low_done) {
        main->data.mausoleum.work_flags |= MAUSOLEUM_BLOCKS_LOW_DONE;
    }
    release_worker(b, f, done_action);
}

bool mausoleum_part_has_active_worker(int32_t building_id)
{
    building *b = building_get(building_id);
    if (b->type != BUILDING_MAUSOLEUM) {
        return 0;
    }
    int32_t worker_id = b->data.mausoleum.worker_figure_id;
    if (worker_id == 0) {
        return 0;
    }
    figure *f = figure_get(worker_id);
    if (f->state != FIGURE_STATE_NONE) {
        int32_t action = f->action_state;
        if (action == FIGURE_ACTION_ON_FERRY || action == FIGURE_ACTION_WAIT_FOR_FERRY) {
            action = f->action_state_before_attack;
        }
        switch (f->type) {
            case FIGURE_LABORER:
                if (action == FIGURE_ACTION_LABORER_TO_STORAGE_YARD || action == FIGURE_ACTION_LABORER_AT_STORAGE_YARD ||
                    action == FIGURE_ACTION_LABORER_SLED_TO_MAUSOLEUM ||
                    action == FIGURE_ACTION_LABORER_TO_MAUSOLEUM || action == FIGURE_ACTION_LABORER_MAUSOLEUM_WORK) {
                    return 1;
                }
                break;
            case FIGURE_STONEMASON:
                if (action == FIGURE_ACTION_STONEMASON_GO_TO_SITE || action == FIGURE_ACTION_STONEMASON_WORK ||
                    action == FIGURE_ACTION_STONEMASON_WAIT_FOR_BLOCK) {
                    return 1;
                }
                break;
            case FIGURE_CARPENTER:
                if (action == FIGURE_ACTION_CARPENTER_GO_TO_SITE || action == FIGURE_ACTION_CARPENTER_WORK) {
                    return 1;
                }
                break;
        }
    }
    b->data.mausoleum.worker_figure_id = 0;
    return 0;
}

static void send_to_part(building *b, figure *f, int32_t figure_id, int32_t action)
{
    f->action_state = (uint8_t) action;
    b->data.mausoleum.worker_figure_id = figure_id;
    f->destination_x = b->road_access_x;
    f->destination_y = b->road_access_y;
    f->destination_grid_offset = b->grid_offset;
    f->roam_length = 0;
}

bool mausoleum_check_work_for_figure(int32_t building_id, int32_t figure_id, bool assign)
{
    figure *f = figure_get(figure_id);
    int32_t is_laborer = f->type == FIGURE_LABORER;
    int32_t is_carpenter = f->type == FIGURE_CARPENTER;
    int32_t is_mason = f->type == FIGURE_STONEMASON;
    building *b = building_get(building_id);
    uint32_t flags = building_get(building_id - b->subtype)->data.mausoleum.work_flags;
    int32_t low_done = flags & MAUSOLEUM_BLOCKS_LOW_DONE;
    int32_t ground_started = (flags & MAUSOLEUM_GROUND_STARTED) != 0;
    int32_t full = (flags & MAUSOLEUM_BLOCKS_FULL) != 0;
    int32_t all_finished = (flags & MAUSOLEUM_BLOCKS_FINISHED) == MAUSOLEUM_BLOCKS_FINISHED;
    if (b->subtype == 0 && full) {
        for (int32_t i = 0; i < MAUSOLEUM_BLOCK_KINDS; i++) {
            int32_t part_id = i - b->subtype + building_id;
            building *part = building_get(part_id);
            if (part->data.mausoleum.stage < MAUSOLEUM_BLOCK_FULL) {
                part->data.mausoleum.stage = MAUSOLEUM_BLOCK_FULL;
                mausoleum_update_image(part_id);
            }
        }
    }
    if (is_carpenter && (all_finished || !low_done)) {
        return 0;
    }
    int32_t kind_word = mausoleum_part_descs[b->subtype].kind;
    int32_t kind = kind_word & MAUSOLEUM_KIND_MASK;
    if (kind < MAUSOLEUM_BLOCK_KINDS || kind == 11 || kind == 13) {
        return 0;
    }
    if ((kind_word & MAUSOLEUM_KIND_BLOCK_MASK) != 0 && b->data.mausoleum.stage >= MAUSOLEUM_GROUND_LEVELED && !full) {
        int32_t block = block_index(kind_word);
        if (block < 0) {
            return 0;
        }
        if (flags & block_finished_flag(block)) {
            if (is_carpenter) {
                return 0;
            }
        } else if (is_mason && low_done) {
            return 0;
        }
        int32_t block_id = building_id - b->subtype + block;
        if (is_laborer) {
            if (!mausoleum_part_has_active_worker(building_id)) {
                return 0;
            }
            int32_t result = monument_check_material_laborer(b->data.mausoleum.worker_figure_id, figure_id, building_id, RESOURCE_SANDSTONE,
                FIGURE_ACTION_LABORER_SLED_TO_MAUSOLEUM, assign);
            if (result == MATERIAL_NO_WORK) {
                return 0;
            }
            if (result == MATERIAL_ASSIGNED) {
                return 1;
            }
        } else if (is_mason) {
            int32_t mask = building_get(block_id)->data.mausoleum.stage;
            int32_t pair = pair_index(kind_word);
            if (pair < 0) {
                return 0;
            }
            int32_t low = MAUSOLEUM_PAIR_LOW_SUBTILES << (pair * 2);
            int32_t high = MAUSOLEUM_PAIR_HIGH_SUBTILES << (pair * 2);
            if ((mask & low) == low && ((mask & high) == high || !low_done)) {
                return 0;
            }
            if (mausoleum_part_has_active_worker(building_id)) {
                return 0;
            }
            if (assign) {
                send_to_part(b, f, figure_id, FIGURE_ACTION_STONEMASON_GO_TO_SITE);
            }
            return 1;
        } else if (is_carpenter) {
            if (mausoleum_part_has_active_worker(building_id)) {
                return 0;
            }
            if (assign) {
                send_to_part(b, f, figure_id, FIGURE_ACTION_CARPENTER_GO_TO_SITE);
            }
            return 1;
        } else {
            return 0;
        }
    }
    if (!is_laborer) {
        return 0;
    }
    int32_t stage = b->data.mausoleum.stage;
    if (stage == MAUSOLEUM_GROUND_FINISHED) {
        return 0;
    }
    if (stage == MAUSOLEUM_GROUND_LEVELED && (!ground_started || !full)) {
        return 0;
    }
    if (mausoleum_part_has_active_worker(building_id)) {
        return 0;
    }
    if (assign) {
        send_to_part(b, f, figure_id, FIGURE_ACTION_LABORER_TO_MAUSOLEUM);
    }
    return 1;
}

int32_t mausoleum_find_work_for_figure(int32_t figure_id)
{
    for (int32_t i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (b->state == BUILDING_STATE_IN_USE && b->type == BUILDING_MAUSOLEUM && mausoleum_check_work_for_figure(i, figure_id, 1)) {
            return i;
        }
    }
    return 0;
}

int32_t mausoleum_get_image(int32_t building_id, int32_t *flags)
{
    static const int32_t rotation[16] = { 0, 1, 0, 1, 3, 2, 3, 2, 0, 1, 0, 1, 3, 2, 3, 2 };
    *flags = 0;
    building *b = building_get(building_id);
    int32_t orientation = b->data.mausoleum.orientation;
    monument_part_desc desc;
    mausoleum_get_part_desc(&desc, b->subtype, map_info.orientation, orientation);
    int32_t kind = desc.kind & MAUSOLEUM_KIND_MASK;
    int32_t in_use = b->created_sequence != MONUMENT_KEPT_FROM_EARLIER_MISSION;
    switch (kind) {
        case 0:
        case 1:
        case 2:
        case 3: {
            if (!in_use) {
                return image_group(MAUSOLEUM_IMAGE_GROUP_BLOCK_DONE_FIRST + kind) - (((map_info.orientation + orientation) / 2) & 1) + 1;
            }
            int32_t stage = b->data.mausoleum.stage;
            int32_t variant;
            if (stage < MAUSOLEUM_BLOCK_LOW_DONE_MASK) {
                variant = 0;
            } else if (!(building_get(building_id - b->subtype)->data.mausoleum.work_flags & block_finished_flag(kind))) {
                variant = 0;
            } else {
                variant = (stage >= MAUSOLEUM_BLOCK_FULL) + 1;
            }
            int32_t script = ((variant + (scenario_mausoleum_size_cached * 3)) * MAUSOLEUM_BLOCK_KINDS) + kind;
            anim_load_script(mausoleum_block_scripts[script], MONUMENT_SCRIPT_SLOT_STAGE_A + kind);
            return image_group(MAUSOLEUM_IMAGE_GROUP_BLOCK_FIRST + kind) - (((orientation + map_info.orientation) / 2) & 1) + 1;
        }
        case 4:
        case 5:
        case 6:
        case 7:
            if (in_use) {
                *flags = MONUMENT_PART_IMAGE_PER_TILE | MONUMENT_PART_IMAGE_UNBUILT;
            }
            if (b->data.mausoleum.stage == MAUSOLEUM_GROUND_FINISHED) {
                return image_group(IMAGE_GROUP_MAUSOLEUM_GROUND) + (kind - MAUSOLEUM_KIND_GROUND_FIRST);
            }
            break;
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
            if (in_use) {
                *flags = MONUMENT_PART_IMAGE_PER_TILE;
            }
            if (b->data.mausoleum.stage == MAUSOLEUM_GROUND_FINISHED) {
                int32_t rot = rotation[(map_info.orientation / 2) + ((orientation / 2) * 4)];
                int32_t opposite = (rot - 2) & 3;
                switch (kind) {
                    case 8:
                        return rot + MAUSOLEUM_GROUND_IMAGE_SIDES + image_group(IMAGE_GROUP_MAUSOLEUM_GROUND);
                    case 9:
                        return opposite + MAUSOLEUM_GROUND_IMAGE_SIDES + image_group(IMAGE_GROUP_MAUSOLEUM_GROUND);
                    case 10:
                        return image_group(IMAGE_GROUP_MAUSOLEUM_GROUND) + MAUSOLEUM_GROUND_IMAGE_ENDS + 1 + (opposite * 2);
                    case 11:
                        return image_group(IMAGE_GROUP_MAUSOLEUM_GROUND) + MAUSOLEUM_GROUND_IMAGE_ENDS + (opposite * 2);
                    case 12:
                        return image_group(IMAGE_GROUP_MAUSOLEUM_GROUND) + MAUSOLEUM_GROUND_IMAGE_ENDS + (rot * 2);
                    default:
                        return image_group(IMAGE_GROUP_MAUSOLEUM_GROUND) + MAUSOLEUM_GROUND_IMAGE_ENDS + 1 + (rot * 2);
                }
            }
            *flags |= MONUMENT_PART_IMAGE_UNBUILT;
            break;
        default:
            return 0;
    }
    if (b->data.mausoleum.stage == MAUSOLEUM_GROUND_LEVELED) {
        return image_group(IMAGE_GROUP_LEVELED_GROUND);
    }
    if (desc.kind & MAUSOLEUM_KIND_WALKWAY) {
        return (scenario_data.animation_set != MONUMENT_ERA_ALEXANDRIA) ? image_group(IMAGE_GROUP_MAUSOLEUM_WALKWAY) : image_group(IMAGE_GROUP_MAUSOLEUM_WALKWAY_ALEXANDRIA);
    }
    return 0;
}

void mausoleum_part_update_position(int32_t building_id)
{
    building *b = building_get(building_id);
    monument_part_desc desc;
    mausoleum_get_part_desc(&desc, b->subtype, map_info.orientation, b->data.mausoleum.orientation);
    int32_t x = b->data.mausoleum.origin_x + desc.dx;
    int32_t y = b->data.mausoleum.origin_y + desc.dy;
    b->x = (int16_t) x;
    b->y = (int16_t) y;
    b->grid_offset = map_info.grid_start + y * GRID_SIZE + x;
    mausoleum_get_work_tile(building_id, &x, &y);
    b->road_access_x = (int16_t) x;
    b->road_access_y = (int16_t) y;
    int32_t worker_id = b->data.mausoleum.worker_figure_id;
    if (worker_id) {
        figure *f = figure_get(worker_id);
        figure *redirect = 0;
        int32_t redirect_id = 0;
        switch (f->type) {
            case FIGURE_CARPENTER:
                if (f->action_state == FIGURE_ACTION_CARPENTER_GO_TO_SITE) {
                    redirect = f;
                    redirect_id = worker_id;
                } else if (f->action_state == FIGURE_ACTION_CARPENTER_WORK) {
                    map_figure_delete(worker_id);
                    f->x = b->x;
                    f->y = b->y;
                    f->grid_offset = b->grid_offset;
                    map_figure_add(worker_id);
                }
                break;
            case FIGURE_STONEMASON:
                if (f->action_state == FIGURE_ACTION_STONEMASON_GO_TO_SITE) {
                    redirect = f;
                    redirect_id = worker_id;
                } else if (f->action_state == FIGURE_ACTION_STONEMASON_WORK || f->action_state == FIGURE_ACTION_STONEMASON_WAIT_FOR_BLOCK) {
                    map_figure_delete(worker_id);
                    f->x = b->x;
                    f->y = b->y;
                    f->grid_offset = b->grid_offset;
                    f->destination_grid_offset = b->grid_offset;
                    map_figure_add(worker_id);
                    if (f->action_state == FIGURE_ACTION_STONEMASON_WAIT_FOR_BLOCK) {
                        int32_t partner_id = figure_get(figure_current_id)->leading_figure_id;
                        if (partner_id) {
                            figure *partner = figure_get(partner_id);
                            if (partner->type == FIGURE_LABORER && partner->action_state == FIGURE_ACTION_LABORER_SLED_TO_MAUSOLEUM) {
                                partner->destination_x = b->road_access_x;
                                partner->destination_y = b->road_access_y;
                                partner->destination_grid_offset = b->grid_offset;
                                redirect = partner;
                                redirect_id = partner_id;
                            }
                        }
                    }
                }
                break;
            case FIGURE_LABORER:
                if (f->action_state == FIGURE_ACTION_LABORER_TO_MAUSOLEUM) {
                    redirect = f;
                    redirect_id = worker_id;
                } else if (f->action_state == FIGURE_ACTION_LABORER_MAUSOLEUM_WORK) {
                    map_figure_delete(worker_id);
                    f->x = b->x;
                    f->y = b->y;
                    f->grid_offset = b->grid_offset;
                    map_figure_add(worker_id);
                }
                break;
        }
        if (redirect) {
            if (redirect == f) {
                f->destination_x = b->road_access_x;
                f->destination_y = (int16_t) y;
                f->destination_grid_offset = b->grid_offset;
            }
            figure_route_remove(redirect_id);
            redirect->roam_length = 0;
        }
    }
    mausoleum_update_image(building_id);
}

void mausoleum_get_work_tile(int32_t building_id, int32_t *x, int32_t *y)
{
    const building *b = building_get(building_id);
    *x = b->x;
    *y = b->y;
}

void mausoleum_get_entry_tile(int32_t building_id, int32_t *x, int32_t *y)
{
    building *b = building_get(building_id);
    *x = b->data.mausoleum.origin_x + MAUSOLEUM_ENTRY_DX;
    *y = b->data.mausoleum.origin_y + MAUSOLEUM_ENTRY_DY;
}
