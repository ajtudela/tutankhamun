// The route methods, path rebuild and route buttons of empire_editor_routes.c.

#ifndef EDITOR_EMPIRE_EDITOR_ROUTES_H
#define EDITOR_EMPIRE_EDITOR_ROUTES_H

#include "empire/empire_route.h"
#include "window/input_dialogs.h"
#include <stdbool.h>
#include <stdint.h>

int32_t empire_route_last_point(empire_route *self);
bool empire_route_clear(empire_route *self);
void empire_route_set_in_use(empire_route *self, int32_t value);
int32_t empire_route_start_object(empire_route *self);
int32_t empire_route_end_object(empire_route *self);
int32_t empire_route_point_at(empire_route *self, int32_t x, int32_t y, uint8_t *on_point);
bool empire_route_delete_point(empire_route *self, int32_t index);
bool empire_route_rebuild(empire_route *route);
int32_t empire_route_first_free(void);

void numeric_input_show_int8(int32_t x, int32_t y, uint8_t *target, input_dialog_callback on_ok);
void route_number_changed(void);
void route_step_changed(void);
void numeric_input_show_int32(int32_t x, int32_t y, int32_t *target, input_dialog_callback on_ok);
void route_step_button(void);
void route_number_button(void);
void route_kind_button(void);
void route_delete_button(void);

#endif
