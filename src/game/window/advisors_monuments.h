// The monuments advisor (advisors_monuments.c): its draws and buttons, the burial provisions a monument still needs,
// and the texts of the monuments advisor's group.

#ifndef WINDOW_ADVISORS_MONUMENTS_H
#define WINDOW_ADVISORS_MONUMENTS_H

#include <stdint.h>

enum monuments_advisor_text {
    MONUMENTS_TEXT_OVERSEER = 0,
    MONUMENTS_TEXT_IN_STORAGE = 1,
    MONUMENTS_TEXT_FINISHED = 2,
    MONUMENTS_TEXT_CLICK_PROVISIONS_TO_DELIVER = 3,
    MONUMENTS_TEXT_AMOUNT_TO_DISPATCH = 4,
    MONUMENTS_TEXT_ALL = 5,
    MONUMENTS_TEXT_CLICK_TO_DISPATCH = 6,
    MONUMENTS_TEXT_INCREASE_AMOUNT = 8,
    MONUMENTS_TEXT_DECREASE_AMOUNT = 9,
    MONUMENTS_TEXT_DISPATCH_BURIAL_PROVISIONS = 10,
    MONUMENTS_TEXT_MONUMENT_RATING = 11,
    MONUMENTS_TEXT_NO_PROVISIONS_NEEDED = 12,
    MONUMENTS_TEXT_VISIT_SITE = 13,
    PYRAMID_TEXT_FIRST = 14,
    BRICK_PYRAMID_TEXT_FIRST = 19,
    STEPPED_PYRAMID_TEXT_FIRST = 24,
    BENT_PYRAMID_TEXT_FIRST = 29,
    MASTABA_TEXT_FIRST = 34,
    SPHINX_TEXT_FIRST = 39,
    OBELISK_TEXT_FIRST = 43,
    SUN_TEMPLE_TEXT_FIRST = 47,
    MAUSOLEUM_TEXT_FIRST = 51,
    MONUMENTS_TEXT_BLOCK_STORED = 57,
    MONUMENTS_TEXT_BLOCKS_STORED = 58,
    LIBRARY_TEXT_FIRST = 59,
    CAESAREUM_TEXT_FIRST = 63,
    LIGHTHOUSE_TEXT_FIRST = 67,
    ABU_SIMBEL_TEXT_FIRST = 71,
    SMALL_TOMB_TEXT_FIRST = 75,
    MEDIUM_TOMB_TEXT_FIRST = 79,
    LARGE_TOMB_TEXT_FIRST = 83,
    GRAND_TOMB_TEXT_FIRST = 87,
};

void window_advisor_monuments_draw_background(void);
void window_advisor_monuments_draw_foreground(void);
int32_t monument_count_needed_resources(int32_t skip);
int32_t monument_needed_resource_nth(int32_t n);
void button_monument_resource(void);
void button_monument_go_to(void);

#endif
