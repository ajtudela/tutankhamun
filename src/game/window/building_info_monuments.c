// The building info panels of the monuments (pyramids, sphinx, obelisks, sun temple, the library, Abu Simbel,
// Caesareum, lighthouse, royal tombs, mausoleum) and the two text tokens of their requirement texts. Each panel shows
// the construction foreman's text and the requirement lines; the pyramid styles come from pyramid_style_rules.
//
// The library's guild tests read the Caesareum's progress report.

#include "window/building_info_monuments.h"
#include "building/abu_simbel.h"
#include "building/building.h"
#include "building/building_figure_helpers.h"
#include "building/building_maintenance.h"
#include "building/building_type.h"
#include "building/library.h"
#include "building/mausoleum.h"
#include "building/monument.h"
#include "building/monument_data.h"
#include "building/monument_tables.h"
#include "building/monument_work_spots.h"
#include "building/obelisk.h"
#include "building/pyramid_blocks.h"
#include "building/royal_tomb.h"
#include "building/sphinx.h"
#include "building/sun_temple.h"
#include "city/kingdom.h"
#include "city/resource_type.h"
#include "figure/figure_type.h"
#include "flood/flood_types.h"
#include "game/game_internal.h"
#include "graphics/graphics_data.h"
#include "graphics/image_collection.h"
#include "graphics/panel_draw.h"
#include "graphics/text_draw.h"
#include "graphics/ui_layout.h"
#include "map/grid.h"
#include "text/event_text.h"
#include "text/lang.h"
#include "text/text_data.h"
#include "window/building_info.h"
#include "window/building_info_panels.h"
#include "window/window_data.h"
#include "window/window_image_group.h"
#include <stdint.h>
#include <string.h>

enum monument_text {
    MONUMENT_TEXT_COMPLETE = 0,
    MONUMENT_TEXT_FINISHING_OUTSIDE = 1,
    MONUMENT_TEXT_TASK_IS = 2,
    MONUMENT_TEXT_CLEARING_PYRAMID_SITE = 3,
    MONUMENT_TEXT_GROOVES_CUT = 5,
    MONUMENT_TEXT_SITE_PREPARED = 10,
    MONUMENT_TEXT_ROOF = 11,
    MONUMENT_TEXT_FOREMAN = 12,
    MONUMENT_TEXT_NO_WORK_CAMP = 13,
    MONUMENT_TEXT_NO_STONEMASONS = 14,
    MONUMENT_TEXT_NO_BRICKLAYERS = 15,
    MONUMENT_TEXT_NO_CARPENTERS = 16,
    MONUMENT_TEXT_NO_PEASANTS = 17,
    MONUMENT_TEXT_STONEMASONS_NOT_COMING = 18,
    MONUMENT_TEXT_BRICKLAYERS_NOT_COMING = 19,
    MONUMENT_TEXT_CARPENTERS_NOT_COMING = 20,
    MONUMENT_TEXT_WAITING_FOR_CARPENTER = 21,
    MONUMENT_TEXT_NEEDS_STONE = 22,
    MONUMENT_TEXT_NEEDS_LIMESTONE = 23,
    MONUMENT_TEXT_NEEDS_SANDSTONE = 24,
    MONUMENT_TEXT_NEEDS_GRANITE = 25,
    MONUMENT_TEXT_NEEDS_MARBLE = 26,
    MONUMENT_TEXT_NEEDS_BRICKS = 27,
    MONUMENT_TEXT_NEEDS_COPPER = 29,
    MONUMENT_TEXT_OBELISK_CARPENTERS_FINISHING = 42,
    MONUMENT_TEXT_OBELISK_CARVING = 43,
    MONUMENT_TEXT_OBELISK_COMPLETE = 44,
    MONUMENT_TEXT_SPHINX_ROUGH_SHAPE = 45,
    MONUMENT_TEXT_SPHINX_FINE_CARVING = 46,
    MONUMENT_TEXT_SPHINX_FINISHING = 47,
    MONUMENT_TEXT_SPHINX_COMPLETE = 48,
    MONUMENT_TEXT_CARPENTRY_IS = 49,
    MONUMENT_TEXT_MASON_WORK_IS = 50,
    MONUMENT_TEXT_PEASANTS_ON_FLOODPLAIN = 51,
    MONUMENT_TEXT_STAGING_AREA_BLOCKED = 52,
    MONUMENT_TEXT_SUN_TEMPLE_BEGIN = 53,
    MONUMENT_TEXT_SUN_TEMPLE_SCAFFOLDING = 54,
    MONUMENT_TEXT_SUN_TEMPLE_OBELISK = 55,
    MONUMENT_TEXT_SUN_TEMPLE_VESTIBULE = 56,
    MONUMENT_TEXT_SUN_TEMPLE_COMPLETE = 57,
    MONUMENT_TEXT_MAUSOLEUM_BEGIN = 58,
    MONUMENT_TEXT_MAUSOLEUM_WORKING = 59,
    MONUMENT_TEXT_MAUSOLEUM_COMPLETE = 60,
    MONUMENT_TEXT_PERCENT_COMPLETE = 61,
    MONUMENT_TEXT_NO_COURSES = 62,
    MONUMENT_TEXT_ONE_COURSE = 63,
    MONUMENT_TEXT_COURSES = 64,
    MONUMENT_TEXT_COURSE_NEEDS_MAIN = 65,
    MONUMENT_TEXT_COURSE_NEEDS_CASING = 66,
    MONUMENT_TEXT_COURSE_NEEDS_BOTH = 67,
    MONUMENT_TEXT_REMAINDER_NEEDS_MAIN = 68,
    MONUMENT_TEXT_REMAINDER_NEEDS_CASING = 69,
    MONUMENT_TEXT_REMAINDER_NEEDS_BOTH = 70,
    MONUMENT_TEXT_DONE_MAIN = 71,
    MONUMENT_TEXT_DONE_BOTH = 72,
    MONUMENT_TEXT_MASTABA = 73,
    MONUMENT_TEXT_PYRAMID = 74,
    MONUMENT_TEXT_STEPPED_PYRAMID = 75,
    MONUMENT_TEXT_BENT_PYRAMID = 76,
    MONUMENT_TEXT_BRICK_PYRAMID = 77,
    MONUMENT_TEXT_OBELISK = 78,
    MONUMENT_TEXT_SPHINX = 79,
    MONUMENT_TEXT_SUN_TEMPLE = 80,
    MONUMENT_TEXT_LIBRARY = 81,
    MONUMENT_TEXT_ABU_SIMBEL = 82,
    MONUMENT_TEXT_SMALL_ROYAL_TOMB = 83,
    MONUMENT_TEXT_MEDIUM_ROYAL_TOMB = 84,
    MONUMENT_TEXT_LARGE_ROYAL_TOMB = 85,
    MONUMENT_TEXT_GRAND_ROYAL_TOMB = 86,
    MONUMENT_TEXT_CAESAREUM = 87,
    MONUMENT_TEXT_LIGHTHOUSE = 88,
    MONUMENT_TEXT_MAUSOLEUM = 89,
    MONUMENT_TEXT_PLAIN_STONE_BLOCKS = 90,
    MONUMENT_TEXT_LIMESTONE_BLOCKS = 91,
    MONUMENT_TEXT_GRANITE_BLOCKS = 92,
    MONUMENT_TEXT_SANDSTONE_BLOCKS = 93,
    MONUMENT_TEXT_BRICKS = 94,
    MONUMENT_TEXT_MARBLE_BLOCKS = 95,
    MONUMENT_TEXT_COPPER_INGOTS = 96,
    MONUMENT_TEXT_MAUSOLEUM_NEEDS_CARPENTER = 97,
    MONUMENT_TEXT_STONE_STOCKPILED = 98,
    MONUMENT_TEXT_LIMESTONE_STOCKPILED = 99,
    MONUMENT_TEXT_SANDSTONE_STOCKPILED = 100,
    MONUMENT_TEXT_MARBLE_STOCKPILED = 101,
    MONUMENT_TEXT_GRANITE_STOCKPILED = 102,
    MONUMENT_TEXT_BRICKS_STOCKPILED = 103,
    MONUMENT_TEXT_COPPER_STOCKPILED = 104,
    MONUMENT_TEXT_NO_ARTISANS = 105,
    MONUMENT_TEXT_ARTISANS_NOT_COMING = 106,
    MONUMENT_TEXT_LIBRARY_CLEARING = 114,
    MONUMENT_TEXT_CAESAREUM_CLEARING = 122,
    MONUMENT_TEXT_LIGHTHOUSE_CLEARING = 132,
    MONUMENT_TEXT_LIGHTHOUSE_COMPLETE = 140,
    MONUMENT_TEXT_ABU_SIMBEL_WORKING = 141,
    MONUMENT_TEXT_ABU_SIMBEL_FIGURES = 142,
    MONUMENT_TEXT_TOMB_REFUSED = 143,
    MONUMENT_TEXT_TOMB_WORKING = 144,
    MONUMENT_TEXT_TOMB_COMPLETE = 145,
    MONUMENT_TEXT_TOMB_READY = 146,
    MONUMENT_TEXT_LAMPS = 147,
    MONUMENT_TEXT_TOMB_ENTRANCE_BLOCKED = 148,
    MONUMENT_TEXT_MONUMENT_BLOCKED = 149,
};

enum monument_layout {
    MONUMENT_TEXT_X = 32,
    MONUMENT_TEXT_Y = 56,
    MONUMENT_REQUIREMENTS_Y = 80,
    MONUMENT_LINE_HEIGHT = 24,
    MONUMENT_TEXT_WIDTH_PER_BLOCK = 15,
    TOMB_LAMPS_Y = 60,
    TOMB_LAMPS_AMOUNT_X = 60,
    TOMB_LAMPS_WIDTH_PER_BLOCK = 6,
    EXPANDED_TEXT_SIZE = 500,
    COURSES_LINE_SIZE = 1000,
};

enum monument_values {
    PYRAMID_FIRST_SITE_LEVEL = 2,
    PYRAMID_LAST_EARLY_SITE_LEVEL = 5,
    PYRAMID_LAST_SITE_LEVEL = 11,
    MASTABA_LAST_GROOVE_LEVEL = 10,
    SPHINX_STAGE_COMPLETE = 16,
    SPHINX_STAGE_FINE_CARVING = 7,
    SPHINX_STAGE_FINISHING = 13,
    MATERIAL_LOADS_PER_BLOCK = 400,
    SANDSTONE_BLOCKS_PER_MASON = 5,
    SANDSTONE_LOADS_PER_BLOCK = 80,
    LIBRARY_MARBLE_STAGE_END = 21,
    LIBRARY_COPPER_STAGE_END = 19,
    LIBRARY_COPPER_STOCK = 800,
    CAESAREUM_MARBLE_STAGE_END = 18,
    CAESAREUM_GRANITE_STAGE_END = 19,
    CAESAREUM_MARBLE_STOCK = 800,
    LIGHTHOUSE_STAGE_DONE = 9,
    ABU_SIMBEL_FIGURES_STAGE = 0x38,
};

pyramid_style_rule pyramid_style_rules[PYRAMID_STYLE_RULE_COUNT] = {
    [PYRAMID_STYLE_RULE_OTHER] = { 38, RESOURCE_STONE, RESOURCE_LIMESTONE, 0, 1 },
    [PYRAMID_STYLE_RULE_BENT] = { 54, RESOURCE_STONE, RESOURCE_LIMESTONE, 1, 0 },
    [PYRAMID_STYLE_RULE_MUDBRICK] = { 55, RESOURCE_BRICKS, RESOURCE_LIMESTONE, 1, 1 },
    [PYRAMID_STYLE_RULE_STEPPED] = { 375, RESOURCE_STONE, -1, 0, 0 },
    [PYRAMID_STYLE_RULE_PYRAMID] = { 38, RESOURCE_STONE, RESOURCE_LIMESTONE, 1, 0 },
    [PYRAMID_STYLE_RULE_MASTABA] = { 371, RESOURCE_BRICKS, -1, 0, 1 },
};

library_info_report bi_library_report;

static const pyramid_progress *pyramid_report(void)
{
    return &bi_progress_pyramid;
}

static const int32_t *sun_temple_report(void)
{
    return bi_progress_sun_temple;
}

static const library_info_progress *library_report(void)
{
    return &bi_library_report.progress;
}

static const library_info_stock *library_stock(void)
{
    return &bi_library_report.stock;
}

static const scripted_monument_info_progress *caesareum_report(void)
{
    return &bi_progress_caesareum;
}

static const scripted_monument_info_progress *lighthouse_report(void)
{
    return &bi_progress_lighthouse;
}

static const abu_simbel_info_progress *abu_simbel_report(void)
{
    return &bi_progress_abu_simbel;
}

static const int32_t *tomb_report(void)
{
    return bi_progress_tomb;
}

static const mausoleum_info_progress *mausoleum_report(void)
{
    return &bi_progress_mausoleum;
}

static monument_text_resources *text_resources(void)
{
    return &bi_monument_resources;
}

static int32_t built(int32_t type)
{
    return building_count_total[type] != 0;
}

static int32_t working(int32_t type)
{
    return building_count_active[type] != 0;
}

static int32_t text_width(void)
{
    return info_w_blocks * BUILDING_INFO_BLOCK - BUILDING_INFO_BLOCK;
}

static void draw_panel_title(void)
{
    window_building_draw_panel_title(LANG_GROUP_MONUMENTS, MONUMENT_TEXT_FOREMAN);
}

static void start_monument_panel(int32_t help)
{
    info_help_id = building_info_help_messages[help];
    info_first_draw = 0;
    draw_panel_title();
}

static void count_text_drawn(int32_t index)
{
    (void) index;
}

static void draw_text(int32_t index, int32_t x, int32_t y)
{
    count_text_drawn(index);
    lang_text_draw_multiline_ui(
        LANG_GROUP_MONUMENTS, index, x, y, info_w_blocks * MONUMENT_TEXT_WIDTH_PER_BLOCK, image_group(IMAGE_GROUP_FONT_BLACK), 0, building_info_x, building_info_y);
}

static void draw_label(int32_t index, int32_t x, int32_t y)
{
    lang_text_draw_ui(LANG_GROUP_MONUMENTS, index, x, y, image_group(IMAGE_GROUP_FONT_BLACK), 0, building_info_x, building_info_y);
}

static void draw_percent_complete(int32_t percentage, int32_t x, int32_t y)
{
    text_draw_number_ui(percentage, NUMBER_PREFIX_NONE, pct_postfix, g_text_width + x, y, image_group(IMAGE_GROUP_FONT_BLACK), 0, building_info_x, building_info_y);
    draw_label(MONUMENT_TEXT_COMPLETE, g_text_width + x, y);
}

static void expand_text(int32_t index, char *buffer, int32_t size)
{
    event_text_expand(lang_get_string(LANG_GROUP_MONUMENTS, index), buffer, size);
}

static void draw_expanded(int32_t index, int32_t x, int32_t y, int32_t width)
{
    char buffer[EXPANDED_TEXT_SIZE];
    count_text_drawn(index);
    expand_text(index, buffer, EXPANDED_TEXT_SIZE);
    text_draw_multiline(buffer, x, y, width, image_group(IMAGE_GROUP_FONT_BLACK), 0);
}

static void draw_expanded_below(int32_t index, int32_t x, int32_t y, int32_t width)
{
    char buffer[EXPANDED_TEXT_SIZE];
    count_text_drawn(index);
    expand_text(index, buffer, EXPANDED_TEXT_SIZE);
    text_draw_multiline(buffer, x, g_draw_y_accum + y, width, image_group(IMAGE_GROUP_FONT_BLACK), 0);
}

static int32_t road_tile_blocked(int32_t building_id)
{
    const building *b = building_get(building_id);
    return routing_grids.land_citizen[map_grid_offset(b->road_access_x, b->road_access_y)] < 0;
}

static int32_t no_peasants_text(void)
{
    return floods_is_running(&floods_data) ? MONUMENT_TEXT_PEASANTS_ON_FLOODPLAIN : MONUMENT_TEXT_NO_PEASANTS;
}

static int32_t material_shortage_text(int32_t resource)
{
    int32_t stockpiled = city_player_current()->stockpiled[resource] != 0;
    switch (resource) {
        case RESOURCE_STONE:
            return stockpiled ? MONUMENT_TEXT_STONE_STOCKPILED : MONUMENT_TEXT_NEEDS_STONE;
        case RESOURCE_LIMESTONE:
            return stockpiled ? MONUMENT_TEXT_LIMESTONE_STOCKPILED : MONUMENT_TEXT_NEEDS_LIMESTONE;
        case RESOURCE_BRICKS:
            return stockpiled ? MONUMENT_TEXT_BRICKS_STOCKPILED : MONUMENT_TEXT_NEEDS_BRICKS;
        case RESOURCE_SANDSTONE:
            return stockpiled ? MONUMENT_TEXT_SANDSTONE_STOCKPILED : MONUMENT_TEXT_NEEDS_SANDSTONE;
        case RESOURCE_MARBLE:
            return stockpiled ? MONUMENT_TEXT_MARBLE_STOCKPILED : MONUMENT_TEXT_NEEDS_MARBLE;
        case RESOURCE_GRANITE:
            return stockpiled ? MONUMENT_TEXT_GRANITE_STOCKPILED : MONUMENT_TEXT_NEEDS_GRANITE;
        default:
            return stockpiled ? MONUMENT_TEXT_COPPER_STOCKPILED : MONUMENT_TEXT_NEEDS_COPPER;
    }
}

static int32_t monument_type_name(int32_t type)
{
    switch (type) {
        case BUILDING_MAUSOLEUM:
            return MONUMENT_TEXT_MAUSOLEUM;
        case BUILDING_ALEXANDRIA_LIBRARY:
            return MONUMENT_TEXT_LIBRARY;
        case BUILDING_LIGHTHOUSE:
            return MONUMENT_TEXT_LIGHTHOUSE;
        case BUILDING_SMALL_ROYAL_TOMB:
            return MONUMENT_TEXT_SMALL_ROYAL_TOMB;
        case BUILDING_MEDIUM_ROYAL_TOMB:
            return MONUMENT_TEXT_MEDIUM_ROYAL_TOMB;
        case BUILDING_LARGE_ROYAL_TOMB:
            return MONUMENT_TEXT_LARGE_ROYAL_TOMB;
        case BUILDING_GRAND_ROYAL_TOMB:
            return MONUMENT_TEXT_GRAND_ROYAL_TOMB;
        default:
            return MONUMENT_TEXT_COMPLETE;
    }
}

static int32_t pyramid_style_name(int32_t style)
{
    switch (style) {
        case PYRAMID_STYLE_BENT:
            return MONUMENT_TEXT_BENT_PYRAMID;
        case PYRAMID_STYLE_MUDBRICK:
            return MONUMENT_TEXT_BRICK_PYRAMID;
        case PYRAMID_STYLE_STEPPED:
            return MONUMENT_TEXT_STEPPED_PYRAMID;
        case PYRAMID_STYLE_PYRAMID:
            return MONUMENT_TEXT_PYRAMID;
        case PYRAMID_STYLE_MASTABA:
            return MONUMENT_TEXT_MASTABA;
        default:
            return -1;
    }
}

static int32_t monument_def_name(int32_t subkind)
{
    switch (subkind) {
        case MONUMENT_DEF_SPHINX:
            return MONUMENT_TEXT_SPHINX;
        case MONUMENT_DEF_OBELISK:
            return MONUMENT_TEXT_OBELISK;
        case MONUMENT_DEF_SUN_TEMPLE:
            return MONUMENT_TEXT_SUN_TEMPLE;
        case MONUMENT_DEF_MAUSOLEUM:
            return MONUMENT_TEXT_MAUSOLEUM;
        case MONUMENT_DEF_LIGHTHOUSE:
            return MONUMENT_TEXT_LIGHTHOUSE;
        case MONUMENT_DEF_LIBRARY:
            return MONUMENT_TEXT_LIBRARY;
        case MONUMENT_DEF_CAESAREUM:
            return MONUMENT_TEXT_CAESAREUM;
        case MONUMENT_DEF_SMALL_ROYAL_TOMB:
            return MONUMENT_TEXT_SMALL_ROYAL_TOMB;
        case MONUMENT_DEF_MEDIUM_ROYAL_TOMB:
            return MONUMENT_TEXT_MEDIUM_ROYAL_TOMB;
        case MONUMENT_DEF_LARGE_ROYAL_TOMB:
            return MONUMENT_TEXT_LARGE_ROYAL_TOMB;
        case MONUMENT_DEF_GRAND_ROYAL_TOMB:
            return MONUMENT_TEXT_GRAND_ROYAL_TOMB;
        case MONUMENT_DEF_ABU_SIMBEL:
            return MONUMENT_TEXT_ABU_SIMBEL;
        default:
            return -1;
    }
}

const char *window_building_monument_name(int32_t building_id)
{
    int32_t main_id = building_main(building_id);
    const building *main = building_get(main_id);
    if (main->created_sequence == MONUMENT_KEPT_FROM_EARLIER_MISSION) {
        return lang_get_string(LANG_GROUP_MONUMENTS, monument_type_name(main->type));
    }
    int32_t slot = 0;
    while (city_player_current()->monument_building_id[slot] != main_id) {
        if (++slot >= MONUMENT_SLOTS) {
            return 0;
        }
    }
    int32_t def = scenario_data.monument_def_ids[slot];
    if (def < 0) {
        return 0;
    }
    int32_t index = (monument_defs[def].kind == MONUMENT_DEF_KIND_PYRAMID_FAMILY) ? pyramid_style_name(scenario_pyramid_style)
                                                                                  : monument_def_name(monument_defs[def].subkind);
    return (index < 0) ? 0 : lang_get_string(LANG_GROUP_MONUMENTS, index);
}

static int32_t material_name(int32_t resource)
{
    switch (resource) {
        case RESOURCE_BRICKS:
            return MONUMENT_TEXT_BRICKS;
        case RESOURCE_STONE:
            return MONUMENT_TEXT_PLAIN_STONE_BLOCKS;
        case RESOURCE_LIMESTONE:
            return MONUMENT_TEXT_LIMESTONE_BLOCKS;
        case RESOURCE_GRANITE:
            return MONUMENT_TEXT_GRANITE_BLOCKS;
        case RESOURCE_COPPER:
            return MONUMENT_TEXT_COPPER_INGOTS;
        case RESOURCE_SANDSTONE:
            return MONUMENT_TEXT_SANDSTONE_BLOCKS;
        case RESOURCE_MARBLE:
            return MONUMENT_TEXT_MARBLE_BLOCKS;
        default:
            return -1;
    }
}

const char *window_building_monument_resource_name(int32_t resource)
{
    int32_t index = material_name(resource);
    return (index < 0) ? 0 : lang_get_string(LANG_GROUP_MONUMENTS, index);
}

static const pyramid_style_rule *pyramid_style_rule_of(int32_t style)
{
    return &pyramid_style_rules[(style >= PYRAMID_STYLE_BENT && style <= PYRAMID_STYLE_MASTABA) ? style : PYRAMID_STYLE_RULE_OTHER];
}

static int32_t monument_def_of_info_building(void)
{
    int32_t main_id = building_main(building_info_building_id);
    for (int32_t slot = 0; slot < MONUMENT_SLOTS; slot++) {
        if (city_player_current()->monument_building_id[slot] == main_id) {
            return scenario_data.monument_def_ids[slot];
        }
    }
    return 0;
}

static int32_t no_road_to_staging_area(int32_t building_id)
{
    return road_tile_blocked(building_id) || building_get(building_id)->distance_from_entry == 0;
}

static int32_t pyramid_site_text(const pyramid_progress *report, int32_t style)
{
    if (!built(BUILDING_WORK_CAMP)) {
        return MONUMENT_TEXT_NO_WORK_CAMP;
    }
    if (!working(BUILDING_WORK_CAMP) && report->laborers == 0) {
        return no_peasants_text();
    }
    int32_t level = report->prep_max_stage;
    if (level == report->prep_min_stage) {
        level++;
    }
    if (style == PYRAMID_STYLE_MASTABA && level > PYRAMID_LAST_EARLY_SITE_LEVEL && level <= MASTABA_LAST_GROOVE_LEVEL) {
        level = PYRAMID_LAST_EARLY_SITE_LEVEL;
    }
    if (level >= PYRAMID_FIRST_SITE_LEVEL && level <= PYRAMID_LAST_EARLY_SITE_LEVEL) {
        return MONUMENT_TEXT_CLEARING_PYRAMID_SITE + (style == PYRAMID_STYLE_MASTABA);
    }
    if (level > PYRAMID_LAST_EARLY_SITE_LEVEL && level <= PYRAMID_LAST_SITE_LEVEL) {
        return level - 1;
    }
    return MONUMENT_TEXT_ROOF;
}

static int32_t guild_serves(int32_t guild_type, int32_t def)
{
    return (uint8_t) guild_serves_monument_def(guild_type, def) != 0;
}

static int32_t has_ramp_work(void)
{
    return (uint8_t) pyramid_has_ramp_work(building_main(building_info_building_id)) != 0;
}

static int32_t pyramid_needs_available(int32_t resource, int32_t amount)
{
    uint8_t available = monument_resource_available(resource, amount);
    return (uint8_t) monument_needs_resource(building_main(building_info_building_id), resource) != 0 && available;
}

static int32_t pyramid_blocks_text(const pyramid_progress *report, const pyramid_style_rule *rule, int32_t def, int32_t x, int32_t y, int32_t *text_y)
{
    g_draw_y_accum = 0;
    int32_t main_id = building_main(building_info_building_id);
    pyramid_progress copy;
    memcpy(&copy, &bi_progress_pyramid, sizeof(copy));
    uint8_t can_build = pyramid_can_build_next(def, copy, main_id);
    int32_t stonemasons_tested = !rule->bricklayers || rule->stonemasons;
    if (!built(BUILDING_WORK_CAMP)) {
        return MONUMENT_TEXT_NO_WORK_CAMP;
    }
    if (!built(BUILDING_STONEMASONS_GUILD) && stonemasons_tested && guild_serves(BUILDING_STONEMASONS_GUILD, def)) {
        return MONUMENT_TEXT_NO_STONEMASONS;
    }
    if (!built(BUILDING_BRICKLAYERS_GUILD) && rule->bricklayers && guild_serves(BUILDING_BRICKLAYERS_GUILD, def)) {
        return MONUMENT_TEXT_NO_BRICKLAYERS;
    }
    if (report->block_workers == report->waiting && !can_build) {
        return MONUMENT_TEXT_NO_PEASANTS;
    }
    int32_t masons_wanted = (!rule->bricklayers && report->frontier_main != 0) || (rule->stonemasons && report->frontier_casing != 0);
    if (report->stonemasons == 0 && masons_wanted && guild_serves(BUILDING_STONEMASONS_GUILD, def)) {
        return MONUMENT_TEXT_STONEMASONS_NOT_COMING;
    }
    if (report->bricklayers == 0 && rule->bricklayers && report->frontier_main != 0 && guild_serves(BUILDING_BRICKLAYERS_GUILD, def)) {
        return MONUMENT_TEXT_BRICKLAYERS_NOT_COMING;
    }
    if (!built(BUILDING_CARPENTERS_GUILD) && has_ramp_work() && guild_serves(BUILDING_CARPENTERS_GUILD, def)) {
        return MONUMENT_TEXT_NO_CARPENTERS;
    }
    if (report->carpenters == 0 && has_ramp_work() && guild_serves(BUILDING_CARPENTERS_GUILD, def)) {
        return MONUMENT_TEXT_WAITING_FOR_CARPENTER;
    }
    if (report->waiting == 0) {
        return -1;
    }
    if (pyramid_needs_available(rule->main_material, report->material_main)) {
        draw_text(material_shortage_text(rule->main_material), x, g_draw_y_accum + y);
    }
    if (rule->casing_material != -1 && pyramid_needs_available(rule->casing_material, report->material_casing)) {
        *text_y = g_draw_y_accum + y;
        return material_shortage_text(rule->casing_material);
    }
    return -1;
}

static int32_t pyramid_finishing_text(const pyramid_progress *report, const pyramid_style_rule *rule, int32_t def, int32_t x, int32_t *y)
{
    if (!built(BUILDING_STONEMASONS_GUILD) && (!rule->bricklayers || rule->stonemasons) && guild_serves(BUILDING_STONEMASONS_GUILD, def)) {
        return MONUMENT_TEXT_NO_STONEMASONS;
    }
    if (report->stonemasons == 0 && guild_serves(BUILDING_STONEMASONS_GUILD, def)) {
        return MONUMENT_TEXT_STONEMASONS_NOT_COMING;
    }
    g_text_width = 0;
    draw_label(MONUMENT_TEXT_FINISHING_OUTSIDE, x, *y);
    *y += MONUMENT_LINE_HEIGHT;
    g_text_width = 0;
    draw_label(MONUMENT_TEXT_TASK_IS, x, *y);
    draw_percent_complete(report->phase_percent, x, *y);
    return -1;
}

static void draw_pyramid_courses(const pyramid_progress *report, int32_t x, int32_t y, int32_t width)
{
    char line[COURSES_LINE_SIZE];
    char second[EXPANDED_TEXT_SIZE];
    int32_t courses = (report->height == 0) ? MONUMENT_TEXT_NO_COURSES : (report->height == 1) ? MONUMENT_TEXT_ONE_COURSE : MONUMENT_TEXT_COURSES;
    expand_text(courses, line, COURSES_LINE_SIZE);
    int32_t needs = (report->frontier_main != 0) ? ((report->frontier_casing != 0) ? MONUMENT_TEXT_COURSE_NEEDS_BOTH : MONUMENT_TEXT_COURSE_NEEDS_MAIN)
                                                 : MONUMENT_TEXT_COURSE_NEEDS_CASING;
    expand_text(needs, second, EXPANDED_TEXT_SIZE);
    strcat(line, second);
    text_draw_multiline(line, x, g_draw_y_accum + y, width, image_group(IMAGE_GROUP_FONT_BLACK), 0);
}

static int32_t pyramid_remainder_text(const pyramid_progress *report)
{
    if (report->percent >= MONUMENT_PROGRESS_COMPLETE) {
        return (report->casing_total != 0) ? MONUMENT_TEXT_DONE_BOTH : MONUMENT_TEXT_DONE_MAIN;
    }
    if (report->main_remaining != 0) {
        return (report->casing_remaining != 0) ? MONUMENT_TEXT_REMAINDER_NEEDS_BOTH : MONUMENT_TEXT_REMAINDER_NEEDS_MAIN;
    }
    return (report->casing_remaining != 0) ? MONUMENT_TEXT_REMAINDER_NEEDS_CASING : -1;
}

void window_building_draw_pyramid(void)
{
    const pyramid_progress *report = pyramid_report();
    int32_t style = scenario_pyramid_style;
    const pyramid_style_rule *rule = pyramid_style_rule_of(style);
    info_help_id = rule->help_message;
    text_resources()->first = rule->main_material;
    text_resources()->second = rule->casing_material;
    info_first_draw = 0;
    outer_panel_draw(building_info_x, building_info_y, info_w_blocks, building_info_h);
    int32_t def = monument_def_of_info_building();
    int32_t no_road = no_road_to_staging_area(building_info_building_id);
    lang_text_draw_centered_ui(LANG_GROUP_MONUMENTS, MONUMENT_TEXT_FOREMAN, building_info_x, building_info_y + BUILDING_INFO_TITLE_Y,
        info_w_blocks * BUILDING_INFO_BLOCK, image_group(IMAGE_GROUP_FONT_LARGE), 0, building_info_x, building_info_y);
    int32_t x = building_info_x + MONUMENT_TEXT_X;
    int32_t y = building_info_y + MONUMENT_TEXT_Y;
    int32_t text = -1;
    int32_t text_y = -1;
    if (no_road && report->percent < MONUMENT_PROGRESS_COMPLETE) {
        y += MONUMENT_LINE_HEIGHT;
        g_text_width = 0;
        g_draw_y_accum = 0;
        text = MONUMENT_TEXT_STAGING_AREA_BLOCKED;
    } else {
        g_draw_y_accum = 0;
        switch (report->phase) {
            case PYRAMID_PHASE_SITE_PREPARATION:
                text = pyramid_site_text(report, style);
                break;
            case PYRAMID_PHASE_BLOCKS:
                text = pyramid_blocks_text(report, rule, def, x, y, &text_y);
                break;
            case PYRAMID_PHASE_FINISHING:
                text = pyramid_finishing_text(report, rule, def, x, &y);
                break;
        }
    }
    if (text >= 0) {
        draw_text(text, x, (text_y == -1) ? y : text_y);
    }
    y += g_draw_y_accum + MONUMENT_LINE_HEIGHT;
    int32_t width = text_width();
    g_draw_y_accum = 0;
    if (report->percent < MONUMENT_PROGRESS_COMPLETE) {
        draw_expanded_below(MONUMENT_TEXT_PERCENT_COMPLETE, x, y, width);
    }
    if (report->frontier_main != 0 || report->frontier_casing != 0) {
        draw_pyramid_courses(report, x, y, width);
    }
    int32_t remainder = pyramid_remainder_text(report);
    if (remainder >= 0) {
        draw_expanded_below(remainder, x, y, width);
    }
}

static int32_t sphinx_text(int32_t carpenters_assigned, int32_t stonemasons_assigned, uint8_t carpenter_working, uint8_t stonemason_working, int32_t no_road)
{
    if (no_road) {
        return MONUMENT_TEXT_MONUMENT_BLOCKED;
    }
    if (!built(BUILDING_STONEMASONS_GUILD)) {
        return MONUMENT_TEXT_NO_STONEMASONS;
    }
    if (!built(BUILDING_CARPENTERS_GUILD)) {
        return MONUMENT_TEXT_NO_CARPENTERS;
    }
    if (stonemasons_assigned == 0 && !stonemason_working && sphinx_has_free_spot_for_type(FIGURE_STONEMASON) != 0) {
        return MONUMENT_TEXT_STONEMASONS_NOT_COMING;
    }
    if (carpenters_assigned == 0 && !carpenter_working && sphinx_has_free_spot_for_type(FIGURE_CARPENTER) != 0) {
        return MONUMENT_TEXT_CARPENTERS_NOT_COMING;
    }
    int16_t stage = city_player_current()->sphinx_stage;
    if (stage < SPHINX_STAGE_FINE_CARVING) {
        return MONUMENT_TEXT_SPHINX_ROUGH_SHAPE;
    }
    return (stage < SPHINX_STAGE_FINISHING) ? MONUMENT_TEXT_SPHINX_FINE_CARVING : MONUMENT_TEXT_SPHINX_FINISHING;
}

static void draw_work_line(int32_t label, int32_t percentage, int32_t x, int32_t y)
{
    g_text_width = 0;
    draw_text(label, x, y);
    draw_percent_complete(percentage, x, y);
}

void window_building_draw_sphinx(void)
{
    start_monument_panel(BUILDING_INFO_HELP_SPHINX);
    int32_t top = building_info_y;
    int32_t x = building_info_x + MONUMENT_TEXT_X;
    int32_t y = building_info_y + MONUMENT_TEXT_Y;
    int32_t carpenters_assigned;
    int32_t stonemasons_assigned;
    sphinx_count_assigned(FIGURE_CARPENTER, &carpenters_assigned);
    sphinx_count_assigned(FIGURE_STONEMASON, &stonemasons_assigned);
    uint8_t carpenter_working = (uint8_t) sphinx_figure_is_working_on(FIGURE_CARPENTER);
    uint8_t stonemason_working = (uint8_t) sphinx_figure_is_working_on(FIGURE_STONEMASON);
    int32_t no_road = no_road_to_staging_area(building_info_building_id);
    g_draw_y_accum = 0;
    int32_t complete = city_player_current()->sphinx_stage >= SPHINX_STAGE_COMPLETE;
    if (!complete) {
        draw_text(sphinx_text(carpenters_assigned, stonemasons_assigned, carpenter_working, stonemason_working, no_road), x, y);
    }
    int32_t lines_y = top + MONUMENT_REQUIREMENTS_Y + g_draw_y_accum;
    if (complete) {
        draw_label(MONUMENT_TEXT_SPHINX_COMPLETE, x, lines_y);
        return;
    }
    int32_t carpentry_percent = 0;
    int32_t mason_percent = 0;
    int32_t carpenters;
    int32_t carpenters_done;
    int32_t masons;
    int32_t masons_done;
    g_draw_y_accum = 0;
    sphinx_get_work_percentages(&carpentry_percent, &mason_percent, &carpenters, &carpenters_done, &masons, &masons_done);
    draw_work_line(MONUMENT_TEXT_CARPENTRY_IS, carpentry_percent, x, g_draw_y_accum + lines_y);
    draw_work_line(MONUMENT_TEXT_MASON_WORK_IS, mason_percent, x, g_draw_y_accum + lines_y);
}

static int32_t obelisk_waiting_text(int32_t carpenters_assigned, int32_t stonemasons_assigned, uint8_t carpenter_working, uint8_t stonemason_working,
    uint8_t stonemason_spot_free, uint8_t carpenter_spot_free)
{
    if (!built(BUILDING_STONEMASONS_GUILD)) {
        return MONUMENT_TEXT_NO_STONEMASONS;
    }
    if (!built(BUILDING_CARPENTERS_GUILD)) {
        return MONUMENT_TEXT_NO_CARPENTERS;
    }
    if (stonemasons_assigned == 0 && !stonemason_working && stonemason_spot_free) {
        return MONUMENT_TEXT_STONEMASONS_NOT_COMING;
    }
    if (carpenters_assigned == 0 && !carpenter_working && carpenter_spot_free) {
        return MONUMENT_TEXT_CARPENTERS_NOT_COMING;
    }
    return -1;
}

void window_building_draw_obelisk(void)
{
    info_help_id = building_info_help_messages[BUILDING_INFO_HELP_OBELISK];
    int32_t x = building_info_x + MONUMENT_TEXT_X;
    int32_t y = building_info_y + MONUMENT_TEXT_Y;
    info_first_draw = 0;
    draw_panel_title();
    int32_t id = building_info_building_id;
    int32_t carpenters_assigned;
    int32_t stonemasons_assigned;
    obelisk_count_assigned_figures(FIGURE_CARPENTER, &carpenters_assigned, id);
    obelisk_count_assigned_figures(FIGURE_STONEMASON, &stonemasons_assigned, building_info_building_id);
    uint8_t carpenter_working = (uint8_t) obelisk_figure_is_working_on(FIGURE_CARPENTER, building_info_building_id);
    uint8_t stonemason_working = (uint8_t) obelisk_figure_is_working_on(FIGURE_STONEMASON, building_info_building_id);
    uint8_t stonemason_spot_free = (uint8_t) obelisk_has_free_spot_for_type(FIGURE_STONEMASON, building_info_building_id);
    uint8_t carpenter_spot_free = (uint8_t) obelisk_has_free_spot_for_type(FIGURE_CARPENTER, building_info_building_id);
    int32_t text = obelisk_waiting_text(
        carpenters_assigned, stonemasons_assigned, carpenter_working, stonemason_working, stonemason_spot_free, carpenter_spot_free);
    if (text >= 0) {
        draw_text(text, x, y);
        return;
    }
    if (building_info_building()->data.obelisk.progress >= MONUMENT_PROGRESS_COMPLETE) {
        draw_label(MONUMENT_TEXT_OBELISK_COMPLETE, x, y);
        return;
    }
    int32_t carpenters_only = !(stonemasons_assigned != 0 || stonemason_working) && (carpenters_assigned != 0 || carpenter_working);
    g_draw_y_accum = 0;
    draw_text(carpenters_only ? MONUMENT_TEXT_OBELISK_CARPENTERS_FINISHING : MONUMENT_TEXT_OBELISK_CARVING, x, y);
    int32_t width = info_w_blocks * BUILDING_INFO_BLOCK;
    y += g_draw_y_accum;
    g_draw_y_accum = 0;
    draw_expanded(MONUMENT_TEXT_PERCENT_COMPLETE, x, y, width - BUILDING_INFO_BLOCK);
}

static void draw_requirements_or_done(int32_t percent, int32_t material, int32_t x, int32_t y, int32_t width)
{
    if (percent < MONUMENT_PROGRESS_COMPLETE) {
        draw_expanded(MONUMENT_TEXT_PERCENT_COMPLETE, x, y, width);
        y += g_draw_y_accum;
        if (material != 0) {
            g_draw_y_accum = 0;
            draw_expanded(MONUMENT_TEXT_REMAINDER_NEEDS_MAIN, x, y, width);
        }
    } else {
        draw_expanded(MONUMENT_TEXT_DONE_MAIN, x, y, width);
    }
}

static int32_t sandstone_available(int32_t stonemasons_waiting)
{
    return monument_resource_available(
        RESOURCE_SANDSTONE, (stonemasons_waiting * SANDSTONE_BLOCKS_PER_MASON + SANDSTONE_BLOCKS_PER_MASON) * SANDSTONE_LOADS_PER_BLOCK);
}

static int32_t sun_temple_stone_text(uint8_t stonemason_spot_free, uint8_t carpenter_spot_free)
{
    const int32_t *report = sun_temple_report();
    if (stonemason_spot_free) {
        if (!built(BUILDING_STONEMASONS_GUILD)) {
            return MONUMENT_TEXT_NO_STONEMASONS;
        }
        if (!working(BUILDING_STONEMASONS_GUILD) && report[SUN_TEMPLE_PROGRESS_STONEMASONS] == 0) {
            return MONUMENT_TEXT_STONEMASONS_NOT_COMING;
        }
        return MONUMENT_TEXT_SUN_TEMPLE_OBELISK;
    }
    if (!carpenter_spot_free) {
        return MONUMENT_TEXT_SUN_TEMPLE_OBELISK;
    }
    if (!built(BUILDING_CARPENTERS_GUILD)) {
        return MONUMENT_TEXT_NO_CARPENTERS;
    }
    if (!working(BUILDING_CARPENTERS_GUILD) && report[SUN_TEMPLE_PROGRESS_CARPENTERS] == 0) {
        return MONUMENT_TEXT_CARPENTERS_NOT_COMING;
    }
    return MONUMENT_TEXT_SUN_TEMPLE_OBELISK;
}

static int32_t sun_temple_temple_text(uint32_t flags, int32_t sandstone_ok, int32_t y, int32_t *text_y)
{
    const int32_t *report = sun_temple_report();
    if (sun_temple_get_progress(building_info_building_id, 0) >= MONUMENT_PROGRESS_COMPLETE) {
        return MONUMENT_TEXT_SUN_TEMPLE_COMPLETE;
    }
    if (!built(BUILDING_WORK_CAMP)) {
        return MONUMENT_TEXT_NO_WORK_CAMP;
    }
    if (!built(BUILDING_STONEMASONS_GUILD)) {
        return MONUMENT_TEXT_NO_STONEMASONS;
    }
    if (!working(BUILDING_WORK_CAMP) && report[SUN_TEMPLE_PROGRESS_LABORERS] == 0 && !(flags & SUN_TEMPLE_KIND8_DONE)) {
        return MONUMENT_TEXT_NO_PEASANTS;
    }
    int32_t stonemasons = report[SUN_TEMPLE_PROGRESS_STONEMASONS];
    int32_t waiting = report[SUN_TEMPLE_PROGRESS_STONEMASONS_WAITING];
    if (stonemasons == waiting && !sandstone_ok) {
        return MONUMENT_TEXT_NEEDS_SANDSTONE;
    }
    if (stonemasons == 0 && (!(flags & SUN_TEMPLE_KIND1_DONE) || !(flags & SUN_TEMPLE_KIND2_DONE))) {
        return MONUMENT_TEXT_STONEMASONS_NOT_COMING;
    }
    if (waiting == 0 || !sandstone_ok || report[SUN_TEMPLE_PROGRESS_MATERIAL] == 0) {
        return MONUMENT_TEXT_SUN_TEMPLE_VESTIBULE;
    }
    *text_y = g_draw_y_accum + y;
    return material_shortage_text(RESOURCE_SANDSTONE);
}

void window_building_draw_sun_temple(void)
{
    const int32_t *report = sun_temple_report();
    start_monument_panel(BUILDING_INFO_HELP_SUN_TEMPLE);
    int32_t x = building_info_x + MONUMENT_TEXT_X;
    int32_t y = building_info_y + MONUMENT_TEXT_Y;
    const building *b = building_info_building();
    uint8_t carpenter_spot_free = (uint8_t) sun_temple_has_free_spot_for_type(FIGURE_CARPENTER, b->data.monument.stage);
    uint8_t stonemason_spot_free = (uint8_t) sun_temple_has_free_spot_for_type(FIGURE_STONEMASON, b->data.monument.stage);
    uint32_t flags = (uint32_t) b->data.monument.work_flags;
    int32_t sandstone_ok = sandstone_available(report[SUN_TEMPLE_PROGRESS_STONEMASONS_WAITING]);
    g_draw_y_accum = 0;
    int32_t text_y = y;
    int32_t text;
    switch (report[SUN_TEMPLE_PROGRESS_STATUS]) {
        case SUN_TEMPLE_STATUS_SITE:
            if (!built(BUILDING_WORK_CAMP)) {
                text = MONUMENT_TEXT_NO_WORK_CAMP;
            } else if (!working(BUILDING_WORK_CAMP) && report[SUN_TEMPLE_PROGRESS_LABORERS] == 0) {
                text = no_peasants_text();
            } else {
                text = MONUMENT_TEXT_SUN_TEMPLE_BEGIN;
            }
            break;
        case SUN_TEMPLE_STATUS_TEMPLE_STARTED:
            if (!built(BUILDING_CARPENTERS_GUILD)) {
                text = MONUMENT_TEXT_NO_CARPENTERS;
            } else if (!working(BUILDING_CARPENTERS_GUILD) && report[SUN_TEMPLE_PROGRESS_CARPENTERS] == 0) {
                text = MONUMENT_TEXT_CARPENTERS_NOT_COMING;
            } else {
                text = MONUMENT_TEXT_SUN_TEMPLE_SCAFFOLDING;
            }
            break;
        case SUN_TEMPLE_STATUS_TEMPLE_RISING:
            text = sun_temple_stone_text(stonemason_spot_free, carpenter_spot_free);
            break;
        case SUN_TEMPLE_STATUS_TEMPLE_DONE:
            text = sun_temple_temple_text(flags, sandstone_ok, y, &text_y);
            break;
        default:
            return;
    }
    draw_text(text, x, text_y);
    y += g_draw_y_accum;
    text_resources()->first = RESOURCE_SANDSTONE;
    g_draw_y_accum = 0;
    draw_requirements_or_done(report[SUN_TEMPLE_PROGRESS_PERCENT], report[SUN_TEMPLE_PROGRESS_MATERIAL], x, y, text_width());
}

static int32_t monument_blocked(int32_t building_id, int32_t recheck_id)
{
    if (road_tile_blocked(building_id)) {
        return 1;
    }
    if (building_get(building_id)->distance_from_entry == 0) {
        building_maintenance_check_kingdom_access();
        if (building_get(recheck_id)->distance_from_entry == 0) {
            return 1;
        }
    }
    return 0;
}

static int32_t guild_text(int32_t stonemasons_wanted, int32_t carpenters_wanted, int32_t stonemasons_delivered, int32_t carpenters_delivered)
{
    if (stonemasons_wanted) {
        if (!built(BUILDING_STONEMASONS_GUILD)) {
            return MONUMENT_TEXT_NO_STONEMASONS;
        }
        if (!working(BUILDING_STONEMASONS_GUILD) && stonemasons_delivered == 0) {
            return MONUMENT_TEXT_STONEMASONS_NOT_COMING;
        }
    }
    if (carpenters_wanted) {
        if (!built(BUILDING_CARPENTERS_GUILD)) {
            return MONUMENT_TEXT_NO_CARPENTERS;
        }
        if (!working(BUILDING_CARPENTERS_GUILD) && carpenters_delivered == 0) {
            return MONUMENT_TEXT_CARPENTERS_NOT_COMING;
        }
    }
    return 0;
}

static int32_t work_camp_text(int32_t laborers_delivered)
{
    if (!built(BUILDING_WORK_CAMP)) {
        return MONUMENT_TEXT_NO_WORK_CAMP;
    }
    if (!working(BUILDING_WORK_CAMP) && laborers_delivered == 0) {
        return MONUMENT_TEXT_NO_PEASANTS;
    }
    return 0;
}

static int32_t library_stage_text(int32_t stage)
{
    static const int32_t stage_ends[] = { 3, 6, 9, 12, 16, 20 };
    for (int32_t i = 0; i < (int32_t) (sizeof(stage_ends) / sizeof(stage_ends[0])); i++) {
        if (stage < stage_ends[i]) {
            return MONUMENT_TEXT_LIBRARY_CLEARING + i;
        }
    }
    return (stage == 20) ? MONUMENT_TEXT_LIBRARY_CLEARING + 6 : MONUMENT_TEXT_LIBRARY_CLEARING + 7;
}

void window_building_draw_great_library(void)
{
    const library_info_progress *report = library_report();
    const library_info_stock *stock = library_stock();
    const scripted_monument_info_progress *caesareum = caesareum_report();
    start_monument_panel(BUILDING_INFO_HELP_LIBRARY_MONUMENT);
    int32_t top = building_info_y;
    int32_t x = building_info_x + MONUMENT_TEXT_X;
    int32_t y = building_info_y + MONUMENT_TEXT_Y;
    int32_t stage = building_get(monument_main_part_id(building_info_building_id))->data.monument.stage;
    uint8_t carpenter_spot_free = library_has_free_spot_for_type(FIGURE_CARPENTER, stage);
    uint8_t stonemason_spot_free = library_has_free_spot_for_type(FIGURE_STONEMASON, stage);
    library_get_progress(&library_object, building_info_building_id, 0);
    uint8_t resource_ok = 0;
    if (stage < LIBRARY_MARBLE_STAGE_END) {
        resource_ok = monument_resource_available(RESOURCE_MARBLE, report->stonemasons_waiting * MATERIAL_LOADS_PER_BLOCK);
    } else if (stage < LIBRARY_COPPER_STAGE_END + 1) {
        resource_ok = monument_resource_available(RESOURCE_COPPER, report->stonemasons_waiting * MATERIAL_LOADS_PER_BLOCK);
    }
    g_draw_y_accum = 0;
    int32_t text = -1;
    if (monument_blocked(building_info_building_id, building_info_building_id)) {
        text = MONUMENT_TEXT_MONUMENT_BLOCKED;
    } else if (report->stonemasons_waiting != 0 && resource_ok && stock->material != 0) {
        if (stage < LIBRARY_MARBLE_STAGE_END) {
            text = material_shortage_text(RESOURCE_MARBLE);
            y += g_draw_y_accum;
        } else if (stage < LIBRARY_COPPER_STAGE_END) {
            text = material_shortage_text(RESOURCE_COPPER);
            y += g_draw_y_accum;
        }
    } else {
        text = 0;
        if (stonemason_spot_free) {
            text = work_camp_text(report->laborers);
            if (text == 0) {
                text = guild_text(1, 0, caesareum->stonemasons, 0);
            }
        }
        if (text == 0 && carpenter_spot_free) {
            text = guild_text(0, 1, 0, caesareum->carpenters);
        }
        if (text == 0) {
            text = library_stage_text(report->stage);
        }
    }
    if (text >= 0) {
        draw_text(text, x, y);
    }
    int32_t lines_y = top + MONUMENT_REQUIREMENTS_Y + g_draw_y_accum;
    int32_t width = text_width();
    text_resources()->first = RESOURCE_MARBLE;
    if (stock->percent >= MONUMENT_PROGRESS_COMPLETE) {
        return;
    }
    g_draw_y_accum = 0;
    draw_expanded(MONUMENT_TEXT_PERCENT_COMPLETE, x, lines_y, width);
    lines_y += g_draw_y_accum;
    if (stock->material != 0) {
        g_draw_y_accum = 0;
        draw_expanded(MONUMENT_TEXT_REMAINDER_NEEDS_MAIN, x, lines_y, width);
        lines_y += g_draw_y_accum;
    }
    if (stock->material > LIBRARY_COPPER_STOCK) {
        text_resources()->first = RESOURCE_COPPER;
        g_draw_y_accum = 0;
        draw_expanded(MONUMENT_TEXT_REMAINDER_NEEDS_MAIN, x, lines_y, width);
    }
}

void window_building_draw_abu_simbel(void)
{
    const scripted_monument_info_progress *caesareum = caesareum_report();
    start_monument_panel(BUILDING_INFO_HELP_ABU_SIMBEL);
    int32_t top = building_info_y;
    int32_t x = building_info_x + MONUMENT_TEXT_X;
    int32_t y = building_info_y + MONUMENT_TEXT_Y;
    int32_t stage = building_get(monument_main_part_id(building_info_building_id))->data.abu_simbel.stage;
    uint8_t carpenter_spot_free = abu_simbel_has_free_spot_for_type(FIGURE_CARPENTER, stage);
    uint8_t stonemason_spot_free = abu_simbel_has_free_spot_for_type(FIGURE_STONEMASON, stage);
    abu_simbel_get_progress(&abu_simbel_object, building_info_building_id, 0);
    g_draw_y_accum = 0;
    int32_t text;
    if (monument_blocked(building_info_building_id, building_info_building_id)) {
        text = MONUMENT_TEXT_MONUMENT_BLOCKED;
    } else {
        text = guild_text(stonemason_spot_free, carpenter_spot_free, caesareum->stonemasons, caesareum->carpenters);
        if (text == 0) {
            text = (abu_simbel_report()->stage < ABU_SIMBEL_FIGURES_STAGE) ? MONUMENT_TEXT_ABU_SIMBEL_WORKING : MONUMENT_TEXT_ABU_SIMBEL_FIGURES;
        }
    }
    draw_text(text, x, y);
    int32_t lines_y = top + MONUMENT_REQUIREMENTS_Y + g_draw_y_accum;
    if (abu_simbel_report()->percent < MONUMENT_PROGRESS_COMPLETE) {
        g_draw_y_accum = 0;
        draw_expanded(MONUMENT_TEXT_PERCENT_COMPLETE, x, lines_y, info_w_blocks * BUILDING_INFO_BLOCK - BUILDING_INFO_BLOCK);
    }
}

static int32_t caesareum_stage_text(int32_t stage)
{
    if (stage < 0) {
        return MONUMENT_TEXT_CAESAREUM_CLEARING;
    }
    if (stage == 0) {
        return MONUMENT_TEXT_CAESAREUM_CLEARING + 1;
    }
    static const int32_t stage_ends[] = { 6, 9, 12, 14, 17 };
    for (int32_t i = 0; i < (int32_t) (sizeof(stage_ends) / sizeof(stage_ends[0])); i++) {
        if (stage < stage_ends[i]) {
            return MONUMENT_TEXT_CAESAREUM_CLEARING + 2 + i;
        }
    }
    if (stage == 17) {
        return MONUMENT_TEXT_CAESAREUM_CLEARING + 7;
    }
    return (stage == 18) ? MONUMENT_TEXT_CAESAREUM_CLEARING + 8 : MONUMENT_TEXT_CAESAREUM_CLEARING + 9;
}

static void draw_scripted_monument_text(int32_t text, int32_t x, int32_t y)
{
    if (text >= 0) {
        draw_text(text, x, y);
    }
}

void window_building_draw_caesareum(void)
{
    const scripted_monument_info_progress *report = caesareum_report();
    info_help_id = building_info_help_messages[BUILDING_INFO_HELP_CAESAREUM];
    int32_t stage = building_get(monument_main_part_id(building_info_building_id))->data.monument.stage;
    info_first_draw = 0;
    draw_panel_title();
    int32_t top = building_info_y;
    int32_t x = building_info_x + MONUMENT_TEXT_X;
    int32_t y = building_info_y + MONUMENT_TEXT_Y;
    uint8_t carpenter_spot_free = caesareum_has_free_spot_for_type(FIGURE_CARPENTER, stage);
    uint8_t stonemason_spot_free = caesareum_has_free_spot_for_type(FIGURE_STONEMASON, stage);
    g_draw_y_accum = 0;
    uint8_t resource_ok = 0;
    if (stage < CAESAREUM_MARBLE_STAGE_END) {
        resource_ok = monument_resource_available(RESOURCE_MARBLE, report->stonemasons_waiting * MATERIAL_LOADS_PER_BLOCK);
    } else if (stage < CAESAREUM_GRANITE_STAGE_END) {
        resource_ok = monument_resource_available(RESOURCE_GRANITE, report->stonemasons_waiting * MATERIAL_LOADS_PER_BLOCK);
    }
    g_draw_y_accum = 0;
    int32_t text = -1;
    if (monument_blocked(building_info_building_id, building_info_building_id)) {
        text = MONUMENT_TEXT_MONUMENT_BLOCKED;
    } else if (report->stonemasons_waiting != 0 && resource_ok && report->material != 0) {
        if (stage < CAESAREUM_MARBLE_STAGE_END) {
            text = material_shortage_text(RESOURCE_MARBLE);
            y += g_draw_y_accum;
        } else if (stage < CAESAREUM_GRANITE_STAGE_END) {
            text = material_shortage_text(RESOURCE_GRANITE);
            y += g_draw_y_accum;
        }
    } else {
        text = work_camp_text(report->laborers);
        if (text == 0) {
            text = guild_text(stonemason_spot_free, carpenter_spot_free, report->stonemasons, report->carpenters);
        }
        if (text == 0) {
            text = caesareum_stage_text(stage);
        }
    }
    draw_scripted_monument_text(text, x, y);
    text_resources()->first = RESOURCE_MARBLE;
    int32_t width = text_width();
    int32_t lines_y = top + MONUMENT_REQUIREMENTS_Y + g_draw_y_accum;
    if (report->percent >= MONUMENT_PROGRESS_COMPLETE) {
        return;
    }
    g_draw_y_accum = 0;
    draw_expanded(MONUMENT_TEXT_PERCENT_COMPLETE, x, lines_y, width);
    lines_y += g_draw_y_accum;
    if (report->material > CAESAREUM_MARBLE_STOCK) {
        g_draw_y_accum = 0;
        draw_expanded(MONUMENT_TEXT_REMAINDER_NEEDS_MAIN, x, lines_y, width);
        lines_y += g_draw_y_accum;
    }
    if (report->material != 0) {
        text_resources()->first = RESOURCE_GRANITE;
        g_draw_y_accum = 0;
        draw_expanded(MONUMENT_TEXT_REMAINDER_NEEDS_MAIN, x, lines_y, width);
    }
}

static int32_t lighthouse_stage_text(int32_t stage)
{
    if (stage < 0) {
        return MONUMENT_TEXT_LIGHTHOUSE_CLEARING;
    }
    switch (stage) {
        case 0:
            return MONUMENT_TEXT_LIGHTHOUSE_CLEARING + 1;
        case 1:
            return MONUMENT_TEXT_LIGHTHOUSE_CLEARING + 2;
        case 2:
        case 3:
            return MONUMENT_TEXT_LIGHTHOUSE_CLEARING + 3;
        case 4:
            return MONUMENT_TEXT_LIGHTHOUSE_CLEARING + 4;
        case 5:
        case 6:
            return MONUMENT_TEXT_LIGHTHOUSE_CLEARING + 5;
        case 7:
            return MONUMENT_TEXT_LIGHTHOUSE_CLEARING + 6;
        case 8:
            return MONUMENT_TEXT_LIGHTHOUSE_CLEARING + 7;
        default:
            return -1;
    }
}

void window_building_draw_lighthouse(void)
{
    const scripted_monument_info_progress *report = lighthouse_report();
    info_help_id = building_info_help_messages[BUILDING_INFO_HELP_LIGHTHOUSE];
    int32_t stage = building_get(monument_main_part_id(building_info_building_id))->data.monument.stage;
    info_first_draw = 0;
    draw_panel_title();
    int32_t top = building_info_y;
    int32_t x = building_info_x + MONUMENT_TEXT_X;
    int32_t y = building_info_y + MONUMENT_TEXT_Y;
    uint8_t carpenter_spot_free = lighthouse_has_free_spot_for_type(FIGURE_CARPENTER, stage);
    uint8_t stonemason_spot_free = lighthouse_has_free_spot_for_type(FIGURE_STONEMASON, stage);
    g_draw_y_accum = 0;
    uint8_t resource_ok = monument_resource_available(RESOURCE_MARBLE, report->stonemasons_waiting * MATERIAL_LOADS_PER_BLOCK);
    int32_t blocked = monument_blocked(building_info_building_id, building_info_building_id);
    int32_t text = -1;
    if (stage >= LIGHTHOUSE_STAGE_DONE) {
        text = MONUMENT_TEXT_LIGHTHOUSE_COMPLETE;
    } else if (blocked) {
        text = MONUMENT_TEXT_MONUMENT_BLOCKED;
    } else if (report->stonemasons_waiting != 0 && resource_ok && report->material != 0) {
        text = material_shortage_text(RESOURCE_MARBLE);
        y += g_draw_y_accum;
    } else {
        text = 0;
        if (stonemason_spot_free) {
            text = work_camp_text(report->laborers);
            if (text == 0) {
                text = guild_text(1, 0, report->stonemasons, 0);
            }
        }
        if (text == 0 && carpenter_spot_free) {
            text = guild_text(0, 1, 0, report->carpenters);
        }
        if (text == 0) {
            text = lighthouse_stage_text(stage);
        }
    }
    draw_scripted_monument_text(text, x, y);
    text_resources()->first = RESOURCE_MARBLE;
    int32_t lines_y = top + MONUMENT_REQUIREMENTS_Y + g_draw_y_accum;
    int32_t width = text_width();
    if (report->percent >= MONUMENT_PROGRESS_COMPLETE) {
        return;
    }
    g_draw_y_accum = 0;
    draw_expanded(MONUMENT_TEXT_PERCENT_COMPLETE, x, lines_y, width);
    lines_y += g_draw_y_accum;
    if (report->material != 0) {
        g_draw_y_accum = 0;
        draw_expanded(MONUMENT_TEXT_REMAINDER_NEEDS_MAIN, x, lines_y, width);
    }
}

static int32_t tomb_idle(int32_t main_id)
{
    tomb_object *object = royal_tomb_object_for_type(building_info_building()->type);
    return object != 0 && royal_tomb_is_idle(object, main_id) != 0;
}

static int32_t tomb_goals_reached(void)
{
    return royal_tomb_object_for_type(building_info_building()->type) != 0 && city_victory_goals_reached() != 0;
}

static int32_t tomb_has_chamber_at(int32_t main_id, int32_t chambers, int32_t first_stage, int32_t last_stage)
{
    for (int32_t i = 0; i < chambers; i++) {
        int32_t stage = building_get(main_id + i)->data.royal_tomb.stage;
        if (stage >= first_stage && stage <= last_stage) {
            return 1;
        }
    }
    return 0;
}

static int32_t tomb_work_text(int32_t main_id, int32_t wants_stone, int32_t wants_artisans)
{
    const int32_t *report = tomb_report();
    int32_t text = work_camp_text(report[TOMB_PROGRESS_LABORERS]);
    if (text == 0 && tomb_idle(main_id)) {
        text = MONUMENT_TEXT_TOMB_REFUSED;
    }
    if (text == 0 && wants_stone) {
        text = guild_text(1, 0, report[TOMB_PROGRESS_STONEMASONS], 0);
    }
    if (text == 0 && wants_artisans) {
        if (!built(BUILDING_ARTISANS_GUILD)) {
            text = MONUMENT_TEXT_NO_ARTISANS;
        } else if (!working(BUILDING_ARTISANS_GUILD) && report[TOMB_PROGRESS_ARTISANS] == 0) {
            text = MONUMENT_TEXT_ARTISANS_NOT_COMING;
        }
    }
    return (text == 0) ? MONUMENT_TEXT_TOMB_WORKING : text;
}

void window_building_draw_royal_tomb(void)
{
    const int32_t *report = tomb_report();
    int32_t main_id = monument_main_part_id(building_info_building_id);
    info_help_id = building_info_help_messages[BUILDING_INFO_HELP_ROYAL_TOMB];
    const building *main = building_get(main_id);
    int32_t chambers = royal_tomb_chambers_of_type(main->type);
    int32_t wants_stone = tomb_has_chamber_at(main_id, chambers, TOMB_STAGE_STONEMASONS, TOMB_STAGE_STONEMASONS);
    int32_t wants_artisans = tomb_has_chamber_at(main_id, chambers, TOMB_STAGE_STONEMASONS + 1, TOMB_STAGE_ARTISANS_LAST);
    draw_panel_title();
    int32_t top = building_info_y;
    int32_t x = building_info_x + MONUMENT_TEXT_X;
    g_draw_y_accum = 0;
    g_text_width = 0;
    lang_text_draw_multiline_ui(LANG_GROUP_MONUMENTS, MONUMENT_TEXT_LAMPS, x, building_info_y + TOMB_LAMPS_Y, info_w_blocks * TOMB_LAMPS_WIDTH_PER_BLOCK,
        image_group(IMAGE_GROUP_FONT_BLACK), 0, building_info_x, building_info_y);
    lang_text_draw_amount(LANG_GROUP_AMOUNTS, AMOUNT_TEXT_UNIT, report[TOMB_PROGRESS_STOCK], g_text_width + building_info_x + TOMB_LAMPS_AMOUNT_X,
        building_info_y + TOMB_LAMPS_Y, image_group(IMAGE_GROUP_FONT_BLACK), 0);
    int32_t y = top + MONUMENT_TEXT_Y + g_draw_y_accum + MONUMENT_LINE_HEIGHT;
    if (main->created_sequence == MONUMENT_KEPT_FROM_EARLIER_MISSION) {
        draw_text(MONUMENT_TEXT_TOMB_READY, x, g_draw_y_accum + y);
        return;
    }
    int32_t text;
    int32_t text_y = g_draw_y_accum + y;
    if (report[TOMB_PROGRESS_PERCENT] < MONUMENT_PROGRESS_COMPLETE) {
        if (monument_blocked(main_id, building_info_building_id)) {
            text = MONUMENT_TEXT_TOMB_ENTRANCE_BLOCKED;
            text_y = y;
        } else {
            text_y = g_draw_y_accum + y;
            text = tomb_work_text(main_id, wants_stone, wants_artisans);
        }
    } else {
        text = tomb_goals_reached() ? MONUMENT_TEXT_TOMB_READY : MONUMENT_TEXT_TOMB_COMPLETE;
    }
    draw_text(text, x, text_y);
    int32_t lines_y = y + MONUMENT_LINE_HEIGHT + g_draw_y_accum;
    if (report[TOMB_PROGRESS_PERCENT] < MONUMENT_PROGRESS_COMPLETE) {
        g_draw_y_accum = 0;
        draw_expanded(MONUMENT_TEXT_PERCENT_COMPLETE, x, lines_y, info_w_blocks * BUILDING_INFO_BLOCK - BUILDING_INFO_BLOCK);
    }
}

static int32_t mausoleum_stone_text(const mausoleum_info_progress *report, int32_t sandstone_ok)
{
    if (!built(BUILDING_WORK_CAMP)) {
        return MONUMENT_TEXT_NO_WORK_CAMP;
    }
    if (!built(BUILDING_STONEMASONS_GUILD)) {
        return MONUMENT_TEXT_NO_STONEMASONS;
    }
    if (report->stonemasons == report->stonemasons_waiting && !sandstone_ok) {
        return MONUMENT_TEXT_NEEDS_SANDSTONE;
    }
    if (report->stonemasons == 0) {
        return MONUMENT_TEXT_STONEMASONS_NOT_COMING;
    }
    if (report->stonemasons_waiting == 0 || !sandstone_ok || report->material == 0) {
        return MONUMENT_TEXT_MAUSOLEUM_WORKING;
    }
    return (city_player_current()->stockpiled[RESOURCE_SANDSTONE] == 0) ? MONUMENT_TEXT_NEEDS_SANDSTONE : MONUMENT_TEXT_SANDSTONE_STOCKPILED;
}

static int32_t mausoleum_building_text(const mausoleum_info_progress *report, int32_t sandstone_ok)
{
    if (mausoleum_get_progress(building_info_building_id, 0) >= MONUMENT_PROGRESS_COMPLETE) {
        return MONUMENT_TEXT_MAUSOLEUM_COMPLETE;
    }
    if (report->material == 0) {
        if (report->parts_below_stage_2 == 0) {
            return MONUMENT_TEXT_MAUSOLEUM_WORKING;
        }
        if (!built(BUILDING_WORK_CAMP)) {
            return MONUMENT_TEXT_NO_WORK_CAMP;
        }
        return (report->laborers == 0) ? MONUMENT_TEXT_NO_PEASANTS : MONUMENT_TEXT_MAUSOLEUM_WORKING;
    }
    if (!built(BUILDING_WORK_CAMP)) {
        return MONUMENT_TEXT_NO_WORK_CAMP;
    }
    if (!built(BUILDING_STONEMASONS_GUILD)) {
        return MONUMENT_TEXT_NO_STONEMASONS;
    }
    if (report->blocks_left != 0) {
        return MONUMENT_TEXT_MAUSOLEUM_NEEDS_CARPENTER;
    }
    if (report->stonemasons == report->stonemasons_waiting && !sandstone_ok) {
        return MONUMENT_TEXT_NEEDS_SANDSTONE;
    }
    if (report->stonemasons == 0) {
        return MONUMENT_TEXT_STONEMASONS_NOT_COMING;
    }
    return (report->stonemasons_waiting != 0 && sandstone_ok) ? MONUMENT_TEXT_NEEDS_SANDSTONE : MONUMENT_TEXT_MAUSOLEUM_WORKING;
}

void window_building_draw_mausoleum(void)
{
    const mausoleum_info_progress *report = mausoleum_report();
    start_monument_panel(BUILDING_INFO_HELP_MAUSOLEUM);
    int32_t x = building_info_x + MONUMENT_TEXT_X;
    int32_t y = building_info_y + MONUMENT_TEXT_Y;
    int32_t sandstone_ok = sandstone_available(report->stonemasons_waiting);
    g_draw_y_accum = 0;
    int32_t text;
    switch (report->status) {
        case MAUSOLEUM_STATUS_SITE:
            if (!built(BUILDING_WORK_CAMP)) {
                text = MONUMENT_TEXT_NO_WORK_CAMP;
            } else if (working(BUILDING_WORK_CAMP) || report->laborers != 0) {
                text = MONUMENT_TEXT_MAUSOLEUM_BEGIN;
            } else {
                text = no_peasants_text();
            }
            break;
        case MAUSOLEUM_STATUS_STARTED:
            text = mausoleum_stone_text(report, sandstone_ok);
            break;
        case MAUSOLEUM_STATUS_BLOCKS_RISING:
            text = mausoleum_building_text(report, sandstone_ok);
            break;
        default:
            g_draw_y_accum = 0;
            return;
    }
    draw_text(text, x, y);
    y += g_draw_y_accum;
    text_resources()->first = RESOURCE_SANDSTONE;
    g_draw_y_accum = 0;
    draw_requirements_or_done(report->percent, report->material, x, y, text_width());
}
