// Playing sounds: a sound effect at the effects volume, a figure's sound panned by its x in the view (quieter out of
// view, for soldiers, enemies and natives only), and the fade-out and fade-down of the streams.

#include "sound/sound_play.h"

#include "building/pyramid_images.h"
#include "figure/figure.h"
#include "figure/figure_type.h"
#include "game/game_data.h"
#include "graphics/graphics_data.h"
#include "sound/effect.h"
#include "sound/sound_data.h"
#include "sound/sound_lib.h"
#include "widget/widget_data.h"
#include "window/window_data.h"

enum sound_play_limits {
    FIGURE_SOUND_QUIET_DIVISOR = 4,
    SOUND_FADE_STEP = 5,
};

void sound_effect_play(int32_t effect)
{
    if (!sound_digital_ok || effect >= SOUND_EFFECT_COUNT || !settings_data.sound_effects_enabled) {
        return;
    }
    int32_t channel = effect_channels[effect];
    if (channel <= -1) {
        return;
    }
    sound_manager.request.volume = settings_data.sound_effects_volume;
    sound_manager.request.pan = SOUND_PAN_CENTRE;
    sound_manager.request.name_index = channel;
    sound_manager.request.id = -1;
    sound_manager.request.loop_count = 1;
    sound_lib_play_sample(&sound_manager, &sound_manager.request);
}

static int32_t figure_sounds_out_of_view(int32_t type)
{
    switch (type) {
        case FIGURE_ARCHER:
        case FIGURE_INFANTRY:
        case FIGURE_CHARIOTEER:
        case FIGURE_ENEMY_ARCHER:
        case FIGURE_ENEMY_INFANTRY:
        case FIGURE_ENEMY_CHARIOT:
        case FIGURE_ENEMY_EGYPTIAN_INFANTRY:
        case FIGURE_ENEMY_EGYPTIAN_ARCHER:
        case FIGURE_ENEMY_EGYPTIAN_CHARIOT:
        case FIGURE_TRANSPORT_SHIP:
        case FIGURE_WARSHIP:
        case FIGURE_ENEMY_TRANSPORT_SHIP:
        case FIGURE_ENEMY_WARSHIP:
        case FIGURE_BEDOUIN:
        case FIGURE_ENEMY_EGYPTIAN_WARSHIP:
        case FIGURE_ENEMY_EGYPTIAN_TRANSPORT_SHIP:
            return 1;
    }
    return 0;
}

void sound_figure_play(int32_t figure_id, int32_t sound, int32_t variant)
{
    int32_t height = screen_height;
    if (!sound_digital_ok || sound >= FIGURE_SOUND_COUNT || variant >= FIGURE_SOUND_VARIANTS || !settings_data.sound_effects_enabled) {
        return;
    }
    int32_t width = g_sidebar_collapsed ? city_view_width_collapsed : city_view_width_expanded;
    int32_t x, y;
    const figure *f = figure_get(figure_id);
    city_view_tile_to_pixel(f->x, f->y, &x, &y);
    int32_t quiet = 0;
    if (x < 0 || y < 0 || x > width || y > height) {
        if (!figure_sounds_out_of_view(f->type)) {
            return;
        }
        quiet = 1;
    }
    int32_t channel = figure_channels[(sound * FIGURE_SOUND_VARIANTS) + variant];
    if (channel <= -1) {
        return;
    }
    int32_t view_width = g_sidebar_collapsed ? city_view_width_collapsed : city_view_width_expanded;
    sound_manager.request.pan = (int32_t) ((double) x * ((double) SOUND_PAN_RANGE / (double) view_width));
    if (quiet) {
        sound_manager.request.volume = settings_data.sound_effects_volume / FIGURE_SOUND_QUIET_DIVISOR;
    } else {
        sound_manager.request.volume = settings_data.sound_effects_volume;
    }
    sound_manager.request.name_index = channel;
    sound_manager.request.id = -1;
    sound_manager.request.loop_count = 1;
    sound_lib_play_sample(&sound_manager, &sound_manager.request);
}

void sound_stream_fade_out(int32_t stream)
{
    if (!sound_digital_ok || !sound_manager.streams[stream] || !sound_lib_stream_is_playing(&sound_manager, stream)) {
        return;
    }
    int32_t volume = sound_lib_get_stream_volume(&sound_manager, stream);
    while (volume > 0) {
        volume -= SOUND_FADE_STEP;
        sound_lib_set_stream_volume(&sound_manager, stream, volume);
    }
    sound_lib_stop_stream(&sound_manager, stream);
}

void sound_stream_fade_down(int32_t stream)
{
    if (sound_digital_ok && sound_manager.streams[stream] && sound_lib_stream_is_playing(&sound_manager, stream)) {
        int32_t volume = sound_lib_get_stream_volume(&sound_manager, stream) - SOUND_FADE_STEP;
        if (volume >= 0) {
            sound_lib_set_stream_volume(&sound_manager, stream, volume);
            return;
        }
        sound_lib_stop_stream(&sound_manager, stream);
    }
    if (stream == SOUND_STREAM_MUSIC) {
        sound_music_fading = 0;
    } else if (stream == SOUND_STREAM_SPEECH) {
        sound_speech_fading = 0;
    }
}

