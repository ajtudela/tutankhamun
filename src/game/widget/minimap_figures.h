// The minimap's figure tests and the figure kinds they return, and a building type's overlay tile.

#ifndef WIDGET_MINIMAP_FIGURES_H
#define WIDGET_MINIMAP_FIGURES_H

#include <stdint.h>

enum minimap_military_kind {
    MINIMAP_MILITARY_NONE = 0,
    MINIMAP_MILITARY_OWN_SOLDIER = 1,
    MINIMAP_MILITARY_ENEMY = 2,
    MINIMAP_MILITARY_PREDATOR = 3,
};

enum minimap_ship_kind {
    MINIMAP_SHIP_NONE = 0,
    MINIMAP_SHIP_OWN = 1,
    MINIMAP_SHIP_ENEMY = 2,
};

int32_t building_type_overlay_tile_index(int32_t type);
int32_t minimap_figure_show(int32_t figure_id);
int32_t minimap_figure_military_kind(int32_t figure_id);
int32_t minimap_figure_ship_kind(int32_t figure_id);

#endif
