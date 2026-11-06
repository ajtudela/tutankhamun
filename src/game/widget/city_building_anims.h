// The building animation helpers and the tables of their drawing places: offsets from a building's sprite
// origin per animation and view, the granary pile spots and the amounts that give a pile.

#ifndef WIDGET_CITY_BUILDING_ANIMS_H
#define WIDGET_CITY_BUILDING_ANIMS_H

#include "building/building.h"
#include <stdint.h>

enum building_anim {
    BUILDING_ANIM_MINE = 0,
    BUILDING_ANIM_QUARRY_FIRST = 1,
    BUILDING_ANIM_QUARRY_SECOND = 2,
    BUILDING_ANIM_BANDSTAND_GATE = 3,
    BUILDING_ANIM_BANDSTAND_STAGE = 4,
    BUILDING_ANIM_CONSERVATORY = 5,
    BUILDING_ANIM_BOOTH = 6,
    BUILDING_ANIM_PAVILION = 7,
    BUILDING_ANIM_DANCE_SCHOOL = 8,
    BUILDING_ANIM_GRANARY = 9,
    BUILDING_ANIM_STORAGE_YARD = 10,
    BUILDING_ANIM_STORAGE_YARD_ROOF = 11,
    BUILDING_ANIM_FORT_FLAG = 12,
    BUILDING_ANIM_HOUSE_SICK_FLAG = 13,
    BUILDING_ANIMS = 14,
};

enum building_anim_table_size {
    SHIPWRIGHT_ANIM_VIEWS = 4,
    FISHING_WHARF_ANIM_IMAGES = 8,
    DOCK_ANIM_IMAGES = 8,
    WATER_LIFT_ANIM_VIEWS = 4,
    GRANARY_PILE_SPOTS = 8,
};

enum storage_yard_roof {
    STORAGE_YARD_ROOF_IMAGE = 17,
};

enum granary_pile_rule {
    GRANARY_PILE_RULE_FIRST_AMOUNT = 0,
    GRANARY_PILE_RULE_AMOUNT_STEP = 1,
    GRANARY_PILE_RULE_FULL_AMOUNT = 2,
    GRANARY_PILE_RULE_MAX_PILES = 3,
    GRANARY_PILE_RULES = 4,
};

typedef struct {
    int32_t dx;
    int32_t dy;
} building_anim_offset;

extern building_anim_offset building_anim_offsets[BUILDING_ANIMS];
extern building_anim_offset shipwright_anim_offsets[SHIPWRIGHT_ANIM_VIEWS];
extern building_anim_offset fishing_wharf_anim_offsets[FISHING_WHARF_ANIM_IMAGES];
extern building_anim_offset dock_anim_offsets[DOCK_ANIM_IMAGES];
extern building_anim_offset water_lift_anim_offsets[WATER_LIFT_ANIM_VIEWS];
extern building_anim_offset granary_pile_spots[GRANARY_PILE_SPOTS];
extern int32_t granary_pile_rules[GRANARY_PILE_RULES];

void building_draw_shipwright_anim(int32_t image_id, int32_t anim, int32_t x, int32_t y, int32_t holder);
void building_draw_fishing_wharf_anim(int32_t image_id, int32_t anim, int32_t x, int32_t y, int32_t boat);
void building_draw_mine_anim(int32_t image_id, int32_t anim, int32_t x, int32_t y);
void building_draw_quarry_anim(int32_t image_id, int32_t anim, int32_t x, int32_t y);
void building_draw_bandstand_anim(int32_t image_id, int32_t anim, int32_t x, int32_t y);
void building_draw_conservatory_anim(int32_t image_id, int32_t anim, int32_t x, int32_t y);
void building_draw_booth_anim(int32_t image_id, int32_t anim, int32_t x, int32_t y);
void building_draw_pavilion_anim(int32_t image_id, int32_t anim, int32_t x, int32_t y);
void building_draw_dance_school_anim(int32_t image_id, int32_t anim, int32_t x, int32_t y);
void building_draw_storage_yard_roof(int32_t image_id, int32_t x, int32_t y);
void building_draw_type_anim(const building *b, int32_t image_id, int32_t frame, int32_t size, int32_t x, int32_t y);
void draw_granary_stores(int32_t building_id, int32_t grid_offset, int32_t x, int32_t y);

#endif
