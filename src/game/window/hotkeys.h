// The hotkeys of the city window and their helpers (hotkeys.c).
//
// The bookmark index is checked against 0..4, so index 4 addresses the word past the four bookmarks.

#ifndef WINDOW_HOTKEYS_H
#define WINDOW_HOTKEYS_H

#include <stdint.h>

enum map_bookmark_count {
    MAP_BOOKMARKS = 4,
    MAP_BOOKMARK_LAST_INDEX = 4,
    MAP_BOOKMARK_NONE = -1,
};

typedef struct {
    int32_t grid_offset;
    int32_t unused;
} map_bookmark;

typedef map_bookmark map_bookmark_table[MAP_BOOKMARKS];

void hotkey_show_advisor(int32_t advisor);
void hotkey_toggle_overlay(int32_t overlay);
void hotkey_set_bookmark(int32_t index);
void map_bookmarks_reset(void);
void hotkey_go_to_bookmark(int32_t index);
void hotkey_cycle_legion(void);
void hotkey_change_game_speed(int32_t down);
int32_t mouse_inside_rect(int32_t x, int32_t y, int32_t width, int32_t height);
int32_t building_type_to_god(int32_t type);

#endif
