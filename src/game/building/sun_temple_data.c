// The built-in stone blocks of the sun temple: per stage the number of blocks drawn, per block its screen offset.
// config/sun_temple_stage_blocks.json and config/sun_temple_block_offsets.json can override them.

#include "building/sun_temple.h"

int32_t sun_temple_stage_blocks[SUN_TEMPLE_STAGES] = { 0, 1, 2, 3, 4, 4, 3, 3, 2, 2, 1, 1, 0, 0 };

sun_temple_block_offset sun_temple_block_offsets[SUN_TEMPLE_BLOCKS] = {
    { 60, -29 },
    { 187, -29 },
    { 82, -96 },
    { 170, -100 },
};
