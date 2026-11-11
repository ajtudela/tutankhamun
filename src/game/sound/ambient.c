// The ambient MP3 streams on the city stream (shared with the disasters) and the terrain channels of the random
// city samples. The city drawing scores each ambient stream; each frame with the city sound on, the stream with the
// highest score plays and fades up to its volume.
//
// The monument threshold is always above its bound, and the industrial compare plays the same file either way.

#include "sound/ambient.h"

#include "city/city_player.h"
#include "sound/effect.h"
#include "sound/sound_data.h"
#include "sound/sound_lib.h"
#include "sound/sound_play.h"
#include "widget/widget_data.h"
#include "window/window_data.h"

enum ambient_play {
    AMBIENT_SCORE_COUNT = 12,
    AMBIENT_FADE_STEP = 5,
    AMBIENT_FULL_VOLUME = 100,
    AMBIENT_MONUMENT_FULL_THRESHOLD = 1000,
    HOUSING_AMBIENT_POPULATION_1 = 500,
    HOUSING_AMBIENT_POPULATION_2 = 2000,
    HOUSING_AMBIENT_POPULATION_3 = 5000,
    HOUSING_AMBIENT_POPULATION_4 = 8000,
};

enum ambient_bind_mode {
    AMBIENT_BIND_ROAM = 0,
    AMBIENT_BIND_VARIANT = 1,
};

static void ambient_play_mp3(const char *path)
{
    sound_lib_play_stream(&sound_manager, path, SOUND_STREAM_CITY, 0);
    sound_lib_set_stream_volume(&sound_manager, SOUND_STREAM_CITY, 0);
}

static void ambient_stop_stream(void)
{
    sound_lib_stop_stream(&sound_manager, SOUND_STREAM_CITY);
    sound_lib_set_stream_volume(&sound_manager, SOUND_STREAM_CITY, 0);
}

void sound_ambient_clear_scores(void)
{
    for (int32_t i = 0; i < AMBIENT_SCORE_COUNT; i++) {
        ambient_scores[i] = 0;
    }
}

static int32_t stream_setting_volume(int32_t stream, int32_t volume)
{
    if (stream == SOUND_STREAM_MUSIC) {
        return (volume > settings_data.sound_music_volume) ? settings_data.sound_music_volume : volume;
    }
    if (stream == SOUND_STREAM_SPEECH) {
        return (volume > settings_data.sound_speech_volume) ? settings_data.sound_speech_volume : volume;
    }
    if (stream == SOUND_STREAM_CITY) {
        return (volume > settings_data.sound_city_volume) ? settings_data.sound_city_volume : volume;
    }
    return volume;
}

void sound_ambient_fade_up(int32_t channel)
{
    if (!sound_digital_ok || !sound_manager.streams[channel]) {
        return;
    }
    if (!sound_lib_stream_is_playing(&sound_manager, channel)) {
        return;
    }
    if (!sound_stream_fade_step_due(channel)) {
        return;
    }
    int32_t volume = sound_lib_get_stream_volume(&sound_manager, channel) + AMBIENT_FADE_STEP;
    sound_lib_set_stream_volume(&sound_manager, channel, stream_setting_volume(channel, volume));
}

static int32_t loudest_ambient(void)
{
    int32_t best = 0;
    int32_t best_type = -1;
    for (int32_t i = 0; i < AMBIENT_SCORE_COUNT; i++) {
        if (ambient_scores[i] > best) {
            best = ambient_scores[i];
            best_type = i;
        }
    }
    return best_type;
}

static const char *housing_ambient_file(int32_t population)
{
    if (population < HOUSING_AMBIENT_POPULATION_1) {
        return "audio\\ambient\\housing1.mp3";
    }
    if (population < HOUSING_AMBIENT_POPULATION_2) {
        return "audio\\ambient\\housing2.mp3";
    }
    if (population < HOUSING_AMBIENT_POPULATION_3) {
        return "audio\\ambient\\housing3.mp3";
    }
    if (population < HOUSING_AMBIENT_POPULATION_4) {
        return "audio\\ambient\\housing4.mp3";
    }
    return "audio\\ambient\\housing5.mp3";
}

static const char *ambient_file(int32_t type)
{
    switch (type) {
        case AMBIENT_HOUSING:
            return housing_ambient_file(city_player_current()->population);
        case AMBIENT_INDUSTRIAL:
            return "audio\\ambient\\industrial.mp3";
        case AMBIENT_MEADOW:
        case AMBIENT_GRASSLAND:
            return "audio\\ambient\\meadow.mp3";
        case AMBIENT_MONUMENT:
            return (ambient_monument_threshold >= AMBIENT_MONUMENT_FULL_THRESHOLD) ? "audio\\ambient\\monument_f.mp3" : "audio\\ambient\\monument_b.mp3";
        case AMBIENT_DESERT:
            return "audio\\ambient\\desert.mp3";
        case AMBIENT_FLOODPLAIN:
            return "audio\\ambient\\floodplain.mp3";
        case AMBIENT_ROCK:
            return "audio\\ambient\\rock.mp3";
        case AMBIENT_WATER:
            return "audio\\ambient\\water.mp3";
        case AMBIENT_VEGETATION:
            return "audio\\ambient\\vegetation.mp3";
        default:
            return 0;
    }
}

void sound_ambient_update(void)
{
    if (!sound_digital_ok || !settings_data.sound_city_enabled) {
        return;
    }
    int32_t best_type = loudest_ambient();
    if ((best_type == ambient_current_type) && (sound_lib_stream_is_playing(&sound_manager, SOUND_STREAM_CITY) != 0)) {
        if (sound_lib_get_stream_volume(&sound_manager, SOUND_STREAM_CITY) < AMBIENT_FULL_VOLUME) {
            sound_ambient_fade_up(SOUND_STREAM_CITY);
        }
        return;
    }
    ambient_current_type = best_type;
    const char *file = ambient_file(best_type);
    if (file) {
        ambient_play_mp3(file);
    } else {
        ambient_stop_stream();
    }
}

void sound_ambient_bind_channel(int32_t slot, const char *path, int32_t mode)
{
    ambient_channel *channel = &ambient_channel_base.slots[slot];
    if (mode == AMBIENT_BIND_ROAM) {
        channel->roam = sound_channel_load_file(path);
    } else if ((mode == AMBIENT_BIND_VARIANT) && (channel->variant_count < AMBIENT_CHANNEL_VARIANTS)) {
        channel->variants[channel->variant_count] = sound_channel_load_file(path);
        channel->variant_count += 1;
    }
}

static const struct {
    int32_t slot;
    int32_t mode;
    const char *path;
} ambient_binds[] = {
    { AMBIENT_TERRAIN_DESERT, AMBIENT_BIND_ROAM, "audio\\wavs\\desert_r.wav" },
    { AMBIENT_TERRAIN_DESERT, AMBIENT_BIND_VARIANT, "audio\\wavs\\desert1.wav" },
    { AMBIENT_TERRAIN_DESERT, AMBIENT_BIND_VARIANT, "audio\\wavs\\desert2.wav" },
    { AMBIENT_TERRAIN_DESERT, AMBIENT_BIND_VARIANT, "audio\\wavs\\desert3.wav" },
    { AMBIENT_TERRAIN_DESERT, AMBIENT_BIND_VARIANT, "audio\\wavs\\desert4.wav" },
    { AMBIENT_TERRAIN_DESERT, AMBIENT_BIND_VARIANT, "audio\\wavs\\desert5.wav" },
    { AMBIENT_TERRAIN_DESERT, AMBIENT_BIND_VARIANT, "audio\\wavs\\desert6.wav" },
    { AMBIENT_TERRAIN_DESERT, AMBIENT_BIND_VARIANT, "audio\\wavs\\desert7.wav" },
    { AMBIENT_TERRAIN_WATER, AMBIENT_BIND_ROAM, "audio\\wavs\\water_r.wav" },
    { AMBIENT_TERRAIN_WATER, AMBIENT_BIND_VARIANT, "audio\\wavs\\water1.wav" },
    { AMBIENT_TERRAIN_WATER, AMBIENT_BIND_VARIANT, "audio\\wavs\\water2.wav" },
    { AMBIENT_TERRAIN_WATER, AMBIENT_BIND_VARIANT, "audio\\wavs\\water3.wav" },
    { AMBIENT_TERRAIN_WATER, AMBIENT_BIND_VARIANT, "audio\\wavs\\water4.wav" },
    { AMBIENT_TERRAIN_WATER, AMBIENT_BIND_VARIANT, "audio\\wavs\\water5.wav" },
    { AMBIENT_TERRAIN_WATER, AMBIENT_BIND_VARIANT, "audio\\wavs\\water6.wav" },
    { AMBIENT_TERRAIN_WATER, AMBIENT_BIND_VARIANT, "audio\\wavs\\water7.wav" },
    { AMBIENT_TERRAIN_ROCK, AMBIENT_BIND_ROAM, "audio\\wavs\\rock_r.wav" },
    { AMBIENT_TERRAIN_ROCK, AMBIENT_BIND_VARIANT, "audio\\wavs\\rock1.wav" },
    { AMBIENT_TERRAIN_ROCK, AMBIENT_BIND_VARIANT, "audio\\wavs\\rock2.wav" },
    { AMBIENT_TERRAIN_ROCK, AMBIENT_BIND_VARIANT, "audio\\wavs\\rock3.wav" },
    { AMBIENT_TERRAIN_ROCK, AMBIENT_BIND_VARIANT, "audio\\wavs\\rock4.wav" },
    { AMBIENT_TERRAIN_ROCK, AMBIENT_BIND_VARIANT, "audio\\wavs\\rock5.wav" },
    { AMBIENT_TERRAIN_ROCK, AMBIENT_BIND_VARIANT, "audio\\wavs\\rock6.wav" },
    { AMBIENT_TERRAIN_ROCK, AMBIENT_BIND_VARIANT, "audio\\wavs\\rock7.wav" },
    { AMBIENT_TERRAIN_FARM, AMBIENT_BIND_ROAM, "audio\\wavs\\farm_r.wav" },
    { AMBIENT_TERRAIN_FARM, AMBIENT_BIND_VARIANT, "audio\\wavs\\farm1.wav" },
    { AMBIENT_TERRAIN_FARM, AMBIENT_BIND_VARIANT, "audio\\wavs\\farm2.wav" },
    { AMBIENT_TERRAIN_FARM, AMBIENT_BIND_VARIANT, "audio\\wavs\\farm3.wav" },
    { AMBIENT_TERRAIN_FARM, AMBIENT_BIND_VARIANT, "audio\\wavs\\farm4.wav" },
    { AMBIENT_TERRAIN_FARM, AMBIENT_BIND_VARIANT, "audio\\wavs\\farm5.wav" },
    { AMBIENT_TERRAIN_FARM, AMBIENT_BIND_VARIANT, "audio\\wavs\\farm6.wav" },
    { AMBIENT_TERRAIN_FARM, AMBIENT_BIND_VARIANT, "audio\\wavs\\farm7.wav" },
    { AMBIENT_TERRAIN_FLOOD, AMBIENT_BIND_ROAM, "audio\\wavs\\flood_r.wav" },
    { AMBIENT_TERRAIN_FLOOD, AMBIENT_BIND_VARIANT, "audio\\wavs\\flood1.wav" },
    { AMBIENT_TERRAIN_FLOOD, AMBIENT_BIND_VARIANT, "audio\\wavs\\flood2.wav" },
    { AMBIENT_TERRAIN_FLOOD, AMBIENT_BIND_VARIANT, "audio\\wavs\\flood3.wav" },
    { AMBIENT_TERRAIN_FLOOD, AMBIENT_BIND_VARIANT, "audio\\wavs\\flood4.wav" },
    { AMBIENT_TERRAIN_FLOOD, AMBIENT_BIND_VARIANT, "audio\\wavs\\flood5.wav" },
    { AMBIENT_TERRAIN_FLOOD, AMBIENT_BIND_VARIANT, "audio\\wavs\\flood6.wav" },
    { AMBIENT_TERRAIN_FLOOD, AMBIENT_BIND_VARIANT, "audio\\wavs\\flood7.wav" },
    { AMBIENT_TERRAIN_FOREST, AMBIENT_BIND_ROAM, "audio\\wavs\\forest_r.wav" },
    { AMBIENT_TERRAIN_FOREST, AMBIENT_BIND_VARIANT, "audio\\wavs\\forest1.wav" },
    { AMBIENT_TERRAIN_FOREST, AMBIENT_BIND_VARIANT, "audio\\wavs\\forest2.wav" },
    { AMBIENT_TERRAIN_FOREST, AMBIENT_BIND_VARIANT, "audio\\wavs\\forest3.wav" },
    { AMBIENT_TERRAIN_FOREST, AMBIENT_BIND_VARIANT, "audio\\wavs\\forest4.wav" },
    { AMBIENT_TERRAIN_FOREST, AMBIENT_BIND_VARIANT, "audio\\wavs\\forest5.wav" },
    { AMBIENT_TERRAIN_FOREST, AMBIENT_BIND_VARIANT, "audio\\wavs\\forest6.wav" },
    { AMBIENT_TERRAIN_FOREST, AMBIENT_BIND_VARIANT, "audio\\wavs\\forest7.wav" },
};

void sound_city_init_ambient_channels(void)
{
    for (int32_t i = 0; i < AMBIENT_CHANNEL_SLOTS; i++) {
        ambient_channel *channel = &ambient_channel_base.slots[i];
        for (int32_t j = 0; j < AMBIENT_CHANNEL_VARIANTS; j++) {
            channel->variants[j] = -1;
        }
        channel->roam = -1;
        channel->variant_count = 0;
    }
    for (int32_t i = 0; i < (int32_t) (sizeof(ambient_binds) / sizeof(ambient_binds[0])); i++) {
        sound_ambient_bind_channel(ambient_binds[i].slot, ambient_binds[i].path, ambient_binds[i].mode);
    }
}
