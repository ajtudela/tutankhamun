// The map options panel (editor_map_options.c): its functions, buttons and layout.

#ifndef EDITOR_EDITOR_MAP_OPTIONS_H
#define EDITOR_EDITOR_MAP_OPTIONS_H

#include <stdint.h>

enum editor_options_button {
    OPTIONS_BUTTON_STARTING_CONDITIONS = 1,
    OPTIONS_BUTTON_CLIMATE = 2,
    OPTIONS_BUTTON_EVENTS = 3,
    OPTIONS_BUTTON_ENEMY = 4,
    OPTIONS_BUTTON_FIRST_WINDOW = 5,
    OPTIONS_BUTTON_SOLDIER_IMAGES = 9,
};

enum editor_options_layout {
    OPTIONS_PANEL_FRAME_TOP = 28,
    OPTIONS_PANEL_WIDTH_BLOCKS = 30,
    OPTIONS_PANEL_HEIGHT_BLOCKS = 25,
    OPTIONS_PANEL_MARGIN = 12,
    OPTIONS_PANEL_TOP = 40,
    OPTIONS_PANEL_BUTTON_COLUMN = 200,
    OPTIONS_BUTTON_WIDTH = 250,
    OPTIONS_BUTTON_HEIGHT = 30,
    OPTIONS_ROW_SPACING = 40,
    OPTIONS_WINDOW_ROWS = 4,
};

enum scenario_image_set {
    SCENARIO_IMAGE_LAST_PHARAOH = 18,
    SCENARIO_IMAGE_FIRST_CLEOPATRA = 19,
};

void editor_options_panel_draw(void);
void editor_menu_exit_builder(void);

#endif
