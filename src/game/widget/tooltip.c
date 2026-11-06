// Tooltips of the city, the advisors and the dialogs: the hovered button's text after the tooltip delay, the
// senate box over a palace, the trade advisor's luxury goods box and the editor's map flag goods; the active
// overlay's text for the building or tile under the mouse; placing and drawing the box.
//
// Some tooltips read beyond their tables:
// - the bazaar access overlay counts only the foods 1..8;
// - a trade city's luxury goods tooltip reads the city cultures as words indexed by the city's name id without the
//   record stride.

#include "widget/tooltip.h"
#include "building/building.h"
#include "building/building_data.h"
#include "building/building_figure_helpers.h"
#include "building/building_type.h"
#include "building/house_evolve.h"
#include "building/house_services.h"
#include "city/city_data.h"
#include "city/city_player.h"
#include "city/resource_type.h"
#include "core/calc.h"
#include "core/text.h"
#include "editor/editor_data.h"
#include "empire/empire.h"
#include "empire/empire_data.h"
#include "figure/figure.h"
#include "figure/figure_type.h"
#include "game/game_internal.h"
#include "graphics/graphics_data.h"
#include "graphics/graphics_display.h"
#include "graphics/graphics_primitives.h"
#include "graphics/image_collection.h"
#include "graphics/rich_text.h"
#include "graphics/text_draw.h"
#include "input/input_data.h"
#include "map/grid.h"
#include "map/map_layers.h"
#include "map/terrain.h"
#include "text/lang.h"
#include "widget/city_overlay_goods.h"
#include "widget/city_with_overlay.h"
#include "widget/tooltip_bands.h"
#include "widget/tooltip_helpers.h"
#include "widget/tooltip_overlay.h"
#include "widget/widget_data.h"
#include "window/advisors.h"
#include "window/building_info.h"
#include "window/top_menu_entries.h"
#include "window/window_data.h"
#include "window/window_id.h"
#include "window/window_image_group.h"
#include <stdint.h>
#include <string.h>

int32_t tooltip_rich_lines_saved;

int32_t tooltip_rich_viewport_saved;

int32_t tooltip_rich_scroll_saved;

enum tooltip_timing {
    TOOLTIP_DELAY_MS = 150,
    TOOLTIP_ACTIVE = 1,
};

enum tooltip_box {
    TOOLTIP_PADDING = 5,
    TOOLTIP_TEXT_TOP = 7,
    TOOLTIP_BORDER = 20,
    TOOLTIP_LINE_HEIGHT = 16,
    TOOLTIP_BOX_EXTRA_HEIGHT = 10,
    TOOLTIP_VIEWPORT_LINES = 30,
    TOOLTIP_NARROW_LINES_MAX = 2,
    TOOLTIP_TEXT_BOX_WIDTH = 220,
    TOOLTIP_TEXT_BOX_TEXT_WIDTH = 190,
    TOOLTIP_TEXT_BOX_WIDE_TEXT_WIDTH = 270,
    TOOLTIP_TEXT_BOX_TEXT_INSET = 30,
    TOOLTIP_BUTTON_BOX_WIDTH = 200,
    TOOLTIP_WIDE_BOX_WIDTH = 300,
    TOOLTIP_MOUSE_GAP = 20,
    TOOLTIP_TEXT_BOX_BELOW = 50,
    TOOLTIP_TEXT_BOX_ABOVE = 72,
    TOOLTIP_LOWER_HALF_Y = 200,
    TOOLTIP_ADVISOR_GAP = 50,
    TOOLTIP_ADVISOR_LEFT = 100,
    TOOLTIP_ADVISOR_BOTTOM = 432,
    TOOLTIP_BUTTON_BELOW = 40,
    TOOLTIP_BUTTON_ABOVE = 62,
    TOOLTIP_LABOR_PRIORITY_ABOVE = 72,
    TOOLTIP_LABOR_PRIORITY_LEFT = 10,
    TOOLTIP_DONATE_ABOVE = 52,
    TOOLTIP_LABOR_ADVISOR_ABOVE = 74,
    TOOLTIP_TRADE_ADVISOR_ABOVE = 54,
    TOOLTIP_POPULATION_ADVISOR_ABOVE = 58,
    TOOLTIP_ADVISOR_ABOVE = 64,
};

enum senate_box {
    SENATE_WIDTH = 180,
    SENATE_HEIGHT = 120,
    SENATE_RIGHT_EDGE = 200,
    SENATE_BELOW = 10,
    SENATE_ABOVE = 32,
    SENATE_ROWS = 6,
    SENATE_ROW_HEIGHT = 14,
    SENATE_VALUE_X = 140,
    SENATE_SHADOW = 1,
};

enum senate_text {
    SENATE_TEXT_UNEMPLOYMENT = 135,
    SENATE_TEXT_CULTURE = 136,
    SENATE_TEXT_PROSPERITY = 137,
    SENATE_TEXT_MONUMENT = 138,
    SENATE_TEXT_KINGDOM = 139,
    SENATE_TEXT_TAX = 142,
};

enum tooltip_color {
    TOOLTIP_FILL_RGB565 = 0xBB88,
    TOOLTIP_BORDER_RGB565 = 0x58A0,
    TOOLTIP_SHADOW_RGB565 = 0x3922,
    TOOLTIP_TEXT_RGB565 = 0xFF3A,
    TOOLTIP_FILL_RGB555 = 0x5DC8,
    TOOLTIP_BORDER_RGB555 = 0x2C40,
    TOOLTIP_SHADOW_RGB555 = 0x1C82,
    TOOLTIP_TEXT_RGB555 = 0x7F9A,
    TOOLTIP_MEASURE_COLOR = 0x4208,
};

enum empire_tooltip {
    EMPIRE_TOOLTIP_LUXURY_GOODS = RESOURCE_LUXURY_GOODS,
    EMPIRE_TOOLTIP_LUXURY_GOODS_ANY = 38,
    EMPIRE_TOOLTIP_BUTTON_TEXT_BASE = 133,
};

enum button_tooltip_text {
    TT_HELP = 1,
    TT_EXIT = 2,
    TT_SHOW_CONTROL_PANEL = 12,
    TT_BUILD_HOUSING = 20,
    TT_MINIMAP = 36,
    TT_VISIT_OVERSEERS = 41,
    TT_WORLD_MAP = 42,
    TT_ORIENT_NORTH = 44,
    TT_TOP_MENU_FILE = 51,
    TT_CITY_FUNDS = 61,
    TT_OVERSEER_OF_COMMERCE = 70,
    TT_ADVISOR_BUTTONS = 71,
    TT_LABOR_SET_PRIORITY = 92,
    TT_LABOR_WAGES = 93,
    TT_PRIORITY_REMOVE = 94,
    TT_PRIORITY_LEVEL = 95,
    TT_DONATE_TO_CITY = 96,
    TT_DONATE_EXIT = 100,
    TT_DONATE = 101,
    TT_DONATE_SET_AMOUNT = 102,
    TT_DONATE_ADJUST = 103,
    TT_RATING_CULTURE_ADVICE = 104,
    TT_TRADE_PRICES = 108,
    TT_INDUSTRY_STATUS = 109,
    TT_SELECT_GRAPH = 113,
    TT_FESTIVAL_INFO = 114,
    TT_FESTIVAL_ARRANGE = 115,
    TT_FESTIVAL_NONE = 116,
    TT_FESTIVAL_OSIRIS = 117,
    TT_TAX_RATE = 122,
    TT_TALK = 123,
    TT_SEND_GIFT = 133,
    TT_VIEW_ARMY = 143,
    TT_VIEW_NAVY = 144,
    TT_EDITOR_KINGDOM_MAP = 145,
    TT_EDITOR_ADD_CITY = 158,
    TT_EDITOR_PRICES = 166,
    TT_EDITOR_SCENARIO_RANK = 167,
    TT_EDITOR_ROTATE_VIEW = 175,
    TT_EDITOR_CHANGE_GRAPHIC = 176,
    TT_EDITOR_KILLER_TYPES = 177,
};

enum tooltip_button_id {
    BUTTON_HELP = 10,
    BUTTON_EXIT = 11,
    BUTTON_SIDEBAR_FIRST = 10,
    BUTTON_ROTATE = 20,
    BUTTON_SIDEBAR_BUILD_FIRST = 30,
    BUTTON_REVIEW_MISSION = 45,
    BUTTON_SIDEBAR_TOP_FIRST = 50,
    BUTTON_SIDEBAR_END = 70,
    BUTTON_ADVISOR_FIRST = 10,
    BUTTON_ADVISOR_HELP_FIRST = 25,
    BUTTON_ADVISOR_END = 30,
    BUTTON_INFO_LAST = 29,
    BUTTON_INFO_PHRASE_FIRST = 40,
    BUTTON_INFO_PHRASE_LAST = 54,
    BUTTON_FESTIVAL_ARRANGE = 12,
    BUTTON_FESTIVAL_NONE = 13,
    BUTTON_EMPIRE_COMMERCE = 12,
    BUTTON_EDITOR_KILLER_TYPES = 14,
    BUTTON_EDITOR_SIDEBAR_END = 17,
    BUTTON_EDITOR_ROTATE = 20,
    BUTTON_EDITOR_GRAPHIC = 21,
    EDITOR_SCENARIO_BUTTONS = 8,
    EDITOR_EMPIRE_BUTTONS = 8,
    EDITOR_EMPIRE_PRICES_BUTTON = 10,
    FESTIVAL_GODS = 5,
    FORCES_SWITCH_BUTTON = 19,
    TRADE_BUTTON_PRICES = 1,
    TRADE_BUTTON_EMPIRE = 2,
    TRADE_BUTTON_LUXURY_GOODS = 3,
    LABOR_PRIORITY_BUTTON_REMOVE = 1,
    DONATE_BUTTON_EXIT = 1,
    DONATE_BUTTON_DONATE = 2,
    RATING_BUTTONS = 4,
};

enum service_access_text {
    SERVICE_ACCESS_NONE = 0,
    SERVICE_ACCESS_VERY_GOOD = 1,
    SERVICE_ACCESS_GOOD = 2,
    SERVICE_ACCESS_POOR = 3,
    SERVICE_ACCESS_GOOD_FROM = 20,
    SERVICE_ACCESS_VERY_GOOD_FROM = 80,
};

enum overlay_tooltip_band_text {
    TOOLTIP_WATER_NONE = 0,
    TOOLTIP_WATER_GROUNDWATER = 1,
    TOOLTIP_WATER_NO_WELL_HOUSE = 2,
    TOOLTIP_WATER_WELL_RANGE = 3,
    TOOLTIP_BAZAAR_NO_FOOD = 4,
    TOOLTIP_BAZAAR_SOME_FOOD = 5,
    TOOLTIP_BAZAAR_ENOUGH_FOOD = 6,
    TOOLTIP_BAZAAR_PLENTY_FOOD = 7,
    TOOLTIP_WATER_SUPPLY_FULL = 9,
    TOOLTIP_WATER_SUPPLY_GOOD = 10,
    TOOLTIP_WATER_SUPPLY_LOW = 11,
    TOOLTIP_RELIGION_GODS = 12,
    TOOLTIP_SCRIBAL_SCHOOL = 19,
    TOOLTIP_LIBRARY = 23,
    TOOLTIP_DENTIST = 31,
    TOOLTIP_APOTHECARY = 35,
    TOOLTIP_MORTUARY = 39,
    TOOLTIP_TAX_NOT_REGISTERED = 43,
    TOOLTIP_TAX_NO_INCOME = 44,
    TOOLTIP_TAX_INCOME = 45,
    TOOLTIP_FIRE_RISK = 46,
    TOOLTIP_DAMAGE_RISK = 52,
    TOOLTIP_CRIME_RISK = 58,
    TOOLTIP_ENTERTAINMENT = 64,
    TOOLTIP_JUGGLER = 75,
    TOOLTIP_MUSICIAN = 79,
    TOOLTIP_DANCER = 83,
    TOOLTIP_SENET = 87,
    TOOLTIP_LABOR = 94,
    TOOLTIP_EDUCATION_NONE = 100,
    TOOLTIP_EDUCATION_SCHOOL = 101,
    TOOLTIP_EDUCATION_LIBRARY = 102,
    TOOLTIP_BAZAAR_NO_PEOPLE = 104,
    TOOLTIP_FERTILITY_HIGH = 105,
    TOOLTIP_FERTILITY_GOOD = 106,
    TOOLTIP_FERTILITY_MEDIUM = 107,
    TOOLTIP_FERTILITY_LOW = 108,
    TOOLTIP_FERTILITY_NONE = 109,
    TOOLTIP_SICK_DISEASE = 131,
    TOOLTIP_PHYSICIAN_NONE = 132,
    TOOLTIP_PHYSICIAN_POOR = 133,
    TOOLTIP_PHYSICIAN_GOOD = 134,
    TOOLTIP_PHYSICIAN_VERY_GOOD = 135,
    TOOLTIP_EMPTY_HOUSE = 156,
    TOOLTIP_MAGISTRATE = 158,
    TOOLTIP_DESIRABILITY = 162,
    TOOLTIP_ZOOKEEPER = 168,
    TOOLTIP_SICK_FROGS = 172,
};

enum overlay_tooltip_band {
    BAZAAR_FOOD_FIRST = RESOURCE_MIN_FOOD,
    BAZAAR_FOOD_END = 9,
    BAZAAR_PLENTY_PERCENT = 1001,
    BAZAAR_ENOUGH_PERCENT = 500,
    LABOR_FULL = 300,
    WATER_SUPPLY_FULL_FROM = 80,
    WATER_SUPPLY_GOOD_FROM = 17,
    HOUSE_SICK_DISEASE_KIND = 1,
    HOUSE_SICK_FROGS_KIND = 2,
};

typedef struct {
    int32_t overlay;
    uint8_t (*access)(const building *b);
    int32_t text;
} service_tooltip;

static void append_cstr(char *dst, int32_t *inout_len, const char *src)
{
    if (!src) {
        return;
    }
    int32_t len = (int32_t) strlen(src) + 1;
    memcpy(dst + *inout_len, src, (size_t) len);
    *inout_len += len - 1;
}

static void save_rich_text_state(void)
{
    tooltip_rich_viewport_saved = rich_viewport_lines;
    tooltip_rich_scroll_saved = rich_scroll_pos;
    tooltip_rich_lines_saved = rich_line_count;
}

static void activate_button_tooltip(void)
{
    save_rich_text_state();
    tooltip_type = TOOLTIP_KIND_BUTTON;
    tooltip_window_id = window_current_id;
    if (window_redraw_request == WINDOW_REDRAW_NONE) {
        window_redraw_request = WINDOW_REDRAW_FOREGROUND;
    }
}

static void activate_box_tooltip(int32_t kind, int32_t window_id)
{
    save_rich_text_state();
    tooltip_type = kind;
    tooltip_text_id = TOOLTIP_ACTIVE;
    tooltip_window_id = window_id;
    window_redraw_request = WINDOW_REDRAW_FOREGROUND;
}

static int32_t finish_button_tooltip(int32_t elapsed)
{
    if (settings_data.tooltips != TOOLTIPS_FULL) {
        tooltip_last_time = time_current_millis;
        tooltip_text_id = 0;
        return 0;
    }
    if (elapsed < TOOLTIP_DELAY_MS) {
        tooltip_text_id = 0;
        return 0;
    }
    activate_button_tooltip();
    return 1;
}

static int32_t no_tooltip(void)
{
    tooltip_last_time = time_current_millis;
    return 0;
}

int32_t tooltip_reset(void)
{
    if (tooltip_text_id < 1) {
        return 0;
    }
    rich_scroll_pos = tooltip_rich_scroll_saved;
    tooltip_text_id = 0;
    rich_viewport_lines = tooltip_rich_viewport_saved;
    rich_line_count = tooltip_rich_lines_saved;
    if (window_redraw_request == WINDOW_REDRAW_NONE) {
        window_redraw_request = WINDOW_REDRAW_FOREGROUND;
    }
    return 1;
}

int32_t tooltip_invalidate(void)
{
    int32_t now = time_current_millis;
    tooltip_text_id = 0;
    tooltip_last_time = now;
    return now;
}

static void draw_box(int32_t width, int32_t height)
{
    int32_t rgb565 = (display_mode == DISPLAY_MODE_RGB565);
    graphics_draw_rect(tooltip_box_x, tooltip_box_y, width, height, rgb565 ? TOOLTIP_BORDER_RGB565 : TOOLTIP_BORDER_RGB555);
    graphics_fill_rect(tooltip_box_x + 1, tooltip_box_y + 1, width - 2, height - 2, rgb565 ? TOOLTIP_FILL_RGB565 : TOOLTIP_FILL_RGB555);
}

static void measure_text(const char *text, int32_t width)
{
    rich_text_draw((const uint8_t *) text, tooltip_box_x + TOOLTIP_PADDING, tooltip_box_y + TOOLTIP_PADDING, width, 1, 0);
    tooltip_box_height = (rich_line_count * TOOLTIP_LINE_HEIGHT) + TOOLTIP_BOX_EXTRA_HEIGHT;
}

static void reset_rich_text(void)
{
    rich_scroll_pos = 0;
    rich_viewport_lines = TOOLTIP_VIEWPORT_LINES;
}

void tooltip_draw_text_box(void)
{
    rich_text_normal_font = image_group(IMAGE_GROUP_FONT_SMALL);
    rich_text_link_font = image_group(IMAGE_GROUP_FONT_SMALL);
    tooltip_box_width = TOOLTIP_TEXT_BOX_WIDTH;
    reset_rich_text();
    g_draw_color = TOOLTIP_MEASURE_COLOR;
    measure_text(tooltip_composed, TOOLTIP_TEXT_BOX_TEXT_WIDTH);
    if (rich_line_count > TOOLTIP_NARROW_LINES_MAX) {
        tooltip_box_width = TOOLTIP_WIDE_BOX_WIDTH;
        reset_rich_text();
        measure_text(tooltip_composed, TOOLTIP_TEXT_BOX_WIDE_TEXT_WIDTH);
    }
    if (mouse_x < tooltip_box_width + TOOLTIP_BORDER) {
        tooltip_box_x = mouse_x + TOOLTIP_MOUSE_GAP;
    } else {
        tooltip_box_x = mouse_x - tooltip_box_width - TOOLTIP_MOUSE_GAP;
    }
    tooltip_box_y = mouse_y + TOOLTIP_TEXT_BOX_BELOW;
    if (mouse_y >= TOOLTIP_LOWER_HALF_Y) {
        tooltip_box_y = mouse_y - TOOLTIP_TEXT_BOX_ABOVE;
    }
    draw_box(tooltip_box_width + TOOLTIP_BORDER, tooltip_box_height);
    rich_text_draw((const uint8_t *) tooltip_composed, tooltip_box_x + TOOLTIP_PADDING, tooltip_box_y + TOOLTIP_TEXT_TOP,
        tooltip_box_width - TOOLTIP_TEXT_BOX_TEXT_INSET, 0, 0);
}

void tooltip_draw_senate(void)
{
    int32_t rgb565 = (display_mode == DISPLAY_MODE_RGB565);
    int32_t shadow = rgb565 ? TOOLTIP_SHADOW_RGB565 : TOOLTIP_SHADOW_RGB555;
    int32_t color = rgb565 ? TOOLTIP_TEXT_RGB565 : TOOLTIP_TEXT_RGB555;
    static const int32_t labels[SENATE_ROWS] = {
        SENATE_TEXT_UNEMPLOYMENT,
        SENATE_TEXT_CULTURE,
        SENATE_TEXT_PROSPERITY,
        SENATE_TEXT_MONUMENT,
        SENATE_TEXT_KINGDOM,
        SENATE_TEXT_TAX,
    };
    const city_player *player = city_player_current();
    int32_t values[SENATE_ROWS] = {
        player->unemployment_percentage,
        player->rating_culture,
        player->rating_prosperity,
        player->rating_monument,
        player->rating_kingdom,
        player->tax_percentage,
    };
    const char *postfixes[SENATE_ROWS] = { pct_postfix, postfix_none, postfix_none, postfix_none, postfix_none, pct_postfix };

    tooltip_box_width = SENATE_WIDTH;
    tooltip_box_height = SENATE_HEIGHT;
    if (mouse_x < SENATE_RIGHT_EDGE) {
        tooltip_box_x = mouse_x + TOOLTIP_MOUSE_GAP;
    } else {
        tooltip_box_x = mouse_x - SENATE_RIGHT_EDGE;
    }
    tooltip_box_y = mouse_y + SENATE_BELOW;
    if (mouse_y >= TOOLTIP_LOWER_HALF_Y) {
        tooltip_box_y = mouse_y - SENATE_ABOVE;
    }
    draw_box(SENATE_WIDTH, SENATE_HEIGHT);

    for (int32_t i = 0, y = 0; i < SENATE_ROWS; i++, y += SENATE_ROW_HEIGHT) {
        int32_t text_x = tooltip_box_x + TOOLTIP_PADDING;
        int32_t value_x = tooltip_box_x + SENATE_VALUE_X;
        int32_t row_y = y + TOOLTIP_PADDING + tooltip_box_y;
        lang_text_draw(LANG_GROUP_BUTTON_TOOLTIPS, labels[i], text_x, row_y, image_group(IMAGE_GROUP_FONT_SMALL), shadow);
        lang_text_draw(LANG_GROUP_BUTTON_TOOLTIPS, labels[i], text_x - SENATE_SHADOW, row_y - SENATE_SHADOW, image_group(IMAGE_GROUP_FONT_SMALL), color);
        text_draw_number(values[i], NUMBER_PREFIX_NONE, postfixes[i], value_x, row_y, image_group(IMAGE_GROUP_FONT_SMALL), shadow);
        text_draw_number(values[i], NUMBER_PREFIX_NONE, postfixes[i], value_x - SENATE_SHADOW, row_y - SENATE_SHADOW, image_group(IMAGE_GROUP_FONT_SMALL), color);
    }
}

static void empire_text(int32_t empire_text_id, uint8_t any_luxury, int32_t empire_index, int32_t *group, int32_t *text_id)
{
    if (empire_text_id == 0) {
        *group = LANG_GROUP_BUTTON_TOOLTIPS;
        *text_id = tooltip_text_id;
        return;
    }
    *group = LANG_GROUP_RESOURCE_NAMES;
    *text_id = empire_text_id;
    if (empire_text_id == EMPIRE_TOOLTIP_LUXURY_GOODS) {
        *text_id = any_luxury ? EMPIRE_TOOLTIP_LUXURY_GOODS_ANY : (&empire_culture_table[0].luxury_kind)[empire_map_objects[empire_index].name_id];
    }
}

static void place_button_tooltip(void)
{
    int32_t near_left = (mouse_x_ui < ui_origin_x + TOOLTIP_ADVISOR_LEFT + tooltip_box_width);
    if (window_current_id == WINDOW_ADVISORS) {
        tooltip_box_x = near_left ? mouse_x_ui + TOOLTIP_ADVISOR_GAP : mouse_x_ui - tooltip_box_width - TOOLTIP_MOUSE_GAP;
        tooltip_box_y = ui_origin_y + TOOLTIP_ADVISOR_BOTTOM;
        if (mouse_y_ui >= tooltip_box_y) {
            return;
        }
        switch (advisor_shown) {
            case ADVISOR_LABOR:
                tooltip_box_y = mouse_y_ui - TOOLTIP_LABOR_ADVISOR_ABOVE;
                return;
            case ADVISOR_TRADE:
                tooltip_box_y = mouse_y_ui - TOOLTIP_TRADE_ADVISOR_ABOVE;
                return;
            case ADVISOR_POPULATION:
                tooltip_box_y = mouse_y_ui - TOOLTIP_POPULATION_ADVISOR_ABOVE;
                return;
            default:
                tooltip_box_y = mouse_y_ui - TOOLTIP_ADVISOR_ABOVE;
                return;
        }
    }
    int32_t below = (ui_origin_y + TOOLTIP_LOWER_HALF_Y <= mouse_y_ui);
    tooltip_box_x = near_left ? mouse_x_ui + TOOLTIP_MOUSE_GAP : mouse_x_ui - tooltip_box_width - TOOLTIP_MOUSE_GAP;
    if (window_current_id == WINDOW_DONATE_TO_CITY) {
        tooltip_box_y = mouse_y_ui - TOOLTIP_DONATE_ABOVE;
    } else if (window_current_id == WINDOW_LABOR_PRIORITY) {
        tooltip_box_y = below ? mouse_y_ui - TOOLTIP_LABOR_PRIORITY_ABOVE : mouse_y_ui + TOOLTIP_BUTTON_BELOW;
        tooltip_box_x = mouse_x_ui - (tooltip_box_width / 2) - TOOLTIP_LABOR_PRIORITY_LEFT;
    } else {
        tooltip_box_y = below ? mouse_y_ui - TOOLTIP_BUTTON_ABOVE : mouse_y_ui + TOOLTIP_BUTTON_BELOW;
    }
}

void tooltip_draw(void)
{
    int32_t text_group = LANG_GROUP_BUTTON_TOOLTIPS;
    int32_t text_id = tooltip_text_id;
    uint8_t flag = 0;

    switch (tooltip_type) {
        case TOOLTIP_KIND_OVERLAY:
            tooltip_draw_text_box();
            return;
        case TOOLTIP_KIND_SENATE:
            tooltip_draw_senate();
            return;
        case TOOLTIP_KIND_LUXURY_GOODS:
            tooltip_draw_luxury_goods();
            return;
        case TOOLTIP_KIND_FIGURE_GOODS:
            tooltip_draw_figure_goods();
            return;
        default:
            break;
    }

    rich_text_normal_font = image_group(IMAGE_GROUP_FONT_SMALL);
    rich_text_link_font = image_group(IMAGE_GROUP_FONT_SMALL);
    tooltip_box_width = TOOLTIP_BUTTON_BOX_WIDTH;
    reset_rich_text();
    g_draw_color = (uint16_t) ((display_mode == DISPLAY_MODE_RGB565) ? TOOLTIP_SHADOW_RGB565 : TOOLTIP_SHADOW_RGB555);

    if (window_current_id == WINDOW_EMPIRE) {
        int32_t empire_index = g_empire_selected_object - 1;
        int32_t empire_text_id = empire_tooltip_text(&flag);
        empire_text(empire_text_id, flag, empire_index, &text_group, &text_id);
    } else if (window_current_id == WINDOW_EDITOR_EMPIRE) {
        int32_t empire_text_id = trade_prices_tooltip_text(&flag);
        empire_text(empire_text_id, flag, (int32_t) g_empire_selected_object, &text_group, &text_id);
    } else if (window_current_id == WINDOW_EDITOR_TRADE_PRICES) {
        empire_text(mission_brief_tooltip_text(), 1, 0, &text_group, &text_id);
    }

    const char *text = lang_get_string(text_group, text_id);
    measure_text(text, tooltip_box_width - TOOLTIP_BORDER);
    if (rich_line_count > TOOLTIP_NARROW_LINES_MAX) {
        tooltip_box_width = TOOLTIP_WIDE_BOX_WIDTH;
        reset_rich_text();
        text = lang_get_string(LANG_GROUP_BUTTON_TOOLTIPS, tooltip_text_id);
        measure_text(text, tooltip_box_width - TOOLTIP_BORDER);
    }
    place_button_tooltip();
    draw_box(tooltip_box_width + TOOLTIP_BORDER, tooltip_box_height);
    rich_text_draw(
        (const uint8_t *) text, tooltip_box_x + TOOLTIP_PADDING, tooltip_box_y + TOOLTIP_TEXT_TOP, tooltip_box_width - TOOLTIP_BORDER, 0, 0);
}

static int32_t is_palace(int16_t type)
{
    return type == BUILDING_TOWN_PALACE || type == BUILDING_VILLAGE_PALACE_UP || type == BUILDING_TOWN_PALACE_UP || type == BUILDING_CITY_PALACE;
}

static int32_t editor_tooltip(void)
{
    int32_t btn = image_button_focus_id;
    int16_t figure_id = map_grids.figure_ids[map_info.selected_grid];
    if (editor_map_preview > 0 && focus_button_id > 0 && focus_button_id <= EDITOR_SCENARIO_BUTTONS) {
        tooltip_text_id = TT_EDITOR_SCENARIO_RANK + focus_button_id - 1;
    }
    if (btn == BUTTON_EDITOR_KILLER_TYPES) {
        tooltip_text_id = TT_EDITOR_KILLER_TYPES;
    } else {
        if (btn > 0 && btn < BUTTON_EDITOR_SIDEBAR_END) {
            tooltip_text_id = TT_EDITOR_KINGDOM_MAP + btn - 1;
        }
        if (btn == BUTTON_EDITOR_ROTATE) {
            tooltip_text_id = TT_EDITOR_ROTATE_VIEW;
        } else if (btn == BUTTON_EDITOR_GRAPHIC) {
            tooltip_text_id = TT_EDITOR_CHANGE_GRAPHIC;
        }
    }
    while (figure_id != 0) {
        const figure *f = figure_get(figure_id);
        if (f->type == FIGURE_MAP_FLAG) {
            activate_box_tooltip(TOOLTIP_KIND_FIGURE_GOODS, WINDOW_CITY);
            return 1;
        }
        figure_id = f->next_figure_id_on_same_tile;
    }
    return no_tooltip();
}

static int32_t palace_tooltip(void)
{
    if (map_info.selected_grid == 0 || (city_overlay != OVERLAY_NONE && city_overlay != OVERLAY_HIDE_CLIFFS) ||
        (map_grids.terrain[map_info.selected_grid] & TERRAIN_ROAD) != 0) {
        return no_tooltip();
    }
    int32_t building_id = map_grids.building_ids[map_info.selected_grid];
    int16_t type = building_get(building_id)->type;
    if (type == BUILDING_FESTIVAL_SQUARE || building_id < 1 || !is_palace(type)) {
        return no_tooltip();
    }
    activate_box_tooltip(TOOLTIP_KIND_SENATE, WINDOW_CITY);
    return 1;
}

static int32_t sidebar_overlay_button_text(int32_t no_overlay_text)
{
    return (rotate_button_hovered_part != 0) ? TT_ORIENT_NORTH + rotate_button_hovered_part - 1 : no_overlay_text;
}

static int32_t top_menu_tooltip(int32_t elapsed, int32_t *handled)
{
    *handled = 1;
    if (top_menu_focus_menu_id != 0) {
        tooltip_text_id = TT_TOP_MENU_FILE + top_menu_focus_menu_id - 1;
        return finish_button_tooltip(elapsed);
    }
    if (top_menu_focus_info_id != 0) {
        tooltip_text_id = TT_CITY_FUNDS + top_menu_focus_info_id - 1;
        return finish_button_tooltip(elapsed);
    }
    *handled = 0;
    return 0;
}

static int32_t tooltip_handle_city(int32_t elapsed)
{
    int32_t btn = image_button_focus_id;
    int32_t handled;
    int32_t result;

    if (editor_active == 1) {
        return editor_tooltip();
    }
    if (g_sidebar_collapsed != 0) {
        if (g_sidebar_collapsed != 1) {
            return 0;
        }
        if (btn == BUTTON_ROTATE) {
            tooltip_text_id = sidebar_overlay_button_text(TT_SHOW_CONTROL_PANEL);
            return finish_button_tooltip(elapsed);
        }
        if (btn >= BUTTON_SIDEBAR_FIRST && btn < BUTTON_SIDEBAR_BUILD_FIRST) {
            tooltip_text_id = TT_SHOW_CONTROL_PANEL;
            return finish_button_tooltip(elapsed);
        }
        if (btn >= BUTTON_SIDEBAR_BUILD_FIRST && btn < BUTTON_SIDEBAR_TOP_FIRST) {
            tooltip_text_id = TT_BUILD_HOUSING + btn - BUTTON_SIDEBAR_BUILD_FIRST;
            return finish_button_tooltip(elapsed);
        }
        result = top_menu_tooltip(elapsed, &handled);
        return handled ? result : palace_tooltip();
    }
    if (btn == BUTTON_ROTATE) {
        tooltip_text_id = sidebar_overlay_button_text(btn);
        return finish_button_tooltip(elapsed);
    }
    if (btn >= BUTTON_SIDEBAR_FIRST && btn < BUTTON_SIDEBAR_BUILD_FIRST) {
        tooltip_text_id = btn;
        return finish_button_tooltip(elapsed);
    }
    if (btn >= BUTTON_SIDEBAR_BUILD_FIRST && btn < BUTTON_SIDEBAR_TOP_FIRST) {
        if (btn != BUTTON_REVIEW_MISSION || settings_data.is_custom_scenario == 0) {
            tooltip_text_id = TT_BUILD_HOUSING + btn - BUTTON_SIDEBAR_BUILD_FIRST;
        }
        return finish_button_tooltip(elapsed);
    }
    if (btn >= BUTTON_SIDEBAR_TOP_FIRST && btn < BUTTON_SIDEBAR_END) {
        tooltip_text_id = TT_VISIT_OVERSEERS + btn - BUTTON_SIDEBAR_TOP_FIRST;
        return finish_button_tooltip(elapsed);
    }
    if (minimap_hovered != 0) {
        tooltip_text_id = TT_MINIMAP;
        return finish_button_tooltip(elapsed);
    }
    result = top_menu_tooltip(elapsed, &handled);
    return handled ? result : palace_tooltip();
}

static int32_t tooltip_handle_festival(int32_t elapsed)
{
    int32_t btn = image_button_focus_id;
    if (btn == BUTTON_HELP) {
        tooltip_text_id = TT_HELP;
        return finish_button_tooltip(elapsed);
    }
    if (btn == BUTTON_EXIT) {
        tooltip_text_id = TT_EXIT;
        return finish_button_tooltip(elapsed);
    }
    if (btn == BUTTON_FESTIVAL_ARRANGE) {
        tooltip_text_id = TT_FESTIVAL_ARRANGE;
        return finish_button_tooltip(elapsed);
    }
    if (btn == BUTTON_FESTIVAL_NONE) {
        tooltip_text_id = TT_FESTIVAL_NONE;
        return finish_button_tooltip(elapsed);
    }
    if (focus_button_id < 1 || focus_button_id > FESTIVAL_GODS) {
        return no_tooltip();
    }
    if (scenario_data.god_status[focus_button_id - 1] != 0) {
        tooltip_text_id = TT_FESTIVAL_OSIRIS + focus_button_id - 1;
    }
    return finish_button_tooltip(elapsed);
}

static int32_t tooltip_handle_advisors(int32_t elapsed)
{
    int32_t btn = image_button_focus_id;
    if (btn >= BUTTON_ADVISOR_FIRST && btn < BUTTON_ADVISOR_HELP_FIRST) {
        tooltip_text_id = TT_ADVISOR_BUTTONS + btn - BUTTON_ADVISOR_FIRST;
        return finish_button_tooltip(elapsed);
    }
    if (btn >= BUTTON_ADVISOR_HELP_FIRST && btn < BUTTON_ADVISOR_END) {
        tooltip_text_id = TT_HELP + btn - BUTTON_ADVISOR_HELP_FIRST;
        return finish_button_tooltip(elapsed);
    }
    switch (advisor_shown) {
        case ADVISOR_LABOR:
            if (focus_button_id != 0) {
                tooltip_text_id = TT_LABOR_SET_PRIORITY;
            } else if (arrow_button_focus_id != 0) {
                tooltip_text_id = TT_LABOR_WAGES;
            } else {
                return no_tooltip();
            }
            break;
        case ADVISOR_KINGDOM:
            if (focus_button_id == 2) {
                if (city_player_current()->enemies == 0) {
                    return 0;
                }
            } else if (focus_button_id == 0 || focus_button_id > 3) {
                return no_tooltip();
            } else if (focus_button_id == 3) {
                tooltip_text_id = TT_SEND_GIFT;
                return finish_button_tooltip(elapsed);
            }
            tooltip_text_id = TT_DONATE_TO_CITY + focus_button_id - 1;
            break;
        case ADVISOR_RATINGS:
            if (focus_button_id < 1 || focus_button_id > RATING_BUTTONS) {
                return no_tooltip();
            }
            tooltip_text_id = TT_RATING_CULTURE_ADVICE + focus_button_id - 1;
            break;
        case ADVISOR_MILITARY:
            if (focus_button_id != FORCES_SWITCH_BUTTON) {
                return no_tooltip();
            }
            tooltip_text_id = TT_VIEW_NAVY;
            break;
        case ADVISOR_FLEET:
            if (focus_button_id != FORCES_SWITCH_BUTTON) {
                return no_tooltip();
            }
            tooltip_text_id = TT_VIEW_ARMY;
            break;
        case ADVISOR_TRADE:
            if (focus_button_id == TRADE_BUTTON_LUXURY_GOODS) {
                activate_box_tooltip(TOOLTIP_KIND_LUXURY_GOODS, WINDOW_ADVISORS);
                return 1;
            }
            if (focus_button_id == 0) {
                return no_tooltip();
            }
            if (focus_button_id == TRADE_BUTTON_PRICES) {
                tooltip_text_id = TT_TRADE_PRICES;
            } else if (focus_button_id == TRADE_BUTTON_EMPIRE) {
                tooltip_text_id = TT_WORLD_MAP;
            } else {
                tooltip_text_id = TT_INDUSTRY_STATUS;
            }
            break;
        case ADVISOR_POPULATION:
            if (focus_button_id == 0) {
                return no_tooltip();
            }
            tooltip_text_id = TT_SELECT_GRAPH;
            break;
        case ADVISOR_RELIGION:
            if (focus_button_id == 0) {
                return no_tooltip();
            }
            tooltip_text_id = TT_FESTIVAL_INFO;
            break;
        case ADVISOR_FINANCIAL:
            if (arrow_button_focus_id == 0) {
                return no_tooltip();
            }
            tooltip_text_id = TT_TAX_RATE;
            break;
        default:
            return no_tooltip();
    }
    return finish_button_tooltip(elapsed);
}

static int32_t building_info_shows_unit(void)
{
    return building_info_mode == BUILDING_INFO_MODE_LEGION || building_info_mode == BUILDING_INFO_MODE_TRANSPORT_SHIP ||
        building_info_mode == BUILDING_INFO_MODE_WARSHIP;
}

static int32_t tooltip_handle_building_info(int32_t elapsed)
{
    int32_t btn = image_button_focus_id;
    if (building_info_shows_unit()) {
        if (btn >= BUTTON_HELP && btn <= BUTTON_INFO_LAST) {
            tooltip_text_id = TT_HELP + btn - BUTTON_HELP;
        }
        return finish_button_tooltip(elapsed);
    }
    if (btn >= BUTTON_HELP && btn <= BUTTON_INFO_LAST) {
        tooltip_text_id = TT_HELP + btn - BUTTON_HELP;
    } else if (btn >= BUTTON_INFO_PHRASE_FIRST && btn <= BUTTON_INFO_PHRASE_LAST) {
        tooltip_text_id = phrase_buttons.building_info_tooltips[btn - BUTTON_INFO_PHRASE_FIRST];
    } else if (building_info_mode == BUILDING_INFO_MODE_BUILDING) {
        int16_t type = building_info_building()->type;
        if (type < BUILDING_HOUSE_CRUDE_HUT || type > BUILDING_HOUSE_PALATIAL_ESTATE || focus_button_id == 0) {
            return no_tooltip();
        }
        tooltip_text_id = TT_TALK;
    } else {
        return no_tooltip();
    }
    return finish_button_tooltip(elapsed);
}

static int32_t empire_button_text(int32_t empire_text_id, int32_t editor)
{
    if (empire_text_id != 0) {
        return empire_text_id + EMPIRE_TOOLTIP_BUTTON_TEXT_BASE;
    }
    if (image_button_focus_id == BUTTON_EMPIRE_COMMERCE) {
        return TT_OVERSEER_OF_COMMERCE;
    }
    if (image_button_focus_id >= BUTTON_HELP && image_button_focus_id <= BUTTON_INFO_LAST) {
        return TT_HELP + image_button_focus_id - BUTTON_HELP;
    }
    if (editor && focus_button_id >= 1 && focus_button_id <= EDITOR_EMPIRE_BUTTONS) {
        return TT_EDITOR_ADD_CITY + focus_button_id - 1;
    }
    if (editor && focus_button_id == EDITOR_EMPIRE_PRICES_BUTTON) {
        return TT_EDITOR_PRICES;
    }
    return 0;
}

static int32_t tooltip_handle_message_list(int32_t elapsed)
{
    if (image_button_focus_id == BUTTON_HELP) {
        tooltip_text_id = TT_HELP;
        return finish_button_tooltip(elapsed);
    }
    if (image_button_focus_id == BUTTON_EXIT) {
        tooltip_text_id = TT_EXIT;
        return finish_button_tooltip(elapsed);
    }
    if (focus_button_id == 0) {
        return no_tooltip();
    }
    city_message_current_id = city_message_list_scroll - 1 + focus_button_id;
    if (city_message_total <= city_message_current_id) {
        return no_tooltip();
    }
    return finish_button_tooltip(elapsed);
}

int32_t tooltip_handle(void)
{
    uint8_t flag = 0;
    int32_t text_id;
    if (window_redraw_request != WINDOW_REDRAW_NONE) {
        return 0;
    }
    if (settings_data.tooltips == TOOLTIPS_NONE) {
        tooltip_last_time = time_current_millis;
        return 0;
    }
    int32_t elapsed = time_current_millis - tooltip_last_time;

    switch (window_current_id) {
        case WINDOW_CITY:
            return tooltip_handle_city(elapsed);
        case WINDOW_ADVISORS:
            return tooltip_handle_advisors(elapsed);
        case WINDOW_LABOR_PRIORITY:
            if (focus_button_id == 0) {
                return no_tooltip();
            }
            tooltip_text_id = (focus_button_id == LABOR_PRIORITY_BUTTON_REMOVE) ? TT_PRIORITY_REMOVE : TT_PRIORITY_LEVEL;
            return finish_button_tooltip(elapsed);
        case WINDOW_SET_SALARY:
        case WINDOW_SALARY_DIALOG:
        case WINDOW_RESOURCE_SETTINGS:
            return no_tooltip();
        case WINDOW_DONATE_TO_CITY:
            if (focus_button_id == 0 && arrow_button_focus_id == 0) {
                return no_tooltip();
            }
            if (focus_button_id == DONATE_BUTTON_EXIT) {
                tooltip_text_id = TT_DONATE_EXIT;
            } else if (focus_button_id == DONATE_BUTTON_DONATE) {
                tooltip_text_id = TT_DONATE;
            } else if (focus_button_id == 0) {
                tooltip_text_id = TT_DONATE_ADJUST;
            } else {
                tooltip_text_id = TT_DONATE_SET_AMOUNT;
            }
            return finish_button_tooltip(elapsed);
        case WINDOW_FESTIVAL_DIALOG:
        case WINDOW_HOLD_FESTIVAL:
            return tooltip_handle_festival(elapsed);
        case WINDOW_BUILDING_INFO:
            return tooltip_handle_building_info(elapsed);
        case WINDOW_FILE_DIALOG:
        case WINDOW_MAP_FILE_DIALOG:
            return 0;
        case WINDOW_EMPIRE:
            text_id = empire_button_text(empire_tooltip_text(&flag), 0);
            if (text_id == 0) {
                return no_tooltip();
            }
            tooltip_text_id = text_id;
            return finish_button_tooltip(elapsed);
        case WINDOW_EDITOR_EMPIRE:
            text_id = empire_button_text(trade_prices_tooltip_text(&flag), 1);
            if (text_id == 0) {
                return no_tooltip();
            }
            tooltip_text_id = text_id;
            return finish_button_tooltip(elapsed);
        case WINDOW_MESSAGE_LIST:
            return tooltip_handle_message_list(elapsed);
        case WINDOW_EDITOR_TRADE_PRICES:
            text_id = mission_brief_tooltip_text();
            if (text_id == 0) {
                return no_tooltip();
            }
            tooltip_text_id = text_id + EMPIRE_TOOLTIP_BUTTON_TEXT_BASE;
            return finish_button_tooltip(elapsed);
        default:
            return no_tooltip();
    }
}

static int32_t service_text(uint8_t access, int32_t first_text)
{
    if (access == 0) {
        return first_text + SERVICE_ACCESS_NONE;
    }
    if (access >= SERVICE_ACCESS_VERY_GOOD_FROM) {
        return first_text + SERVICE_ACCESS_VERY_GOOD;
    }
    if (access < SERVICE_ACCESS_GOOD_FROM) {
        return first_text + SERVICE_ACCESS_POOR;
    }
    return first_text + SERVICE_ACCESS_GOOD;
}

static int32_t water_text(const building *b, int32_t grid_offset)
{
    uint32_t terrain = map_grids.terrain[grid_offset];
    if (b == 0 || b->house_size == 0) {
        if ((terrain & TERRAIN_WELL_RANGE) != 0) {
            return TOOLTIP_WATER_WELL_RANGE;
        }
        return ((terrain & TERRAIN_GROUNDWATER) != 0) ? TOOLTIP_WATER_GROUNDWATER : -1;
    }
    uint8_t supply = b->data.house.water_supply;
    if (supply >= WATER_SUPPLY_FULL_FROM) {
        return TOOLTIP_WATER_SUPPLY_FULL;
    }
    if (supply >= WATER_SUPPLY_GOOD_FROM) {
        return TOOLTIP_WATER_SUPPLY_GOOD;
    }
    if (supply != 0) {
        return TOOLTIP_WATER_SUPPLY_LOW;
    }
    if ((terrain & TERRAIN_WELL_RANGE) != 0 || b->has_well_access != 0) {
        return TOOLTIP_WATER_WELL_RANGE;
    }
    return TOOLTIP_WATER_NO_WELL_HOUSE;
}

static int32_t bazaar_text(const building *b)
{
    int32_t capacity = 0;
    int32_t stored = 0;
    if (b->house_population > 0 && house_models[b->subtype].food_types != 0) {
        capacity = b->house_population;
    }
    for (int32_t resource = BAZAAR_FOOD_FIRST; resource < BAZAAR_FOOD_END; resource++) {
        if (empire_can_produce_resource(resource) != 0 || empire_can_import_resource_potentially(resource) != 0) {
            stored += b->data.house.inventory[resource - RESOURCE_MIN_FOOD];
        }
    }
    if (capacity < 1) {
        return TOOLTIP_BAZAAR_NO_PEOPLE;
    }
    int32_t percent = calc_percentage(stored, capacity);
    if (percent >= BAZAAR_PLENTY_PERCENT) {
        return TOOLTIP_BAZAAR_PLENTY_FOOD;
    }
    if (percent >= BAZAAR_ENOUGH_PERCENT) {
        return TOOLTIP_BAZAAR_ENOUGH_FOOD;
    }
    return (percent < 1) ? TOOLTIP_BAZAAR_NO_FOOD : TOOLTIP_BAZAAR_SOME_FOOD;
}

static int32_t labor_text(const building *b)
{
    if (b->houses_covered < 1) {
        return TOOLTIP_LABOR;
    }
    return TOOLTIP_LABOR + 1 + tooltip_band_index(TOOLTIP_BAND_LABOR, calc_percentage(b->houses_covered, LABOR_FULL));
}

static int32_t education_text(const building *b)
{
    switch (b->data.house.education) {
        case HOUSE_EDUCATION_NONE:
            return TOOLTIP_EDUCATION_NONE;
        case HOUSE_EDUCATION_SCHOOL_OR_LIBRARY:
            return TOOLTIP_EDUCATION_SCHOOL;
        case HOUSE_EDUCATION_SCHOOL_AND_LIBRARY:
            return TOOLTIP_EDUCATION_LIBRARY;
        default:
            return -1;
    }
}

static int32_t physician_text(uint8_t access)
{
    if (access == 0) {
        return TOOLTIP_PHYSICIAN_NONE;
    }
    if (access >= SERVICE_ACCESS_VERY_GOOD_FROM) {
        return TOOLTIP_PHYSICIAN_VERY_GOOD;
    }
    return (access < SERVICE_ACCESS_GOOD_FROM) ? TOOLTIP_PHYSICIAN_POOR : TOOLTIP_PHYSICIAN_GOOD;
}

static int32_t service_access_text(const building *b, int32_t overlay)
{
    switch (overlay) {
        case OVERLAY_JUGGLER:
            return service_text(b->data.house.juggler, TOOLTIP_JUGGLER);
        case OVERLAY_MUSICIAN:
            return service_text(b->data.house.musician, TOOLTIP_MUSICIAN);
        case OVERLAY_DANCER:
            return service_text(b->data.house.dancer, TOOLTIP_DANCER);
        case OVERLAY_SENET:
            return service_text(b->data.house.senet, TOOLTIP_SENET);
        case OVERLAY_ZOOKEEPER:
            return service_text(b->data.house.zoo, TOOLTIP_ZOOKEEPER);
        case OVERLAY_SCRIBAL_SCHOOL:
            return service_text(b->data.house.school, TOOLTIP_SCRIBAL_SCHOOL);
        case OVERLAY_LIBRARY:
            return service_text(b->data.house.library, TOOLTIP_LIBRARY);
        case OVERLAY_DENTIST:
            return service_text(b->data.house.dentist, TOOLTIP_DENTIST);
        case OVERLAY_APOTHECARY:
            return service_text(b->data.house.apothecary, TOOLTIP_APOTHECARY);
        case OVERLAY_MORTUARY:
            return service_text(b->data.house.mortuary, TOOLTIP_MORTUARY);
        case OVERLAY_MAGISTRATE:
            return service_text(b->data.house.courthouse, TOOLTIP_MAGISTRATE);
        default:
            return -1;
    }
}

static int32_t risk_text(const building *b, int32_t overlay)
{
    switch (overlay) {
        case OVERLAY_RELIGION:
            return TOOLTIP_RELIGION_GODS + tooltip_band_index(TOOLTIP_BAND_GODS, b->data.house.num_gods);
        case OVERLAY_FIRE:
            return TOOLTIP_FIRE_RISK + tooltip_band_index(TOOLTIP_BAND_FIRE, b->fire_risk);
        case OVERLAY_DAMAGE:
            return TOOLTIP_DAMAGE_RISK + tooltip_band_index(TOOLTIP_BAND_DAMAGE, b->damage_risk);
        case OVERLAY_CRIME:
            return TOOLTIP_CRIME_RISK + tooltip_band_index(TOOLTIP_BAND_CRIME, b->crime_risk);
        case OVERLAY_ENTERTAINMENT:
            return TOOLTIP_ENTERTAINMENT + tooltip_band_index(TOOLTIP_BAND_ENTERTAINMENT, b->data.house.entertainment);
        default:
            return -1;
    }
}

static int32_t desirability_text(int8_t desirability)
{
    return TOOLTIP_DESIRABILITY + tooltip_band_index(TOOLTIP_BAND_DESIRABILITY, desirability);
}

static int32_t fertility_text(uint8_t fertility)
{
    return TOOLTIP_FERTILITY_NONE - tooltip_band_index(TOOLTIP_BAND_FERTILITY, fertility);
}

static const char *goods_tooltip(int32_t overlay, int32_t main_id, int32_t *composed_len)
{
    goods_overlay goods;
    if (!city_overlay_goods(overlay, &goods) || !goods.shows_tooltip) {
        return NULL;
    }
    return tooltip_overlay_goods(main_id, goods.resource, goods.inventory_slot, composed_len);
}

static int32_t is_menu_title_overlay(int32_t overlay)
{
    return overlay == OVERLAY_NONE || (overlay >= 1 && overlay <= 3) || (overlay >= 5 && overlay <= 7);
}

static int32_t overlay_shows_tooltip_at(int32_t overlay, int32_t main_id, int32_t grid_offset)
{
    const overlay_tooltip_rule *rule = &overlay_tooltip_rules[overlay];
    if (rule->needs_building != 0 && main_id == 0) {
        return 0;
    }
    uint8_t house_size = building_get(main_id)->house_size;
    if (rule->buildings == OVERLAY_TOOLTIP_HOUSES_ONLY && house_size == 0) {
        return 0;
    }
    if (rule->buildings == OVERLAY_TOOLTIP_NO_HOUSES && house_size != 0) {
        return 0;
    }
    return rule->needs_fertile_soil == 0 || map_grids.soil_fertility[grid_offset] != 0;
}

static void activate_overlay_tooltip(int32_t grid_offset)
{
    save_rich_text_state();
    tooltip_window_id = window_current_id;
    tooltip_text_id = TOOLTIP_ACTIVE;
    tooltip_type = TOOLTIP_KIND_OVERLAY;
    window_redraw_request = WINDOW_REDRAW_ALL;
}

static int32_t overlay_text(const building *b, int32_t overlay, int32_t main_id, int32_t grid_offset, int32_t *composed_len)
{
    int32_t text_id;
    switch (overlay) {
        case OVERLAY_WATER:
            return water_text(b, grid_offset);
        case OVERLAY_RELIGION:
        case OVERLAY_FIRE:
        case OVERLAY_DAMAGE:
        case OVERLAY_CRIME:
        case OVERLAY_ENTERTAINMENT:
            return risk_text(b, overlay);
        case OVERLAY_JUGGLER:
        case OVERLAY_MUSICIAN:
        case OVERLAY_DANCER:
        case OVERLAY_SENET:
        case OVERLAY_ZOOKEEPER:
        case OVERLAY_SCRIBAL_SCHOOL:
        case OVERLAY_LIBRARY:
        case OVERLAY_DENTIST:
        case OVERLAY_APOTHECARY:
        case OVERLAY_MORTUARY:
        case OVERLAY_MAGISTRATE:
            return service_access_text(b, overlay);
        case OVERLAY_EDUCATION:
            return education_text(b);
        case OVERLAY_PHYSICIAN:
            return physician_text(b->data.house.physician);
        case OVERLAY_TAX_INCOME:
            text_id = building_house_tax_income(main_id);
            if (text_id < 1) {
                return (b->house_tax_coverage == 0) ? TOOLTIP_TAX_NOT_REGISTERED : TOOLTIP_TAX_NO_INCOME;
            }
            string_from_int(text_id, 0, 0);
            *composed_len = 0;
            append_cstr(tooltip_composed, composed_len, string_number_buffer);
            return TOOLTIP_TAX_INCOME;
        case OVERLAY_BAZAAR_ACCESS:
            return bazaar_text(b);
        case OVERLAY_DESIRABILITY:
            return desirability_text(map_grids.desirability[grid_offset]);
        case OVERLAY_FERTILITY:
            return fertility_text(map_grids.soil_fertility[grid_offset]);
        case OVERLAY_LABOR:
            return labor_text(b);
        default:
            return -1;
    }
}

int32_t tooltip_handle_overlay(void)
{
    int32_t grid_offset = map_info.selected_grid;
    int32_t overlay = city_overlay;
    int32_t main_id = 0;
    int32_t composed_len = 0;

    if (settings_data.tooltips == TOOLTIPS_NONE || mouse_right_down != 0 || window_current_id != WINDOW_CITY || editor_active == 1 || grid_offset == 0 ||
        is_menu_title_overlay(overlay)) {
        return 0;
    }
    if ((map_grids.terrain[grid_offset] & TERRAIN_ROAD) == 0 && building_get(map_grids.building_ids[grid_offset])->type != BUILDING_FESTIVAL_SQUARE) {
        main_id = building_main(map_grids.building_ids[grid_offset]);
    }
    if (!overlay_shows_tooltip_at(overlay, main_id, grid_offset)) {
        return 0;
    }

    const char *text = lang_get_string(LANG_GROUP_OVERLAY_TOOLTIPS, 0);
    const building *b = main_id ? building_get(main_id) : 0;

    switch (overlay) {
        case OVERLAY_NATIVE:
        case OVERLAY_CITY_DEFENSES:
        case OVERLAY_HIDE_CLIFFS:
            return 0;
        case OVERLAY_PROBLEMS:
            if (tooltip_overlay_problems(main_id) == 0) {
                return 0;
            }
            activate_overlay_tooltip(grid_offset);
            return 1;
        case OVERLAY_DISEASE:
            text = tooltip_overlay_disease(main_id);
            break;
        case OVERLAY_MALARIA:
            text = tooltip_overlay_malaria(main_id);
            break;
        case OVERLAY_INFECTED_HOUSING:
            if (b->house_size == 0) {
                return 0;
            }
            if (b->data.house.is_sick == HOUSE_SICK_DISEASE_KIND) {
                text = lang_get_string(LANG_GROUP_OVERLAY_TOOLTIPS, TOOLTIP_SICK_DISEASE);
            } else if (b->data.house.is_sick == HOUSE_SICK_FROGS_KIND) {
                text = lang_get_string(LANG_GROUP_OVERLAY_TOOLTIPS, TOOLTIP_SICK_FROGS);
            } else {
                return 0;
            }
            break;
        case OVERLAY_EMPTY_HOUSING:
            if (b->house_size == 0 || b->house_population != 0) {
                return 0;
            }
            text = lang_get_string(LANG_GROUP_OVERLAY_TOOLTIPS, TOOLTIP_EMPTY_HOUSE);
            break;
        default: {
            const char *goods_text = goods_tooltip(overlay, main_id, &composed_len);
            if (goods_text) {
                text = goods_text;
                break;
            }
            int32_t text_id = overlay_text(b, overlay, main_id, grid_offset, &composed_len);
            if (text_id >= 0) {
                text = lang_get_string(LANG_GROUP_OVERLAY_TOOLTIPS, text_id);
            } else if (overlay == OVERLAY_WATER) {
                return 0;
            }
            break;
        }
    }

    append_cstr(tooltip_composed, &composed_len, text);
    activate_overlay_tooltip(grid_offset);
    return 1;
}
