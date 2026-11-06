// The parts the panels of the building info window share: the outer panel with the title, text blocks, the inner
// panel with the workers line (and the reason for missing workers), the stock line, the service and walker states,
// the right-click hint, the farms' workers line and the goods a trade figure carries.
//
// A carried good outside the house inventory slots shows the icon before the first resource.

#include "window/building_info_panels.h"
#include "building/building.h"
#include "building/building_industry.h"
#include "building/house_evolve.h"
#include "city/resource_type.h"
#include "figure/figure.h"
#include "game/game_internal.h"
#include "graphics/image_collection.h"
#include "graphics/image_draw.h"
#include "graphics/panel_draw.h"
#include "graphics/text_draw.h"
#include "graphics/ui_layout.h"
#include "text/lang.h"
#include "window/building_info.h"
#include "window/window_data.h"
#include "window/window_image_group.h"
#include <stdint.h>

enum {
    GOOD_LABOR_ACCESS_HOUSES = 40,
    FIGURE_GOODS_SLOTS = 4,
    FIGURE_GOODS_ICON_SPACING = 32,
    EMPLOYMENT_ICON_X = 40,
    EMPLOYMENT_ICON_Y = 6,
    EMPLOYMENT_ICON = 14,
    EMPLOYMENT_X = 60,
    EMPLOYMENT_NEEDED_X = 70,
    EMPLOYMENT_TEXT_Y = 10,
    EMPLOYMENT_SINGLE_LINE_Y = 16,
    EMPLOYMENT_NOTE_Y = 26,
    EMPLOYMENT_NOTE_WIDTH = 400,
    FARM_TEXT_X = 65,
    FARM_TEXT_NEEDS_LABORERS = 5,
    FARM_TEXT_FULL_CREW = 6,
    RIGHT_CLICK_HINT_FROM_BOTTOM = 24,
};

int32_t building_info_help_messages[BUILDING_INFO_HELP_COUNT] = {
    [BUILDING_INFO_HELP_MISSION_POST] = 8,
    [BUILDING_INFO_HELP_SCRIBAL_SCHOOL] = 68,
    [BUILDING_INFO_HELP_LIBRARY] = 70,
    [BUILDING_INFO_HELP_ARCHITECT_POST] = 81,
    [BUILDING_INFO_HELP_POLICE_STATION] = 86,
    [BUILDING_INFO_HELP_FIREHOUSE] = 355,
    [BUILDING_INFO_HELP_WATER_LIFT] = 59,
    [BUILDING_INFO_HELP_WATER_SUPPLY] = 61,
    [BUILDING_INFO_HELP_WELL] = 62,
    [BUILDING_INFO_HELP_APOTHECARY] = 65,
    [BUILDING_INFO_HELP_PHYSICIAN] = 64,
    [BUILDING_INFO_HELP_DENTIST] = 63,
    [BUILDING_INFO_HELP_MORTUARY] = 66,
    [BUILDING_INFO_HELP_PALACE] = 77,
    [BUILDING_INFO_HELP_COURTHOUSE] = 76,
    [BUILDING_INFO_HELP_TAX_COLLECTOR] = 80,
    [BUILDING_INFO_HELP_MANSION] = 78,
    [BUILDING_INFO_HELP_GARDENS] = 79,
    [BUILDING_INFO_HELP_WALLS] = 85,
    [BUILDING_INFO_HELP_IRRIGATION_DITCH] = 59,
    [BUILDING_INFO_HELP_PLAZA] = 79,
    [BUILDING_INFO_HELP_STATUE] = 79,
    [BUILDING_INFO_HELP_BANDSTAND] = 72,
    [BUILDING_INFO_HELP_BOOTH] = 71,
    [BUILDING_INFO_HELP_SENET_HOUSE] = 74,
    [BUILDING_INFO_HELP_ZOO] = 479,
    [BUILDING_INFO_HELP_PAVILION] = 73,
    [BUILDING_INFO_HELP_PERFORMER_SCHOOL] = 75,
    [BUILDING_INFO_HELP_ORACLE] = 67,
    [BUILDING_INFO_HELP_TEMPLE] = 67,
    [BUILDING_INFO_HELP_FESTIVAL_SQUARE] = 366,
    [BUILDING_INFO_HELP_BAZAAR] = 2,
    [BUILDING_INFO_HELP_GRANARY] = 3,
    [BUILDING_INFO_HELP_STORAGE_YARD] = 4,
    [BUILDING_INFO_HELP_HOUSE] = 56,
    [BUILDING_INFO_HELP_WORK_CAMP] = 8,
    [BUILDING_INFO_HELP_GRAIN_FARM] = 89,
    [BUILDING_INFO_HELP_BARLEY_FARM] = 89,
    [BUILDING_INFO_HELP_LETTUCE_FARM] = 90,
    [BUILDING_INFO_HELP_FIG_FARM] = 90,
    [BUILDING_INFO_HELP_POMEGRANATE_FARM] = 90,
    [BUILDING_INFO_HELP_CHICKPEA_FARM] = 90,
    [BUILDING_INFO_HELP_FLAX_FARM] = 91,
    [BUILDING_INFO_HELP_HENNA_FARM] = 91,
    [BUILDING_INFO_HELP_REED_GATHERER] = 94,
    [BUILDING_INFO_HELP_CATTLE_RANCH] = 360,
    [BUILDING_INFO_HELP_STONE_QUARRY] = 95,
    [BUILDING_INFO_HELP_QUARRY] = 95,
    [BUILDING_INFO_HELP_GOLD_MINE] = 93,
    [BUILDING_INFO_HELP_COPPER_MINE] = 93,
    [BUILDING_INFO_HELP_GEMSTONE_MINE] = 361,
    [BUILDING_INFO_HELP_WOOD_CUTTER] = 94,
    [BUILDING_INFO_HELP_CLAY_PIT] = 92,
    [BUILDING_INFO_HELP_BREWERY] = 96,
    [BUILDING_INFO_HELP_WEAVER] = 60,
    [BUILDING_INFO_HELP_PAPYRUS_MAKER] = 97,
    [BUILDING_INFO_HELP_LAMP_MAKER] = 473,
    [BUILDING_INFO_HELP_PAINT_MAKER] = 470,
    [BUILDING_INFO_HELP_CHARIOT_MAKER] = 98,
    [BUILDING_INFO_HELP_BRICKWORKS] = 364,
    [BUILDING_INFO_HELP_WEAPONSMITH] = 98,
    [BUILDING_INFO_HELP_JEWELER] = 99,
    [BUILDING_INFO_HELP_POTTER] = 1,
    [BUILDING_INFO_HELP_CARPENTERS_GUILD] = 363,
    [BUILDING_INFO_HELP_BRICKLAYERS_GUILD] = 363,
    [BUILDING_INFO_HELP_STONEMASONS_GUILD] = 363,
    [BUILDING_INFO_HELP_ARTISANS_GUILD] = 472,
    [BUILDING_INFO_HELP_HUNTING_LODGE] = 359,
    [BUILDING_INFO_HELP_LEGION] = 87,
    [BUILDING_INFO_HELP_FORT] = 37,
    [BUILDING_INFO_HELP_GATEHOUSE] = 85,
    [BUILDING_INFO_HELP_ROADBLOCK] = 358,
    [BUILDING_INFO_HELP_TOWER] = 85,
    [BUILDING_INFO_HELP_MILITARY_ACADEMY] = 88,
    [BUILDING_INFO_HELP_RECRUITER] = 88,
    [BUILDING_INFO_HELP_TRANSPORT_SHIP] = 367,
    [BUILDING_INFO_HELP_WARSHIP] = 365,
    [BUILDING_INFO_HELP_SHIPWRIGHT] = 82,
    [BUILDING_INFO_HELP_DOCK] = 83,
    [BUILDING_INFO_HELP_FISHING_WHARF] = 84,
    [BUILDING_INFO_HELP_TRANSPORT_WHARF] = 357,
    [BUILDING_INFO_HELP_WARSHIP_WHARF] = 356,
    [BUILDING_INFO_HELP_FERRY_LANDING] = 58,
    [BUILDING_INFO_HELP_SPHINX] = 362,
    [BUILDING_INFO_HELP_OBELISK] = 372,
    [BUILDING_INFO_HELP_SUN_TEMPLE] = 69,
    [BUILDING_INFO_HELP_LIBRARY_MONUMENT] = 481,
    [BUILDING_INFO_HELP_ABU_SIMBEL] = 492,
    [BUILDING_INFO_HELP_CAESAREUM] = 482,
    [BUILDING_INFO_HELP_LIGHTHOUSE] = 487,
    [BUILDING_INFO_HELP_ROYAL_TOMB] = 478,
    [BUILDING_INFO_HELP_MAUSOLEUM] = 368,
    [BUILDING_INFO_HELP_WINDOW] = 10,
    [BUILDING_INFO_HELP_ROAD] = 57,
    [BUILDING_INFO_HELP_BRIDGE] = 58,
    [BUILDING_INFO_HELP_FIGURE] = 42,
    [BUILDING_INFO_HELP_FIGURE_FROM_SHIPWRECK_ON] = 330,
};

int32_t window_building_worker_percentage_text(int32_t full_staff_text)
{
    int32_t percentage = bi_road_access_block.worker_percentage;
    if (percentage >= 100) {
        return full_staff_text;
    }
    if (percentage >= 75) {
        return full_staff_text + 1;
    }
    if (percentage >= 50) {
        return full_staff_text + 2;
    }
    if (percentage >= 25) {
        return full_staff_text + 3;
    }
    return full_staff_text + 4;
}

void window_building_draw_panel_title_at(int32_t group, int32_t index, int32_t dy)
{
    outer_panel_draw(building_info_x, building_info_y, info_w_blocks, building_info_h);
    lang_text_draw_centered_ui(
        group, index, building_info_x, building_info_y + dy, info_w_blocks * BUILDING_INFO_BLOCK, image_group(IMAGE_GROUP_FONT_LARGE), 0, building_info_x, building_info_y);
}

void window_building_draw_panel_title(int32_t group, int32_t index)
{
    window_building_draw_panel_title_at(group, index, BUILDING_INFO_TITLE_Y);
}

void window_building_draw_text(int32_t group, int32_t index, int32_t y)
{
    lang_text_draw_multiline_ui(
        group, index, building_info_x + BUILDING_INFO_TEXT_X, y, (info_w_blocks - 4) * BUILDING_INFO_BLOCK, image_group(IMAGE_GROUP_FONT_BLACK), 0, building_info_x, building_info_y);
}

void window_building_draw_no_road_text(int32_t y)
{
    window_building_draw_text(LANG_GROUP_BUILDING_INFO, BUILDING_INFO_TEXT_NOT_ADJACENT_TO_ROAD, y);
}

void window_building_draw_inner_panel(int32_t dy, int32_t height_blocks)
{
    inner_panel_draw_ui(building_info_x + BUILDING_INFO_BLOCK, building_info_y + dy, info_w_blocks - 2, height_blocks, building_info_x, building_info_y);
}

void window_building_draw_workers_panel(void)
{
    window_building_draw_inner_panel(BUILDING_INFO_WORKERS_PANEL_Y, BUILDING_INFO_WORKERS_PANEL_BLOCKS);
    window_building_draw_employment(building_info_y + BUILDING_INFO_EMPLOYMENT_Y);
}

void window_building_draw_service_state(int32_t group, int32_t working_text)
{
    int32_t y = building_info_y + BUILDING_INFO_TEXT_Y;
    if (bi_road_access_block.has_road_access < 1) {
        window_building_draw_no_road_text(y);
    } else {
        const building *b = building_info_building();
        window_building_draw_text(group, (b->num_workers < 1) ? SERVICE_TEXT_NOT_WORKING : working_text, y);
    }
}

void window_building_draw_walker_state(int32_t group)
{
    int32_t y = building_info_y + BUILDING_INFO_TEXT_Y;
    if (bi_road_access_block.has_road_access < 1) {
        window_building_draw_no_road_text(y);
        return;
    }
    const building *b = building_info_building();
    if (b->num_workers == 0) {
        window_building_draw_text(group, WALKER_TEXT_NO_STAFF, y);
        return;
    }
    window_building_draw_text(group, (b->figure_id == 0) ? WALKER_TEXT_PREPARING : WALKER_TEXT_OUT, y);
    window_building_draw_text(group, window_building_worker_percentage_text(WALKER_TEXT_FULL_STAFF), building_info_y + BUILDING_INFO_SECOND_TEXT_Y);
}

void window_building_draw_stock(int32_t resource)
{
    const building *b = building_info_building();
    int32_t y = building_info_y + BUILDING_INFO_STOCK_TEXT_Y;
    g_text_width = 0;
    image_draw(image_group(IMAGE_GROUP_RESOURCE_ICONS) + resource, building_info_x + BUILDING_INFO_STOCK_ICON_X, building_info_y + BUILDING_INFO_STOCK_ICON_Y);
    text_draw_number_ui(
        b->loads_stored, NUMBER_PREFIX_NONE, postfix_none, g_text_width + BUILDING_INFO_STOCK_TEXT_X + building_info_x, y, image_group(IMAGE_GROUP_FONT_BLACK), 0, building_info_x, building_info_y);
    lang_text_draw_ui(
        LANG_GROUP_RESOURCE_NAMES, resource, g_text_width + BUILDING_INFO_STOCK_TEXT_X + building_info_x, y, image_group(IMAGE_GROUP_FONT_BLACK), 0, building_info_x, building_info_y);
}

void window_building_draw_right_click_hint(void)
{
    lang_text_draw_centered_ui(LANG_GROUP_WINDOW_HINTS, WINDOW_HINT_RIGHT_CLICK_TO_CONTINUE, building_info_x,
        (building_info_h * BUILDING_INFO_BLOCK) - RIGHT_CLICK_HINT_FROM_BOTTOM + building_info_y, info_w_blocks * BUILDING_INFO_BLOCK, image_group(IMAGE_GROUP_FONT_BLACK), 0,
        building_info_x, building_info_y);
}

static int32_t employment_text(const building *b, int32_t laborers)
{
    if (b->num_workers >= laborers) {
        return (b->houses_covered < GOOD_LABOR_ACCESS_HOUSES) ? BUILDING_INFO_TEXT_POOR_ACCESS_TO_WORKERS : BUILDING_INFO_TEXT_EMPLOYEES_NEEDED;
    }
    const city_player *player = city_player_current();
    if (player->population < 1) {
        return BUILDING_INFO_TEXT_NO_PEOPLE_IN_CITY;
    }
    if (b->houses_covered < 1) {
        return BUILDING_INFO_TEXT_NO_WORKERS_NEARBY;
    }
    if (b->houses_covered < GOOD_LABOR_ACCESS_HOUSES) {
        return BUILDING_INFO_TEXT_POOR_ACCESS_TO_WORKERS;
    }
    if (player->labor_categories[b->labor_category].workers_allocated > 0) {
        return BUILDING_INFO_TEXT_LABOR_COULD_BE_SHIFTED_ALLOCATED;
    }
    return BUILDING_INFO_TEXT_LABOR_COULD_BE_SHIFTED;
}

static void draw_employees(const building *b, int32_t laborers, int32_t y)
{
    int32_t font = image_group(IMAGE_GROUP_FONT_GREEN);
    lang_text_draw_amount(LANG_GROUP_AMOUNTS, AMOUNT_TEXT_EMPLOYEE, b->num_workers, building_info_x + EMPLOYMENT_X, y, font, 0);
    text_draw_number_ui(laborers, '(', postfix_none, g_text_width + EMPLOYMENT_NEEDED_X + building_info_x, y, font, 0, building_info_x, building_info_y);
    lang_text_draw_ui(LANG_GROUP_BUILDING_INFO, BUILDING_INFO_TEXT_EMPLOYEES_NEEDED, g_text_width + EMPLOYMENT_NEEDED_X + building_info_x, y, font, 0,
        building_info_x, building_info_y);
}

void window_building_draw_employment(int32_t y_offset)
{
    const building *b = building_info_building();
    int32_t laborers = building_model_table[b->type].laborers;
    int32_t text_id = employment_text(b, laborers);
    image_draw(image_group(IMAGE_GROUP_CONTEXT_ICONS) + EMPLOYMENT_ICON, building_info_x + EMPLOYMENT_ICON_X, y_offset + EMPLOYMENT_ICON_Y);
    g_text_width = 0;
    if (text_id == BUILDING_INFO_TEXT_EMPLOYEES_NEEDED) {
        draw_employees(b, laborers, y_offset + EMPLOYMENT_SINGLE_LINE_Y);
        return;
    }
    draw_employees(b, laborers, y_offset + EMPLOYMENT_TEXT_Y);
    g_text_width = 0;
    lang_text_draw_multiline_ui(LANG_GROUP_BUILDING_INFO, text_id, building_info_x + EMPLOYMENT_NEEDED_X, y_offset + EMPLOYMENT_NOTE_Y, EMPLOYMENT_NOTE_WIDTH,
        image_group(IMAGE_GROUP_FONT_GREEN), 0, building_info_x, building_info_y);
}

static int32_t house_inventory_resource(int32_t slot)
{
    switch (slot) {
        case INVENTORY_GRAIN:
            return RESOURCE_GRAIN;
        case INVENTORY_MEAT:
            return RESOURCE_MEAT;
        case INVENTORY_LETTUCE:
            return RESOURCE_LETTUCE;
        case INVENTORY_CHICKPEAS:
            return RESOURCE_CHICKPEAS;
        case INVENTORY_POMEGRANATES:
            return RESOURCE_POMEGRANATES;
        case INVENTORY_FIGS:
            return RESOURCE_FIGS;
        case INVENTORY_FISH:
            return RESOURCE_FISH;
        case INVENTORY_GAMEMEAT:
            return RESOURCE_GAMEMEAT;
        case INVENTORY_BEER:
            return RESOURCE_BEER;
        case INVENTORY_POTTERY:
            return RESOURCE_POTTERY;
        case INVENTORY_LINEN:
            return RESOURCE_LINEN;
        case INVENTORY_JEWELRY:
            return RESOURCE_LUXURY_GOODS;
        default:
            return -1;
    }
}

void window_building_draw_figure_goods(int32_t figure_id, int32_t x, int32_t y)
{
    const figure *f = figure_get(figure_id);
    int32_t saved_text_width = g_text_width;
    for (int32_t i = 0; i < FIGURE_GOODS_SLOTS; i++) {
        int32_t slot = f->buyer_resources[i];
        if (slot == INVENTORY_NONE) {
            continue;
        }
        int32_t resource = house_inventory_resource(slot);
        if (f->buyer_amounts_bought[i] > 0) {
            image_draw(image_group(IMAGE_GROUP_RESOURCE_ICONS) + resource, x, y);
            x += FIGURE_GOODS_ICON_SPACING;
            g_text_width = saved_text_width;
        }
    }
}

void window_building_draw_farm_workers(int32_t y)
{
    if (building_is_on_floodplain(building_info_building_id) != 0) {
        const building *b = building_info_building();
        int32_t text = (b->num_workers == 0) ? FARM_TEXT_NEEDS_LABORERS : FARM_TEXT_FULL_CREW;
        lang_text_draw_multiline_ui(LANG_GROUP_FARM, text, building_info_x + FARM_TEXT_X, y + EMPLOYMENT_TEXT_Y, (info_w_blocks - 4) * BUILDING_INFO_BLOCK,
            image_group(IMAGE_GROUP_FONT_GREEN), 0, building_info_x, building_info_y);
        image_draw(image_group(IMAGE_GROUP_CONTEXT_ICONS) + EMPLOYMENT_ICON, building_info_x + EMPLOYMENT_ICON_X, y + EMPLOYMENT_ICON_Y);
        return;
    }
    window_building_draw_employment(y);
}
