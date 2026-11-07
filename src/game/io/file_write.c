// File writers (a part at an offset, appended, a whole file), the listing of saved games or maps for the file dialogs,
// and settings_save, run at exit (Pharaoh.inf and Pharaohmap.inf).
//
// A failed seek of the part and append writers returns 0 and leaves the file open; the whole-file writer returns 1 even
// if the write fails.

#include "io/file_write.h"

#include "building/building_data.h"
#include "editor/editor_data.h"
#include "game/game_data.h"
#include "graphics/graphics_data.h"
#include "io/io_data.h"
#include "platform/platform_file.h"
#include "platform/status_log.h"
#include "text/lang.h"
#include "text/text_data.h"
#include "window/file_list.h"
#include "window/player_selection.h"
#include "window/window_data.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum file_write_limit {
    SETTINGS_FILE_SIZE = 564,
    PHARAOHMAP_FILE_SIZE = 84,
    SAVE_PATH_SIZE = 260,
};

static int32_t write_at(const char *path, const void *buffer, int32_t size, int32_t offset, int32_t origin)
{
    int32_t fd = platform_file_open(path, PLATFORM_OPEN_WRITE);
    if (fd == -1) {
        return 0;
    }
    if (platform_file_seek(fd, offset, origin) == -1) {
        return 0;
    }
    int32_t written = platform_file_write(fd, buffer, (uint32_t) size);
    platform_file_close(fd);
    return written;
}

int32_t io_write_file_part(const char *path, const void *buffer, int32_t size, int32_t offset)
{
    return write_at(path, buffer, size, offset, SEEK_SET);
}

int32_t io_append_file(const char *path, const void *buffer, int32_t size)
{
    return write_at(path, buffer, size, 0, SEEK_END);
}

int32_t io_write_file(const char *path, const void *buffer, int32_t size)
{
    int32_t fd = platform_file_open(path, PLATFORM_OPEN_REPLACE);
    if (fd == -1) {
        return 0;
    }
    platform_file_write(fd, buffer, (uint32_t) size);
    platform_file_close(fd);
    return 1;
}

void dir_list_files(char maps)
{
    if (maps) {
        file_list_list_dir(&file_list_saves, map_file_pattern, 0);
    } else {
        char path[SAVE_PATH_SIZE];
        strcpy(path, dir_save_path());
        strcat(path, save_file_pattern);
        file_list_list_dir(&file_list_saves, path, 0);
    }
    file_dialog_scroll_total = file_list_saves.count;
    for (int32_t i = 0; i < file_dialog_scroll_total; i++) {
        file_remove_extension(file_list_saves.names + file_list_saves.offsets[i]);
    }
    file_list_sort(&file_list_saves, platform_text_compare_nocase);
    file_dialog_scroll = 0;
    file_dialog_selected = 0;
    file_dialog_error_frames = 0;
    file_dialog_scroll_layout = 0;
}

void settings_save(void)
{
    io_write_file(str_pharaoh_inf, &settings_data, SETTINGS_FILE_SIZE);
    io_write_file(str_pharaohmap_inf, &map_info, PHARAOHMAP_FILE_SIZE);
    log_info(str_removed_manmades, 0, buildings_without_id_cleared);
    if (spare_screen) {
        free(spare_screen);
    }
    if (lang_text_base) {
        free(lang_text_base);
        lang_text_base = 0;
    }
    if (lang_message_data) {
        free(lang_message_data);
        lang_message_data = 0;
    }
}
