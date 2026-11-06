// The scenario editor's map options panel over the map preview (scenario name, climate, soldiers, enemy and the
// other options) and File > Exit builder.

#include "editor/editor_map_options.h"
#include "editor/editor_internal.h"
#include "editor/editor_text.h"
#include "graphics/button_border.h"
#include "graphics/graphics_data.h"
#include "graphics/image_collection.h"
#include "graphics/image_draw.h"
#include "graphics/panel_draw.h"
#include "graphics/text_draw.h"
#include "text/lang.h"
#include "window/text_input.h"
#include "window/window_id.h"
#include "window/window_image_group.h"
#include <stdint.h>

static const int16_t options_window_labels[OPTIONS_WINDOW_ROWS] = {
    EDITOR_TEXT_GODS_SETTINGS,
    EDITOR_TEXT_BUILDINGS_ALLOWED,
    EDITOR_TEXT_WIN_CRITERIA,
    EDITOR_TEXT_FLOOD_PLAIN_SETTINGS,
};

void editor_options_panel_draw(void)
{
    int32_t x = ui_origin_x;
    int32_t y = ui_origin_y;
    int32_t left = x + OPTIONS_PANEL_MARGIN;
    int32_t top = y + OPTIONS_PANEL_TOP;
    int32_t col = left + OPTIONS_PANEL_BUTTON_COLUMN;
    outer_panel_draw(x, y + OPTIONS_PANEL_FRAME_TOP, OPTIONS_PANEL_WIDTH_BLOCKS, OPTIONS_PANEL_HEIGHT_BLOCKS);
    text_cursor_reset();
    inner_panel_draw(left + 97, top, 18, 2);
    text_draw_centered((const uint8_t *) scenario_data.name, left + 100, top + 10, 280, image_group(IMAGE_GROUP_FONT_WHITE), 0);
    graphics_draw_text_cursor(g_cursor_x_offset + left + 100, top + 10, 0xFFFF);
    button_border_draw(col, top + 36, OPTIONS_BUTTON_WIDTH, OPTIONS_BUTTON_HEIGHT, focus_button_id == OPTIONS_BUTTON_STARTING_CONDITIONS);
    lang_text_draw_centered(LANG_GROUP_EDITOR, EDITOR_TEXT_STARTING_CONDITIONS, col, top + 45, OPTIONS_BUTTON_WIDTH, image_group(IMAGE_GROUP_FONT_BLACK), 0);
    lang_text_draw(LANG_GROUP_EDITOR, EDITOR_TEXT_TERRAIN_SET, left + 20, top + 85, image_group(IMAGE_GROUP_FONT_BLACK), 0);
    button_border_draw(col, top + 76, OPTIONS_BUTTON_WIDTH, OPTIONS_BUTTON_HEIGHT, focus_button_id == OPTIONS_BUTTON_CLIMATE);
    lang_text_draw_centered(LANG_GROUP_EDITOR, scenario_data.climate_type + EDITOR_TEXT_HUMID_CLIMATE, col, top + 85, OPTIONS_BUTTON_WIDTH, image_group(IMAGE_GROUP_FONT_BLACK), 0);
    button_border_draw(col, top + 116, OPTIONS_BUTTON_WIDTH, OPTIONS_BUTTON_HEIGHT, focus_button_id == OPTIONS_BUTTON_EVENTS);
    lang_text_draw_centered(LANG_GROUP_EDITOR, EDITOR_TEXT_EVENTS, col, top + 125, OPTIONS_BUTTON_WIDTH, image_group(IMAGE_GROUP_FONT_BLACK), 0);
    button_border_draw(left + 5, top + 156, 175, OPTIONS_BUTTON_HEIGHT, focus_button_id == OPTIONS_BUTTON_SOLDIER_IMAGES);
    int32_t soldiers_label = (scenario_data.soldier_images_alt == 1) ? EDITOR_TEXT_PLAYER_FIGHTS_EGYPT_AS : EDITOR_TEXT_ENEMY_IS;
    lang_text_draw_centered(LANG_GROUP_EDITOR, soldiers_label, left + 7, top + 165, 173, image_group(IMAGE_GROUP_FONT_BLACK), 0);
    button_border_draw(col, top + 156, OPTIONS_BUTTON_WIDTH, OPTIONS_BUTTON_HEIGHT, focus_button_id == OPTIONS_BUTTON_ENEMY);
    lang_text_draw_centered(LANG_GROUP_ENEMY_NAMES, scenario_data.enemy_type, col, top + 165, OPTIONS_BUTTON_WIDTH, image_group(IMAGE_GROUP_FONT_BLACK), 0);
    for (int32_t i = 0; i < OPTIONS_WINDOW_ROWS; i++) {
        int32_t row_y = top + 196 + (i * OPTIONS_ROW_SPACING);
        button_border_draw(col, row_y, OPTIONS_BUTTON_WIDTH, OPTIONS_BUTTON_HEIGHT, focus_button_id == (OPTIONS_BUTTON_FIRST_WINDOW + i));
        lang_text_draw_centered(LANG_GROUP_EDITOR, options_window_labels[i], col, row_y + 9, OPTIONS_BUTTON_WIDTH, image_group(IMAGE_GROUP_FONT_BLACK), 0);
    }
    button_border_draw(left + 6, top + 198, 184, 144, 0);
    int16_t image = scenario_data.image;
    if (image <= SCENARIO_IMAGE_LAST_PHARAOH) {
        image_draw(image + image_group(IMAGE_GROUP_MISSION_BACKGROUNDS), left + 8, top + 200);
    } else {
        image_draw(image + image_group(IMAGE_GROUP_CLEOPATRA_BACKGROUNDS) - SCENARIO_IMAGE_FIRST_CLEOPATRA, left + 8, top + 200);
    }
}

void editor_menu_exit_builder(void)
{
    window_current_id = WINDOW_FRONT_MENU;
    window_redraw_request = 1;
}
