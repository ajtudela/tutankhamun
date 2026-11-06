// The building info panels of the mission post and the native buildings (hut, meeting, crops): a title and a text.

#include "window/building_info_natives.h"
#include "window/building_info_panels.h"
#include "window/window_data.h"
#include <stdint.h>

enum {
    MISSION_POST_TEXT_Y = 106,
    NATIVE_TEXT_FROM_BOTTOM = 143,
};

void window_building_draw_mission_post(void)
{
    info_help_id = building_info_help_messages[BUILDING_INFO_HELP_MISSION_POST];
    info_first_draw = 0;
    window_building_draw_panel_title(LANG_GROUP_MISSION_POST, BUILDING_TEXT_TITLE);
    window_building_draw_text(LANG_GROUP_MISSION_POST, BUILDING_TEXT_DESCRIPTION, building_info_y + MISSION_POST_TEXT_Y);
}

static void draw_native(int32_t group)
{
    info_help_id = building_info_help_messages[BUILDING_INFO_HELP_NATIVES];
    info_first_draw = 0;
    window_building_draw_panel_title(group, BUILDING_TEXT_TITLE);
    window_building_draw_text(group, BUILDING_TEXT_DESCRIPTION, (building_info_h * BUILDING_INFO_BLOCK) - NATIVE_TEXT_FROM_BOTTOM + building_info_y);
}

void window_building_draw_native_hut(void)
{
    draw_native(LANG_GROUP_NATIVE_HUT);
}

void window_building_draw_native_meeting(void)
{
    draw_native(LANG_GROUP_NATIVE_MEETING);
}

void window_building_draw_native_crops(void)
{
    draw_native(LANG_GROUP_NATIVE_CROPS);
}
