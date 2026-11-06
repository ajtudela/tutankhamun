// The gameplay config tables god_rules.json (god_rules, the tunable values of the moods, blessings and curses) and
// god_bolts.json (god_bolts_tables, per difficulty level the bolts a god's mood gives).

#include "city/city_data.h"
#include "city/difficulty.h"
#include "city/gods.h"
#include "core/config.h"

static const config_name god_rule_names[] = {
    { "tutorial_mood", GOD_RULE_TUTORIAL_MOOD },
    { "festival_months_cap", GOD_RULE_FESTIVAL_MONTHS_CAP },
    { "festival_mood_bonus", GOD_RULE_FESTIVAL_MOOD_BONUS },
    { "first_mission_min_mood", GOD_RULE_FIRST_MISSION_MIN_MOOD },
    { "complex_god_percentage", GOD_RULE_COMPLEX_GOD_PERCENTAGE },
    { "patron_without_complex_percentage", GOD_RULE_PATRON_WITHOUT_COMPLEX_PERCENTAGE },
    { "patron_beside_complex_percentage", GOD_RULE_PATRON_BESIDE_COMPLEX_PERCENTAGE },
    { "local_beside_complex_percentage", GOD_RULE_LOCAL_BESIDE_COMPLEX_PERCENTAGE },
    { "angry_mood", GOD_RULE_ANGRY_MOOD },
    { "angry_message_delay", GOD_RULE_ANGRY_MESSAGE_DELAY },
    { "tutorial_bast_mood", GOD_RULE_TUTORIAL_BAST_MOOD },
    { "bolts_max", GOD_RULE_BOLTS_MAX },
    { "minor_event_bolts", GOD_RULE_MINOR_EVENT_BOLTS },
    { "blessing_festival_months", GOD_RULE_BLESSING_FESTIVAL_MONTHS },
    { "curse_festival_months", GOD_RULE_CURSE_FESTIVAL_MONTHS },
    { "major_event_mood_change", GOD_RULE_MAJOR_EVENT_MOOD_CHANGE },
    { "minor_event_mood_change", GOD_RULE_MINOR_EVENT_MOOD_CHANGE },
    { "ra_effect_months", GOD_RULE_RA_EFFECT_MONTHS },
    { "seth_protection_months", GOD_RULE_SETH_PROTECTION_MONTHS },
    { "seth_enemies_to_leave", GOD_RULE_SETH_ENEMIES_TO_LEAVE },
    { "ra_major_kingdom_change", GOD_RULE_RA_MAJOR_KINGDOM_CHANGE },
    { "ra_minor_kingdom_change", GOD_RULE_RA_MINOR_KINGDOM_CHANGE },
    { "ptah_frogs", GOD_RULE_PTAH_FROGS },
    { 0, 0 },
};

static const config_field god_rule_fields[] = {
    { "value", god_rules, 1, 1, 1, 1000, 0 },
};

static const config_storage god_rule_storage[] = {
    { god_rules, sizeof(god_rules) },
};

const config_table god_rule_table = {
    "god_rules.json",
    "rule",
    god_rule_names,
    GOD_RULE_COUNT,
    CONFIG_TABLE_COLUMNS(god_rule_fields, god_rule_storage),
};

static const config_field god_bolts_fields[] = {
    { "happy_at_90", (int32_t *) god_bolts_tables + (GOD_BOLTS_MOOD_90 * DIFFICULTY_COUNT), 1, 1, 0, 50, 0 },
    { "happy_at_80", (int32_t *) god_bolts_tables + (GOD_BOLTS_MOOD_80 * DIFFICULTY_COUNT), 1, 1, 0, 50, 0 },
    { "happy_at_70", (int32_t *) god_bolts_tables + (GOD_BOLTS_MOOD_70 * DIFFICULTY_COUNT), 1, 1, 0, 50, 0 },
    { "happy_above_50", (int32_t *) god_bolts_tables + (GOD_BOLTS_MOOD_ABOVE_50 * DIFFICULTY_COUNT), 1, 1, 0, 50, 0 },
    { "at_50", (int32_t *) god_bolts_tables + (GOD_BOLTS_MOOD_50 * DIFFICULTY_COUNT), 1, 1, 0, 50, 0 },
    { "wrath_above_30", (int32_t *) god_bolts_tables + (GOD_BOLTS_MOOD_ABOVE_30 * DIFFICULTY_COUNT), 1, 1, 0, 50, 0 },
    { "wrath_above_20", (int32_t *) god_bolts_tables + (GOD_BOLTS_MOOD_ABOVE_20 * DIFFICULTY_COUNT), 1, 1, 0, 50, 0 },
    { "wrath_above_10", (int32_t *) god_bolts_tables + (GOD_BOLTS_MOOD_ABOVE_10 * DIFFICULTY_COUNT), 1, 1, 0, 50, 0 },
    { "wrath_at_10_or_less", (int32_t *) god_bolts_tables + (GOD_BOLTS_MOOD_10_OR_LESS * DIFFICULTY_COUNT), 1, 1, 0, 50, 0 },
};

static const config_storage god_bolts_storage[] = {
    { god_bolts_tables, sizeof(int32_t) * GOD_BOLTS_MOODS *DIFFICULTY_COUNT },
};

const config_table god_bolts_table = {
    "god_bolts.json",
    "level",
    config_difficulty_names,
    DIFFICULTY_COUNT,
    CONFIG_TABLE_COLUMNS(god_bolts_fields, god_bolts_storage),
};
