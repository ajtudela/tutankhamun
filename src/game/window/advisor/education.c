// Education advisor background.

#include "window/advisor/education.h"
#include "building/building_type.h"
#include "city/culture.h"
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

static int32_t education_advice_text(void)
{
    int32_t demand = city_player_current()->education_demand;
    if (demand == 1) {
        return (city_player_current()->houses_requiring_school != 0) + 19;
    }
    if (demand == 2) {
        return (city_player_current()->houses_requiring_library != 0) + 21;
    }
    if (demand == 3) {
        return 23;
    }
    if (city_player_current()->houses_requiring_school == 0) {
        return 24;
    }
    if (city_player_current()->houses_requiring_library == 0) {
        return (culture_coverage[CULTURE_COVERAGE_SCHOOL] < 100) + 25;
    }
    if (culture_coverage[CULTURE_COVERAGE_SCHOOL] <= culture_coverage[CULTURE_COVERAGE_LIBRARY]) {
        return 26;
    }
    return ((culture_coverage[CULTURE_COVERAGE_LIBRARY] > culture_coverage[CULTURE_COVERAGE_SCHOOL]) ? 0 : 2) + 25;
}

static void draw_coverage_cell(int32_t coverage, int32_t y)
{
    int32_t text_id;
    if (coverage == 0) {
        text_id = 7;
    } else if (coverage < 100) {
        text_id = coverage / 10 + 8;
    } else {
        text_id = 18;
    }
    lang_text_draw_centered_ui(57, text_id, advisor_x + 420, y, 200, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
}

void window_advisor_education_draw_background(void)
{
    int32_t anim = 5 - label_animation[LABEL_SLOT_ADVISOR_OPEN];
    int32_t schools;
    int32_t libraries;

    advisor_x = ui_origin_x;
    advisor_y = ui_origin_y;
    advisor_height_blocks = 16;
    if (anim >= 5) {
        advisor_height_blocks = 16;
        advisor_y = ui_origin_y;
        advisor_x = ui_origin_x;
        return;
    }
    if (anim > 0) {
        graphics_draw_rect(anim * 16 + ui_origin_x, ui_origin_y + anim * 8, (40 - anim * 2) * 16, (16 - anim * 2) * 16, 0);
        return;
    }

    outer_panel_draw(ui_origin_x, ui_origin_y, 40, 16);
    image_draw(image_group(IMAGE_GROUP_ADVISOR_ICONS) + 7, advisor_x + 10, advisor_y + 10);
    lang_text_draw_ui(57, 0, advisor_x + 60, advisor_y + 12, image_group(IMAGE_GROUP_FONT_LARGE), 0, ui_origin_x, ui_origin_y);

    g_text_width = 0;
    lang_text_draw_ui(57, 1, advisor_x + 180, advisor_y + 86, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_ui(57, 2, advisor_x + 290, advisor_y + 86, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_ui(57, 3, advisor_x + 478, advisor_y + 86, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    inner_panel_draw_ui(advisor_x + 32, advisor_y + 100, 36, 3, ui_origin_x, ui_origin_y);

    schools = building_count_active[BUILDING_SCRIBAL_SCHOOL];
    lang_text_draw_ui(57, 28, advisor_x + 40, advisor_y + 105, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
    text_draw_number_centered(schools, NUMBER_PREFIX_NONE, postfix_none, advisor_x + 150, advisor_y + 105, 100, image_group(IMAGE_GROUP_FONT_WHITE), 0);
    g_text_width = 0;
    text_draw_number_ui(schools * 300, NUMBER_PREFIX_NONE, postfix_none, advisor_x + 280, advisor_y + 105, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_ui(57, 4, g_text_width + 280 + advisor_x, advisor_y + 105, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
    draw_coverage_cell(culture_coverage[CULTURE_COVERAGE_SCHOOL], advisor_y + 105);

    libraries = building_count_active[BUILDING_LIBRARY];
    lang_text_draw_ui(57, 29, advisor_x + 40, advisor_y + 125, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
    text_draw_number_centered(libraries, NUMBER_PREFIX_NONE, postfix_none, advisor_x + 150, advisor_y + 125, 100, image_group(IMAGE_GROUP_FONT_WHITE), 0);
    g_text_width = 0;
    text_draw_number_ui(libraries * 800, NUMBER_PREFIX_NONE, postfix_none, advisor_x + 280, advisor_y + 125, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_ui(57, 6, g_text_width + 280 + advisor_x, advisor_y + 125, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
    draw_coverage_cell(culture_coverage[CULTURE_COVERAGE_LIBRARY], advisor_y + 125);

    lang_text_draw_multiline_ui(57, education_advice_text(), advisor_x + 60, advisor_y + 180, 512, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
}
