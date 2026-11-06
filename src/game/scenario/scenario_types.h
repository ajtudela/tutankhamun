// The scenario record (scenario_data), the 1592 bytes of a map's scenario settings: start year, climate, gods, funds,
// enemy, map size, name and briefing, the herd, fishing and invasion points, win criteria and time limits, entry,
// exit and river points, allowed buildings, monuments, burial provisions and the other settings. A point not set is
// -1. The start flags' entry 6 is the tutorial lock of the empire map. enemy_type_high is the word after the enemy
// type, which the foreign army text reads with it as one 32-bit value.

#ifndef SCENARIO_SCENARIO_TYPES_H
#define SCENARIO_SCENARIO_TYPES_H

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

enum scenario_record_sizes {
    SCENARIO_GODS = 5,
    SCENARIO_NAME_SIZE = 64,
    SCENARIO_BRIEFING_SIZE = 512,
    SCENARIO_HERD_POINTS = 4,
    SCENARIO_FISHING_POINTS = 8,
    SCENARIO_LAND_INVASION_POINTS = 8,
    SCENARIO_INVASION_POINTS = 16,
    SCENARIO_INVASION_LANDINGS = 3,
    SCENARIO_ALLOWED_BUILDINGS = 104,
    SCENARIO_MONUMENTS = 3,
    SCENARIO_BURIAL_RESOURCES = 36,
};

enum scenario_start_flag {
    SCENARIO_START_FLAG_EMPIRE_LOCKED = 6,
};

typedef union scenario_start {
    int16_t year;
    int32_t word;
} scenario_start;

typedef struct scenario_record {
    scenario_start start;
    int16_t climate;
    uint8_t reserved_1[4];
    int16_t god_status[SCENARIO_GODS];
    uint8_t reserved_2[12];
    int32_t initial_funds;
    int16_t enemy_type;
    int16_t enemy_type_high;
    uint8_t reserved_3[4];
    int32_t map_width;
    int32_t map_height;
    int32_t grid_start;
    int32_t grid_border_size;
    char name[SCENARIO_NAME_SIZE];
    char briefing[SCENARIO_BRIEFING_SIZE];
    uint8_t reserved_4[10];
    int16_t image;
    int16_t open_play;
    int16_t initial_rank;
    int16_t predator_herd_x[SCENARIO_HERD_POINTS];
    int16_t predator_herd_y[SCENARIO_HERD_POINTS];
    int16_t fishing_point_x[SCENARIO_FISHING_POINTS];
    int16_t fishing_point_y[SCENARIO_FISHING_POINTS];
    int16_t predator_variant;
    uint8_t reserved_5[38];
    int32_t kingdom_supplies_grain;
    int16_t invasion_point_x[SCENARIO_INVASION_POINTS];
    int16_t invasion_point_y[SCENARIO_INVASION_POINTS];
    uint8_t reserved_6[36];
    int32_t win_culture;
    int32_t win_prosperity;
    int32_t win_monument;
    int32_t win_kingdom;
    int32_t win_houses;
    int32_t win_houses_level;
    uint8_t win_culture_enabled;
    uint8_t win_prosperity_enabled;
    uint8_t win_monument_enabled;
    uint8_t win_kingdom_enabled;
    uint8_t reserved_7[8];
    int32_t time_limit;
    int32_t time_limit_years;
    int32_t survival;
    int32_t survival_years;
    int32_t win_population_enabled;
    int32_t win_population;
    int16_t earthquake_x;
    int16_t earthquake_y;
    int16_t entry_point_x;
    int16_t entry_point_y;
    int16_t exit_point_x;
    int16_t exit_point_y;
    uint8_t reserved_8[32];
    int16_t river_entry_x;
    int16_t river_entry_y;
    int16_t river_exit_x;
    int16_t river_exit_y;
    int32_t bailout_amount;
    int32_t milestone25_year;
    int32_t milestone50_year;
    int32_t milestone75_year;
    int32_t native_image_hut;
    int32_t native_image_meeting;
    int32_t native_image_crops;
    uint8_t climate_type;
    uint8_t flotsam_enabled;
    uint8_t reserved_9[2];
    int32_t empire_expanded;
    uint8_t reserved_10[4];
    uint8_t animation_set;
    uint8_t soldier_images_alt;
    uint8_t open_play_scenario_id;
    uint8_t reserved_11;
    int32_t prey_herd_x[SCENARIO_HERD_POINTS];
    int32_t prey_herd_y[SCENARIO_HERD_POINTS];
    uint16_t allowed_buildings[SCENARIO_ALLOWED_BUILDINGS];
    int16_t temple_complexes_allowed[SCENARIO_GODS];
    uint8_t reserved_12[10];
    int32_t invasion_landing_x[SCENARIO_INVASION_LANDINGS];
    int32_t invasion_landing_y[SCENARIO_INVASION_LANDINGS];
    int32_t interest_rate;
    int16_t monument_def_ids[SCENARIO_MONUMENTS];
    uint8_t reserved_13[2];
    int32_t provisions_needed[SCENARIO_BURIAL_RESOURCES];
    int32_t provisions_sent[SCENARIO_BURIAL_RESOURCES];
    int32_t pharaoh_name_id;
    int32_t player_title_id;
} scenario_record;

static_assert(sizeof(scenario_record) == 0x638, "scenario_record is a savegame and scenario record of 1592 bytes");
static_assert(offsetof(scenario_record, god_status) == 0xA && offsetof(scenario_record, name) == 0x3C && offsetof(scenario_record, image) == 0x286 &&
                  offsetof(scenario_record, kingdom_supplies_grain) == 0x2E4 && offsetof(scenario_record, win_culture) == 0x34C &&
                  offsetof(scenario_record, time_limit) == 0x370 && offsetof(scenario_record, river_entry_x) == 0x3B4 &&
                  offsetof(scenario_record, empire_expanded) == 0x3DC && offsetof(scenario_record, allowed_buildings) == 0x408 &&
                  offsetof(scenario_record, temple_complexes_allowed) == 0x4D8 && offsetof(scenario_record, invasion_landing_x) == 0x4EC &&
                  offsetof(scenario_record, provisions_needed) == 0x510 && offsetof(scenario_record, pharaoh_name_id) == 0x630,
              "scenario_record is the savegame and scenario layout");

#endif
