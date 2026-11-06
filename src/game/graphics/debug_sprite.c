// The developers' animated test sprite and the manager that owns a pool of its slots: animations of a model
// (loop, play once, hold the last frame, chain to the next, loop with doubled repeats), directions picked from
// the view angle (mirrored animations store half a circle), frames advanced by a speed per tick.
// Only the debug frame update reaches it; in the game both objects stay null.
//
// Results are only partly written:
// - a direction result leaves its high word unset;
// - a sprite without a model returns with only the low word of the result cleared.

#include "graphics/debug_sprite.h"
#include "core/crt_compat.h"
#include "graphics/graphics_data.h"
#include "widget/widget_data.h"
#include <stdint.h>
#include <stdlib.h>

enum debug_sprite_angle {
    VIEW_HALF_CIRCLE = 180,
    VIEW_FULL_CIRCLE = 360,
    MIRRORED_VIEW_START = 135,
    PLAIN_VIEW_START = 225,
};

enum debug_sprite_timing {
    TICKS_PER_SPEED_UNIT = 6000,
    MS_PER_SECOND = 1000,
};

static uint16_t anim_repeats(const debug_sprite_animation *animation)
{
    return (uint16_t) (animation->repeat_count << ((animation->mode & DEBUG_SPRITE_MODE_MASK) == DEBUG_SPRITE_MODE_LOOP_TWICE));
}

void debug_sprite_set_frame(debug_sprite *s, int32_t requested_frame)
{
    int16_t frame = (int16_t) requested_frame;
    debug_sprite_direction *direction = s->direction;
    if (!direction) {
        return;
    }
    while (frame < 0) {
        frame = (int16_t) (frame + direction->frame_count);
    }
    int32_t count = direction->frame_count;
    while (frame >= count) {
        frame = (int16_t) (frame % count);
    }
    s->frame = (float) frame;
}

void debug_sprite_set_direction(debug_sprite *s, int32_t direction)
{
    if (s->animation) {
        s->direction = &s->animation->directions[(uint16_t) direction];
    }
}

void debug_sprite_set_direction_frame(debug_sprite *s, int32_t direction, int32_t frame)
{
    if (s->animation) {
        debug_sprite_set_direction(s, direction);
        s->frame = (float) (int16_t) frame;
    }
}

int16_t debug_sprite_first_frame(const debug_sprite_animation *animation, const debug_sprite_direction *direction)
{
    return 0;
}

static void single_view_direction(float angle, int32_t *mirrored)
{
    *mirrored = angle >= (float) VIEW_HALF_CIRCLE;
}

static uint16_t mirrored_view_direction(int32_t count, float angle, int32_t *mirrored)
{
    int32_t views = (count * 2) - 2;
    int32_t half_step = VIEW_FULL_CIRCLE / (views * 2);
    double v = ((double) VIEW_FULL_CIRCLE - ((double) angle - (double) MIRRORED_VIEW_START)) + (double) half_step;
    while (!(v >= 0.0)) {
        v += (double) VIEW_FULL_CIRCLE;
    }
    while (v >= (double) VIEW_FULL_CIRCLE) {
        v -= (double) VIEW_FULL_CIRCLE;
    }
    float step = (float) ((double) VIEW_FULL_CIRCLE / (double) views);
    double q = v / (double) step;
    float qf = (float) q;
    if (!(q > ((double) views * 0.5))) {
        return (uint16_t) crt_ftol_double((double) qf);
    }
    uint16_t direction = (uint16_t) crt_ftol_double((((double) VIEW_FULL_CIRCLE - v) / (double) step) + 1.0);
    *mirrored = 1;
    return direction;
}

static uint16_t plain_view_direction(int32_t count, float angle)
{
    int32_t half_step = VIEW_FULL_CIRCLE / (count * 2);
    double v = ((double) VIEW_FULL_CIRCLE - ((double) angle + (double) PLAIN_VIEW_START)) + (double) half_step;
    if (!(v >= 0.0)) {
        v += (double) VIEW_FULL_CIRCLE;
    } else if (v >= (double) VIEW_FULL_CIRCLE) {
        v -= (double) VIEW_FULL_CIRCLE;
    }
    return (uint16_t) crt_ftol_double(v / ((double) VIEW_FULL_CIRCLE / (double) count));
}

direction_result *debug_sprite_direction_from_angle(direction_result *out, const debug_sprite_animation *animation, float angle)
{
    uint16_t direction = 0;
    int32_t mirrored = 0;
    uint8_t count = animation->direction_count;
    if (count <= 1) {
        single_view_direction(angle, &mirrored);
    } else if (animation->direction_flags & DEBUG_SPRITE_MIRRORED_DIRECTIONS) {
        direction = mirrored_view_direction(count, angle, &mirrored);
    } else {
        direction = plain_view_direction(count, angle);
    }
    out->direction = direction;
    out->mirrored = mirrored;
    return out;
}

uint16_t debug_sprite_view_direction(debug_sprite *s)
{
    if (!s->model) {
        return 0;
    }
    if (!s->model->data) {
        return 0;
    }
    const debug_sprite_animation *animation = s->animation;
    float angle = s->view.ops->angle(&s->view);
    direction_result r;
    debug_sprite_direction_from_angle(&r, animation, angle);
    if (r.mirrored) {
        s->flags |= DEBUG_SPRITE_MIRRORED;
    } else {
        s->flags &= ~DEBUG_SPRITE_MIRRORED;
    }
    return (uint16_t) r.direction;
}

static void set_frame_sizes(debug_sprite *s, const debug_sprite_animation *animation)
{
    const debug_sprite_frame *middle = animation->directions[animation->direction_count >> 1].frames;
    const debug_sprite_frame *first = animation->directions[0].frames;
    s->width = first->width;
    s->middle_width = middle->width;
    s->middle_offset = middle->height - (s->width / 2);
}

void debug_sprite_set_anim(debug_sprite *s, int32_t anim_index, int32_t requested_frame)
{
    uint16_t anim = (uint16_t) anim_index;
    int16_t frame = (int16_t) requested_frame;
    if (!s->model) {
        return;
    }
    if (anim != 0) {
        const debug_sprite_model_data *data = s->model->data;
        if (anim >= data->animation_count) {
            anim = (uint16_t) (data->animation_count - 1);
        }
    }
    s->flags &= ~DEBUG_SPRITE_ENDED;
    s->animation_index = anim;
    debug_sprite_model_data *data = s->model ? s->model->data : 0;
    s->animation = &data->animations[anim];
    if (frame == DEBUG_SPRITE_KEEP_FRAME) {
        debug_sprite_set_direction(s, debug_sprite_view_direction(s));
    } else {
        debug_sprite_set_direction_frame(s, debug_sprite_view_direction(s), frame);
    }
    debug_sprite_animation *animation = s->animation;
    s->repeats_left = anim_repeats(animation);
    if (frame == DEBUG_SPRITE_FIRST_FRAME) {
        debug_sprite_set_frame(s, debug_sprite_first_frame(animation, s->direction));
    }
    uint16_t count = s->direction->frame_count;
    if (!((double) count > (double) s->frame)) {
        debug_sprite_set_frame(s, (int16_t) (count - 1));
    }
    animation = s->animation;
    s->speed = (float) animation->speed;
    if (animation->direction_count > 1) {
        set_frame_sizes(s, animation);
    }
}

void debug_sprite_start_anim(debug_sprite *s, int32_t anim)
{
    debug_sprite_set_anim(s, (uint16_t) anim, DEBUG_SPRITE_FIRST_FRAME);
}

int32_t debug_sprite_advance_frame(debug_sprite *s, float step)
{
    double sum = (double) step + (double) s->frame;
    s->frame = (float) sum;
    return sum >= (double) s->direction->frame_count;
}

int32_t debug_sprite_rewind_frame(debug_sprite *s, float step)
{
    double sum = (double) step + (double) s->frame;
    s->frame = (float) sum;
    return !(sum >= 0.0);
}

static int32_t count_repeat(debug_sprite *s)
{
    uint16_t repeats = s->repeats_left;
    if (repeats == 0) {
        return 0;
    }
    repeats--;
    s->repeats_left = repeats;
    return repeats == 0;
}

int32_t debug_sprite_wrap_forward(debug_sprite *s)
{
    float count = (float) s->direction->frame_count;
    int32_t ended = 0;
    while (s->frame >= count && !ended) {
        double v = (double) s->frame - (double) count;
        s->frame = (float) v;
        if (count_repeat(s)) {
            if (v >= (double) count) {
                s->frame = (float) ((double) count - 1.0);
            }
            ended = 1;
            s->repeats_left = anim_repeats(s->animation);
        }
    }
    return ended;
}

int32_t debug_sprite_wrap_backward(debug_sprite *s)
{
    float count = (float) s->direction->frame_count;
    int32_t ended = 0;
    while (!(s->frame >= 0.0f) && !ended) {
        double v = (double) count + (double) s->frame;
        s->frame = (float) v;
        if (count_repeat(s)) {
            if (!(v >= 0.0)) {
                s->frame = 0.0f;
            }
            ended = 1;
            s->repeats_left = anim_repeats(s->animation);
        }
    }
    return ended;
}

int32_t debug_sprite_step(debug_sprite *s)
{
    double v = (double) s->speed * (double) s->speed_scale * (double) (float) (1.0 / (double) TICKS_PER_SPEED_UNIT);
    if (debug_key_toggle != 0) {
        double ms = (double) (uint32_t) frame_time_delta;
        int32_t frame_ms = crt_ftol_double((double) (float) MS_PER_SECOND / (double) debug_key_value);
        v = v * (ms / (double) frame_ms);
    }
    float step = (float) v;
    if (!(v >= 0.0)) {
        return debug_sprite_rewind_frame(s, step) ? debug_sprite_wrap_backward(s) : 0;
    }
    return debug_sprite_advance_frame(s, step) ? debug_sprite_wrap_forward(s) : 0;
}

int32_t debug_sprite_update_loop(debug_sprite *s)
{
    return debug_sprite_step(s) != 0;
}

int32_t debug_sprite_update_once(debug_sprite *s)
{
    if (debug_sprite_step(s)) {
        s->flags |= DEBUG_SPRITE_ONCE_ENDED;
        return 1;
    }
    return 0;
}

int32_t debug_sprite_update_hold_last(debug_sprite *s)
{
    int16_t frame = (int16_t) crt_ftol_double((double) s->frame);
    if ((int32_t) frame == (int32_t) s->direction->frame_count - 1) {
        s->flags |= DEBUG_SPRITE_ENDED;
        return 1;
    }
    if (debug_sprite_step(s)) {
        debug_sprite_set_frame(s, (int16_t) (s->direction->frame_count - 1));
        return 1;
    }
    return 0;
}

int32_t debug_sprite_update_chain(debug_sprite *s)
{
    if (debug_sprite_step(s)) {
        debug_sprite_start_anim(s, s->animation->next_animation);
        return 1;
    }
    return 0;
}

int32_t debug_sprite_update(void *sprite)
{
    debug_sprite *s = sprite;
    if (!s->model) {
        return 0;
    }
    int32_t r;
    switch (s->animation->mode & DEBUG_SPRITE_MODE_MASK) {
        case DEBUG_SPRITE_MODE_LOOP:
            r = debug_sprite_update_loop(s);
            break;
        case DEBUG_SPRITE_MODE_HOLD_LAST:
            return debug_sprite_update_hold_last(s);
        case DEBUG_SPRITE_MODE_ONCE:
            r = debug_sprite_update_once(s);
            break;
        case DEBUG_SPRITE_MODE_CHAIN:
            r = debug_sprite_update_chain(s);
            break;
        case DEBUG_SPRITE_MODE_LOOP_TWICE:
            r = debug_sprite_update_loop(s);
            break;
        default:
            r = 0;
            break;
    }
    if (r) {
        s->flags |= DEBUG_SPRITE_ENDED;
        return r;
    }
    s->flags &= ~DEBUG_SPRITE_ENDED;
    return 0;
}

int32_t debug_sprite_manager_release_slot(debug_sprite_manager *m, int32_t index)
{
    if (index >= 0) {
        debug_sprite_block_stack *free_slots = m->free_slots;
        free_slots->top--;
        *free_slots->top = &m->slots[index];
    }
    return 0;
}

void debug_sprite_slot_destroy(debug_sprite_slot *slot)
{
    if (slot->block) {
        free(slot->block);
        slot->block = 0;
    }
    if (slot->manager && slot->index >= 0) {
        debug_sprite_manager_release_slot(slot->manager, slot->index);
    }
}

static void delete_block_stack(debug_sprite_block_stack *stack)
{
    free(stack->block);
    free(stack);
}

static void delete_slots(debug_sprite_slot *slots)
{
    int32_t *array_header = (int32_t *) slots - 1;
    for (int32_t i = *array_header - 1; i >= 0; i--) {
        debug_sprite_slot_destroy(&slots[i]);
    }
    free(array_header);
}

void debug_sprite_manager_destroy(debug_sprite_manager *m)
{
    debug_sprite_owned_object *object = m->object;
    if (object) {
        object->ops->destroy(object, DEBUG_SPRITE_DELETE_ARRAY);
        m->object = 0;
    }
    if (m->buffers) {
        delete_block_stack(m->buffers);
        m->buffers = 0;
    }
    if (m->slots) {
        delete_slots(m->slots);
        m->slots = 0;
    }
    if (m->free_slots) {
        delete_block_stack(m->free_slots);
        m->free_slots = 0;
    }
    if (debug_frame.sprite_pool_buffer) {
        free(debug_frame.sprite_pool_buffer);
        debug_frame.sprite_pool_buffer = 0;
        debug_frame.sprite_pool_buffer_size = 0;
    }
}

void debug_sprite_manager_delete(void **manager_ptr)
{
    debug_sprite_manager *m = *manager_ptr;
    if (m) {
        debug_sprite_manager_destroy(m);
        free(m);
        *manager_ptr = 0;
    }
}
