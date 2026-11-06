// The building info panels of the industry: the work camp, farms (production, fertility, the next harvest or flood),
// reed gatherer, cattle ranch, quarries, mines, wood cutter, clay pit, workshops, guilds and hunting lodge, each with
// the first state text that applies (no road, halted, blighted, no workers, no raw material, else by staffing).
//
// Some state tests look at the wrong good or table:
// - the chariot maker tests the halt of timber, not of chariots;
// - the stonemasons' guild tests its stone, the bricklayers' guild nothing;
// - a building that is not a farm reads its harvest month before the start of the harvest table.

#include "window/building_info_industry.h"
#include "building/building.h"
#include "building/building_data.h"
#include "building/building_industry.h"
#include "building/building_type.h"
#include "city/resource_type.h"
#include "core/calc.h"
#include "game/game_data.h"
#include "game/game_internal.h"
#include "graphics/image_collection.h"
#include "graphics/image_draw.h"
#include "graphics/panel_draw.h"
#include "graphics/text_draw.h"
#include "graphics/ui_layout.h"
#include "map/farm.h"
#include "map/map_data.h"
#include "text/lang.h"
#include "window/building_info.h"
#include "window/building_info_panels.h"
#include "window/window_data.h"
#include "window/window_image_group.h"
#include <stdint.h>

enum producer_text {
    PRODUCER_TEXT_PRODUCTION_IS = 2,
    PRODUCER_TEXT_COMPLETE = 3,
    PRODUCER_TEXT_HALTED = 4,
    PRODUCER_TEXT_NO_WORKERS = 5,
    PRODUCER_TEXT_FULL_STAFF = 6,
    PRODUCER_TEXT_NO_RAW_MATERIAL = 11,
    PRODUCER_TEXT_STORED_RAW_MATERIAL = 12,
    PRODUCER_TEXT_STORED_FIRST_RAW_MATERIAL = 13,
    PRODUCER_TEXT_STORED_SECOND_RAW_MATERIAL = 14,
    PRODUCER_TEXT_NO_SECOND_RAW_MATERIAL = 12,
    PRODUCER_TEXT_STORED = 2,
};

enum farm_text {
    FARM_TEXT_BLIGHTED_BY_LOCUSTS = 11,
    FARM_TEXT_LAND_IS = 12,
    FARM_TEXT_FERTILE = 13,
    FARM_TEXT_NEXT_HARVEST_IN = 14,
    FARMLAND_TEXT_IRRIGATED = 0,
    FARMLAND_TEXT_NOT_IRRIGATED = 1,
    FARMLAND_TEXT_NEXT_FLOODS_IN = 2,
};

enum work_camp_text {
    WORK_CAMP_TEXT_NEEDS_WORKERS = 2,
    WORK_CAMP_TEXT_SUPPLYING_LABOR = 3,
    WORK_CAMP_TEXT_LOOKING_FOR_WORK = 4,
    WORK_CAMP_TEXT_ON_FARMS = 5,
    WORK_CAMP_TEXT_ON_MONUMENTS = 6,
};

enum artisans_guild_text {
    ARTISANS_TEXT_NO_WORKERS = 2,
    ARTISANS_TEXT_FULL_STAFF = 3,
    ARTISANS_TEXT_NO_PAINT = 8,
    ARTISANS_TEXT_NO_CLAY = 9,
    ARTISANS_TEXT_PAINT = 10,
    ARTISANS_TEXT_CLAY = 11,
};

enum industry_lang_group {
    LANG_GROUP_GRAIN_FARM = 112,
    LANG_GROUP_LETTUCE_FARM = 113,
    LANG_GROUP_POMEGRANATE_FARM = 114,
    LANG_GROUP_FLAX_FARM = 115,
    LANG_GROUP_REED_GATHERER = 116,
    LANG_GROUP_CATTLE_RANCH = 117,
    LANG_GROUP_STONE_QUARRY = 118,
    LANG_GROUP_LIMESTONE_QUARRY = 119,
    LANG_GROUP_WOOD_CUTTER = 120,
    LANG_GROUP_CLAY_PIT = 121,
    LANG_GROUP_BREWERY = 122,
    LANG_GROUP_WEAVER = 123,
    LANG_GROUP_WEAPONSMITH = 124,
    LANG_GROUP_JEWELER = 125,
    LANG_GROUP_POTTER = 126,
    LANG_GROUP_HUNTING_LODGE = 154,
    LANG_GROUP_GOLD_MINE = 162,
    LANG_GROUP_GEMSTONE_MINE = 163,
    LANG_GROUP_CARPENTERS_GUILD = 171,
    LANG_GROUP_BRICKLAYERS_GUILD = 172,
    LANG_GROUP_STONEMASONS_GUILD = 173,
    LANG_GROUP_WORK_CAMP = 179,
    LANG_GROUP_BRICKWORKS = 180,
    LANG_GROUP_BARLEY_FARM = 181,
    LANG_GROUP_CHICKPEA_FARM = 182,
    LANG_GROUP_FIG_FARM = 183,
    LANG_GROUP_CHARIOT_WORKSHOP = 185,
    LANG_GROUP_PAPYRUS_MAKER = 190,
    LANG_GROUP_GRANITE_QUARRY = 192,
    LANG_GROUP_COPPER_MINE = 193,
    LANG_GROUP_SANDSTONE_QUARRY = 194,
    LANG_GROUP_HENNA_FARM = 306,
    LANG_GROUP_ARTISANS_GUILD = 312,
    LANG_GROUP_PAINT_MAKER = 313,
    LANG_GROUP_LAMP_MAKER = 314,
};

enum industry_layout {
    TITLE_ICON_X = 10,
    TITLE_ICON_Y = 10,
    WORK_CAMP_TEXT_Y = 56,
    WORK_CAMP_WORKERS_PANEL_Y = 136,
    WORK_CAMP_WORKERS_PANEL_BLOCKS = 6,
    WORK_CAMP_EMPLOYMENT_Y = 138,
    WORK_CAMP_DESCRIPTION_FROM_BOTTOM = 105,
    FARM_CROP_FROM_BOTTOM = 158,
    FARM_IRRIGATION_FROM_BOTTOM = 143,
    PRODUCTION_Y = 44,
    WORKSHOP_PRODUCTION_Y = 40,
    PRODUCER_STATE_Y = 70,
    WORKSHOP_STATE_Y = 86,
    DESCRIPTION_FROM_BOTTOM = 113,
    STOCK_ICON_X = 32,
    STOCK_TEXT_X = 60,
    SECOND_STOCK_ICON_X = 230,
    SECOND_STOCK_TEXT_X = 260,
    STOCK_ICON_DY = -4,
    REED_STOCK_Y = 54,
    CATTLE_STOCK_Y = 100,
    WORKSHOP_STOCK_Y = 60,
    LOCUST_BLIGHT_DAYS = 5,
    CATTLE_RANCH_PROGRESS_MAX = 400,
    WORKSHOP_PROGRESS_MAX = 400,
    RAW_PRODUCER_PROGRESS_MAX = 200,
};

static void draw_line(int32_t group, int32_t index, int32_t y)
{
    lang_text_draw_ui(group, index, g_text_width + BUILDING_INFO_TEXT_X + building_info_x, y, image_group(IMAGE_GROUP_FONT_BLACK), 0, building_info_x, building_info_y);
}

static void draw_number(int32_t value, const char *postfix, int32_t y)
{
    text_draw_number_ui(value, NUMBER_PREFIX_NONE, postfix, g_text_width + BUILDING_INFO_TEXT_X + building_info_x, y, image_group(IMAGE_GROUP_FONT_BLACK), 0, building_info_x, building_info_y);
}

static void draw_title_with_icon(int32_t group, int32_t icon)
{
    outer_panel_draw(building_info_x, building_info_y, info_w_blocks, building_info_h);
    if (icon != RESOURCE_NONE) {
        image_draw(image_group(IMAGE_GROUP_RESOURCE_ICONS) + icon, building_info_x + TITLE_ICON_X, building_info_y + TITLE_ICON_Y);
    }
    lang_text_draw_centered_ui(group, BUILDING_TEXT_TITLE, building_info_x, building_info_y + BUILDING_INFO_TITLE_Y, info_w_blocks * BUILDING_INFO_BLOCK,
        image_group(IMAGE_GROUP_FONT_LARGE), 0, building_info_x, building_info_y);
}

static void draw_description(int32_t group)
{
    window_building_draw_text(group, BUILDING_TEXT_DESCRIPTION, (building_info_h * BUILDING_INFO_BLOCK) - DESCRIPTION_FROM_BOTTOM + building_info_y);
}

static void draw_production(int32_t group, int32_t percentage, int32_t y)
{
    g_text_width = 0;
    lang_text_draw_ui(group, PRODUCER_TEXT_PRODUCTION_IS, building_info_x + BUILDING_INFO_TEXT_X, y, image_group(IMAGE_GROUP_FONT_BLACK), 0, building_info_x, building_info_y);
    draw_number(percentage, pct_postfix, y);
    draw_line(group, PRODUCER_TEXT_COMPLETE, y);
}

static void draw_stock_line(int32_t icon, int32_t group, int32_t index, int32_t icon_x, int32_t text_x, int32_t y, int32_t amount)
{
    image_draw(image_group(IMAGE_GROUP_RESOURCE_ICONS) + icon, building_info_x + icon_x, building_info_y + y + STOCK_ICON_DY);
    g_text_width = 0;
    lang_text_draw_ui(group, index, building_info_x + text_x, building_info_y + y, image_group(IMAGE_GROUP_FONT_BLACK), 0, building_info_x, building_info_y);
    lang_text_draw_amount(
        LANG_GROUP_AMOUNTS, AMOUNT_TEXT_UNIT, (amount < 1) ? 0 : amount, g_text_width + text_x + building_info_x, building_info_y + y, image_group(IMAGE_GROUP_FONT_BLACK), 0);
}

static int32_t is_halted(int32_t resource)
{
    return city_player_current()->mothballed[resource] != 0;
}

static int32_t work_camp_text(const building *b)
{
    if (b->data.industry.worker_count == 0) {
        return WORK_CAMP_TEXT_SUPPLYING_LABOR;
    }
    int32_t on_farms = b->data.industry.labor_days_left[0] != 0;
    int32_t on_monuments = b->data.industry.labor_days_left[1] != 0;
    return WORK_CAMP_TEXT_LOOKING_FOR_WORK + on_farms + (on_monuments * (WORK_CAMP_TEXT_ON_MONUMENTS - WORK_CAMP_TEXT_LOOKING_FOR_WORK));
}

void window_building_draw_work_camp(void)
{
    const building *b = building_info_building();
    info_help_id = building_info_help_messages[BUILDING_INFO_HELP_WORK_CAMP];
    info_first_draw = 0;
    window_building_draw_panel_title(LANG_GROUP_WORK_CAMP, BUILDING_TEXT_TITLE);
    int32_t y = building_info_y + WORK_CAMP_TEXT_Y;
    if (bi_road_access_block.has_road_access < 1) {
        window_building_draw_no_road_text(y);
    } else if (b->num_workers < 1) {
        window_building_draw_text(LANG_GROUP_WORK_CAMP, WORK_CAMP_TEXT_NEEDS_WORKERS, y);
    } else {
        window_building_draw_text(LANG_GROUP_WORK_CAMP, work_camp_text(b), y);
    }
    window_building_draw_inner_panel(WORK_CAMP_WORKERS_PANEL_Y, WORK_CAMP_WORKERS_PANEL_BLOCKS);
    window_building_draw_employment(building_info_y + WORK_CAMP_EMPLOYMENT_Y);
    window_building_draw_text(
        LANG_GROUP_WORK_CAMP, BUILDING_TEXT_DESCRIPTION, (building_info_h * BUILDING_INFO_BLOCK) - WORK_CAMP_DESCRIPTION_FROM_BOTTOM + building_info_y);
}

void window_building_draw_farm_crop(int32_t group)
{
    int32_t month = farm_next_harvest_month(building_info_building_id);
    if (month >= 0) {
        g_text_width = 0;
        int32_t text = FARM_TEXT_NEXT_HARVEST_IN;
        if (building_is_on_floodplain(building_info_building_id) != 0) {
            text = FARMLAND_TEXT_NEXT_FLOODS_IN;
            group = LANG_GROUP_FARM;
        }
        int32_t y = (building_info_h * BUILDING_INFO_BLOCK) - FARM_CROP_FROM_BOTTOM + building_info_y;
        draw_line(group, text, y);
        draw_line(LANG_GROUP_MONTHS, month, y);
    }
    if (building_is_crop_farm(building_info_building_id) != 0) {
        g_text_width = 0;
        int32_t text = (farm_has_water_access_of(building_info_building_id) != 0) ? FARMLAND_TEXT_IRRIGATED : FARMLAND_TEXT_NOT_IRRIGATED;
        draw_line(LANG_GROUP_FARM, text, (building_info_h * BUILDING_INFO_BLOCK) - FARM_IRRIGATION_FROM_BOTTOM + building_info_y);
    }
}

static void draw_producer_state(int32_t group, int32_t stock_resource, int32_t can_be_blighted, int32_t y)
{
    if (bi_road_access_block.has_road_access < 1) {
        window_building_draw_no_road_text(y);
        return;
    }
    const building *b = building_info_building();
    int32_t text;
    if (is_halted(stock_resource)) {
        text = PRODUCER_TEXT_HALTED;
    } else if (can_be_blighted && b->data.industry.curse_days_left >= LOCUST_BLIGHT_DAYS) {
        text = FARM_TEXT_BLIGHTED_BY_LOCUSTS;
    } else if (b->num_workers < 1) {
        text = PRODUCER_TEXT_NO_WORKERS;
    } else {
        text = window_building_worker_percentage_text(PRODUCER_TEXT_FULL_STAFF);
    }
    window_building_draw_text(group, text, y);
}

static void draw_farm(int32_t help, int32_t resource, int32_t group)
{
    info_help_id = building_info_help_messages[help];
    info_first_draw = 0;
    draw_title_with_icon(group, resource);
    int32_t y = building_info_y + PRODUCTION_Y;
    draw_production(group, farm_growth_percentage_of(building_info_building_id), y);
    draw_line(LANG_GROUP_GRAIN_FARM, FARM_TEXT_LAND_IS, y);
    int32_t fertility_x = g_text_width + BUILDING_INFO_TEXT_X + building_info_x;
    int32_t font = image_group(IMAGE_GROUP_FONT_BLACK);
    int32_t clip_x = building_info_x;
    int32_t clip_y = building_info_y;
    int32_t fertility = building_farm_fertility(building_info_building_id);
    text_draw_number_ui(fertility, NUMBER_PREFIX_NONE, pct_postfix, fertility_x, y, font, 0, clip_x, clip_y);
    draw_line(LANG_GROUP_GRAIN_FARM, FARM_TEXT_FERTILE, y);
    draw_producer_state(group, resource, 1, building_info_y + PRODUCER_STATE_Y);
    window_building_draw_inner_panel(BUILDING_INFO_WORKERS_PANEL_Y, BUILDING_INFO_WORKERS_PANEL_BLOCKS);
    window_building_draw_farm_workers(building_info_y + BUILDING_INFO_EMPLOYMENT_Y);
    window_building_draw_farm_crop(group);
    draw_description(group);
}

void window_building_draw_barley_farm(void)
{
    draw_farm(BUILDING_INFO_HELP_BARLEY_FARM, RESOURCE_BARLEY, LANG_GROUP_BARLEY_FARM);
}

void window_building_draw_grain_farm(void)
{
    draw_farm(BUILDING_INFO_HELP_GRAIN_FARM, RESOURCE_GRAIN, LANG_GROUP_GRAIN_FARM);
}

void window_building_draw_lettuce_farm(void)
{
    draw_farm(BUILDING_INFO_HELP_LETTUCE_FARM, RESOURCE_LETTUCE, LANG_GROUP_LETTUCE_FARM);
}

void window_building_draw_fig_farm(void)
{
    draw_farm(BUILDING_INFO_HELP_FIG_FARM, RESOURCE_FIGS, LANG_GROUP_FIG_FARM);
}

void window_building_draw_pomegranate_farm(void)
{
    draw_farm(BUILDING_INFO_HELP_POMEGRANATE_FARM, RESOURCE_POMEGRANATES, LANG_GROUP_POMEGRANATE_FARM);
}

void window_building_draw_flax_farm(void)
{
    draw_farm(BUILDING_INFO_HELP_FLAX_FARM, RESOURCE_FLAX, LANG_GROUP_FLAX_FARM);
}

void window_building_draw_henna_farm(void)
{
    draw_farm(BUILDING_INFO_HELP_HENNA_FARM, RESOURCE_HENNA, LANG_GROUP_HENNA_FARM);
}

void window_building_draw_chickpea_farm(void)
{
    draw_farm(BUILDING_INFO_HELP_CHICKPEA_FARM, RESOURCE_CHICKPEAS, LANG_GROUP_CHICKPEA_FARM);
}

void window_building_draw_reed_gatherer(void)
{
    info_help_id = building_info_help_messages[BUILDING_INFO_HELP_REED_GATHERER];
    info_first_draw = 0;
    draw_title_with_icon(LANG_GROUP_REED_GATHERER, RESOURCE_REEDS);
    draw_stock_line(RESOURCE_REEDS, LANG_GROUP_REED_GATHERER, PRODUCER_TEXT_STORED, STOCK_ICON_X, STOCK_TEXT_X, REED_STOCK_Y, building_info_building()->loads_stored);
    draw_producer_state(LANG_GROUP_REED_GATHERER, RESOURCE_REEDS, 1, building_info_y + PRODUCER_STATE_Y);
    window_building_draw_workers_panel();
    draw_description(LANG_GROUP_REED_GATHERER);
}

void window_building_draw_cattle_ranch(void)
{
    const building *b = building_info_building();
    info_help_id = building_info_help_messages[BUILDING_INFO_HELP_CATTLE_RANCH];
    info_first_draw = 0;
    draw_title_with_icon(LANG_GROUP_CATTLE_RANCH, RESOURCE_MEAT);
    draw_production(LANG_GROUP_GRAIN_FARM, calc_percentage(b->data.industry.progress, CATTLE_RANCH_PROGRESS_MAX), building_info_y + PRODUCTION_Y);
    draw_stock_line(RESOURCE_STRAW, LANG_GROUP_RESOURCE_NAMES, RESOURCE_STRAW, STOCK_ICON_X, STOCK_TEXT_X, CATTLE_STOCK_Y, b->loads_stored);
    draw_producer_state(LANG_GROUP_CATTLE_RANCH, RESOURCE_MEAT, 1, building_info_y + PRODUCER_STATE_Y);
    window_building_draw_workers_panel();
    draw_description(LANG_GROUP_CATTLE_RANCH);
}

static void draw_raw_producer(int32_t help, int32_t resource, int32_t group, int32_t shows_stock)
{
    const building *b = building_info_building();
    info_help_id = building_info_help_messages[help];
    info_first_draw = 0;
    draw_title_with_icon(group, resource);
    int32_t y = building_info_y + PRODUCTION_Y;
    if (shows_stock) {
        g_text_width = 0;
        lang_text_draw_ui(group, PRODUCER_TEXT_STORED, building_info_x + BUILDING_INFO_TEXT_X, y, image_group(IMAGE_GROUP_FONT_BLACK), 0, building_info_x, building_info_y);
        draw_number(b->loads_stored, empty_text, y);
        draw_line(group, PRODUCER_TEXT_COMPLETE, y);
    } else {
        draw_production(group, calc_percentage(b->data.industry.progress, RAW_PRODUCER_PROGRESS_MAX), y);
    }
    draw_producer_state(group, resource, 0, building_info_y + PRODUCER_STATE_Y);
    window_building_draw_workers_panel();
    draw_description(group);
}

void window_building_draw_stone_quarry(void)
{
    draw_raw_producer(BUILDING_INFO_HELP_STONE_QUARRY, RESOURCE_STONE, LANG_GROUP_STONE_QUARRY, 0);
}

static int32_t quarry_group(int32_t resource)
{
    if (resource == RESOURCE_LIMESTONE) {
        return LANG_GROUP_LIMESTONE_QUARRY;
    }
    return (resource == RESOURCE_GRANITE) ? LANG_GROUP_GRANITE_QUARRY : LANG_GROUP_SANDSTONE_QUARRY;
}

void window_building_draw_quarry(int32_t resource)
{
    draw_raw_producer(BUILDING_INFO_HELP_QUARRY, resource, quarry_group(resource), 0);
}

void window_building_draw_gold_mine(void)
{
    draw_raw_producer(BUILDING_INFO_HELP_GOLD_MINE, RESOURCE_GOLD, LANG_GROUP_GOLD_MINE, 0);
}

void window_building_draw_copper_mine(void)
{
    draw_raw_producer(BUILDING_INFO_HELP_COPPER_MINE, RESOURCE_COPPER, LANG_GROUP_COPPER_MINE, 0);
}

void window_building_draw_gemstone_mine(void)
{
    draw_raw_producer(BUILDING_INFO_HELP_GEMSTONE_MINE, RESOURCE_GEMS, LANG_GROUP_GEMSTONE_MINE, 0);
}

void window_building_draw_wood_cutter(void)
{
    draw_raw_producer(BUILDING_INFO_HELP_WOOD_CUTTER, RESOURCE_TIMBER, LANG_GROUP_WOOD_CUTTER, 1);
}

void window_building_draw_clay_pit(void)
{
    draw_raw_producer(BUILDING_INFO_HELP_CLAY_PIT, RESOURCE_CLAY, LANG_GROUP_CLAY_PIT, 0);
}

static void draw_workshop_state(int32_t group, int32_t halt_resource, int32_t no_workers, int32_t no_raw, int32_t no_raw2, int32_t full_staff)
{
    int32_t y = building_info_y + WORKSHOP_STATE_Y;
    if (bi_road_access_block.has_road_access < 1) {
        window_building_draw_no_road_text(y);
        return;
    }
    const building *b = building_info_building();
    int32_t text;
    if (halt_resource != RESOURCE_NONE && is_halted(halt_resource)) {
        text = PRODUCER_TEXT_HALTED;
    } else if (b->num_workers < 1) {
        text = no_workers;
    } else if (no_raw >= 0 && b->loads_stored < 1) {
        text = no_raw;
    } else if (no_raw2 >= 0 && b->second_input_stock < 1) {
        text = no_raw2;
    } else {
        text = window_building_worker_percentage_text(full_staff);
    }
    window_building_draw_text(group, text, y);
}

static void draw_workshop(int32_t help, int32_t product, int32_t group, int32_t raw, int32_t second_raw, int32_t halt_resource)
{
    const building *b = building_info_building();
    info_help_id = building_info_help_messages[help];
    info_first_draw = 0;
    draw_title_with_icon(group, product);
    draw_production(group, calc_percentage(b->data.industry.progress, WORKSHOP_PROGRESS_MAX), building_info_y + WORKSHOP_PRODUCTION_Y);
    int32_t has_second_raw = second_raw != RESOURCE_NONE;
    int32_t raw_text = has_second_raw ? PRODUCER_TEXT_STORED_FIRST_RAW_MATERIAL : PRODUCER_TEXT_STORED_RAW_MATERIAL;
    draw_stock_line(raw, group, raw_text, STOCK_ICON_X, STOCK_TEXT_X, WORKSHOP_STOCK_Y, b->loads_stored);
    if (has_second_raw) {
        draw_stock_line(
            second_raw, group, PRODUCER_TEXT_STORED_SECOND_RAW_MATERIAL, SECOND_STOCK_ICON_X, SECOND_STOCK_TEXT_X, WORKSHOP_STOCK_Y, b->second_input_stock);
    }
    draw_workshop_state(group, halt_resource, PRODUCER_TEXT_NO_WORKERS, PRODUCER_TEXT_NO_RAW_MATERIAL,
        has_second_raw ? PRODUCER_TEXT_NO_SECOND_RAW_MATERIAL : -1, PRODUCER_TEXT_FULL_STAFF);
    window_building_draw_workers_panel();
}

void window_building_draw_brewery(void)
{
    draw_workshop(BUILDING_INFO_HELP_BREWERY, RESOURCE_BEER, LANG_GROUP_BREWERY, RESOURCE_BARLEY, RESOURCE_NONE, RESOURCE_BEER);
}

void window_building_draw_weaver(void)
{
    draw_workshop(BUILDING_INFO_HELP_WEAVER, RESOURCE_LINEN, LANG_GROUP_WEAVER, RESOURCE_FLAX, RESOURCE_NONE, RESOURCE_LINEN);
}

void window_building_draw_papyrus_maker(void)
{
    draw_workshop(BUILDING_INFO_HELP_PAPYRUS_MAKER, RESOURCE_PAPYRUS, LANG_GROUP_PAPYRUS_MAKER, RESOURCE_REEDS, RESOURCE_NONE, RESOURCE_PAPYRUS);
}

void window_building_draw_lamp_maker(void)
{
    draw_workshop(BUILDING_INFO_HELP_LAMP_MAKER, RESOURCE_LAMPS, LANG_GROUP_LAMP_MAKER, RESOURCE_OIL, RESOURCE_POTTERY, RESOURCE_LAMPS);
}

void window_building_draw_paint_maker(void)
{
    draw_workshop(BUILDING_INFO_HELP_PAINT_MAKER, RESOURCE_PAINT, LANG_GROUP_PAINT_MAKER, RESOURCE_HENNA, RESOURCE_NONE, RESOURCE_PAINT);
}

void window_building_draw_chariot_maker(void)
{
    draw_workshop(BUILDING_INFO_HELP_CHARIOT_MAKER, RESOURCE_CHARIOTS, LANG_GROUP_CHARIOT_WORKSHOP, RESOURCE_TIMBER, RESOURCE_NONE, RESOURCE_TIMBER);
}

void window_building_draw_brickworks(void)
{
    draw_workshop(BUILDING_INFO_HELP_BRICKWORKS, RESOURCE_BRICKS, LANG_GROUP_BRICKWORKS, RESOURCE_CLAY, RESOURCE_STRAW, RESOURCE_BRICKS);
}

void window_building_draw_weaponsmith(void)
{
    draw_workshop(BUILDING_INFO_HELP_WEAPONSMITH, RESOURCE_WEAPONS, LANG_GROUP_WEAPONSMITH, RESOURCE_COPPER, RESOURCE_NONE, RESOURCE_WEAPONS);
}

void window_building_draw_jeweler(void)
{
    draw_workshop(BUILDING_INFO_HELP_JEWELER, RESOURCE_LUXURY_GOODS, LANG_GROUP_JEWELER, RESOURCE_GEMS, RESOURCE_NONE, RESOURCE_LUXURY_GOODS);
}

void window_building_draw_potter(void)
{
    draw_workshop(BUILDING_INFO_HELP_POTTER, RESOURCE_POTTERY, LANG_GROUP_POTTER, RESOURCE_CLAY, RESOURCE_NONE, RESOURCE_POTTERY);
}

void window_building_draw_carpenters_guild(void)
{
    draw_workshop(BUILDING_INFO_HELP_CARPENTERS_GUILD, RESOURCE_TIMBER, LANG_GROUP_CARPENTERS_GUILD, RESOURCE_TIMBER, RESOURCE_NONE, RESOURCE_NONE);
}

static void draw_guild(int32_t help, int32_t icon, int32_t group, int32_t no_raw)
{
    info_help_id = building_info_help_messages[help];
    info_first_draw = 0;
    draw_title_with_icon(group, icon);
    draw_workshop_state(group, RESOURCE_NONE, PRODUCER_TEXT_NO_WORKERS, no_raw, -1, PRODUCER_TEXT_FULL_STAFF);
    window_building_draw_workers_panel();
}

void window_building_draw_bricklayers_guild(void)
{
    draw_guild(BUILDING_INFO_HELP_BRICKLAYERS_GUILD, RESOURCE_BRICKS, LANG_GROUP_BRICKLAYERS_GUILD, -1);
}

void window_building_draw_stonemasons_guild(void)
{
    draw_guild(BUILDING_INFO_HELP_STONEMASONS_GUILD, RESOURCE_STONE, LANG_GROUP_STONEMASONS_GUILD, PRODUCER_TEXT_NO_RAW_MATERIAL);
}

void window_building_draw_artisans_guild(void)
{
    const building *b = building_info_building();
    info_help_id = building_info_help_messages[BUILDING_INFO_HELP_ARTISANS_GUILD];
    info_first_draw = 0;
    draw_title_with_icon(LANG_GROUP_ARTISANS_GUILD, RESOURCE_NONE);
    g_text_width = 0;
    draw_stock_line(RESOURCE_PAINT, LANG_GROUP_ARTISANS_GUILD, ARTISANS_TEXT_PAINT, STOCK_ICON_X, STOCK_TEXT_X, WORKSHOP_STOCK_Y, b->loads_stored);
    draw_stock_line(
        RESOURCE_CLAY, LANG_GROUP_ARTISANS_GUILD, ARTISANS_TEXT_CLAY, SECOND_STOCK_ICON_X, SECOND_STOCK_TEXT_X, WORKSHOP_STOCK_Y, b->second_input_stock);
    draw_workshop_state(
        LANG_GROUP_ARTISANS_GUILD, RESOURCE_NONE, ARTISANS_TEXT_NO_WORKERS, ARTISANS_TEXT_NO_PAINT, ARTISANS_TEXT_NO_CLAY, ARTISANS_TEXT_FULL_STAFF);
    window_building_draw_workers_panel();
}

void window_building_draw_hunting_lodge(void)
{
    info_help_id = building_info_help_messages[BUILDING_INFO_HELP_HUNTING_LODGE];
    info_first_draw = 0;
    draw_title_with_icon(LANG_GROUP_HUNTING_LODGE, RESOURCE_NONE);
    draw_stock_line(RESOURCE_GAMEMEAT, LANG_GROUP_HUNTING_LODGE, PRODUCER_TEXT_STORED_FIRST_RAW_MATERIAL, STOCK_ICON_X, STOCK_TEXT_X, WORKSHOP_STOCK_Y,
        building_info_building()->loads_stored);
    draw_workshop_state(LANG_GROUP_HUNTING_LODGE, RESOURCE_GAMEMEAT, PRODUCER_TEXT_NO_WORKERS, PRODUCER_TEXT_NO_RAW_MATERIAL, -1, PRODUCER_TEXT_FULL_STAFF);
    window_building_draw_workers_panel();
}

static int32_t farm_crop_row(int32_t building_id)
{
    return (building_is_farm(building_id) != 0) ? building_farm_crop(building_get(building_id)->type) : FARM_CROP_NONE;
}

int32_t farm_next_harvest_month(int32_t building_id)
{
    int32_t crop = farm_crop_row(building_id);
    if (building_is_on_floodplain(building_id)) {
        return floods_data.season_month;
    }
    int32_t nearest_months_ahead = MONTHS_PER_YEAR;
    int32_t month = -1;
    for (int32_t i = 0; i < FARM_HARVESTS_PER_CROP; i++) {
        int32_t harvest_month = farm_harvest_months[(crop * FARM_HARVESTS_PER_CROP) + i];
        if (harvest_month == -1) {
            break;
        }
        int32_t months_ahead = harvest_month - game_time_month;
        if (months_ahead <= 0) {
            months_ahead += MONTHS_PER_YEAR;
        }
        if (months_ahead < nearest_months_ahead) {
            nearest_months_ahead = months_ahead;
            month = harvest_month;
        }
    }
    return month;
}
