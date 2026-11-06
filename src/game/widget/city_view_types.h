// The city view's state record: the colour mask, disease message counters, bridge ghost pieces, water
// animation, minimap bounds, selection, the map size the lookup is built for, scroll bounds, viewport, camera,
// the tile being drawn and the view lookup, in a fixed field order.
//
// Reading the row above the top row of the lookup reaches the fields before it, so the order matters.

#ifndef WIDGET_CITY_VIEW_TYPES_H
#define WIDGET_CITY_VIEW_TYPES_H

#include <stdint.h>

enum city_view_state_size {
    CITY_VIEW_BRIDGE_DRAW_INTS = 160,
    CITY_VIEW_LOOKUP_INTS = 105567,
};

typedef struct city_view_state {
    int32_t reserved_1[11];
    uint16_t color_mask;
    uint16_t reserved_2;
    int32_t disease_message_plague_building;
    int32_t disease_message_malaria_building;
    int32_t disease_message_other_building;
    int32_t disease_message_plague;
    int32_t disease_message_malaria;
    int32_t disease_message_other;
    int32_t bridge_draw_count;
    int32_t bridge_draw_buf[CITY_VIEW_BRIDGE_DRAW_INTS];
    int32_t last_water_ms;
    int32_t advance_water;
    int32_t minimap_y1;
    int32_t minimap_x1;
    int32_t minimap_y0;
    int32_t minimap_x0;
    int32_t minimap_abs_y;
    int32_t minimap_abs_x;
    int32_t selected_figure_pixel_y;
    int32_t selected_figure_pixel_x;
    int32_t selected_pixel_x;
    int32_t selected_pixel_y;
    int32_t reserved_3[2];
    int32_t last_grid_offset;
    int32_t last_edge_y;
    int32_t last_edge_x;
    int32_t last_row;
    int32_t last_col;
    int32_t map_size_a;
    int32_t map_size_b;
    int32_t map_start_a;
    int32_t map_start_b;
    int32_t lookup_orientation;
    int32_t y_max;
    int32_t x_max;
    int32_t y_min;
    int32_t x_min;
    int32_t viewport_y1;
    int32_t viewport_y;
    int32_t viewport_x1;
    int32_t viewport_x;
    int32_t size4_y;
    int32_t size3_y;
    int32_t tile_height;
    int32_t tile_half_height;
    int32_t size4_x;
    int32_t size3_x;
    int32_t tile_width;
    int32_t tile_half_width;
    int32_t height_tiles;
    int32_t width_tiles;
    int32_t camera_col_max;
    int32_t map_height_tiles;
    int32_t map_width_tiles;
    int32_t camera_row;
    int32_t camera_col;
    int32_t row;
    int32_t col;
    int32_t selected_tile;
    int32_t grid_offset;
    int32_t origin_y;
    int32_t origin_x;
    int32_t lookup[CITY_VIEW_LOOKUP_INTS];
} city_view_state;

#endif
