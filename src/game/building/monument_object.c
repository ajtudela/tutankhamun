// Base class of the scripted monuments (library, Caesareum, lighthouse, Abu Simbel, royal tombs): part
// descriptors and their per-view offsets, the placement check and preview, the incomplete check, the image update
// by a monument's parts (the update_image of the Caesareum and Abu Simbel), the animation tick,
// constructor and destructor, the material laborer, the work spot resets and the library's spot tick.
//
// Behaviour that looks accidental but stays:
// - the incomplete check asks the first monument of the type in use with an id below the part count;
// - odd views, unused by the game, draw the preview marker 0.

#include "building/monument_object.h"
#include "building/abu_simbel.h"
#include "building/building_data.h"
#include "building/building_type.h"
#include "building/caesareum.h"
#include "building/library.h"
#include "building/lighthouse.h"
#include "building/mausoleum.h"
#include "building/monument.h"
#include "building/monument_city.h"
#include "building/monument_data.h"
#include "building/monument_spots.h"
#include "building/monument_static.h"
#include "building/monument_work_spots.h"
#include "building/pyramid_images.h"
#include "city/city_player.h"
#include "core/static_init.h"
#include "figure/action.h"
#include "figure/figure.h"
#include "figure/figure_action.h"
#include "figure/figure_core.h"
#include "figure/figure_type.h"
#include "graphics/image_blit_mask.h"
#include "graphics/image_collection.h"
#include "map/building_tiles.h"
#include "map/grid.h"
#include "map/map_data.h"
#include "map/terrain.h"
#include "map/terrain_image_group.h"
#include "widget/widget_data.h"
#include <stdbool.h>
#include <string.h>

enum {
    PREVIEW_COLOR_FREE = 0x1FE3,
    PREVIEW_COLOR_BLOCKED = 0xF863,
};

void monument_part_desc_rotate(monument_part_desc *self)
{
    int32_t step_y = city_view.tile_half_height;
    int32_t step_x = city_view.tile_half_width + 1;
    int32_t x = self->dx;
    int32_t y = self->dy;
    self->grid_delta = x + y * GRID_SIZE;
    self->view_x[0] = (x - y) * step_x;
    self->view_x[1] = (x + y) * step_x;
    self->view_x[2] = (y - x) * step_x;
    self->view_x[3] = -((x + y) * step_x);
    self->view_y[0] = (x + y) * step_y;
    self->view_y[1] = (y - x) * step_y;
    self->view_y[2] = -((x + y) * step_y);
    self->view_y[3] = (x - y) * step_y;
}

void monument_part_desc_rotate_n(monument_part_desc *descs, int32_t count)
{
    for (; count > 0; count--, descs++) {
        monument_part_desc_rotate(descs);
    }
}

int32_t monument_object_no_progress(monument_object *self, int32_t building_id, int32_t *out)
{
    (void) self;
    (void) building_id;
    (void) out;
    return 0;
}

bool monument_object_can_place(monument_object *self, int32_t grid_offset, const monument_part_desc *descs, int32_t count)
{
    int32_t offset = grid_offset - map_info.grid_start;
    int32_t gx = offset % GRID_SIZE;
    int32_t gy = offset / GRID_SIZE;
    uint8_t ok = 1;
    for (int32_t part = 0; part < count && ok; part++) {
        const monument_part_desc *desc = &descs[part];
        int32_t y = desc->dy + gy;
        int32_t x = desc->dx + gx;
        map_tile_cursor_offset = map_info.grid_start + y * GRID_SIZE + x;
        for (int32_t row = 0; row < desc->size && ok; row++) {
            for (int32_t col = 0; col < desc->size && ok; col++) {
                if (map_grid_is_inside_size(map_info.diamond_x_min + col + x, map_info.diamond_y_min + row + y, 3)) {
                    ok = 0;
                } else {
                    uint32_t terrain = map_grids.terrain[building_square_tile_offset(row, col) + map_tile_cursor_offset];
                    ok = self->vtable->tile_is_free(self, terrain, part);
                }
            }
        }
    }
    return ok;
}

void monument_object_draw_placement(monument_object *self, const monument_part_desc *descs, int32_t count)
{
    int32_t view = map_info.orientation / 2;
    int32_t step_x = city_view.tile_half_width + 1;
    int32_t step_y = city_view.tile_half_height;
    int32_t pixel_x, pixel_y;
    city_view_tile_to_pixel(map_info.hover_x, map_info.hover_y, &pixel_x, &pixel_y);
    int32_t offset = map_info.hover_offset - map_info.grid_start;
    int32_t gx = offset % GRID_SIZE;
    int32_t gy = offset / GRID_SIZE;
    for (int32_t part = 0; part < count; part++) {
        const monument_part_desc *desc = &descs[part];
        int32_t x = desc->dx + gx;
        int32_t y = desc->dy + gy;
        map_tile_cursor_offset = map_info.grid_start + y * GRID_SIZE + x;
        int32_t size = desc->size;
        for (int32_t row = 0; row < desc->size; row++) {
            for (int32_t col = 0; col < desc->size; col++) {
                int32_t sx = desc->view_x[view] + pixel_x;
                int32_t sy = desc->view_y[view] + step_y + pixel_y;
                if (map_grid_is_inside_size(map_info.diamond_x_min + col + x, map_info.diamond_y_min + row + y, 3)) {
                    continue;
                }
                uint32_t terrain = map_grids.terrain[building_square_tile_offset(row, col) + map_tile_cursor_offset];
                int32_t color = self->vtable->tile_is_free(self, terrain, part) ? PREVIEW_COLOR_FREE : PREVIEW_COLOR_BLOCKED;
                int32_t index = 0;
                switch (map_info.orientation) {
                    case 0:
                        index = footprint_index_table[(col * BUILDING_SQUARE_SIDE) + row];
                        break;
                    case 2:
                        index = footprint_index_table[(row * BUILDING_SQUARE_SIDE) + size - col - 1];
                        sx += (size - 1) * step_x;
                        sy -= (size - 1) * step_y;
                        break;
                    case 4:
                        index = footprint_index_table[(size * (BUILDING_SQUARE_SIDE + 1)) - (col * BUILDING_SQUARE_SIDE) - row - (BUILDING_SQUARE_SIDE + 1)];
                        sy -= 2 * (size - 1) * step_y;
                        break;
                    case 6:
                        index = footprint_index_table[((size - row - 1) * BUILDING_SQUARE_SIDE) + col];
                        sx -= (size - 1) * step_x;
                        sy -= (size - 1) * step_y;
                        break;
                    default:
                        break;
                }
                image_draw_masked(
                    image_group(IMAGE_GROUP_FLAT_TILE), construction_ghost_tile_offsets[index].x + sx, construction_ghost_tile_offsets[index].y - step_y + sy, color);
            }
        }
    }
}

void monument_object_update_image_parts(monument_object *self, int32_t building_id, const monument_part_desc *parts)
{
    int32_t flags;
    int32_t image = self->vtable->get_image(self, building_id, &flags);
    building *b = building_get(building_id);
    const monument_part_desc *desc = &parts[b->subtype];
    map_building_tiles_add(building_id, b->x, b->y, desc->size, desc->size, image, TERRAIN_BUILDING);
}

bool monument_object_anim_tick(monument_object *self)
{
    if (++self->frame >= self->frame_count) {
        self->frame = 0;
        return 1;
    }
    return 0;
}

static void reset_work_spots(monument_work_spot *spots, int32_t count)
{
    for (int32_t i = 0; i < count; i++) {
        spots[i].spot->figure_id = 0;
        monument_spot_start_anim(spots[i].spot, MONUMENT_SPOT_IDLE);
        spots[i].spot->work_remaining = spots[i].initial_work;
    }
}

void library_figure_work_tick_spot(int32_t building_id, int32_t figure_id, uint8_t done_action)
{
    building *b = building_get(building_id);
    int32_t stage = building_get(building_id - b->subtype)->data.monument.stage;
    figure *f = figure_get(figure_id);
    f->is_ghost = 1;
    monument_work_spot *spot = 0;
    for (int32_t i = 0; i < LIBRARY_WORK_SPOTS; i++) {
        if (library_work_spots[i].spot->figure_id == figure_id) {
            if (library_work_spots[i].stage != stage) {
                f->action_state = done_action;
                return;
            }
            spot = &library_work_spots[i];
            break;
        }
    }
    if (!spot) {
        for (int32_t i = 0; i < LIBRARY_WORK_SPOTS; i++) {
            monument_work_spot *s = &library_work_spots[i];
            if (s->spot->figure_id == 0 && s->stage == stage && s->figure_type == f->type && s->spot->work_remaining > 0) {
                spot = s;
                break;
            }
        }
        if (!spot) {
            f->action_state = done_action;
            return;
        }
    }
    if (spot->spot->figure_id == 0) {
        monument_spot_attach_figure(spot->spot, figure_id);
        monument_spot_start_anim(spot->spot, MONUMENT_SPOT_LOOP);
    }
    spot->spot->work_remaining--;
    if (spot->spot->work_remaining < 1) {
        library_check_stage_done(building_id);
        f->action_state = done_action;
    }
}

void monument_object_init(monument_object *self)
{
    self->base_part = (monument_part_desc) { 0, 0, 1, 7 };
    self->base_scripts[0] = sun_temple_scripts[0];
    self->base_scripts[1] = sun_temple_scripts[1];
    self->base_scripts[2] = sun_temple_scripts[2];
    self->frame_count = 1;
    self->frame = 0;
}

void monument_object_init_parts(monument_part_desc *parts, const monument_part_init *init, int32_t count)
{
    for (int32_t i = 0; i < count; i++) {
        parts[i] = (monument_part_desc) { init[i].dx, init[i].dy, init[i].size, init[i].kind };
    }
}

int32_t monument_check_material_laborer(int32_t mason_id, int32_t figure_id, int32_t storage_building, int32_t resource, int32_t carry_action, bool assign)
{
    figure *mason = figure_get(mason_id);
    if (mason->type != FIGURE_STONEMASON || mason->action_state != FIGURE_ACTION_STONEMASON_WAIT_FOR_BLOCK) {
        return MATERIAL_NO_WORK;
    }
    int32_t partner_id = mason->leading_figure_id;
    if (partner_id) {
        figure *partner = figure_get(partner_id);
        if (partner->state != FIGURE_STATE_NONE && partner->type == FIGURE_LABORER) {
            int32_t action = partner->action_state;
            if ((action == FIGURE_ACTION_ON_FERRY || action == FIGURE_ACTION_WAIT_FOR_FERRY || action == FIGURE_ACTION_LABORER_TO_STORAGE_YARD ||
                    action == FIGURE_ACTION_LABORER_AT_STORAGE_YARD || action == carry_action) &&
                partner->leading_figure_id == mason_id) {
                return MATERIAL_PARTNER_BUSY;
            }
        }
        mason->leading_figure_id = 0;
    }
    if (!assign) {
        return MATERIAL_ASSIGNED;
    }
    figure *f = figure_get(figure_id);
    int32_t storage_id = building_storageyard_for_getting(storage_building, resource, 1);
    if (!storage_id) {
        f->state = FIGURE_STATE_DEAD;
        return MATERIAL_ASSIGNED;
    }
    f->collecting_item_id = (uint8_t) resource;
    f->resource_amount = 0;
    f->destination_building_id = (int16_t) storage_id;
    f->leading_figure_id = (int16_t) mason_id;
    f->action_state = FIGURE_ACTION_LABORER_TO_STORAGE_YARD;
    f->wait_ticks = 0;
    f->destination_x = (int16_t) map_result_x;
    f->destination_y = (int16_t) map_result_y;
    mason->leading_figure_id = (int16_t) figure_id;
    return MATERIAL_ASSIGNED;
}

bool monument_object_tile_is_free(monument_object *self, uint32_t terrain, int32_t part)
{
    return (terrain & TERRAIN_BLOCKS_MONUMENT) == 0;
}

void library_reset_work_spots(void)
{
    reset_work_spots(library_work_spots, LIBRARY_WORK_SPOTS);
}
void abu_simbel_reset_work_spots(void)
{
    reset_work_spots(abu_simbel_work_spots, ABU_SIMBEL_WORK_SPOTS);
}
void caesareum_reset_work_spots(void)
{
    reset_work_spots(caesareum_work_spots, CAESAREUM_WORK_SPOTS);
}
void lighthouse_reset_work_spots(void)
{
    reset_work_spots(lighthouse_work_spots, LIGHTHOUSE_WORK_SPOTS);
}

bool monument_object_has_incomplete(void *self, int32_t type, int32_t count)
{
    for (int32_t id = 1; id < count; id++) {
        const building *b = building_get(id);
        if (b->state == BUILDING_STATE_IN_USE && b->type == type) {
            monument_object *object = self;
            return object->vtable->get_progress(object, id, NULL) < MONUMENT_PROGRESS_COMPLETE;
        }
    }
    return 0;
}
