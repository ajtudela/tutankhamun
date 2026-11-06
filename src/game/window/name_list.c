// The name list window: the names of the player's family to choose from in two columns ("Male Names", "Female
// Names") and an OK button, centred on the screen; a click copies the name into the player name being typed.

#include "window/name_list.h"
#include "graphics/button.h"
#include "graphics/button_border.h"
#include "graphics/image_collection.h"
#include "graphics/panel_draw.h"
#include "text/lang.h"
#include "widget/screen_layout.h"
#include "window/input_dialogs.h"
#include "window/options_windows.h"
#include "window/text_input.h"
#include "window/window_id.h"
#include "window/window_image_group.h"
#include <stdint.h>
#include <string.h>

enum name_list_layout {
    NAME_LIST_WIDTH_BLOCKS = 26,
    NAME_LIST_HEIGHT_BLOCKS = 28,
    NAME_LIST_TEXT_DX = 115,
    NAME_LIST_TEXT_DY = 30,
    NAME_LIST_UI_X = 110,
    NAME_LIST_UI_Y = 30,
};

static const generic_button *name_list_button(int32_t index)
{
    return &name_list_buttons.buttons[index];
}

void window_name_list_draw(void)
{
    int32_t x, y;
    if (!screen_layout_is_standard_size()) {
        x = ui_origin_x + NAME_LIST_UI_X;
        y = ui_origin_y + NAME_LIST_UI_Y;
    } else if (settings_data.display_mode == DISPLAY_MODE_640X480) {
        x = 110;
        y = 30;
    } else if (settings_data.display_mode == DISPLAY_MODE_800X600) {
        x = 190;
        y = 90;
    } else {
        x = 302;
        y = 174;
    }
    outer_panel_draw(x, y, NAME_LIST_WIDTH_BLOCKS, NAME_LIST_HEIGHT_BLOCKS);
    for (int32_t i = 0; i < NAME_LIST_ENTRIES; i++) {
        int32_t font;
        if ((i == PLAYER_NAMES_MALE_HEADING) || (i == PLAYER_NAMES_FEMALE_HEADING)) {
            font = image_group(IMAGE_GROUP_FONT_BLACK);
        } else if ((i + 1) == focus_button_id) {
            font = image_group(IMAGE_GROUP_COLOR_NORMAL_BLACK);
        } else if (i == name_list_choice) {
            font = image_group(IMAGE_GROUP_FONT_RED);
        } else {
            font = image_group(IMAGE_GROUP_FONT_BLACK);
        }
        lang_text_draw_rect_ui(LANG_GROUP_PLAYER_NAMES, i, &name_list_button(i)->x, font, 0, NAME_LIST_TEXT_DX, NAME_LIST_TEXT_DY, ui_origin_x, ui_origin_y);
    }
    const int16_t *ok = &name_list_button(NAME_LIST_ENTRIES)->x;
    button_border_draw_rect(ok, focus_button_id == NAME_LIST_OK_BUTTON, 110, 30);
    lang_text_draw_centered_in_rect(LANG_GROUP_YES_NO, LANG_OK, name_list_button(NAME_LIST_ENTRIES), image_group(IMAGE_GROUP_FONT_BLACK), 0, 114, 34, ui_origin_x, ui_origin_y);
}

void name_list_choose(void)
{
    strcpy(settings_data.player_name, lang_get_string(LANG_GROUP_PLAYER_NAMES, button_parameter));
    keyboard_capture_set(KEYBOARD_FIELD_PLAYER_NAME);
    keyboard_home();
    keyboard_end();
    name_list_choice = button_parameter;
}

void window_go_name_list(void)
{
    window_current_id = WINDOW_NAME_LIST;
    window_redraw_request = 1;
}
