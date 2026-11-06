// The build menu (build_menu.c): its draws, input and entry tests, and the groups whose entries need more than their
// enable flags.

#ifndef WINDOW_BUILD_MENU_H
#define WINDOW_BUILD_MENU_H

#include <stdbool.h>
#include <stdint.h>

enum build_menu_group {
    BUILD_MENU_EDITOR_ROCKS = 0,
    BUILD_MENU_RELIGION = 6,
    BUILD_MENU_INDUSTRY = 10,
    BUILD_MENU_EDITOR_BRUSH_SIZE = 14,
    BUILD_MENU_EDITOR_ENTRY_EXIT_POINTS = 15,
    BUILD_MENU_EDITOR_ELEVATION = 16,
    BUILD_MENU_EDITOR_INVASION_POINTS = 17,
    BUILD_MENU_EDITOR_RIVER_POINTS = 18,
    BUILD_MENU_EDITOR_NATIVES = 19,
    BUILD_MENU_FARMS = 20,
    BUILD_MENU_RAW_MATERIALS = 21,
    BUILD_MENU_GUILDS = 22,
    BUILD_MENU_TEMPLES = 23,
    BUILD_MENU_TEMPLE_COMPLEXES = 24,
    BUILD_MENU_FORTS = 25,
    BUILD_MENU_EDITOR_SPECIAL_POINTS = 26,
    BUILD_MENU_EDITOR_WATER = 27,
    BUILD_MENU_MONUMENTS = 28,
    BUILD_MENU_SHRINES = 29,
    BUILD_MENU_MUD_STRUCTURES = 30,
    BUILD_MENU_BRICK_STRUCTURES = 31,
    BUILD_MENU_DEFENSES = 32,
    BUILD_MENU_WATER_CROSSINGS = 34,
    BUILD_MENU_BEAUTIFICATION = 35,
    BUILD_MENU_TEMPLE_COMPLEX_UPGRADES = 36,
    BUILD_MENU_EDITOR_ANIMALS = 37,
};

int32_t build_menu_next_item(int32_t group, int32_t prev);
int32_t build_menu_handle_input(void);
void window_build_menu_draw_background(void);
bool build_flags_is_kind_a(int32_t flags);
bool build_flags_is_kind_b(int32_t flags);
bool build_flags_has_bit1(int32_t flags);

#endif
