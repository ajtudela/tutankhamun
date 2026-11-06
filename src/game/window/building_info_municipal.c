// The building info panels of the architect's post, the police station (with the thefts and the gold stolen this year)
// and the firehouse: the walker's state and the workers line.

#include "window/building_info_municipal.h"
#include "city/city_player.h"
#include "graphics/image_collection.h"
#include "graphics/text_draw.h"
#include "graphics/ui_layout.h"
#include "text/lang.h"
#include "window/building_info_panels.h"
#include "window/window_image_group.h"
#include <stdint.h>

enum police_station_text {
    POLICE_TEXT_THEFTS_THIS_YEAR = 10,
    POLICE_TEXT_GOLD_STOLEN_THIS_YEAR = 11,
};

enum {
    POLICE_THEFTS_Y = 104,
    POLICE_GOLD_STOLEN_Y = 119,
    FIREHOUSE_DESCRIPTION_FROM_BOTTOM = 113,
};

void window_building_draw_architect_post(void)
{
    info_help_id = building_info_help_messages[BUILDING_INFO_HELP_ARCHITECT_POST];
    info_first_draw = 0;
    window_building_draw_panel_title(LANG_GROUP_ARCHITECT_POST, BUILDING_TEXT_TITLE);
    window_building_draw_walker_state(LANG_GROUP_ARCHITECT_POST);
    window_building_draw_workers_panel();
}

static void draw_stolen_number(int32_t value, const char *postfix, int32_t dy)
{
    text_draw_number_ui(value, NUMBER_PREFIX_NONE, postfix, building_info_x + BUILDING_INFO_TEXT_X, building_info_y + dy, image_group(IMAGE_GROUP_FONT_BLACK), 0, building_info_x, building_info_y);
}

void window_building_draw_police_station(void)
{
    info_help_id = building_info_help_messages[BUILDING_INFO_HELP_POLICE_STATION];
    info_first_draw = 0;
    window_building_draw_panel_title(LANG_GROUP_POLICE_STATION, BUILDING_TEXT_TITLE);
    window_building_draw_walker_state(LANG_GROUP_POLICE_STATION);
    const char *thefts_text = lang_get_string(LANG_GROUP_POLICE_STATION, POLICE_TEXT_THEFTS_THIS_YEAR);
    const char *gold_stolen_text = lang_get_string(LANG_GROUP_POLICE_STATION, POLICE_TEXT_GOLD_STOLEN_THIS_YEAR);
    const city_player *player = city_player_current();
    draw_stolen_number(player->palace_robberies, thefts_text, POLICE_THEFTS_Y);
    draw_stolen_number(player->stolen_this_year, gold_stolen_text, POLICE_GOLD_STOLEN_Y);
    window_building_draw_workers_panel();
}

void window_building_draw_firehouse(void)
{
    info_help_id = building_info_help_messages[BUILDING_INFO_HELP_FIREHOUSE];
    info_first_draw = 0;
    window_building_draw_panel_title(LANG_GROUP_FIREHOUSE, BUILDING_TEXT_TITLE);
    window_building_draw_walker_state(LANG_GROUP_FIREHOUSE);
    window_building_draw_workers_panel();
    window_building_draw_text(
        LANG_GROUP_FIREHOUSE, BUILDING_TEXT_DESCRIPTION, (building_info_h * BUILDING_INFO_BLOCK) - FIREHOUSE_DESCRIPTION_FROM_BOTTOM + building_info_y);
}
