// The records of window state of window/window_data.h: the window a dialog returns to, the file list, the map
// selection's list, the popup dialog's state followed by the top menu's drop-down state, and what the building info
// window found about its building (road access, staffing, canal and groundwater, the type under rubble) and the tile it
// shows.

#ifndef WINDOW_WINDOW_STATE_TYPES_H
#define WINDOW_WINDOW_STATE_TYPES_H

#include "graphics/menu.h"
#include <stdint.h>

enum map_list_size {
    MAP_LIST_MAX_MISSIONS = 100,
};

typedef union {
    uint8_t id;
    int32_t word;
} window_return_slot;

typedef struct {
    int32_t y;
    int32_t x;
    int32_t grid_offset;
} building_info_tile;

typedef struct {
    uint32_t reserved;
    int32_t *offsets;
    int32_t count;
    int32_t offsets_capacity;
    char *names;
    int32_t names_capacity;
    int32_t names_used;
} file_list;

typedef struct {
    file_list files;
    int32_t missions[MAP_LIST_MAX_MISSIONS];
    int32_t selection_kind;
    int32_t campaign;
    int32_t total;
    int32_t scroll;
    int32_t selected;
} map_list;

typedef struct popup_state {
    int32_t reserved_1;
    void (*callback)(void);
    int32_t button_count;
    int32_t return_window;
    int32_t text;
    int32_t y;
    int32_t x;
    int32_t menu_dropdown_keep_open;
    menu_item *menu_dropdown_item;
} popup_state;

typedef struct building_info_details {
    int32_t has_road_access;
    int32_t worker_percentage;
    int32_t canal;
    int32_t has_groundwater;
    int32_t rubble_building_type;
} building_info_details;

#endif
