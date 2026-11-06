// The work spots of the library, Abu Simbel, the Caesareum and the lighthouse while they are built: the
// stonemasons' and carpenters' ticks at a spot, the stage-done check and stage advance, the free spot searches,
// Abu Simbel's work tile and worker check, and the spots drawn on the city view.
//
// The spot handling differs per monument:
// - the Caesareum searches a figure's own spot from the index equal to the stage on;
// - Abu Simbel does nothing when a spot is done; its stage table sets built-part flags or part stages instead.

#include "building/monument_work_spots.h"
#include "building/abu_simbel.h"
#include "building/caesareum.h"
#include "building/library.h"
#include "building/lighthouse.h"
#include "building/monument_data.h"
#include "building/monument_object.h"
#include "building/monument_spots.h"
#include "city/city_player.h"
#include "figure/figure.h"
#include "figure/figure_action.h"
#include "figure/figure_type.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

static building *main_part(int32_t building_id)
{
    return building_get(monument_main_part_id(building_id));
}

static int32_t stage_has_work(const monument_work_spot *spots, int32_t count, int32_t stage)
{
    for (int32_t i = 0; i < count; i++) {
        if (spots[i].stage == stage && spots[i].spot->work_remaining > 0) {
            return 1;
        }
    }
    return 0;
}

static int32_t find_free_spot(const monument_work_spot *spots, int32_t count, int32_t stage, int32_t figure_type)
{
    for (int32_t i = 0; i < count; i++) {
        const monument_work_spot *s = &spots[i];
        if (s->spot->figure_id == 0 && s->stage == stage && s->figure_type == figure_type && s->spot->work_remaining > 0) {
            return i;
        }
    }
    return -1;
}

static int32_t work_at_spot(monument_work_spot *s, int32_t figure_id)
{
    if (s->spot->figure_id == 0) {
        monument_spot_attach_figure(s->spot, figure_id);
        monument_spot_start_anim(s->spot, 0);
    }
    s->spot->work_remaining--;
    return s->spot->work_remaining < 1;
}

void library_check_stage_done(int32_t building_id)
{
    if (!stage_has_work(library_work_spots, LIBRARY_WORK_SPOTS, main_part(building_id)->data.monument.stage)) {
        library_advance_stage(&library_object.base, building_id);
    }
}

int32_t library_has_free_spot(int32_t building_id, int32_t figure_id)
{
    int32_t stage = main_part(building_id)->data.monument.stage;
    return (find_free_spot(library_work_spots, LIBRARY_WORK_SPOTS, stage, figure_get(figure_id)->type) >= 0) ? building_id : 0;
}

void library_advance_stage(monument_object *self, int32_t building_id)
{
    int32_t main_id = monument_main_part_id(building_id);
    int32_t stage = building_get(main_id)->data.monument.stage;
    if (stage >= LIBRARY_STAGE_COMPLETE) {
        return;
    }
    for (int32_t id = main_id; id < main_id + LIBRARY_PARTS; id++) {
        building_get(id)->data.monument.stage = stage + 1;
        self->vtable->update_image(self, id);
    }
}

void abu_simbel_check_stage_done(int32_t building_id)
{
    int32_t stage = main_part(building_id)->data.monument.city_slot;
    if (!stage_has_work(abu_simbel_work_spots, ABU_SIMBEL_WORK_SPOTS, stage)) {
        abu_simbel_advance_stage(&abu_simbel_object.base, building_id);
    }
}

void abu_simbel_figure_work_tick_spot(int32_t building_id, int32_t figure_id, uint8_t done_action)
{
    int32_t stage = main_part(building_id)->data.monument.city_slot;
    figure *f = figure_get(figure_id);
    f->is_ghost = 1;
    int32_t index = -1;
    for (int32_t i = 0; i < ABU_SIMBEL_WORK_SPOTS; i++) {
        monument_work_spot *s = &abu_simbel_work_spots[i];
        if (s->spot->figure_id == figure_id && s->stage == stage && s->spot->work_remaining > 0) {
            index = i;
            break;
        }
    }
    if (index < 0) {
        index = find_free_spot(abu_simbel_work_spots, ABU_SIMBEL_WORK_SPOTS, stage, f->type);
        if (index < 0) {
            f->action_state = done_action;
            return;
        }
    }
    monument_work_spot *s = &abu_simbel_work_spots[index];
    if (work_at_spot(s, figure_id)) {
        abu_simbel_check_stage_done(building_id);
        f->action_state = done_action;
        s->spot->mode = MONUMENT_SPOT_IDLE;
    }
}

int32_t abu_simbel_has_free_spot(int32_t building_id, int32_t figure_id)
{
    int32_t stage = main_part(building_id)->data.monument.city_slot;
    return (find_free_spot(abu_simbel_work_spots, ABU_SIMBEL_WORK_SPOTS, stage, figure_get(figure_id)->type) >= 0) ? building_id : 0;
}

void abu_simbel_advance_stage(monument_object *self, int32_t building_id)
{
    int32_t main_id = monument_main_part_id(building_id);
    building *main = building_get(main_id);
    int32_t stage = main->data.monument.city_slot;
    if (stage >= ABU_SIMBEL_STAGE_COMPLETE) {
        return;
    }
    const uint8_t *entry = abu_simbel_stage_table + stage * 4;
    if (entry[1] == 0) {
        int32_t index = entry[3] - 1;
        main->data.abu_simbel.built_parts[index] = entry[0] == 3;
    } else {
        int32_t part = main_id;
        switch (entry[2]) {
            case 1:
                part = main_id + 14;
                break;
            case 2:
                part = main_id + 11;
                break;
            case 3:
                part = main_id + 13;
                break;
            case 4:
                part = main_id + 15;
                break;
            case 5:
                part = main_id + 12;
                break;
        }
        building_get(part)->data.monument.stage = entry[3];
    }
    for (int32_t id = main_id; id < main_id + ABU_SIMBEL_PARTS; id++) {
        building_get(id)->data.monument.city_slot = stage + 1;
        self->vtable->update_image(self, id);
    }
}

bool abu_simbel_get_work_tile(monument_object *self, int32_t building_id, int32_t *x, int32_t *y)
{
    (void) self;
    const building *b = main_part(building_id);
    *x = b->x + 6;
    *y = b->y - 8;
    return 1;
}

bool abu_simbel_part_has_active_worker(monument_object *self, int32_t building_id)
{
    (void) self;
    building *b = main_part(building_id);
    int32_t worker = b->data.abu_simbel.worker_figure_id;
    if (worker == 0) {
        return 0;
    }
    const figure *f = figure_get(worker);
    if (f->state != FIGURE_STATE_NONE) {
        int32_t action = f->action_state;
        if (action == FIGURE_ACTION_ON_FERRY || action == FIGURE_ACTION_WAIT_FOR_FERRY) {
            action = f->action_state_before_attack;
        }
        if (f->type == FIGURE_CARPENTER) {
            return (action == FIGURE_ACTION_CARPENTER_GO_TO_SITE) || (action == FIGURE_ACTION_CARPENTER_WORK);
        }
        if (f->type == FIGURE_STONEMASON) {
            return (action == FIGURE_ACTION_STONEMASON_GO_TO_SITE) || (action == FIGURE_ACTION_STONEMASON_WORK);
        }
    }
    b->data.abu_simbel.worker_figure_id = 0;
    return 0;
}

bool abu_simbel_check_work_no_assign(monument_object *self, int32_t building_id, int32_t figure_id, bool assign)
{
    return abu_simbel_check_work_for_figure((abu_simbel_object_t *) self, building_id, figure_id, assign, 0);
}

void caesareum_check_stage_done(int32_t building_id)
{
    if (!stage_has_work(caesareum_work_spots, CAESAREUM_WORK_SPOTS, building_get(building_id)->data.monument.stage)) {
        caesareum_advance_stage(&caesareum_object.base, building_id);
    }
}

void caesareum_figure_work_tick_spot(int32_t building_id, int32_t figure_id, uint8_t done_action)
{
    building *b = building_get(building_id);
    int32_t stage = b->data.monument.stage;
    figure *f = figure_get(figure_id);
    f->is_ghost = 1;
    int32_t index = -1;
    if (stage < CAESAREUM_WORK_SPOTS) {
        for (int32_t i = stage; i < CAESAREUM_WORK_SPOTS; i++) {
            if (caesareum_work_spots[i].spot->figure_id == figure_id) {
                if (caesareum_work_spots[i].stage != stage) {
                    f->action_state = done_action;
                    return;
                }
                index = i;
                break;
            }
        }
    }
    if (index < 0) {
        index = find_free_spot(caesareum_work_spots, CAESAREUM_WORK_SPOTS, stage, f->type);
        if (index < 0) {
            f->action_state = done_action;
            return;
        }
    }
    if (work_at_spot(&caesareum_work_spots[index], figure_id)) {
        caesareum_check_stage_done(building_id);
        b->data.monument.worker_figure_id = 0;
        f->action_state = done_action;
    }
}

int32_t caesareum_has_free_spot(int32_t building_id, int32_t figure_id)
{
    int32_t stage = building_get(building_id)->data.monument.stage;
    return (find_free_spot(caesareum_work_spots, CAESAREUM_WORK_SPOTS, stage, figure_get(figure_id)->type) >= 0) ? building_id : 0;
}

void caesareum_advance_stage(monument_object *self, int32_t building_id)
{
    int32_t main_id = monument_main_part_id(building_id);
    building *b = building_get(main_id);
    if (b->data.monument.stage >= CAESAREUM_STAGE_COMPLETE) {
        return;
    }
    if (++b->data.monument.stage == CAESAREUM_STAGE_COMPLETE) {
        b->data.monument.work_flags |= MONUMENT_WORK_COMPLETE;
    }
    for (int32_t i = 0; i < CAESAREUM_PARTS; i++) {
        self->vtable->update_image(self, main_id + i);
    }
}

void lighthouse_check_stage_done(int32_t building_id)
{
    if (!stage_has_work(lighthouse_work_spots, LIGHTHOUSE_WORK_SPOTS, building_get(building_id)->data.monument.stage)) {
        lighthouse_advance_stage(&lighthouse_object.base, building_id);
    }
}

void lighthouse_figure_work_tick_spot(int32_t building_id, int32_t figure_id, uint8_t done_action)
{
    int32_t stage = building_get(building_id)->data.monument.stage;
    figure *f = figure_get(figure_id);
    f->is_ghost = 1;
    int32_t index = -1;
    for (int32_t i = 0; i < LIGHTHOUSE_WORK_SPOTS; i++) {
        if (lighthouse_work_spots[i].spot->figure_id == figure_id) {
            if (lighthouse_work_spots[i].stage != stage) {
                f->action_state = done_action;
                return;
            }
            index = i;
            break;
        }
    }
    if (index < 0) {
        index = find_free_spot(lighthouse_work_spots, LIGHTHOUSE_WORK_SPOTS, stage, f->type);
        if (index < 0) {
            f->action_state = done_action;
            return;
        }
    }
    if (work_at_spot(&lighthouse_work_spots[index], figure_id)) {
        lighthouse_check_stage_done(building_id);
        f->action_state = done_action;
    }
}

int32_t lighthouse_has_free_spot(int32_t building_id, int32_t figure_id)
{
    int32_t stage = building_get(building_id)->data.monument.stage;
    return (find_free_spot(lighthouse_work_spots, LIGHTHOUSE_WORK_SPOTS, stage, figure_get(figure_id)->type) >= 0) ? building_id : 0;
}

void lighthouse_advance_stage(monument_object *self, int32_t building_id)
{
    building *b = building_get(building_id);
    if (b->data.monument.stage >= LIGHTHOUSE_STAGE_COMPLETE) {
        return;
    }
    if (++b->data.monument.stage == LIGHTHOUSE_STAGE_COMPLETE) {
        b->data.monument.work_flags |= MONUMENT_WORK_COMPLETE;
    }
    self->vtable->update_image(self, building_id);
}

static void draw_spot_table(const monument_work_spot *spots, int32_t count, int32_t x, int32_t y)
{
    for (int32_t i = 0; i < count; i++) {
        int32_t fx, fy, image;
        monument_spot_get_draw_info(spots[i].spot, &fx, &fy, &image);
        if (image != 0) {
            monument_draw_image(image, fx + x, fy + y);
        }
    }
}

static bool spot_table_has_free(const monument_work_spot *spots, int32_t count, int32_t figure_type, int32_t stage)
{
    for (int32_t i = 0; i < count; i++) {
        if (spots[i].spot->figure_id == 0 && spots[i].stage == stage && spots[i].figure_type == figure_type && spots[i].spot->work_remaining > 0) {
            return 1;
        }
    }
    return 0;
}

int32_t library_spot_table_get_variant(void)
{
    return (library_spot_variant / 2) & 1;
}

void library_spot_table_set_variant(int32_t variant)
{
    if (variant == 0 || variant == 4) {
        library_spot_variant = 0;
        memcpy(library_anim_positions, library_anim_positions_a, sizeof(library_anim_positions));
    } else {
        library_spot_variant = 2;
        memcpy(library_anim_positions, library_anim_positions_b, sizeof(library_anim_positions));
    }
}

void library_draw_spots(int32_t x, int32_t y)
{
    int32_t variant = (map_info.orientation / 2) & 1;
    if (variant != library_spot_table_get_variant()) {
        library_spot_table_set_variant(variant);
    }
    draw_spot_table(library_work_spots, LIBRARY_WORK_SPOTS, x, y);
}

void abu_simbel_draw_spots(int32_t x, int32_t y)
{
    draw_spot_table(abu_simbel_work_spots, ABU_SIMBEL_WORK_SPOTS, x, y);
}

void caesareum_draw_spots(int32_t x, int32_t y)
{
    draw_spot_table(caesareum_work_spots, CAESAREUM_WORK_SPOTS, x, y);
}

void lighthouse_draw_spots(int32_t x, int32_t y)
{
    draw_spot_table(lighthouse_work_spots, LIGHTHOUSE_WORK_SPOTS, x, y);
}

bool library_has_free_spot_for_type(int32_t figure_type, int32_t stage)
{
    return spot_table_has_free(library_work_spots, LIBRARY_WORK_SPOTS, figure_type, stage);
}

bool abu_simbel_has_free_spot_for_type(int32_t figure_type, int32_t stage)
{
    return spot_table_has_free(abu_simbel_work_spots, ABU_SIMBEL_WORK_SPOTS, figure_type, stage);
}

bool caesareum_has_free_spot_for_type(int32_t figure_type, int32_t stage)
{
    return spot_table_has_free(caesareum_work_spots, CAESAREUM_WORK_SPOTS, figure_type, stage);
}

bool lighthouse_has_free_spot_for_type(int32_t figure_type, int32_t stage)
{
    return spot_table_has_free(lighthouse_work_spots, LIGHTHOUSE_WORK_SPOTS, figure_type, stage);
}
