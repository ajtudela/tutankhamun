// The widget tables of widget/widget_data.c: the sidebar, the advisor bar, the top menu, the minimap, the overlays and
// the construction ghosts, with their sizes.

#ifndef WIDGET_WIDGET_DATA_H
#define WIDGET_WIDGET_DATA_H

#include "graphics/button.h"
#include "widget/city_view_types.h"
#include "widget/city_with_overlay.h"
#include "widget/ghost_tile_types.h"
#include <stdint.h>

enum widget_table_sizes {
    FIGURE_CROSS_COUNTRY_ROWS = 16,
    FIGURE_CROSS_COUNTRY_OFFSETS = 256,
    FIGURE_PROGRESS_DIRECTIONS = 8,
    FIGURE_PROGRESS_STEPS = 16,
    FIGURE_PROGRESS_OFFSETS = 128,
    FOOTPRINT_SIZES = 7,
    GHOST_FOOTPRINT_TILES = 36,
    BUILDING_OVERLAY_TILE_TYPES = 236,
    SIDEBAR_SLIDE_OFFSETS = 48,
    ADVISOR_BAR_BUTTONS = 14,
};

enum ghost_table_sizes {
    GHOST_VENUE_VIEWS = 4,
    GHOST_BOOTH_TILES = 4,
    GHOST_PAVILION_TILES = 16,
    GHOST_BANDSTAND_TILES = 9,
    GHOST_FESTIVAL_TILES = 25,
    GHOST_FORT_VIEWS = 4,
    MONUMENT_GHOST_BLOCK_SIZE = 2,
};

enum overlay_tooltip_buildings {
    OVERLAY_TOOLTIP_ANY_BUILDING = 0,
    OVERLAY_TOOLTIP_HOUSES_ONLY = 1,
    OVERLAY_TOOLTIP_NO_HOUSES = 2,
};

typedef struct overlay_tooltip_rule {
    uint8_t buildings;
    uint8_t needs_building;
    uint8_t needs_fertile_soil;
} overlay_tooltip_rule;

extern int8_t tbl_cc_ori0_x[FIGURE_CROSS_COUNTRY_OFFSETS];

extern int8_t tbl_cc_ori0_y[FIGURE_CROSS_COUNTRY_OFFSETS];

extern int8_t tbl_cc_ori2_x[FIGURE_CROSS_COUNTRY_OFFSETS];

extern int8_t tbl_cc_ori2_y[FIGURE_CROSS_COUNTRY_OFFSETS];

extern int8_t tbl_progress_x[FIGURE_PROGRESS_OFFSETS];

extern int8_t tbl_progress_y[FIGURE_PROGRESS_OFFSETS];

extern int32_t building_type_overlay_tile[BUILDING_OVERLAY_TILE_TYPES];

extern int32_t tile_count_for_size[FOOTPRINT_SIZES];

extern ghost_tile_pair construction_ghost_tile_offsets[GHOST_FOOTPRINT_TILES];

enum {
    BUILDING_PART_OFFSETS = 16,
};

typedef struct building_part_offset {
    int32_t x;
    int32_t y;
} building_part_offset;

extern building_part_offset building_part_offsets[BUILDING_PART_OFFSETS];

extern int32_t venue_booth_tiles[GHOST_VENUE_VIEWS][GHOST_BOOTH_TILES];

extern int32_t ghost_bandstand_tiles[GHOST_VENUE_VIEWS][GHOST_BANDSTAND_TILES];

extern int32_t venue_pavilion_tiles[GHOST_VENUE_VIEWS][GHOST_PAVILION_TILES];

extern int32_t ghost_festival_tiles[GHOST_VENUE_VIEWS][GHOST_FESTIVAL_TILES];

extern int32_t overlay_fire_buildings[2];

extern int32_t overlay_damage_buildings[2];

extern int32_t overlay_crime_buildings[7];

extern int32_t overlay_disease_buildings[2];

extern int32_t overlay_malaria_buildings[2];

extern int32_t overlay_entertainment_buildings[10];

extern int32_t overlay_juggler_buildings[3];

extern int32_t overlay_musician_buildings[3];

extern int32_t overlay_dancer_buildings[3];

extern int32_t overlay_senet_buildings[2];

extern int32_t overlay_zookeeper_buildings[2];

extern int32_t overlay_education_buildings[3];

extern int32_t overlay_scribal_school_buildings[2];

extern int32_t overlay_library_buildings[2];

extern int32_t overlay_tax_income_buildings[6];

extern int32_t overlay_bazaar_access_buildings[8];

extern int32_t overlay_water_buildings[3];

extern int32_t overlay_dentist_buildings[2];

extern int32_t overlay_apothecary_buildings[2];

extern int32_t overlay_physician_buildings[2];

extern int32_t overlay_mortuary_buildings[2];

extern int32_t overlay_religion_buildings[20];

extern int32_t overlay_city_defenses_buildings[25];

extern int32_t overlay_magistrate_buildings[2];

extern int32_t *overlay_show_building_lists[OVERLAY_COUNT];

extern int32_t overlay_tiles_size2[4];

extern int32_t overlay_tiles_size3[9];

extern int32_t overlay_tiles_size4[16];

extern int32_t overlay_tiles_size5[25];

extern int32_t overlay_tiles_size6[36];

extern ghost_tile_pair ghost_fort_ground_tiles[GHOST_FORT_VIEWS];

extern ghost_tile_pair ghost_fort_ground_half_tiles[GHOST_FORT_VIEWS];

extern int32_t ghost_fort_ground_pixels[GHOST_FORT_VIEWS];

extern int32_t ghost_fort_ground_grid_offsets[GHOST_FORT_VIEWS];

extern int32_t ghost_sphinx_part_grid_offsets[GHOST_SPHINX_PART_SLOTS];

extern ghost_tile_pair ghost_sphinx_part_tiles[GHOST_SPHINX_PART_SLOTS];

extern int32_t ghost_sphinx_part_pixels[GHOST_SPHINX_PART_SLOTS];

extern overlay_tooltip_rule overlay_tooltip_rules[OVERLAY_COUNT];

extern char format_numbered[8];

extern char text_line_break[4];

extern uint8_t str_comma[8];

extern int32_t sidebar_slide_offsets[SIDEBAR_SLIDE_OFFSETS];

extern const char hud_level_label[8];

extern uint8_t str_event[8];

extern uint8_t str_minor_bad[12];

extern uint8_t str_major_bad[12];

extern uint8_t str_minor_good[12];

extern uint8_t str_major_good[12];

extern const char hud_max_prosperity_label[16];

extern const char hud_happiness_label[12];

extern uint8_t str_allow_all[12];

extern float debug_key_value;

extern int32_t debug_key_toggle;

extern int32_t debug_keys_enabled;

extern image_button advisor_bar_buttons[ADVISOR_BAR_BUTTONS];

extern uint8_t military_empire_button_shown;


extern uint8_t construction_placing_royal_tomb;

extern int32_t editor_wide_bounds;

extern int32_t rotate_button_hovered_part;

extern int32_t top_menu_focus_info_id;

extern int32_t minimap_hovered;

extern int32_t top_menu_focus_menu_id;

extern int32_t menu_bar_focus_item;

extern city_view_state city_view;

extern uint8_t g_fog_disabled;

extern int32_t scenario_max_year;

extern int32_t debug_figure_info_enabled;

extern int32_t debug_building_info_enabled;

extern int32_t debug_evolve_info_enabled;

extern int32_t debug_evolve_error;

extern int32_t undo_ready;

extern int32_t undo_available;

extern int32_t bridge_direction;

extern char tooltip_composed[500];

extern int32_t tooltip_type;

extern int32_t tooltip_last_time;

extern int32_t tooltip_box_height;

extern int32_t tooltip_box_width;

extern int32_t tooltip_box_y;

extern int32_t tooltip_box_x;

extern int32_t tooltip_text_id;

extern int32_t g_last_image_id;

extern int32_t scroll_in_progress;

extern int32_t time_current_millis;

extern int32_t g_map_visible_y1;

extern int32_t g_map_visible_y0;

extern int32_t g_map_visible_x1;

extern int32_t g_map_visible_x0;

extern int32_t city_view_orientation;

extern int32_t city_overlay;

extern int32_t city_overlay_previous;

extern void *debug_sprite_a;

extern uint8_t *debug_sprite_b;

extern int32_t debug_city_info_enabled;

extern int32_t city_view_width_collapsed;

extern int32_t city_view_width_expanded;

extern int32_t debug_show_happiness;

extern int32_t g_top_menu_force;

extern int32_t g_sidebar_collapsed;

extern int32_t g_sidebar_slide_step;

extern int32_t ambient_scores[12];


#endif
