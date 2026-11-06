// The scenario editor's empire object handlers (empire_editor_input.c).

#ifndef WINDOW_EMPIRE_EDITOR_INPUT_H
#define WINDOW_EMPIRE_EDITOR_INPUT_H

#include <stdint.h>

void editor_warning_draw(int32_t text);
void editor_scenario_image_arrow(void);
void empire_editor_image_arrow(void);
void empire_editor_place_object(int32_t object, int32_t keep);
void editor_map_terrain_reload(void);
void empire_editor_nudge_object(int32_t direction, int32_t amount);
int32_t empire_editor_object_at_mouse(void);

#endif
