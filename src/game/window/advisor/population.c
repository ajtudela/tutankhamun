// The population advisor: background and foreground, the history / census / society graphs with their scale, and the
// buttons that choose the graph.

#include "window/advisor/population.h"
#include "city/city_internal.h"
#include "city/food_supply.h"
#include "city/population.h"
#include "city/population_extra.h"
#include "game/game_internal.h"
#include "graphics/color.h"
#include "graphics/graphics_data.h"
#include "graphics/graphics_present.h"
#include "graphics/graphics_primitives.h"
#include "graphics/image_collection.h"
#include "graphics/image_draw.h"
#include "graphics/label_animation_slots.h"
#include "graphics/panel_draw.h"
#include "graphics/text_draw.h"
#include "graphics/ui_layout.h"
#include "platform/log.h"
#include "window/advisors_internal.h"
#include "window/window_image_group.h"
#include <stdint.h>

int32_t graph_y_shift;

int32_t graph_y_max;

int32_t population_last_focus = -1;

static void population_graph_set_y_axis(int32_t max_value)
{
    if (max_value <= 100) {
        graph_y_max = 100;
        graph_y_shift = -1;
        return;
    }
    if (max_value <= 200) {
        graph_y_max = 200;
        graph_y_shift = 0;
        return;
    }
    if (max_value <= 400) {
        graph_y_max = 400;
        graph_y_shift = 1;
        return;
    }
    if (max_value <= 800) {
        graph_y_max = 800;
        graph_y_shift = 2;
        return;
    }
    if (max_value <= 1600) {
        graph_y_max = 1600;
        graph_y_shift = 3;
        return;
    }
    if (max_value <= 3200) {
        graph_y_max = 3200;
        graph_y_shift = 4;
        return;
    }
    if (max_value <= 6400) {
        graph_y_max = 6400;
        graph_y_shift = 5;
        return;
    }
    if (max_value <= 12800) {
        graph_y_max = 12800;
        graph_y_shift = 6;
        return;
    }
    if (max_value <= 25600) {
        graph_y_max = 25600;
        graph_y_shift = 7;
        return;
    }
    graph_y_max = 51200;
    graph_y_shift = 8;
}

static int32_t history_max_months(int32_t full_size)
{
    int32_t count = city_player_current()->monthly_population_count;
    int32_t max_months;
    if (count <= 20) {
        max_months = 20;
    } else if (count <= 40) {
        max_months = 40;
    } else if (count <= 100) {
        max_months = 100;
    } else if (count > 200) {
        max_months = 400;
    } else {
        max_months = 200;
    }
    if (full_size == 0) {
        max_months = (max_months > 40) ? 100 : 20;
    }
    return max_months;
}

static int32_t bar_height(int32_t pop)
{
    if (graph_y_shift == -1) {
        return pop << 1;
    }
    return pop >> graph_y_shift;
}

void window_advisor_population_draw_history_graph(int32_t full_size, int32_t x, int32_t y)
{
    int32_t max_months = history_max_months(full_size);
    int32_t max_value = 0;
    int32_t m;
    for (m = 0; m < max_months; m++) {
        int32_t value = city_population_at_month(max_months, m);
        if (value > max_value) {
            max_value = value;
        }
    }
    population_graph_set_y_axis(max_value);

    if (full_size == 0) {
        int32_t x_pos = x;
        graph_y_shift += 2;
        for (m = 0; m < max_months; m++) {
            int32_t val = city_population_at_month(max_months, m) >> graph_y_shift;
            if (val > 0) {
                if (max_months == 20) {
                    graphics_fill_rect(x_pos, (y - val) + 50, 4, val + 1, COLOR_RED);
                } else {
                    graphics_draw_line(x + m, (y - val) + 50, x + m, y + 50, COLOR_RED);
                }
            }
            x_pos += 5;
        }
        return;
    }

    text_draw_number_centered(graph_y_max, NUMBER_PREFIX_NONE, postfix_none, x - 66, y - 3, 60, image_group(IMAGE_GROUP_FONT_SMALL), 0);
    text_draw_number_centered(graph_y_max / 2, NUMBER_PREFIX_NONE, postfix_none, x - 66, y + 96, 60, image_group(IMAGE_GROUP_FONT_SMALL), 0);
    text_draw_number_centered(0, NUMBER_PREFIX_NONE, postfix_none, x - 66, y + 196, 60, image_group(IMAGE_GROUP_FONT_SMALL), 0);
    city_population_graph_axis(max_months);
    g_text_width = 0;
    lang_text_draw_ui(25, month_axis_start_month, x - 20, y + 210, image_group(IMAGE_GROUP_FONT_SMALL), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_year(0, month_axis_start_year, g_text_width - 20 + x, y + 210, image_group(IMAGE_GROUP_FONT_SMALL), 0);
    g_text_width = 0;
    lang_text_draw_ui(25, month_axis_end_month, x + 380, y + 210, image_group(IMAGE_GROUP_FONT_SMALL), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_year(0, month_axis_end_year, g_text_width + 380 + x, y + 210, image_group(IMAGE_GROUP_FONT_SMALL), 0);

    g_clip_y1 = y + 200;
    {
        int32_t x20 = x;
        int32_t x40 = x;
        int32_t x100 = x;
        int32_t x200 = x;
        int32_t x_base = x;
        for (m = 0; m < max_months; m++) {
            int32_t pop = city_population_at_month(max_months, m);
            int32_t val = bar_height(pop);
            if (val > 0) {
                if (max_months == 20) {
                    image_draw(image_group(IMAGE_GROUP_POPULATION_GRAPH_BAR), x20, (y - val) + 200);
                } else if (max_months == 40) {
                    image_draw(image_group(IMAGE_GROUP_POPULATION_GRAPH_BAR) + 1, x40, (y - val) + 200);
                } else if (max_months == 100) {
                    image_draw(image_group(IMAGE_GROUP_POPULATION_GRAPH_BAR) + 2, x100, (y - val) + 200);
                } else if (max_months == 200) {
                    image_draw(image_group(IMAGE_GROUP_POPULATION_GRAPH_BAR) + 3, x200, (y - val) + 200);
                } else {
                    graphics_draw_line(m + x_base, (y - val) + 200, m + x_base, y + 199, COLOR_RED);
                }
            }
            x20 += 20;
            x40 += 10;
            x100 += 4;
            x200 += 2;
        }
    }
    g_clip_y1 = g_screen_height;
}

void window_advisor_population_draw_census_graph(int32_t full_size, int32_t x, int32_t y)
{
    int32_t max_value = 0;
    int32_t i;
    for (i = 0; i < 100; i++) {
        int32_t value = city_player_current()->population_at_age[i];
        if (value > max_value) {
            max_value = value;
        }
    }
    population_graph_set_y_axis(max_value);

    if (full_size != 0) {
        int32_t label_x = x - 66;
        text_draw_number_centered(graph_y_max, NUMBER_PREFIX_NONE, postfix_none, label_x, y - 3, 60, image_group(IMAGE_GROUP_FONT_SMALL), 0);
        text_draw_number_centered(graph_y_max / 2, NUMBER_PREFIX_NONE, postfix_none, label_x, y + 96, 60, image_group(IMAGE_GROUP_FONT_SMALL), 0);
        text_draw_number_centered(0, NUMBER_PREFIX_NONE, postfix_none, label_x, y + 196, 60, image_group(IMAGE_GROUP_FONT_SMALL), 0);
        for (int32_t age = 0, ax = x - 22; age <= 100; age += 10, ax += 40) {
            text_draw_number_centered(age, NUMBER_PREFIX_NONE, postfix_none, ax, y + 210, 40, image_group(IMAGE_GROUP_FONT_SMALL), 0);
        }
        g_clip_y1 = y + 200;
        {
            int32_t bx = x;
            for (i = 0; i < 100; i++) {
                int32_t val = bar_height(city_player_current()->population_at_age[i]);
                if (val > 0) {
                    image_draw(image_group(IMAGE_GROUP_POPULATION_GRAPH_BAR) + 2, bx, (y - val) + 200);
                }
                bx += 4;
            }
        }
        g_clip_y1 = g_screen_height;
        return;
    }

    graph_y_shift += 2;
    for (i = 0; i < 100; i++) {
        int32_t val = city_player_current()->population_at_age[i] >> graph_y_shift;
        if (val > 0) {
            graphics_draw_line(i + x, (y - val) + 50, i + x, y + 50, COLOR_RED);
        }
    }
}

void window_advisor_population_draw_society_graph(int32_t full_size, int32_t x, int32_t y)
{
    int32_t max_value = 0;
    int32_t i;
    for (i = 0; i < 20; i++) {
        int32_t value = city_player_current()->population_per_level[i];
        if (value > max_value) {
            max_value = value;
        }
    }
    population_graph_set_y_axis(max_value);

    if (full_size != 0) {
        int32_t label_x = x - 66;
        text_draw_number_centered(graph_y_max, NUMBER_PREFIX_NONE, postfix_none, label_x, y - 3, 60, image_group(IMAGE_GROUP_FONT_SMALL), 0);
        text_draw_number_centered(graph_y_max / 2, NUMBER_PREFIX_NONE, postfix_none, label_x, y + 96, 60, image_group(IMAGE_GROUP_FONT_SMALL), 0);
        text_draw_number_centered(0, NUMBER_PREFIX_NONE, postfix_none, label_x, y + 196, 60, image_group(IMAGE_GROUP_FONT_SMALL), 0);
        lang_text_draw_centered_ui(55, 9, x - 80, y + 210, 200, image_group(IMAGE_GROUP_FONT_SMALL), 0, ui_origin_x, ui_origin_y);
        lang_text_draw_centered_ui(55, 10, x + 280, y + 210, 200, image_group(IMAGE_GROUP_FONT_SMALL), 0, ui_origin_x, ui_origin_y);
        g_clip_y1 = y + 200;
        {
            int32_t bx = x;
            for (i = 0; i < 20; i++) {
                int32_t val = bar_height(city_player_current()->population_per_level[i]);
                if (val > 0) {
                    image_draw(image_group(IMAGE_GROUP_POPULATION_GRAPH_BAR), bx, (y - val) + 200);
                }
                bx += 20;
            }
        }
        g_clip_y1 = g_screen_height;
        return;
    }

    graph_y_shift += 2;
    {
        int32_t bx = x;
        for (i = 0; i < 20; i++) {
            int32_t val = city_player_current()->population_per_level[i] >> graph_y_shift;
            if (val > 0) {
                graphics_fill_rect(bx, (y - val) + 50, 4, val + 1, COLOR_RED);
            }
            bx += 5;
        }
    }
}

static void draw_graphs_for_order(int32_t order, int32_t ox, int32_t oy)
{
    int32_t big_text;
    int32_t top_text;
    int32_t bot_text;
    switch (order) {
        case 0:
            big_text = 6;
            top_text = 4;
            bot_text = 5;
            break;
        case 1:
            big_text = 6;
            top_text = 5;
            bot_text = 4;
            break;
        case 2:
            big_text = 7;
            top_text = 3;
            bot_text = 5;
            break;
        case 3:
            big_text = 7;
            top_text = 5;
            bot_text = 3;
            break;
        case 4:
            big_text = 8;
            top_text = 3;
            bot_text = 4;
            break;
        default:
            big_text = 8;
            top_text = 4;
            bot_text = 3;
            break;
    }
    lang_text_draw_centered_ui(55, big_text, ox + 60, oy + 280, 400, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_centered_ui(55, top_text, ox + 504, oy + 120, 100, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_centered_ui(55, bot_text, ox + 504, oy + 220, 100, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);

    switch (order) {
        case 0:
            window_advisor_population_draw_history_graph(1, ox + 64, oy + 64);
            window_advisor_population_draw_census_graph(0, ox + 505, oy + 63);
            window_advisor_population_draw_society_graph(0, ox + 505, oy + 163);
            break;
        case 1:
            window_advisor_population_draw_history_graph(1, ox + 64, oy + 64);
            window_advisor_population_draw_society_graph(0, ox + 505, oy + 63);
            window_advisor_population_draw_census_graph(0, ox + 505, oy + 163);
            break;
        case 2:
            window_advisor_population_draw_census_graph(1, ox + 64, oy + 64);
            window_advisor_population_draw_history_graph(0, ox + 505, oy + 63);
            window_advisor_population_draw_society_graph(0, ox + 505, oy + 163);
            break;
        case 3:
            window_advisor_population_draw_census_graph(1, ox + 64, oy + 64);
            window_advisor_population_draw_society_graph(0, ox + 505, oy + 63);
            window_advisor_population_draw_history_graph(0, ox + 505, oy + 163);
            break;
        case 4:
            window_advisor_population_draw_society_graph(1, ox + 64, oy + 64);
            window_advisor_population_draw_history_graph(0, ox + 505, oy + 63);
            window_advisor_population_draw_census_graph(0, ox + 505, oy + 163);
            break;
        default:
            window_advisor_population_draw_society_graph(1, ox + 64, oy + 64);
            window_advisor_population_draw_census_graph(0, ox + 505, oy + 63);
            window_advisor_population_draw_history_graph(0, ox + 505, oy + 163);
            break;
    }
}

static void draw_migration_panel(int32_t ox, int32_t oy)
{
    int32_t food_shown;
    int32_t pct;
    int32_t text_id;
    int32_t color;
    int32_t status;
    int32_t cause;

    if (city_player_current()->population == 0) {
        pct = 0;
    } else {
        pct = (city_resource_food_supply_percentage() * 100) / city_player_current()->population;
    }
    food_shown = city_resource_food_supply_percentage();

    color = image_group(IMAGE_GROUP_FONT_GREEN);
    if (pct >= 151) {
        text_id = 51;
    } else if (pct >= 111) {
        text_id = 48;
    } else if (pct >= 96) {
        text_id = 45;
    } else if (pct >= 86) {
        text_id = 42;
    } else if (pct >= 71) {
        text_id = 39;
        color = image_group(IMAGE_GROUP_FONT_WHITE);
    } else {
        text_id = 36;
        color = image_group(IMAGE_GROUP_FONT_RED);
    }
    if (city_player_current()->migration_percentage < 0) {
        text_id += 1;
    } else if (city_player_current()->migration_percentage > 0) {
        text_id += 2;
    }

    g_text_width = 0;
    lang_text_draw_ui(55, 34, ox + 75, oy + 308, image_group(IMAGE_GROUP_FONT_GREEN), 0, ui_origin_x, ui_origin_y);
    text_draw_number_ui(food_shown, NUMBER_PREFIX_NONE, postfix_none, g_text_width + 75 + ox, oy + 308, image_group(IMAGE_GROUP_FONT_GREEN), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_ui(55, 35, g_text_width + 75 + ox, oy + 308, image_group(IMAGE_GROUP_FONT_GREEN), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_ui(55, text_id, ox + 75, oy + 324, color, 0, ui_origin_x, ui_origin_y);

    if (scenario_data.kingdom_supplies_grain == 0) {
        g_text_width = 0;
        lang_text_draw_amount(8, 6, city_player_current()->granaries_operating, ox + 75, oy + 342, image_group(IMAGE_GROUP_FONT_WHITE), 0);
        if (city_player_current()->food_supply_months < 1) {
            if (city_player_current()->food_needed_per_month / 2 < city_player_current()->food_stored_total) {
                lang_text_draw_ui(55, 13, g_text_width + 75 + ox, oy + 342, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
            } else if (city_player_current()->food_stored_total < 1) {
                lang_text_draw_ui(55, 14, g_text_width + 75 + ox, oy + 342, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
            } else {
                lang_text_draw_ui(55, 15, g_text_width + 75 + ox, oy + 342, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
            }
        } else {
            lang_text_draw_ui(55, 12, g_text_width + 75 + ox, oy + 342, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
            lang_text_draw_amount(8, 4, city_player_current()->food_supply_months, g_text_width + 75 + ox, oy + 342, image_group(IMAGE_GROUP_FONT_WHITE), 0);
        }
    } else {
        lang_text_draw_ui(55, 11, ox + 75, oy + 342, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
    }

    g_text_width = 0;
    lang_text_draw_ui(55, 16, ox + 75, oy + 360, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
    text_draw_number_ui(
        city_player_current()->room_in_houses, NUMBER_PREFIX_NONE, postfix_none, g_text_width + 75 + ox, oy + 360, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_ui(55, 17, g_text_width + 75 + ox, oy + 360, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);

    status = city_player_current()->migration_direction;
    cause = city_player_current()->no_immigration_cause;
    if (status == 0) {
        g_text_width = 0;
        if (city_player_current()->kingdom_soldiers + city_player_current()->enemies > 3) {
            lang_text_draw_ui(55, 26, ox + 75, oy + 378, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
            g_text_width = 0;
            lang_text_draw_ui(61, 43, ox + 75, oy + 396, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
            return;
        }
        if (city_player_current()->migration_newcomers > 4) {
            lang_text_draw_ui(55, 25, ox + 75, oy + 378, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
            g_text_width = 0;
            text_draw_number_ui(city_player_current()->migration_newcomers, NUMBER_PREFIX_NONE, postfix_none, ox + 75, oy + 396, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
            lang_text_draw_ui(55, 18, g_text_width + 75 + ox, oy + 396, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
            return;
        }
        if (city_player_current()->refused_immigrants_today != 0 || city_player_current()->room_in_houses < 1) {
            lang_text_draw_ui(55, 25, ox + 75, oy + 378, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
            g_text_width = 0;
            lang_text_draw_ui(55, 20, ox + 75, oy + 396, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
            return;
        }
        if (city_player_current()->migration_percentage > 79) {
            lang_text_draw_ui(55, 25, ox + 75, oy + 378, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
            g_text_width = 0;
            text_draw_number_ui(city_player_current()->migration_newcomers, NUMBER_PREFIX_NONE, postfix_none, ox + 75, oy + 396, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
            if (city_player_current()->migration_newcomers != 1) {
                lang_text_draw_ui(55, 18, g_text_width + 75 + ox, oy + 396, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
                return;
            }
            lang_text_draw_ui(55, 19, g_text_width + 75 + ox, oy + 396, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
            return;
        }
        lang_text_draw_ui(55, 26, ox + 75, oy + 378, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
        g_text_width = 0;
        if (cause == 0) {
            lang_text_draw_ui(55, 21, ox + 75, oy + 396, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
        }
        if (cause == 1) {
            lang_text_draw_ui(55, 22, g_text_width + 75 + ox, oy + 396, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
        }
        if (cause == 2) {
            lang_text_draw_ui(55, 23, g_text_width + 75 + ox, oy + 396, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
        }
        if (cause == 3) {
            lang_text_draw_ui(55, 24, g_text_width + 75 + ox, oy + 396, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
        }
        if (cause == 4) {
            lang_text_draw_ui(55, 32, g_text_width + 75 + ox, oy + 396, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
        }
        if (cause == 5) {
            lang_text_draw_ui(55, 33, g_text_width + 75 + ox, oy + 396, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
        }
        return;
    }

    if (status == 1) {
        lang_text_draw_ui(55, 26, ox + 75, oy + 378, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
        if (cause == 0) {
            lang_text_draw_ui(55, 21, g_text_width + 75 + ox, oy + 396, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
        }
        if (cause == 1) {
            lang_text_draw_ui(55, 22, g_text_width + 75 + ox, oy + 396, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
        }
        if (cause == 2) {
            lang_text_draw_ui(55, 23, g_text_width + 75 + ox, oy + 396, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
        }
        if (cause == 3) {
            lang_text_draw_ui(55, 24, g_text_width + 75 + ox, oy + 396, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
        }
        if (cause == 4) {
            lang_text_draw_ui(55, 32, g_text_width + 75 + ox, oy + 396, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
        }
        if (cause == 5) {
            lang_text_draw_ui(55, 33, g_text_width + 75 + ox, oy + 396, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
        }
        return;
    }

    if (status == 2) {
        lang_text_draw_multiline_ui(55, 30, ox + 75, oy + 378, 496, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
        return;
    }
    lang_text_draw_multiline_ui(55, 31, ox + 75, oy + 378, 496, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
}

void window_advisor_population_draw_background(void)
{
    int32_t remain = 5 - label_animation[LABEL_SLOT_ADVISOR_OPEN];
    int32_t title;
    int32_t ox;
    int32_t oy;

    advisor_x = ui_origin_x;
    advisor_y = ui_origin_y;
    advisor_height_blocks = 27;
    if (remain > 4) {
        return;
    }
    if (remain > 0) {
        graphics_draw_rect(remain * 16 + ui_origin_x, ui_origin_y + remain * 8, (remain * -2 + 40) * 16, (remain * -2 + 27) * 16, 0);
        return;
    }

    outer_panel_draw(ui_origin_x, ui_origin_y, 40, 27);
    image_draw(image_group(IMAGE_GROUP_ADVISOR_ICONS) + 5, advisor_x + 10, advisor_y + 10);

    if (city_population_graph_order < 4) {
        if (city_population_graph_order > 1) {
            title = 1;
        } else if (city_population_graph_order >= 0) {
            title = 0;
        } else {
            title = -1;
        }
    } else {
        title = 2;
    }
    if (title >= 0) {
        lang_text_draw_ui(55, title, advisor_x + 60, advisor_y + 12, image_group(IMAGE_GROUP_FONT_LARGE), 0, ui_origin_x, ui_origin_y);
    }

    image_draw(image_group(IMAGE_GROUP_PANEL_WINDOWS) + 14, advisor_x + 56, advisor_y + 60);
    button_border_draw_ui(advisor_x + 501, advisor_y + 60, 106, 57, 0, ui_origin_x, ui_origin_y);
    button_border_draw_ui(advisor_x + 501, advisor_y + 160, 106, 57, 0, ui_origin_x, ui_origin_y);

    ox = advisor_x;
    oy = advisor_y;
    draw_graphs_for_order(city_population_graph_order, ox, oy);

    inner_panel_draw_ui(ox + 48, oy + 305, 34, 7, ui_origin_x, ui_origin_y);
    image_draw(image_group(IMAGE_GROUP_BULLET), ox + 56, oy + 310);
    image_draw(image_group(IMAGE_GROUP_BULLET), ox + 56, oy + 344);
    image_draw(image_group(IMAGE_GROUP_BULLET), ox + 56, oy + 362);
    image_draw(image_group(IMAGE_GROUP_BULLET), ox + 56, oy + 380);
    image_draw(image_group(IMAGE_GROUP_BULLET), ox + 56, oy + 398);
    draw_migration_panel(ox, oy);
}

void button_population_graph(void)
{
    int32_t order = city_population_graph_order;
    int32_t button = button_parameter;
    int32_t next = -1;
    switch (order) {
        case 0:
            next = button == 0 ? 2 : button == 1 ? 5 : -1;
            break;
        case 1:
            next = button == 0 ? 4 : button == 1 ? 3 : -1;
            break;
        case 2:
            next = button == 0 ? 0 : button == 1 ? 4 : -1;
            break;
        case 3:
            next = button == 0 ? 5 : button == 1 ? 1 : -1;
            break;
        case 4:
            next = button == 0 ? 1 : button == 1 ? 2 : -1;
            break;
        case 5:
            next = button == 0 ? 3 : button == 1 ? 0 : -1;
            break;
    }
    if (next >= 0) {
        city_population_graph_order = next;
    }
    population_last_focus = -1;
    window_redraw_request = 1;
}

void window_advisor_population_draw_foreground(void)
{
    if (focus_button_id == population_last_focus) {
        return;
    }
    if (focus_button_id == 0 || focus_button_id == 1 || focus_button_id == 2) {
        population_last_focus = focus_button_id;
        int32_t focus = focus_button_id;
        button_border_draw_ui(advisor_x + 501, advisor_y + 60, 106, 57, focus == 1, ui_origin_x, ui_origin_y);
        button_border_draw_ui(advisor_x + 501, advisor_y + 160, 106, 57, focus == 2, ui_origin_x, ui_origin_y);
    }
}

void population_graph_scale(int32_t max_value)
{
    static const int32_t limits[] = { 100, 200, 400, 800, 1600, 3200, 6400, 12800, 25600 };
    for (int32_t i = 0; i < 9; i++) {
        if (max_value <= limits[i]) {
            graph_y_max = limits[i];
            graph_y_shift = i - 1;
            return;
        }
    }
    graph_y_max = 51200;
    graph_y_shift = 8;
}
