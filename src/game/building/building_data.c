// The built-in data of the buildings (not the monuments): properties, tiles, festivals, services and their
// names. building/building_data.h declares them.

#include "building/monument_object.h"
#include "window/advisor/ratings.h"
#include "building/house_evolve.h"

#include <assert.h>

uint8_t abu_simbel_stage_table[240] = { 3, 0, 0, 1, 3, 0, 0, 4, 3, 0, 0, 7, 3, 0, 0, 10, 3, 0, 0, 2, 3, 0, 0, 5, 3, 0, 0, 8, 3, 0, 0, 11, 3, 0, 0, 3, 3, 0, 0,
    6, 3, 0, 0, 9, 3, 0, 0, 12, 4, 1, 1, 1, 4, 1, 5, 1, 4, 1, 2, 1, 4, 1, 4, 1, 4, 0, 0, 10, 4, 1, 1, 2, 4, 0, 0, 11, 4, 1, 5, 2, 4, 0, 0, 8, 4, 1, 2, 2, 4, 0,
    0, 12, 4, 1, 4, 2, 4, 0, 0, 7, 4, 1, 1, 3, 4, 1, 5, 3, 4, 0, 0, 9, 4, 1, 4, 3, 4, 1, 1, 4, 4, 1, 2, 3, 4, 1, 4, 4, 4, 0, 0, 4, 4, 1, 1, 5, 4, 0, 0, 5, 4, 1,
    5, 4, 4, 0, 0, 6, 4, 1, 4, 5, 4, 0, 0, 1, 4, 1, 1, 6, 4, 0, 0, 2, 4, 1, 2, 4, 4, 1, 5, 5, 4, 1, 3, 1, 4, 1, 2, 5, 4, 0, 0, 3, 4, 1, 4, 6, 4, 1, 1, 7, 4, 1,
    5, 6, 4, 1, 2, 6, 4, 1, 4, 7, 4, 1, 1, 8, 4, 1, 4, 8, 4, 1, 1, 9, 4, 1, 4, 9, 4, 1, 3, 2 };

int32_t farm_harvest_months[40] = { 1, 7, -1, -1, 11, -1, -1, -1, 4, 0, -1, -1, 3, -1, -1, -1, 5, 10, -1, -1, 3, -1, -1, -1, -1, -1, -1, -1, 8, -1, -1, -1, 2,
    6, -1, -1, 11, -1, -1, -1 };

trade_price_walk trade_goods_list = {
    .list = {
        .resources = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 22, 23, 24, 25, 26, 28, 29, 30, 31, 32, 33, 34, 35 },
        .beauty_variant = -1,
        .after_beauty_variant = 0,
    },
};

int32_t overlay_submenu_ids[OVERLAY_SUBMENU_IDS] = { 373, 0, 0, 0, 1, 2, 3, 4, 5, 0, 0 };

int32_t gatehouse_rubble_offsets[98] = { 0, 0, 1, 0, 1, 1, 0, 1, 2, 0, 2, 1, 2, 2, 1, 2, 0, 2, 3, 0, 3, 1, 3, 2, 3, 3, 2, 3, 1, 3, 0, 3, 4, 0, 4, 1, 4, 2, 4, 3,
    4, 4, 3, 4, 2, 4, 1, 4, 0, 4, 5, 0, 5, 1, 5, 2, 5, 3, 5, 4, 5, 5, 4, 5, 3, 5, 2, 5, 1, 5, 0, 5, 6, 0, 6, 1, 6, 2, 6, 3, 6, 4, 6, 5, 6, 6, 5, 6, 4, 6, 3, 6,
    2, 6, 1, 6, 0, 6 };

footprint_offset_tables tile_grid_offsets_base = {
    .grid_offsets = {
        {
            0, GRID_SIZE, 1, GRID_SIZE + 1, 2 * GRID_SIZE, 2, 2 * GRID_SIZE + 1, GRID_SIZE + 2, 2 * GRID_SIZE + 2, 3 * GRID_SIZE, 3, 3 * GRID_SIZE + 1,
            GRID_SIZE + 3, 3 * GRID_SIZE + 2, 2 * GRID_SIZE + 3, 3 * GRID_SIZE + 3, 4 * GRID_SIZE, 4, 4 * GRID_SIZE + 1, GRID_SIZE + 4, 4 * GRID_SIZE + 2,
            2 * GRID_SIZE + 4, 4 * GRID_SIZE + 3, 3 * GRID_SIZE + 4, 4 * GRID_SIZE + 4, 5 * GRID_SIZE, 5, 5 * GRID_SIZE + 1, GRID_SIZE + 5, 5 * GRID_SIZE + 2,
            2 * GRID_SIZE + 5, 5 * GRID_SIZE + 3, 3 * GRID_SIZE + 5, 5 * GRID_SIZE + 4, 4 * GRID_SIZE + 5, 5 * GRID_SIZE + 5
        },
        {
            0, -1, GRID_SIZE, GRID_SIZE - 1, -2, 2 * GRID_SIZE, GRID_SIZE - 2, 2 * GRID_SIZE - 1, 2 * GRID_SIZE - 2, -3, 3 * GRID_SIZE, GRID_SIZE - 3,
            3 * GRID_SIZE - 1, 2 * GRID_SIZE - 3, 3 * GRID_SIZE - 2, 3 * GRID_SIZE - 3, -4, 4 * GRID_SIZE, GRID_SIZE - 4, 4 * GRID_SIZE - 1, 2 * GRID_SIZE - 4,
            4 * GRID_SIZE - 2, 3 * GRID_SIZE - 4, 4 * GRID_SIZE - 3, 4 * GRID_SIZE - 4, -5, 5 * GRID_SIZE, GRID_SIZE - 5, 5 * GRID_SIZE - 1, 2 * GRID_SIZE - 5,
            5 * GRID_SIZE - 2, 3 * GRID_SIZE - 5, 5 * GRID_SIZE - 3, 4 * GRID_SIZE - 5, 5 * GRID_SIZE - 4, 5 * GRID_SIZE - 5
        },
        {
            0, -GRID_SIZE, -1, -GRID_SIZE - 1, -2 * GRID_SIZE, -2, -2 * GRID_SIZE - 1, -GRID_SIZE - 2, -2 * GRID_SIZE - 2, -3 * GRID_SIZE, -3,
            -3 * GRID_SIZE - 1, -GRID_SIZE - 3, -3 * GRID_SIZE - 2, -2 * GRID_SIZE - 3, -3 * GRID_SIZE - 3, -4 * GRID_SIZE, -4, -4 * GRID_SIZE - 1,
            -GRID_SIZE - 4, -4 * GRID_SIZE - 2, -2 * GRID_SIZE - 4, -4 * GRID_SIZE - 3, -3 * GRID_SIZE - 4, -4 * GRID_SIZE - 4, -5 * GRID_SIZE, -5,
            -5 * GRID_SIZE - 1, -GRID_SIZE - 5, -5 * GRID_SIZE - 2, -2 * GRID_SIZE - 5, -5 * GRID_SIZE - 3, -3 * GRID_SIZE - 5, -5 * GRID_SIZE - 4,
            -4 * GRID_SIZE - 5, -5 * GRID_SIZE - 5
        },
        {
            0, 1, -GRID_SIZE, -GRID_SIZE + 1, 2, -2 * GRID_SIZE, -GRID_SIZE + 2, -2 * GRID_SIZE + 1, -2 * GRID_SIZE + 2, 3, -3 * GRID_SIZE, -GRID_SIZE + 3,
            -3 * GRID_SIZE + 1, -2 * GRID_SIZE + 3, -3 * GRID_SIZE + 2, -3 * GRID_SIZE + 3, 4, -4 * GRID_SIZE, -GRID_SIZE + 4, -4 * GRID_SIZE + 1,
            -2 * GRID_SIZE + 4, -4 * GRID_SIZE + 2, -3 * GRID_SIZE + 4, -4 * GRID_SIZE + 3, -4 * GRID_SIZE + 4, 5, -5 * GRID_SIZE, -GRID_SIZE + 5,
            -5 * GRID_SIZE + 1, -2 * GRID_SIZE + 5, -5 * GRID_SIZE + 2, -3 * GRID_SIZE + 5, -5 * GRID_SIZE + 3, -4 * GRID_SIZE + 5, -5 * GRID_SIZE + 4,
            -5 * GRID_SIZE + 5
        },
    },
    .size2_pixels = {
        30, -15, 0, 0, 60, 0, 30, 15
    },
    .size3_pixels = {
        60, -30, 30, -15, 90, -15, 0, 0, 60, 0, 120, 0, 30, 15, 90, 15, 60, 30
    },
    .size4_pixels = {
        90, -45, 60, -30, 120, -30, 30, -15, 90, -15, 150, -15, 0, 0, 60, 0, 120, 0, 180, 0, 30, 15, 90, 15, 150, 15, 60, 30, 120, 30, 90, 45
    },
    .size5_pixels = {
        120, -60, 90, -45, 150, -45, 60, -30, 120, -30, 180, -30, 30, -15, 90, -15, 150, -15, 210, -15, 0, 0, 60, 0, 120, 0, 180, 0, 240, 0, 30, 15, 90, 15,
        150, 15, 210, 15, 60, 30, 120, 30, 180, 30, 90, 45, 150, 45, 120, 60
    },
    .size6_pixels = {
        150, -75, 120, -60, 180, -60, 90, -45, 150, -45, 210, -45, 60, -30, 120, -30, 180, -30, 240, -30, 30, -15, 90, -15, 150, -15, 210, -15, 270, -15, 0, 0,
        60, 0, 120, 0, 180, 0, 240, 0, 300, 0, 30, 15, 90, 15, 150, 15, 210, 15, 270, 15, 60, 30, 120, 30, 180, 30, 240, 30, 90, 45, 150, 45, 210, 45, 120, 60,
        180, 60, 150, 75
    },
};

int32_t house_image_variants[40] = { 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };

int32_t house_image_group[20] = { 295, 295, 296, 296, 297, 297, 298, 298, 299, 299, 300, 300, 301, 301, 302, 302, 303, 303, 304, 304 };

int32_t house_image_offset[20] = { 0, 2, 0, 2, 0, 2, 0, 2, 0, 2, 0, 2, 0, 2, 0, 1, 0, 1, 0, 1 };

int32_t building_properties[1185] = { 0, 0, 0, 0, -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, -1, 1, 0, 540, 0, -1, 1, 0, 0, 26, -1,
    2, 0, 547, 0, 0, 1, 0, 506, 2, -1, 0, 0, 0, 0, -1, 1, 0, 0, 0, -1, 1, 0, 0, 0, -1, 1, 0, 0, 0, -1, 1, 0, 0, 0, -1, 1, 0, 0, 0, -1, 1, 0, 0, 0, -1, 1, 0, 0,
    0, -1, 1, 0, 0, 0, -1, 1, 0, 0, 0, -1, 1, 0, 0, 0, -1, 2, 0, 0, 0, -1, 2, 0, 0, 0, -1, 2, 0, 0, 0, -1, 2, 0, 0, 0, -1, 3, 0, 0, 0, -1, 3, 0, 0, 0, -1, 3, 0,
    0, 0, -1, 3, 0, 0, 0, -1, 4, 0, 0, 0, -1, 4, 0, 0, 0, -1, 1, 0, 0, 0, 2, 1, 0, 0, 0, 2, 4, 0, 286, 0, 2, 2, 0, 0, 0, 2, 3, 0, 320, 0, 2, 4, 0, 321, 0, 2, 2,
    0, 315, 0, 2, 3, 0, 0, 0, -1, 1, 1, 437, 0, -1, 1, 1, 328, 0, -1, 3, 1, 335, 0, -1, 1, 1, 330, 0, -1, 2, 1, 277, 0, -1, 3, 1, 276, 0, -1, 3, 1, 335, 0, -1,
    3, 1, 335, 0, -1, 1, 0, 337, 0, 5, 2, 0, 444, 0, 5, 0, 0, 0, 0, -1, 1, 0, 336, 0, 5, 3, 0, 335, 0, -1, 2, 0, 311, 0, 4, 0, 0, 0, 0, -1, 3, 0, 312, 0, 4, 4,
    1, 335, 1, -1, 1, 0, 333, 0, 6, 3, 1, 0, 0, -1, 3, 1, 335, 0, 8, 2, 1, 0, 1, 8, 2, 1, 0, 0, 8, 3, 0, 294, 0, 3, 3, 0, 288, 0, 3, 3, 0, 289, 0, 3, 3, 0, 290,
    0, 3, 3, 0, 345, 0, 3, 3, 0, 294, 1, 3, 3, 0, 288, 1, 3, 3, 0, 289, 1, 3, 3, 0, 290, 1, 3, 3, 0, 345, 1, 3, 2, 0, 291, 0, 0, 4, 0, 368, 0, 0, 1, 0, 351, 0,
    1, 1, 0, 351, 0, -1, 3, 0, 523, 0, 1, 3, 0, 546, 0, 1, 2, 0, 515, 0, 0, 3, 0, 354, 0, -1, 4, 0, 355, 0, -1, 5, 0, 356, 0, -1, 2, 1, 453, 0, -1, 1, 0, 350,
    0, 6, 1, 1, 0, 0, -1, 1, 1, 0, 0, -1, 0, 0, 0, 0, 7, 5, 0, 434, 0, 7, 2, 0, 332, 0, 7, 0, 0, 0, 0, 7, 1, 1, 452, 0, -1, 2, 1, 452, 2, -1, 2, 1, 547, 0, 0,
    0, 0, 0, 0, -1, 1, 1, 292, 0, -1, 1, 1, 325, 0, -1, 4, 0, 442, 0, 8, 3, 0, 435, 0, 8, 0, 0, 0, 0, -1, 0, 0, 0, -1, 0, 2, 0, 0, 0, -1, 1, 1, 0, 0, -1, 3, 0,
    494, 0, 1, 3, 0, 494, 0, 1, 3, 0, 494, 0, 0, 3, 0, 494, 0, 0, 3, 0, 494, 0, 0, 3, 0, 494, 0, 0, 2, 0, 456, 0, 1, 2, 0, 439, 0, 1, 2, 0, 334, 0, 1, 2, 0,
    309, 0, 1, 2, 0, 385, 0, 1, 2, 0, 391, 0, 1, 2, 0, 392, 0, 8, 2, 0, 388, 0, 1, 2, 0, 394, 0, 1, 2, 0, 445, 0, 0, 1, 1, 0, 0, -1, 1, 1, 0, 0, -1, 1, 1, 0, 0,
    -1, 1, 1, 0, 0, -1, 1, 1, 0, 0, -1, 1, 1, 0, 0, -1, 1, 1, 0, 0, -1, 1, 1, 0, 0, -1, 1, 1, 0, 0, -1, 1, 1, 0, 0, -1, 1, 1, 0, 0, -1, 1, 1, 0, 0, -1, 1, 1, 0,
    0, -1, 2, 1, 0, 0, -1, 1, 1, 0, 0, -1, 1, 1, 0, 0, -1, 1, 1, 0, 0, -1, 1, 1, 0, 0, -1, 1, 1, 0, 0, -1, 0, 0, 0, 0, -1, 2, 0, 520, 0, 6, 1, 0, 540, 0, -1, 1,
    0, 367, 0, -1, 1, 1, 0, 0, -1, 1, 0, 344, 0, 3, 1, 0, 341, 0, 3, 1, 0, 342, 0, 3, 1, 0, 343, 0, 3, 1, 0, 340, 0, 3, 1, 0, 344, 0, 3, 1, 0, 344, 0, 3, 1, 0,
    344, 0, 3, 1, 0, 344, 0, 3, 1, 0, 344, 0, 3, 0, 0, 0, 0, -1, 3, 0, 294, 1, 3, 3, 0, 294, 1, 3, 3, 0, 294, 1, 3, 3, 0, 294, 1, 3, 3, 0, 294, 1, 3, 3, 0, 294,
    1, 3, 3, 0, 294, 1, 3, 3, 0, 294, 1, 3, 3, 0, 294, 1, 3, 3, 0, 294, 1, 3, 2, 0, 454, 0, 1, 2, 0, 457, 0, 1, 1, 0, 505, 0, -1, 1, 0, 505, 0, -1, 1, 0, 505,
    0, -1, 1, 0, 505, 0, -1, 1, 0, 347, 0, 6, 1, 0, 0, 26, -1, 1, 0, 407, 26, -1, 2, 1, 0, 1, -1, 2, 1, 404, 1, -1, 2, 1, 404, 0, 8, 2, 1, 404, 0, 8, 0, 0, 0,
    0, -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, -1, 2, 0, 360, 0, 1, 2, 0, 326, 0, 1, 2, 0, 357, 0, 1, 2, 0, 338, 0, 5, 2, 0, 514, 0, 8, 3, 0, 525, 0, 8, 2, 1, 338, 0,
    -1, 3, 0, 331, 0, 7, 4, 0, 442, 0, 8, 4, 0, 442, 0, 8, 4, 0, 316, 0, 7, 5, 0, 308, 0, 7, 6, 0, 287, 0, 7, 2, 0, 314, 0, 0, 4, 0, 368, 0, 0, 3, 0, 524, 0, 0,
    1, 0, 351, 0, 0, 3, 0, 346, 0, 0, 2, 0, 346, 0, 1, 3, 0, 346, 0, 0, 1, 1, 0, 0, -1, 1, 1, 0, 0, -1, 2, 0, 346, 0, 0, 2, 1, 0, 0, 8, 2, 1, 0, 0, 8, 2, 1,
    430, 0, 8, 2, 0, 313, 0, 1, 2, 0, 393, 0, 1, 4, 0, 389, 0, 8, 2, 0, 339, 0, 5, 3, 1, 564, 0, 7, 5, 1, 564, 0, 7, 5, 0, 0, 0, -1, 6, 0, 564, 0, 7, 0, 0, 0,
    0, -1, 0, 0, 0, 0, -1, 1, 0, 564, 0, 7, 1, 1, 0, 0, -1, 1, 1, 497, 0, 7, 2, 0, 307, 0, 1, 2, 0, 465, 0, 1, 2, 1, 760, 0, 7, 2, 1, 761, 0, 7, 2, 1, 762, 0,
    7, 2, 0, 466, 0, 1, 1, 1, 564, 0, 7, 1, 0, 0, 0, -1, 3, 0, 494, 0, 1, 1, 0, 766, 0, 7, 6, 0, 716, 0, 2, 1, 0, 764, 0, 7, 1, 0, 765, 0, 7, 1, 0, 795, 0, 7,
    1, 0, 766, 0, 7, 2, 0, 741, 0, 1, 2, 0, 736, 0, 1, 2, 0, 737, 0, 1 };

int32_t venue_direction_offsets[4] = { -GRID_SIZE, 1, GRID_SIZE, -1 };

uint8_t venue_patterns2[4] = { 1, 1, 1, 1 };

uint8_t venue_patterns3[12] = { 0, 1, 0, 1, 0, 1, 0, 1 };

uint8_t venue_patterns4[16] = { 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1 };

uint8_t venue_patterns5[28] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 };

int32_t venue_spot_rank3[9] = { 0, 1, 0, 1, 2, 1, 0, 1 };

int32_t venue_spot_rank4[35] = { 0, 1, 1, 0, 1, 2, 2, 1, 1, 2, 2, 1, 0, 1, 1, 0, 0, 1, 0, 1, 2, 1, 1, 2, 1, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0 };

char fmt_continue_question[40] = "%s\r\n\r\nWould you still like to continue?";

int32_t house_expand_delta[8] = { 0, 0, -1, -1, -1, 0, 0, -1 };

int32_t house_expand_direction[4] = { 0, -GRID_SIZE - 1, -1, -GRID_SIZE };

int32_t house_tile_offsets[16] = { 0, 1, GRID_SIZE, GRID_SIZE + 1, 2, GRID_SIZE + 2, 2 * GRID_SIZE + 2, 2 * GRID_SIZE + 1, 2 * GRID_SIZE, 3, GRID_SIZE + 3,
    2 * GRID_SIZE + 3, 3 * GRID_SIZE + 3, 3 * GRID_SIZE + 2, 3 * GRID_SIZE + 1, 3 * GRID_SIZE };

global_function house_evolve_callbacks[20] = { (global_function) house_evolve_crude_hut, (global_function) house_evolve_sturdy_hut,
    (global_function) house_evolve_meager_shanty, (global_function) house_evolve_common_shanty, (global_function) house_evolve_rough_cottage,
    (global_function) house_evolve_ordinary_cottage, (global_function) house_evolve_modest_homestead, (global_function) house_evolve_spacious_homestead,
    (global_function) house_evolve_modest_apartment, (global_function) house_evolve_spacious_apartment, (global_function) house_evolve_common_residence,
    (global_function) house_evolve_spacious_residence, (global_function) house_evolve_elegant_residence, (global_function) house_evolve_fancy_residence,
    (global_function) house_evolve_common_manor, (global_function) house_evolve_spacious_manor, (global_function) house_evolve_elegant_manor,
    (global_function) house_evolve_stately_manor, (global_function) house_evolve_modest_estate, (global_function) house_evolve_palatial_estate };

int32_t festival_group_rates[4] = { 15, 20, 30, 35 };

int32_t festival_type_lists[48] = { 51, 53, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 24, 25, 26, 27, 28, 29, 0, 0, 0, 0, 0, 0, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 0,
    0, 31, 30, 33, 32, 36, 34, 33 };

const char *building_type_debug_names[237] = { str_build_nothing, str_build_undo, str_build_farms, str_build_extractors, str_build_construction, str_build_road,
    str_build_mud_wall, str_build_aqua_system, str_build_irrigation, str_build_dig, str_build_house1, str_build_house2, str_build_house3, str_build_house4,
    str_build_house5, str_build_house6, str_build_house7, str_build_house8, str_build_house9, str_build_house10, str_build_house11, str_build_house12,
    str_build_house13, str_build_house14, str_build_house15, str_build_house16, str_build_house17, str_build_house18, str_build_house19, str_build_house20,
    str_build_musician, str_build_juggler, str_build_senet_house, str_build_dancer, str_build_musician_train, str_build_dancer_train, str_build_juggling_train,
    str_build_bull_fighting_train, str_build_plaza, str_build_small_park, str_build_leg_fort, str_build_statue1, str_build_statue2, str_build_statue3,
    str_build_aux_mis_fort, str_build_aux_lgt_fort, str_build_apothecary, str_build_mortuary, str_build_monuments, str_build_dentist, str_build_dist_centre,
    str_build_school, str_build_water_crossing, str_build_library, str_build_training_ground, str_build_prefecture, str_build_arch, str_build_fort,
    str_build_mud_gatehouse, str_build_mud_tower, str_build_temp_osiris_small, str_build_temp_ra_small, str_build_temp_ptah_small, str_build_temp_seth_small,
    str_build_temp_bast_small, str_build_temp_osiris_large, str_build_temp_ra_large, str_build_temp_ptah_large, str_build_temp_seth_large,
    str_build_temp_bast_large, str_build_market, str_build_granary, str_build_warehouse, str_build_warehouse_space, str_build_shipyard, str_build_dock,
    str_build_fishing_wharf, str_build_gov_mansion1, str_build_gov_mansion2, str_build_gov_mansion3, str_build_mission, str_build_eng_post,
    str_build_foot_bridge, str_build_road_bridge, str_build_senate1, str_build_senate2, str_build_forum1, str_build_forum2, str_build_native_hut,
    str_build_native_cheif, str_build_waterlift, str_build_beautification, str_build_well, str_build_native_feild, str_build_military_academy,
    str_build_barracks, str_build_small_temple, str_build_large_temple, str_build_oracle, str_build_ruin, str_build_barley_farm, str_build_flax_farm,
    str_build_wheat_farm, str_build_lettuce_farm, str_build_pom_farm, str_build_chicpeas_farm, str_build_quarry, str_build_special_quarry,
    str_build_lumber_mill, str_build_clay_pit, str_build_brewery, str_build_linen_workshop, str_build_weapons_workshop, str_build_lux_goods_workshop,
    str_build_pottery_workshop, str_build_hunting_lodge, str_build_map_grass, str_build_map_trees, str_build_map_water, str_build_map_earthquake,
    str_build_map_scrub, str_build_map_rocks, str_build_map_meadow, str_build_map_plateau, str_build_map_anti_plateau, str_build_map_road,
    str_build_map_invasion_point, str_build_map_entry_point, str_build_map_exit_point, str_build_map_ramp, str_build_map_river_in, str_build_map_river_out,
    str_build_map_fishing_point, str_build_map_killer_point, str_build_map_floodplain, str_build_map_irrigation, str_build_ferry, str_build_road_system,
    str_build_no_access, str_build_map_prey_point, str_build_shrine_god0, str_build_shrine_god1, str_build_shrine_god2, str_build_shrine_god3,
    str_build_shrine_god4, str_build_shrine_god5, str_build_shrine_god6, str_build_shrine_god7, str_build_shrine_god8, str_build_shrine_god9, str_build_shrine,
    str_build_temp_small5, str_build_temp_small6, str_build_temp_small7, str_build_temp_small8, str_build_temp_small9, str_build_temp_large5,
    str_build_temp_large6, str_build_temp_large7, str_build_temp_large8, str_build_temp_large9, str_build_gold_mine, str_build_gem_mine,
    str_build_map_ordinary_rock, str_build_map_gold_rock, str_build_map_gem_rock, str_build_map_finishing_rock, str_build_firehouse, str_build_brick_wall,
    str_build_stone_wall, str_build_brick_gatehouse, str_build_stone_gatehouse, str_build_brick_tower, str_build_stone_tower, str_build_mud_structures,
    str_build_brick_structures, str_build_stone_structures, str_build_carpenter_lodge, str_build_bricklayer_lodge, str_build_stonemason_lodge,
    str_build_large_well, str_build_transport_wharf, str_build_warship_wharf, str_build_pyramid, str_build_courthouse, str_build_military_academy2,
    str_build_military_academy3, str_build_palace1, str_build_palace2, str_build_palace3, str_build_market2, str_build_granary2, str_build_dock2,
    str_build_warehouse2, str_build_meat_farm, str_build_reed_farm, str_build_figs_farm, str_build_map_marshland, str_build_map_sand_dune,
    str_build_peasant_house, str_build_large_mud_gatehouse, str_build_large_brick_gatehouse, str_build_large_stone_gatehouse, str_build_papyrus_workshop,
    str_build_brick_maker, str_build_chariot_builder, str_build_physician, str_build_obelisk3x3, str_build_obelisk5x5, str_build_festival, str_build_sphinx,
    str_build_temp_upgrade1, str_build_temp_upgrade2, str_build_sun_temple, str_build_map_disembark_point, str_build_thingamabob, str_build_granite_quarry,
    str_build_copper_mine, str_build_mortuary_temple, str_build_causeway, str_build_valley_temple, str_build_sandstone_quarry, str_build_mausoleum,
    str_build_map_cliff, str_build_henna_farm, str_build_alex_library, str_build_zoo, str_build_caesareum, str_build_lighthouse, str_build_small_tomb,
    str_build_abu_simbel, str_build_artisans_guild, str_build_lamp_maker, str_build_paint_maker, str_build_med_tomb, str_build_large_tomb, str_build_grand_tomb };

const char str_build_grand_tomb[20] = "BUILD_GRAND_TOMB";
const char str_build_large_tomb[20] = "BUILD_LARGE_TOMB";
const char str_build_med_tomb[16] = "BUILD_MED_TOMB";
const char str_build_paint_maker[20] = "BUILD_PAINT_MAKER";
const char str_build_lamp_maker[20] = "BUILD_LAMP_MAKER";
const char str_build_artisans_guild[24] = "BUILD_ARTISANS_GUILD";
const char str_build_abu_simbel[20] = "BUILD_ABU_SIMBEL";
const char str_build_small_tomb[20] = "BUILD_SMALL_TOMB";
const char str_build_lighthouse[20] = "BUILD_LIGHTHOUSE";
const char str_build_caesareum[16] = "BUILD_CAESAREUM";
const char str_build_zoo[12] = "BUILD_ZOO";
const char str_build_alex_library[20] = "BUILD_ALEX_LIBRARY";
const char str_build_henna_farm[20] = "BUILD_HENNA_FARM";
const char str_build_map_cliff[16] = "BUILD_MAP_CLIFF";
const char str_build_mausoleum[16] = "BUILD_MAUSOLEUM";
const char str_build_sandstone_quarry[24] = "BUILD_SANDSTONE_QUARRY";
const char str_build_valley_temple[20] = "BUILD_VALLEY_TEMPLE";
const char str_build_causeway[16] = "BUILD_CAUSEWAY";
const char str_build_mortuary_temple[24] = "BUILD_MORTUARY_TEMPLE";
const char str_build_copper_mine[20] = "BUILD_COPPER_MINE";
const char str_build_granite_quarry[24] = "BUILD_GRANITE_QUARRY";
const char str_build_thingamabob[20] = "BUILD_THINGAMABOB";
const char str_build_map_disembark_point[28] = "BUILD_MAP_DISEMBARK_POINT";
const char str_build_sun_temple[20] = "BUILD_SUN_TEMPLE";
const char str_build_temp_upgrade2[20] = "BUILD_TEMP_UPGRADE2";
const char str_build_temp_upgrade1[20] = "BUILD_TEMP_UPGRADE1";
const char str_build_sphinx[16] = "BUILD_SPHINX";
const char str_build_festival[16] = "BUILD_FESTIVAL";
const char str_build_obelisk5x5[20] = "BUILD_OBELISK5x5";
const char str_build_obelisk3x3[20] = "BUILD_OBELISK3x3";
const char str_build_physician[16] = "BUILD_PHYSICIAN";
const char str_build_chariot_builder[24] = "BUILD_CHARIOT_BUILDER";
const char str_build_brick_maker[20] = "BUILD_BRICK_MAKER";
const char str_build_papyrus_workshop[24] = "BUILD_PAPYRUS_WORKSHOP";
const char str_build_large_stone_gatehouse[28] = "BUILD_LARGE_STONE_GATEHOUSE";
const char str_build_large_brick_gatehouse[28] = "BUILD_LARGE_BRICK_GATEHOUSE";
const char str_build_large_mud_gatehouse[28] = "BUILD_LARGE_MUD_GATEHOUSE";
const char str_build_peasant_house[20] = "BUILD_PEASANT_HOUSE";
const char str_build_map_sand_dune[20] = "BUILD_MAP_SAND_DUNE";
const char str_build_map_marshland[20] = "BUILD_MAP_MARSHLAND";
const char str_build_figs_farm[16] = "BUILD_FIGS_FARM";
const char str_build_reed_farm[16] = "BUILD_REED_FARM";
const char str_build_meat_farm[16] = "BUILD_MEAT_FARM";
const char str_build_warehouse2[20] = "BUILD_WAREHOUSE2";
const char str_build_dock2[12] = "BUILD_DOCK2";
const char str_build_granary2[16] = "BUILD_GRANARY2";
const char str_build_market2[16] = "BUILD_MARKET2";
const char str_build_palace3[16] = "BUILD_PALACE3";
const char str_build_palace2[16] = "BUILD_PALACE2";
const char str_build_palace1[16] = "BUILD_PALACE1";
const char str_build_military_academy3[24] = "BUILD_MILITARY_ACADEMY3";
const char str_build_military_academy2[24] = "BUILD_MILITARY_ACADEMY2";
const char str_build_courthouse[20] = "BUILD_COURTHOUSE";
const char str_build_pyramid[16] = "BUILD_PYRAMID";
const char str_build_warship_wharf[20] = "BUILD_WARSHIP_WHARF";
const char str_build_transport_wharf[24] = "BUILD_TRANSPORT_WHARF";
const char str_build_large_well[20] = "BUILD_LARGE_WELL";
const char str_build_stonemason_lodge[24] = "BUILD_STONEMASON_LODGE";
const char str_build_bricklayer_lodge[24] = "BUILD_BRICKLAYER_LODGE";
const char str_build_carpenter_lodge[24] = "BUILD_CARPENTER_LODGE";
const char str_build_stone_structures[24] = "BUILD_STONE_STRUCTURES";
const char str_build_brick_structures[24] = "BUILD_BRICK_STRUCTURES";
const char str_build_mud_structures[24] = "BUILD_MUD_STRUCTURES";
const char str_build_stone_tower[20] = "BUILD_STONE_TOWER";
const char str_build_brick_tower[20] = "BUILD_BRICK_TOWER";
const char str_build_stone_gatehouse[24] = "BUILD_STONE_GATEHOUSE";
const char str_build_brick_gatehouse[24] = "BUILD_BRICK_GATEHOUSE";
const char str_build_stone_wall[20] = "BUILD_STONE_WALL";
const char str_build_brick_wall[20] = "BUILD_BRICK_WALL";
const char str_build_firehouse[16] = "BUILD_FIREHOUSE";
const char str_build_map_finishing_rock[28] = "BUILD_MAP_FINISHING_ROCK";
const char str_build_map_gem_rock[20] = "BUILD_MAP_GEM_ROCK";
const char str_build_map_gold_rock[20] = "BUILD_MAP_GOLD_ROCK";
const char str_build_map_ordinary_rock[24] = "BUILD_MAP_ORDINARY_ROCK";
const char str_build_gem_mine[16] = "BUILD_GEM_MINE";
const char str_build_gold_mine[16] = "BUILD_GOLD_MINE";
const char str_build_temp_large9[20] = "BUILD_TEMP_LARGE9";
const char str_build_temp_large8[20] = "BUILD_TEMP_LARGE8";
const char str_build_temp_large7[20] = "BUILD_TEMP_LARGE7";
const char str_build_temp_large6[20] = "BUILD_TEMP_LARGE6";
const char str_build_temp_large5[20] = "BUILD_TEMP_LARGE5";
const char str_build_temp_small9[20] = "BUILD_TEMP_SMALL9";
const char str_build_temp_small8[20] = "BUILD_TEMP_SMALL8";
const char str_build_temp_small7[20] = "BUILD_TEMP_SMALL7";
const char str_build_temp_small6[20] = "BUILD_TEMP_SMALL6";
const char str_build_temp_small5[20] = "BUILD_TEMP_SMALL5";
const char str_build_shrine[16] = "BUILD_SHRINE";
const char str_build_shrine_god9[20] = "BUILD_SHRINE_GOD9";
const char str_build_shrine_god8[20] = "BUILD_SHRINE_GOD8";
const char str_build_shrine_god7[20] = "BUILD_SHRINE_GOD7";
const char str_build_shrine_god6[20] = "BUILD_SHRINE_GOD6";
const char str_build_shrine_god5[20] = "BUILD_SHRINE_GOD5";
const char str_build_shrine_god4[20] = "BUILD_SHRINE_GOD4";
const char str_build_shrine_god3[20] = "BUILD_SHRINE_GOD3";
const char str_build_shrine_god2[20] = "BUILD_SHRINE_GOD2";
const char str_build_shrine_god1[20] = "BUILD_SHRINE_GOD1";
const char str_build_shrine_god0[20] = "BUILD_SHRINE_GOD0";
const char str_build_map_prey_point[24] = "BUILD_MAP_PREY_POINT";
const char str_build_no_access[16] = "BUILD_NO_ACCESS";
const char str_build_road_system[20] = "BUILD_ROAD_SYSTEM";
const char str_build_ferry[12] = "BUILD_FERRY";
const char str_build_map_irrigation[24] = "BUILD_MAP_IRRIGATION";
const char str_build_map_floodplain[24] = "BUILD_MAP_FLOODPLAIN";
const char str_build_map_killer_point[24] = "BUILD_MAP_KILLER_POINT";
const char str_build_map_fishing_point[24] = "BUILD_MAP_FISHING_POINT";
const char str_build_map_river_out[20] = "BUILD_MAP_RIVER_OUT";
const char str_build_map_river_in[20] = "BUILD_MAP_RIVER_IN";
const char str_build_map_ramp[16] = "BUILD_MAP_RAMP";
const char str_build_map_exit_point[24] = "BUILD_MAP_EXIT_POINT";
const char str_build_map_entry_point[24] = "BUILD_MAP_ENTRY_POINT";
const char str_build_map_invasion_point[28] = "BUILD_MAP_INVASION_POINT";
const char str_build_map_road[16] = "BUILD_MAP_ROAD";
const char str_build_map_anti_plateau[24] = "BUILD_MAP_ANTI_PLATEAU";
const char str_build_map_plateau[20] = "BUILD_MAP_PLATEAU";
const char str_build_map_meadow[20] = "BUILD_MAP_MEADOW";
const char str_build_map_rocks[16] = "BUILD_MAP_ROCKS";
const char str_build_map_scrub[16] = "BUILD_MAP_SCRUB";
const char str_build_map_earthquake[24] = "BUILD_MAP_EARTHQUAKE";
const char str_build_map_water[16] = "BUILD_MAP_WATER";
const char str_build_map_trees[16] = "BUILD_MAP_TREES";
const char str_build_map_grass[16] = "BUILD_MAP_GRASS";
const char str_build_hunting_lodge[20] = "BUILD_HUNTING_LODGE";
const char str_build_pottery_workshop[24] = "BUILD_POTTERY_WORKSHOP";
const char str_build_lux_goods_workshop[28] = "BUILD_LUX_GOODS_WORKSHOP";
const char str_build_weapons_workshop[24] = "BUILD_WEAPONS_WORKSHOP";
const char str_build_linen_workshop[24] = "BUILD_LINEN_WORKSHOP";
const char str_build_brewery[16] = "BUILD_BREWERY";
const char str_build_clay_pit[16] = "BUILD_CLAY_PIT";
const char str_build_lumber_mill[20] = "BUILD_LUMBER_MILL";
const char str_build_special_quarry[24] = "BUILD_SPECIAL_QUARRY";
const char str_build_quarry[16] = "BUILD_QUARRY";
const char str_build_chicpeas_farm[20] = "BUILD_CHICPEAS_FARM";
const char str_build_pom_farm[16] = "BUILD_POM_FARM";
const char str_build_lettuce_farm[20] = "BUILD_LETTUCE_FARM";
const char str_build_wheat_farm[20] = "BUILD_WHEAT_FARM";
const char str_build_flax_farm[16] = "BUILD_FLAX_FARM";
const char str_build_barley_farm[20] = "BUILD_BARLEY_FARM";
const char str_build_ruin[12] = "BUILD_RUIN";
const char str_build_oracle[16] = "BUILD_ORACLE";
const char str_build_large_temple[20] = "BUILD_LARGE_TEMPLE";
const char str_build_small_temple[20] = "BUILD_SMALL_TEMPLE";
const char str_build_barracks[16] = "BUILD_BARRACKS";
const char str_build_military_academy[24] = "BUILD_MILITARY_ACADEMY";
const char str_build_native_feild[20] = "BUILD_NATIVE_FEILD";
const char str_build_well[12] = "BUILD_WELL";
const char str_build_beautification[24] = "BUILD_BEAUTIFICATION";
const char str_build_waterlift[16] = "BUILD_WATERLIFT";
const char str_build_native_cheif[20] = "BUILD_NATIVE_CHEIF";
const char str_build_native_hut[20] = "BUILD_NATIVE_HUT";
const char str_build_forum2[16] = "BUILD_FORUM2";
const char str_build_forum1[16] = "BUILD_FORUM1";
const char str_build_senate2[16] = "BUILD_SENATE2";
const char str_build_senate1[16] = "BUILD_SENATE1";
const char str_build_road_bridge[20] = "BUILD_ROAD_BRIDGE";
const char str_build_foot_bridge[20] = "BUILD_FOOT_BRIDGE";
const char str_build_eng_post[16] = "BUILD_ENG_POST";
const char str_build_mission[16] = "BUILD_MISSION";
const char str_build_gov_mansion3[20] = "BUILD_GOV_MANSION3";
const char str_build_gov_mansion2[20] = "BUILD_GOV_MANSION2";
const char str_build_gov_mansion1[20] = "BUILD_GOV_MANSION1";
const char str_build_fishing_wharf[20] = "BUILD_FISHING_WHARF";
const char str_build_dock[12] = "BUILD_DOCK";
const char str_build_shipyard[16] = "BUILD_SHIPYARD";
const char str_build_warehouse_space[24] = "BUILD_WAREHOUSE_SPACE";
const char str_build_warehouse[16] = "BUILD_WAREHOUSE";
const char str_build_granary[16] = "BUILD_GRANARY";
const char str_build_market[16] = "BUILD_MARKET";
const char str_build_temp_bast_large[24] = "BUILD_TEMP_BAST_LARGE";
const char str_build_temp_seth_large[24] = "BUILD_TEMP_SETH_LARGE";
const char str_build_temp_ptah_large[24] = "BUILD_TEMP_PTAH_LARGE";
const char str_build_temp_ra_large[20] = "BUILD_TEMP_RA_LARGE";
const char str_build_temp_osiris_large[24] = "BUILD_TEMP_OSIRIS_LARGE";
const char str_build_temp_bast_small[24] = "BUILD_TEMP_BAST_SMALL";
const char str_build_temp_seth_small[24] = "BUILD_TEMP_SETH_SMALL";
const char str_build_temp_ptah_small[24] = "BUILD_TEMP_PTAH_SMALL";
const char str_build_temp_ra_small[20] = "BUILD_TEMP_RA_SMALL";
const char str_build_temp_osiris_small[24] = "BUILD_TEMP_OSIRIS_SMALL";
const char str_build_mud_tower[16] = "BUILD_MUD_TOWER";
const char str_build_mud_gatehouse[20] = "BUILD_MUD_GATEHOUSE";
const char str_build_fort[12] = "BUILD_FORT";
const char str_build_arch[12] = "BUILD_ARCH";
const char str_build_prefecture[20] = "BUILD_PREFECTURE";
const char str_build_training_ground[24] = "BUILD_TRAINING_GROUND";
const char str_build_library[16] = "BUILD_LIBRARY";
const char str_build_water_crossing[24] = "BUILD_WATER_CROSSING";
const char str_build_school[16] = "BUILD_SCHOOL";
const char str_build_dist_centre[20] = "BUILD_DIST_CENTRE";
const char str_build_dentist[16] = "BUILD_DENTIST";
const char str_build_monuments[16] = "BUILD_MONUMENTS";
const char str_build_mortuary[16] = "BUILD_MORTUARY";
const char str_build_apothecary[20] = "BUILD_APOTHECARY";
const char str_build_aux_lgt_fort[20] = "BUILD_AUX_LGT_FORT";
const char str_build_aux_mis_fort[20] = "BUILD_AUX_MIS_FORT";
const char str_build_statue3[16] = "BUILD_STATUE3";
const char str_build_statue2[16] = "BUILD_STATUE2";
const char str_build_statue1[16] = "BUILD_STATUE1";
const char str_build_leg_fort[16] = "BUILD_LEG_FORT";
const char str_build_small_park[20] = "BUILD_SMALL_PARK";
const char str_build_plaza[12] = "BUILD_PLAZA";
const char str_build_bull_fighting_train[28] = "BUILD_BULL_FIGHTING_TRAIN";
const char str_build_juggling_train[24] = "BUILD_JUGGLING_TRAIN";
const char str_build_dancer_train[20] = "BUILD_DANCER_TRAIN";
const char str_build_musician_train[24] = "BUILD_MUSICIAN_TRAIN";
const char str_build_dancer[16] = "BUILD_DANCER";
const char str_build_senet_house[20] = "BUILD_SENET_HOUSE";
const char str_build_juggler[16] = "BUILD_JUGGLER";
const char str_build_musician[16] = "BUILD_MUSICIAN";
const char str_build_house20[16] = "BUILD_HOUSE20";
const char str_build_house19[16] = "BUILD_HOUSE19";
const char str_build_house18[16] = "BUILD_HOUSE18";
const char str_build_house17[16] = "BUILD_HOUSE17";
const char str_build_house16[16] = "BUILD_HOUSE16";
const char str_build_house15[16] = "BUILD_HOUSE15";
const char str_build_house14[16] = "BUILD_HOUSE14";
const char str_build_house13[16] = "BUILD_HOUSE13";
const char str_build_house12[16] = "BUILD_HOUSE12";
const char str_build_house11[16] = "BUILD_HOUSE11";
const char str_build_house10[16] = "BUILD_HOUSE10";
const char str_build_house9[16] = "BUILD_HOUSE9";
const char str_build_house8[16] = "BUILD_HOUSE8";
const char str_build_house7[16] = "BUILD_HOUSE7";
const char str_build_house6[16] = "BUILD_HOUSE6";
const char str_build_house5[16] = "BUILD_HOUSE5";
const char str_build_house4[16] = "BUILD_HOUSE4";
const char str_build_house3[16] = "BUILD_HOUSE3";
const char str_build_house2[16] = "BUILD_HOUSE2";
const char str_build_house1[16] = "BUILD_HOUSE1";
const char str_build_dig[12] = "BUILD_DIG";
const char str_build_irrigation[20] = "BUILD_IRRIGATION";
const char str_build_aqua_system[20] = "BUILD_AQUA_SYSTEM";
const char str_build_mud_wall[16] = "BUILD_MUD_WALL";
const char str_build_road[12] = "BUILD_ROAD";
const char str_build_construction[20] = "BUILD_CONSTRUCTION";
const char str_build_extractors[20] = "BUILD_EXTRACTORS";
const char str_build_farms[12] = "BUILD_FARMS";
const char str_build_undo[12] = "BUILD_UNDO";
const char str_build_nothing[16] = "BUILD_NOTHING";

int32_t canal_lift_ends[45] = { 0, -2, 2, 0, 0, 2, -2, 0, -1, 1, 1, -1, 1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, 1, 1, -1, -1, 1, 1, 1, -1, -1, -1, 1, 1, -1, -1,
    -1, 1, 1, 250, 375, 500, 625, 750 };

int32_t complex_part_groups[9] = { 430, 488, 487, 430, 486, 482, 485, 483, 484 };

int32_t gatehouse_orientation_parts[33] = { 0, 1, 2, 3, 0, 1, 2, 3, 0, 2, 1, 3, 3, 1, 2, 0, 3, 2, 1, 0, 3, 2, 1, 0, 3, 1, 2, 0, 0, 2, 1, 3 };

gatehouse_part_tables gatehouse_parts = {
    .parts = {
        { 0, 2, 0, 0 }, { 1, 1, 2, 0 }, { 1, 1, 2, 1 }, { 0, 2, 3, 0 }
    },
    .oracle_tiles = {
        { 1, 1 }, { 1, -2 }, { -2, -2 }, { -2, 1 }
    },
    .altar_tiles = {
        { 3, 3 }, { 3, -3 }, { -3, -3 }, { -3, 3 }
    },
    .oracle_pixels = {
        3, 2, -3, -2
    },
    .altar_pixels = {
        6, 5, -6, -5
    },
    .part_groups = {
        569, 570, 571, 571, 570, 569, 571, 570, 569, 569, 570, 571, 571, 570, 569, 571, 570, 569, 569, 570, 571, 569, 570, 571, 571, 570, 569, 569, 570, 571,
        569, 570, 571, 571, 570, 569, 569, 570, 571, 569, 570, 571, 571, 570, 569, 571, 570, 569
    },
    .part_steps = {
        { 3, 0 }, { 3, 0 }, { 0, -3 }, { 3, 0 }, { -3, 0 }, { 3, 0 }, { 0, 3 }, { 3, 0 }
    },
};

int32_t temple_complex_tiles[455] = { 573, 0, 3, 2, 1, 573, 0, 3, 2, 1, 572, 1, 1, 1, 1, 573, 0, 3, 2, 1, 573, 0, 3, 2, 1, 572, 1, 1, 1, 1, 573, 0, 3, 2, 1,
    573, 0, 3, 2, 1, 572, 0, 0, 0, 0, 572, 2, 2, 2, 2, 572, 3, 3, 3, 3, 572, 2, 2, 2, 2, 572, 3, 3, 3, 3, 572, 0, 0, 0, 0, 572, 0, 0, 0, 0, 572, 1, 1, 1, 1,
    572, 0, 0, 0, 0, 572, 0, 0, 0, 0, 572, 1, 1, 1, 1, 572, 0, 0, 0, 0, 572, 0, 0, 0, 0, 572, 0, 0, 0, 0, 572, 0, 0, 0, 0, 574, 5, 2, 0, 7, 574, 5, 2, 0, 7,
    574, 5, 2, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 572,
    0, 0, 0, 0, 574, 4, 3, 1, 6, 574, 4, 3, 1, 6, 574, 4, 3, 1, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 572, 1, 1, 1, 1, 572, 1, 1, 1, 1, 572, 1, 1, 1, 1, 572, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 572, 0, 0, 0, 0, 574, 1, 6, 4, 3, 574, 1, 6, 4, 3, 574, 1, 6, 4,
    3, 572, 0, 0, 0, 0, 572, 0, 0, 0, 0, 572, 1, 1, 1, 1, 572, 0, 0, 0, 0, 572, 0, 0, 0, 0, 572, 1, 1, 1, 1, 572, 0, 0, 0, 0, 572, 0, 0, 0, 0, 572, 0, 0, 0, 0,
    572, 0, 0, 0, 0, 574, 0, 7, 5, 2, 574, 0, 7, 5, 2, 574, 0, 7, 5, 2, 573, 2, 1, 0, 3, 573, 2, 1, 0, 3, 572, 1, 1, 1, 1, 573, 2, 1, 0, 3, 573, 2, 1, 0, 3,
    572, 1, 1, 1, 1, 573, 2, 1, 0, 3, 573, 2, 1, 0, 3, 572, 0, 0, 0, 0, 572, 2, 2, 2, 2, 572, 3, 3, 3, 3, 572, 2, 2, 2, 2, 572, 3, 3, 3, 3 };

int32_t temple_complex_origin[48] = { 0, -2, 4, 2, 2, 4, -2, 0, -2, 0, 0, -2, 4, 2, 2, 4, 2, 4, -2, 0, 0, -2, 4, 2, 4, 2, 2, 4, -2, 0, 0, -2, 0, 6, 4, 2, 2, 0,
    6, 4, 4, 2, 0, 6, 6, 4, 2 };

const char temple_nile_script_name[12] = "TEMPLE_NILE";

const char *temple_complex_god_script_names[5] = { temple_nile_script_name, str_temple_ra, str_temple_ptah, str_temple_seth,
    str_temple_bast };

const char str_temple_bast[12] = "TEMPLE_BAST";
const char str_temple_seth[12] = "TEMPLE_SETH";
const char str_temple_ptah[12] = "TEMPLE_PTAH";
const char str_temple_ra[12] = "TEMPLE_RA";

const char *mausoleum_block_scripts[36] = { str_maus0_1a, str_maus0_1b, str_maus0_1c, str_maus0_1d, str_maus0_2a, str_maus0_2b, str_maus0_2c, str_maus0_2d,
    str_maus0_3a, str_maus0_3b, str_maus0_3c, str_maus0_3d, str_maus1_1a, str_maus1_1b, str_maus1_1c, str_maus1_1d, str_maus1_2a, str_maus1_2b, str_maus1_2c,
    str_maus1_2d, str_maus1_3a, str_maus1_3b, str_maus1_3c, str_maus1_3d, str_maus2_1a, str_maus2_1b, str_maus2_1c, str_maus2_1d, str_maus2_2a, str_maus2_2b,
    str_maus2_2c, str_maus2_2d, str_maus2_3a, str_maus2_3b, str_maus2_3c, str_maus2_3d };

const char str_maus2_3d[12] = "maus2_3d";
const char str_maus2_3c[12] = "maus2_3c";
const char str_maus2_3b[12] = "maus2_3b";
const char str_maus2_3a[12] = "maus2_3a";
const char str_maus2_2d[12] = "maus2_2d";
const char str_maus2_2c[12] = "maus2_2c";
const char str_maus2_2b[12] = "maus2_2b";
const char str_maus2_2a[12] = "maus2_2a";
const char str_maus2_1d[12] = "maus2_1d";
const char str_maus2_1c[12] = "maus2_1c";
const char str_maus2_1b[12] = "maus2_1b";
const char str_maus2_1a[12] = "maus2_1a";
const char str_maus1_3d[12] = "maus1_3d";
const char str_maus1_3c[12] = "maus1_3c";
const char str_maus1_3b[12] = "maus1_3b";
const char str_maus1_3a[12] = "maus1_3a";
const char str_maus1_2d[12] = "maus1_2d";
const char str_maus1_2c[12] = "maus1_2c";
const char str_maus1_2b[12] = "maus1_2b";
const char str_maus1_2a[12] = "maus1_2a";
const char str_maus1_1d[12] = "maus1_1d";
const char str_maus1_1c[12] = "maus1_1c";
const char str_maus1_1b[12] = "maus1_1b";
const char str_maus1_1a[12] = "maus1_1a";
const char str_maus0_3d[12] = "maus0_3d";
const char str_maus0_3c[12] = "maus0_3c";
const char str_maus0_3b[12] = "maus0_3b";
const char str_maus0_3a[12] = "maus0_3a";
const char str_maus0_2d[12] = "maus0_2d";
const char str_maus0_2c[12] = "maus0_2c";
const char str_maus0_2b[12] = "maus0_2b";
const char str_maus0_2a[12] = "maus0_2a";
const char str_maus0_1d[12] = "maus0_1d";
const char str_maus0_1c[12] = "maus0_1c";
const char str_maus0_1b[12] = "maus0_1b";
const char str_maus0_1a[12] = "maus0_1a";

int32_t city_warning_positions[10] = { 300, 30, 300, 55, 300, 80, 300, 105, 300, 130 };

int32_t footprint_index_table[36] = { 0, 1, 4, 9, 16, 25, 2, 3, 6, 11, 18, 27, 5, 7, 8, 13, 20, 29, 10, 12, 14, 15, 22, 31, 17, 19, 21, 23, 24, 33, 26, 28, 30,
    32, 34, 35 };

const char str_suntemple3[12] = "suntemple3";
const char str_suntemple2[12] = "suntemple2";
const char str_suntemple1[16] = "suntemple1";

char lighthouse_script_names[10][12] = { "LtHouse10", "LtHouse9", "LtHouse8", "LtHouse7", "LtHouse6", "LtHouse5", "LtHouse4", "LtHouse3", "LtHouse2",
    "LtHouse1" };

pyramid_preset_part_table pyramid_preset_part_data = {
    .reserved_0 = 0,
    .sizes = {
        { { 4, 0 }, { 7, 1 }, { 12, GRID_SIZE }, { 15, GRID_SIZE + 1 } },
        { { 16, 0 }, { 21, 1 }, { 30, GRID_SIZE }, { 35, GRID_SIZE + 1 } },
        { { 36, 0 }, { 43, 1 }, { 56, GRID_SIZE }, { 63, GRID_SIZE + 1 } },
        { { 64, 0 }, { 73, 1 }, { 90, GRID_SIZE }, { 99, GRID_SIZE + 1 } },
    },
    .largest = { { 100, 0 }, { 104, 0 }, { 107, 1 }, { 111, 1 }, { 118, 0 }, { 119, 1 }, { 124, GRID_SIZE }, { 125, GRID_SIZE + 1 }, { 132, GRID_SIZE },
        { 136, GRID_SIZE }, { 139, GRID_SIZE + 1 }, { 143, GRID_SIZE + 1 } },
};

pyramid_preset_table pyramid_preset_parts = {
    .lists = {
        { pyramid_preset_part_data.sizes[0], PYRAMID_PRESET_PARTS },
        { pyramid_preset_part_data.sizes[1], PYRAMID_PRESET_PARTS },
        { pyramid_preset_part_data.sizes[2], PYRAMID_PRESET_PARTS },
        { pyramid_preset_part_data.sizes[3], PYRAMID_PRESET_PARTS },
        { pyramid_preset_part_data.largest, PYRAMID_PRESET_PARTS_LARGEST },
    },
    .mastaba_parts = {
        { { 1, 0 }, { 2, 1 }, { 8, GRID_SIZE }, { 9, GRID_SIZE + 1 } },
        { { 10, 0 }, { 12, 1 }, { 18, GRID_SIZE }, { 20, GRID_SIZE + 1 } },
        { { 21, 0 }, { 24, 1 }, { 32, GRID_SIZE }, { 35, GRID_SIZE + 1 } },
    },
};

pyramid_preset_list mastaba_preset_parts[MASTABA_SIZE_COUNT] = {
    { pyramid_preset_parts.mastaba_parts[0], PYRAMID_PRESET_PARTS },
    { pyramid_preset_parts.mastaba_parts[1], PYRAMID_PRESET_PARTS },
    { pyramid_preset_parts.mastaba_parts[2], PYRAMID_PRESET_PARTS },
};

int32_t pyramid_level_neighbour_offsets[PYRAMID_LEVEL_NEIGHBOURS] = { -GRID_SIZE, -GRID_SIZE + 1, 1, GRID_SIZE + 1, GRID_SIZE, GRID_SIZE - 1, -1, -GRID_SIZE - 1 };

generic_button ratings_advisor_buttons[RATINGS_ADVISOR_BUTTONS] = {
    { 80, 286, 200, 352, button_select_rating, button_none, 0, 0, 0, 1, 1, 0 },
    { 200, 286, 320, 352, button_select_rating, button_none, 0, 0, 0, 1, 2, 0 },
    { 320, 286, 440, 352, button_select_rating, button_none, 0, 0, 0, 1, 3, 0 },
    { 440, 286, 560, 352, button_select_rating, button_none, 0, 0, 0, 1, 4, 0 },
};

int32_t routed_building_direction_indices[32] = { 0, 2, 6, 4, 0, 2, 6, 4, 2, 4, 0, 6, 2, 4, 0, 6, 4, 6, 2, 0, 4, 6, 2, 0, 6, 0, 4, 2, 6, 0, 4, 2 };

const char str_obelisk_extra[16] = "OBELISK_EXTRA";

const char str_sun_temple_extra[20] = "SUN_TEMPLE_EXTRA";

const char str_mausoleum1_extra[20] = "MAUSOLEUM1_EXTRA";

const char str_mausoleum2_extra[20] = "MAUSOLEUM2_EXTRA";

const char str_mausoleum0_extra[20] = "MAUSOLEUM0_EXTRA";

char glory_of_ptolemy_name[164] = "The Glory of Ptolemy\0\0\0\0"
                                  "Graphic file out of sync\0\0\0\0"
                                  "The file \"\0\0"
                                  "\" contains \0"
                                  " tags, whereas the program expected \0\0\0\0"
                                  " tags. This could very well lead to a crash!";

const char *model_names[5] = { str_pharaoh_model_veryeasy_txt, str_pharaoh_model_easy_txt, str_pharaoh_model_normal_txt, str_pharaoh_model_hard_txt,
    str_pharaoh_model_impossible_txt };

const char *figure_names[5] = { str_figure_model_veryeasy_txt, str_figure_model_easy_txt, str_figure_model_normal_txt, str_figure_model_hard_txt,
    str_figure_model_impossible_txt };

const char *tax_names[5] = { str_tax_sentiment_model_veryeasy_txt, str_tax_sentiment_model_easy_txt, str_tax_sentiment_model_normal_txt,
    str_tax_sentiment_model_hard_txt, str_tax_sentiment_model_impossible_txt };

const char str_tax_sentiment_model_impossible_txt[36] = "Tax_Sentiment_Model_Impossible.txt";
const char str_tax_sentiment_model_hard_txt[32] = "Tax_Sentiment_Model_Hard.txt";
const char str_tax_sentiment_model_normal_txt[32] = "Tax_Sentiment_Model_Normal.txt";
const char str_tax_sentiment_model_easy_txt[32] = "Tax_Sentiment_Model_Easy.txt";
const char str_tax_sentiment_model_veryeasy_txt[36] = "Tax_Sentiment_Model_VeryEasy.txt";
const char str_figure_model_impossible_txt[28] = "Figure_model_Impossible.txt";
const char str_figure_model_hard_txt[24] = "Figure_model_Hard.txt";
const char str_figure_model_normal_txt[24] = "Figure_model_Normal.txt";
const char str_figure_model_easy_txt[24] = "Figure_model_Easy.txt";
const char str_figure_model_veryeasy_txt[28] = "Figure_model_VeryEasy.txt";
const char str_pharaoh_model_impossible_txt[32] = "Pharaoh_Model_Impossible.txt";
const char str_pharaoh_model_hard_txt[24] = "Pharaoh_Model_Hard.txt";
const char str_pharaoh_model_normal_txt[28] = "Pharaoh_Model_Normal.txt";
const char str_pharaoh_model_easy_txt[24] = "Pharaoh_Model_Easy.txt";
const char str_pharaoh_model_veryeasy_txt[28] = "Pharaoh_Model_VeryEasy.txt";

const char str_building_model_loaded[20] = " OK: model loaded";

const char str_all_houses[12] = "ALL HOUSES";

const char str_building_model_out_of_sync[36] = "Building model files out of sync";

char fmt_building_model_bad_lines[64] = "ERR: Building model has incorrect no of lines (%d), expected %d";

const char str_all_buildings[16] = "ALL BUILDINGS";

const char str_no_building_model_file[32] = "ERR:No Pharaoh_model.txt file";

const char str_no_spare_screen_model[32] = "ERR:no spare screen for model";

const char str_figure_model_loaded[32] = " OK: figure model data loaded";

const char str_all_enemies[12] = "ALL ENEMIES";

const char str_figure_model_out_of_sync[32] = "Figure model files out of sync";

char fmt_figure_model_bad_lines[64] = "ERR: Figure model has incorrect no of lines (%d), expected %d";

const char str_all_figures[12] = "ALL FIGURES";

const char str_no_figure_model_file[32] = "ERR:No Figure_model.txt file";

const char str_tax_sentiment_loaded[32] = " OK: tax sentiment data loaded";

const char str_tax_sentiment_out_of_sync[40] = "tax sentiment model files out of sync";

char fmt_tax_sentiment_bad_lines[72] = "ERR: Tax sentiment model has incorrect no of lines (%d), expected %d";

const char *sun_temple_scripts[4] = { str_suntemple1, str_suntemple2, str_suntemple3 };

int32_t sun_temple_block_pixel_offsets[8] = { 60, -29, 187, -29, 82, -96, 170, -100 };

int32_t ghost_tomb_footprint_tiles[36] = { 0, 1, 4, 9, 16, 25, 2, 3, 6, 11, 18, 27, 5, 7, 8, 13, 20, 29, 10, 12, 14, 15, 22, 31, 17, 19, 21, 23, 24, 33, 26, 28,
    30, 32, 34, 35 };

lighthouse_object_t lighthouse_object;

abu_simbel_object_t abu_simbel_object;

large_royal_tomb_record large_royal_tomb_object;

grand_royal_tomb_record grand_royal_tomb_object;

medium_royal_tomb_record medium_royal_tomb_object;

small_royal_tomb_record small_royal_tomb_object;

caesareum_object_t caesareum_object;

library_object_t library_object;

int32_t library_spot_variant;

building_type_counts building_count_active;

int32_t building_count_total[237];

static_assert(sizeof(building_model_record) * 237 == 9480, "building_model_table is the building model file's table of 9480 bytes");
building_model_record building_model_table[237];

static_assert(sizeof(enemy_models_table) == 3360, "enemy_model is the enemy model file's table of 3360 bytes");
enemy_models_table enemy_model;

static_assert(sizeof(house_model_table) == 1760, "house_models is the house model file's table of 1760 bytes");
house_model_table house_models;

int32_t venue_road_grid[25];

int32_t venue_remove_leaves_rubble;

int32_t house_evolve_expanded;

int32_t building_corrupt_houses_b;

int32_t building_corrupt_houses_a;

building_counter_block building_counters;

int32_t building_count_industry_active[36];

int32_t building_count_industry_total[36];

int32_t building_storage_highest_id;

int32_t building_highest_id_ever;

int32_t building_highest_id;

int32_t building_extra_sequence;

int32_t house_evolve_status;

int32_t building_list_small_size;

int32_t building_list_small_index;

int32_t building_list_burning_index;

int32_t building_burning_size;

int32_t building_burning_total;

int32_t building_list_large_size;

int32_t building_list_large_index;

int16_t granary_ids[100];

int32_t granary_count;

int16_t building_list_small[500];

int16_t building_list_burning[500];

int16_t building_list_large[4000];

static_assert(sizeof(building_storage) * 200 == 39200, "building_storages is the savegame's storage table of 39200 bytes");
building_storage building_storages[200];

static_assert(sizeof(building_table) == 1056108, "city_buildings is the savegame's building table of 1056108 bytes");
building_table city_buildings;

static_assert(sizeof(ferry_path) * 50 == 2200, "ferry_path_records is the savegame's ferry path table of 2200 bytes");
ferry_path ferry_path_records[50];

int32_t houses_frozen;

int32_t criminal_day_counter;

static_assert(sizeof(vegetation_growth_record) == 20, "vegetation_growth_state is the savegame's vegetation state of 20 bytes");
vegetation_growth_record vegetation_growth_state;

uint8_t map_edge_backup[51984];

uint8_t map_bitfields_backup[51984];

uint8_t map_canals_backup[51984];

uint32_t map_terrain_backup[51984];

static_assert(sizeof(map_grid_set) == 1663488, "map_grids is the savegame's map grids of 1663488 bytes");
map_grid_set map_grids;

int32_t undo_cost;

int32_t undo_mode;

clear_land_rectangle clear_land_rect;

clear_land_confirmations clear_land_confirmed;

int32_t construction_size;

int32_t water_orient_raw;

int32_t construction_bridge_length;

int32_t bridge_end_grid_offset;

int32_t construction_bridge_step;

int32_t construction_orientation;

int32_t construction_cost_refund;

int32_t routed_building_items_placed;

int32_t construction_cost;

int32_t construction_place_cost;

int32_t construction_ferry_first;

static_assert(sizeof(pharaohmap_info) == 84, "map_info is the savegame and scenario map info of 84 bytes");
pharaohmap_info map_info;

int32_t construction_ferry_state;

uint8_t construction_ferry_site_ok;

int32_t construction_canals_changed;

uint8_t osiris_double_yield_used;

int32_t farm_max_progress;

uint8_t farm_harvest_flags[FARM_CROP_ROWS];

int32_t building_barracks_tower_sentry_request;

int32_t building_warship_wharf_count;

int32_t formation_num_legions;

int32_t formation_id_last_legion;

int32_t formation_id_last_in_use;

int32_t storage_staff_missing;

uint16_t *screen_rgb565_ptr;

int32_t scenario_mausoleum_size_cached;

int32_t message_collapse_last_time;

int32_t message_fire_last_time;

int32_t tax_model[286];

int32_t city_current_player;

pyramid_complex_refresh_slots pyramid_complex_refresh_queue;

int32_t pyramid_sort_origin_x;

int32_t pyramid_frontier_parts[144];

uint8_t pyramid_complex_found;

int32_t pyramid_complex_search_building;

int32_t pyramid_complex_build_type;

int32_t pyramid_complex_free_tile;

int32_t pyramid_complex_area_end;

int32_t pyramid_frontier_count;

static_assert(sizeof(ferry_queue_records) == 11200, "ferry_queues is the savegame's ferry queues of 11200 bytes");
ferry_queue_records ferry_queues;
