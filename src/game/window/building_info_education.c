// The building info panels of the scribal school and the library: the papyrus stored, the service state and the
// workers line.

#include "window/building_info_education.h"
#include "city/resource_type.h"
#include "window/building_info_panels.h"
#include "window/window_data.h"
#include <stdint.h>

static void draw_school(int32_t help, int32_t group)
{
    info_help_id = building_info_help_messages[help];
    info_first_draw = 0;
    window_building_draw_panel_title(group, BUILDING_TEXT_TITLE);
    window_building_draw_stock(RESOURCE_PAPYRUS);
    window_building_draw_service_state(group, SERVICE_TEXT_WORKING);
    window_building_draw_workers_panel();
}

void window_building_draw_scribal_school(void)
{
    draw_school(BUILDING_INFO_HELP_SCRIBAL_SCHOOL, LANG_GROUP_SCRIBAL_SCHOOL);
}

void window_building_draw_library(void)
{
    draw_school(BUILDING_INFO_HELP_LIBRARY, LANG_GROUP_LIBRARY);
}
