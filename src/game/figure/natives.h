// The emigrant, native, native trader and flotsam actions of figure/natives.c, the creation of the flotsam and the
// fishing points, and the flotsam's frame, start and restart wait tables.

#ifndef FIGURE_NATIVES_H
#define FIGURE_NATIVES_H

#include "scenario/scenario_types.h"
#include <stdint.h>

enum emigrant_limit {
    EMIGRANT_FRAMES = 12,
    EMIGRANT_WAIT_TICKS = 4,
    EMIGRANT_ROAD_RADIUS = 5,
    EMIGRANT_DROWNING_TICKS = 33,
    EMIGRANT_BUBBLES_LAST_FRAME = 22,
};

enum native_limit {
    NATIVE_FRAMES = 12,
    NATIVE_MAX_ROAM_LENGTH = 800,
    NATIVE_WAIT_TICKS = 10,
    NATIVE_WAIT_SPREAD = 3,
    NATIVE_ATTACK_SECURITY_BREACH = 10,
    NATIVE_FIGHT_WINDUP = 12,
};

enum native_image {
    NATIVE_IMAGE_MARCH = 96,
    NATIVE_IMAGE_FIGHT = 192,
    NATIVE_IMAGE_CORPSE = 240,
};

enum native_trader_limit {
    NATIVE_TRADER_FRAMES = 12,
    NATIVE_TRADER_WAIT_TICKS = 10,
    NATIVE_TRADER_AMOUNT_PER_PURCHASE = 3,
};

enum flotsam_kind {
    FLOTSAM_KIND_BOBBING = 0,
    FLOTSAM_KIND_DEBRIS_A = 1,
    FLOTSAM_KIND_DEBRIS_B = 2,
    FLOTSAM_KIND_SINKING = 3,
};

enum flotsam_limit {
    FLOTSAM_COUNT = 20,
    FLOTSAM_BOB_FRAMES = 12,
    FLOTSAM_DEBRIS_FRAMES = 24,
    FLOTSAM_SUNK_FRAME = -1,
    FLOTSAM_RESTART_STEPS = 5,
};

typedef struct flotsam_start {
    int32_t kind;
    int32_t wait;
} flotsam_start;

typedef struct flotsam_restart_wait {
    int32_t floated_at_least;
    int32_t random_mask;
    int32_t base;
} flotsam_restart_wait;

extern int32_t flotsam_bob_frames[FLOTSAM_BOB_FRAMES];
extern int32_t flotsam_debris_frames[FLOTSAM_DEBRIS_FRAMES];
extern int32_t flotsam_sinking_frames[FLOTSAM_DEBRIS_FRAMES];
extern flotsam_start flotsam_starts[FLOTSAM_COUNT];
extern flotsam_restart_wait flotsam_restart_waits[FLOTSAM_RESTART_STEPS];

void figure_emigrant_action(void);
void figure_indigenous_native_action(void);
void figure_native_trader_action(void);
void figure_flotsam_action(void);
void figure_create_flotsam(int32_t x, int32_t y, int32_t create);

#endif
