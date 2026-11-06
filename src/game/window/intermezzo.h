// The intermezzo, the mission choice and the briefing's scroll buttons (intermezzo.c).

#ifndef WINDOW_INTERMEZZO_H
#define WINDOW_INTERMEZZO_H

#include <stdint.h>

enum intermezzo_type {
    INTERMEZZO_MISSION_BRIEFING = 0,
    INTERMEZZO_FIRED = 1,
    INTERMEZZO_MISSION_WON = 2,
};

enum intermezzo_duration {
    INTERMEZZO_DURATION_MS = 1000,
};

void mission_start_intermezzo(void);
void window_end_picture_2_draw(void);
void window_end_picture_3_draw(void);
void window_end_picture_final_draw(void);
void mission_button_accept(void);
void mission_button_back(void);
void mission_show_briefing(void);
void mission_briefing_draw_scroll(void);
void mission_briefing_handle_scroll(void);
void mission_choice_click(void);
void mission_choice_draw_markers(void);
void window_mission_choice_draw(void);
void window_intermezzo_draw(void);
void window_intermezzo_handle(void);
int32_t window_intermezzo_show(int32_t type, int32_t next_window, int32_t duration_ms);
void window_end_picture_1_draw(void);
int32_t mission_briefing_message(void);
int32_t mission_transition_text(void);

#endif
