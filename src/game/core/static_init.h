// The functions of core/static_init.c: the constructors of the global objects, their start-up initialization
// (game_static_init), the sound manager's exit shutdown and the key text table EVENTMSG.TXT is read into.

#ifndef CORE_STATIC_INIT_H
#define CORE_STATIC_INIT_H

#include "core/key_text_types.h"
#include "core/queue.h"
#include "core/text_key.h"
#include "empire/empire_route.h"
#include "scenario/campaign.h"
#include "scenario/scenario_event_types.h"
#include "sound/music_types.h"
#include "sound/sound_lib_types.h"
#include "window/window_state_types.h"

#include <stdint.h>

void scenario_event_clear_amounts(void *e);

void file_list_construct(file_list *self);
void scenario_event_set_defaults(void *e);
void file_list_destroy(file_list *self);
void campaign_construct(campaign *c);

void sound_lib_object_construct(sound_name_list *list);

int32_t farm_static_init_max_progress(void);
void fade_static_init(void);
int32_t vegetation_growth_state_static_init(void);
void event_phrases_static_init(void);
void empire_route_construct(empire_route *route);
void key_text_table_load(key_text_table *self, const char *filename, const text_key *keys);
void scenario_event_construct(scenario_event *e);
void scenario_events_construct(scenario_events_t *list);
void dyn_queue_construct(dyn_queue *q);
void dyn_queue_construct_capacity(dyn_queue *q, int32_t capacity);
void ring_queue_construct(ring_queue *q);
void sound_lib_manager_init(sound_lib_state *mgr);
void sound_lib_manager_destroy(sound_lib_state *self);
void music_object_construct(music_state *music);
void sound_manager_shutdown(void);
void game_static_init(void);

#endif
