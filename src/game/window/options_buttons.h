// The Options menu's entries and the options windows' buttons (options_buttons.c).

#ifndef WINDOW_OPTIONS_BUTTONS_H
#define WINDOW_OPTIONS_BUTTONS_H

#include <stdint.h>

void display_mode_confirm_open(void);
void window_go_city(void);
void window_display_options_show(void);
void window_game_options_show(void);
void display_options_button_mode(void);
void display_options_button_resolution(void);
void game_options_button_toggle(void);
void display_check_timeout(void);
void display_check_button_ok(void);
void window_difficulty_options_show(void);
void difficulty_options_arrow(void);
void window_sound_options_show(void);
void sound_options_button_cancel(void);
void sound_options_button_toggle(void);
void sound_options_arrow_music(void);
void sound_options_arrow_speech(void);
void sound_options_arrow_effects(void);
void sound_options_arrow_city(void);
void window_speed_options_show(void);
void speed_options_button_cancel(void);
void speed_options_arrow_scroll(void);
void options_toggle_autosave(void);
void options_toggle_pyramid_speedup(void);

#endif
