// The records of the music object (music_object): per mission its tracks (path, duration, population range) and ambient
// tracks, then the playback state (playlist range, next track, state, wait start).

#ifndef SOUND_MUSIC_TYPES_H
#define SOUND_MUSIC_TYPES_H

#include <stdint.h>

enum music_limits {
    MUSIC_PATH_SIZE = 100,
    MUSIC_MAX_TRACKS = 50,
    MUSIC_MAX_AMBIENT = 10,
    MUSIC_MISSIONS = 53,
    MUSIC_LAST_MISSION_ACCEPTED = 53,
};

enum music_state_kind {
    MUSIC_STATE_PLAY_TRACK = 0,
    MUSIC_STATE_PLAY_AMBIENT = 1,
    MUSIC_STATE_START_TIMER = 2,
    MUSIC_STATE_WAIT_TRACK_END = 3,
};

typedef struct {
    char path[MUSIC_PATH_SIZE];
    int32_t duration;
    int32_t population_min;
    int32_t population_max;
} music_track;

typedef struct {
    music_track tracks[MUSIC_MAX_TRACKS];
    music_track ambient[MUSIC_MAX_AMBIENT];
    int32_t ambient_after_track[MUSIC_MAX_TRACKS];
    int32_t track_count;
    int32_t ambient_count;
} music_mission;

typedef struct {
    music_mission missions[MUSIC_MISSIONS];
    int32_t reserved_1;
    int32_t range_first;
    int32_t range_last;
    int32_t previous_range_first;
    int32_t next_track;
    int32_t state;
    int32_t wait_start_tick;
    int32_t track_finished;
    int32_t invasion_music_request;
} music_state;

#endif
