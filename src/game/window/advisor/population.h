// The functions of population.c that other files call, and the graph button last drawn as focused.

#ifndef WINDOW_ADVISOR_POPULATION_H
#define WINDOW_ADVISOR_POPULATION_H

#include <stdint.h>

void window_advisor_population_draw_background(void);
void window_advisor_population_draw_history_graph(int32_t full_size, int32_t x, int32_t y);
void window_advisor_population_draw_census_graph(int32_t full_size, int32_t x, int32_t y);
void window_advisor_population_draw_society_graph(int32_t full_size, int32_t x, int32_t y);
void window_advisor_population_draw_foreground(void);
void population_graph_scale(int32_t max_value);
void button_population_graph(void);

extern int32_t population_last_focus;

#endif
