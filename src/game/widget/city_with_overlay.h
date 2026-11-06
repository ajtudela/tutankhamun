// The city view with an overlay: which buildings and figures it shows and the footprints and tops of the
// buildings drawn with it. Overlay ids follow the overlay names; 0 is the normal view, 51 the view with the
// cliffs hidden.

#ifndef WIDGET_CITY_WITH_OVERLAY_H
#define WIDGET_CITY_WITH_OVERLAY_H

#include <stdbool.h>
#include <stdint.h>

enum city_overlay_id {
    OVERLAY_NONE = 0,
    OVERLAY_RELIGION = 4,
    OVERLAY_FIRE = 8,
    OVERLAY_DAMAGE = 9,
    OVERLAY_CRIME = 10,
    OVERLAY_ENTERTAINMENT = 11,
    OVERLAY_JUGGLER = 12,
    OVERLAY_MUSICIAN = 13,
    OVERLAY_DANCER = 14,
    OVERLAY_SENET = 15,
    OVERLAY_ZOOKEEPER = 16,
    OVERLAY_EDUCATION = 17,
    OVERLAY_SCRIBAL_SCHOOL = 18,
    OVERLAY_LIBRARY = 19,
    OVERLAY_WATER_CROSSINGS = 20,
    OVERLAY_DENTIST = 21,
    OVERLAY_PHYSICIAN = 22,
    OVERLAY_APOTHECARY = 23,
    OVERLAY_MORTUARY = 24,
    OVERLAY_TAX_INCOME = 25,
    OVERLAY_BAZAAR_ACCESS = 26,
    OVERLAY_DESIRABILITY = 27,
    OVERLAY_FERTILITY = 28,
    OVERLAY_LABOR = 29,
    OVERLAY_NATIVE = 30,
    OVERLAY_PROBLEMS = 31,
    OVERLAY_GRAIN = 33,
    OVERLAY_CHICKPEAS = 34,
    OVERLAY_POMEGRANATES = 35,
    OVERLAY_FIGS = 36,
    OVERLAY_MEAT = 37,
    OVERLAY_GAME = 38,
    OVERLAY_POTTERY = 39,
    OVERLAY_JEWELRY = 40,
    OVERLAY_LINEN = 41,
    OVERLAY_BEER = 42,
    OVERLAY_DISEASE = 43,
    OVERLAY_INFECTED_HOUSING = 44,
    OVERLAY_WATER = 45,
    OVERLAY_EMPTY_HOUSING = 46,
    OVERLAY_IRRIGATION = 47,
    OVERLAY_MALARIA = 48,
    OVERLAY_CITY_DEFENSES = 49,
    OVERLAY_MAGISTRATE = 50,
    OVERLAY_HIDE_CLIFFS = 51,
    OVERLAY_COUNT = 52,
};

bool city_with_overlay_show_building(int32_t building_id);
int32_t city_with_overlay_show_figure(int32_t figure_id);
int32_t city_with_overlay_draw_building_footprint(int32_t building_id, int32_t x, int32_t y);
void city_with_overlay_draw_footprint(void);
void city_with_overlay_draw_top(void);

#endif
