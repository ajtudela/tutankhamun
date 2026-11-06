// The building info panels of the entertainment buildings: the bandstand, booth and pavilion with their shows and the
// days each still runs, the senet house (beer), the zoo (game meat and straw) and the schools of performers.

#include "window/building_info_entertainment.h"
#include "building/building.h"
#include "city/resource_type.h"
#include "graphics/image_collection.h"
#include "graphics/image_draw.h"
#include "graphics/text_draw.h"
#include "graphics/ui_layout.h"
#include "text/lang.h"
#include "window/building_info.h"
#include "window/building_info_panels.h"
#include "window/window_image_group.h"
#include <stdint.h>

enum venue_show {
    SHOW_JUGGLING = 0,
    SHOW_MUSIC = 1,
    SHOW_DANCING = 2,
};

enum bandstand_text {
    BANDSTAND_TEXT_NO_SHOWS = 2,
    BANDSTAND_TEXT_JUGGLING_AND_MUSIC = 3,
    BANDSTAND_TEXT_MUSIC_ONLY = 4,
    BANDSTAND_TEXT_JUGGLING_ONLY = 5,
    BANDSTAND_TEXT_NO_STAFF = 6,
    BANDSTAND_TEXT_NO_MUSIC = 7,
    BANDSTAND_TEXT_MUSIC_DAYS = 8,
    BANDSTAND_TEXT_NO_JUGGLING = 9,
    BANDSTAND_TEXT_JUGGLING_DAYS = 10,
};

enum booth_text {
    BOOTH_TEXT_NO_SHOWS = 2,
    BOOTH_TEXT_JUGGLING = 3,
    BOOTH_TEXT_NO_STAFF = 4,
    BOOTH_TEXT_NO_JUGGLING = 5,
    BOOTH_TEXT_JUGGLING_DAYS = 6,
};

enum senet_house_text {
    SENET_TEXT_NO_STAFF = 4,
    SENET_TEXT_STORED_BEER = 7,
};

enum zoo_text {
    ZOO_TEXT_WORKING = 1,
    ZOO_TEXT_NEEDS_GAME_MEAT = 2,
    ZOO_TEXT_NEEDS_STRAW = 3,
    ZOO_TEXT_NO_STAFF = 4,
    ZOO_TEXT_GAME_MEAT = 6,
    ZOO_TEXT_STRAW = 7,
};

enum pavilion_text {
    PAVILION_TEXT_NO_SHOWS = 2,
    PAVILION_TEXT_ALL_SHOWS = 3,
    PAVILION_TEXT_DANCING_AND_MUSIC = 4,
    PAVILION_TEXT_DANCING_AND_JUGGLING = 5,
    PAVILION_TEXT_MUSIC_AND_JUGGLING = 6,
    PAVILION_TEXT_JUGGLING_ONLY = 7,
    PAVILION_TEXT_MUSIC_ONLY = 8,
    PAVILION_TEXT_DANCING_ONLY = 9,
    PAVILION_TEXT_NO_STAFF = 10,
    PAVILION_TEXT_NO_JUGGLING = 11,
    PAVILION_TEXT_JUGGLING_DAYS = 12,
    PAVILION_TEXT_NO_MUSIC = 13,
    PAVILION_TEXT_MUSIC_DAYS = 14,
    PAVILION_TEXT_NO_DANCING = 15,
    PAVILION_TEXT_DANCING_DAYS = 16,
};

enum performer_school_text {
    SCHOOL_TEXT_FULL_STAFF = 2,
    SCHOOL_TEXT_NO_STAFF = 7,
};

enum {
    BANDSTAND_PANEL_BLOCKS = 7,
    VENUE_PANEL_BLOCKS = 6,
    FIRST_SHOW_Y = 182,
    BANDSTAND_SECOND_SHOW_Y = 202,
    PAVILION_SECOND_SHOW_Y = 197,
    PAVILION_THIRD_SHOW_Y = 212,
    SHOW_DAYS_PER_COUNT = 2,
    VENUE_STOCK_ICON_Y = 36,
    VENUE_STOCK_TEXT_Y = 40,
    FIRST_STOCK_ICON_X = 32,
    FIRST_STOCK_TEXT_X = 60,
    SECOND_STOCK_ICON_X = 255,
    SECOND_STOCK_TEXT_X = 280,
};

static void draw_venue_text(int32_t group, int32_t text)
{
    window_building_draw_text(group, text, building_info_y + BUILDING_INFO_TEXT_Y);
}

static void draw_venue_workers(int32_t panel_blocks)
{
    window_building_draw_inner_panel(BUILDING_INFO_WORKERS_PANEL_Y, panel_blocks);
    window_building_draw_employment(building_info_y + BUILDING_INFO_SHOWS_EMPLOYMENT_Y);
}

static void draw_show_days(int32_t group, int32_t no_show_text, int32_t days_text, int32_t days, int32_t dy)
{
    int32_t x = building_info_x + BUILDING_INFO_TEXT_X;
    int32_t y = building_info_y + dy;
    if (days == 0) {
        lang_text_draw_ui(group, no_show_text, x, y, image_group(IMAGE_GROUP_FONT_GREEN), 0, building_info_x, building_info_y);
        return;
    }
    g_text_width = 0;
    lang_text_draw_ui(group, days_text, x, y, image_group(IMAGE_GROUP_FONT_GREEN), 0, building_info_x, building_info_y);
    lang_text_draw_amount(LANG_GROUP_AMOUNTS, AMOUNT_TEXT_DAY, days * SHOW_DAYS_PER_COUNT, g_text_width + x, y, image_group(IMAGE_GROUP_FONT_GREEN), 0);
}

static int32_t bandstand_text(const building *b)
{
    if (b->num_workers < 1) {
        return BANDSTAND_TEXT_NO_STAFF;
    }
    if (b->data.entertainment.num_shows == 0) {
        return BANDSTAND_TEXT_NO_SHOWS;
    }
    if (b->data.entertainment.num_shows == 2) {
        return BANDSTAND_TEXT_JUGGLING_AND_MUSIC;
    }
    if (b->data.entertainment.show_days_left[SHOW_MUSIC] != 0) {
        return BANDSTAND_TEXT_MUSIC_ONLY;
    }
    if (b->data.entertainment.show_days_left[SHOW_JUGGLING] != 0) {
        return BANDSTAND_TEXT_JUGGLING_ONLY;
    }
    return -1;
}

void window_building_draw_bandstand(void)
{
    info_help_id = building_info_help_messages[BUILDING_INFO_HELP_BANDSTAND];
    info_first_draw = 0;
    window_building_draw_panel_title(LANG_GROUP_BANDSTAND, BUILDING_TEXT_TITLE);
    const building *b = building_info_building();
    if (bi_road_access_block.has_road_access < 1) {
        window_building_draw_no_road_text(building_info_y + BUILDING_INFO_TEXT_Y);
    } else {
        int32_t text = bandstand_text(b);
        if (text >= 0) {
            draw_venue_text(LANG_GROUP_BANDSTAND, text);
        }
    }
    draw_venue_workers(BANDSTAND_PANEL_BLOCKS);
    const uint8_t *days = b->data.entertainment.show_days_left;
    draw_show_days(LANG_GROUP_BANDSTAND, BANDSTAND_TEXT_NO_JUGGLING, BANDSTAND_TEXT_JUGGLING_DAYS, days[SHOW_JUGGLING], FIRST_SHOW_Y);
    draw_show_days(LANG_GROUP_BANDSTAND, BANDSTAND_TEXT_NO_MUSIC, BANDSTAND_TEXT_MUSIC_DAYS, days[SHOW_MUSIC], BANDSTAND_SECOND_SHOW_Y);
}

static int32_t booth_text(const building *b)
{
    if (b->num_workers < 1) {
        return BOOTH_TEXT_NO_STAFF;
    }
    if (b->data.entertainment.num_shows == 0) {
        return BOOTH_TEXT_NO_SHOWS;
    }
    if (b->data.entertainment.show_days_left[SHOW_JUGGLING] != 0) {
        return BOOTH_TEXT_JUGGLING;
    }
    return -1;
}

void window_building_draw_booth(void)
{
    info_help_id = building_info_help_messages[BUILDING_INFO_HELP_BOOTH];
    info_first_draw = 0;
    window_building_draw_panel_title(LANG_GROUP_BOOTH, BUILDING_TEXT_TITLE);
    const building *b = building_info_building();
    if (bi_road_access_block.has_road_access < 1) {
        window_building_draw_no_road_text(building_info_y + BUILDING_INFO_TEXT_Y);
    } else {
        int32_t text = booth_text(b);
        if (text >= 0) {
            draw_venue_text(LANG_GROUP_BOOTH, text);
        }
    }
    draw_venue_workers(VENUE_PANEL_BLOCKS);
    draw_show_days(LANG_GROUP_BOOTH, BOOTH_TEXT_NO_JUGGLING, BOOTH_TEXT_JUGGLING_DAYS, b->data.entertainment.show_days_left[SHOW_JUGGLING], FIRST_SHOW_Y);
}

static void draw_venue_stock(int32_t resource, int32_t icon_dx, int32_t text_dx, int32_t group, int32_t text, int32_t amount)
{
    image_draw(image_group(IMAGE_GROUP_RESOURCE_ICONS) + resource, building_info_x + icon_dx, building_info_y + VENUE_STOCK_ICON_Y);
    int32_t y = building_info_y + VENUE_STOCK_TEXT_Y;
    g_text_width = 0;
    lang_text_draw_ui(group, text, building_info_x + text_dx, y, image_group(IMAGE_GROUP_FONT_BLACK), 0, building_info_x, building_info_y);
    lang_text_draw_amount(LANG_GROUP_AMOUNTS, AMOUNT_TEXT_UNIT, (amount < 1) ? 0 : amount, g_text_width + text_dx + building_info_x, y, image_group(IMAGE_GROUP_FONT_BLACK), 0);
}

void window_building_draw_senet_house(void)
{
    info_help_id = building_info_help_messages[BUILDING_INFO_HELP_SENET_HOUSE];
    info_first_draw = 0;
    window_building_draw_panel_title(LANG_GROUP_SENET_HOUSE, BUILDING_TEXT_TITLE);
    const building *b = building_info_building();
    draw_venue_stock(RESOURCE_BEER, FIRST_STOCK_ICON_X, FIRST_STOCK_TEXT_X, LANG_GROUP_SENET_HOUSE, SENET_TEXT_STORED_BEER, b->loads_stored);
    if (bi_road_access_block.has_road_access < 1) {
        window_building_draw_no_road_text(building_info_y + BUILDING_INFO_TEXT_Y);
    } else if (b->num_workers < 1) {
        draw_venue_text(LANG_GROUP_SENET_HOUSE, SENET_TEXT_NO_STAFF);
    }
    draw_venue_workers(VENUE_PANEL_BLOCKS);
}

static int32_t zoo_text(const building *b)
{
    if (b->num_workers < 1) {
        return ZOO_TEXT_NO_STAFF;
    }
    if (b->loads_stored < 1) {
        return ZOO_TEXT_NEEDS_GAME_MEAT;
    }
    if (b->second_input_stock < 1) {
        return ZOO_TEXT_NEEDS_STRAW;
    }
    return ZOO_TEXT_WORKING;
}

void window_building_draw_zoo(void)
{
    info_help_id = building_info_help_messages[BUILDING_INFO_HELP_ZOO];
    info_first_draw = 0;
    window_building_draw_panel_title(LANG_GROUP_ZOO, BUILDING_TEXT_TITLE);
    const building *b = building_info_building();
    draw_venue_stock(RESOURCE_GAMEMEAT, FIRST_STOCK_ICON_X, FIRST_STOCK_TEXT_X, LANG_GROUP_ZOO, ZOO_TEXT_GAME_MEAT, b->loads_stored);
    draw_venue_stock(RESOURCE_STRAW, SECOND_STOCK_ICON_X, SECOND_STOCK_TEXT_X, LANG_GROUP_ZOO, ZOO_TEXT_STRAW, b->second_input_stock);
    int32_t y = building_info_y + BUILDING_INFO_SECOND_TEXT_Y;
    if (bi_road_access_block.has_road_access < 1) {
        window_building_draw_no_road_text(y);
    } else {
        window_building_draw_text(LANG_GROUP_ZOO, zoo_text(b), y);
    }
    draw_venue_workers(VENUE_PANEL_BLOCKS);
}

static int32_t pavilion_text(const building *b)
{
    const uint8_t *days = b->data.entertainment.show_days_left;
    int32_t juggling = days[SHOW_JUGGLING] != 0;
    int32_t music = days[SHOW_MUSIC] != 0;
    int32_t dancing = days[SHOW_DANCING] != 0;
    int32_t shows = juggling + music + dancing;
    if (b->num_workers < 1) {
        return PAVILION_TEXT_NO_STAFF;
    }
    if (shows == 0) {
        return PAVILION_TEXT_NO_SHOWS;
    }
    if (shows == 3) {
        return PAVILION_TEXT_ALL_SHOWS;
    }
    if (dancing && music) {
        return PAVILION_TEXT_DANCING_AND_MUSIC;
    }
    if (dancing && juggling) {
        return PAVILION_TEXT_DANCING_AND_JUGGLING;
    }
    if (music && juggling) {
        return PAVILION_TEXT_MUSIC_AND_JUGGLING;
    }
    if (juggling) {
        return PAVILION_TEXT_JUGGLING_ONLY;
    }
    if (music) {
        return PAVILION_TEXT_MUSIC_ONLY;
    }
    return PAVILION_TEXT_DANCING_ONLY;
}

void window_building_draw_pavilion(void)
{
    info_help_id = building_info_help_messages[BUILDING_INFO_HELP_PAVILION];
    info_first_draw = 0;
    window_building_draw_panel_title(LANG_GROUP_PAVILION, BUILDING_TEXT_TITLE);
    const building *b = building_info_building();
    if (bi_road_access_block.has_road_access < 1) {
        window_building_draw_no_road_text(building_info_y + BUILDING_INFO_TEXT_Y);
    } else {
        draw_venue_text(LANG_GROUP_PAVILION, pavilion_text(b));
    }
    draw_venue_workers(VENUE_PANEL_BLOCKS);
    const uint8_t *days = b->data.entertainment.show_days_left;
    draw_show_days(LANG_GROUP_PAVILION, PAVILION_TEXT_NO_JUGGLING, PAVILION_TEXT_JUGGLING_DAYS, days[SHOW_JUGGLING], FIRST_SHOW_Y);
    draw_show_days(LANG_GROUP_PAVILION, PAVILION_TEXT_NO_MUSIC, PAVILION_TEXT_MUSIC_DAYS, days[SHOW_MUSIC], PAVILION_SECOND_SHOW_Y);
    draw_show_days(LANG_GROUP_PAVILION, PAVILION_TEXT_NO_DANCING, PAVILION_TEXT_DANCING_DAYS, days[SHOW_DANCING], PAVILION_THIRD_SHOW_Y);
}

static void draw_school(int32_t group)
{
    info_help_id = building_info_help_messages[BUILDING_INFO_HELP_PERFORMER_SCHOOL];
    info_first_draw = 0;
    window_building_draw_panel_title(group, BUILDING_TEXT_TITLE);
    if (bi_road_access_block.has_road_access < 1) {
        window_building_draw_no_road_text(building_info_y + BUILDING_INFO_TEXT_Y);
    } else {
        const building *b = building_info_building();
        int32_t text = (b->num_workers < 1) ? SCHOOL_TEXT_NO_STAFF : window_building_worker_percentage_text(SCHOOL_TEXT_FULL_STAFF);
        draw_venue_text(group, text);
    }
    window_building_draw_workers_panel();
}

void window_building_draw_conservatory(void)
{
    draw_school(LANG_GROUP_CONSERVATORY);
}

void window_building_draw_dance_school(void)
{
    draw_school(LANG_GROUP_DANCE_SCHOOL);
}

void window_building_draw_juggler_school(void)
{
    draw_school(LANG_GROUP_JUGGLER_SCHOOL);
}

void window_building_draw_bullfight_school(void)
{
    draw_school(LANG_GROUP_BULLFIGHT_SCHOOL);
}
