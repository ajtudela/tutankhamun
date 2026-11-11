// The File menu and the file dialogs' buttons: load, save and delete over the player's saved games (".sav" added,
// "File does not exist" for a missing file, a yes / no question before overwriting), cancel, scrolling, picking a row
// and exit; and the chief advisor's rows, which open the advisor's comment.

#include "window/file_dialog_buttons.h"
#include "building/building_type.h"
#include "city/city_player.h"
#include "core/calc.h"
#include "core/text.h"
#include "game/player_record.h"
#include "graphics/button.h"
#include "input/input_data.h"
#include "io/file.h"
#include "io/file_write.h"
#include "io/game_file.h"
#include "platform/platform_dialog.h"
#include "platform/platform_file.h"
#include "platform/platform_input.h"
#include "platform/platform_message.h"
#include "scenario/campaign.h"
#include "sound/sound_data.h"
#include "sound/sound_lib.h"
#include "text/lang.h"
#include "window/file_dialog.h"
#include "window/file_list.h"
#include "window/main_menu.h"
#include "window/message_box.h"
#include "window/mission_flow.h"
#include "window/player_selection.h"
#include "window/popup_dialog.h"
#include "window/text_input.h"
#include "window/window_data.h"
#include "window/window_id.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

enum file_dialog_rules {
    FILE_DIALOG_PATH_SIZE = 100,
    FILE_DIALOG_NAME_CLEARED = 64,
    FILE_DIALOG_ERROR_FRAMES = 100,
    FILE_DIALOG_EXTENSION_LENGTH = 3,
};

enum file_dialog_scrollbar_area {
    FILE_SCROLLBAR_LEFT = 464,
    FILE_SCROLLBAR_RIGHT = 496,
    FILE_SCROLLBAR_TOP = 145,
    FILE_SCROLLBAR_BOTTOM = 300,
    FILE_SCROLLBAR_RANGE = 130,
};

enum file_dialog_button_parameter {
    FILE_DIALOG_BUTTON_CANCEL = 0,
    FILE_DIALOG_BUTTON_OK = 1,
};

enum chief_advisor_row {
    CHIEF_ADVISOR_ROW_SENTIMENT = 0,
    CHIEF_ADVISOR_ROW_MIGRATION = 1,
};

enum chief_advisor_rules {
    CHIEF_ADVISOR_MAX_TROUBLE = 3,
    CHIEF_ADVISOR_SENTIMENT_TOP = 100,
};

static void file_dialog_open(int32_t type)
{
    dir_list_files(0);
    keyboard_accepted = 0;
    window_current_id = WINDOW_FILE_DIALOG;
    file_dialog_return_window.word = WINDOW_CITY;
    file_dialog_type = (uint8_t) type;
    memset(file_dialog_name, 0, FILE_DIALOG_NAME_CLEARED);
    strcpy(file_dialog_name, file_dialog_last_name);
    keyboard_capture_set(KEYBOARD_FIELD_FILE_NAME);
    keyboard_home();
    keyboard_end();
    window_redraw_request = 1;
}

void file_menu_load_game(void)
{
    construction_building_type = BUILDING_NONE;
    file_dialog_open(FILE_DIALOG_LOAD);
}

void file_menu_save_game(void)
{
    file_dialog_open(FILE_DIALOG_SAVE);
}

void file_menu_delete_game(void)
{
    file_dialog_open(FILE_DIALOG_DELETE);
}

// The game's texts have no message for a save that cannot be written.
static const char save_failed_title[] = "Cannot save the game";
static const char save_failed_text[] = "The saved game could not be written to the family's folder.";

static int32_t file_dialog_confirm_overwrite(const char *path)
{
    FILE *file = platform_fopen(path, file_mode_read);
    if (!file) {
        return 1;
    }
    fclose(file);
    const char *title = lang_get_string(LANG_GROUP_POPUP, POPUP_FILE_EXISTS);
    const char *text = lang_get_string(LANG_GROUP_POPUP, POPUP_OVERWRITE_EXISTING_FILE);
    return window_message_box_show(title, text, PLATFORM_BOX_YES_NO) != PLATFORM_ANSWER_NO;
}

static int32_t file_dialog_save(const char *path)
{
    if (!file_dialog_confirm_overwrite(path)) {
        file_remove_extension(file_dialog_name);
        return 0;
    }
    // The family's folder may be missing (a player record without its folder): create it rather than fail.
    platform_dir_create(dir_save_path());
    if (game_file_io_write_saved_game(path) != 0) {
        window_message_box_show(save_failed_title, save_failed_text, PLATFORM_BOX_OK);
        file_remove_extension(file_dialog_name);
        return 0;
    }
    if (game_is_replay == GAME_PLAY_CAMPAIGN) {
        strcpy(campaign_object.campaign.saved_game_path, path);
        player_data_write_file(&campaign_object, player_record_path());
    }
    return 1;
}

void file_dialog_button(void)
{
    char path[FILE_DIALOG_PATH_SIZE];
    focus_button_id_secondary = 0;
    focus_button_id = 0;
    image_button_focus_id = 0;
    focus_button_id_tertiary = 0;
    if (button_parameter == FILE_DIALOG_BUTTON_CANCEL) {
        window_current_id = file_dialog_return_window.id;
        window_redraw_request = 1;
        return;
    }
    if (button_parameter != FILE_DIALOG_BUTTON_OK) {
        return;
    }
    file_remove_extension(file_dialog_name);
    memcpy(file_extension, save_file_extension, FILE_DIALOG_EXTENSION_LENGTH);
    file_append_extension(file_dialog_name);
    strcpy(path, dir_save_path());
    strcat(path, path_separator);
    strcat(path, file_dialog_name);
    if ((file_dialog_type != FILE_DIALOG_SAVE) && !file_exists(path)) {
        file_remove_extension(file_dialog_name);
        file_dialog_error_frames = FILE_DIALOG_ERROR_FRAMES;
        return;
    }
    if (file_dialog_type == FILE_DIALOG_LOAD) {
        sound_lib_stop_stream(&sound_manager, SOUND_STREAM_MUSIC);
        game_file_load_saved_game(path);
    } else if (file_dialog_type == FILE_DIALOG_DELETE) {
        game_file_io_delete_saved_game(path);
    } else if (!file_dialog_save(path)) {
        return;
    }
    file_remove_extension(file_dialog_name);
    strcpy(file_dialog_last_name, file_dialog_name);
    if (file_dialog_type != FILE_DIALOG_DELETE) {
        window_current_id = WINDOW_CITY;
    }
    window_redraw_request = 1;
}

void file_dialog_button_scroll(void)
{
    if (file_dialog_scroll_total <= FILE_DIALOG_ROWS) {
        return;
    }
    if (button_parameter == SCROLL_ARROW_UP) {
        if (file_dialog_scroll > 0) {
            file_dialog_error_frames = 0;
            file_dialog_scroll--;
        }
        return;
    }
    if (button_parameter == SCROLL_ARROW_DOWN) {
        if ((file_dialog_scroll + FILE_DIALOG_ROWS) >= file_dialog_scroll_total) {
            return;
        }
        file_dialog_scroll++;
    }
    file_dialog_error_frames = 0;
}

void file_menu_exit(void)
{
    platform_message_post(platform_main_window, PLATFORM_MSG_KEY_DOWN, PLATFORM_KEY_ESCAPE, 0);
}

void chief_advisor_button_row(void)
{
    const city_player *player = city_player_current();
    if (button_parameter == CHIEF_ADVISOR_ROW_MIGRATION) {
        if (player->migration_direction == 0) {
            if ((player->kingdom_soldiers + player->enemies) > CHIEF_ADVISOR_MAX_TROUBLE) {
                return;
            }
            window_current_id = WINDOW_ADVISOR_COMMENT;
            window_redraw_request = 1;
            return;
        }
    } else if ((button_parameter == CHIEF_ADVISOR_ROW_SENTIMENT) && (player->sentiment == CHIEF_ADVISOR_SENTIMENT_TOP)) {
        return;
    }
    window_current_id = WINDOW_ADVISOR_COMMENT;
    window_redraw_request = 1;
}

int32_t file_dialog_handle_scrollbar(void)
{
    if (file_dialog_scroll_total <= FILE_DIALOG_ROWS) {
        return 0;
    }
    if (mouse_left_down == 0) {
        return 0;
    }
    int32_t range = file_dialog_scroll_total - FILE_DIALOG_ROWS;
    if ((mouse_x < (ui_origin_x + FILE_SCROLLBAR_LEFT)) || (mouse_x > (ui_origin_x + FILE_SCROLLBAR_RIGHT))) {
        return 0;
    }
    if ((mouse_y < (ui_origin_y + FILE_SCROLLBAR_TOP)) || (mouse_y > (ui_origin_y + FILE_SCROLLBAR_BOTTOM))) {
        return 0;
    }
    int32_t offset = mouse_y - ui_origin_y - FILE_SCROLLBAR_TOP;
    if (offset > FILE_SCROLLBAR_RANGE) {
        offset = FILE_SCROLLBAR_RANGE;
    }
    file_dialog_scroll = calc_adjust_with_percentage(range, calc_percentage(offset, FILE_SCROLLBAR_RANGE));
    window_redraw_request = 1;
    return 1;
}

void file_dialog_button_row(void)
{
    if (button_parameter >= file_dialog_scroll_total) {
        return;
    }
    file_dialog_selected = button_parameter;
    const char *name = file_list_name(file_dialog_selected + file_dialog_scroll);
    if (window_current_id == WINDOW_FILE_DIALOG) {
        memset(file_dialog_name, 0, FILE_DIALOG_NAME_CLEARED);
        strcpy(file_dialog_name, name);
        keyboard_capture_set(KEYBOARD_FIELD_FILE_NAME);
    } else {
        memset(scenario_map_name, 0, FILE_DIALOG_NAME_CLEARED);
        strcpy(scenario_map_name, name);
        keyboard_capture_set(KEYBOARD_FIELD_MAP_NAME);
    }
    keyboard_home();
    keyboard_end();
    file_dialog_error_frames = 0;
}
