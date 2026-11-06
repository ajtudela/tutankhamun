// The methods of the music object (music.c) and the speech plays.

#ifndef SOUND_MUSIC_H
#define SOUND_MUSIC_H

#include "sound/music_types.h"

#include <stdint.h>

void sound_music_set_range(music_state *self, int32_t first, int32_t last);
int32_t sound_music_add_track(
    music_state *self, const char *path, int32_t mission, int32_t duration, int32_t pop_min, int32_t pop_max, int32_t is_ambient);
void sound_music_select_by_population(music_state *self, int32_t force);
void sound_music_load_config(music_state *self);
void sound_music_update(music_state *self);
int32_t sound_speech_play_walker(const char *filename);
int32_t sound_speech_play_phrase(int32_t type, int32_t phrase);
int32_t sound_speech_play_mission(const char *filename);

#endif
