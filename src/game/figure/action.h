// The per-tick figure dispatcher of figure/action.c, its timing profile and the empty action.

#ifndef FIGURE_ACTION_H
#define FIGURE_ACTION_H

#include "core/core_data.h"
#include "figure/figure_profile_types.h"

#include <stdint.h>

void figure_action_none(void);
void figure_action_handle(void);
void figure_action_profile_begin(int32_t type);
void figure_action_profile_end(void);

extern figure_profile_clock figure_profile_frequency;
extern figure_profile_table figure_action_profile;
extern int32_t figure_count_processed;
extern uint8_t figure_profile_qpc_available;

#endif
