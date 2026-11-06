// City gods (city/gods.c, city/gods_curses.c): the moods, the blessings and curses and the timers of the god effects;
// god_rules and god_bolts_tables hold the tunable values.

#ifndef CITY_GODS_H
#define CITY_GODS_H

#include <stdint.h>

enum god {
    GOD_OSIRIS = 0,
    GOD_RA = 1,
    GOD_PTAH = 2,
    GOD_SETH = 3,
    GOD_BAST = 4,
    GOD_COUNT = 5,
};

enum god_bolts_mood {
    GOD_BOLTS_MOOD_90 = 0,
    GOD_BOLTS_MOOD_80 = 1,
    GOD_BOLTS_MOOD_70 = 2,
    GOD_BOLTS_MOOD_ABOVE_50 = 3,
    GOD_BOLTS_MOOD_50 = 4,
    GOD_BOLTS_MOOD_ABOVE_30 = 5,
    GOD_BOLTS_MOOD_ABOVE_20 = 6,
    GOD_BOLTS_MOOD_ABOVE_10 = 7,
    GOD_BOLTS_MOOD_10_OR_LESS = 8,
    GOD_BOLTS_MOODS = 9,
};

enum god_event {
    GOD_EVENT_RANDOM = -1,
    GOD_EVENT_MAJOR_BLESSING = 1,
    GOD_EVENT_MINOR_BLESSING = 2,
    GOD_EVENT_MAJOR_CURSE = 3,
    GOD_EVENT_MINOR_CURSE = 4,
};

enum god_rule {
    GOD_RULE_TUTORIAL_MOOD,
    GOD_RULE_FESTIVAL_MONTHS_CAP,
    GOD_RULE_FESTIVAL_MOOD_BONUS,
    GOD_RULE_FIRST_MISSION_MIN_MOOD,
    GOD_RULE_COMPLEX_GOD_PERCENTAGE,
    GOD_RULE_PATRON_WITHOUT_COMPLEX_PERCENTAGE,
    GOD_RULE_PATRON_BESIDE_COMPLEX_PERCENTAGE,
    GOD_RULE_LOCAL_BESIDE_COMPLEX_PERCENTAGE,
    GOD_RULE_ANGRY_MOOD,
    GOD_RULE_ANGRY_MESSAGE_DELAY,
    GOD_RULE_TUTORIAL_BAST_MOOD,
    GOD_RULE_BOLTS_MAX,
    GOD_RULE_MINOR_EVENT_BOLTS,
    GOD_RULE_BLESSING_FESTIVAL_MONTHS,
    GOD_RULE_CURSE_FESTIVAL_MONTHS,
    GOD_RULE_MAJOR_EVENT_MOOD_CHANGE,
    GOD_RULE_MINOR_EVENT_MOOD_CHANGE,
    GOD_RULE_RA_EFFECT_MONTHS,
    GOD_RULE_SETH_PROTECTION_MONTHS,
    GOD_RULE_SETH_ENEMIES_TO_LEAVE,
    GOD_RULE_RA_MAJOR_KINGDOM_CHANGE,
    GOD_RULE_RA_MINOR_KINGDOM_CHANGE,
    GOD_RULE_PTAH_FROGS,
    GOD_RULE_COUNT,
};

extern int32_t god_rules[GOD_RULE_COUNT];

enum god_status {
    GOD_STATUS_NONE = 0,
    GOD_STATUS_LOCAL = 1,
    GOD_STATUS_PATRON = 2,
};

void city_gods_calculate_moods(int32_t update_moods);
void city_gods_perform_blessing_or_curse(int32_t god, int32_t force);
void city_gods_decrease_effect_timers(void);
int32_t city_gods_ra_trade_modifier(void);
int32_t city_gods_ra_adjust_percentage(int32_t percentage);
int32_t city_gods_with_status(int32_t status);
int32_t city_gods_find_least_happy(void);
int32_t city_gods_ptah_fill_storage_yard(void);
int32_t city_gods_ptah_minor_blessing(void);
void city_gods_bast_major_blessing(void);

int32_t city_gods_ptah_major_curse(void);
void city_gods_ptah_minor_curse(int32_t destroy);
void building_storageyard_remove_resource_curse(int32_t building_id, int32_t amount);
int32_t city_gods_seth_minor_curse(void);
int city_gods_house_level_compare(const void *a, const void *b);
void city_gods_bast_major_curse(void);

enum osiris_curse {
    PTAH_CURSE_INDUSTRIES = 6,
    PTAH_CURSE_NO_INDUSTRY = -1,
    PTAH_CURSE_LOADS = 16,
};

enum bast_curse {
    BAST_CURSE_HOUSES = 20,
};

#endif
