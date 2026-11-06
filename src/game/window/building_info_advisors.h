// The advisor buttons of the building info window (building_info_advisors.c): their draw and input, the offers per
// building type (building_info_advisor_offers.json) and the advisor per house evolve text (house_evolve_advisors.json).

#ifndef WINDOW_BUILDING_INFO_ADVISORS_H
#define WINDOW_BUILDING_INFO_ADVISORS_H

#include "building/building_type.h"
#include "building/house_evolve.h"
#include <stdint.h>

enum building_info_advisor_request {
    ADVISOR_REQUEST_NONE = 0,
    ADVISOR_REQUEST_FOOD = 1,
    ADVISOR_REQUEST_GOODS = 2,
};

enum {
    BUILDING_INFO_FIXED_ADVISORS = 3,
};

typedef struct {
    int32_t advisors[BUILDING_INFO_FIXED_ADVISORS];
    int32_t labor;
    int32_t burial_provisions;
    int32_t monument_material;
    int32_t kingdom_request;
    int32_t monument_workers;
    int32_t house_evolve;
} building_info_advisor_offer;

extern building_info_advisor_offer building_info_advisor_offers[BUILDING_TYPE_COUNT];
extern int32_t house_evolve_advisors[EVOLVE_TEXT_COUNT];

void window_building_info_advisors(int32_t *buttons);
int32_t window_building_house_advisor(int32_t text);
void window_building_handle_advisor_buttons(void);
void window_building_draw_advisor_buttons(void);

#endif
