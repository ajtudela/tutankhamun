// The build tool's entry points (building/construction_ui.c), the editor's special points menu and the
// scenario's monument menu definitions.

#ifndef BUILDING_CONSTRUCTION_UI_H
#define BUILDING_CONSTRUCTION_UI_H

#include "window/window_data.h"
#include <stdint.h>

enum editor_special_point_menu {
    SPECIAL_POINT_MENU_LAST_FISHING = 8,
    SPECIAL_POINT_MENU_LAST_KILLER = 12,
    SPECIAL_POINT_MENU_LAST_PREY = 16,
};

enum construction_menu_limit {
    SPHINX_MENU_STAGE = 1,
};

static inline int16_t scenario_monument_menu_def(int32_t item)
{
    return ((const int16_t *) &scenario_data.interest_rate)[1 + item];
}

void build_menu_set_type(void);
void building_construction_start(void);
void widget_city_build_move(void);
void widget_city_build_end(void);

#endif
