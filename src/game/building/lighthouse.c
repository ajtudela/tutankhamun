// The lighthouse: a scripted monument of a 6x6 tower on water and a 1x1 land part where its workers stand.
//
// Placement checks (rock under the tower, a free land tile), the tower's stage advanced by stonemasons and
// carpenters while laborers bring marble, progress and material still needed, and the images of both parts.
//
// While its worker slot is active but empty, the tower reports progress 0 and leaves the report unfilled.

#include "building/lighthouse.h"
#include "building/building_data.h"
#include "building/building_image_group.h"
#include "building/building_type.h"
#include "building/monument.h"
#include "building/monument_object.h"
#include "building/monument_spots.h"
#include "building/monument_work_spots.h"
#include "city/resource_type.h"
#include "figure/figure.h"
#include "figure/figure_action.h"
#include "figure/figure_type.h"
#include "graphics/image_collection.h"
#include "map/grid.h"
#include "map/terrain.h"
#include <stdbool.h>

void lighthouse_object_init(lighthouse_object_t *self)
{
    monument_object_init(&self->base);
    self->base.vtable = &lighthouse_vtable;
    for (int32_t i = 0; i < LIGHTHOUSE_SCRIPTS; i++) {
        self->scripts[i] = lighthouse_script_names[(LIGHTHOUSE_SCRIPTS - 1) - i];
    }
    const monument_part_layout *layout = &monument_part_layouts[MONUMENT_LAYOUT_LIGHTHOUSE];
    monument_object_init_parts(self->parts, layout->parts, layout->count);
    self->base.frame_count = 3;
}

bool lighthouse_tile_is_free(monument_object *self, uint32_t terrain, int32_t part)
{
    terrain &= TERRAIN_BLOCKS_MONUMENT;
    if (part == LIGHTHOUSE_PART_LAND) {
        return terrain == 0;
    }
    return terrain == TERRAIN_ROCK;
}

bool lighthouse_has_incomplete(monument_object *object)
{
    lighthouse_object_t *self = (lighthouse_object_t *) object;
    return monument_object_has_incomplete(self, BUILDING_LIGHTHOUSE, LIGHTHOUSE_PARTS);
}

int32_t lighthouse_get_progress(lighthouse_object_t *self, int32_t building_id, scripted_monument_info_progress *out)
{
    int32_t main_id = monument_main_part_id(building_id);
    building *main = building_get(main_id);
    int32_t stage = main->data.monument.stage;
    int32_t percent = (stage == LIGHTHOUSE_STAGE_COMPLETE) ? MONUMENT_PROGRESS_COMPLETE : ((stage * 5) + 5) * 2;
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
            if (f->action_state == FIGURE_ACTION_STONEMASON_WAIT_FOR_BLOCK) {
                waiting = 1;
            }
            masons = 1;
        } else if (f->type == FIGURE_LABORER) {
            laborers = 1;
        }
    }
    int32_t material = (LIGHTHOUSE_STAGE_COMPLETE - stage) * self->base.frame_count * MONUMENT_MATERIAL_PER_LOAD;
    if (material < 0) {
        material = 0;
    }
    if (out) {
        out->stage = stage;
        out->stonemasons = masons;
        out->stonemasons_waiting = waiting;
        out->laborers = laborers;
        out->carpenters = carpenters;
        out->material = material;
        out->percent = percent;
        out->material_total = self->base.frame_count * (LIGHTHOUSE_STAGE_COMPLETE * MONUMENT_MATERIAL_PER_LOAD);
    }
    return percent;
}

void lighthouse_place(monument_object *object, int32_t x, int32_t y, int32_t building_id, bool no_slot)
{
    lighthouse_object_t *self = (lighthouse_object_t *) object;
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
    main->subtype = 0;
    main->size = (uint8_t) self->parts[0].size;
    main->prev_part_building_id = 0;
    main->data.monument.city_slot = city_slot;
    main->data.monument.stage = 0;
    main->data.monument.is_main_part = 1;
    main->data.monument.worker_figure_id = 0;
    game_undo_add_building(building_id);
    self->base.vtable->update_image(&self->base, building_id);

    int32_t id = building_create(city_current_player, BUILDING_LIGHTHOUSE, x + self->parts[1].dx, y + self->parts[1].dy, 0, building_id + 1);
    building *b = building_get(id);
    b->prev_part_building_id = (int16_t) building_id;
    main->next_part_building_id = (int16_t) id;
    b->next_part_building_id = 0;
    b->subtype = (int16_t) self->parts[1].kind;
    b->size = (uint8_t) self->parts[1].size;
    b->data.monument.city_slot = city_slot;
    b->data.monument.stage = 0;
    b->data.monument.is_main_part = 0;
    b->data.monument.worker_figure_id = 0;
    game_undo_add_building(id);
    self->base.vtable->update_image(&self->base, id);
    self->base.frame = 0;
    map_tiles_update_all();
}

void lighthouse_update_image(monument_object *object, int32_t building_id)
{
    lighthouse_object_t *self = (lighthouse_object_t *) object;
    int32_t flags;
    int32_t image = self->base.vtable->get_image(&self->base, building_id, &flags);
    building *b = building_get(building_id);
    const monument_part_desc *desc = &self->parts[b->subtype];
    if (desc->kind == LIGHTHOUSE_PART_LAND) {
        int32_t tile = map_info.grid_start + b->x + b->y * GRID_SIZE;
        map_grids.building_ids[tile] = (int16_t) building_id;
        map_grids.terrain[tile] |= TERRAIN_MONUMENT;
        return;
    }
    map_building_tiles_add(building_id, b->x, b->y, desc->size, desc->size, image, TERRAIN_BUILDING);
}

int32_t lighthouse_get_image(monument_object *object, int32_t building_id, int32_t *flags)
{
    lighthouse_object_t *self = (lighthouse_object_t *) object;
    building *b = building_get(building_id);
    int32_t kind = self->parts[b->subtype].kind;
    if (kind == LIGHTHOUSE_PART_TOWER) {
        *flags = 0;
        anim_load_script(self->scripts[b->data.monument.stage], MONUMENT_SCRIPT_SLOT_LIGHTHOUSE);
        return image_group(IMAGE_GROUP_LIGHTHOUSE);
    }
    if (kind == LIGHTHOUSE_PART_LAND) {
        return image_group(IMAGE_GROUP_LIGHTHOUSE_LAND);
    }
    return 0;
}

bool lighthouse_get_work_tile(monument_object *self, int32_t building_id, int32_t *x, int32_t *y)
{
    building *b = building_get(building_id);
    *x = b->x + LIGHTHOUSE_WORK_TILE_DX;
    *y = b->y + LIGHTHOUSE_WORK_TILE_DY;
    return 1;
}

static int32_t carpenter_at_work(int32_t action)
{
    return action == FIGURE_ACTION_CARPENTER_GO_TO_SITE || action == FIGURE_ACTION_CARPENTER_WORK;
}

static int32_t stonemason_at_work(int32_t action)
{
    return action == FIGURE_ACTION_STONEMASON_GO_TO_SITE || action == FIGURE_ACTION_STONEMASON_WORK || action == FIGURE_ACTION_STONEMASON_WAIT_FOR_BLOCK;
}

static int32_t laborer_fetching_material(int32_t action)
{
    int32_t at_storage_yard = action == FIGURE_ACTION_LABORER_TO_STORAGE_YARD || action == FIGURE_ACTION_LABORER_AT_STORAGE_YARD;
    return at_storage_yard || action == FIGURE_ACTION_LABORER_SLED_TO_LIGHTHOUSE;
}

bool lighthouse_part_has_active_worker(monument_object *self, int32_t building_id)
{
    building *b = building_get(building_id);
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
        case FIGURE_CARPENTER:
            return carpenter_at_work(action);
        case FIGURE_STONEMASON:
            return stonemason_at_work(action);
        case FIGURE_LABORER:
            return laborer_fetching_material(action);
        default:
            b->data.monument.worker_figure_id = 0;
            return 0;
    }
}

bool lighthouse_check_work_for_figure(monument_object *object, int32_t building_id, int32_t figure_id, bool assign)
{
    lighthouse_object_t *self = (lighthouse_object_t *) object;
    building *b = building_get(building_id);
    if (self->parts[b->subtype].kind != LIGHTHOUSE_PART_TOWER) {
        return 0;
    }
    figure *f = figure_get(figure_id);
    if (f->type == FIGURE_LABORER) {
        if (!self->base.vtable->part_has_active_worker(&self->base, building_id)) {
            return 0;
        }
        int32_t mason_id = b->data.monument.worker_figure_id;
        int32_t result = monument_check_material_laborer(mason_id, figure_id, building_id, RESOURCE_MARBLE, FIGURE_ACTION_LABORER_SLED_TO_LIGHTHOUSE, assign);
        return result == MATERIAL_ASSIGNED;
    }
    if (self->base.vtable->part_has_active_worker(&self->base, building_id)) {
        return 0;
    }
    int32_t spot_building = lighthouse_has_free_spot(building_id, figure_id);
    if (!spot_building) {
        return 0;
    }
    if (assign) {
        if (f->type == FIGURE_CARPENTER || f->type == FIGURE_STONEMASON) {
            f->action_state = FIGURE_ACTION_STONEMASON_GO_TO_SITE;
        }
        building *target = building_get(spot_building);
        target->data.monument.worker_figure_id = figure_id;
        f->destination_x = target->road_access_x;
        f->destination_y = target->road_access_y;
        f->destination_grid_offset = target->grid_offset;
        f->roam_length = 0;
        f->wait_ticks = 0;
    }
    return 1;
}

int32_t lighthouse_find_work_for_figure(monument_object *object, int32_t x, int32_t y, int32_t figure_id)
{
    lighthouse_object_t *self = (lighthouse_object_t *) object;
    int32_t result = 0;
    for (int32_t i = 1; i < MAX_BUILDINGS && !result; i++) {
        building *b = building_get(i);
        int32_t is_lighthouse = b->state == BUILDING_STATE_IN_USE && b->type == BUILDING_LIGHTHOUSE;
        if (is_lighthouse && self->base.vtable->check_work_for_figure(&self->base, i, figure_id, 1)) {
            result = i;
        }
    }
    return result;
}

void lighthouse_figure_work_tick(monument_object *object, int32_t building_id, int32_t figure_id, uint8_t done_action)
{
    lighthouse_object_t *self = (lighthouse_object_t *) object;
    if (self->parts[building_get(building_id)->subtype].kind == LIGHTHOUSE_PART_TOWER) {
        lighthouse_figure_work_tick_spot(building_id, figure_id, done_action);
    }
}

bool lighthouse_can_place(monument_object *object, int32_t grid_offset)
{
    lighthouse_object_t *self = (lighthouse_object_t *) object;
    return monument_object_can_place(&self->base, grid_offset, self->parts, LIGHTHOUSE_PARTS);
}
