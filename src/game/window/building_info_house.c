// The building info panels of the houses and the vacant lot: people, room left, taxes, crime, the evolve text with
// the goods still needed, the luxury goods a house could get by trade, and the worst desirability building near the
// house (found when the window opens).
//
// Two house panel details:
// - with imports, the luxury goods slots are cleared by the city's own trade status, tested once per empire city;
// - the second variant of the needed goods line keeps the y of the first line.

#include "window/building_info_house.h"
#include "building/building.h"
#include "building/building_figure_helpers.h"
#include "building/building_type.h"
#include "building/house_evolve.h"
#include "building/house_services.h"
#include "city/resource_type.h"
#include "core/calc.h"
#include "empire/empire.h"
#include "game/game_internal.h"
#include "graphics/graphics_data.h"
#include "graphics/image_collection.h"
#include "graphics/image_draw.h"
#include "graphics/panel_draw.h"
#include "graphics/text_draw.h"
#include "graphics/ui_layout.h"
#include "map/grid.h"
#include "map/road_access.h"
#include "text/lang.h"
#include "widget/widget_data.h"
#include "window/building_info.h"
#include "window/building_info_panels.h"
#include "window/window_data.h"
#include "window/window_image_group.h"
#include <stdint.h>
#include <string.h>

int32_t house_worst_desirability;

enum house_text {
    HOUSE_TEXT_OCCUPANTS = 20,
    HOUSE_TEXT_TOO_MANY = 21,
    HOUSE_TEXT_EXTRA_ROOM_FOR = 22,
    HOUSE_TEXT_NO_TAX_COLLECTOR = 23,
    HOUSE_TEXT_TAX_GENERATED = 24,
    HOUSE_TEXT_TAX_SO_FAR = 25,
    HOUSE_TEXT_NO_CRIME = 26,
    HOUSE_TEXT_THIEVES = 32,
    HOUSE_TEXT_HUTS_NEED_FOOD = 33,
    HOUSE_TEXT_EVOLVE_FIRST = 40,
    HOUSE_TEXT_INFECTED_FOR = 107,
    HOUSE_TEXT_FROGS_FOR = 109,
};

enum vacant_lot_text {
    VACANT_LOT_TEXT_UNOCCUPIED = 0,
    VACANT_LOT_TEXT_VACANT = 1,
    VACANT_LOT_TEXT_TOO_FAR_FROM_ROAD = 2,
};

enum house_layout {
    HOUSE_TITLE_Y = 10,
    VACANT_LOT_TITLE_Y = 8,
    VACANT_LOT_HINT_FROM_BOTTOM = 22,
    VACANT_LOT_TEXT_FROM_BOTTOM = 113,
    VACANT_LOT_ROAD_RADIUS = 2,
    HOUSE_INNER_PANEL_Y = 148,
    HOUSE_INNER_PANEL_BLOCKS = 10,
    HOUSE_POPULATION_Y = 154,
    HOUSE_TAXES_Y = 194,
    HOUSE_CRIME_Y = 214,
    HOUSE_HUTS_TEXT_X = 36,
    HOUSE_HUTS_TEXT_Y = 234,
    HOUSE_FOODS_Y = 110,
    HOUSE_GOODS_Y = 130,
    HOUSE_GOOD_X = 32,
    HOUSE_GOOD_AMOUNT_X = 64,
    HOUSE_GOOD_SPACING = 110,
    HOUSE_SICK_TEXT_Y = 76,
    HOUSE_EVOLVE_TEXT_Y = 36,
    HOUSE_EVOLVE_SECOND_TEXT_Y = 52,
    HOUSE_NEEDED_GOODS_DY = 23,
    HOUSE_TEXT_LINE_HEIGHT = 16,
    HOUSE_INFO_X = 36,
    POPULATION_ICON = 13,
    POPULATION_ICON_X = 34,
    POPULATION_ICON_DY = 4,
    POPULATION_TEXT_DY = 14,
    POPULATION_TEXT_X = 50,
    WORST_BUILDING_COLOR = 0xF800,
};

enum house_luxury_goods {
    LUXURY_GOOD_SLOTS = 16,
    LUXURY_GOOD_TEXT_FIRST = 38,
    LUXURY_GOODS_SHOWN = 4,
    WORST_DESIRABILITY_RADIUS = 6,
};

int32_t house_crime_text_rules[HOUSE_CRIME_TEXT_RULE_COUNT] = {
    [HOUSE_CRIME_TEXT_RULE_NO_CRIME] = 50,
    [HOUSE_CRIME_TEXT_RULE_PEACEFUL] = 40,
    [HOUSE_CRIME_TEXT_RULE_SOME_CRIME] = 30,
    [HOUSE_CRIME_TEXT_RULE_BECOMING_A_PROBLEM] = 20,
    [HOUSE_CRIME_TEXT_RULE_HIGH_CRIME] = 10,
    [HOUSE_CRIME_TEXT_RULE_LAWLESS] = 1,
};

static const int32_t house_goods[] = { RESOURCE_POTTERY, RESOURCE_LUXURY_GOODS, RESOURCE_LINEN, RESOURCE_BEER };

static void draw_text(int32_t group, int32_t index, int32_t y)
{
    window_building_draw_text(group, index, y);
}

static void draw_label(int32_t group, int32_t index, int32_t x, int32_t y, int32_t font)
{
    lang_text_draw_ui(group, index, x, y, font, 0, building_info_x, building_info_y);
}

static int32_t is_trading_city(const empire_city *city)
{
    return city->type == EMPIRE_CITY_PHARAOH_TRADING || city->type == EMPIRE_CITY_EGYPTIAN_TRADING || city->type == EMPIRE_CITY_FOREIGN_TRADING;
}

static int32_t sells_luxury_goods(int32_t city_id)
{
    return empire_city_get(city_id)->sells[RESOURCE_LUXURY_GOODS] != 0 && empire_city_has_trade_limit(city_id, RESOURCE_LUXURY_GOODS) != 0;
}

static int32_t luxury_good_slot(const empire_city *city)
{
    const int32_t *culture = (const int32_t *) empire_culture_table;
    return culture[(city->name_id * 2) + 1] - LUXURY_GOOD_TEXT_FIRST;
}

static int32_t imports_luxury_goods(void)
{
    int16_t status = city_player_current()->trade_status[RESOURCE_LUXURY_GOODS];
    return status == TRADE_STATUS_IMPORT || status == TRADE_STATUS_IMPORT_AS_NEEDED;
}

int32_t window_building_house_trade_goods(int32_t *counts, int32_t imports)
{
    if ((char) imports == 0) {
        if (empire_can_produce_resource_potentially(RESOURCE_GEMS) != 0) {
            counts[0] = 1;
        }
    } else if (building_count_industry_total[RESOURCE_LUXURY_GOODS] < 1) {
        counts[0] = 1;
    }
    for (int32_t city_id = 1; city_id < EMPIRE_CITY_COUNT; city_id++) {
        const empire_city *city = empire_city_get(city_id);
        if (city->in_use != 0 && is_trading_city(city) && sells_luxury_goods(city_id)) {
            counts[luxury_good_slot(city)]++;
        }
    }
    if ((char) imports != 0) {
        for (int32_t city_id = 1; city_id < EMPIRE_CITY_COUNT; city_id++) {
            const empire_city *city = empire_city_get(city_id);
            if (city->in_use != 0 && imports_luxury_goods() && sells_luxury_goods(city_id)) {
                counts[luxury_good_slot(city)] = 0;
            }
        }
    }
    int32_t count = 0;
    for (int32_t i = 0; i < LUXURY_GOOD_SLOTS; i++) {
        if (counts[i] != 0) {
            count++;
        }
    }
    return count;
}

void window_building_draw_house_vacant_lot(void)
{
    window_building_prepare_figure_list();
    window_building_draw_panel_title_at(LANG_GROUP_VACANT_LOT, VACANT_LOT_TEXT_UNOCCUPIED, VACANT_LOT_TITLE_Y);
    lang_text_draw_centered_ui(LANG_GROUP_WINDOW_HINTS, WINDOW_HINT_RIGHT_CLICK_TO_CONTINUE, building_info_x,
        (building_info_h * BUILDING_INFO_BLOCK) - VACANT_LOT_HINT_FROM_BOTTOM + building_info_y, info_w_blocks * BUILDING_INFO_BLOCK, image_group(IMAGE_GROUP_FONT_BLACK), 0,
        building_info_x, building_info_y);
    window_building_draw_figure_list();
    const building *b = building_info_building();
    int32_t road = map_closest_road_within_radius(b->x, b->y, 1, VACANT_LOT_ROAD_RADIUS);
    draw_text(LANG_GROUP_VACANT_LOT, (road == 0) ? VACANT_LOT_TEXT_TOO_FAR_FROM_ROAD : VACANT_LOT_TEXT_VACANT,
        (building_info_h * BUILDING_INFO_BLOCK) - VACANT_LOT_TEXT_FROM_BOTTOM + building_info_y);
}

static void draw_house_good(const building *b, int32_t resource, int32_t dx, int32_t y)
{
    image_draw(image_group(IMAGE_GROUP_RESOURCE_ICONS) + resource, dx + HOUSE_GOOD_X + building_info_x, y);
    text_draw_number_ui(b->data.house.inventory[house_food_inventory_index(resource)], NUMBER_PREFIX_NONE, postfix_none, dx + HOUSE_GOOD_AMOUNT_X + building_info_x, y,
        image_group(IMAGE_GROUP_FONT_BLACK), 0, building_info_x, building_info_y);
}

static void draw_house_needed_goods(const int32_t *counts, int32_t count, int32_t keep_first_line_y)
{
    int32_t first_line_y = g_draw_y_accum;
    int32_t drawn = 0;
    for (int32_t i = 0; i < LUXURY_GOOD_SLOTS; i++) {
        if (counts[i] == 0) {
            continue;
        }
        int32_t line_y = keep_first_line_y ? first_line_y : g_draw_y_accum;
        int32_t x = g_text_width + BUILDING_INFO_TEXT_X + building_info_x;
        if (count > 1 && drawn == count - 1) {
            text_draw((const uint8_t *) "or", x, line_y + HOUSE_NEEDED_GOODS_DY + building_info_y, image_group(IMAGE_GROUP_FONT_BLACK), 0);
        } else if (drawn > 0) {
            text_draw(str_comma, x, line_y + HOUSE_NEEDED_GOODS_DY + building_info_y, image_group(IMAGE_GROUP_FONT_BLACK), 0);
        }
        line_y = keep_first_line_y ? first_line_y : g_draw_y_accum;
        draw_label(LANG_GROUP_RESOURCE_NAMES, LUXURY_GOOD_TEXT_FIRST + i + 1, g_text_width + BUILDING_INFO_TEXT_X + building_info_x,
            line_y + HOUSE_NEEDED_GOODS_DY + building_info_y, image_group(IMAGE_GROUP_FONT_BLACK));
        drawn++;
        if (drawn >= count) {
            return;
        }
    }
}

static void draw_house_evolve_goods(const building *b, int32_t imports)
{
    int32_t counts[LUXURY_GOOD_SLOTS];
    memset(counts, 0, sizeof(counts));
    int32_t count = window_building_house_trade_goods(counts, imports);
    if (count > LUXURY_GOODS_SHOWN) {
        count = LUXURY_GOODS_SHOWN;
    } else if (count == 0) {
        count = 1;
        counts[0] = 1;
    }
    g_text_width = 0;
    g_draw_y_accum = 0;
    draw_text(LANG_GROUP_HOUSE, b->data.house.evolve_text_id + HOUSE_TEXT_EVOLVE_FIRST, building_info_y + HOUSE_EVOLVE_TEXT_Y);
    g_draw_y_accum += HOUSE_TEXT_LINE_HEIGHT;
    g_text_width = 0;
    draw_house_needed_goods(counts, count, imports);
}

static void draw_house_worst_desirability(const building *b)
{
    int32_t evolve = b->data.house.evolve_text_id;
    int32_t y = building_info_y + HOUSE_EVOLVE_TEXT_Y;
    g_text_width = 0;
    g_draw_y_accum = 0;
    draw_label(LANG_GROUP_HOUSE, evolve + HOUSE_TEXT_EVOLVE_FIRST, building_info_x + BUILDING_INFO_TEXT_X, y, image_group(IMAGE_GROUP_FONT_BLACK));
    const building *worst = building_get(house_worst_desirability_building);
    int32_t x = g_text_width + BUILDING_INFO_TEXT_X + building_info_x;
    if (worst->type == BUILDING_HOUSE_CRUDE_HUT && worst->house_population == 0) {
        lang_text_draw_ui(LANG_GROUP_VACANT_LOT, VACANT_LOT_TEXT_UNOCCUPIED, x, y, image_group(IMAGE_GROUP_FONT_BLACK), WORST_BUILDING_COLOR, building_info_x, building_info_y);
    } else {
        lang_text_draw_ui(LANG_GROUP_BUILDING_TYPES, worst->type, x, y, image_group(IMAGE_GROUP_FONT_BLACK), WORST_BUILDING_COLOR, building_info_x, building_info_y);
    }
    text_draw((const uint8_t *) ")", g_text_width + BUILDING_INFO_TEXT_X + building_info_x, y, image_group(IMAGE_GROUP_FONT_BLACK), 0);
    draw_text(LANG_GROUP_HOUSE, b->data.house.evolve_text_id + HOUSE_TEXT_EVOLVE_FIRST + 1, building_info_y + HOUSE_EVOLVE_SECOND_TEXT_Y);
}

static void draw_house_inventory(const building *b)
{
    for (int32_t i = 0; i < city_foods.count; i++) {
        int32_t resource = city_foods.list[i];
        if (resource != RESOURCE_NONE) {
            draw_house_good(b, resource, i * HOUSE_GOOD_SPACING, building_info_y + HOUSE_FOODS_Y);
        }
    }
    for (int32_t i = 0; i < (int32_t) (sizeof(house_goods) / sizeof(house_goods[0])); i++) {
        draw_house_good(b, house_goods[i], i * HOUSE_GOOD_SPACING, building_info_y + HOUSE_GOODS_Y);
    }
}

static void draw_house_evolve_text(const building *b)
{
    int32_t evolve = b->data.house.evolve_text_id;
    switch (evolve) {
        case EVOLVE_TEXT_NEXT_DESIRABILITY_E:
            draw_house_worst_desirability(b);
            return;
        case EVOLVE_TEXT_NEXT_JEWELRY:
        case EVOLVE_TEXT_JEWELRY:
            draw_house_evolve_goods(b, 0);
            return;
        case EVOLVE_TEXT_SECOND_JEWELRY:
        case EVOLVE_TEXT_NEXT_SECOND_JEWELRY:
            draw_house_evolve_goods(b, 1);
            return;
        default:
            draw_text(LANG_GROUP_HOUSE, evolve + HOUSE_TEXT_EVOLVE_FIRST, building_info_y + HOUSE_EVOLVE_TEXT_Y);
            return;
    }
}

void window_building_draw_house(void)
{
    const building *b = building_info_building();
    info_help_id = building_info_help_messages[BUILDING_INFO_HELP_HOUSE];
    info_first_draw = 0;
    if (b->house_population < 1 && b->data.house.is_sick == 0) {
        window_building_draw_house_vacant_lot();
        return;
    }
    window_building_draw_panel_title_at(LANG_GROUP_HOUSE_LEVELS, b->type - BUILDING_HOUSE_CRUDE_HUT, HOUSE_TITLE_Y);
    building_info_details_shown = 0;
    window_building_draw_inner_panel(HOUSE_INNER_PANEL_Y, HOUSE_INNER_PANEL_BLOCKS);
    window_building_draw_house_population(building_info_y + HOUSE_POPULATION_Y);
    window_building_draw_house_taxes(building_info_y + HOUSE_TAXES_Y);
    window_building_draw_house_crime(building_info_y + HOUSE_CRIME_Y);
    if (house_models[b->subtype].food_types == 0) {
        lang_text_draw_multiline_ui(LANG_GROUP_HOUSE, HOUSE_TEXT_HUTS_NEED_FOOD, building_info_x + HOUSE_HUTS_TEXT_X, building_info_y + HOUSE_HUTS_TEXT_Y,
            (info_w_blocks - 6) * BUILDING_INFO_BLOCK, image_group(IMAGE_GROUP_FONT_GREEN), 0, building_info_x, building_info_y);
    }
    draw_house_inventory(b);
    int32_t sickness = b->data.house.is_sick;
    if (sickness == HOUSE_SICK_DISEASE || sickness == HOUSE_SICK_FROGS) {
        g_text_width = 0;
        draw_text(LANG_GROUP_HOUSE, (sickness == HOUSE_SICK_DISEASE) ? HOUSE_TEXT_INFECTED_FOR : HOUSE_TEXT_FROGS_FOR, building_info_y + HOUSE_SICK_TEXT_Y);
        lang_text_draw_amount(LANG_GROUP_AMOUNTS, AMOUNT_TEXT_MONTH, b->house_sick_months, g_text_width + BUILDING_INFO_TEXT_X + building_info_x,
            building_info_y + HOUSE_SICK_TEXT_Y, image_group(IMAGE_GROUP_FONT_BLACK), 0);
        return;
    }
    draw_house_evolve_text(b);
}

static void consider_worst_desirability(const building *house, int32_t id, int32_t x, int32_t y)
{
    const building *b = building_get(id);
    if (id <= 0 || b->state != BUILDING_STATE_IN_USE || id == building_info_building_id) {
        return;
    }
    if (b->house_size != 0 && b->type >= house->type) {
        return;
    }
    const building_model_record *model = &building_model_table[b->type];
    int32_t value = model->desirability_value;
    if (value >= 0) {
        return;
    }
    int32_t distance = calc_maximum_distance(x, y, house->x, house->y);
    if (distance > model->desirability_range) {
        return;
    }
    if (distance > 1) {
        value += (distance - 1) * model->desirability_step_size;
    }
    if (value < house_worst_desirability) {
        house_worst_desirability = value;
        house_worst_desirability_building = (b->type == BUILDING_FORT_GROUND) ? building_main(id) : id;
    }
}

void window_building_house_find_worst_desirability(void)
{
    const building *house = building_info_building();
    int32_t x_min = house->x - WORST_DESIRABILITY_RADIUS;
    int32_t x_max = house->x + WORST_DESIRABILITY_RADIUS;
    int32_t y_min = house->y - WORST_DESIRABILITY_RADIUS;
    int32_t y_max = house->y + WORST_DESIRABILITY_RADIUS;
    house_worst_desirability = 0;
    house_worst_desirability_building = 0;
    if (x_min < 0) {
        x_min = 0;
    }
    if (y_min < 0) {
        y_min = 0;
    }
    if (x_max >= map_info.grid_width) {
        x_max = map_info.grid_width - 1;
    }
    if (y_max >= map_info.grid_height) {
        y_max = map_info.grid_height - 1;
    }
    for (int32_t y = y_min; y <= y_max; y++) {
        for (int32_t x = x_min; x <= x_max; x++) {
            consider_worst_desirability(house, map_grids.building_ids[map_grid_offset(x, y)], x, y);
        }
    }
}

void window_building_draw_house_population(int32_t y)
{
    const building *b = building_info_building();
    int32_t font = image_group(IMAGE_GROUP_FONT_GREEN);
    g_text_width = 0;
    image_draw(image_group(IMAGE_GROUP_CONTEXT_ICONS) + POPULATION_ICON, building_info_x + POPULATION_ICON_X, y + POPULATION_ICON_DY);
    y += POPULATION_TEXT_DY;
    text_draw_number_ui(
        b->house_population, NUMBER_PREFIX_NONE, postfix_none, building_info_x + POPULATION_TEXT_X + g_text_width, y, font, 0, building_info_x, building_info_y);
    draw_label(LANG_GROUP_HOUSE, HOUSE_TEXT_OCCUPANTS, building_info_x + POPULATION_TEXT_X + g_text_width, y, font);
    int32_t room = b->house_population_room;
    if (room < 0) {
        text_draw_number_ui(-room, NUMBER_PREFIX_NONE, postfix_none, building_info_x + POPULATION_TEXT_X + g_text_width, y, font, 0, building_info_x, building_info_y);
        draw_label(LANG_GROUP_HOUSE, HOUSE_TEXT_TOO_MANY, building_info_x + POPULATION_TEXT_X + g_text_width, y, font);
    } else if (room > 0) {
        draw_label(LANG_GROUP_HOUSE, HOUSE_TEXT_EXTRA_ROOM_FOR, building_info_x + POPULATION_TEXT_X + g_text_width, y, font);
        text_draw_number_ui(room, NUMBER_PREFIX_NONE, postfix_none, building_info_x + POPULATION_TEXT_X + g_text_width, y, font, 0, building_info_x, building_info_y);
    }
}

void window_building_draw_house_taxes(int32_t y)
{
    int32_t font = image_group(IMAGE_GROUP_FONT_GREEN);
    g_text_width = 0;
    if (building_info_building()->house_tax_coverage == 0) {
        draw_label(LANG_GROUP_HOUSE, HOUSE_TEXT_NO_TAX_COLLECTOR, building_info_x + HOUSE_INFO_X, y, image_group(IMAGE_GROUP_FONT_GREEN));
        return;
    }
    int32_t income = building_house_tax_income(building_info_building_id);
    draw_label(LANG_GROUP_HOUSE, HOUSE_TEXT_TAX_GENERATED, building_info_x + HOUSE_INFO_X + g_text_width, y, font);
    int32_t width = g_text_width;
    lang_text_draw_amount(LANG_GROUP_AMOUNTS, AMOUNT_TEXT_DEBEN, income, building_info_x + HOUSE_INFO_X + g_text_width, y, font, 0);
    g_text_width += width;
    draw_label(LANG_GROUP_HOUSE, HOUSE_TEXT_TAX_SO_FAR, building_info_x + HOUSE_INFO_X + g_text_width, y, font);
}

static int32_t crime_text(int32_t sentiment)
{
    for (int32_t rule = 0; rule < HOUSE_CRIME_TEXT_RULE_COUNT; rule++) {
        if (sentiment >= house_crime_text_rules[rule]) {
            return HOUSE_TEXT_NO_CRIME + rule;
        }
    }
    return HOUSE_TEXT_THIEVES;
}

void window_building_draw_house_crime(int32_t y)
{
    draw_label(LANG_GROUP_HOUSE, crime_text(building_info_building()->sentiment.house), building_info_x + HOUSE_INFO_X, y, image_group(IMAGE_GROUP_FONT_GREEN));
}
