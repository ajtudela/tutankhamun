// Entertainment advisor background.

#include "window/advisor/entertainment.h"
#include "building/building_type.h"
#include "city/culture.h"
#include "city/gods.h"
#include "game/game_internal.h"
#include "graphics/graphics_primitives.h"
#include "graphics/image_collection.h"
#include "graphics/image_draw.h"
#include "graphics/label_animation_slots.h"
#include "graphics/panel_draw.h"
#include "graphics/text_draw.h"
#include "graphics/ui_layout.h"
#include "window/window_image_group.h"
#include <stdint.h>

static int32_t entertainment_advice_text(void)
{
    int32_t most_needed;
    if (city_player_current()->houses_missing_more_entertainment < city_player_current()->houses_missing_entertainment) {
        return 10;
    }
    if (city_player_current()->houses_missing_more_entertainment == 0) {
        return (city_player_current()->average_entertainment != 0) + 7;
    }
    most_needed = city_player_current()->venue_needing_shows;
    if (most_needed == 0) {
        return 9;
    }
    if (most_needed == 1) {
        return 11;
    }
    if (most_needed == 2) {
        return 12;
    }
    return (most_needed != 3) + 13;
}

void window_advisor_entertainment_draw_background(void)
{
    int32_t anim;
    int32_t row;

    city_gods_calculate_moods(0);
    city_culture_calculate();

    anim = 5 - label_animation[LABEL_SLOT_ADVISOR_OPEN];
    advisor_x = ui_origin_x;
    advisor_y = ui_origin_y;
    advisor_height_blocks = 19;
    if (anim >= 5) {
        return;
    }
    if (anim > 0) {
        graphics_draw_rect(anim * 16 + ui_origin_x, ui_origin_y + anim * 8, (40 - anim * 2) * 16, (19 - anim * 2) * 16, 0);
        return;
    }

    outer_panel_draw(ui_origin_x, ui_origin_y, 40, 19);
    image_draw(image_group(IMAGE_GROUP_ADVISOR_ICONS) + 8, advisor_x + 10, advisor_y + 10);
    lang_text_draw_ui(58, 0, advisor_x + 60, advisor_y + 12, image_group(IMAGE_GROUP_FONT_LARGE), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_ui(58, 1, advisor_x + 180, advisor_y + 42, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_ui(58, 55, advisor_x + 180, advisor_y + 62, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_ui(58, 2, advisor_x + 260, advisor_y + 62, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_ui(58, 3, advisor_x + 340, advisor_y + 62, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_ui(58, 4, advisor_x + 480, advisor_y + 62, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    inner_panel_draw_ui(advisor_x + 32, advisor_y + 76, 36, 8, ui_origin_x, ui_origin_y);

    for (row = 0; row < 5; row++) {
        int32_t building_type;
        int32_t name_text;
        int32_t coverage_index;
        int32_t shows;
        int32_t people_per;
        int32_t y;
        int32_t coverage;
        int32_t cov_text;

        if (row == 0) {
            building_type = BUILDING_BOOTH;
            name_text = 47;
            coverage_index = 0;
            shows = city_player_current()->juggler_shows;
            people_per = 400;
        } else if (row == 1) {
            building_type = BUILDING_BANDSTAND;
            name_text = 48;
            coverage_index = 1;
            shows = city_player_current()->musician_shows;
            people_per = 700;
        } else if (row == 2) {
            building_type = BUILDING_PAVILION;
            name_text = 49;
            coverage_index = 2;
            shows = city_player_current()->dancer_shows;
            people_per = 1200;
        } else if (row == 3) {
            building_type = BUILDING_SENET_HOUSE;
            name_text = 50;
            coverage_index = 3;
            shows = city_player_current()->senet_shows;
            people_per = 5000;
        } else {
            building_type = BUILDING_ZOO;
            name_text = 56;
            coverage_index = 4;
            shows = city_player_current()->zoo_shows;
            people_per = 7500;
        }
        y = row * 24 + 80 + advisor_y;

        lang_text_draw_ui(58, name_text, advisor_x + 40, y, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
        text_draw_number_centered(building_count_active[building_type], NUMBER_PREFIX_NONE, postfix_none, advisor_x + 150, y, 100, image_group(IMAGE_GROUP_FONT_WHITE), 0);
        text_draw_number_centered(shows, NUMBER_PREFIX_NONE, postfix_none, advisor_x + 230, y, 100, image_group(IMAGE_GROUP_FONT_WHITE), 0);

        g_text_width = 0;
        text_draw_number_ui(
            building_count_active[building_type] * people_per, NUMBER_PREFIX_NONE, postfix_none, advisor_x + 340, y, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
        lang_text_draw_ui(58, 5, g_text_width + 340 + advisor_x, y, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);

        coverage = culture_coverage[coverage_index];
        if (coverage == 0) {
            cov_text = 35;
        } else if (coverage < 100) {
            cov_text = coverage / 10 + 36;
        } else {
            cov_text = 46;
        }
        lang_text_draw_centered_ui(58, cov_text, advisor_x + 470, y, 100, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
    }

    lang_text_draw_multiline_ui(58, entertainment_advice_text(), advisor_x + 60, advisor_y + 220, 512, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
}
