// The per-tick figure dispatcher of figure/action.c, its timing profile and the empty action.

#ifndef FIGURE_ACTION_H
#define FIGURE_ACTION_H

#include <stdint.h>

void figure_action_none(void);
void figure_action_handle(void);
void figure_action_profile_begin(int32_t type);
void figure_action_profile_end(void);

#endif
