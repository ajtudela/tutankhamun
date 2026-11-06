// The high scores (high_scores.c): the window, the table's file and the record of a won mission with its result.

#ifndef WINDOW_HIGH_SCORES_H
#define WINDOW_HIGH_SCORES_H

#include <stdbool.h>
#include <stdint.h>

enum high_score_result {
    HIGH_SCORE_RESULT_NONE = 0,
    HIGH_SCORE_RESULT_CHEATED = 1,
    HIGH_SCORE_RESULT_NEW = 2,
    HIGH_SCORE_RESULT_OWN_KEPT = 3,
    HIGH_SCORE_RESULT_OWN_BEATEN = 4,
    HIGH_SCORE_RESULT_OTHER_BEATEN = 5,
    HIGH_SCORE_RESULT_OTHER_KEPT = 6,
};

void high_scores_draw_list(void);
void highscore_set_entry(int32_t mission, int32_t monuments, int32_t score, int32_t months);
void highscore_record_mission(void);
void window_high_scores_draw_background(void);
void high_scores_draw_scrollbar_dot(void);
void high_scores_scroll(void);
int32_t high_scores_handle_scrollbar(void);
bool highscore_load(void);
bool highscore_write_file(void *self, const char *path);

#endif
