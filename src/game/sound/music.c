// The music: music.txt read into the music object, the playlist of the population's range, and the state machine on
// stream 0 that plays tracks and ambient tracks in turn; and the speech on stream 1 (a walker's phrase or a mission's
// speech).
//
// A track is accepted for one mission past the mission records, which lies on the playback state.

#include "sound/music.h"

#include "city/city_player.h"
#include "core/crt_compat.h"
#include "platform/platform_time.h"
#include "sound/sound_data.h"
#include "sound/sound_lib.h"
#include "window/window_data.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char speech_mission_path[300];

int32_t sound_music_track_duration_ms;

enum music_text {
    MUSIC_LINE_SIZE = 200,
    MUSIC_FILE_PATH_SIZE = 100,
    MUSIC_DEFAULT_POPULATION_MAX = 100,
    MUSIC_NUMBER_BASE = 10,
    MILLISECONDS_PER_SECOND = 1000,
    SPEECH_PATH_SIZE = 300,
};

enum speech_phrase_table {
    PHRASE_GROUP_ZOOKEEPER = 108,
    PHRASES_PER_GROUP = 20,
    PHRASE_NAME_SIZE = 24,
    PHRASE_BUFFER_SIZE = 32,
};

static const char music_delimiters[] = " ,\t\n";

void sound_music_set_range(music_state *self, int32_t first, int32_t last)
{
    self->range_first = first;
    self->range_last = last;
}

int32_t sound_music_add_track(
    music_state *self, const char *path, int32_t mission, int32_t duration, int32_t pop_min, int32_t pop_max, int32_t is_ambient)
{
    if (mission > MUSIC_LAST_MISSION_ACCEPTED) {
        return -1;
    }
    music_mission *m = &self->missions[mission];
    if (is_ambient) {
        int32_t n = m->ambient_count;
        if (n == MUSIC_MAX_AMBIENT) {
            return -1;
        }
        music_track *track = &m->ambient[n];
        strcpy(track->path, path);
        track->duration = duration;
        track->population_min = 0;
        track->population_max = 0;
        m->ambient_count = n + 1;
        return 1;
    }
    int32_t n = m->track_count;
    if (n == MUSIC_MAX_TRACKS) {
        return -1;
    }
    music_track *track = &m->tracks[n];
    strcpy(track->path, path);
    track->duration = duration;
    track->population_min = pop_min;
    track->population_max = pop_max;
    m->track_count = n + 1;
    return 1;
}

static int32_t first_track_holding(const music_mission *m, int32_t population)
{
    int32_t first = 0;
    while (first < m->track_count) {
        const music_track *track = &m->tracks[first];
        if ((track->population_min <= population) && (population <= track->population_max)) {
            break;
        }
        first++;
    }
    return first;
}

static int32_t end_of_tracks_starting_below(const music_mission *m, int32_t first, int32_t population)
{
    int32_t end = first;
    while (end < m->track_count) {
        if (population < m->tracks[end].population_min) {
            break;
        }
        end++;
    }
    return end;
}

void sound_music_select_by_population(music_state *self, int32_t force)
{
    int32_t population = city_player_current()->population;
    const music_mission *m = &self->missions[settings_data.mission_id];
    const music_track *current = &m->tracks[self->next_track];
    if ((current->population_max < population) || (population < current->population_min) || force) {
        int32_t first = first_track_holding(m, population);
        int32_t end = end_of_tracks_starting_below(m, first, population);
        sound_music_set_range(self, first, end - 1);
    }
    int32_t range_first = self->range_first;
    if ((self->next_track < range_first) || (self->range_last < self->next_track)) {
        self->next_track = range_first;
    }
    if (range_first != self->previous_range_first) {
        self->previous_range_first = range_first;
        if (self->track_finished == 0) {
            self->state = MUSIC_STATE_PLAY_AMBIENT;
        }
    }
}

static void read_number(int32_t *value)
{
    char *token = strtok(NULL, music_delimiters);
    if (token) {
        *value = strtol(token, NULL, MUSIC_NUMBER_BASE);
    }
}

void sound_music_load_config(music_state *self)
{
    FILE *fp = fopen("music.txt", "rt");
    if (!fp) {
        return;
    }
    char line[MUSIC_LINE_SIZE];
    char path_buf[MUSIC_FILE_PATH_SIZE];
    int32_t duration = 0;
    int32_t pop_min = 1;
    int32_t pop_max = MUSIC_DEFAULT_POPULATION_MAX;
    int32_t mission = 0;
    while (fgets(line, MUSIC_LINE_SIZE, fp)) {
        if ((line[0] == ';') || (line[0] == '\0')) {
            continue;
        }
        char *tok = strtok(line, music_delimiters);
        if (!tok) {
            continue;
        }
        if (strcmp(tok, "AMBIENT") == 0) {
            music_mission *m = &self->missions[mission];
            m->ambient_after_track[m->track_count] = 1;
            continue;
        }
        char *type_tok = strtok(NULL, music_delimiters);
        int32_t is_ambient = (type_tok && (strcmp(type_tok, "A") == 0)) ? 1 : 0;
        read_number(&mission);
        read_number(&duration);
        read_number(&pop_min);
        read_number(&pop_max);
        sprintf(path_buf, "audio\\music\\%s", tok);
        sound_music_add_track(&music_object, path_buf, mission, duration, pop_min, pop_max, is_ambient);
    }
    fclose(fp);
}

static void play_next_track(music_state *self, const music_mission *m, int32_t mission)
{
    if (m->track_count <= 0) {
        return;
    }
    int32_t index = self->next_track;
    const music_track *track = &m->tracks[index];
    sound_lib_play_stream(&sound_manager, track->path, SOUND_STREAM_MUSIC, 1);
    sound_music_track_duration_ms = track->duration * MILLISECONDS_PER_SECOND;
    int32_t next = index + 1;
    self->track_finished = 0;
    self->next_track = next;
    int32_t ambient_after = self->missions[mission].ambient_after_track[next];
    self->state = (ambient_after != 1) ? MUSIC_STATE_START_TIMER : MUSIC_STATE_PLAY_AMBIENT;
}

static void play_random_ambient(music_state *self, const music_mission *m)
{
    int32_t ambient_count = m->ambient_count;
    if (ambient_count > 0) {
        int32_t pick = (crt_rand() * ambient_count) / CRT_RAND_RANGE;
        sound_lib_play_stream(&sound_manager, m->ambient[pick].path, SOUND_STREAM_MUSIC, 1);
    }
    self->state = MUSIC_STATE_START_TIMER;
}

static void wait_track_end(music_state *self)
{
    if ((uint32_t) (self->wait_start_tick + sound_music_track_duration_ms) < platform_tick_count()) {
        if (self->range_last < self->next_track) {
            self->next_track = self->range_first;
        }
        self->state = MUSIC_STATE_PLAY_TRACK;
        self->track_finished = 1;
    }
}

void sound_music_update(music_state *self)
{
    if (!sound_digital_ok || !settings_data.sound_music_enabled) {
        return;
    }
    if (self->state == MUSIC_STATE_PLAY_TRACK) {
        sound_music_select_by_population(self, 0);
    }
    sound_lib_set_stream0_volume(&sound_manager, settings_data.sound_music_volume);
    int32_t mission = settings_data.mission_id;
    const music_mission *m = &self->missions[mission];
    switch (self->state) {
        case MUSIC_STATE_PLAY_TRACK:
            play_next_track(self, m, mission);
            return;
        case MUSIC_STATE_PLAY_AMBIENT:
            play_random_ambient(self, m);
            return;
        case MUSIC_STATE_START_TIMER:
            self->state = MUSIC_STATE_WAIT_TRACK_END;
            self->wait_start_tick = (int32_t) platform_tick_count();
            return;
        case MUSIC_STATE_WAIT_TRACK_END:
            wait_track_end(self);
            break;
    }
}

static int32_t play_speech_stream(const char *path)
{
    int32_t volume = settings_data.sound_speech_volume;
    sound_lib_set_stream_volume(&sound_manager, SOUND_STREAM_SPEECH, volume);
    sound_lib_set_stream1_volume(&sound_manager, volume);
    return sound_lib_play_stream(&sound_manager, path, SOUND_STREAM_SPEECH, 1);
}

int32_t sound_speech_play_walker(const char *filename)
{
    if (!sound_digital_ok || !settings_data.sound_speech_enabled) {
        return 0;
    }
    strcpy(speech_walker_path, "audio\\voice\\walker\\");
    strcat(speech_walker_path, filename);
    return play_speech_stream(speech_walker_path);
}

int32_t sound_speech_play_phrase(int32_t type, int32_t phrase)
{
    if ((type <= 0) || (phrase < 0)) {
        return 0;
    }
    char name[PHRASE_BUFFER_SIZE];
    memset(name, 0, sizeof(name));
    const char *src;
    if (type == PHRASE_GROUP_ZOOKEEPER) {
        src = phrase_names_zookeeper[phrase];
    } else {
        src = phrase_names_walkers + ((phrase - PHRASES_PER_GROUP + (type * PHRASES_PER_GROUP)) * PHRASE_NAME_SIZE);
    }
    strcpy(name, src);
    return sound_speech_play_walker(name);
}

int32_t sound_speech_play_mission(const char *filename)
{
    if (!sound_digital_ok || !settings_data.sound_speech_enabled) {
        return 0;
    }
    char local[SPEECH_PATH_SIZE];
    strcpy(local, "audio\\voice\\mission\\");
    strcat(local, filename);
    strcpy(speech_mission_path, local);
    FILE *fp = fopen(local, "rb");
    if (!fp) {
        return -1;
    }
    fclose(fp);
    return play_speech_stream(speech_mission_path);
}
