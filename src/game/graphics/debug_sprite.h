// The developers' animated test sprite and its manager: the records (sprite, model, animation, direction,
// frame, manager, slot) and their methods, which take the object as their first argument.

#ifndef GRAPHICS_DEBUG_SPRITE_H
#define GRAPHICS_DEBUG_SPRITE_H

#include <stdint.h>

enum debug_sprite_mode {
    DEBUG_SPRITE_MODE_LOOP = 0,
    DEBUG_SPRITE_MODE_HOLD_LAST = 1,
    DEBUG_SPRITE_MODE_ONCE = 2,
    DEBUG_SPRITE_MODE_CHAIN = 3,
    DEBUG_SPRITE_MODE_LOOP_TWICE = 4,
    DEBUG_SPRITE_MODE_MASK = 7,
};

enum debug_sprite_flag {
    DEBUG_SPRITE_MIRRORED = 0x1,
    DEBUG_SPRITE_ONCE_ENDED = 0x8,
    DEBUG_SPRITE_ENDED = 0x100,
};

enum debug_sprite_animation_flag {
    DEBUG_SPRITE_MIRRORED_DIRECTIONS = 0x80,
};

enum debug_sprite_frame_request {
    DEBUG_SPRITE_FIRST_FRAME = 0,
    DEBUG_SPRITE_KEEP_FRAME = 1000,
};

typedef struct debug_sprite_frame {
    uint16_t reserved_1;
    uint16_t width;
    uint16_t height;
    uint16_t reserved_2;
} debug_sprite_frame;

typedef struct debug_sprite_direction {
    uint16_t index;
    uint16_t frame_count;
    debug_sprite_frame *frames;
} debug_sprite_direction;

typedef struct debug_sprite_animation {
    uint8_t mode;
    uint8_t direction_flags;
    uint8_t speed;
    uint8_t repeat_count;
    uint8_t next_animation;
    uint8_t reserved_1[6];
    uint8_t direction_count;
    debug_sprite_direction *directions;
} debug_sprite_animation;

typedef struct debug_sprite_model_data {
    uint8_t reserved_1[26];
    uint16_t animation_count;
    debug_sprite_animation *animations;
} debug_sprite_model_data;

typedef struct debug_sprite_model {
    uint8_t reserved_1[8];
    debug_sprite_model_data *data;
    uint8_t reserved_2[4];
} debug_sprite_model;

typedef struct debug_sprite_view debug_sprite_view;

typedef struct {
    float (*angle)(debug_sprite_view *view);
} debug_sprite_view_ops;

struct debug_sprite_view {
    const debug_sprite_view_ops *ops;
};

typedef struct debug_sprite {
    uint8_t reserved_1[4];
    debug_sprite_view view;
    uint8_t reserved_2[16];
    int32_t width;
    int32_t middle_width;
    int32_t middle_offset;
    uint8_t reserved_3[16];
    uint32_t flags;
    debug_sprite_model *model;
    uint16_t animation_index;
    uint8_t reserved_4[2];
    debug_sprite_animation *animation;
    debug_sprite_direction *direction;
    float frame;
    float speed_scale;
    float speed;
    uint8_t reserved_5[36];
    uint16_t repeats_left;
    uint8_t reserved_6[6];
} debug_sprite;

enum debug_sprite_destructor_flag {
    DEBUG_SPRITE_DELETE_OBJECT = 1,
    DEBUG_SPRITE_DELETE_ARRAY = 3,
};

typedef struct debug_sprite_owned_object debug_sprite_owned_object;

typedef struct {
    void (*destroy)(debug_sprite_owned_object *self, int32_t flags);
    void (*reserved[5])(void);
    void (*draw)(debug_sprite_owned_object *self, int32_t x, int32_t y);
} debug_sprite_owned_ops;

struct debug_sprite_owned_object {
    const debug_sprite_owned_ops *ops;
    int32_t reserved_1;
};

struct debug_sprite_slot;

typedef struct debug_sprite_block_stack {
    struct debug_sprite_slot **top;
    int32_t reserved_1;
    void *block;
} debug_sprite_block_stack;

typedef struct debug_sprite_manager {
    uint8_t reserved_1[276];
    debug_sprite_owned_object *object;
    debug_sprite_block_stack *buffers;
    struct debug_sprite_slot *slots;
    debug_sprite_block_stack *free_slots;
} debug_sprite_manager;

typedef struct debug_sprite_slot {
    uint8_t reserved_1[4];
    debug_sprite_manager *manager;
    void *block;
    uint8_t reserved_2[4];
    int32_t index;
    uint8_t reserved_3[4];
} debug_sprite_slot;

typedef struct {
    uint32_t direction;
    int32_t mirrored;
} direction_result;

void debug_sprite_set_frame(debug_sprite *s, int32_t requested_frame);
void debug_sprite_set_direction(debug_sprite *s, int32_t direction);
void debug_sprite_set_direction_frame(debug_sprite *s, int32_t direction, int32_t frame);
int16_t debug_sprite_first_frame(const debug_sprite_animation *animation, const debug_sprite_direction *direction);
direction_result *debug_sprite_direction_from_angle(direction_result *out, const debug_sprite_animation *animation, float angle);
uint16_t debug_sprite_view_direction(debug_sprite *s);
void debug_sprite_set_anim(debug_sprite *s, int32_t anim_index, int32_t requested_frame);
void debug_sprite_start_anim(debug_sprite *s, int32_t anim);
int32_t debug_sprite_advance_frame(debug_sprite *s, float step);
int32_t debug_sprite_rewind_frame(debug_sprite *s, float step);
int32_t debug_sprite_wrap_forward(debug_sprite *s);
int32_t debug_sprite_wrap_backward(debug_sprite *s);
int32_t debug_sprite_step(debug_sprite *s);
int32_t debug_sprite_update_loop(debug_sprite *s);
int32_t debug_sprite_update_once(debug_sprite *s);
int32_t debug_sprite_update_hold_last(debug_sprite *s);
int32_t debug_sprite_update_chain(debug_sprite *s);
int32_t debug_sprite_update(void *sprite);
int32_t debug_sprite_manager_release_slot(debug_sprite_manager *m, int32_t index);
void debug_sprite_slot_destroy(debug_sprite_slot *slot);
void debug_sprite_manager_destroy(debug_sprite_manager *m);
void debug_sprite_manager_delete(void **manager_ptr);

#endif
