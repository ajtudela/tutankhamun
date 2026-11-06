// The functions of empire_file.c: loading the empire file, the empire editor's switch and save, and the route paths
// (freeing, validity check and reset, drawing).

#ifndef IO_EMPIRE_FILE_H
#define IO_EMPIRE_FILE_H

#include "empire/empire_route.h"

#include <stdint.h>

void empire_file_load(int32_t cleopatra);
void empire_file_select(int32_t index);
void empire_file_save(void);
void empire_route_free_paths(void);
int32_t empire_route_is_valid(const empire_route *route);
void empire_route_reset(empire_route *route);
void empire_route_prepare_path(empire_route *route);
void empire_route_redraw_path(empire_route *route);
char empire_route_find_objects(empire_route *route);
int32_t empire_route_draw_segment(empire_route *route, int32_t from, int32_t to, int32_t at);

#endif
