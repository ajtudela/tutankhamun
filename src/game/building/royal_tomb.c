// Royal tombs (Cleopatra): small, medium, large and grand.
//
// Placement of the tomb and its parts, the construction stages of each part
// (stonemasons, then tomb artisans), worker spots per chamber kind, the material
// stock and the drawing of parts and completed tombs.
//
// Worth knowing when reading the chamber code:
// - chamber table 13 starts with the work of table 2;
// - chamber workers are counted only while the main part has a worker.

#include "building/royal_tomb.h"
#include "building/building_data.h"
#include "building/building_image_group.h"
#include "building/building_type.h"
#include "building/construction_place.h"
#include "building/monument.h"
#include "building/monument_data.h"
#include "building/monument_internal.h"
#include "building/monument_object.h"
#include "building/monument_place.h"
#include "building/monument_spots.h"
#include "city/kingdom.h"
#include "city/resource_type.h"
#include "figure/construction_worker.h"
#include "figure/figure.h"
#include "figure/figure_action.h"
#include "figure/figure_core.h"
#include "figure/figure_type.h"
#include "game/undo_build.h"
#include "graphics/image_collection.h"
#include "map/building_tiles.h"
#include "map/grid.h"
#include "map/map_data.h"
#include "map/terrain.h"
#include "map/terrain_image_group.h"
#include "map/tiles.h"
#include "platform/log.h"
#include "widget/widget_data.h"
#include <stdbool.h>

int32_t royal_tomb_spot_table_of(int32_t chamber_kind)
{
    if ((uint32_t) chamber_kind >= ROYAL_TOMB_CHAMBER_KINDS) {
        return -1;
    }
    return royal_tomb_chamber_kinds[chamber_kind].spot_table;
}

void royal_tomb_reset_work_spots(void)
{
    static const int8_t order[ROYAL_TOMB_SPOT_TABLES] = {
        0,
        1,
        2,
        12,
        13,
        3,
        4,
        5,
        6,
        14,
        15,
        7,
        8,
        19,
        16,
        20,
        21,
        9,
        10,
        11,
        17,
        18,
        22,
        23,
        24,
    };
    for (int32_t i = 0; i < ROYAL_TOMB_SPOT_TABLES; i++) {
        int32_t t = order[i];
        int32_t work_table = (t == TOMB_SPOT_TABLE_SHARING_WORK) ? TOMB_SPOT_TABLE_WORK_SOURCE : t;
        for (int32_t s = 0; s < ROYAL_TOMB_SPOTS_PER_TABLE; s++) {
            monument_spot *spot = royal_tomb_chamber_work_spots[t * ROYAL_TOMB_SPOTS_PER_TABLE + s].spot;
            spot->figure_id = 0;
            monument_spot_start_anim(spot, MONUMENT_SPOT_IDLE);
            spot->work_remaining = royal_tomb_chamber_work_spots[work_table * ROYAL_TOMB_SPOTS_PER_TABLE + s].initial_work;
        }
    }
}

static int32_t part_has_work_for(const monument_work_spot *entry, const building *b, int32_t stage)
{
    return entry->stage == stage && b->data.royal_tomb.remaining_work[stage - 1] > 0;
}

static void part_advance(int32_t building_id)
{
    switch (building_get(building_id)->type) {
        case BUILDING_SMALL_ROYAL_TOMB:
            small_royal_tomb_part_advance(&small_royal_tomb_object.base, building_id);
            break;
        case BUILDING_MEDIUM_ROYAL_TOMB:
            medium_royal_tomb_part_advance(&medium_royal_tomb_object.base, building_id);
            break;
        case BUILDING_LARGE_ROYAL_TOMB:
            large_royal_tomb_part_advance(&large_royal_tomb_object.base, building_id);
            break;
        case BUILDING_GRAND_ROYAL_TOMB:
            grand_royal_tomb_part_advance(&grand_royal_tomb_object.base, building_id);
            break;
        default:
            break;
    }
}

void royal_tomb_part_check_stage_done(int32_t building_id)
{
    building *b = building_get(building_id);
    int32_t stage = b->data.royal_tomb.stage;
    int32_t t = royal_tomb_spot_table_of(b->data.royal_tomb.chamber_kind);
    if (t >= 0) {
        for (int32_t s = 0; s < ROYAL_TOMB_SPOTS_PER_TABLE; s++) {
            if (part_has_work_for(&royal_tomb_chamber_work_spots[t * ROYAL_TOMB_SPOTS_PER_TABLE + s], b, stage)) {
                return;
            }
        }
    }
    part_advance(building_id);
}

int32_t royal_tomb_part_find_spot(int32_t building_id, int32_t figure_id)
{
    building *b = building_get(building_id);
    int32_t stage = b->data.royal_tomb.stage;
    if (!b->data.royal_tomb.can_take_workers) {
        return 0;
    }
    int32_t t = royal_tomb_spot_table_of(b->data.royal_tomb.chamber_kind);
    if (t < 0) {
        return 0;
    }
    int32_t type = figure_get(figure_id)->type;
    for (int32_t s = 0; s < ROYAL_TOMB_SPOTS_PER_TABLE; s++) {
        const monument_work_spot *entry = &royal_tomb_chamber_work_spots[t * ROYAL_TOMB_SPOTS_PER_TABLE + s];
        if (entry->spot->figure_id == 0 && entry->figure_type == type && part_has_work_for(entry, b, stage)) {
            return building_id;
        }
    }
    return 0;
}

static void part_work_tick(int32_t table, int32_t building_id, int32_t figure_id, uint8_t next_action)
{
    figure *f = figure_get(figure_id);
    building *b = building_get(building_id);
    int32_t stage = b->data.royal_tomb.stage;
    const monument_work_spot *spots = &royal_tomb_chamber_work_spots[table * ROYAL_TOMB_SPOTS_PER_TABLE];
    const monument_work_spot *entry = 0;
    f->is_ghost = 1;
    for (int32_t s = 0; s < ROYAL_TOMB_SPOTS_PER_TABLE; s++) {
        if (spots[s].spot->figure_id == figure_id) {
            entry = &spots[s];
            break;
        }
    }
    if (entry) {
        if (entry->stage != stage) {
            f->action_state = next_action;
            return;
        }
    } else {
        for (int32_t s = 0; s < ROYAL_TOMB_SPOTS_PER_TABLE; s++) {
            if (spots[s].spot->figure_id == 0 && spots[s].figure_type == f->type && part_has_work_for(&spots[s], b, stage)) {
                entry = &spots[s];
                break;
            }
        }
        if (!entry) {
            f->action_state = next_action;
            return;
        }
    }
    if (entry->spot->figure_id == 0) {
        monument_spot_attach_figure(entry->spot, figure_id);
        monument_spot_start_anim(entry->spot, MONUMENT_SPOT_LOOP);
    }
    if (--b->data.royal_tomb.remaining_work[stage - 1] >= 1) {
        return;
    }
    b->data.royal_tomb.worker_figure_id = 0;
    royal_tomb_part_check_stage_done(building_id);
    f->action_state = next_action;
}

void royal_tomb_figure_work_tick(tomb_object *self, int32_t building_id, int32_t figure_id, uint8_t next_action)
{
    (void) self;
    int32_t t = royal_tomb_spot_table_of(building_get(building_id)->data.royal_tomb.chamber_kind);
    if (t < 0) {
        return;
    }
    part_work_tick(t, building_id, figure_id, next_action);
}

int32_t royal_tomb_check_work_for_figure(tomb_object *self, int32_t building_id, int32_t figure_id, bool assign)
{
    figure *f = figure_get(figure_id);
    int32_t is_laborer = f->type == FIGURE_LABORER;
    if (self->vtable->part_has_active_worker(self, building_id)) {
        return 0;
    }
    building *b = building_get(building_id);
    if (is_laborer && b->subtype == 0 && !self->material_requested && b->data.royal_tomb.stock < TOMB_STOCK_REFILL_BELOW) {
        int32_t yard = building_storageyard_for_getting(building_id, RESOURCE_LAMPS, 1);
        if (yard == 0) {
            return 0;
        }
        if (!assign) {
            return 1;
        }
        f->collecting_item_id = RESOURCE_LAMPS;
        f->resource_amount = 0;
        f->destination_building_id = (int16_t) yard;
        f->leading_figure_id = (int16_t) figure_id;
        f->tomb_building_id = (int16_t) building_id;
        f->action_state = FIGURE_ACTION_LABORER_TO_STORAGE_YARD;
        f->wait_ticks = 0;
        f->destination_x = (int16_t) map_result_x;
        f->destination_y = (int16_t) map_result_y;
        f->destination_grid_offset = building_get(yard)->grid_offset;
        f->roam_length = 0;
        self->material_requested = 1;
        return 1;
    }
    building *main = building_get(monument_main_part_id(building_id));
    if (main->data.royal_tomb.stock < TOMB_LOAD) {
        return 0;
    }
    int32_t part_id = royal_tomb_part_find_spot(building_id, figure_id);
    if (part_id == 0) {
        return 0;
    }
    if (!assign) {
        return 1;
    }
    if (f->type == FIGURE_STONEMASON || f->type == FIGURE_TOMB_ARTISAN) {
        f->action_state = ROYAL_TOMB_WORKER_GO_TO_PART;
    }
    main->data.royal_tomb.stock -= TOMB_LOAD;
    building *part = building_get(part_id);
    part->data.royal_tomb.worker_figure_id = figure_id;
    f->destination_x = part->road_access_x;
    f->destination_y = part->road_access_y;
    f->destination_grid_offset = part->grid_offset;
    f->roam_length = 0;
    f->wait_ticks = 0;
    return 1;
}

static int32_t find_work_for_figure(tomb_object *self, int32_t type, int32_t figure_id)
{
    int32_t found = 0;
    for (int32_t i = 1; i < MAX_BUILDINGS && found == 0; i++) {
        building *b = building_get(i);
        if (b->state != BUILDING_STATE_IN_USE || b->type != type) {
            continue;
        }
        if (type == BUILDING_GRAND_ROYAL_TOMB && b->data.royal_tomb.stage >= TOMB_STAGE_DONE) {
            continue;
        }
        if (royal_tomb_check_work_for_figure(self, i, figure_id, 1)) {
            found = i;
        }
    }
    return found;
}

tomb_object *royal_tomb_object_for_type(int32_t type)
{
    switch (type) {
        case BUILDING_SMALL_ROYAL_TOMB:
            return &small_royal_tomb_object.base;
        case BUILDING_MEDIUM_ROYAL_TOMB:
            return &medium_royal_tomb_object.base;
        case BUILDING_LARGE_ROYAL_TOMB:
            return &large_royal_tomb_object.base;
        case BUILDING_GRAND_ROYAL_TOMB:
            return &grand_royal_tomb_object.base;
        default:
            return 0;
    }
}

int32_t royal_tomb_chambers_of_type(int32_t type)
{
    switch (type) {
        case BUILDING_SMALL_ROYAL_TOMB:
            return SMALL_TOMB_CHAMBERS;
        case BUILDING_MEDIUM_ROYAL_TOMB:
            return MEDIUM_TOMB_CHAMBERS;
        case BUILDING_LARGE_ROYAL_TOMB:
            return LARGE_TOMB_CHAMBERS;
        case BUILDING_GRAND_ROYAL_TOMB:
            return GRAND_TOMB_CHAMBERS;
        default:
            return 0;
    }
}

int32_t royal_tomb_get_progress(tomb_object *self, int32_t building_id, int32_t chambers, int32_t *out)
{
    int32_t main_id = monument_main_part_id(building_id);
    int32_t stages = 0;
    for (int32_t i = main_id; i < main_id + chambers; i++) {
        stages += building_get(i)->data.royal_tomb.stage;
    }
    double percent = (100.0 / (chambers * (double) TOMB_STAGE_DONE)) * stages;
    int32_t result = (percent > 99.9) ? MONUMENT_PROGRESS_COMPLETE : (int32_t) percent;
    int32_t stonemasons = 0, laborers = 0, artisans = 0;
    for (int32_t i = 0; i < chambers; i++) {
        if (!self->vtable->part_has_active_worker(self, main_id)) {
            continue;
        }
        int32_t worker = building_get(main_id + i)->data.royal_tomb.worker_figure_id;
        if (worker == 0) {
            break;
        }
        switch (figure_get(worker)->type) {
            case FIGURE_STONEMASON:
                stonemasons++;
                break;
            case FIGURE_LABORER:
                laborers++;
                break;
            case FIGURE_TOMB_ARTISAN:
                artisans++;
                break;
            default:
                break;
        }
    }
    if (out) {
        out[TOMB_PROGRESS_PERCENT] = result;
        out[TOMB_PROGRESS_STONEMASONS] = stonemasons;
        out[TOMB_PROGRESS_LABORERS] = laborers;
        out[TOMB_PROGRESS_ARTISANS] = artisans;
        out[TOMB_PROGRESS_STOCK] = building_get(main_id)->data.royal_tomb.stock;
    }
    return result;
}

int32_t royal_tomb_progress(int32_t type, int32_t building_id, int32_t *out)
{
    return royal_tomb_get_progress(royal_tomb_object_for_type(type), building_id, royal_tomb_chambers_of_type(type), out);
}

void royal_tomb_part_advance_stage(tomb_object *self, int32_t building_id, int32_t part_count)
{
    building *b = building_get(building_id);
    int32_t main_id = building_id - b->subtype;
    if (b->data.royal_tomb.stage < TOMB_STAGE_DONE) {
        if (++b->data.royal_tomb.stage == TOMB_STAGE_OPENS_NEIGHBOURS) {
            for (int32_t i = 0; i < TOMB_NEIGHBOURS; i++) {
                int32_t offset = b->data.royal_tomb.neighbour_offsets[i];
                if (offset > -1) {
                    building *n = building_get(main_id + offset);
                    n->data.royal_tomb.stage++;
                    n->data.royal_tomb.can_take_workers = 1;
                }
            }
        }
        if (b->data.royal_tomb.stage == TOMB_STAGE_DONE) {
            b->data.royal_tomb.can_take_workers = 0;
        }
        for (int32_t i = 0; i < part_count; i++) {
            self->vtable->update_image(self, main_id + i);
        }
    }
    if (b->data.royal_tomb.can_take_workers != 0 || !city_victory_goals_reached()) {
        return;
    }
    main_id = building_id - b->subtype;
    building *main = building_get(main_id);
    tomb_object *object = royal_tomb_object_for_type(main->type);
    if (!object) {
        return;
    }
    int32_t progress = royal_tomb_progress(main->type, main_id, 0);
    if (progress == MONUMENT_PROGRESS_COMPLETE) {
        main->data.royal_tomb.completed = 1;
        royal_tomb_update_image(object, main_id);
    }
}

static void place(tomb_object *self, int32_t type, enum royal_tomb_size size, int32_t x, int32_t y, int32_t building_id, bool no_slot)
{
    royal_tomb_parts l = royal_tomb_parts_of(self, size);
    int32_t part_count = l.count;
    int32_t slot = 0;
    if (!no_slot) {
        slot = monument_register(building_id);
    }
    const monument_part_desc *desc = l.descs;
    self->material_requested = 0;
    self->idle = 1;
    building *main = building_get(building_id);
    main->x = (int16_t) (x + desc->dx);
    main->y = (int16_t) (y + desc->dy);
    main->grid_offset = map_info.grid_start + (desc->dy + y) * GRID_SIZE + desc->dx + x;
    main->subtype = (int16_t) desc->kind;
    main->size = (uint8_t) desc->size;
    main->prev_part_building_id = 0;
    main->data.monument.city_slot = slot;
    main->data.royal_tomb.completed = 0;
    main->data.royal_tomb.stage = TOMB_STAGE_STONEMASONS;
    main->data.royal_tomb.worker_figure_id = 0;
    main->data.royal_tomb.can_take_workers = 1;
    main->data.royal_tomb.closed_image_variant = l.closed_image_variant[0];
    main->data.royal_tomb.anim_key = l.anim_key[0];
    main->data.royal_tomb.chamber_kind = l.chamber_kind[0];
    main->data.royal_tomb.completed_image = l.completed_image[0];
    main->data.royal_tomb.stock = 0;
    for (int32_t i = 0; i < TOMB_NEIGHBOURS; i++) {
        main->data.royal_tomb.neighbour_offsets[i] = l.neighbours[0][i];
    }
    for (int32_t s = 0; s < TOMB_WORK_STAGES; s++) {
        main->data.royal_tomb.remaining_work[s] = l.work[0][s];
    }
    game_undo_add_building(building_id);
    self->anim_frame = 0;
    int32_t prev_id = building_id;
    for (int32_t i = 1; i < part_count; i++) {
        desc++;
        int32_t id = building_create(city_current_player, type, desc->dx + x, desc->dy + y, 0, prev_id + 1);
        building *part = building_get(id);
        part->prev_part_building_id = (int16_t) prev_id;
        building_get(prev_id)->next_part_building_id = (int16_t) id;
        part->next_part_building_id = 0;
        part->subtype = (int16_t) desc->kind;
        part->size = (uint8_t) desc->size;
        part->data.monument.city_slot = slot;
        part->data.royal_tomb.completed = 0;
        part->data.royal_tomb.stage = TOMB_STAGE_CLOSED;
        part->data.royal_tomb.worker_figure_id = 0;
        part->data.royal_tomb.anim_key = l.anim_key[i];
        part->data.royal_tomb.can_take_workers = 0;
        part->data.royal_tomb.chamber_kind = l.chamber_kind[i];
        part->data.royal_tomb.completed_image = l.completed_image[i];
        part->data.royal_tomb.stock = 0;
        part->data.royal_tomb.closed_image_variant = l.closed_image_variant[i];
        for (int32_t n = 0; n < TOMB_NEIGHBOURS; n++) {
            part->data.royal_tomb.neighbour_offsets[n] = l.neighbours[i][n];
        }
        for (int32_t s = 0; s < TOMB_WORK_STAGES; s++) {
            part->data.royal_tomb.remaining_work[s] = l.work[i][s];
        }
        game_undo_add_building(id);
        prev_id = id;
    }
    for (int32_t i = 0; i < part_count; i++) {
        self->vtable->update_image(self, building_id + i);
    }
    map_tiles_update_all();
}

void royal_tomb_set_completed(tomb_object *self, int32_t building_id)
{
    int32_t main_id = monument_main_part_id(building_id);
    int32_t end = self->vtable->part_count(self) + main_id;
    building *main = building_get(main_id);
    main->data.royal_tomb.stock = TOMB_STOCK_FULL;
    main->data.royal_tomb.completed = 1;
    for (int32_t i = main_id; i < end; i++) {
        building *b = building_get(i);
        if (b->data.royal_tomb.chamber_kind != TOMB_CHAMBER_SEALING) {
            b->data.royal_tomb.stage = TOMB_STAGE_DONE;
        }
        b->created_sequence = MONUMENT_KEPT_FROM_EARLIER_MISSION;
        b->data.royal_tomb.completion_shown = 1;
    }
    for (int32_t i = main_id; i < end; i++) {
        self->vtable->update_image(self, i);
    }
}

void royal_tomb_update_part_images(tomb_object *self, int32_t building_id)
{
    int32_t main_id = monument_main_part_id(building_id);
    int32_t end = self->vtable->chamber_count(self) + main_id;
    for (int32_t i = main_id; i < end; i++) {
        self->vtable->update_image(self, i);
    }
}

bool royal_tomb_is_idle(tomb_object *self, int32_t building_id)
{
    if (building_get(monument_main_part_id(building_id))->data.royal_tomb.stock > 0) {
        return 0;
    }
    int32_t chambers = self->vtable->chamber_count(self);
    for (int32_t i = 0; i < chambers; i++) {
        if (self->vtable->part_has_active_worker(self, building_id + i)) {
            return 0;
        }
    }
    return 1;
}

void royal_tombs_update_all_images(void)
{
    for (int32_t i = 1; i <= building_highest_id; i++) {
        tomb_object *object = royal_tomb_object_for_type(building_get(i)->type);
        if (object) {
            royal_tomb_update_image(object, i);
        }
    }
}

void royal_tomb_update_image(tomb_object *self, int32_t building_id)
{
    building *b = building_get(building_id);
    building *main = building_get(building_id - b->subtype);
    if (b->subtype == 0) {
        self->idle = royal_tomb_is_idle(self, building_id);
    }
    int32_t size = b->size;
    int32_t base = map_info.grid_start + b->x + b->y * GRID_SIZE;
    if (main->data.royal_tomb.completed == 1 && city_overlay != ROYAL_TOMB_OVERLAY_SHOWS_BUILDINGS) {
        if (b->subtype == self->last_part) {
            int32_t tile = base + building_tile_offsets.square[0].grid_offset;
            map_grids.bitfields[tile] &= (uint8_t) ~(MAP_BITFIELD_SIZE | MAP_BITFIELD_OVERLAY);
            map_grids.edge[tile] |= MAP_EDGE_DRAW_TILE;
            map_grids.images[tile] = image_group(IMAGE_GROUP_LEVELED_GROUND) + (map_grids.random[tile] % ALTERNATE_GRASS_IMAGES);
            return;
        }
        for (int32_t row = 0; row < size; row++) {
            for (int32_t col = 0; col < size; col++) {
                int32_t tile = base + building_square_tile_offset(row, col);
                int32_t offset = map_tiles_rock_monument_image(tile);
                map_grids.bitfields[tile] &= (uint8_t) ~(MAP_BITFIELD_SIZE | MAP_BITFIELD_OVERLAY);
                map_grids.edge[tile] |= MAP_EDGE_DRAW_TILE;
                map_grids.images[tile] = image_group(IMAGE_GROUP_ROYAL_TOMB_GROUND) + offset;
            }
        }
        return;
    }
    int32_t flags;
    int32_t image = self->vtable->get_image(self, building_id, &flags);
    if (flags & MONUMENT_PART_IMAGE_PER_TILE) {
        for (int32_t row = 0; row < size; row++) {
            for (int32_t col = 0; col < size; col++) {
                int32_t tile = base + building_square_tile_offset(row, col);
                map_grids.terrain[tile] = TERRAIN_BUILDING;
                map_grids.building_ids[tile] = (int16_t) building_id;
                map_grids.bitfields[tile] &= (uint8_t) ~(MAP_BITFIELD_SIZE | MAP_BITFIELD_OVERLAY);
                map_grids.edge[tile] |= MAP_EDGE_DRAW_TILE;
                if (image != 0) {
                    map_grids.images[tile] = (image == image_group(IMAGE_GROUP_LEVELED_GROUND)) ? image + (map_grids.random[tile] % ALTERNATE_GRASS_IMAGES) : image;
                }
            }
        }
        return;
    }
    map_building_tiles_add(building_id, b->x, b->y, size, size, image, TERRAIN_WALKABLE_BUILDING);
    int32_t kind = b->data.royal_tomb.chamber_kind;
    if ((uint32_t) kind >= ROYAL_TOMB_CHAMBER_KINDS || royal_tomb_chamber_kinds[kind].closed_tiles == TOMB_CLOSED_TILES_NONE) {
        return;
    }
    if (royal_tomb_chamber_kinds[kind].closed_tiles == TOMB_CLOSED_TILES_FIRST_ONLY) {
        map_grids.terrain[base + building_tile_offsets.square[0].grid_offset] = TERRAIN_BUILDING;
        return;
    }
    uint32_t key = (uint32_t) b->data.royal_tomb.anim_key;
    if (key > TOMB_LAST_ANIM_KEY) {
        return;
    }
    for (const int8_t *t = royal_tomb_closed_tiles[royal_tomb_chamber_kinds[kind].closed_tiles][key].tiles; *t >= 0; t++) {
        map_grids.terrain[base + building_tile_offsets.square[*t].grid_offset] = TERRAIN_BUILDING;
    }
}

void royal_tomb_draw_spots(int32_t x, int32_t y, int32_t building_id)
{
    building *b = building_get(building_id);
    int32_t t = royal_tomb_spot_table_of(b->data.royal_tomb.chamber_kind);
    if (t < 0) {
        return;
    }
    for (int32_t s = 0; s < ROYAL_TOMB_SPOTS_PER_TABLE; s++) {
        const monument_work_spot *entry = &royal_tomb_chamber_work_spots[t * ROYAL_TOMB_SPOTS_PER_TABLE + s];
        if (b->data.royal_tomb.stage != entry->stage) {
            continue;
        }
        int32_t dx, dy, image_id;
        monument_spot_get_draw_info_dir(entry->spot, &dx, &dy, &image_id, b->data.royal_tomb.anim_key);
        if (image_id != 0) {
            monument_draw_image(image_id, dx + x, dy + y);
        }
    }
}



void small_royal_tomb_part_advance(tomb_object *self, int32_t building_id)
{
    royal_tomb_part_advance_stage(self, building_id, SMALL_TOMB_PARTS);
}
void medium_royal_tomb_part_advance(tomb_object *self, int32_t building_id)
{
    royal_tomb_part_advance_stage(self, building_id, MEDIUM_TOMB_PARTS);
}
void large_royal_tomb_part_advance(tomb_object *self, int32_t building_id)
{
    royal_tomb_part_advance_stage(self, building_id, LARGE_TOMB_PARTS);
}
void grand_royal_tomb_part_advance(tomb_object *self, int32_t building_id)
{
    royal_tomb_part_advance_stage(self, building_id, GRAND_TOMB_PARTS);
}

int32_t small_royal_tomb_find_work_for_figure(tomb_object *self, int32_t x, int32_t y, int32_t figure_id)
{
    return find_work_for_figure(self, BUILDING_SMALL_ROYAL_TOMB, figure_id);
}
int32_t medium_royal_tomb_find_work_for_figure(tomb_object *self, int32_t x, int32_t y, int32_t figure_id)
{
    return find_work_for_figure(self, BUILDING_MEDIUM_ROYAL_TOMB, figure_id);
}
int32_t large_royal_tomb_find_work_for_figure(tomb_object *self, int32_t x, int32_t y, int32_t figure_id)
{
    return find_work_for_figure(self, BUILDING_LARGE_ROYAL_TOMB, figure_id);
}
int32_t grand_royal_tomb_find_work_for_figure(tomb_object *self, int32_t x, int32_t y, int32_t figure_id)
{
    return find_work_for_figure(self, BUILDING_GRAND_ROYAL_TOMB, figure_id);
}


int32_t royal_tomb_sized_check_work_for_figure(tomb_object *self, int32_t building_id, int32_t figure_id, bool assign)
{
    return royal_tomb_check_work_for_figure(self, building_id, figure_id, assign);
}


void small_royal_tomb_place(tomb_object *self, int32_t x, int32_t y, int32_t building_id, bool no_slot)
{
    place(self, BUILDING_SMALL_ROYAL_TOMB, ROYAL_TOMB_SMALL, x, y, building_id, no_slot);
}
void medium_royal_tomb_place(tomb_object *self, int32_t x, int32_t y, int32_t building_id, bool no_slot)
{
    place(self, BUILDING_MEDIUM_ROYAL_TOMB, ROYAL_TOMB_MEDIUM, x, y, building_id, no_slot);
}
void large_royal_tomb_place(tomb_object *self, int32_t x, int32_t y, int32_t building_id, bool no_slot)
{
    place(self, BUILDING_LARGE_ROYAL_TOMB, ROYAL_TOMB_LARGE, x, y, building_id, no_slot);
}
void grand_royal_tomb_place(tomb_object *self, int32_t x, int32_t y, int32_t building_id, bool no_slot)
{
    place(self, BUILDING_GRAND_ROYAL_TOMB, ROYAL_TOMB_GRAND, x, y, building_id, no_slot);
}




bool small_royal_tomb_has_incomplete(tomb_object *self)
{
    return monument_object_has_incomplete(self, BUILDING_SMALL_ROYAL_TOMB, SMALL_TOMB_PARTS);
}

bool medium_royal_tomb_has_incomplete(tomb_object *self)
{
    return monument_object_has_incomplete(self, BUILDING_MEDIUM_ROYAL_TOMB, MEDIUM_TOMB_PARTS);
}

bool large_royal_tomb_has_incomplete(tomb_object *self)
{
    return monument_object_has_incomplete(self, BUILDING_LARGE_ROYAL_TOMB, LARGE_TOMB_PARTS);
}

bool grand_royal_tomb_has_incomplete(tomb_object *self)
{
    return monument_object_has_incomplete(self, BUILDING_GRAND_ROYAL_TOMB, GRAND_TOMB_PARTS);
}

int32_t small_royal_tomb_chamber_count(tomb_object *self)
{
    (void) self;
    return SMALL_TOMB_CHAMBERS;
}
int32_t small_royal_tomb_part_count(tomb_object *self)
{
    (void) self;
    return SMALL_TOMB_PARTS;
}
int32_t grand_royal_tomb_chamber_count(tomb_object *self)
{
    (void) self;
    return GRAND_TOMB_CHAMBERS;
}
int32_t grand_royal_tomb_part_count(tomb_object *self)
{
    (void) self;
    return GRAND_TOMB_PARTS;
}
int32_t large_royal_tomb_chamber_count(tomb_object *self)
{
    (void) self;
    return LARGE_TOMB_CHAMBERS;
}
int32_t large_royal_tomb_part_count(tomb_object *self)
{
    (void) self;
    return LARGE_TOMB_PARTS;
}
int32_t medium_royal_tomb_chamber_count(tomb_object *self)
{
    (void) self;
    return MEDIUM_TOMB_CHAMBERS;
}
int32_t medium_royal_tomb_part_count(tomb_object *self)
{
    (void) self;
    return MEDIUM_TOMB_PARTS;
}

int32_t royal_tomb_no_progress(tomb_object *self, int32_t building_id, int32_t *out)
{
    return monument_object_no_progress((monument_object *) self, building_id, out);
}

bool royal_tomb_anim_tick(tomb_object *self)
{
    return monument_object_anim_tick((monument_object *) self);
}

bool royal_tomb_part_worker_active(tomb_object *self, int32_t building_id)
{
    building *part = &city_buildings.records[building_id];
    int32_t figure_id = part->data.royal_tomb.worker_figure_id;
    if (figure_id == 0) {
        return 0;
    }
    const figure *worker = &city_figures.records[figure_id];
    if (worker->state != FIGURE_STATE_NONE) {
        int32_t action = worker->action_state;
        if (action == FIGURE_ACTION_ON_FERRY || action == FIGURE_ACTION_WAIT_FOR_FERRY) {
            action = worker->action_state_before_attack;
        }
        int32_t type = worker->type;
        if (type == FIGURE_STONEMASON) {
            return (action == FIGURE_ACTION_STONEMASON_GO_TO_SITE) || (action == FIGURE_ACTION_STONEMASON_WORK);
        }
        if (type == FIGURE_TOMB_ARTISAN) {
            return (action == FIGURE_ACTION_TOMB_ARTISAN_GO_TO_SITE) || (action == FIGURE_ACTION_TOMB_ARTISAN_WORK);
        }
    }
    part->data.royal_tomb.worker_figure_id = 0;
    return 0;
}

bool royal_tomb_terrain_allowed(tomb_object *self, uint32_t terrain, int32_t kind)
{
    uint32_t t = terrain & TERRAIN_BLOCKS_MONUMENT;
    if (kind == self->last_part) {
        return t == 0;
    }
    return t == (TERRAIN_ROCK_NEAR_MONUMENT | TERRAIN_ROCK);
}
