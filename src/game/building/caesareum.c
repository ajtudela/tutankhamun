// The Caesareum: a scripted monument of nine 5x5 parts.
//
// Placement checks, the stage kept in the main part and advanced by stonemasons and carpenters while laborers
// bring marble (granite in the last stages), progress and material still needed, the part images turned with the
// view and the work tiles of the workers.
//
// A main part with an active worker slot but no worker returns progress 0 without filling the report.

#include "building/caesareum.h"
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
#include <stdbool.h>

const char caesareum_script_names[CAESAREUM_SCRIPTS][12] = { "caesareum20", "caesareum19", "caesareum18", "caesareum17", "caesareum16", "caesareum15", "caesareum14", "caesareum13", "caesareum12",
    "caesareum11", "caesareum10", "caesareum9", "caesareum8", "caesareum7", "caesareum6", "caesareum5", "caesareum4", "caesareum3", "caesareum2", "caesareum1" };

void caesareum_object_init(caesareum_object_t *self)
{
    monument_object_init(&self->base);
    self->base.vtable = &caesareum_vtable;
    for (int32_t i = 0; i < CAESAREUM_SCRIPTS; i++) {
        self->scripts[i] = caesareum_script_names[(CAESAREUM_SCRIPTS - 1) - i];
    }
    const monument_part_layout *layout = &monument_part_layouts[MONUMENT_LAYOUT_CAESAREUM];
    monument_object_init_parts(self->parts, layout->parts, layout->count);
    self->base.frame_count = 2;
}

int32_t caesareum_get_progress(caesareum_object_t *self, int32_t building_id, scripted_monument_info_progress *out)
{
    int32_t main_id = monument_main_part_id(building_id);
    building *main = building_get(main_id);
    int32_t stage = main->data.monument.stage;
    int32_t percent = (stage == CAESAREUM_STAGE_COMPLETE) ? MONUMENT_PROGRESS_COMPLETE : (stage * 5) + 5;
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
    int32_t material = (CAESAREUM_MATERIAL_LOADS - (self->base.frame_count * stage)) * MONUMENT_MATERIAL_PER_LOAD;
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
        out->material_total = CAESAREUM_MATERIAL_LOADS * MONUMENT_MATERIAL_PER_LOAD;
    }
    return percent;
}

static void init_part(caesareum_object_t *self, int32_t building_id, int32_t subtype, int32_t city_slot)
{
    building *b = building_get(building_id);
    b->subtype = (int16_t) subtype;
    b->size = (uint8_t) self->parts[subtype].size;
    b->data.monument.city_slot = city_slot;
    b->data.monument.stage = 0;
    b->data.monument.is_main_part = 1;
    b->data.monument.worker_figure_id = 0;
    self->base.vtable->update_image(&self->base, building_id);
    game_undo_add_building(building_id);
}

void caesareum_place(monument_object *object, int32_t x, int32_t y, int32_t building_id, bool no_slot)
{
    caesareum_object_t *self = (caesareum_object_t *) object;
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
    for (int32_t i = 1; i < CAESAREUM_PARTS; i++) {
        int32_t id = building_create(city_current_player, BUILDING_CAESAREUM, x + self->parts[i].dx, y + self->parts[i].dy, 0, prev_id + 1);
        building *b = building_get(id);
        b->prev_part_building_id = (int16_t) prev_id;
        building_get(prev_id)->next_part_building_id = (int16_t) id;
        b->next_part_building_id = 0;
        init_part(self, id, i, city_slot);
        prev_id = id;
    }
}

void caesareum_update_image(monument_object *object, int32_t building_id)
{
    caesareum_object_t *self = (caesareum_object_t *) object;
    monument_object_update_image_parts(object, building_id, self->parts);
}

int32_t caesareum_get_image(monument_object *object, int32_t building_id, int32_t *flags)
{
    caesareum_object_t *self = (caesareum_object_t *) object;
    static const int32_t view_part[4][9] = {
        { 0, 1, 2, 3, 4, 5, 6, 7, 8 },
        { 2, 5, 8, 1, 4, 7, 0, 3, 6 },
        { 8, 7, 6, 5, 4, 3, 2, 1, 0 },
        { 6, 3, 0, 7, 4, 1, 8, 5, 2 },
    };
    building *b = building_get(building_id);
    int32_t part = view_part[map_info.orientation / 2][b->subtype];
    *flags = 0;
    building *main = building_get(building_id - b->subtype);
    anim_load_script(self->scripts[main->data.monument.stage], MONUMENT_SCRIPT_SLOT_CAESAREUM);
    return part + image_group(IMAGE_GROUP_CAESAREUM);
}

bool caesareum_get_work_tile(monument_object *self, int32_t building_id, int32_t *x, int32_t *y)
{
    building *b = building_get(building_id);
    building *main = building_get(building_id - b->subtype);
    switch (map_info.orientation / 2) {
        case 0:
            *x = main->x + 8;
            *y = main->y + 15;
            break;
        case 1:
            *x = main->x - 1;
            *y = main->y + 8;
            break;
        case 2:
            *x = main->x + 6;
            *y = main->y - 1;
            break;
        case 3:
            *x = main->x + 15;
            *y = main->y + 6;
            break;
    }
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
    return at_storage_yard || action == FIGURE_ACTION_LABORER_SLED_TO_CAESAREUM;
}

bool caesareum_part_has_active_worker(monument_object *self, int32_t building_id)
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

bool caesareum_check_work_for_figure(monument_object *object, int32_t building_id, int32_t figure_id, bool assign)
{
    caesareum_object_t *self = (caesareum_object_t *) object;
    building *b = building_get(building_id);
    int32_t main_id = building_id - b->subtype;
    if (self->parts[b->subtype].kind != CAESAREUM_PART_KIND_WORKED) {
        return 0;
    }
    figure *f = figure_get(figure_id);
    if (f->type == FIGURE_LABORER) {
        if (!self->base.vtable->part_has_active_worker(&self->base, building_id)) {
            return 0;
        }
        int32_t resource = (building_get(main_id)->data.monument.stage == CAESAREUM_STAGE_GRANITE) ? RESOURCE_GRANITE : RESOURCE_MARBLE;
        int32_t mason_id = b->data.monument.worker_figure_id;
        int32_t result = monument_check_material_laborer(mason_id, figure_id, building_id, resource, FIGURE_ACTION_LABORER_SLED_TO_CAESAREUM, assign);
        return result == MATERIAL_ASSIGNED;
    }
    if (self->base.vtable->part_has_active_worker(&self->base, building_id)) {
        return 0;
    }
    int32_t spot_building = caesareum_has_free_spot(building_id, figure_id);
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

int32_t caesareum_find_work_for_figure(monument_object *object, int32_t x, int32_t y, int32_t figure_id)
{
    caesareum_object_t *self = (caesareum_object_t *) object;
    int32_t result = 0;
    for (int32_t i = 1; i < MAX_BUILDINGS && !result; i++) {
        building *b = building_get(i);
        int32_t is_caesareum = b->state == BUILDING_STATE_IN_USE && b->type == BUILDING_CAESAREUM;
        if (is_caesareum && self->base.vtable->check_work_for_figure(&self->base, i, figure_id, 1)) {
            result = i;
        }
    }
    return result;
}

void caesareum_figure_work_tick(monument_object *object, int32_t building_id, int32_t figure_id, uint8_t done_action)
{
    caesareum_object_t *self = (caesareum_object_t *) object;
    if (self->parts[building_get(building_id)->subtype].kind == CAESAREUM_PART_KIND_WORKED) {
        caesareum_figure_work_tick_spot(building_id, figure_id, done_action);
    }
}

bool caesareum_has_incomplete(monument_object *object)
{
    caesareum_object_t *self = (caesareum_object_t *) object;
    return monument_object_has_incomplete(self, BUILDING_CAESAREUM, CAESAREUM_PARTS);
}

bool caesareum_can_place(monument_object *object, int32_t grid_offset)
{
    caesareum_object_t *self = (caesareum_object_t *) object;
    return monument_object_can_place(&self->base, grid_offset, self->parts, CAESAREUM_PARTS);
}
