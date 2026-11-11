// The functions of player_selection.c that other files call.

#ifndef WINDOW_PLAYER_SELECTION_H
#define WINDOW_PLAYER_SELECTION_H

#include <stdint.h>

const char *dir_save_path(void);
void player_selection_create_player(void);
void window_player_selection_show(void);
void player_selection_draw_background(void);
void window_player_selection_draw_foreground(void);
void player_selection_button_new_player(void);
void player_selection_proceed_to_free_play(int32_t free_play);
void player_selection_button_proceed(void);
void player_selection_button_select(void);
void player_selection_confirm_delete(void);
void window_player_selection_draw_background(void);
void player_selection_button_scroll(void);
void player_selection_button_delete(void);
int32_t player_selection_image_button_count(void);
int32_t player_selection_handle_scrollbar(void);
void player_selection_load_files(const char *pattern);

#endif
