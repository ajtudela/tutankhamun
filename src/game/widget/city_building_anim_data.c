// Built-in drawing places of the building animations (per building, shipwright view, fishing wharf and dock
// image, water lift view) and of the granary food piles; the config/*_anim_offsets.json and granary_pile_*.json
// files can override them.

#include "widget/city_building_anims.h"

building_anim_offset building_anim_offsets[BUILDING_ANIMS] = {
    [BUILDING_ANIM_MINE] = { 54, 15 },
    [BUILDING_ANIM_QUARRY_FIRST] = { 90, 17 },
    [BUILDING_ANIM_QUARRY_SECOND] = { 40, 20 },
    [BUILDING_ANIM_BANDSTAND_GATE] = { 52, -3 },
    [BUILDING_ANIM_BANDSTAND_STAGE] = { 20, 0 },
    [BUILDING_ANIM_CONSERVATORY] = { 82, 14 },
    [BUILDING_ANIM_BOOTH] = { 29, 14 },
    [BUILDING_ANIM_PAVILION] = { 64, 0 },
    [BUILDING_ANIM_DANCE_SCHOOL] = { 108, 1 },
    [BUILDING_ANIM_GRANARY] = { 113, 3 },
    [BUILDING_ANIM_STORAGE_YARD] = { 22, 24 },
    [BUILDING_ANIM_STORAGE_YARD_ROOF] = { -4, -42 },
    [BUILDING_ANIM_FORT_FLAG] = { 93, -20 },
    [BUILDING_ANIM_HOUSE_SICK_FLAG] = { 18, -32 },
};

building_anim_offset shipwright_anim_offsets[SHIPWRIGHT_ANIM_VIEWS] = {
    [0] = { 110, 20 },
    [1] = { 85, 20 },
    [2] = { 85, 20 },
    [3] = { 100, 10 },
};

building_anim_offset fishing_wharf_anim_offsets[FISHING_WHARF_ANIM_IMAGES] = {
    [0] = { 75, 10 },
    [1] = { 78, 7 },
    [2] = { 62, 18 },
    [3] = { 48, 8 },
    [4] = { 75, 10 },
    [5] = { 78, 7 },
    [6] = { 62, 18 },
    [7] = { 48, 8 },
};

building_anim_offset dock_anim_offsets[DOCK_ANIM_IMAGES] = {
    [0] = { 132, -7 },
    [1] = { 115, 25 },
    [2] = { 60, 20 },
    [3] = { 49, -4 },
    [4] = { 152, -13 },
    [5] = { 123, 16 },
    [6] = { 65, 14 },
    [7] = { 38, -12 },
};

building_anim_offset water_lift_anim_offsets[WATER_LIFT_ANIM_VIEWS] = {
    [0] = { 4, 0 },
    [1] = { 3, -2 },
    [2] = { 12, 0 },
    [3] = { 15, 6 },
};

building_anim_offset granary_pile_spots[GRANARY_PILE_SPOTS] = {
    [0] = { 110, -73 },
    [1] = { 126, -64 },
    [2] = { 145, -55 },
    [3] = { 161, -47 },
    [4] = { 94, -66 },
    [5] = { 111, -57 },
    [6] = { 130, -47 },
    [7] = { 147, -38 },
};

int32_t granary_pile_rules[GRANARY_PILE_RULES] = {
    [GRANARY_PILE_RULE_FIRST_AMOUNT] = 600,
    [GRANARY_PILE_RULE_AMOUNT_STEP] = 400,
    [GRANARY_PILE_RULE_FULL_AMOUNT] = 3200,
    [GRANARY_PILE_RULE_MAX_PILES] = 8,
};
