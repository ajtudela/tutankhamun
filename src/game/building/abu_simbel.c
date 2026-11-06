// Abu Simbel: a scripted monument of sixteen parts cut into a cliff.
//
// Placement checks of its parts (all but one kind need rock near the monument), the stage kept in the main part
// and advanced only by the work spots of stonemasons and carpenters (no laborers, no material), its progress, and
// the drawing of the built parts in views 0 and 3.
//
// The stage lives in the main part's field that holds the city slot in the other monuments, and the drawing of the
// built parts is a method the game never calls; both stay as they are.

#include "building/abu_simbel.h"
#include "building/building_data.h"
#include "building/building_image_group.h"
#include "building/building_type.h"
#include "building/monument.h"
#include "building/monument_object.h"
#include "building/monument_spots.h"
#include "building/monument_work_spots.h"
#include "figure/figure.h"
#include "figure/figure_action.h"
#include "figure/figure_type.h"
#include "graphics/image_collection.h"
#include "map/grid.h"
#include "map/terrain.h"
#include <stdbool.h>
#include <string.h>

void abu_simbel_object_init(abu_simbel_object_t *self)
{
    monument_object_init(&self->base);
    self->base.vtable = &abu_simbel_vtable;
    const monument_part_layout *layout = &monument_part_layouts[MONUMENT_LAYOUT_ABU_SIMBEL];
    monument_object_init_parts(self->parts, layout->parts, layout->count);
    self->base.frame_count = 0;
}

bool abu_simbel_tile_is_free(monument_object *object, uint32_t terrain, int32_t part)
{
    abu_simbel_object_t *self = (abu_simbel_object_t *) object;
    terrain &= TERRAIN_BLOCKS_MONUMENT;
    if (self->parts[part].kind == ABU_SIMBEL_PART_KIND_ON_OPEN_GROUND) {
        return terrain == 0;
    }
    return terrain == (TERRAIN_ROCK | TERRAIN_ROCK_NEAR_MONUMENT);
}

int32_t abu_simbel_get_progress(abu_simbel_object_t *self, int32_t building_id, abu_simbel_info_progress *out)
{
    building *main = building_get(monument_main_part_id(building_id));
    int32_t stage = main->data.abu_simbel.stage;
    int32_t percent;
    if (stage == ABU_SIMBEL_STAGE_COMPLETE) {
        percent = MONUMENT_PROGRESS_COMPLETE;
    } else {
        percent = (int32_t) (((double) stage * (1.0 / ABU_SIMBEL_STAGE_COMPLETE)) * MONUMENT_PROGRESS_COMPLETE);
    }
    if (out) {
        out->stage = stage;
        out->percent = percent;
    }
    return percent;
}

static void init_part(abu_simbel_object_t *self, int32_t building_id, int32_t subtype)
{
    building *b = building_get(building_id);
    b->subtype = (int16_t) subtype;
    b->size = (uint8_t) self->parts[subtype].size;
    b->data.abu_simbel.stage = 0;
    b->data.monument.worker_figure_id = 0;
    b->data.monument.stage = 0;
    int32_t x, y;
    self->base.vtable->get_work_tile(&self->base, building_id, &x, &y);
    b->road_access_x = (int16_t) x;
    b->road_access_y = (int16_t) y;
    memset(b->data.abu_simbel.built_parts, 0, sizeof(b->data.abu_simbel.built_parts));
    self->base.vtable->update_image(&self->base, building_id);
    game_undo_add_building(building_id);
}

void abu_simbel_place(monument_object *object, int32_t x, int32_t y, int32_t building_id, bool no_slot)
{
    abu_simbel_object_t *self = (abu_simbel_object_t *) object;
    if (!no_slot) {
        monument_register(building_id);
    }
    building *main = building_get(building_id);
    int32_t dx = self->parts[0].dx;
    int32_t dy = self->parts[0].dy;
    main->x = (int16_t) (x + dx);
    main->y = (int16_t) (y + dy);
    main->grid_offset = map_info.grid_start + (dy + y) * GRID_SIZE + dx + x;
    main->prev_part_building_id = 0;
    init_part(self, building_id, 0);
    self->base.frame = 0;
    int32_t prev_id = building_id;
    for (int32_t i = 1; i < ABU_SIMBEL_PARTS; i++) {
        int32_t id = building_create(city_current_player, BUILDING_ABU_SIMBEL, x + self->parts[i].dx, y + self->parts[i].dy, 0, prev_id + 1);
        building *b = building_get(id);
        b->prev_part_building_id = (int16_t) prev_id;
        building_get(prev_id)->next_part_building_id = (int16_t) id;
        b->next_part_building_id = 0;
        init_part(self, id, i);
        prev_id = id;
    }
    map_tiles_update_all();
}

void abu_simbel_update_image(monument_object *object, int32_t building_id)
{
    abu_simbel_object_t *self = (abu_simbel_object_t *) object;
    monument_object_update_image_parts(object, building_id, self->parts);
}

bool abu_simbel_check_work_for_figure(
    abu_simbel_object_t *self, int32_t building_id, int32_t figure_id, bool assign, int32_t carpenter_waits)
{
    int32_t main_id = monument_main_part_id(building_id);
    building *main = building_get(main_id);
    int32_t kind = self->parts[main->subtype].kind;
    if (kind < 0 || kind >= ABU_SIMBEL_PART_KINDS_WORKED) {
        return 0;
    }
    if (self->base.vtable->part_has_active_worker(&self->base, building_id)) {
        return 0;
    }
    if (!abu_simbel_has_free_spot(building_id, figure_id)) {
        return 0;
    }
    if (!assign) {
        return 1;
    }
    figure *f = figure_get(figure_id);
    if (f->type == FIGURE_STONEMASON) {
        main->data.monument.worker_figure_id = figure_id;
        f->action_state = FIGURE_ACTION_STONEMASON_GO_TO_SITE;
        f->destination_building_id = (int16_t) main_id;
        f->destination_x = main->road_access_x;
        f->destination_y = main->road_access_y;
        f->destination_grid_offset = main->grid_offset;
        f->roam_length = 0;
        f->wait_ticks = 0;
    } else if (f->type == FIGURE_CARPENTER) {
        if ((uint8_t) carpenter_waits) {
            f->action_state = FIGURE_ACTION_CARPENTER_WORK;
        } else {
            f->action_state = FIGURE_ACTION_CARPENTER_GO_TO_SITE;
            f->destination_x = main->road_access_x;
            f->destination_y = main->road_access_y;
            f->destination_grid_offset = main->grid_offset;
            f->roam_length = 0;
            f->wait_ticks = 0;
        }
        f->destination_building_id = (int16_t) main_id;
        main->data.monument.worker_figure_id = figure_id;
        f->wait_ticks = 0;
    }
    return 1;
}

int32_t abu_simbel_find_work_for_figure_with_wait(
    abu_simbel_object_t *self, int32_t x, int32_t y, int32_t figure_id, int32_t carpenter_waits)
{
    int32_t result = 0;
    for (int32_t i = 1; i < MAX_BUILDINGS && !result; i++) {
        building *b = building_get(i);
        int32_t is_abu_simbel = b->state == BUILDING_STATE_IN_USE && b->type == BUILDING_ABU_SIMBEL;
        if (is_abu_simbel && abu_simbel_check_work_for_figure(self, i, figure_id, 1, carpenter_waits)) {
            result = i;
        }
    }
    return result;
}

int32_t abu_simbel_find_work_for_figure(monument_object *object, int32_t x, int32_t y, int32_t figure_id)
{
    abu_simbel_object_t *self = (abu_simbel_object_t *) object;
    return abu_simbel_find_work_for_figure_with_wait(self, x, y, figure_id, 0);
}

void abu_simbel_figure_work_tick(monument_object *self, int32_t building_id, int32_t figure_id, uint8_t done_action)
{
    abu_simbel_figure_work_tick_spot(building_id, figure_id, done_action);
}

bool abu_simbel_has_incomplete(monument_object *object)
{
    abu_simbel_object_t *self = (abu_simbel_object_t *) object;
    return monument_object_has_incomplete(self, BUILDING_ABU_SIMBEL, ABU_SIMBEL_PARTS);
}

bool abu_simbel_can_place(monument_object *object, int32_t grid_offset)
{
    abu_simbel_object_t *self = (abu_simbel_object_t *) object;
    return monument_object_can_place(&self->base, grid_offset, self->parts, ABU_SIMBEL_PARTS);
}

void abu_simbel_draw_built_parts(abu_simbel_object_t *self, int32_t building_id, int32_t x, int32_t y)
{
    int32_t main_id = building_id - city_buildings.records[building_id].subtype;
    const uint8_t *flags = city_buildings.records[main_id].data.abu_simbel.built_parts;
    int32_t view = map_info.orientation / 2;
    for (int32_t n = 0; n < ABU_SIMBEL_BUILT_PARTS; n++) {
        if (flags[n] == 0) {
            continue;
        }
        int32_t group = n / ABU_SIMBEL_BUILT_PARTS_PER_GROUP;
        int32_t image_group_id = (group < ABU_SIMBEL_BUILT_PART_A_GROUPS) ? IMAGE_GROUP_ABU_SIMBEL_BUILT_PART_A : IMAGE_GROUP_ABU_SIMBEL_BUILT_PART_B;
        int32_t image = image_group(image_group_id) + (view & 1);
        const abu_simbel_built_part_offset *offset = &abu_simbel_built_part_offsets[n];
        if (view == 0) {
            monument_draw_image(image, x + offset->view0_x, y + offset->view0_y);
        } else if (view == 3) {
            monument_draw_image(image, x + offset->view3_x, y + offset->view3_y);
        }
    }
}
