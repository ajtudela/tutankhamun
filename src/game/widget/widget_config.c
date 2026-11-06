// Config tables of the widgets over their built-in C tables: bridge ghost pieces, temple complex part
// offsets, footprint tile counts and offsets, sidebar slide offsets, figure cross-country and progress offsets,
// the venue, fort and sphinx ghost tiles, the overlays' building lists, tooltip rules and footprint tiles, the
// building overlay tiles, the sidebar build pictures and the temple complex ghost passes.
//
// An overlay's building list ends at its first 0, so a file can change its types but not make it longer.

#include "building/building_type.h"
#include "core/config.h"
#include "widget/city_building_ghost.h"
#include "widget/city_building_tiles.h"
#include "widget/city_with_overlay.h"
#include "widget/construction_ghost_temple_complex.h"
#include "widget/sidebar_buttons.h"
#include "widget/widget_data.h"

static const config_field building_part_offset_fields[] = {
    { "x", &building_part_offsets[0].x, (int32_t) (sizeof(building_part_offset) / sizeof(int32_t)), 1, -100, 100, 0 },
    { "y", &building_part_offsets[0].y, (int32_t) (sizeof(building_part_offset) / sizeof(int32_t)), 1, -100, 100, 0 },
};

static const config_storage building_part_offset_storage[] = {
    { building_part_offsets, sizeof(building_part_offsets) },
};

const config_table building_part_offset_table = {
    "building_part_offsets.json",
    "index",
    0,
    BUILDING_PART_OFFSETS,
    CONFIG_TABLE_COLUMNS(building_part_offset_fields, building_part_offset_storage),
};

static const config_field sidebar_build_image_fields[] = {
    { "image", &sidebar_build_images[0].image, (int32_t) (sizeof(sidebar_build_image) / sizeof(int32_t)), 1, 0, 100, 0 },
};

static const config_storage sidebar_build_image_storage[] = {
    { sidebar_build_images, sizeof(sidebar_build_images) },
};

const config_table sidebar_build_image_table = {
    "sidebar_build_images.json",
    "group",
    0,
    SIDEBAR_BUILD_GROUPS,
    CONFIG_TABLE_COLUMNS(sidebar_build_image_fields, sidebar_build_image_storage),
};

enum { TEMPLE_COMPLEX_GHOST_PASS_STRIDE = sizeof(temple_complex_ghost_pass) / sizeof(int32_t) };

static const config_field temple_complex_ghost_pass_fields[] = {
    { "image_field", &temple_complex_ghost_passes[0].image_field[0], TEMPLE_COMPLEX_GHOST_PASS_STRIDE, TEMPLE_COMPLEX_GHOST_TURNS, 1, 4, 0 },
    { "first_rows_from", &temple_complex_ghost_passes[0].first_rows_from[0], TEMPLE_COMPLEX_GHOST_PASS_STRIDE, TEMPLE_COMPLEX_GHOST_TURNS, 0, 6, 0 },
    { "first_rows_to", &temple_complex_ghost_passes[0].first_rows_to[0], TEMPLE_COMPLEX_GHOST_PASS_STRIDE, TEMPLE_COMPLEX_GHOST_TURNS, 0, 6, 0 },
    { "second_rows_from", &temple_complex_ghost_passes[0].second_rows_from[0], TEMPLE_COMPLEX_GHOST_PASS_STRIDE, TEMPLE_COMPLEX_GHOST_TURNS, 0, 6, 0 },
    { "second_rows_to", &temple_complex_ghost_passes[0].second_rows_to[0], TEMPLE_COMPLEX_GHOST_PASS_STRIDE, TEMPLE_COMPLEX_GHOST_TURNS, 0, 6, 0 },
};

static const config_storage temple_complex_ghost_pass_storage[] = {
    { temple_complex_ghost_passes, sizeof(temple_complex_ghost_passes) },
};

const config_table temple_complex_ghost_pass_table = {
    "temple_complex_ghost_passes.json",
    "view",
    0,
    TEMPLE_COMPLEX_GHOST_VIEWS,
    CONFIG_TABLE_COLUMNS(temple_complex_ghost_pass_fields, temple_complex_ghost_pass_storage),
};

enum { BRIDGE_GHOST_PIECE_STRIDE = sizeof(bridge_ghost_piece_set) / sizeof(int32_t) };

static const config_field bridge_ghost_piece_fields[] = {
    { "low_start", &bridge_ghost_pieces[0].low_start, BRIDGE_GHOST_PIECE_STRIDE, 1, 0, 20, 0 },
    { "low_end", &bridge_ghost_pieces[0].low_end, BRIDGE_GHOST_PIECE_STRIDE, 1, 0, 20, 0 },
    { "low_span", &bridge_ghost_pieces[0].low_span, BRIDGE_GHOST_PIECE_STRIDE, 1, 0, 20, 0 },
    { "ship_start_image", &bridge_ghost_pieces[0].ship_start.image, BRIDGE_GHOST_PIECE_STRIDE, 1, 0, 20, 0 },
    { "ship_start_dx", &bridge_ghost_pieces[0].ship_start.dx, BRIDGE_GHOST_PIECE_STRIDE, 1, -100, 100, 0 },
    { "ship_start_dy", &bridge_ghost_pieces[0].ship_start.dy, BRIDGE_GHOST_PIECE_STRIDE, 1, -100, 100, 0 },
    { "ship_end_image", &bridge_ghost_pieces[0].ship_end.image, BRIDGE_GHOST_PIECE_STRIDE, 1, 0, 20, 0 },
    { "ship_end_dx", &bridge_ghost_pieces[0].ship_end.dx, BRIDGE_GHOST_PIECE_STRIDE, 1, -100, 100, 0 },
    { "ship_end_dy", &bridge_ghost_pieces[0].ship_end.dy, BRIDGE_GHOST_PIECE_STRIDE, 1, -100, 100, 0 },
    { "ship_middle_image", &bridge_ghost_pieces[0].ship_middle.image, BRIDGE_GHOST_PIECE_STRIDE, 1, 0, 20, 0 },
    { "ship_middle_dx", &bridge_ghost_pieces[0].ship_middle.dx, BRIDGE_GHOST_PIECE_STRIDE, 1, -100, 100, 0 },
    { "ship_middle_dy", &bridge_ghost_pieces[0].ship_middle.dy, BRIDGE_GHOST_PIECE_STRIDE, 1, -100, 100, 0 },
    { "ship_span_image", &bridge_ghost_pieces[0].ship_span.image, BRIDGE_GHOST_PIECE_STRIDE, 1, 0, 20, 0 },
    { "ship_span_dx", &bridge_ghost_pieces[0].ship_span.dx, BRIDGE_GHOST_PIECE_STRIDE, 1, -100, 100, 0 },
    { "ship_span_dy", &bridge_ghost_pieces[0].ship_span.dy, BRIDGE_GHOST_PIECE_STRIDE, 1, -100, 100, 0 },
};

static const config_storage bridge_ghost_piece_storage[] = {
    { bridge_ghost_pieces, sizeof(bridge_ghost_pieces) },
};

const config_table bridge_ghost_piece_table = {
    "bridge_ghost_pieces.json",
    "direction",
    0,
    BRIDGE_GHOST_DIRECTIONS,
    CONFIG_TABLE_COLUMNS(bridge_ghost_piece_fields, bridge_ghost_piece_storage),
};

enum { GHOST_TILE_PAIR_STRIDE = sizeof(ghost_tile_pair) / sizeof(int32_t) };

static const config_field ghost_fort_ground_fields[] = {
    { "tile_x", &ghost_fort_ground_tiles[0].x, GHOST_TILE_PAIR_STRIDE, 1, -20, 20, 0 },
    { "tile_y", &ghost_fort_ground_tiles[0].y, GHOST_TILE_PAIR_STRIDE, 1, -20, 20, 0 },
    { "half_tile_x", &ghost_fort_ground_half_tiles[0].x, GHOST_TILE_PAIR_STRIDE, 1, -20, 20, 0 },
    { "half_tile_y", &ghost_fort_ground_half_tiles[0].y, GHOST_TILE_PAIR_STRIDE, 1, -20, 20, 0 },
    { "pixel_x", &ghost_fort_ground_pixels[0], 1, 1, -100, 100, 0 },
    { "grid_offset", &ghost_fort_ground_grid_offsets[0], 1, 1, -10000, 10000, 0 },
};

static const config_storage ghost_fort_ground_storage[] = {
    { ghost_fort_ground_tiles, sizeof(ghost_fort_ground_tiles) },
    { ghost_fort_ground_half_tiles, sizeof(ghost_fort_ground_half_tiles) },
    { ghost_fort_ground_pixels, sizeof(ghost_fort_ground_pixels) },
    { ghost_fort_ground_grid_offsets, sizeof(ghost_fort_ground_grid_offsets) },
};

const config_table ghost_fort_ground_table = {
    "ghost_fort_ground.json",
    "view",
    0,
    GHOST_FORT_VIEWS,
    CONFIG_TABLE_COLUMNS(ghost_fort_ground_fields, ghost_fort_ground_storage),
};

static const config_field ghost_sphinx_part_fields[] = {
    { "tile_x", &ghost_sphinx_part_tiles[0].x, GHOST_TILE_PAIR_STRIDE, 1, -20, 20, 0 },
    { "tile_y", &ghost_sphinx_part_tiles[0].y, GHOST_TILE_PAIR_STRIDE, 1, -20, 20, 0 },
    { "pixel_x", &ghost_sphinx_part_pixels[0], 1, 1, -100, 100, 0 },
    { "grid_offset", &ghost_sphinx_part_grid_offsets[0], 1, 1, -10000, 10000, 0 },
};

static const config_storage ghost_sphinx_part_storage[] = {
    { ghost_sphinx_part_tiles, sizeof(ghost_sphinx_part_tiles) },
    { ghost_sphinx_part_pixels, sizeof(ghost_sphinx_part_pixels) },
    { ghost_sphinx_part_grid_offsets, sizeof(ghost_sphinx_part_grid_offsets) },
};

const config_table ghost_sphinx_part_table = {
    "ghost_sphinx_parts.json",
    "index",
    0,
    GHOST_SPHINX_PART_SLOTS,
    CONFIG_TABLE_COLUMNS(ghost_sphinx_part_fields, ghost_sphinx_part_storage),
};

static void *overlay_building_list(int32_t overlay, int32_t *count)
{
    int32_t *list = overlay_show_building_lists[overlay];
    *count = 0;
    while (list && list[*count] != 0) {
        (*count)++;
    }
    return list;
}

static const config_field overlay_building_fields[] = {
    { "type", 0, 0, 1, 1, BUILDING_TYPE_COUNT - 1, config_building_type_names, 0 },
};

static const config_list overlay_building_list_columns = {
    "building",
    sizeof(int32_t),
    overlay_building_fields,
    CONFIG_COUNT(overlay_building_fields),
    overlay_building_list,
    0,
};

static const config_field overlay_show_building_fields[] = {
    { "buildings", 0, 0, 1, 0, 0, 0, 0, 0, &overlay_building_list_columns },
};

static const config_storage overlay_show_building_storage[] = {
    { overlay_fire_buildings, sizeof(overlay_fire_buildings) },
    { overlay_damage_buildings, sizeof(overlay_damage_buildings) },
    { overlay_crime_buildings, sizeof(overlay_crime_buildings) },
    { overlay_disease_buildings, sizeof(overlay_disease_buildings) },
    { overlay_malaria_buildings, sizeof(overlay_malaria_buildings) },
    { overlay_entertainment_buildings, sizeof(overlay_entertainment_buildings) },
    { overlay_juggler_buildings, sizeof(overlay_juggler_buildings) },
    { overlay_musician_buildings, sizeof(overlay_musician_buildings) },
    { overlay_dancer_buildings, sizeof(overlay_dancer_buildings) },
    { overlay_senet_buildings, sizeof(overlay_senet_buildings) },
    { overlay_zookeeper_buildings, sizeof(overlay_zookeeper_buildings) },
    { overlay_education_buildings, sizeof(overlay_education_buildings) },
    { overlay_scribal_school_buildings, sizeof(overlay_scribal_school_buildings) },
    { overlay_library_buildings, sizeof(overlay_library_buildings) },
    { overlay_tax_income_buildings, sizeof(overlay_tax_income_buildings) },
    { overlay_bazaar_access_buildings, sizeof(overlay_bazaar_access_buildings) },
    { overlay_water_buildings, sizeof(overlay_water_buildings) },
    { overlay_dentist_buildings, sizeof(overlay_dentist_buildings) },
    { overlay_apothecary_buildings, sizeof(overlay_apothecary_buildings) },
    { overlay_physician_buildings, sizeof(overlay_physician_buildings) },
    { overlay_mortuary_buildings, sizeof(overlay_mortuary_buildings) },
    { overlay_religion_buildings, sizeof(overlay_religion_buildings) },
    { overlay_city_defenses_buildings, sizeof(overlay_city_defenses_buildings) },
    { overlay_magistrate_buildings, sizeof(overlay_magistrate_buildings) },
};

const config_table overlay_show_building_table = {
    "overlay_buildings.json",
    "overlay",
    config_overlay_names,
    OVERLAY_COUNT,
    CONFIG_TABLE_COLUMNS(overlay_show_building_fields, overlay_show_building_storage),
};

static void *overlay_tooltip_rule_block(int32_t overlay, int32_t *count)
{
    *count = 1;
    return &overlay_tooltip_rules[overlay];
}

static const config_field overlay_tooltip_rule_fields[] = {
    { "buildings", 0, 0, 1, OVERLAY_TOOLTIP_ANY_BUILDING, OVERLAY_TOOLTIP_NO_HOUSES, 0, offsetof(overlay_tooltip_rule, buildings), 1 },
    { "needs_building", 0, 0, 1, 0, 1, 0, offsetof(overlay_tooltip_rule, needs_building), 1 },
    { "needs_fertile_soil", 0, 0, 1, 0, 1, 0, offsetof(overlay_tooltip_rule, needs_fertile_soil), 1 },
};

static const config_list overlay_tooltip_rule_list = {
    "rule",
    sizeof(overlay_tooltip_rule),
    overlay_tooltip_rule_fields,
    CONFIG_COUNT(overlay_tooltip_rule_fields),
    overlay_tooltip_rule_block,
    0,
};

static const config_field overlay_tooltip_fields[] = {
    { "tooltip", 0, 0, 1, 0, 0, 0, 0, 0, &overlay_tooltip_rule_list },
};

static const config_storage overlay_tooltip_storage[] = {
    { overlay_tooltip_rules, sizeof(overlay_tooltip_rules) },
};

const config_table overlay_tooltip_table = {
    "overlay_tooltips.json",
    "overlay",
    config_overlay_names,
    OVERLAY_COUNT,
    CONFIG_TABLE_COLUMNS(overlay_tooltip_fields, overlay_tooltip_storage),
};

enum { OVERLAY_FOOTPRINT_SIZES = 7 };

static void *overlay_footprint_tile_block(int32_t size, int32_t *count)
{
    static int32_t *const tables[OVERLAY_FOOTPRINT_SIZES] = { 0, 0, overlay_tiles_size2, overlay_tiles_size3, overlay_tiles_size4,
        overlay_tiles_size5, overlay_tiles_size6 };
    *count = tables[size] ? size * size : 0;
    return tables[size];
}

static const config_field overlay_footprint_tile_element_fields[] = {
    { "tile", 0, 0, 1, OVERLAY_TILE_CORNER, OVERLAY_TILE_INNER, config_overlay_tile_names, 0 },
};

static const config_list overlay_footprint_tile_list = {
    "tile",
    sizeof(int32_t),
    overlay_footprint_tile_element_fields,
    CONFIG_COUNT(overlay_footprint_tile_element_fields),
    overlay_footprint_tile_block,
    0,
};

static const config_field overlay_footprint_tile_fields[] = {
    { "tiles", 0, 0, 1, 0, 0, 0, 0, 0, &overlay_footprint_tile_list },
};

static const config_storage overlay_footprint_tile_storage[] = {
    { overlay_tiles_size2, sizeof(overlay_tiles_size2) },
    { overlay_tiles_size3, sizeof(overlay_tiles_size3) },
    { overlay_tiles_size4, sizeof(overlay_tiles_size4) },
    { overlay_tiles_size5, sizeof(overlay_tiles_size5) },
    { overlay_tiles_size6, sizeof(overlay_tiles_size6) },
};

const config_table overlay_footprint_tile_table = {
    "overlay_footprint_tiles.json",
    "size",
    0,
    OVERLAY_FOOTPRINT_SIZES,
    CONFIG_TABLE_COLUMNS(overlay_footprint_tile_fields, overlay_footprint_tile_storage),
};

static const config_field building_overlay_tile_fields[] = {
    { "overlay_tile", &building_type_overlay_tile[0], 1, 1, 0, 100, 0 },
};

static const config_storage building_overlay_tile_storage[] = {
    { building_type_overlay_tile, sizeof(building_type_overlay_tile) },
};

const config_table building_overlay_tile_table = {
    "building_overlay_tiles.json",
    "type",
    config_building_type_names,
    BUILDING_OVERLAY_TILE_TYPES,
    CONFIG_TABLE_COLUMNS(building_overlay_tile_fields, building_overlay_tile_storage),
};

static const config_field ghost_bandstand_tiles_fields[] = {
    { "tiles", &ghost_bandstand_tiles[0][0], GHOST_BANDSTAND_TILES, GHOST_BANDSTAND_TILES, 0, GHOST_BANDSTAND_TILES - 1, 0 },
};

static const config_storage ghost_bandstand_tiles_storage[] = {
    { ghost_bandstand_tiles, sizeof(ghost_bandstand_tiles) },
};

const config_table ghost_bandstand_tiles_table = {
    "ghost_bandstand_tiles.json",
    "view",
    0,
    GHOST_VENUE_VIEWS,
    CONFIG_TABLE_COLUMNS(ghost_bandstand_tiles_fields, ghost_bandstand_tiles_storage),
};

static const config_field ghost_booth_tiles_fields[] = {
    { "tiles", &venue_booth_tiles[0][0], GHOST_BOOTH_TILES, GHOST_BOOTH_TILES, 0, GHOST_BOOTH_TILES - 1, 0 },
};

static const config_storage ghost_booth_tiles_storage[] = {
    { venue_booth_tiles, sizeof(venue_booth_tiles) },
};

const config_table ghost_booth_tiles_table = {
    "ghost_booth_tiles.json",
    "view",
    0,
    GHOST_VENUE_VIEWS,
    CONFIG_TABLE_COLUMNS(ghost_booth_tiles_fields, ghost_booth_tiles_storage),
};

static const config_field ghost_festival_tiles_fields[] = {
    { "tiles", &ghost_festival_tiles[0][0], GHOST_FESTIVAL_TILES, GHOST_FESTIVAL_TILES, 0, GHOST_FESTIVAL_TILES - 1, 0 },
};

static const config_storage ghost_festival_tiles_storage[] = {
    { ghost_festival_tiles, sizeof(ghost_festival_tiles) },
};

const config_table ghost_festival_tiles_table = {
    "ghost_festival_tiles.json",
    "view",
    0,
    GHOST_VENUE_VIEWS,
    CONFIG_TABLE_COLUMNS(ghost_festival_tiles_fields, ghost_festival_tiles_storage),
};

static const config_field ghost_pavilion_tiles_fields[] = {
    { "tiles", &venue_pavilion_tiles[0][0], GHOST_PAVILION_TILES, GHOST_PAVILION_TILES, 0, GHOST_PAVILION_TILES - 1, 0 },
};

static const config_storage ghost_pavilion_tiles_storage[] = {
    { venue_pavilion_tiles, sizeof(venue_pavilion_tiles) },
};

const config_table ghost_pavilion_tiles_table = {
    "ghost_pavilion_tiles.json",
    "view",
    0,
    GHOST_VENUE_VIEWS,
    CONFIG_TABLE_COLUMNS(ghost_pavilion_tiles_fields, ghost_pavilion_tiles_storage),
};

static const config_field footprint_tile_count_fields[] = {
    { "tiles", &tile_count_for_size[0], 1, 1, 0, GHOST_FOOTPRINT_TILES, 0 },
};

static const config_storage footprint_tile_count_storage[] = {
    { tile_count_for_size, sizeof(tile_count_for_size) },
};

const config_table footprint_tile_count_table = {
    "footprint_tile_counts.json",
    "size",
    0,
    FOOTPRINT_SIZES,
    CONFIG_TABLE_COLUMNS(footprint_tile_count_fields, footprint_tile_count_storage),
};

static const config_field ghost_footprint_tile_fields[] = {
    { "x", &construction_ghost_tile_offsets[0].x, GHOST_TILE_PAIR_STRIDE, 1, -1000, 1000, 0 },
    { "y", &construction_ghost_tile_offsets[0].y, GHOST_TILE_PAIR_STRIDE, 1, -1000, 1000, 0 },
};

static const config_storage ghost_footprint_tile_storage[] = {
    { construction_ghost_tile_offsets, sizeof(construction_ghost_tile_offsets) },
};

const config_table ghost_footprint_tile_table = {
    "ghost_footprint_tiles.json",
    "tile",
    0,
    GHOST_FOOTPRINT_TILES,
    CONFIG_TABLE_COLUMNS(ghost_footprint_tile_fields, ghost_footprint_tile_storage),
};

static const config_field sidebar_slide_fields[] = {
    { "offset", &sidebar_slide_offsets[0], 1, 1, 0, 1000, 0 },
};

static const config_storage sidebar_slide_storage[] = {
    { sidebar_slide_offsets, sizeof(sidebar_slide_offsets) },
};

const config_table sidebar_slide_table = {
    "sidebar_slide.json",
    "step",
    0,
    SIDEBAR_SLIDE_OFFSETS,
    CONFIG_TABLE_COLUMNS(sidebar_slide_fields, sidebar_slide_storage),
};

static const config_field figure_cross_country_offset_fields[] = {
    { "view0_x", (int32_t *) tbl_cc_ori0_x, FIGURE_CROSS_COUNTRY_ROWS / 4, FIGURE_CROSS_COUNTRY_ROWS, -100, 100, 0, 0, 1 },
    { "view0_y", (int32_t *) tbl_cc_ori0_y, FIGURE_CROSS_COUNTRY_ROWS / 4, FIGURE_CROSS_COUNTRY_ROWS, -100, 100, 0, 0, 1 },
    { "view2_x", (int32_t *) tbl_cc_ori2_x, FIGURE_CROSS_COUNTRY_ROWS / 4, FIGURE_CROSS_COUNTRY_ROWS, -100, 100, 0, 0, 1 },
    { "view2_y", (int32_t *) tbl_cc_ori2_y, FIGURE_CROSS_COUNTRY_ROWS / 4, FIGURE_CROSS_COUNTRY_ROWS, -100, 100, 0, 0, 1 },
};

static const config_storage figure_cross_country_offset_storage[] = {
    { tbl_cc_ori0_x, sizeof(tbl_cc_ori0_x) },
    { tbl_cc_ori0_y, sizeof(tbl_cc_ori0_y) },
    { tbl_cc_ori2_x, sizeof(tbl_cc_ori2_x) },
    { tbl_cc_ori2_y, sizeof(tbl_cc_ori2_y) },
};

const config_table figure_cross_country_offset_table = {
    "figure_cross_country_offsets.json",
    "row",
    0,
    FIGURE_CROSS_COUNTRY_ROWS,
    CONFIG_TABLE_COLUMNS(figure_cross_country_offset_fields, figure_cross_country_offset_storage),
};

static const config_field figure_progress_offset_fields[] = {
    { "x", (int32_t *) tbl_progress_x, FIGURE_PROGRESS_STEPS / 4, FIGURE_PROGRESS_STEPS, -100, 100, 0, 0, 1 },
    { "y", (int32_t *) tbl_progress_y, FIGURE_PROGRESS_STEPS / 4, FIGURE_PROGRESS_STEPS, -100, 100, 0, 0, 1 },
};

static const config_storage figure_progress_offset_storage[] = {
    { tbl_progress_x, sizeof(tbl_progress_x) },
    { tbl_progress_y, sizeof(tbl_progress_y) },
};

const config_table figure_progress_offset_table = {
    "figure_progress_offsets.json",
    "direction",
    0,
    FIGURE_PROGRESS_DIRECTIONS,
    CONFIG_TABLE_COLUMNS(figure_progress_offset_fields, figure_progress_offset_storage),
};
