// The campaign object, filled by the campaign loader: up to 100 mission links and 100 mission names, the saved game
// path and the best result of each mission. A link holds its period, the path that leads to it and the paths merging
// into it, its intro, mission and transition texts, the choice screen and up to two choices, and the choice made.

#ifndef SCENARIO_CAMPAIGN_H
#define SCENARIO_CAMPAIGN_H

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

enum campaign_link_limits {
    CAMPAIGN_LINK_MERGES = 5,
    CAMPAIGN_LINK_CHOICES = 2,
    CAMPAIGN_PERIODS = 9,
    CAMPAIGN_PERIOD_NONE = 0xFF,
    CAMPAIGN_LINK_NONE = 0xFF,
};

typedef struct {
    int32_t x;
    int32_t y;
    int16_t text_id;
    uint8_t path_id;
    uint8_t unused;
} campaign_choice;

typedef struct {
    uint8_t period;
    uint8_t path_id;
    uint8_t merge_ids[CAMPAIGN_LINK_MERGES];
    uint8_t merge_count;
    int32_t intro_id;
    int32_t mission_id;
    int32_t transition_text_id;
    int32_t reserved_1;
    int32_t text_id;
    int32_t background_id;
    campaign_choice choices[CAMPAIGN_LINK_CHOICES];
    int32_t choice_count;
    uint8_t played;
    uint8_t pad[3];
} campaign_link;

static_assert(sizeof(campaign_link) == 0x40, "campaign_link is a player file record of 64 bytes");

typedef struct {
    int32_t score;
    int32_t mission;
    char player_name[32];
    int32_t rating_culture;
    int32_t rating_prosperity;
    int32_t rating_kingdom;
    int32_t population;
    int32_t treasury;
    int32_t months;
    int32_t difficulty;
    int32_t monuments;
    int32_t used;
} mission_record;

static_assert(sizeof(mission_record) == 0x4C, "mission_record is a player file record of 76 bytes");

#define CAMPAIGN_MAX_LINKS 100
#define CAMPAIGN_MAX_MISSIONS 100
#define CAMPAIGN_NAME_LENGTH 50

typedef struct {
    campaign_link links[CAMPAIGN_MAX_LINKS];
    int32_t link_count;
    char mission_names[CAMPAIGN_MAX_MISSIONS][CAMPAIGN_NAME_LENGTH];
    int32_t mission_name_count;
    char saved_game_path[65];
    uint8_t reserved_1[3];
    mission_record mission_records[CAMPAIGN_MAX_MISSIONS];
} campaign;

typedef struct {
    mission_record missions[CAMPAIGN_MAX_MISSIONS];
} mission_record_table;

typedef struct {
    uint8_t choice;
    uint8_t reserved[3];
} campaign_mission_choice_slot;

#define CAMPAIGN_SIZE 0x4A84
static_assert(sizeof(campaign) == CAMPAIGN_SIZE, "campaign is a player file record of 19076 bytes");
static_assert(offsetof(campaign, link_count) == 0x1900 && offsetof(campaign, mission_names) == 0x1904 && offsetof(campaign, mission_name_count) == 0x2C8C &&
                  offsetof(campaign, saved_game_path) == 0x2C90 && offsetof(campaign, mission_records) == 0x2CD4, "campaign is the player file's layout");

#endif
