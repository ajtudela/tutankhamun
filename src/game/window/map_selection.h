// The map selection (map_selection.c): its list and entries, the information panel, the difficulty rating with its
// brackets (map_rating_brackets.json); the picture and the panel are drawn for the free play window too.

#ifndef WINDOW_MAP_SELECTION_H
#define WINDOW_MAP_SELECTION_H

#include "core/static_init.h"
#include "graphics/button.h"
#include "scenario/scenario_event_types.h"
#include <stdbool.h>
#include <stdint.h>

enum map_selection_mode {
    MAP_SELECTION_CAMPAIGN = 2,
    MAP_SELECTION_CUSTOM = 3,
};

enum map_entry_kind {
    MAP_ENTRY_CUSTOM = 0,
    MAP_ENTRY_CAMPAIGN = 1,
    MAP_ENTRY_LOCKED = 2,
    MAP_ENTRY_NONE = 3,
};

enum map_rating_bracket_id {
    MAP_RATING_POPULATION_NONE,
    MAP_RATING_POPULATION_SMALL,
    MAP_RATING_POPULATION_MEDIUM,
    MAP_RATING_POPULATION_LARGE,
    MAP_RATING_POPULATION_HUGE,
    MAP_RATING_INVASIONS_NONE,
    MAP_RATING_INVASIONS_FEW,
    MAP_RATING_INVASIONS_SOME,
    MAP_RATING_INVASIONS_MANY,
    MAP_RATING_INVASIONS_MOST,
    MAP_RATING_REQUESTS_FEW,
    MAP_RATING_REQUESTS_SOME,
    MAP_RATING_REQUESTS_MANY,
    MAP_RATING_REQUESTS_MORE,
    MAP_RATING_REQUESTS_MOST,
    MAP_RATING_GODS_NONE,
    MAP_RATING_GODS_FEW,
    MAP_RATING_GODS_SOME,
    MAP_RATING_GODS_MANY,
    MAP_RATING_GODS_MOST,
    MAP_RATING_RANK_0,
    MAP_RATING_RANK_1,
    MAP_RATING_RANK_2,
    MAP_RATING_RANK_3,
    MAP_RATING_RANK_4,
    MAP_RATING_RANK_5,
    MAP_RATING_RANK_6,
    MAP_RATING_RANK_7,
    MAP_RATING_RANK_8,
    MAP_RATING_RANK_9,
    MAP_RATING_BRACKET_COUNT,
};

typedef struct {
    int32_t max;
    int32_t value;
} map_rating_bracket;

extern map_rating_bracket map_rating_brackets[MAP_RATING_BRACKET_COUNT];

bool map_list_fill(map_list *self, const char *pattern, int32_t keep);
int32_t map_list_entry_mission(map_list *self, int32_t index);
int32_t map_list_entry_kind(map_list *self, int32_t index);
char *map_list_entry_name(map_list *self, int32_t index);
void window_map_selection_show(void);
void map_selection_draw_list(void);
void window_map_selection_draw(void);
void map_selection_draw_scroll_thumb(void);
void map_selection_draw_scenario_picture(int32_t base_x, int32_t image_y);
void map_selection_draw_map_information(int32_t x, int32_t y, int32_t label_x, int32_t font);
void map_selection_select(void);
void map_selection_scroll(void);
void map_selection_toggle_kind(void);
void map_selection_start(void);
void map_selection_back(void);
int32_t map_selection_handle_scrollbar(void);
void map_selection_load_list(const char *pattern);
int32_t map_selection_rating(void);
int32_t map_selection_count_invasions(void);
image_button *map_selection_row_buttons(void);
generic_button *map_selection_toggle_buttons(void);
image_button *map_selection_image_buttons(void);
int32_t map_selection_has_image_buttons(void);
int32_t map_selection_image_button_count(void);
generic_button *map_selection_buttons(void);
int32_t map_selection_button_count(void);
bool scenario_event_has_chain(const scenario_event *self);
bool scenario_event_type_chains(const scenario_event *self);
bool scenario_event_has_followup(const scenario_event *self);

#endif
