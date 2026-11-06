// The scenario editor's sidebar text below the minimap: the tool and terrain names, the climate and the state of the
// scenario's points, green when set and red when not.

#include "editor/editor_sidebar.h"
#include "editor/editor_internal.h"
#include "editor/editor_terrain_tool.h"
#include "editor/editor_text.h"
#include "figure/action.h"
#include "figure/figure_type.h"
#include "graphics/graphics_primitives.h"
#include "graphics/image_collection.h"
#include "graphics/text_draw.h"
#include "graphics/ui_layout.h"
#include "scenario/invasion.h"
#include "scenario/requests.h"
#include "scenario/scenario_climate.h"
#include "text/lang.h"
#include "widget/widget_data.h"
#include "window/window_image_group.h"
#include <stdint.h>

static void sidebar_text(int32_t group, int32_t index, int32_t x, int32_t y, int32_t color)
{
    lang_text_draw_ui(group, index, x, y, image_group(IMAGE_GROUP_FONT_SMALL), color, ui_origin_x, ui_origin_y);
}

void editor_sidebar_draw_text(void)
{
    int32_t left = city_view_width_expanded + 8;
    int32_t x = left + 2;
    graphics_fill_rect(left, 175, 145, 111, 0);
    sidebar_text(LANG_GROUP_EDITOR_TOOLS, editor_tool_kind, x, 177, EDITOR_COLOR_ORANGE);
    int32_t kind = editor_tool_kind;
    int32_t color;
    if (kind == EDITOR_TOOL_GRASS || kind == EDITOR_TOOL_TREES || kind == EDITOR_TOOL_WATER || kind == EDITOR_TOOL_SCRUB || kind == EDITOR_TOOL_ROCKS ||
        kind == EDITOR_TOOL_CLIFF || kind == EDITOR_TOOL_SAND_DUNES || kind == EDITOR_TOOL_MEADOW || kind == EDITOR_TOOL_RAISE_LAND ||
        kind == EDITOR_TOOL_LOWER_LAND || kind == EDITOR_TOOL_FLOODPLAIN) {
        color = EDITOR_COLOR_ORANGE;
    } else {
        color = EDITOR_COLOR_GREY;
    }
    sidebar_text(LANG_GROUP_EDITOR_BRUSHES, editor_brush_size, x, 193, color);
    if (scenario_data.predator_variant == 1) {
        sidebar_text(LANG_GROUP_FIGURE_TYPES, scenario_data.climate_type + FIGURE_ASP, x, 209, EDITOR_COLOR_ORANGE);
    } else if (scenario_data.climate_type == CLIMATE_DESERT) {
        sidebar_text(LANG_GROUP_FIGURE_TYPES, FIGURE_HYENA, x, 209, EDITOR_COLOR_ORANGE);
    } else if (scenario_data.climate_type == CLIMATE_NORTHERN) {
        sidebar_text(LANG_GROUP_FIGURE_TYPES, FIGURE_CROCODILE, x, 209, EDITOR_COLOR_ORANGE);
    } else {
        sidebar_text(LANG_GROUP_FIGURE_TYPES, FIGURE_HIPPO, x, 209, EDITOR_COLOR_ORANGE);
    }
    if (scenario_count_earthquake_events(&scenario_events) != 0) {
        if (scenario_data.earthquake_x != -1 && scenario_data.earthquake_y != -1) {
            sidebar_text(LANG_GROUP_EDITOR, EDITOR_TEXT_QUAKE_POINT_SET, x, 211, EDITOR_COLOR_GREEN);
        } else {
            sidebar_text(LANG_GROUP_EDITOR, EDITOR_TEXT_NEED_QUAKE_POINT, x, 211, EDITOR_COLOR_RED);
        }
    }
    if (scenario_data.entry_point_x != -1 && scenario_data.entry_point_y != -1) {
        sidebar_text(LANG_GROUP_EDITOR, EDITOR_TEXT_ENTRY_POINT_SET, x, 225, EDITOR_COLOR_GREEN);
    } else {
        sidebar_text(LANG_GROUP_EDITOR, EDITOR_TEXT_NO_ENTRY_POINT, x, 225, EDITOR_COLOR_RED);
    }
    if (scenario_data.exit_point_x != -1 && scenario_data.exit_point_y != -1) {
        sidebar_text(LANG_GROUP_EDITOR, EDITOR_TEXT_EXIT_POINT_SET, x, 239, EDITOR_COLOR_GREEN);
    } else {
        sidebar_text(LANG_GROUP_EDITOR, EDITOR_TEXT_NO_EXIT_POINT, x, 239, EDITOR_COLOR_RED);
    }
    int32_t invasions = 0;
    for (int32_t i = 0; i < SCENARIO_INVASION_POINTS; i++) {
        if (scenario_data.invasion_point_x[i] != -1 && scenario_data.invasion_point_y[i] != -1) {
            invasions++;
        }
    }
    if (invasions <= 0) {
        sidebar_text(LANG_GROUP_EDITOR, EDITOR_TEXT_NO_INVASION_POINTS, x, 253, EDITOR_COLOR_RED);
    } else if (invasions == 1) {
        sidebar_text(LANG_GROUP_EDITOR, EDITOR_TEXT_ONE_INVASION_POINT, x, 253, EDITOR_COLOR_GREEN);
    } else {
        g_text_width = 0;
        text_draw_number_ui(invasions, NUMBER_PREFIX_NONE, postfix_none, x, 253, image_group(IMAGE_GROUP_FONT_SMALL), EDITOR_COLOR_GREEN, ui_origin_x, ui_origin_y);
        sidebar_text(LANG_GROUP_EDITOR, EDITOR_TEXT_INVASION_POINTS, g_text_width + left + 2, 253, EDITOR_COLOR_GREEN);
    }
    int32_t entry_x = scenario_data.river_entry_x;
    int32_t entry_y = scenario_data.river_entry_y;
    int32_t exit_x = scenario_data.river_exit_x;
    int32_t exit_y = scenario_data.river_exit_y;
    if ((entry_x == -1 || entry_y == -1) && (exit_x == -1 || exit_y == -1)) {
        sidebar_text(LANG_GROUP_EDITOR, EDITOR_TEXT_NO_RIVER_POINTS, x, 267, EDITOR_COLOR_RED);
    } else if (entry_x != -1 && entry_y != -1 && exit_x != -1 && exit_y != -1) {
        sidebar_text(LANG_GROUP_EDITOR, EDITOR_TEXT_RIVER_POINTS_SET, x, 267, EDITOR_COLOR_GREEN);
    } else {
        sidebar_text(LANG_GROUP_EDITOR, EDITOR_TEXT_ONE_RIVER_POINT_SET, x, 267, EDITOR_COLOR_RED);
    }
}
