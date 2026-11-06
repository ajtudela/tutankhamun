// The tables of city/city_data.c (gods, health, culture, kingdom, finance, disasters and population).

#ifndef CITY_CITY_DATA_H
#define CITY_CITY_DATA_H

#include <stdint.h>

#include "city/city_player_types.h"
#include "city/message.h"
#include "scenario/scenario_climate.h"

extern int32_t god_bolts_tables[45];

extern int32_t culture_lowest_points;

extern int32_t culture_records[11][14];

extern int32_t salary_by_rank[11];

extern int32_t debt_kingdom_penalties[50];

extern int32_t mansion_share_table[22];

extern int32_t palace_share_table[21];

extern char frogs_sound_file[24];

extern char frog_debug_format[56];

extern char locust_sound_file[28];

extern int32_t births_per_age_decennium[10];

extern int32_t deaths_per_health_per_decennium[110];

extern int32_t health_population_bonus[20];

extern int32_t culture_lowest_index;

extern int32_t culture_coverage[23];

extern int32_t mummies_max;

extern int32_t city_locusts_countdown;

extern int32_t city_message_category;

extern uint8_t city_victory_cheated;

extern uint8_t city_victory_forced;

extern int8_t city_victory_state;

extern uint8_t game_fast_forward_year;

extern int32_t city_warning_shown;

extern int32_t city_message_queue[20];

extern int32_t city_message_count_problem;

extern int32_t city_warning_active_count;

extern int32_t city_message_use_popup;

extern int32_t city_message_posted_once;

extern int32_t city_message_scroll_position;

extern int32_t city_message_list_scroll;

extern int32_t city_message_delay[20];

extern int32_t city_message_count[20];

extern uint8_t city_message_population_shown[10];

extern uint32_t city_message_problem_area_time;

extern int32_t city_message_problem_area_b;

extern int32_t city_message_problem_count;

extern int32_t city_message_shown_type;

extern int32_t city_message_shown_index;

extern int32_t city_message_current_id;

extern int32_t city_message_total;

extern int32_t city_message_next_sequence;

extern city_message_table city_messages;

extern int32_t city_message_queue_delay;


extern city_player_name city_player_names[2];

extern city_player_faction_entry city_player_faction[2];

extern city_player_records city_players;

extern uint8_t city_health_recompute_distances;

#endif
