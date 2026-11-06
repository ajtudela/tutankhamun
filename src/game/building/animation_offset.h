// The animation offset of a building (building/animation_offset.c): which frame of its animation a building tile
// shows.

#ifndef BUILDING_ANIMATION_OFFSET_H
#define BUILDING_ANIMATION_OFFSET_H

#include <stdint.h>

enum animation_sprite_bits {
    SPRITE_FRAME_MASK = 0x7F,
    SPRITE_RUNS_BACK = 0x80,
};

enum well_animation {
    WELL_RADIUS = 2,
};

int32_t building_animation_offset(int32_t image, int32_t grid_offset);

#endif
