// The player (family) selection of the main menu: the list of player records Save\*.dat with scroll arrows and a
// scroll bar, "new family", "delete family", "proceed" and "back", and the name input whose result creates a new
// player and its save directory "save\<name>". Deleting a player removes its record, saved games and directory.

#include "window/player_selection.h"
#include "core/calc.h"
#include "core/static_init.h"
#include "game/game_data.h"
#include "game/game_internal.h"
#include "game/player_record.h"
#include "graphics/button_border.h"
#include "graphics/image_buttons.h"
#include "graphics/image_collection.h"
#include "graphics/image_draw.h"
#include "graphics/label_animation_slots.h"
#include "graphics/panel_draw.h"
#include "graphics/text_draw.h"
#include "input/input_data.h"
#include "io/file.h"
#include "io/io_data.h"
#include "platform/platform_data.h"
#include "platform/platform_file.h"
#include "scenario/campaign_load.h"
#include "window/file_list.h"
#include "window/intermezzo.h"
#include "window/main_menu.h"
#include "window/mission_flow.h"
#include "window/popup_dialog.h"
#include "window/text_input.h"
#include "window/window_data.h"
#include "window/window_id.h"
#include "window/window_image_group.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

char dir_save_path_buffer[FILE_PATH_SIZE];

enum player_selection_text {
    PLAYER_SELECTION_TEXT_GROUP = 292,
    PLAYER_SELECTION_TEXT_NEW = 0,
    PLAYER_SELECTION_TEXT_DELETE = 1,
    PLAYER_SELECTION_TEXT_PROCEED = 2,
    PLAYER_SELECTION_TEXT_TITLE = 3,
    PLAYER_SELECTION_TEXT_BACK = 4,
};

enum player_selection_layout {
    PLAYER_LIST_ROWS = 12,
    PLAYER_TEXT_BUTTONS = 4,
    PLAYER_IMAGE_BUTTONS = 2,
    PLAYER_SCROLL_UP = 0,
    PLAYER_SCROLL_DOWN = 1,
    SCROLLBAR_X = 464,
    SCROLLBAR_X_END = 496,
    SCROLLBAR_Y = 145,
    SCROLLBAR_Y_END = 300,
    SCROLLBAR_HEIGHT = 130,
    SCROLLBAR_DOT_IMAGE = 39,
    PLAYER_NAME_SIZE = 32,
    PATH_SIZE = 260,
    POPUP_OK = 1,
    POPUP_OK_CANCEL = 2,
    POPUP_ANSWER_OK = 1,
};

static const int32_t player_text_button_texts[PLAYER_TEXT_BUTTONS] = {
    PLAYER_SELECTION_TEXT_NEW,
    PLAYER_SELECTION_TEXT_DELETE,
    PLAYER_SELECTION_TEXT_PROCEED,
    PLAYER_SELECTION_TEXT_BACK,
};

static char *plist_entry(int32_t index)
{
    if (index < 0 || index >= player_file_list.count) {
        return 0;
    }
    return player_file_list.offsets[index] + player_file_list.names;
}

static int32_t max_scroll(void)
{
    return player_file_list.count - PLAYER_LIST_ROWS;
}

int32_t player_selection_image_button_count(void)
{
    return PLAYER_IMAGE_BUTTONS;
}

static void draw_scrollbar_dot(void)
{
    int32_t percentage;
    if (plist_scroll < 1) {
        percentage = 0;
    } else if (plist_scroll + PLAYER_LIST_ROWS >= player_file_list.count) {
        percentage = 100;
    } else {
        percentage = calc_percentage(plist_scroll, max_scroll());
    }
    int32_t offset = calc_adjust_with_percentage(SCROLLBAR_HEIGHT, percentage);
    image_draw(image_group(IMAGE_GROUP_PANEL_BUTTON) + SCROLLBAR_DOT_IMAGE, ui_origin_x + 472, ui_origin_y + offset + SCROLLBAR_Y);
}

void window_player_selection_draw_background(void)
{
    if (window_redraw_request == WINDOW_REDRAW_ALL) {
        image_draw(image_group(IMAGE_GROUP_BACKGROUND_PLAYER), ui_origin_x, ui_origin_y);
    }
    window_ui_redraw = 1;
    outer_panel_draw(ui_origin_x + 128, ui_origin_y + 40, 24, 23);
    inner_panel_draw(ui_origin_x + 144, ui_origin_y + 120, 20, 13);
    image_buttons_draw(ui_origin_x, ui_origin_y, player_selection_tables.player_selection_image_buttons, player_selection_image_button_count());
    lang_text_draw_centered(PLAYER_SELECTION_TEXT_GROUP, PLAYER_SELECTION_TEXT_TITLE, ui_origin_x + 128, ui_origin_y + 58, 384, image_group(IMAGE_GROUP_FONT_LARGE), 0);
    window_player_selection_draw_foreground();
    inner_panel_draw(ui_origin_x + 467, ui_origin_y + 146, 2, 10);
    if (max_scroll() > 0) {
        draw_scrollbar_dot();
    }
}

void window_player_selection_draw_foreground(void)
{
    for (int32_t row = 0; row < PLAYER_LIST_ROWS; row++) {
        int32_t index = plist_scroll + row;
        const char *name = plist_entry(index);
        if (name == 0) {
            continue;
        }
        int32_t font = focus_button_id == row + 1 || index == plist_selected ? image_group(IMAGE_GROUP_FONT_WHITE) : image_group(IMAGE_GROUP_FONT_GREEN);
        text_draw_in_rect(name, &player_selection_tables.player_row_buttons[row].x, font, 0, 0, 0);
    }
    int32_t font = image_group(IMAGE_GROUP_FONT_BLACK);
    for (int32_t i = 0; i < PLAYER_TEXT_BUTTONS; i++) {
        const generic_button *button = &player_selection_tables.player_row_buttons[PLAYER_LIST_ROWS + i];
        button_border_draw_rect(&button->x, focus_button_id == PLAYER_LIST_ROWS + 1 + i, 0, 0);
        lang_text_draw_centered_rect(PLAYER_SELECTION_TEXT_GROUP, player_text_button_texts[i], &button->x, font, 0, 4, 7);
    }
}

void player_selection_button_scroll(void)
{
    if (player_file_list.count <= PLAYER_LIST_ROWS) {
        return;
    }
    if (button_parameter == PLAYER_SCROLL_UP) {
        if (plist_scroll > 0) {
            window_redraw_request = WINDOW_REDRAW_FOREGROUND;
            plist_scroll--;
        }
        return;
    }
    if (button_parameter == PLAYER_SCROLL_DOWN) {
        if (plist_scroll + PLAYER_LIST_ROWS >= player_file_list.count) {
            return;
        }
        plist_scroll++;
    }
    window_redraw_request = WINDOW_REDRAW_FOREGROUND;
}

int32_t player_selection_handle_scrollbar(void)
{
    if (player_file_list.count <= PLAYER_LIST_ROWS || mouse_left_down == 0) {
        return 0;
    }
    if (mouse_x < ui_origin_x + SCROLLBAR_X || mouse_x > ui_origin_x + SCROLLBAR_X_END) {
        return 0;
    }
    if (mouse_y < ui_origin_y + SCROLLBAR_Y || mouse_y > ui_origin_y + SCROLLBAR_Y_END) {
        return 0;
    }
    int32_t y = mouse_y - ui_origin_y - SCROLLBAR_Y;
    if (y > SCROLLBAR_HEIGHT) {
        y = SCROLLBAR_HEIGHT;
    }
    plist_scroll = calc_adjust_with_percentage(max_scroll(), calc_percentage(y, SCROLLBAR_HEIGHT));
    window_redraw_request = WINDOW_REDRAW_FOREGROUND;
    return 1;
}

void player_selection_button_proceed(void)
{
    if (settings_data.player_name[0] == 0) {
        window_popup_dialog_show(POPUP_NO_PLAYER, button_none, POPUP_OK);
        return;
    }
    window_main_menu_show();
    window_redraw_request = WINDOW_REDRAW_ALL;
}

const char *dir_save_path(void)
{
    strcpy(dir_save_path_buffer, save_folder_path);
    strcat(dir_save_path_buffer, settings_data.player_name);
    return dir_save_path_buffer;
}

void player_selection_button_select(void)
{
    if (button_parameter >= player_file_list.count) {
        return;
    }
    int32_t index = button_parameter + plist_scroll;
    plist_selected = index;
    const char *name = plist_entry(index);
    if (name == 0) {
        return;
    }
    memset(settings_data.player_name, 0, PLAYER_NAME_SIZE);
    strcpy(settings_data.player_name, name);
    file_remove_extension(settings_data.player_name);
    player_data_read_file(&campaign_object, player_record_path());
    window_redraw_request = WINDOW_REDRAW_ALL;
}

void player_selection_load_files(const char *pattern)
{
    file_list_list_dir(&player_file_list, pattern, 0);
    for (int32_t i = 0; i < player_file_list.count; i++) {
        file_remove_extension(plist_entry(i));
    }
    file_list_sort(&player_file_list, platform_text_compare_nocase);
}

void window_player_selection_show(void)
{
    memset(settings_data.player_name, 0, PLAYER_NAME_SIZE);
    player_selection_load_files(player_file_pattern);
    window_current_id = WINDOW_PLAYER_SELECTION;
    file_dialog_error_frames = 0;
    plist_scroll = 0;
    button_parameter = plist_selected;
    player_selection_button_select();
    window_redraw_request = WINDOW_REDRAW_ALL;
    if (player_file_list.count == 0) {
        player_selection_button_new_player();
    }
}

void player_selection_button_new_player(void)
{
    memset(settings_data.player_name, 0, PLAYER_NAME_SIZE);
    window_current_id = WINDOW_NAME_INPUT;
    keyboard_accepted = 0;
    keyboard_capture_set(KEYBOARD_FIELD_PLAYER_NAME);
    keyboard_home();
    keyboard_end();
    window_redraw_request = WINDOW_REDRAW_ALL;
}

void player_selection_button_delete(void)
{
    window_popup_dialog_show(POPUP_DELETE_PLAYER, player_selection_confirm_delete, POPUP_OK_CANCEL);
}

static bool player_record_exists(void)
{
    FILE *file = platform_fopen(player_record_path(), file_mode_read_binary);
    if (file == 0) {
        return false;
    }
    fclose(file);
    return true;
}

static void select_player_named(const char *name)
{
    for (int32_t i = 0; i < player_file_list.count; i++) {
        if (strcmp(plist_entry(i), name) == 0) {
            plist_selected = i;
        }
    }
}

void player_selection_create_player(void)
{
    char name[PLAYER_NAME_SIZE];
    string_trim(settings_data.player_name);
    if (settings_data.player_name[0] == 0) {
        window_player_selection_show();
        return;
    }
    if (player_record_exists()) {
        window_popup_dialog_show(POPUP_PLAYER_EXISTS, player_selection_button_new_player, POPUP_OK);
        return;
    }
    campaign_load(&campaign_object.campaign, campaign_file_name);
    player_data_write_file(&campaign_object, player_record_path());
    platform_dir_create(dir_save_path());
    strcpy(name, settings_data.player_name);
    window_player_selection_show();
    select_player_named(name);
    strcpy(settings_data.player_name, name);
    window_player_selection_show();
    player_selection_button_proceed();
}

static const char *file_list_entry(const file_list *list, int32_t index)
{
    if (index < 0 || index >= list->count) {
        return 0;
    }
    return list->offsets[index] + list->names;
}

static void delete_player_files(void)
{
    file_list saves;
    char pattern[PATH_SIZE];
    char path[PATH_SIZE];
    platform_file_remove(player_record_path());
    file_list_construct(&saves);
    strcpy(pattern, dir_save_path());
    strcat(pattern, save_file_pattern);
    file_list_list_dir(&saves, pattern, 0);
    for (int32_t i = 0; i < saves.count; i++) {
        strcpy(path, dir_save_path());
        strcat(path, path_separator);
        strcat(path, file_list_entry(&saves, i));
        platform_file_remove(path);
    }
    platform_dir_remove(dir_save_path());
    file_list_destroy(&saves);
}

void player_selection_confirm_delete(void)
{
    if (popup_answer == POPUP_ANSWER_OK) {
        delete_player_files();
    }
    window_player_selection_show();
    window_redraw_request = WINDOW_REDRAW_ALL;
}

void player_selection_draw_background(void)
{
    image_draw(image_group(IMAGE_GROUP_MENU_BACKGROUND), ui_origin_x, ui_origin_y);
    label_animation[LABEL_SLOT_REFRESH_COUNTDOWN] = 100;
}
