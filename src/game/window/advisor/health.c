// Health advisor background: the coverage of dentists, physicians and mortuaries and the advice of the health level.
//
// The courthouse demand advice reads "Physicians" (group 56, texts 7 and 8), though its check is magistrate access.

#include "window/advisor/health.h"
#include "building/building_type.h"
#include "city/culture.h"
#include "core/calc.h"
#include "game/game_internal.h"
#include "graphics/graphics_data.h"
#include "graphics/graphics_primitives.h"
#include "graphics/image_collection.h"
#include "graphics/image_draw.h"
#include "graphics/label_animation_slots.h"
#include "graphics/panel_draw.h"
#include "graphics/text_draw.h"
#include "graphics/ui_layout.h"
#include "platform/log.h"
#include "window/advisor/education.h"
#include "window/advisor/entertainment.h"
#include "window/advisors.h"
#include "window/window_image_group.h"
#include <stdint.h>

static int32_t health_advice_text(void)
{
    switch (city_player_current()->health_demand) {
        case 1:
            return (city_player_current()->houses_requiring_courthouse != 0) + 7;
        case 2:
            return (city_player_current()->houses_requiring_dentist != 0) + 9;
        case 3:
            return (city_player_current()->houses_requiring_health != 0) + 11;
        case 4:
            return (city_player_current()->houses_requiring_more_courthouse != 0) + 13;
        default:
            return 15;
    }
}

void window_advisor_health_draw_background(void)
{
    int32_t anim = 5 - label_animation[LABEL_SLOT_ADVISOR_OPEN];
    int32_t row;
    int32_t plague;
    int32_t saved_height;

    advisor_x = ui_origin_x;
    advisor_y = ui_origin_y;
    advisor_height_blocks = 22;
    if (anim >= 5) {
        return;
    }
    if (anim > 0) {
        graphics_draw_rect(anim * 16 + ui_origin_x, ui_origin_y + anim * 8, (40 - anim * 2) * 16, (22 - anim * 2) * 16, 0);
        return;
    }

    outer_panel_draw(ui_origin_x, ui_origin_y, 40, 22);
    image_draw(image_group(IMAGE_GROUP_ADVISOR_ICONS) + 6, advisor_x + 10, advisor_y + 10);
    lang_text_draw_ui(56, 0, advisor_x + 60, advisor_y + 12, image_group(IMAGE_GROUP_FONT_LARGE), 0, ui_origin_x, ui_origin_y);

    {
        int32_t status_text;
        if (city_player_current()->population < 200) {
            status_text = 16;
        } else {
            status_text = city_player_current()->health / 10 + 17;
        }
        lang_text_draw_multiline_ui(56, status_text, advisor_x + 60, advisor_y + 46, 512, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    }

    lang_text_draw_ui(56, 3, advisor_x + 180, advisor_y + 94, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_ui(56, 4, advisor_x + 290, advisor_y + 94, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_ui(56, 5, advisor_x + 478, advisor_y + 94, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    inner_panel_draw_ui(advisor_x + 32, advisor_y + 108, 36, 4, ui_origin_x, ui_origin_y);

    for (row = 0; row < 3; row++) {
        int32_t building_type;
        int32_t name_text;
        int32_t coverage_index;
        int32_t y;
        int32_t coverage;
        int32_t people_x;
        int32_t people_y;

        if (row == 0) {
            building_type = BUILDING_PHYSICIAN;
            name_text = 39;
            coverage_index = CULTURE_COVERAGE_PHYSICIAN;
        } else if (row == 1) {
            building_type = BUILDING_DENTIST;
            name_text = 40;
            coverage_index = CULTURE_COVERAGE_DENTIST;
        } else {
            building_type = BUILDING_MORTUARY;
            name_text = 42;
            coverage_index = CULTURE_COVERAGE_MORTUARY;
        }
        y = row * 20 + 112 + advisor_y;

        lang_text_draw_ui(56, name_text, advisor_x + 40, y, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
        text_draw_number_centered(building_count_active[building_type], NUMBER_PREFIX_NONE, postfix_none, advisor_x + 150, y, 100, image_group(IMAGE_GROUP_FONT_WHITE), 0);

        g_text_width = 0;
        coverage = culture_coverage[coverage_index];
        if (coverage == 0 || city_player_current()->population == 0) {
            text_draw_number_ui(0, NUMBER_PREFIX_NONE, postfix_none, advisor_x + 280, y, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
            people_y = y;
            people_x = g_text_width + 280 + advisor_x;
        } else {
            int32_t people = calc_adjust_with_percentage(city_player_current()->population, coverage);
            text_draw_number_ui(people, NUMBER_PREFIX_NONE, postfix_none, advisor_x + 280, y, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
            people_y = y;
            people_x = g_text_width + 280 + advisor_x;
        }
        lang_text_draw_ui(56, 6, people_x, people_y, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);

        coverage = culture_coverage[coverage_index];
        if (coverage == 0) {
            lang_text_draw_centered_ui(56, 2, advisor_x + 420, y, 200, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
        } else if (coverage < 100) {
            lang_text_draw_centered_ui(56, coverage / 10 + 43, advisor_x + 420, y, 200, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
        } else {
            lang_text_draw_centered_ui(56, 53, advisor_x + 420, y, 200, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
        }
    }

    g_text_width = 0;
    g_draw_y_accum = 0;
    text_draw_number_centered(building_count_active[BUILDING_APOTHECARY], NUMBER_PREFIX_NONE, postfix_none, advisor_x + 60, advisor_y + 194, 5, image_group(IMAGE_GROUP_FONT_BLACK), 0);
    lang_text_draw_multiline_ui(56, 59, g_text_width + 57 + advisor_x, advisor_y + 194, 512, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);

    saved_height = g_draw_y_accum;
    g_text_width = 0;
    plague = city_player_current()->plague_houses;
    {
        int32_t plague_text;
        if (plague == 0) {
            plague_text = 54;
        } else if (plague < 3) {
            plague_text = 55;
        } else if (plague < 6) {
            plague_text = 56;
        } else {
            plague_text = (plague > 8) + 57;
        }
        lang_text_draw_multiline_ui(56, plague_text, advisor_x + 60, advisor_y + 194 + g_draw_y_accum, 512, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    }
    g_draw_y_accum = saved_height;
    lang_text_draw_multiline_ui(56, 60, g_text_width + 57 + advisor_x, advisor_y + 194 + saved_height, 512, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);

    g_text_width = 0;
    text_draw_number_centered(city_player_current()->food_types_available, NUMBER_PREFIX_NONE, postfix_none, advisor_x + 60, advisor_y + 250, 5, image_group(IMAGE_GROUP_FONT_BLACK), 0);
    lang_text_draw_multiline_ui(56, 61, g_text_width + 57 + advisor_x, advisor_y + 250, 512, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);

    lang_text_draw_multiline_ui(56, health_advice_text(), advisor_x + 60, advisor_y + 275, 512, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
}

