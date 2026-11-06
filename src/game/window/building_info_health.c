// The building info panels of the health and water buildings: apothecary, physician, dentist, mortuary (linen), the
// water lift, the water supply (a text per missing worker) and the well (whether nearby houses need it).

#include "window/building_info_health.h"
#include "building/building.h"
#include "city/resource_type.h"
#include "map/water_supply.h"
#include "window/building_info.h"
#include "window/building_info_panels.h"
#include "window/window_data.h"
#include <stdint.h>

enum water_lift_text {
    WATER_LIFT_TEXT_WORKING = 1,
    WATER_LIFT_TEXT_NOT_WORKING = 2,
    WATER_LIFT_TEXT_NO_WATER = 3,
};

enum mortuary_text {
    MORTUARY_TEXT_NO_LINEN = 4,
};

enum water_supply_text {
    WATER_SUPPLY_TEXT_NO_STAFF = 7,
};

enum well_text {
    WELL_TEXT_NEEDED = 1,
    WELL_TEXT_HOUSES_HAVE_WATER_SUPPLY = 2,
    WELL_TEXT_NO_HOUSES_IN_RANGE = 3,
};

enum well_houses {
    WELL_HOUSES_HAVE_WATER_SUPPLY = 1,
    WELL_HOUSES_NONE_IN_RANGE = 2,
};

enum {
    WELL_RADIUS = 2,
    WATER_TEXT_FROM_BOTTOM = 193,
    WELL_TEXT_FROM_BOTTOM = 126,
};

static int32_t text_y_from_bottom(int32_t from_bottom)
{
    return (building_info_h * BUILDING_INFO_BLOCK) - from_bottom + building_info_y;
}

static int32_t water_lift_text(const building *b)
{
    if (b->has_water_access == 0) {
        return WATER_LIFT_TEXT_NO_WATER;
    }
    return (b->num_workers < 1) ? WATER_LIFT_TEXT_NOT_WORKING : WATER_LIFT_TEXT_WORKING;
}

void window_building_draw_water_lift(void)
{
    info_help_id = building_info_help_messages[BUILDING_INFO_HELP_WATER_LIFT];
    info_first_draw = 0;
    window_building_draw_panel_title(LANG_GROUP_WATER_LIFT, BUILDING_TEXT_TITLE);
    window_building_draw_right_click_hint();
    int32_t y = text_y_from_bottom(WATER_TEXT_FROM_BOTTOM);
    if (bi_road_access_block.has_road_access < 1) {
        window_building_draw_no_road_text(y);
    } else {
        window_building_draw_text(LANG_GROUP_WATER_LIFT, water_lift_text(building_info_building()), y);
    }
    window_building_draw_workers_panel();
}

void window_building_draw_water_supply(void)
{
    info_help_id = building_info_help_messages[BUILDING_INFO_HELP_WATER_SUPPLY];
    info_first_draw = 0;
    window_building_draw_panel_title(LANG_GROUP_WATER_SUPPLY, BUILDING_TEXT_TITLE);
    int32_t y = text_y_from_bottom(WATER_TEXT_FROM_BOTTOM);
    if (bi_road_access_block.has_road_access < 1) {
        window_building_draw_no_road_text(y);
    } else {
        window_building_draw_text(LANG_GROUP_WATER_SUPPLY, WATER_SUPPLY_TEXT_NO_STAFF - building_info_building()->num_workers, y);
    }
    window_building_draw_workers_panel();
}

static int32_t well_text(int32_t houses)
{
    if (houses == WELL_HOUSES_HAVE_WATER_SUPPLY) {
        return WELL_TEXT_HOUSES_HAVE_WATER_SUPPLY;
    }
    if (houses == WELL_HOUSES_NONE_IN_RANGE) {
        return WELL_TEXT_NO_HOUSES_IN_RANGE;
    }
    return WELL_TEXT_NEEDED;
}

void window_building_draw_well(void)
{
    info_help_id = building_info_help_messages[BUILDING_INFO_HELP_WELL];
    info_first_draw = 0;
    window_building_draw_panel_title(LANG_GROUP_WELL, BUILDING_TEXT_TITLE);
    int32_t houses = map_houses_water_in_radius(building_info_building_id, WELL_RADIUS);
    window_building_draw_text(LANG_GROUP_WELL, well_text(houses), text_y_from_bottom(WELL_TEXT_FROM_BOTTOM));
}

static void draw_service(int32_t help, int32_t group)
{
    info_help_id = building_info_help_messages[help];
    info_first_draw = 0;
    window_building_draw_panel_title(group, BUILDING_TEXT_TITLE);
    window_building_draw_service_state(group, SERVICE_TEXT_WORKING);
    window_building_draw_workers_panel();
}

void window_building_draw_apothecary(void)
{
    draw_service(BUILDING_INFO_HELP_APOTHECARY, LANG_GROUP_APOTHECARY);
}

void window_building_draw_physician(void)
{
    draw_service(BUILDING_INFO_HELP_PHYSICIAN, LANG_GROUP_PHYSICIAN);
}

void window_building_draw_dentist(void)
{
    draw_service(BUILDING_INFO_HELP_DENTIST, LANG_GROUP_DENTIST);
}

void window_building_draw_mortuary(void)
{
    info_help_id = building_info_help_messages[BUILDING_INFO_HELP_MORTUARY];
    info_first_draw = 0;
    window_building_draw_panel_title(LANG_GROUP_MORTUARY, BUILDING_TEXT_TITLE);
    window_building_draw_stock(RESOURCE_LINEN);
    const building *b = building_info_building();
    int32_t working_text = (b->loads_stored < 1) ? MORTUARY_TEXT_NO_LINEN : SERVICE_TEXT_WORKING;
    window_building_draw_service_state(LANG_GROUP_MORTUARY, working_text);
    window_building_draw_workers_panel();
}
