// The obelisks, small (3x3) and large (5x5), each one building record: their progress, worked by carpenters and
// stonemasons at the spots of the size's table, only one obelisk at a time; the animation script follows the
// progress.
//
// The script formula never reaches the last script.

#include "building/obelisk.h"
#include "building/building_image_group.h"
#include "building/building_type.h"
#include "building/monument.h"
#include "building/monument_spots.h"
#include "city/city_player.h"
#include "figure/figure.h"
#include "graphics/image_collection.h"
#include "map/grid.h"

char obelisk_script_name_text[10][12] = {
    "Obelisk5x5f", "Obelisk5x5e", "Obelisk5x5d", "Obelisk5x5c", "Obelisk5x5b",
    "Obelisk5x5a", "Obelisk3x3d", "Obelisk3x3c", "Obelisk3x3b", "Obelisk3x3a",
};

char *obelisk_script_names[10] = {
    obelisk_script_name_text[9],
    obelisk_script_name_text[8],
    obelisk_script_name_text[7],
    obelisk_script_name_text[6],
    obelisk_script_name_text[5],
    obelisk_script_name_text[4],
    obelisk_script_name_text[3],
    obelisk_script_name_text[2],
    obelisk_script_name_text[1],
    obelisk_script_name_text[0],
};

static int32_t is_small(const building *b)
{
    return b->type == BUILDING_SMALL_OBELISK;
}

static int32_t spot_count(const building *b)
{
    return is_small(b) ? OBELISK_SMALL_WORK_SPOTS : OBELISK_LARGE_WORK_SPOTS;
}

static monument_work_spot *spot_entry(const building *b, int32_t index)
{
    return is_small(b) ? &obelisk_small_work_spots[index] : &obelisk_large_work_spots[index];
}

int32_t obelisk_init_on_place(int32_t building_id)
{
    building *b = building_get(building_id);
    monument_register(building_id);
    b->subtype = (int16_t) city_find_monument_slot(building_id);
    b->data.obelisk.progress = OBELISK_PROGRESS_START;
    obelisk_reset_work_spots(building_id);
    return 1;
}

void obelisk_update_image(int32_t building_id, int32_t x, int32_t y)
{
    building *b = building_get(building_id);
    int32_t scaled = (b->data.obelisk.progress - (2 * OBELISK_PROGRESS_THIRD)) * 3;
    if (scaled <= 0) {
        scaled = 0;
    }
    int32_t scripts = is_small(b) ? OBELISK_SMALL_SCRIPTS : OBELISK_LARGE_SCRIPTS;
    int32_t index = (scripts * scaled) / 100;
    if (index >= scripts - 1) {
        index = scripts - 1;
    }
    anim_load_script(obelisk_script_names[(is_small(b) ? 0 : OBELISK_SMALL_SCRIPTS) + index], MONUMENT_SCRIPT_SLOT_STAGE_A + b->subtype);
    int32_t image;
    switch (b->subtype) {
        case 0:
            image = image_group(IMAGE_GROUP_SPHINX_END_A);
            break;
        case 1:
            image = image_group(IMAGE_GROUP_SPHINX_MIDDLE);
            break;
        case 2:
            image = image_group(IMAGE_GROUP_SPHINX_END_B);
            break;
        default:
            image = 0;
            break;
    }
    map_building_tiles_add(building_id, x, y, b->size, b->size, image, TERRAIN_BUILDING);
}

void obelisk_load_script_for_slot(int32_t slot)
{
    int32_t type = scenario_monument_slot_obelisk_type(slot);
    if (type == BUILDING_SMALL_OBELISK) {
        anim_load_script(obelisk_script_names[0], MONUMENT_SCRIPT_SLOT_STAGE_A + slot);
    } else if (type == BUILDING_LARGE_OBELISK) {
        anim_load_script(obelisk_script_names[OBELISK_SMALL_SCRIPTS], MONUMENT_SCRIPT_SLOT_STAGE_A + slot);
    }
}

void obelisk_load_scripts_all_slots(void)
{
    for (int32_t slot = 0; slot < MONUMENT_SLOTS; slot++) {
        obelisk_load_script_for_slot(slot);
    }
}

int32_t obelisk_find_incomplete(void)
{
    const city_player *player = city_player_current();
    int32_t found = 0;
    for (int32_t slot = 0; slot < MONUMENT_SLOTS && !found; slot++) {
        int32_t id = player->monument_building_id[slot];
        if (id == 0) {
            continue;
        }
        building *b = building_get(id);
        int32_t is_obelisk = b->type == BUILDING_SMALL_OBELISK || b->type == BUILDING_LARGE_OBELISK;
        if (b->state == BUILDING_STATE_IN_USE && is_obelisk && b->data.obelisk.progress != OBELISK_PROGRESS_DONE) {
            found = id;
        }
    }
    return found;
}

int32_t obelisk_get_current_stage(int32_t building_id)
{
    building *b = building_get(building_id);
    int32_t n = spot_count(b);
    int32_t stage = (((b->data.obelisk.progress - OBELISK_PROGRESS_START) * n) / (2 * OBELISK_PROGRESS_THIRD)) + 1;
    return (stage < n) ? stage : n;
}

void obelisk_animate_spots(void)
{
    int32_t id = obelisk_find_incomplete();
    if (id == 0) {
        return;
    }
    building *b = building_get(id);
    for (int32_t i = 0; i < spot_count(b); i++) {
        monument_spot_animate(spot_entry(b, i)->spot);
    }
}

static int32_t progress_step(int32_t building_id)
{
    return (int32_t) (((double) (OBELISK_PROGRESS_DONE - OBELISK_PROGRESS_START) / spot_count(building_get(building_id))) + 0.5);
}

int32_t obelisk_advance_progress(int32_t building_id)
{
    building *b = building_get(building_id);
    if (b->data.obelisk.progress == OBELISK_PROGRESS_DONE) {
        return 1;
    }
    b->data.obelisk.progress = (uint8_t) (b->data.obelisk.progress + progress_step(building_id));
    if (b->data.obelisk.progress > OBELISK_PROGRESS_DONE) {
        b->data.obelisk.progress = OBELISK_PROGRESS_DONE;
    }
    obelisk_update_image(building_id, b->x, b->y);
    return b->data.obelisk.progress == OBELISK_PROGRESS_DONE;
}

void obelisk_reset_work_spots(int32_t building_id)
{
    building *b = building_get(building_id);
    for (int32_t i = 0; i < spot_count(b); i++) {
        monument_work_spot *entry = spot_entry(b, i);
        entry->spot->figure_id = 0;
        monument_spot_start_anim(entry->spot, MONUMENT_SPOT_IDLE);
        entry->spot->work_remaining = entry->initial_work;
    }
}

static monument_work_spot *find_free_spot(int32_t building_id, int32_t figure_type)
{
    building *b = building_get(building_id);
    for (int32_t i = 0; i < spot_count(b); i++) {
        monument_work_spot *entry = spot_entry(b, i);
        if (entry->spot->figure_id == 0 && entry->stage == obelisk_get_current_stage(building_id) && entry->figure_type == figure_type &&
            entry->spot->work_remaining > 0) {
            return entry;
        }
    }
    return 0;
}

int32_t obelisk_find_work_for_figure(int32_t figure_id)
{
    int32_t id = obelisk_find_incomplete();
    if (id == 0) {
        return 0;
    }
    figure *f = figure_get(figure_id);
    if (!find_free_spot(id, f->type)) {
        return 0;
    }
    building *b = building_get(id);
    f->destination_x = b->road_access_x;
    f->destination_y = b->road_access_y;
    f->destination_grid_offset = b->grid_offset;
    f->roam_length = 0;
    return id;
}

void obelisk_figure_work_tick(int32_t building_id, int32_t figure_id, uint8_t next_action)
{
    figure *f = figure_get(figure_id);
    building *b = building_get(building_id);
    f->is_ghost = 1;
    monument_work_spot *entry = 0;
    for (int32_t i = 0; i < spot_count(b); i++) {
        if (spot_entry(b, i)->spot->figure_id == figure_id) {
            entry = spot_entry(b, i);
            if (entry->stage != obelisk_get_current_stage(building_id)) {
                f->action_state = next_action;
                return;
            }
            break;
        }
    }
    if (!entry) {
        entry = find_free_spot(building_id, f->type);
        if (!entry) {
            f->action_state = next_action;
            return;
        }
    }
    if (entry->spot->figure_id == 0) {
        monument_spot_attach_figure(entry->spot, figure_id);
        monument_spot_start_anim(entry->spot, MONUMENT_SPOT_LOOP);
    }
    entry->spot->work_remaining--;
    if (entry->spot->work_remaining < 1) {
        obelisk_advance_progress(building_id);
        f->action_state = next_action;
    }
}

void obelisk_draw_overlay(int32_t building_id, int32_t x, int32_t y)
{
    building *b = building_get(building_id);
    int32_t image = image_group(IMAGE_GROUP_MAUSOLEUM_GROUND);
    int32_t progress = b->data.obelisk.progress;
    if (progress >= OBELISK_PROGRESS_DONE) {
        return;
    }
    int32_t amount = 0;
    if (progress >= (2 * OBELISK_PROGRESS_THIRD)) {
        amount = (OBELISK_PROGRESS_DONE - progress) * 3;
    } else if (progress >= OBELISK_PROGRESS_THIRD) {
        amount = (progress * 3) - (3 * OBELISK_PROGRESS_THIRD);
    }
    if (progress >= OBELISK_PROGRESS_THIRD && amount >= OBELISK_PROGRESS_DONE) {
        amount = OBELISK_PROGRESS_DONE;
    }
    int32_t dx, dy;
    if (is_small(b)) {
        dx = 90;
        dy = 18;
    } else {
        dx = 152;
        dy = 14;
    }
    int32_t width = is_small(b) ? 1 : 6;
    x += dx;
    y += dy;
    float step = 100.0f / (float) (is_small(b) ? 2 : 4) * 0.5f;
    float amount_f = (float) amount;
    float drawn = 0.0f;
    for (int32_t count = 0; drawn < amount_f; count++) {
        int32_t column = (count & 1) ? width : -60 - width;
        monument_draw_image(image, column + x, (-((count / 2) + 1) * 60) - (width / 2) + y);
        drawn += step;
    }
    for (int32_t i = 0; i < spot_count(b); i++) {
        int32_t frame_x, frame_y, frame_image;
        monument_spot_get_draw_info(spot_entry(b, i)->spot, &frame_x, &frame_y, &frame_image);
        if (frame_image != 0) {
            monument_draw_image(frame_image, frame_x + x, frame_y + y);
        }
    }
}

void obelisk_count_assigned_figures(int32_t figure_type, int32_t *count, int32_t building_id)
{
    building *b = building_get(building_id);
    *count = 0;
    for (int32_t i = 0; i < spot_count(b); i++) {
        if (spot_entry(b, i)->spot->figure_id != 0 && spot_entry(b, i)->figure_type == figure_type) {
            ++*count;
        }
    }
}

int32_t obelisk_figure_is_working_on(int32_t figure_type, int32_t building_id)
{
    for (int32_t id = 1; id < MAX_FIGURES; id++) {
        figure *f = figure_get(id);
        if (f->state == FIGURE_STATE_ALIVE && f->type == figure_type) {
            building *b = building_get(building_id);
            if ((b->type == BUILDING_SMALL_OBELISK || b->type == BUILDING_LARGE_OBELISK) && building_id == map_grids.building_ids[f->destination_grid_offset]) {
                return 1;
            }
        }
    }
    return 0;
}

int32_t obelisk_has_free_spot_for_type(int32_t figure_type, int32_t building_id)
{
    return find_free_spot(building_id, figure_type) != 0;
}
