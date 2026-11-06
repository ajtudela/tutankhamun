// The Library of Alexandria: a scripted monument of 25 parts.
//
// Placement, the stages (every part prepared by a laborer, then the main part's stage advanced by stonemasons and
// carpenters while laborers bring marble and copper), the workers' slots, progress and material still needed,
// and the part images by stage and view.
//
// The stonemasons' and carpenters' walk target is the main part's id + 15, a building id rather than a part, and a main
// part whose worker slot is active but empty returns progress 0 without filling the report.

#include "building/library.h"
#include "building/building_data.h"
#include "building/building_image_group.h"
#include "building/building_type.h"
#include "building/monument.h"
#include "building/monument_object.h"
#include "building/monument_place.h"
#include "building/monument_spots.h"
#include "building/monument_work_spots.h"
#include "city/resource_type.h"
#include "figure/figure.h"
#include "figure/figure_action.h"
#include "figure/figure_type.h"
#include "graphics/image_collection.h"
#include "map/building_tiles.h"
#include "map/grid.h"
#include "map/map_data.h"
#include "map/terrain.h"
#include "map/terrain_image_group.h"
#include <stdbool.h>

void library_object_init(library_object_t *self)
{
    monument_object_init(&self->base);
    self->base.vtable = &library_vtable;
    monument_object_init_parts(self->parts, monument_part_layouts[MONUMENT_LAYOUT_LIBRARY].parts, monument_part_layouts[MONUMENT_LAYOUT_LIBRARY].count);
    self->base.frame_count = LIBRARY_FRAME_COUNT;
}

bool library_can_place(monument_object *object, int32_t grid_offset)
{
    library_object_t *self = (library_object_t *) object;
    return monument_object_can_place(&self->base, grid_offset, self->parts, LIBRARY_PARTS);
}

void library_draw_placement(monument_object *object)
{
    library_object_t *self = (library_object_t *) object;
    monument_object_draw_placement(&self->base, self->parts, LIBRARY_PARTS);
}

bool library_has_incomplete(monument_object *object)
{
    library_object_t *self = (library_object_t *) object;
    return monument_object_has_incomplete(self, BUILDING_ALEXANDRIA_LIBRARY, LIBRARY_PARTS);
}

int32_t library_get_progress(library_object_t *self, int32_t building_id, library_info_report *out)
{
    int32_t main_id = monument_main_part_id(building_id);
    building *main = building_get(main_id);
    int32_t stage = main->data.monument.stage;
    int32_t percent;
    if (stage == LIBRARY_STAGE_COMPLETE) {
        percent = MONUMENT_PROGRESS_COMPLETE;
    } else {
        percent = (int32_t) (((double) stage * (1.0 / LIBRARY_STAGE_COMPLETE)) * MONUMENT_PROGRESS_COMPLETE);
    }
    int32_t masons = 0, waiting = 0, laborers = 0, carpenters = 0;
    if (self->base.vtable->part_has_active_worker(&self->base, main_id)) {
        int32_t worker_id = main->data.monument.worker_figure_id;
        if (worker_id == 0) {
            return 0;
        }
        figure *f = figure_get(worker_id);
        if (f->type == FIGURE_CARPENTER) {
            carpenters = 1;
        } else if (f->type == FIGURE_STONEMASON) {
            masons = 1;
            if (f->action_state == FIGURE_ACTION_STONEMASON_WAIT_FOR_BLOCK) {
                waiting = 1;
            }
        } else if (f->type == FIGURE_LABORER) {
            laborers = 1;
        }
    }
    int32_t material = LIBRARY_MATERIAL_LOADS * MONUMENT_MATERIAL_PER_LOAD;
    if (stage > LIBRARY_STAGE_BUILDING) {
        material = (LIBRARY_MATERIAL_LOADS - ((stage - LIBRARY_STAGE_BUILDING) * self->base.frame_count)) * MONUMENT_MATERIAL_PER_LOAD;
        if (material < 0) {
            material = 0;
        }
    }
    if (out) {
        out->progress.stage = stage;
        out->progress.stonemasons = masons;
        out->progress.stonemasons_waiting = waiting;
        out->progress.laborers = laborers;
        out->progress.carpenters = carpenters;
        out->stock.material = material;
        out->stock.percent = percent;
        out->stock.material_total = LIBRARY_MATERIAL_LOADS * MONUMENT_MATERIAL_PER_LOAD;
    }
    return percent;
}

static void init_part(library_object_t *self, int32_t building_id, int32_t subtype, int32_t city_slot)
{
    building *b = building_get(building_id);
    b->subtype = (int16_t) subtype;
    b->size = (uint8_t) self->parts[subtype].size;
    b->data.monument.city_slot = city_slot;
    b->data.monument.stage = LIBRARY_STAGE_STARTED;
    b->data.monument.is_main_part = 1;
    b->data.monument.worker_figure_id = 0;
    int32_t x, y;
    self->base.vtable->get_work_tile(&self->base, building_id, &x, &y);
    b->road_access_x = (int16_t) x;
    b->road_access_y = (int16_t) y;
    self->base.vtable->update_image(&self->base, building_id);
    game_undo_add_building(building_id);
}

void library_place(monument_object *object, int32_t x, int32_t y, int32_t building_id, bool no_slot)
{
    library_object_t *self = (library_object_t *) object;
    int32_t city_slot = 0;
    if (!no_slot) {
        city_slot = monument_register(building_id);
    }
    building *main = building_get(building_id);
    int32_t dx = self->parts[0].dx;
    int32_t dy = self->parts[0].dy;
    main->x = (int16_t) (x + dx);
    main->y = (int16_t) (y + dy);
    main->grid_offset = map_info.grid_start + (dy + y) * GRID_SIZE + dx + x;
    main->prev_part_building_id = 0;
    init_part(self, building_id, 0, city_slot);
    self->base.frame = 0;
    int32_t prev_id = building_id;
    for (int32_t i = 1; i < LIBRARY_PARTS; i++) {
        int32_t id = building_create(city_current_player, BUILDING_ALEXANDRIA_LIBRARY, x + self->parts[i].dx, y + self->parts[i].dy, 0, prev_id + 1);
        building *b = building_get(id);
        b->prev_part_building_id = (int16_t) prev_id;
        building_get(prev_id)->next_part_building_id = (int16_t) id;
        b->next_part_building_id = 0;
        init_part(self, id, i, city_slot);
        prev_id = id;
    }
}

static int32_t keeps_tile_image(int32_t subtype, int32_t row, int32_t col)
{
    switch (subtype) {
        case 13:
            return col > 0 || row < 2;
        case 14:
            return col < 2 || row < 2;
        case 15:
            return col > 0 || row > 0;
        case 16:
            return col < 2 || row > 0;
        default:
            return 0;
    }
}

void library_update_image(monument_object *object, int32_t building_id)
{
    library_object_t *self = (library_object_t *) object;
    int32_t flags;
    int32_t image = self->base.vtable->get_image(&self->base, building_id, &flags);
    building *b = building_get(building_id);
    int32_t subtype = b->subtype;
    int32_t main_id = building_id - subtype;
    int32_t size = self->parts[subtype].size;
    if (!(flags & MONUMENT_PART_IMAGE_PER_TILE)) {
        map_building_tiles_add(building_id, b->x, b->y, size, size, image, TERRAIN_BUILDING);
        return;
    }
    map_tile_cursor_offset = map_info.grid_start + b->x + b->y * GRID_SIZE;
    for (int32_t row = 0; row < size; row++) {
        int32_t stage = building_get(main_id)->data.monument.stage;
        for (int32_t col = 0; col < size; col++) {
            int32_t tile = building_square_tile_offset(row, col) + map_tile_cursor_offset;
            if (stage < LIBRARY_STAGE_BUILDING) {
                map_grids.terrain[tile] |= TERRAIN_MONUMENT;
            } else {
                map_grids.terrain[tile] = TERRAIN_BUILDING;
            }
            map_grids.bitfields[tile] &= (uint8_t) ~(MAP_BITFIELD_SIZE | MAP_BITFIELD_OVERLAY);
            map_grids.edge[tile] |= MAP_EDGE_DRAW_TILE;
            map_grids.building_ids[tile] = (int16_t) building_id;
            if ((flags & MONUMENT_PART_IMAGE_EDGE_TILES) && (image == image_group(IMAGE_GROUP_LIBRARY_FRONT)) && keeps_tile_image(subtype, row, col)) {
                continue;
            }
            if (image != 0) {
                map_grids.images[tile] = (image == image_group(IMAGE_GROUP_LEVELED_GROUND)) ? (map_grids.random[tile] % ALTERNATE_GRASS_IMAGES) + image : image;
            }
        }
    }
}

bool library_get_work_tile(monument_object *object, int32_t building_id, int32_t *x, int32_t *y)
{
    library_object_t *self = (library_object_t *) object;
    building *b = building_get(building_id);
    if (b->data.monument.stage < LIBRARY_STAGE_BUILDING) {
        int32_t half = self->parts[b->subtype].size / 2;
        *x = b->x + half;
        *y = b->y + half;
        return 1;
    }
    building *main = building_get(building_id - b->subtype);
    *x = main->x + LIBRARY_WORK_TILE_DX;
    *y = main->y + LIBRARY_WORK_TILE_DY;
    return 1;
}

bool library_part_has_active_worker(monument_object *self, int32_t building_id)
{
    (void) self;
    building *b = building_get(building_id);
    if (b->data.monument.stage >= LIBRARY_STAGE_BUILDING) {
        b = building_get(building_id - b->subtype);
    }
    int32_t worker_id = b->data.monument.worker_figure_id;
    if (worker_id == 0) {
        return 0;
    }
    figure *f = figure_get(worker_id);
    if (f->state == FIGURE_STATE_NONE) {
        b->data.monument.worker_figure_id = 0;
        return 0;
    }
    int32_t action = f->action_state;
    if (action == FIGURE_ACTION_ON_FERRY || action == FIGURE_ACTION_WAIT_FOR_FERRY) {
        action = f->action_state_before_attack;
    }
    switch (f->type) {
        case FIGURE_LABORER:
            return action == FIGURE_ACTION_LABORER_TO_STORAGE_YARD || action == FIGURE_ACTION_LABORER_AT_STORAGE_YARD ||
                action == FIGURE_ACTION_LABORER_SLED_TO_LIBRARY || action == FIGURE_ACTION_LABORER_TO_LIBRARY || action == FIGURE_ACTION_LABORER_LIBRARY_WORK;
        case FIGURE_STONEMASON:
            return action == FIGURE_ACTION_STONEMASON_GO_TO_SITE || action == FIGURE_ACTION_STONEMASON_WORK ||
                action == FIGURE_ACTION_STONEMASON_WAIT_FOR_BLOCK;
        case FIGURE_CARPENTER:
            return action == FIGURE_ACTION_CARPENTER_GO_TO_SITE || action == FIGURE_ACTION_CARPENTER_WORK;
        default:
            b->data.monument.worker_figure_id = 0;
            return 0;
    }
}

bool library_check_work_for_figure(monument_object *object, int32_t building_id, int32_t figure_id, bool assign)
{
    library_object_t *self = (library_object_t *) object;
    building *b = building_get(building_id);
    int32_t target_id = building_id;
    if (b->data.monument.stage >= LIBRARY_STAGE_BUILDING) {
        target_id = building_id - b->subtype;
    }
    building *target = building_get(target_id);
    int32_t kind = self->parts[target->subtype].kind;
    if ((kind < 0) || (kind >= LIBRARY_PART_KINDS)) {
        return 0;
    }
    figure *f = figure_get(figure_id);
    if (f->type == FIGURE_LABORER) {
        int32_t stage = target->data.monument.stage;
        if (stage < LIBRARY_STAGE_PREPARED) {
            if (target->data.monument.worker_figure_id != 0) {
                return 0;
            }
            if (assign) {
                target->data.monument.worker_figure_id = figure_id;
                f->action_state = FIGURE_ACTION_LABORER_TO_LIBRARY;
                f->destination_building_id = (int16_t) target_id;
                f->destination_x = b->road_access_x;
                f->destination_y = b->road_access_y;
                f->destination_grid_offset = b->grid_offset;
                f->wait_ticks = 0;
                f->roam_length = 0;
            }
            return 1;
        }
        if (stage < LIBRARY_STAGE_BUILDING) {
            return 0;
        }
        if (!self->base.vtable->part_has_active_worker(&self->base, building_id)) {
            return 0;
        }
        stage = target->data.monument.stage;
        int32_t resource = ((stage < LIBRARY_STAGE_COPPER_FIRST) || (stage >= LIBRARY_STAGE_COPPER_END)) ? RESOURCE_MARBLE : RESOURCE_COPPER;
        return monument_check_material_laborer(target->data.monument.worker_figure_id, figure_id, building_id, resource, FIGURE_ACTION_LABORER_SLED_TO_LIBRARY,
                   assign) == MATERIAL_ASSIGNED;
    }
    if (self->base.vtable->part_has_active_worker(&self->base, building_id)) {
        return 0;
    }
    if (target->data.monument.stage < LIBRARY_STAGE_BUILDING) {
        return 0;
    }
    if (!library_has_free_spot(building_id, figure_id)) {
        return 0;
    }
    if (assign) {
        if (f->type == FIGURE_CARPENTER || f->type == FIGURE_STONEMASON) {
            f->action_state = FIGURE_ACTION_STONEMASON_GO_TO_SITE;
        }
        target->data.monument.worker_figure_id = figure_id;
        f->destination_x = target->road_access_x;
        f->destination_y = target->road_access_y;
        f->destination_grid_offset = target->grid_offset;
        f->destination_building_id = (int16_t) (target_id + LIBRARY_SPOT_DESTINATION_OFFSET);
        f->roam_length = 0;
        f->wait_ticks = 0;
    }
    return 1;
}

int32_t library_find_work_for_figure(monument_object *object, int32_t x, int32_t y, int32_t figure_id)
{
    library_object_t *self = (library_object_t *) object;
    (void) x;
    (void) y;
    int32_t result = 0;
    for (int32_t i = 1; i < MAX_BUILDINGS && !result; i++) {
        building *b = building_get(i);
        int32_t is_library = b->state == BUILDING_STATE_IN_USE && b->type == BUILDING_ALEXANDRIA_LIBRARY;
        if (is_library && self->base.vtable->check_work_for_figure(&self->base, i, figure_id, 1)) {
            result = i;
        }
    }
    return result;
}

static int32_t all_parts_prepared(int32_t main_id)
{
    for (int32_t i = main_id; i < main_id + LIBRARY_PARTS; i++) {
        if (building_get(i)->data.monument.stage < LIBRARY_STAGE_PREPARED) {
            return 0;
        }
    }
    return 1;
}

void library_figure_work_tick(monument_object *object, int32_t building_id, int32_t figure_id, uint8_t done_action)
{
    library_object_t *self = (library_object_t *) object;
    building *b = building_get(building_id);
    int32_t main_id = building_id - b->subtype;
    if (all_parts_prepared(main_id)) {
        library_figure_work_tick_spot(building_id, figure_id, done_action);
        return;
    }
    figure *f = figure_get(figure_id);
    if (f->type != FIGURE_LABORER || b->data.monument.stage >= LIBRARY_STAGE_PREPARED) {
        f->action_state = done_action;
        return;
    }
    f->wait_ticks++;
    if (f->wait_ticks <= LIBRARY_LABORER_WORK_TICKS) {
        return;
    }
    f->wait_ticks = 0;
    f->action_state = FIGURE_ACTION_LABORER_IDLE;
    b->data.monument.stage++;
    self->base.vtable->update_image(&self->base, building_id);
    main_id = building_id - b->subtype;
    if (!all_parts_prepared(main_id)) {
        return;
    }
    library_advance_stage(&self->base, main_id);
    for (int32_t i = main_id; i < main_id + LIBRARY_PARTS; i++) {
        int32_t x, y;
        self->base.vtable->get_work_tile(&self->base, i, &x, &y);
        building *part = building_get(i);
        part->road_access_x = (int16_t) x;
        part->road_access_y = (int16_t) y;
        self->base.vtable->update_image(&self->base, i);
    }
}

static const char *const library_script_files[LIBRARY_SCRIPTS] = {
    [LIBRARY_SCRIPT_MAIN_A1] = "LibMaina1",
    [LIBRARY_SCRIPT_MAIN_A2] = "LibMaina2",
    [LIBRARY_SCRIPT_MAIN_A3] = "LibMaina3",
    [LIBRARY_SCRIPT_MAIN_A4] = "LibMaina4",
    [LIBRARY_SCRIPT_MAIN_A5] = "LibMaina5",
    [LIBRARY_SCRIPT_MAIN_B1] = "LibMainb1",
    [LIBRARY_SCRIPT_MAIN_B2] = "LibMainb2",
    [LIBRARY_SCRIPT_MAIN_B3] = "LibMainb3",
    [LIBRARY_SCRIPT_MAIN_B4] = "LibMainb4",
    [LIBRARY_SCRIPT_MAIN_B5] = "LibMainb5",
    [LIBRARY_SCRIPT_MAIN_C1] = "LibMainc1",
    [LIBRARY_SCRIPT_MAIN_C2] = "LibMainc2",
    [LIBRARY_SCRIPT_MAIN_C3] = "LibMainc3",
    [LIBRARY_SCRIPT_MAIN_C4] = "LibMainc4",
    [LIBRARY_SCRIPT_MAIN_C5] = "LibMainc5",
    [LIBRARY_SCRIPT_EXTRA] = "LibExtra",
    [LIBRARY_SCRIPT_EXTRA2] = "LibExtra2",
};

static int32_t library_script_slot(int32_t script)
{
    if (script <= LIBRARY_SCRIPT_MAIN_A5) {
        return LIBRARY_SCRIPT_SLOT_A;
    }
    if (script <= LIBRARY_SCRIPT_MAIN_B5) {
        return LIBRARY_SCRIPT_SLOT_B;
    }
    if (script <= LIBRARY_SCRIPT_MAIN_C5) {
        return LIBRARY_SCRIPT_SLOT_C;
    }
    return LIBRARY_SCRIPT_SLOT_EXTRA;
}

static int32_t library_image_id(int32_t image)
{
    switch (image) {
        case LIBRARY_IMAGE_LEVELED:
            return image_group(IMAGE_GROUP_LEVELED_GROUND);
        case LIBRARY_IMAGE_FRONT:
            return image_group(IMAGE_GROUP_LIBRARY_FRONT);
        case LIBRARY_IMAGE_MAIN_A:
            return image_group(IMAGE_GROUP_LIBRARY_MAIN_A);
        case LIBRARY_IMAGE_MAIN_B:
            return image_group(IMAGE_GROUP_LIBRARY_MAIN_B);
        case LIBRARY_IMAGE_MAIN_C:
            return image_group(IMAGE_GROUP_LIBRARY_MAIN_C);
        case LIBRARY_IMAGE_C4:
            return image_group(IMAGE_GROUP_LIBRARY_C4);
        case LIBRARY_IMAGE_CC:
            return image_group(IMAGE_GROUP_LIBRARY_CC);
        case LIBRARY_IMAGE_DC:
            return image_group(IMAGE_GROUP_LIBRARY_DC);
        case LIBRARY_IMAGE_E4:
            return image_group(IMAGE_GROUP_LIBRARY_E4);
        case LIBRARY_IMAGE_EC:
            return image_group(IMAGE_GROUP_LIBRARY_EC);
        case LIBRARY_IMAGE_F4:
            return image_group(IMAGE_GROUP_LIBRARY_F4);
        case LIBRARY_IMAGE_FC:
            return image_group(IMAGE_GROUP_LIBRARY_FC);
        case LIBRARY_IMAGE_04:
            return image_group(IMAGE_GROUP_LIBRARY_04);
        case LIBRARY_IMAGE_0C:
            return image_group(IMAGE_GROUP_LIBRARY_0C);
        case LIBRARY_IMAGE_14:
            return image_group(IMAGE_GROUP_LIBRARY_14);
        case LIBRARY_IMAGE_1C:
            return image_group(IMAGE_GROUP_LIBRARY_1C);
        case LIBRARY_IMAGE_24:
            return image_group(IMAGE_GROUP_LIBRARY_24);
        case LIBRARY_IMAGE_2C:
            return image_group(IMAGE_GROUP_LIBRARY_2C);
        case LIBRARY_IMAGE_34:
            return image_group(IMAGE_GROUP_LIBRARY_34);
        case LIBRARY_IMAGE_3C:
            return image_group(IMAGE_GROUP_LIBRARY_3C);
        default:
            return 0;
    }
}

static int32_t library_image_flags(int32_t image)
{
    switch (image) {
        case LIBRARY_IMAGE_BLANK:
        case LIBRARY_IMAGE_LEVELED:
            return MONUMENT_PART_IMAGE_PER_TILE;
        case LIBRARY_IMAGE_FRONT:
            return MONUMENT_PART_IMAGE_PER_TILE | MONUMENT_PART_IMAGE_EDGE_TILES | MONUMENT_PART_IMAGE_UNBUILT;
        default:
            return 0;
    }
}

static const library_part_image_step *library_image_step(int32_t kind, int32_t stage)
{
    const library_part_image_step *found = 0;
    for (int32_t i = 0; i < LIBRARY_PART_IMAGE_STEPS; i++) {
        const library_part_image_step *step = &library_part_image_steps[i];
        if (step->kind != kind) {
            continue;
        }
        found = step;
        if (stage < step->until_stage) {
            break;
        }
    }
    return found;
}

int32_t library_get_image(monument_object *object, int32_t building_id, int32_t *flags)
{
    library_object_t *self = (library_object_t *) object;
    building *b = building_get(building_id);
    int32_t stage = b->data.monument.stage;
    if (stage > LIBRARY_STAGE_PREPARED) {
        stage = building_get(building_id - b->subtype)->data.monument.stage;
    }
    int32_t view = map_info.orientation / 2;
    *flags = 0;
    const library_part_image_step *step = library_image_step(self->parts[b->subtype].kind, stage);
    if (!step) {
        return 0;
    }
    const library_view_image *shown = (view >= LIBRARY_VIEW_SOUTH_FIRST) ? &step->south : &step->north;
    if (shown->script != LIBRARY_SCRIPT_NONE) {
        anim_load_script(library_script_files[shown->script], library_script_slot(shown->script));
    }
    *flags = library_image_flags(shown->image);
    int32_t image = library_image_id(shown->image);
    if (shown->next_in_odd_views) {
        image += view & 1;
    }
    return image;
}
