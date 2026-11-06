// The initial data of the city's tables as C tables: gods, health, culture, kingdom, finance, disasters and
// population.

#include "city/city_data.h"

#include <assert.h>

int32_t god_bolts_tables[45] = { 6, 4, 2, 1, 1, 3, 2, 1, 1, 0, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 0, 1, 1, 2, 3, 1, 1,
    2, 4, 5 };

int32_t culture_lowest_points = 100;

int32_t culture_records[11][14] = {
    { 0, 5, 15, 25, 40, 55, 75, 100, 0, 20, 40, 60, 80, 100 },
    { 1, 10, 20, 30, 45, 60, 80, 100, 0, 20, 40, 60, 80, 100 },
    { 2, 15, 25, 35, 50, 65, 85, 100, 0, 20, 40, 60, 80, 100 },
    { 3, 50, 55, 65, 75, 85, 95, 100, 0, 20, 40, 60, 80, 100 },
    { 4, 70, 75, 80, 85, 90, 95, 100, 0, 20, 40, 60, 80, 100 },
    { 7, 0, 10, 25, 40, 60, 85, 100, 0, 20, 40, 60, 80, 100 },
    { 5, 40, 45, 50, 60, 75, 85, 100, 0, 20, 40, 60, 80, 100 },
    { 6, 60, 65, 70, 75, 80, 90, 100, 0, 20, 40, 60, 80, 100 },
    { 8, 15, 20, 25, 35, 50, 65, 100, 0, 20, 40, 60, 80, 90 },
    { 10, 20, 25, 30, 40, 55, 70, 100, 0, 20, 40, 60, 80, 90 },
    { 11, 35, 45, 55, 70, 85, 100, 100, 0, 20, 40, 60, 80, 100 },
};

int32_t salary_by_rank[11] = { 0, 2, 5, 8, 12, 20, 30, 40, 60, 80, 100 };

int32_t debt_kingdom_penalties[50] = { -2, -5, -7, -10, -15, -20, -27, -35, -47, -50, -3, -7, -12, -20, -30, -35, -40, -45, -50, -50, -5, -10, -20, -35, -50,
    -50, -50, -50, -50, -50, -10, -25, -40, -50, -50, -50, -50, -50, -50, -50, -15, -35, -50, -50, -50, -50, -50, -50, -50, -50 };

int32_t mansion_share_table[22] = { 1, 77, 0, 0, 0, 0, 1, 1, 78, 0, 0, 0, 0, 2, 1, 79, 0, 0, 0, 0, 3 };

int32_t palace_share_table[21] = { 2, 86, 87, 0, 0, 0, 2, 1, 184, 0, 0, 0, 0, 3, 5, 187, 188, 189, 84, 85, 50 };

char frogs_sound_file[24] = "audio\\ambient\\frogs.mp3";

char frog_debug_format[56] = "Created frog %d at: %d, %d, headed for building: %d\n";

char locust_sound_file[28] = "audio\\ambient\\locusts.mp3";

int32_t births_per_age_decennium[10] = { 0, 3, 16, 9, 2 };

int32_t deaths_per_health_per_decennium[110] = { 20, 10, 5, 10, 20, 30, 50, 85, 100, 100, 15, 8, 4, 8, 16, 25, 45, 70, 90, 100, 10, 6, 2, 6, 12, 20, 30, 55, 80,
    90, 5, 4, 0, 4, 8, 15, 25, 40, 65, 80, 3, 2, 0, 2, 6, 12, 20, 30, 50, 70, 2, 0, 0, 0, 4, 8, 15, 25, 40, 60, 1, 0, 0, 0, 2, 6, 12, 20, 30, 50, 0, 0, 0, 0, 0,
    4, 8, 15, 20, 40, 0, 0, 0, 0, 0, 2, 6, 10, 15, 30, 0, 0, 0, 0, 0, 0, 4, 5, 10, 20, 0, 0, 0, 0, 0, 0, 0, 2, 5, 10 };

int32_t health_population_bonus[20] = { 200, 100, 400, 90, 600, 80, 800, 70, 1000, 60, 1100, 50, 1200, 40, 1300, 30, 1400, 20, 1500, 10 };

int32_t culture_lowest_index;

int32_t culture_coverage[23];

int32_t mummies_max;

int32_t city_locusts_countdown;

int32_t city_message_category;

uint8_t city_victory_cheated;

uint8_t city_victory_forced;

int8_t city_victory_state;

uint8_t game_fast_forward_year;

int32_t city_warning_shown;

int32_t city_message_queue[20];

int32_t city_message_count_problem;

int32_t city_warning_active_count;

int32_t city_message_use_popup;

int32_t city_message_posted_once;

int32_t city_message_scroll_position;

int32_t city_message_list_scroll;

int32_t city_message_delay[20];

int32_t city_message_count[20];

uint8_t city_message_population_shown[10];

uint32_t city_message_problem_area_time;

int32_t city_message_problem_area_b;

int32_t city_message_problem_count;

int32_t city_message_shown_type;

int32_t city_message_shown_index;

int32_t city_message_current_id;

int32_t city_message_total;

int32_t city_message_next_sequence;

static_assert(sizeof(city_message_table) == 48000, "city_messages is the savegame's message table of 48000 bytes");
city_message_table city_messages;

int32_t city_message_queue_delay;


static_assert(sizeof(city_player_name) * 2 == 64, "city_player_names is the savegame's player names of 64 bytes");
city_player_name city_player_names[2];

static_assert(sizeof(city_player_faction_entry) * 2 == 4, "city_player_faction is the savegame's player factions of 4 bytes");
city_player_faction_entry city_player_faction[2];

static_assert(sizeof(city_player_records) == 37808, "city_players is the savegame's player table of 37808 bytes");
city_player_records city_players;

uint8_t city_health_recompute_distances;
