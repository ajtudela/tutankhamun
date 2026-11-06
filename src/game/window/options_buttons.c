// The Options menu's entries and the buttons of the options windows: display (its rows post F6..F9 to the window
// procedure), the display mode check (back to the previous mode when its time runs out), popup messages, sound and
// speed (kept for cancel), difficulty (the models loaded again), the autosave and pyramid speed-up toggles, and
// "back to the city".

#include "window/options_buttons.h"
#include "building/model_load.h"
#include "city/city_player.h"
#include "city/difficulty.h"
#include "core/calc.h"
#include "editor/editor_data.h"
#include "figure/enemy.h"
#include "game/game_data.h"
#include "game/game_internal.h"
#include "graphics/label_animation_slots.h"
#include "platform/platform_input.h"
#include "platform/platform_message.h"
#include "platform/platform_time.h"
#include "sound/sound_data.h"
#include "sound/sound_lib.h"
#include "sound/sound_lib_samples.h"
#include "window/options_windows.h"
#include "window/top_menu_entries.h"
#include "window/window_data.h"
#include "window/window_id.h"
#include <stdint.h>

int32_t saved_game_speed;

int32_t saved_city_volume;

int32_t saved_city_enabled;

int32_t saved_effects_volume;

int32_t saved_effects_enabled;

int32_t saved_speech_volume;

int32_t saved_speech_enabled;

int32_t saved_music_volume;

int32_t saved_music_enabled;

enum options_buttons_rows {
    DISPLAY_OPTIONS_LAST_RESOLUTION = 3,
    GAME_OPTIONS_TOGGLES = 12,
    GAME_OPTIONS_ROW_OK = 13,
    SOUND_OPTIONS_MUSIC = 1,
    SOUND_OPTIONS_SPEECH = 2,
    SOUND_OPTIONS_EFFECTS = 3,
    SOUND_OPTIONS_CITY = 4,
    ARROW_UP = 0,
    ARROW_DOWN = 1,
    VOLUME_MAX = 100,
    SCROLL_SPEED_STEP = 10,
    SCROLL_SPEED_MAX = 100,
};

void window_go_city(void)
{
    window_current_id = WINDOW_CITY;
    window_redraw_request = 1;
}

void window_game_options_show(void)
{
    window_current_id = WINDOW_GAME_OPTIONS;
    game_options_flags = settings_data.message_filter;
    label_animation[LABEL_SLOT_GAME_OPTIONS] = 0;
    window_redraw_request = 1;
}

static void post_key(uintptr_t key)
{
    platform_message_post(platform_main_window, PLATFORM_MSG_KEY_DOWN, key, 0);
}

void display_options_button_mode(void)
{
    post_key(PLATFORM_KEY_F6);
    window_current_id = WINDOW_CITY;
    window_redraw_request = 1;
}

void display_options_button_resolution(void)
{
    if (button_parameter >= 1 && button_parameter <= DISPLAY_OPTIONS_LAST_RESOLUTION) {
        post_key(PLATFORM_KEY_F6 + button_parameter);
    }
    window_current_id = WINDOW_CITY;
}

void game_options_button_toggle(void)
{
    int32_t row = button_parameter;
    if (row >= 1 && row <= GAME_OPTIONS_TOGGLES) {
        game_options_flags ^= 1u << row;
        return;
    }
    if (row == GAME_OPTIONS_ROW_OK) {
        window_current_id = WINDOW_CITY;
        window_redraw_request = 1;
        settings_data.message_filter = game_options_flags;
    }
}

void display_check_timeout(void)
{
    post_key(PLATFORM_KEY_F7);
    window_redraw_request = 1;
    window_current_id = (uint8_t) popup.return_window;
}

void display_check_button_ok(void)
{
    settings_data.display_mode_start = settings_data.display_mode;
    window_current_id = (uint8_t) popup.return_window;
    window_redraw_request = 1;
}

void window_difficulty_options_show(void)
{
    window_current_id = WINDOW_DIFFICULTY_OPTIONS;
    label_animation[LABEL_SLOT_DISPLAY_OPTIONS] = 0;
    window_redraw_request = 1;
}

void difficulty_options_arrow(void)
{
    if (button_parameter == ARROW_UP) {
        settings_data.difficulty++;
    } else {
        settings_data.difficulty--;
    }
    if (settings_data.difficulty < DIFFICULTY_VERY_EASY) {
        settings_data.difficulty = DIFFICULTY_VERY_EASY;
    } else if (settings_data.difficulty > DIFFICULTY_VERY_HARD) {
        settings_data.difficulty = DIFFICULTY_VERY_HARD;
    }
    building_model_load();
    figure_model_load();
    tax_sentiment_model_load();
    figure_enemy_properties_init();
    if (window_current_id == WINDOW_BRIEFING) {
        city_player_current()->treasury = calc_adjust_with_percentage(scenario_data.initial_funds, difficulty_treasury_percentage[settings_data.difficulty]);
    }
    window_redraw_request = 1;
}

void window_sound_options_show(void)
{
    saved_music_volume = settings_data.sound_music_volume;
    saved_music_enabled = settings_data.sound_music_enabled;
    saved_speech_volume = settings_data.sound_speech_volume;
    saved_speech_enabled = settings_data.sound_speech_enabled;
    saved_effects_enabled = settings_data.sound_effects_enabled;
    saved_effects_volume = settings_data.sound_effects_volume;
    saved_city_enabled = settings_data.sound_city_enabled;
    saved_city_volume = settings_data.sound_city_volume;
    window_current_id = WINDOW_SOUND_OPTIONS;
    label_animation[LABEL_SLOT_SOUND_OPTIONS] = 0;
    window_redraw_request = 1;
}

static void set_music_volume(void)
{
    sound_lib_set_stream_volume(&sound_manager, SOUND_STREAM_MUSIC, settings_data.sound_music_volume);
    sound_lib_set_stream0_volume(&sound_manager, settings_data.sound_music_volume);
}

static void set_speech_volume(void)
{
    sound_lib_set_stream_volume(&sound_manager, SOUND_STREAM_SPEECH, settings_data.sound_speech_volume);
    sound_lib_set_stream1_volume(&sound_manager, settings_data.sound_speech_volume);
}

void sound_options_button_cancel(void)
{
    settings_data.sound_music_volume = saved_music_volume;
    settings_data.sound_speech_enabled = (uint8_t) saved_speech_enabled;
    settings_data.sound_speech_volume = saved_speech_volume;
    settings_data.sound_effects_enabled = (uint8_t) saved_effects_enabled;
    settings_data.sound_effects_volume = saved_effects_volume;
    settings_data.sound_music_enabled = (uint8_t) saved_music_enabled;
    settings_data.sound_city_enabled = (uint8_t) saved_city_enabled;
    settings_data.sound_city_volume = saved_city_volume;
    if ((uint8_t) saved_music_enabled == 0) {
        sound_lib_stop_stream(&sound_manager, SOUND_STREAM_MUSIC);
    }
    set_music_volume();
    sound_lib_set_stream_volume(&sound_manager, SOUND_STREAM_CITY, settings_data.sound_city_volume);
    sound_lib_set_stream2_volume(&sound_manager, settings_data.sound_city_volume);
    set_speech_volume();
    window_current_id = WINDOW_CITY;
    window_redraw_request = 1;
}

static void toggle_stream_sound(uint8_t *enabled, int32_t stream)
{
    if (*enabled == 0) {
        *enabled = 1;
        return;
    }
    *enabled = 0;
    sound_lib_stop_stream(&sound_manager, stream);
}

void sound_options_button_toggle(void)
{
    if (button_parameter == SOUND_OPTIONS_MUSIC) {
        toggle_stream_sound(&settings_data.sound_music_enabled, SOUND_STREAM_MUSIC);
    } else if (button_parameter == SOUND_OPTIONS_SPEECH) {
        toggle_stream_sound(&settings_data.sound_speech_enabled, SOUND_STREAM_SPEECH);
    } else if (button_parameter == SOUND_OPTIONS_EFFECTS) {
        settings_data.sound_effects_enabled = settings_data.sound_effects_enabled == 0;
    } else if (button_parameter == SOUND_OPTIONS_CITY) {
        toggle_stream_sound(&settings_data.sound_city_enabled, SOUND_STREAM_CITY);
    }
    window_redraw_request = 1;
}

static int32_t volume_step(int32_t volume)
{
    if (button_parameter == ARROW_DOWN) {
        volume--;
    } else if (button_parameter == ARROW_UP) {
        volume++;
    }
    if (volume < 0) {
        return 0;
    }
    return volume > VOLUME_MAX ? VOLUME_MAX : volume;
}

void sound_options_arrow_music(void)
{
    settings_data.sound_music_volume = volume_step(settings_data.sound_music_volume);
    set_music_volume();
    window_redraw_request = 1;
}

void sound_options_arrow_speech(void)
{
    settings_data.sound_speech_volume = volume_step(settings_data.sound_speech_volume);
    set_speech_volume();
    window_redraw_request = 1;
}

void sound_options_arrow_effects(void)
{
    settings_data.sound_effects_volume = volume_step(settings_data.sound_effects_volume);
    window_redraw_request = 1;
}

void sound_options_arrow_city(void)
{
    settings_data.sound_city_volume = volume_step(settings_data.sound_city_volume);
    sound_lib_set_stream2_volume(&sound_manager, settings_data.sound_city_volume);
    window_redraw_request = 1;
}

void window_speed_options_show(void)
{
    saved_game_speed = settings_data.game_speed;
    saved_scroll_speed = settings_data.scroll_speed;
    window_current_id = WINDOW_SPEED_OPTIONS;
    label_animation[LABEL_SLOT_SPEED_OPTIONS] = 0;
    window_redraw_request = 1;
}

void speed_options_button_cancel(void)
{
    settings_data.game_speed = saved_game_speed;
    settings_data.scroll_speed = saved_scroll_speed;
    window_current_id = WINDOW_CITY;
    window_redraw_request = 1;
}

void speed_options_arrow_scroll(void)
{
    if (button_parameter == ARROW_DOWN) {
        settings_data.scroll_speed -= SCROLL_SPEED_STEP;
    } else if (button_parameter == ARROW_UP) {
        settings_data.scroll_speed += SCROLL_SPEED_STEP;
    }
    if (settings_data.scroll_speed < 0) {
        settings_data.scroll_speed = 0;
    } else if (settings_data.scroll_speed > SCROLL_SPEED_MAX) {
        settings_data.scroll_speed = SCROLL_SPEED_MAX;
    }
    window_redraw_request = 1;
}

void options_toggle_autosave(void)
{
    settings_data.autosave = settings_data.autosave == 0;
    g_menu_items[MENU_ITEM_OPTIONS_AUTOSAVE].text_number = settings_data.autosave ? OPTIONS_MENU_AUTOSAVE_ON : OPTIONS_MENU_AUTOSAVE_OFF;
    popup.menu_dropdown_keep_open = 1;
}

void options_toggle_pyramid_speedup(void)
{
    settings_data.pyramid_speedup = settings_data.pyramid_speedup == 0;
    g_menu_items[MENU_ITEM_OPTIONS_PYRAMID_SPEEDUP].text_number = settings_data.pyramid_speedup ? OPTIONS_MENU_PYRAMID_SPEEDUP_ON : OPTIONS_MENU_PYRAMID_SPEEDUP_OFF;
    popup.menu_dropdown_keep_open = 1;
}

void display_mode_confirm_open(void)
{
    if (settings_data.display_mode <= settings_data.display_mode_start || settings_data.display_mode <= DISPLAY_MODE_640X480) {
        return;
    }
    display_mode_confirm_time = platform_time_ms();
    popup.return_window = window_current_id;
    window_current_id = WINDOW_DISPLAY_MODE_CONFIRM;
    window_redraw_request = 1;
}

void window_display_options_show(void)
{
    window_current_id = WINDOW_DISPLAY_OPTIONS;
    label_animation[LABEL_SLOT_DISPLAY_OPTIONS] = 0;
    window_redraw_request = 1;
}
