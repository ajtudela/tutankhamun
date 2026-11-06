// The temple complex (building/temple_complex.c): site check, parts, decoration, annexes and the god's bonuses.

#ifndef BUILDING_TEMPLE_COMPLEX_H
#define BUILDING_TEMPLE_COMPLEX_H

#include <stdint.h>

enum temple_complex_upgrade {
    TEMPLE_COMPLEX_ALTAR = 1,
    TEMPLE_COMPLEX_ORACLE = 2,
};

enum temple_complex_layout {
    TEMPLE_COMPLEX_SITE_ROWS = 7,
    TEMPLE_COMPLEX_SITE_COLUMNS = 13,
    TEMPLE_COMPLEX_PARTS = 3,
    TEMPLE_COMPLEX_PART_SIZE = 3,
    TEMPLE_COMPLEX_ANNEX_WALL_REGION = 5,
    TEMPLE_DECOR_RECORD_INTS = 5,
};

enum temple_complex_site {
    TEMPLE_COMPLEX_SITE_ENTRY_POINT = -2,
    TEMPLE_COMPLEX_SITE_BLOCKED = -1,
    TEMPLE_COMPLEX_SITE_FITS = 1,
};

enum temple_complex_annex_slot {
    TEMPLE_COMPLEX_ANNEX_STATUE = 2,
    TEMPLE_COMPLEX_ANNEX_GOD = 3,
};

enum temple_complex_bonus {
    TEMPLE_COMPLEX_RA_KINGDOM_BONUS = 1,
    MILITARY_TRAINING_ACADEMY_BONUS = 25,
    TEMPLE_COMPLEX_SETH_TRAINING_BONUS = 10,
    TEMPLE_COMPLEX_RA_WAGE_BONUS = -2,
    FOOD_CONSUMPTION_PERCENTAGE = 25,
    TEMPLE_COMPLEX_FOOD_CONSUMPTION_PERCENTAGE = 20,
    TEMPLE_COMPLEX_BAST_CULTURE_DECAY_SKIP = 10,
};

int32_t construction_temple_complex_can_place(int32_t x, int32_t y);
void temple_complex_ra_kingdom_bonus(void);
int32_t city_military_training_bonus(int32_t active);
int32_t city_temple_complex_wage_bonus(void);
int32_t city_food_consumption_percentage(int32_t player);
void temple_complex_set_tile(int32_t x, int32_t y, int32_t image_index, int32_t image_offset);
void temple_complex_place_decoration(int32_t building_id);
void building_temple_complex_update_image(int32_t building_id, int32_t image_offset);
void building_gatehouse_update_image(int32_t building_id);
void temple_complex_draw_annex(int32_t building_id);
void building_temple_complex_add_to_map(int32_t building_id, int32_t x, int32_t y, int32_t create_a, int32_t create_b, int32_t place);

#endif
