// The building info panels of terrain and beautification: gardens, the three walls, the irrigation ditch, the plaza
// and statues, and the ruins (with the building type the rubble was).

#include "window/building_info_terrain.h"
#include "graphics/image_collection.h"
#include "graphics/ui_layout.h"
#include "text/lang.h"
#include "window/building_info.h"
#include "window/building_info_panels.h"
#include "window/window_image_group.h"
#include <stdint.h>

enum irrigation_ditch_text {
    DITCH_TEXT_WATERED = 1,
    DITCH_TEXT_DRY = 2,
};

enum statue_text {
    STATUE_TEXT_STATUE = 0,
    STATUE_TEXT_TRIUMPHAL_ARCH = 2,
};

enum {
    RUIN_TYPE_FROM_BOTTOM = 173,
    RUIN_TEXT_FROM_BOTTOM = 143,
    TERRAIN_TEXT_FROM_BOTTOM = 158,
    PLAZA_TEXT_FROM_BOTTOM = 113,
    DITCH_TEXT_BLOCKS_FROM_BOTTOM = 8,
};

static int32_t text_y_from_bottom(int32_t from_bottom)
{
    return (building_info_h * BUILDING_INFO_BLOCK) - from_bottom + building_info_y;
}

static void draw_ruin(int32_t group)
{
    info_help_id = building_info_help_messages[BUILDING_INFO_HELP_RUINS];
    info_first_draw = 0;
    window_building_draw_panel_title(group, BUILDING_TEXT_TITLE);
    lang_text_draw_ui(LANG_GROUP_BUILDING_TYPES, bi_road_access_block.rubble_building_type, building_info_x + BUILDING_INFO_TEXT_X, text_y_from_bottom(RUIN_TYPE_FROM_BOTTOM),
        image_group(IMAGE_GROUP_FONT_BLACK), 0, building_info_x, building_info_y);
    window_building_draw_text(group, BUILDING_TEXT_DESCRIPTION, text_y_from_bottom(RUIN_TEXT_FROM_BOTTOM));
}

void window_building_draw_burning_ruin(void)
{
    draw_ruin(LANG_GROUP_BURNING_RUIN);
}

void window_building_draw_ruins(void)
{
    draw_ruin(LANG_GROUP_RUBBLE);
}

static void draw_terrain(int32_t help, int32_t group)
{
    info_help_id = building_info_help_messages[help];
    info_first_draw = 0;
    window_building_draw_panel_title(group, BUILDING_TEXT_TITLE);
    window_building_draw_text(group, BUILDING_TEXT_DESCRIPTION, text_y_from_bottom(TERRAIN_TEXT_FROM_BOTTOM));
}

void window_building_draw_gardens(void)
{
    draw_terrain(BUILDING_INFO_HELP_GARDENS, LANG_GROUP_GARDENS);
}

void window_building_draw_mud_wall(void)
{
    draw_terrain(BUILDING_INFO_HELP_WALLS, LANG_GROUP_MUD_WALL);
}

void window_building_draw_brick_wall(void)
{
    draw_terrain(BUILDING_INFO_HELP_WALLS, LANG_GROUP_BRICK_WALL);
}

void window_building_draw_strong_wall(void)
{
    draw_terrain(BUILDING_INFO_HELP_WALLS, LANG_GROUP_STRONG_WALL);
}

void window_building_draw_canal(void)
{
    info_help_id = building_info_help_messages[BUILDING_INFO_HELP_IRRIGATION_DITCH];
    info_first_draw = 0;
    window_building_draw_panel_title(LANG_GROUP_IRRIGATION_DITCH, BUILDING_TEXT_TITLE);
    int32_t text = (bi_road_access_block.canal != 0) ? DITCH_TEXT_WATERED : DITCH_TEXT_DRY;
    window_building_draw_text(LANG_GROUP_IRRIGATION_DITCH, text, ((building_info_h - DITCH_TEXT_BLOCKS_FROM_BOTTOM) * BUILDING_INFO_BLOCK) + building_info_y);
}

void window_building_draw_plaza(void)
{
    info_help_id = building_info_help_messages[BUILDING_INFO_HELP_PLAZA];
    info_first_draw = 0;
    window_building_prepare_figure_list();
    window_building_draw_panel_title(LANG_GROUP_PLAZA, BUILDING_TEXT_TITLE);
    window_building_draw_figure_list();
    window_building_draw_text(LANG_GROUP_PLAZA, BUILDING_TEXT_DESCRIPTION, text_y_from_bottom(PLAZA_TEXT_FROM_BOTTOM));
}

static void draw_statue(int32_t title)
{
    info_help_id = building_info_help_messages[BUILDING_INFO_HELP_STATUE];
    info_first_draw = 0;
    window_building_prepare_figure_list();
    window_building_draw_panel_title(LANG_GROUP_STATUE, title);
    window_building_draw_text(LANG_GROUP_STATUE, title + 1, text_y_from_bottom(TERRAIN_TEXT_FROM_BOTTOM));
}

void window_building_draw_statue(void)
{
    draw_statue(STATUE_TEXT_STATUE);
}

void window_building_draw_triumphal_arch(void)
{
    draw_statue(STATUE_TEXT_TRIUMPHAL_ARCH);
}
