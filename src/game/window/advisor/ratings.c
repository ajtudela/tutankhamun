// The ratings advisor: background, foreground, the rating columns and the rating buttons (their draw and the chosen
// rating, whose explanation the advisor shows).

#include "window/advisor/ratings.h"
#include "city/city_internal.h"
#include "city/kingdom.h"
#include "game/game_internal.h"
#include "graphics/graphics_primitives.h"
#include "graphics/image_blit.h"
#include "graphics/image_collection.h"
#include "graphics/image_draw.h"
#include "graphics/label_animation_slots.h"
#include "graphics/panel_draw.h"
#include "graphics/text_draw.h"
#include "graphics/ui_layout.h"
#include "platform/log.h"
#include "window/advisor/labor.h"
#include "window/advisors.h"
#include "window/advisors_internal.h"
#include "window/window_image_group.h"
#include <stdint.h>

const char ratings_number_format[4] = "$";

static int32_t clamp_rating(int32_t value)
{
    if (value > 100) {
        return 100;
    }
    if (value < 0) {
        return 0;
    }
    return value;
}

void window_advisor_ratings_draw_column(int32_t x, int32_t y, int32_t value, int32_t has_reached)
{
    int32_t base = image_group(IMAGE_GROUP_RATINGS_COLUMN);
    int32_t stem_y = y - image_record_height(image_known_id(base));
    image_draw(base, x, stem_y);
    int32_t stem_id = base + 1;
    int32_t segments = value / 2 + value;
    for (int32_t i = 0; i < segments; i++) {
        stem_y--;
        image_draw(stem_id, x + 15, stem_y);
    }
    if (has_reached != 0) {
        int32_t top = base + 2;
        image_draw(top, x - 2, stem_y - image_record_height(image_known_id(top)));
    }
}

void window_advisor_ratings_draw_foreground(void)
{
    int32_t remain = 5 - label_animation[LABEL_SLOT_ADVISOR_OPEN];
    if (remain >= 5 || remain > 0) {
        return;
    }
    for (int32_t rating = 1, x = 120; x <= 480; rating++, x += 120) {
        button_border_draw_ui(x - 40 + advisor_x, advisor_y + 286, 120, 66, focus_button_id == rating, ui_origin_x, ui_origin_y);
    }
}

void window_advisor_ratings_draw_background(void)
{
    int32_t remain = 5 - label_animation[LABEL_SLOT_ADVISOR_OPEN];
    int32_t ox;
    int32_t oy;
    int32_t culture;
    int32_t prosperity;
    int32_t monument;
    int32_t kingdom;
    int32_t has_reached;
    int32_t title_id;
    int32_t explain_a;
    int32_t explain_main;
    int32_t centered;
    int32_t text_y;

    advisor_x = ui_origin_x;
    advisor_y = ui_origin_y;
    advisor_height_blocks = 27;
    if (remain >= 5) {
        return;
    }
    if (remain > 0) {
        graphics_draw_rect(remain * 16 + ui_origin_x, ui_origin_y + remain * 8, (remain * -2 + 40) * 16, (remain * -2 + 27) * 16, 0);
        return;
    }

    outer_panel_draw(ui_origin_x, ui_origin_y, 40, 27);
    image_draw(image_group(IMAGE_GROUP_ADVISOR_ICONS) + 3, advisor_x + 10, advisor_y + 10);

    g_text_width = 0;
    lang_text_draw_ui(53, 0, advisor_x + 60, advisor_y + 12, image_group(IMAGE_GROUP_FONT_LARGE), 0, ui_origin_x, ui_origin_y);
    g_text_width = 0;
    lang_text_draw_ui(53, 6, advisor_x + 280, advisor_y + 17, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    text_draw_number_ui(
        city_player_current()->population, NUMBER_PREFIX_NONE, postfix_none, g_text_width + 280 + advisor_x, advisor_y + 17, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);

    if (scenario_data.win_population_enabled != 0 && scenario_data.open_play == 0) {
        g_text_width = 0;
        text_draw_number_ui(scenario_data.win_population, NUMBER_PREFIX_NONE, postfix_none, advisor_x + 280, advisor_y + 34, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
        lang_text_draw_ui(53, 5, g_text_width + 280 + advisor_x, advisor_y + 34, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    }
    if (scenario_data.win_houses != 0 && scenario_data.open_play == 0) {
        g_text_width = 0;
        lang_text_draw_ui(29, scenario_data.win_houses_level + 40, advisor_x + 430, advisor_y + 17, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
        text_draw_number_ui(
            city_houses_of_win_level, NUMBER_PREFIX_NONE, postfix_none, g_text_width + 430 + advisor_x, advisor_y + 17, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
        g_text_width = 0;
        text_draw_number_ui(scenario_data.win_houses, NUMBER_PREFIX_NONE, postfix_none, advisor_x + 430, advisor_y + 34, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
        lang_text_draw_ui(53, 5, g_text_width + 430 + advisor_x, advisor_y + 34, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    }

    image_draw(image_group(IMAGE_GROUP_RATINGS_BACKGROUND), advisor_x + 60, advisor_y + 48);

    ox = advisor_x;
    oy = advisor_y;

    culture = city_player_current()->rating_culture;
    button_border_draw_ui(ox + 80, oy + 286, 120, 66, focus_button_id == 1, ui_origin_x, ui_origin_y);
    lang_text_draw_centered_ui(53, 1, ox + 80, oy + 294, 120, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    text_draw_number_centered(culture, NUMBER_PREFIX_NONE, ratings_number_format, ox + 80, oy + 309, 110, image_group(IMAGE_GROUP_FONT_LARGE), 0);
    g_text_width = 0;
    if (!scenario_data.win_culture_enabled) {
        text_draw_number_ui(0, NUMBER_PREFIX_NONE, empty_text, ox + 85, oy + 334, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    } else {
        text_draw_number_ui(scenario_data.win_culture, NUMBER_PREFIX_NONE, empty_text, ox + 85, oy + 334, image_collection_group(IMAGE_GROUP_FONT_BLACK), 0,
            ui_origin_x, ui_origin_y);
    }
    lang_text_draw_ui(
        53, 5, g_text_width + 5 + ox + 80, oy + 334, image_collection_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    if (!scenario_data.win_culture_enabled || culture >= scenario_data.win_culture) {
        has_reached = 1;
    } else {
        has_reached = 0;
    }
    window_advisor_ratings_draw_column(ox + 110, oy + 274, clamp_rating(culture), has_reached);

    prosperity = city_player_current()->rating_prosperity;
    button_border_draw_ui(ox + 200, oy + 286, 120, 66, focus_button_id == 2, ui_origin_x, ui_origin_y);
    lang_text_draw_centered_ui(53, 2, ox + 200, oy + 294, 120, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    text_draw_number_centered(prosperity, NUMBER_PREFIX_NONE, ratings_number_format, ox + 200, oy + 309, 110, image_collection_group(IMAGE_GROUP_FONT_LARGE), 0);
    g_text_width = 0;
    if (!scenario_data.win_prosperity_enabled) {
        text_draw_number_ui(
            0, NUMBER_PREFIX_NONE, empty_text, ox + 205, oy + 334, image_collection_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    } else {
        text_draw_number_ui(scenario_data.win_prosperity, NUMBER_PREFIX_NONE, empty_text, ox + 205, oy + 334, image_collection_group(IMAGE_GROUP_FONT_BLACK), 0,
            ui_origin_x, ui_origin_y);
    }
    lang_text_draw_ui(53, 5, ox + 205 + g_text_width, oy + 334, image_collection_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    if (!scenario_data.win_prosperity_enabled || prosperity >= scenario_data.win_prosperity) {
        has_reached = 1;
    } else {
        has_reached = 0;
    }
    window_advisor_ratings_draw_column(ox + 230, oy + 274, clamp_rating(prosperity), has_reached);

    monument = city_player_current()->rating_monument;
    button_border_draw_ui(ox + 320, oy + 286, 120, 66, focus_button_id == 3, ui_origin_x, ui_origin_y);
    lang_text_draw_centered_ui(
        53, 3, ox + 320, oy + 294, 120, image_collection_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    text_draw_number_centered(monument, NUMBER_PREFIX_NONE, ratings_number_format, ox + 320, oy + 309, 110, image_collection_group(IMAGE_GROUP_FONT_LARGE), 0);
    g_text_width = 0;
    if (!scenario_data.win_monument_enabled) {
        text_draw_number_ui(
            0, NUMBER_PREFIX_NONE, empty_text, ox + 325, oy + 334, image_collection_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    } else {
        text_draw_number_ui(scenario_data.win_monument, NUMBER_PREFIX_NONE, empty_text, ox + 325, oy + 334, image_collection_group(IMAGE_GROUP_FONT_BLACK), 0,
            ui_origin_x, ui_origin_y);
    }
    lang_text_draw_ui(
        53, 5, ox + 325 + g_text_width, oy + 334, image_collection_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    if (!scenario_data.win_monument_enabled || monument >= scenario_data.win_monument) {
        has_reached = 1;
    } else {
        has_reached = 0;
    }
    if (city_victory_goals_reached() == 0) {
        has_reached = 0;
    }
    window_advisor_ratings_draw_column(ox + 350, oy + 274, clamp_rating(monument), has_reached);

    kingdom = city_player_current()->rating_kingdom;
    window_advisor_ratings_draw_button(4);
    lang_text_draw_centered_ui(
        53, 4, ox + 440, oy + 294, 120, image_collection_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    text_draw_number_centered(kingdom, NUMBER_PREFIX_NONE, ratings_number_format, ox + 440, oy + 309, 110, image_collection_group(IMAGE_GROUP_FONT_LARGE), 0);
    g_text_width = 0;
    if (!scenario_data.win_kingdom_enabled) {
        text_draw_number_ui(
            0, NUMBER_PREFIX_NONE, empty_text, ox + 445, oy + 334, image_collection_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    } else {
        text_draw_number_ui(scenario_data.win_kingdom, NUMBER_PREFIX_NONE, empty_text, ox + 445, oy + 334, image_collection_group(IMAGE_GROUP_FONT_BLACK), 0,
            ui_origin_x, ui_origin_y);
    }
    lang_text_draw_ui(
        53, 5, ox + 445 + g_text_width, oy + 334, image_collection_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    if (!scenario_data.win_kingdom_enabled || kingdom >= scenario_data.win_kingdom) {
        has_reached = 1;
    } else {
        has_reached = 0;
    }
    window_advisor_ratings_draw_column(ox + 470, oy + 274, clamp_rating(kingdom), has_reached);

    inner_panel_draw_ui(ox + 64, oy + 356, 32, 4, ui_origin_x, ui_origin_y);

    text_y = oy + 374;
    title_id = -1;
    explain_a = -1;
    explain_main = -1;
    centered = 0;
    switch (city_player_current()->selected_rating) {
        case 0:
            text_y = oy + 380;
            explain_main = 8;
            centered = 1;
            break;
        case 1:
            title_id = 1;
            if (city_player_current()->rating_culture <= 90) {
                explain_main = city_player_current()->culture_explanation + 9;
            } else {
                explain_main = 65;
            }
            break;
        case 2:
            title_id = 2;
            if (city_player_current()->rating_prosperity <= 90) {
                explain_a = city_player_current()->prosperity_trend + 20;
                if (city_player_current()->prosperity_explanation != 0) {
                    explain_main = city_player_current()->prosperity_explanation + 20;
                }
            } else {
                explain_main = 66;
            }
            break;
        case 3:
            title_id = 3;
            if (city_player_current()->rating_monument <= 90) {
                explain_main = city_player_current()->monument_explanation + 55;
            } else {
                explain_main = 67;
            }
            break;
        case 4:
            title_id = 4;
            if (city_player_current()->rating_kingdom <= 90) {
                explain_main = city_player_current()->kingdom_explanation + 32;
            } else {
                explain_main = 68;
            }
            break;
        default:
            break;
    }

    g_text_width = 0;
    if (title_id >= 0) {
        lang_text_draw_ui(53, title_id, ox + 72, oy + 359, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
    }
    if (explain_a >= 0) {
        lang_text_draw_ui(
            53, explain_a, g_text_width + 72 + ox, oy + 359, image_collection_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
    }
    g_text_width = 0;
    if (explain_main >= 0) {
        if (centered) {
            lang_text_draw_centered_ui(
                53, explain_main, ox + 72, text_y, 496, image_collection_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
            return;
        }
        lang_text_draw_multiline_ui(
            53, explain_main, ox + 72, text_y, 496, image_collection_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
    }
}

void button_select_rating(void)
{
    window_redraw_request = 1;
    city_player_current()->selected_rating = button_parameter;
}

void window_advisor_ratings_draw_button(int32_t index)
{
    button_border_draw_ui(advisor_x + index * 120 - 40, advisor_y + 286, 120, 66, focus_button_id == index, ui_origin_x, ui_origin_y);
}
