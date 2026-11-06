// The sphinx: three 6x6 records chained to each other, its stage kept in a city field. Workers take a free spot of
// the current stage and work it down, the stage advances when no spot has work left, and the stage scripts follow
// it; also the work percentages of the info window.

#include "building/sphinx.h"
#include "building/building_type.h"
#include "building/monument_spots.h"
#include "city/city_player.h"
#include "figure/figure.h"
#include "game/game_data.h"
#include "map/grid.h"

char sphinx_stage_script_name_text[15][12] = {
    "SPHINX6C", "SPHINX6B", "SPHINX6A", "SPHINX5C", "SPHINX5B", "SPHINX5A", "SPHINX4C", "SPHINX4B",
    "SPHINX4A", "SPHINX3C", "SPHINX3B", "SPHINX3A", "SPHINX2C", "SPHINX2B", "SPHINX2A",
};

char *sphinx_stage_script_names[51] = {
    s_sphinx1d.sphinx1a, s_sphinx1d.sphinx1b, s_sphinx1d.sphinx1c,
    s_sphinx1d.sphinx1a, s_sphinx1d.sphinx1b, s_sphinx1d.sphinx1c,
    sphinx_stage_script_name_text[14], s_sphinx1d.sphinx1b, s_sphinx1d.sphinx1c,
    sphinx_stage_script_name_text[14], sphinx_stage_script_name_text[13], s_sphinx1d.sphinx1c,
    sphinx_stage_script_name_text[14], sphinx_stage_script_name_text[13], sphinx_stage_script_name_text[12],
    sphinx_stage_script_name_text[11], sphinx_stage_script_name_text[13], sphinx_stage_script_name_text[12],
    sphinx_stage_script_name_text[11], sphinx_stage_script_name_text[10], sphinx_stage_script_name_text[12],
    sphinx_stage_script_name_text[11], sphinx_stage_script_name_text[10], sphinx_stage_script_name_text[9],
    sphinx_stage_script_name_text[8], sphinx_stage_script_name_text[10], sphinx_stage_script_name_text[9],
    sphinx_stage_script_name_text[8], sphinx_stage_script_name_text[7], sphinx_stage_script_name_text[9],
    sphinx_stage_script_name_text[8], sphinx_stage_script_name_text[7], sphinx_stage_script_name_text[6],
    sphinx_stage_script_name_text[5], sphinx_stage_script_name_text[7], sphinx_stage_script_name_text[6],
    sphinx_stage_script_name_text[5], sphinx_stage_script_name_text[4], sphinx_stage_script_name_text[6],
    sphinx_stage_script_name_text[5], sphinx_stage_script_name_text[4], sphinx_stage_script_name_text[3],
    sphinx_stage_script_name_text[2], sphinx_stage_script_name_text[4], sphinx_stage_script_name_text[3],
    sphinx_stage_script_name_text[2], sphinx_stage_script_name_text[1], sphinx_stage_script_name_text[3],
    sphinx_stage_script_name_text[2], sphinx_stage_script_name_text[1], sphinx_stage_script_name_text[0],
};

static monument_work_spot *find_free_spot(int32_t figure_type)
{
    int32_t stage = city_player_current()->sphinx_stage;
    for (int32_t i = 0; i < SPHINX_WORK_SPOTS; i++) {
        monument_work_spot *entry = &sphinx_work_spots[i];
        if (entry->spot->figure_id == 0 && entry->stage == stage && entry->figure_type == figure_type && entry->spot->work_remaining > 0) {
            return entry;
        }
    }
    return 0;
}

void sphinx_animate_spots(void)
{
    for (int32_t i = 0; i < SPHINX_WORK_SPOTS; i++) {
        monument_spot_animate(sphinx_work_spots[i].spot);
    }
}

void sphinx_reset_work_spots(void)
{
    for (int32_t i = 0; i < SPHINX_WORK_SPOTS; i++) {
        monument_work_spot *entry = &sphinx_work_spots[i];
        entry->spot->figure_id = 0;
        monument_spot_start_anim(entry->spot, MONUMENT_SPOT_IDLE);
        entry->spot->work_remaining = entry->initial_work;
    }
}

void sphinx_draw_spots(int32_t x, int32_t y)
{
    for (int32_t i = 0; i < SPHINX_WORK_SPOTS; i++) {
        int32_t frame_x, frame_y, image;
        monument_spot_get_draw_info(sphinx_work_spots[i].spot, &frame_x, &frame_y, &image);
        if (image != 0) {
            monument_draw_image(image, frame_x + x, frame_y + y);
        }
    }
}

void sphinx_check_stage_done(void)
{
    int32_t stage = city_player_current()->sphinx_stage;
    for (int32_t i = 0; i < SPHINX_WORK_SPOTS; i++) {
        monument_work_spot *entry = &sphinx_work_spots[i];
        if (entry->stage == stage && entry->spot->work_remaining > 0) {
            return;
        }
    }
    sphinx_advance_stage();
}

void sphinx_figure_work_tick(int32_t building_id, int32_t figure_id, uint8_t next_action)
{
    figure *f = figure_get(figure_id);
    f->is_ghost = 1;
    monument_work_spot *entry = 0;
    for (int32_t i = 0; i < SPHINX_WORK_SPOTS; i++) {
        if (sphinx_work_spots[i].spot->figure_id == figure_id) {
            entry = &sphinx_work_spots[i];
            if (entry->stage != city_player_current()->sphinx_stage) {
                f->action_state = next_action;
                return;
            }
            break;
        }
    }
    if (!entry) {
        entry = find_free_spot(f->type);
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
        sphinx_check_stage_done();
        f->action_state = next_action;
    }
}

int32_t sphinx_find_work_for_figure(int32_t figure_id)
{
    figure *f = figure_get(figure_id);
    if (!find_free_spot(f->type)) {
        return 0;
    }
    for (int32_t id = 1; id < MAX_BUILDINGS; id++) {
        building *b = building_get(id);
        if (b->state == BUILDING_STATE_IN_USE && b->type == BUILDING_SPHINX) {
            f->destination_x = b->road_access_x;
            f->destination_y = b->road_access_y;
            f->destination_grid_offset = b->grid_offset;
            f->roam_length = 0;
            return id;
        }
    }
    return 0;
}

void sphinx_count_assigned(int32_t figure_type, int32_t *count)
{
    *count = 0;
    for (int32_t i = 0; i < SPHINX_WORK_SPOTS; i++) {
        if (sphinx_work_spots[i].spot->figure_id != 0 && sphinx_work_spots[i].figure_type == figure_type) {
            ++*count;
        }
    }
}

int32_t sphinx_figure_is_working_on(int32_t figure_type)
{
    for (int32_t id = 1; id < MAX_FIGURES; id++) {
        figure *f = figure_get(id);
        if (f->state == FIGURE_STATE_ALIVE && f->type == figure_type && building_get(map_grids.building_ids[f->destination_grid_offset])->type == BUILDING_SPHINX) {
            return 1;
        }
    }
    return 0;
}

bool sphinx_has_free_spot_for_type(int32_t figure_type)
{
    return find_free_spot(figure_type) != 0;
}

void sphinx_advance_stage(void)
{
    city_player *player = city_player_current();
    int16_t stage = player->sphinx_stage;
    if (stage != 0 && stage < SPHINX_STAGE_COUNT) {
        player->sphinx_stage = stage + 1;
        sphinx_load_stage_scripts();
    }
}

void sphinx_load_stage_scripts(void)
{
    char *const *stage_scripts = &sphinx_stage_script_names[city_player_current()->sphinx_stage * SPHINX_SCRIPTS_PER_STAGE];
    anim_load_script(stage_scripts[0], MONUMENT_SCRIPT_SLOT_STAGE_A);
    anim_load_script(stage_scripts[1], MONUMENT_SCRIPT_SLOT_STAGE_B);
    anim_load_script(stage_scripts[2], MONUMENT_SCRIPT_SLOT_STAGE_C);
    anim_load_script(s_sphinx1d.sphinx1d, MONUMENT_SCRIPT_SLOT_SPHINX_1D);
}

int32_t sphinx_get_work_percentages(
    int32_t *carpenter_percent, int32_t *mason_percent, int32_t *carpenters, int32_t *carpenters_done, int32_t *masons, int32_t *masons_done)
{
    *masons = 0;
    *carpenters = 0;
    *masons_done = 0;
    *carpenters_done = 0;
    int32_t stage = city_player_current()->sphinx_stage;
    for (int32_t i = 0; i < SPHINX_WORK_SPOTS; i++) {
        const monument_work_spot *s = &sphinx_work_spots[i];
        if (s->figure_type == FIGURE_CARPENTER) {
            (*carpenters)++;
        } else {
            (*masons)++;
        }
        if (s->stage < stage) {
            if (s->figure_type == FIGURE_CARPENTER) {
                (*carpenters_done)++;
            } else {
                (*masons_done)++;
            }
        }
    }
    *carpenter_percent = (*carpenters_done * 100) / *carpenters;
    int32_t percent = (*masons_done * 100) / *masons;
    *mason_percent = percent;
    return percent;
}
