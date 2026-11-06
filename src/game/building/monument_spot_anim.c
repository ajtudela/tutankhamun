// Animation of the monument work spots: starting a spot with a random first sequence, attaching a figure,
// advancing the frame (hold, loop with a random next sequence, or idle), the draw info of the current frame, and
// the per-tick animation of the spot tables of the scripted monuments and royal tombs.
//
// The royal tombs' chamber tables are animated in table order, which the random choices depend on.

#include "building/building_data.h"
#include "building/monument_anim_script.h"
#include "building/monument_data.h"
#include "building/monument_spots.h"
#include "core/crt_compat.h"
#include "figure/figure.h"
#include "graphics/image_collection.h"
#include <string.h>

static int32_t pick_choice(const anim_script *script, int32_t transition, monument_spot *spot)
{
    const anim_transition *t = &script->transitions[transition];
    int32_t count = 0;
    while (count < ANIM_TRANSITION_CHOICES && t->next[count].sequence >= 0) {
        count++;
    }
    if (count == 0) {
        return 0;
    }
    const anim_choice *choice = &t->next[crt_rand() % count];
    spot->sequence = choice->sequence;
    spot->reverse = (uint8_t) choice->reverse;
    if (choice->sequence >= 0 && choice->sequence < script->sequence_count) {
        spot->frame = choice->reverse ? script->sequences[choice->sequence].last_frame : 0;
        return 1;
    }
    return -1;
}

void monument_spot_start_anim(monument_spot *spot, int32_t mode)
{
    spot->mode = (int16_t) mode;
    int32_t s = spot->script;
    if (s < 0 || s >= ANIM_SCRIPT_COUNT) {
        return;
    }
    const anim_script *script = &anim_scripts[s];
    spot->sequence = -1;
    if (script->transition_count <= 0) {
        return;
    }
    int32_t t = 0;
    while (script->transitions[t].sequence >= 0) {
        t++;
        if (t >= script->transition_count) {
            return;
        }
    }
    if (pick_choice(script, t, spot) < 0) {
        spot->sequence = -1;
    }
}

void monument_spot_attach_figure(monument_spot *spot, int32_t figure_id)
{
    spot->figure_id = (int16_t) figure_id;
    if (figure_id) {
        spot->figure_action = figure_get(figure_id)->action_state;
    }
}

void monument_spot_animate(monument_spot *spot)
{
    if (spot->figure_id != 0 && figure_get(spot->figure_id)->action_state != spot->figure_action) {
        spot->figure_id = 0;
        spot->figure_action = 0;
        spot->mode = MONUMENT_SPOT_IDLE;
        return;
    }
    int32_t mode = spot->mode;
    if (mode == MONUMENT_SPOT_FROZEN || mode == MONUMENT_SPOT_IDLE) {
        return;
    }
    int32_t s = spot->script;
    if (s < 0 || s >= ANIM_SCRIPT_COUNT) {
        return;
    }
    const anim_script *script = &anim_scripts[s];
    int32_t sequence = spot->sequence;
    if (sequence < 0 || sequence >= script->sequence_count) {
        return;
    }
    int32_t frame = spot->reverse ? spot->frame - 1 : spot->frame + 1;
    spot->frame = frame;
    int32_t last = script->sequences[sequence].last_frame;
    if (frame <= last && frame >= 0) {
        return;
    }
    if (mode == MONUMENT_SPOT_HOLD) {
        if (frame > last) {
            spot->frame = last;
        } else if (frame < 0) {
            spot->frame = 0;
        }
        return;
    }
    for (int32_t t = 0; t < script->transition_count; t++) {
        const anim_transition *tr = &script->transitions[t];
        if (tr->sequence == sequence && tr->reverse == (int8_t) spot->reverse) {
            if (pick_choice(script, t, spot) > 0) {
                return;
            }
            break;
        }
    }
    monument_spot_start_anim(spot, MONUMENT_SPOT_IDLE);
}

static void spot_get_draw_info_view(monument_spot *spot, int32_t *x, int32_t *y, int32_t *image, int32_t view)
{
    *x = 0;
    *y = 0;
    *image = 0;
    if (spot->mode == MONUMENT_SPOT_IDLE || spot->script < 0 || spot->script >= ANIM_SCRIPT_COUNT) {
        return;
    }
    const anim_script *script = &anim_scripts[spot->script];
    if (view < 0) {
        view = map_info.orientation / 2;
    }
    if (spot->sequence < 0 || spot->sequence >= script->sequence_count) {
        return;
    }
    const anim_sequence *seq = &script->sequences[spot->sequence];
    int32_t add = spot->reverse ? seq->image_add_reverse[view] : seq->image_add[view];
    if (add < 0 || seq->images < 0 || seq->images >= script->image_count) {
        return;
    }
    const anim_images *img = &script->images[seq->images];
    if (seq->position_from < 0 || seq->position_from >= script->position_count || seq->position_to < 0 || seq->position_to >= script->position_count) {
        return;
    }
    const anim_position *from = &script->positions[(seq->position_from * ANIM_VIEWS) + view];
    const anim_position *to = &script->positions[(seq->position_to * ANIM_VIEWS) + view];
    if (spot->frame < 0 || spot->frame > seq->last_frame) {
        return;
    }
    *image = (spot->frame % img->frames) * img->frame_step + image_group(img->image_group) + img->image_offset + add;
    *x = (((to->x - from->x) * spot->frame) / seq->last_frame) + script->x_offset + from->x;
    *y = (((to->y - from->y) * spot->frame) / seq->last_frame) + script->y_offset + from->y;
    int32_t id = *image;
    *x -= image_record_sprite_offset_x(image_known_id(id));
    *y -= image_record_sprite_offset_y(image_known_id(id));
}

void monument_spot_get_draw_info(monument_spot *spot, int32_t *x, int32_t *y, int32_t *image)
{
    spot_get_draw_info_view(spot, x, y, image, -1);
}

void monument_spot_get_draw_info_dir(monument_spot *spot, int32_t *x, int32_t *y, int32_t *image, int32_t direction)
{
    int32_t view = direction - map_info.orientation / 2;
    if (view < 0) {
        view += ANIM_VIEWS;
    }
    spot_get_draw_info_view(spot, x, y, image, view);
}

static void animate_table(const monument_work_spot *first, const monument_work_spot *end)
{
    for (const monument_work_spot *entry = first; entry < end; entry++) {
        monument_spot_animate(entry->spot);
    }
}

void library_animate_spots(void)
{
    animate_table(library_work_spots, library_work_spots + LIBRARY_WORK_SPOTS);
}

void abu_simbel_animate_spots(void)
{
    animate_table(abu_simbel_work_spots, abu_simbel_work_spots + ABU_SIMBEL_WORK_SPOTS);
}

void caesareum_animate_spots(void)
{
    animate_table(caesareum_work_spots, caesareum_work_spots + CAESAREUM_WORK_SPOTS);
}

void lighthouse_animate_spots(void)
{
    animate_table(lighthouse_work_spots, lighthouse_work_spots + LIGHTHOUSE_WORK_SPOTS);
}

void royal_tomb_animate_spots(void)
{
    static const int8_t chamber_order[] = { 0, 1, 2, 12, 13, 3, 4, 5, 6, 14, 15, 7, 8, 19, 16, 20, 21, 9, 10, 11, 17, 18, 22, 23, 24 };
    for (int32_t i = 0; i < (int32_t) sizeof(chamber_order); i++) {
        const monument_work_spot *first = &royal_tomb_chamber_work_spots[chamber_order[i] * ROYAL_TOMB_SPOTS_PER_TABLE];
        animate_table(first, first + ROYAL_TOMB_SPOTS_PER_TABLE);
    }
}

