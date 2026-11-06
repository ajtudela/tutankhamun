// City gods: the daily moods (targets from temples, shrines and festivals, bounded by the population) and the bolts
// they give by the difficulty, the monthly blessings and curses, the timers of the god effects, and the blessings
// themselves (better flood or double yield, trade bonuses, filled storage yards or workshops, enemies sent away,
// filled houses and bazaars, a festival for all gods). The curses that destroy are in city/gods_curses.c.
//
// Odd but relied on:
// - the least happy god is never picked for the monthly blessing or curse (rand() % 5 is never above 4);
// - the pyramid message passes fifteen phrases of which only three are set;
// - the workshop blessing asks the capacity of building types it never passes, so every workshop gets up to 200.

#include "city/gods.h"

#include "building/building_data.h"
#include "building/building_figure_helpers.h"
#include "building/building_storage.h"
#include "building/building_type.h"
#include "building/house_evolve.h"
#include "building/pyramid_level.h"
#include "city/city_data.h"
#include "city/city_disasters.h"
#include "city/city_internal.h"
#include "city/culture.h"
#include "city/difficulty.h"
#include "city/kingdom.h"
#include "city/ratings.h"
#include "city/resource_type.h"
#include "flood/flood_internal.h"
#include "game/game_data.h"
#include "game/tutorial.h"
#include "map/map_data.h"
#include "window/window_data.h"

enum pyramid_speedup_phrase {
    PHRASE_PYRAMID_SPEEDUP_TEXT = 593,
    PHRASE_PYRAMID_SPEEDUP_MAJOR = 594,
    PHRASE_PYRAMID_SPEEDUP_MINOR = 595,
    PHRASE_PYRAMID_SPEEDUP_GOD_FIRST = 596,
};

int32_t god_rules[GOD_RULE_COUNT] = {
    [GOD_RULE_TUTORIAL_MOOD] = 30,
    [GOD_RULE_FESTIVAL_MONTHS_CAP] = 40,
    [GOD_RULE_FESTIVAL_MOOD_BONUS] = 12,
    [GOD_RULE_FIRST_MISSION_MIN_MOOD] = 50,
    [GOD_RULE_COMPLEX_GOD_PERCENTAGE] = 115,
    [GOD_RULE_PATRON_WITHOUT_COMPLEX_PERCENTAGE] = 98,
    [GOD_RULE_PATRON_BESIDE_COMPLEX_PERCENTAGE] = 90,
    [GOD_RULE_LOCAL_BESIDE_COMPLEX_PERCENTAGE] = 95,
    [GOD_RULE_ANGRY_MOOD] = 30,
    [GOD_RULE_ANGRY_MESSAGE_DELAY] = 20,
    [GOD_RULE_TUTORIAL_BAST_MOOD] = 40,
    [GOD_RULE_BOLTS_MAX] = 50,
    [GOD_RULE_MINOR_EVENT_BOLTS] = 20,
    [GOD_RULE_BLESSING_FESTIVAL_MONTHS] = 15,
    [GOD_RULE_CURSE_FESTIVAL_MONTHS] = 3,
    [GOD_RULE_MAJOR_EVENT_MOOD_CHANGE] = 30,
    [GOD_RULE_MINOR_EVENT_MOOD_CHANGE] = 12,
    [GOD_RULE_RA_EFFECT_MONTHS] = 12,
    [GOD_RULE_SETH_PROTECTION_MONTHS] = 10,
    [GOD_RULE_SETH_ENEMIES_TO_LEAVE] = 10,
    [GOD_RULE_RA_MAJOR_KINGDOM_CHANGE] = 15,
    [GOD_RULE_RA_MINOR_KINGDOM_CHANGE] = 5,
    [GOD_RULE_PTAH_FROGS] = 6,
};

enum god_mood_scale {
    GOD_MOOD_NEUTRAL = 50,
    GOD_MOOD_MAX = 100,
};

static int8_t scale_mood(int8_t mood, enum god_rule percentage)
{
    return (int8_t) (((mood * god_rules[percentage]) + 50) / 100);
}

static int8_t god_bolts(enum god_bolts_mood mood)
{
    const int32_t *bolts_by_mood_and_difficulty = god_bolts_tables;
    return (int8_t) bolts_by_mood_and_difficulty[(mood * DIFFICULTY_COUNT) + settings_data.difficulty];
}

static void discard_random_roll(void)
{
    crt_rand();
}

void city_gods_calculate_moods(int32_t update_moods)
{
    city_player *player = city_player_current();
    if (settings_data.is_custom_scenario == 0 && tutorial_flags_a[TUTORIAL_FLAG_GOLD_DELIVERED] == 0) {
        for (int32_t god = 0; god < GOD_COUNT; god++) {
            player->god_target_mood[god] = (int8_t) god_rules[GOD_RULE_TUTORIAL_MOOD];
            player->god_mood[god] = (int8_t) god_rules[GOD_RULE_TUTORIAL_MOOD];
        }
        return;
    }
    for (int32_t god = 0; god < GOD_COUNT; god++) {
        if (scenario_data.god_status[god]) {
            player->god_target_mood[god] = (int8_t) culture_coverage[CULTURE_COVERAGE_GOD + god];
        }
    }
    for (int32_t god = 0; god < GOD_COUNT; god++) {
        if (scenario_data.god_status[god]) {
            int32_t months = player->god_months_since_festival[god];
            if (months > god_rules[GOD_RULE_FESTIVAL_MONTHS_CAP]) {
                months = god_rules[GOD_RULE_FESTIVAL_MONTHS_CAP];
            }
            player->god_target_mood[god] = (int8_t) (player->god_target_mood[god] + (int8_t) (god_rules[GOD_RULE_FESTIVAL_MOOD_BONUS] - months));
        }
    }
    int32_t min_mood;
    int32_t max_mood = 50;
    if (player->population < 400) {
        min_mood = 50;
    } else if (player->population < 450) {
        min_mood = 40;
        max_mood = 60;
    } else if (player->population < 500) {
        min_mood = 30;
        max_mood = 70;
    } else if (player->population < 550) {
        min_mood = 20;
        max_mood = 80;
    } else if (player->population < 600) {
        min_mood = 10;
        max_mood = 90;
    } else {
        min_mood = 0;
        max_mood = 100;
    }
    for (int32_t god = 0; god < GOD_COUNT; god++) {
        if (scenario_data.god_status[god]) {
            if (player->god_target_mood[god] > GOD_MOOD_MAX) {
                player->god_target_mood[god] = GOD_MOOD_MAX;
            }
            if (player->god_target_mood[god] < min_mood) {
                player->god_target_mood[god] = (int8_t) min_mood;
            }
            if (player->god_target_mood[god] > max_mood) {
                player->god_target_mood[god] = (int8_t) max_mood;
            }
        }
    }
    if (!update_moods) {
        return;
    }
    int8_t campaign_setting = (int8_t) settings_data.campaign;
    int32_t custom = settings_data.is_custom_scenario;
    for (int32_t god = 0; god < GOD_COUNT; god++) {
        if (!scenario_data.god_status[god]) {
            continue;
        }
        if (player->god_mood[god] > player->god_target_mood[god]) {
            player->god_mood[god]--;
        } else if (player->god_mood[god] < player->god_target_mood[god]) {
            player->god_mood[god]++;
        }
        if (settings_data.mission_id == 0 && custom == 0 && campaign_setting == 0 && player->god_mood[god] < god_rules[GOD_RULE_FIRST_MISSION_MIN_MOOD]) {
            player->god_mood[god] = (int8_t) god_rules[GOD_RULE_FIRST_MISSION_MIN_MOOD];
        }
    }
    int32_t god = crt_rand() % 5;
    if (god < GOD_COUNT && scenario_data.god_status[god]) {
        int8_t mood = player->god_mood[god];
        if (mood >= 90) {
            player->god_happy_bolts[god] += god_bolts(GOD_BOLTS_MOOD_90);
            player->god_wrath_bolts[god] = 0;
        } else if (mood >= 80) {
            player->god_happy_bolts[god] += god_bolts(GOD_BOLTS_MOOD_80);
            player->god_wrath_bolts[god] = 0;
        } else if (mood >= 70) {
            player->god_happy_bolts[god] += god_bolts(GOD_BOLTS_MOOD_70);
            player->god_wrath_bolts[god] = 0;
        } else if (mood > GOD_MOOD_NEUTRAL) {
            player->god_happy_bolts[god] += god_bolts(GOD_BOLTS_MOOD_ABOVE_50);
            player->god_wrath_bolts[god] = 0;
        } else if (mood == GOD_MOOD_NEUTRAL) {
            player->god_wrath_bolts[god] = 0;
        } else {
            player->god_happy_bolts[god] = 0;
            if (mood > 30) {
                player->god_wrath_bolts[god] += god_bolts(GOD_BOLTS_MOOD_ABOVE_30);
            } else if (mood > 20) {
                player->god_wrath_bolts[god] += god_bolts(GOD_BOLTS_MOOD_ABOVE_20);
            } else if (mood > 10) {
                player->god_wrath_bolts[god] += god_bolts(GOD_BOLTS_MOOD_ABOVE_10);
            } else {
                player->god_wrath_bolts[god] += god_bolts(GOD_BOLTS_MOOD_10_OR_LESS);
            }
        }
        if (player->god_wrath_bolts[god] >= god_rules[GOD_RULE_BOLTS_MAX]) {
            player->god_wrath_bolts[god] = (int8_t) god_rules[GOD_RULE_BOLTS_MAX];
        }
        if (player->god_happy_bolts[god] >= god_rules[GOD_RULE_BOLTS_MAX]) {
            player->god_happy_bolts[god] = (int8_t) god_rules[GOD_RULE_BOLTS_MAX];
        }
    }
    for (int32_t i = 0; i < GOD_COUNT; i++) {
        if (player->god_mood[i] < GOD_MOOD_NEUTRAL) {
            player->god_happy_bolts[i] = 0;
        }
        if (player->god_mood[i] > GOD_MOOD_NEUTRAL) {
            player->god_wrath_bolts[i] = 0;
        }
    }
    if (settings_data.is_custom_scenario == 0 && settings_data.mission_id == tutorial_briefing_tables.tutorial.entries[TUTORIAL_FLAG_GODS_PLEASED].mission && tutorial_flags_a[TUTORIAL_FLAG_GODS_PLEASED] == 0 && tutorial_flags_a[TUTORIAL_FLAG_GOLD_DELIVERED] != 0 &&
        player->god_mood[GOD_BAST] >= god_rules[GOD_RULE_TUTORIAL_BAST_MOOD]) {
        tutorial_set_flag(TUTORIAL_FLAG_GODS_PLEASED);
    }
    if (game_time_day != 0) {
        return;
    }
    city_gods_decrease_effect_timers();
    if (player->has_temple_complex == 0) {
        for (int32_t i = 0; i < GOD_COUNT; i++) {
            if (scenario_data.god_status[i] && scenario_data.god_status[i] == GOD_STATUS_PATRON) {
                player->god_target_mood[i] = scale_mood(player->god_target_mood[i], GOD_RULE_PATRON_WITHOUT_COMPLEX_PERCENTAGE);
            }
        }
    } else {
        for (int32_t i = 0; i < GOD_COUNT; i++) {
            if (!scenario_data.god_status[i]) {
                continue;
            }
            int32_t complex_type = building_type_for_god(i, GOD_BUILDING_TEMPLE_COMPLEX);
            if (building_get(player->temple_complex_building_id)->type == complex_type) {
                player->god_target_mood[i] = scale_mood(player->god_target_mood[i], GOD_RULE_COMPLEX_GOD_PERCENTAGE);
            } else if (scenario_data.god_status[i] == GOD_STATUS_PATRON) {
                player->god_target_mood[i] = scale_mood(player->god_target_mood[i], GOD_RULE_PATRON_BESIDE_COMPLEX_PERCENTAGE);
            } else if (scenario_data.god_status[i] == GOD_STATUS_LOCAL) {
                player->god_target_mood[i] = scale_mood(player->god_target_mood[i], GOD_RULE_LOCAL_BESIDE_COMPLEX_PERCENTAGE);
            }
        }
    }
    for (int32_t i = 0; i < GOD_COUNT; i++) {
        if (scenario_data.god_status[i]) {
            if (player->god_target_mood[i] > GOD_MOOD_MAX) {
                player->god_target_mood[i] = GOD_MOOD_MAX;
            }
            if (player->god_target_mood[i] < min_mood) {
                player->god_target_mood[i] = (int8_t) min_mood;
            }
        }
    }
    for (int32_t i = 0; i < GOD_COUNT; i++) {
        if (scenario_data.god_status[i]) {
            player->god_months_since_festival[i]++;
        }
    }
    if (god >= GOD_COUNT && city_gods_find_least_happy()) {
        god = player->least_happy_god - 1;
    }
    if (settings_data.gods_enabled) {
        discard_random_roll();
        city_gods_perform_blessing_or_curse(god, GOD_EVENT_RANDOM);
        int32_t lowest = GOD_MOOD_MAX;
        for (int32_t i = 0; i < GOD_COUNT; i++) {
            if (scenario_data.god_status[i] && player->god_mood[i] < lowest) {
                lowest = player->god_mood[i];
            }
        }
        if (player->gods_angry_message_delay) {
            player->gods_angry_message_delay--;
        } else if (lowest < god_rules[GOD_RULE_ANGRY_MOOD]) {
            player->gods_angry_message_delay = god_rules[GOD_RULE_ANGRY_MESSAGE_DELAY];
        }
    }
}

static void post_speedup_message(int32_t god, int16_t first)
{
    int16_t phrases[15] = { 0 };
    phrases[0] = first;
    phrases[1] = PHRASE_PYRAMID_SPEEDUP_TEXT;
    if (god >= 0 && god < GOD_COUNT) {
        phrases[8] = (int16_t) (PHRASE_PYRAMID_SPEEDUP_GOD_FIRST + god);
    }
    city_message_use_popup = 1;
    city_message_post(MESSAGE_TEMPLATE_GENERAL, 0, 0, phrases, 0);
}

void city_gods_perform_blessing_or_curse(int32_t god, int32_t force)
{
    city_player *player = city_player_current();
    if (god >= GOD_COUNT || !scenario_data.god_status[god]) {
        return;
    }
    int32_t by_bolts = (force == GOD_EVENT_RANDOM);
    int32_t festival_recent = player->god_months_since_festival[god] < god_rules[GOD_RULE_BLESSING_FESTIVAL_MONTHS];
    int32_t festival_long_ago = player->god_months_since_festival[god] > god_rules[GOD_RULE_CURSE_FESTIVAL_MONTHS];
    int8_t happy = player->god_happy_bolts[god];
    if ((by_bolts && happy == god_rules[GOD_RULE_BOLTS_MAX] && festival_recent) || force == GOD_EVENT_MAJOR_BLESSING) {
        player->god_happy_bolts[god] = 0;
        city_message_use_popup = 1;
        player->god_mood[god] = (int8_t) (player->god_mood[god] - god_rules[GOD_RULE_MAJOR_EVENT_MOOD_CHANGE]);
        if (pyramid_speedup_level() != 0 && (crt_rand() % 2) == 0) {
            post_speedup_message(god, PHRASE_PYRAMID_SPEEDUP_MAJOR);
            pyramid_speedup_start((pyramid_speedup_level() == 2) ? 2 : 1);
            return;
        }
        switch (god) {
            case GOD_OSIRIS:
                if ((crt_rand() % 2) == 0) {
                    city_message_post(MESSAGE_BLESSING_FROM_OSIRIS, 0, 0, NULL, 0);
                    player->osiris_double_farm_yield = 1;
                } else {
                    city_message_post(MESSAGE_BLESSING_INUNDATION_FROM_OSIRIS, 0, 0, NULL, 0);
                    floods_adjust_next_quality(&floods_data.settings, (((crt_rand() % 3) * 5) + 10) * 2);
                }
                return;
            case GOD_RA:
                if ((crt_rand() % 2) == 0) {
                    city_message_post(MESSAGE_BLESSING_TRADE_FROM_RA, 0, 0, NULL, 0);
                    player->ra_exports_bonus_months = god_rules[GOD_RULE_RA_EFFECT_MONTHS];
                } else {
                    city_message_post(MESSAGE_BLESSING_REPUTATION_FROM_RA, 0, 0, NULL, 0);
                    city_ratings_change_kingdom(god_rules[GOD_RULE_RA_MAJOR_KINGDOM_CHANGE]);
                }
                return;
            case GOD_PTAH:
                if (city_gods_ptah_fill_storage_yard()) {
                    city_message_post(MESSAGE_BLESSING_TRADE_FROM_PTAH, 0, 0, NULL, 0);
                } else {
                    city_message_post(MESSAGE_BLESSING_FROM_PTAH, 0, 0, NULL, 0);
                }
                return;
            case GOD_SETH:
                city_message_post(MESSAGE_BLESSING_TRADE_FROM_SETH, 0, 0, NULL, 0);
                player->enemies_to_leave = god_rules[GOD_RULE_SETH_ENEMIES_TO_LEAVE];
                return;
            case GOD_BAST:
                city_message_post(MESSAGE_BLESSING_FROM_BAST, 0, 0, NULL, 0);
                city_gods_bast_major_blessing();
                return;
        }
        return;
    }
    if ((by_bolts && happy >= god_rules[GOD_RULE_MINOR_EVENT_BOLTS] && festival_recent) || force == GOD_EVENT_MINOR_BLESSING) {
        player->god_happy_bolts[god] = 0;
        city_message_use_popup = 1;
        player->god_mood[god] = (int8_t) (player->god_mood[god] - god_rules[GOD_RULE_MINOR_EVENT_MOOD_CHANGE]);
        if (pyramid_speedup_level() == 2 && (crt_rand() % 2) == 0) {
            post_speedup_message(god, PHRASE_PYRAMID_SPEEDUP_MINOR);
            pyramid_speedup_start(1);
            return;
        }
        switch (god) {
            case GOD_OSIRIS:
                city_message_post(MESSAGE_SMALL_BLESSING_FROM_OSIRIS, 0, 0, NULL, 0);
                floods_adjust_next_quality(&floods_data.settings, ((crt_rand() % 4) * 5) + 5);
                return;
            case GOD_RA:
                if ((crt_rand() % 2) == 0) {
                    city_message_post(MESSAGE_MINOR_BLESSING_TRADING_FROM_RA, 0, 0, NULL, 0);
                    player->ra_trade_bonus_months = (int16_t) god_rules[GOD_RULE_RA_EFFECT_MONTHS];
                } else {
                    city_message_post(MESSAGE_MINOR_BLESSING_FROM_RA, 0, 0, NULL, 0);
                    city_ratings_change_kingdom(god_rules[GOD_RULE_RA_MINOR_KINGDOM_CHANGE]);
                }
                return;
            case GOD_PTAH:
                city_message_post(MESSAGE_MINOR_BLESSING_FROM_PTAH, 0, 0, NULL, 0);
                city_gods_ptah_minor_blessing();
                return;
            case GOD_SETH:
                city_message_post(MESSAGE_MINOR_BLESSING_FROM_SETH, 0, 0, NULL, 0);
                player->seth_minor_blessing_months = god_rules[GOD_RULE_SETH_PROTECTION_MONTHS];
                return;
            case GOD_BAST:
                city_message_post(MESSAGE_SMALL_BLESSING_FROM_BAST, 0, 0, NULL, 0);
                player->festival_planned_size = FESTIVAL_ALL_GODS;
                player->festival_planned_months_to_go = 1;
                player->first_festival_effect_months = 1;
                player->festival_planned_god = 0;
                city_festival_update();
                player->god_months_since_festival[1] = 0;
                player->god_months_since_festival[2] = 0;
                player->god_months_since_festival[3] = 0;
                return;
        }
        return;
    }
    int8_t wrath = player->god_wrath_bolts[god];
    if ((by_bolts && wrath == god_rules[GOD_RULE_BOLTS_MAX] && festival_long_ago) || force == GOD_EVENT_MAJOR_CURSE) {
        player->god_wrath_bolts[god] = 0;
        player->god_mood[god] = (int8_t) (player->god_mood[god] + god_rules[GOD_RULE_MAJOR_EVENT_MOOD_CHANGE]);
        city_message_use_popup = 1;
        switch (god) {
            case GOD_OSIRIS:
                if ((crt_rand() % 2) == 0) {
                    city_message_post(MESSAGE_WRATH_OF_OSIRIS, 0, 0, NULL, 0);
                    floods_adjust_next_quality(&floods_data.settings, (-2 - (crt_rand() % 3)) * 10);
                } else {
                    city_message_post(MESSAGE_WRATH_OF_OSIRIS_2, 0, 0, NULL, 0);
                    city_locusts_start();
                }
                return;
            case GOD_RA:
                if ((crt_rand() % 3) == 0) {
                    city_message_post(MESSAGE_WRATH_OF_RA_2, 0, 0, NULL, 0);
                    player->ra_trade_penalty_months = (int16_t) god_rules[GOD_RULE_RA_EFFECT_MONTHS];
                } else if ((crt_rand() % 3) == 1) {
                    city_message_post(MESSAGE_WRATH_OF_RA, 0, 0, NULL, 0);
                    city_ratings_change_kingdom(-god_rules[GOD_RULE_RA_MAJOR_KINGDOM_CHANGE]);
                } else {
                    city_message_post(MESSAGE_WRATH_OF_RA_3, 0, 0, NULL, 0);
                    player->ra_no_traders_months = god_rules[GOD_RULE_RA_EFFECT_MONTHS];
                }
                return;
            case GOD_PTAH:
                if ((crt_rand() % 2) == 0) {
                    if (city_gods_ptah_major_curse()) {
                        city_message_post(MESSAGE_WRATH_OF_PTAH_2, 0, 0, NULL, 0);
                    } else {
                        city_message_post(MESSAGE_WRATH_OF_PTAH, 0, 0, NULL, 0);
                    }
                } else {
                    city_message_post(MESSAGE_WRATH_OF_PTAH_4, 0, 0, NULL, 0);
                    city_frogs_start(god_rules[GOD_RULE_PTAH_FROGS]);
                }
                return;
            case GOD_SETH:
                if ((crt_rand() % 2) == 0) {
                    city_message_post(MESSAGE_WRATH_OF_SETH_2, 0, 0, NULL, 0);
                    figure_sink_all_ships();
                } else {
                    city_message_post(MESSAGE_HAILSTORM_WRATH_OF_SETH, 0, 0, NULL, 0);
                    city_hailstorm_start();
                }
                return;
            case GOD_BAST:
                city_message_post(MESSAGE_WRATH_OF_BAST, 0, 0, NULL, 0);
                city_gods_bast_major_curse();
                return;
        }
        return;
    }
    if ((by_bolts && wrath >= god_rules[GOD_RULE_MINOR_EVENT_BOLTS] && festival_long_ago) || force == GOD_EVENT_MINOR_CURSE) {
        player->god_wrath_bolts[god] = 0;
        city_message_use_popup = 1;
        player->god_mood[god] = (int8_t) (player->god_mood[god] + god_rules[GOD_RULE_MINOR_EVENT_MOOD_CHANGE]);
        switch (god) {
            case GOD_OSIRIS:
                if ((crt_rand() % 2) == 0) {
                    city_message_post(MESSAGE_OSIRIS_IS_UPSET, 0, 0, NULL, 0);
                    player->osiris_flood_destroys_farms = 1;
                } else {
                    city_message_post(MESSAGE_WRATH_OF_OSIRIS_4, 0, 0, NULL, 0);
                    floods_adjust_next_quality(&floods_data.settings, (-1 - (crt_rand() % 4)) * 5);
                }
                return;
            case GOD_RA:
                if ((crt_rand() % 2) == 0) {
                    city_message_post(MESSAGE_RA_IS_UPSET, 0, 0, NULL, 0);
                    player->ra_trade_minor_penalty_months = (int16_t) god_rules[GOD_RULE_RA_EFFECT_MONTHS];
                } else {
                    city_message_post(MESSAGE_RA_IS_UPSET_2, 0, 0, NULL, 0);
                    city_ratings_change_kingdom(-god_rules[GOD_RULE_RA_MINOR_KINGDOM_CHANGE]);
                }
                return;
            case GOD_PTAH:
                if (building_count_total[BUILDING_STORAGE_YARD_UP] + building_count_total[BUILDING_STORAGE_YARD] != 0) {
                    city_message_post(MESSAGE_PTAH_IS_UPSET, 0, 0, NULL, 0);
                    city_gods_ptah_minor_curse(1);
                } else {
                    city_message_post(MESSAGE_WRATH_OF_PTAH, 0, 0, NULL, 0);
                }
                return;
            case GOD_SETH:
                if (city_gods_seth_minor_curse()) {
                    city_message_post(MESSAGE_SETH_IS_UPSET, 0, 0, NULL, 0);
                } else {
                    city_message_post(MESSAGE_WRATH_OF_SETH_NOEFFECT, 0, 0, NULL, 0);
                }
                return;
            case GOD_BAST:
                city_message_post(MESSAGE_BAST_IS_UPSET, 0, 0, NULL, 0);
                city_health_start_plague(1);
                return;
        }
    }
}

void city_gods_decrease_effect_timers(void)
{
    city_player *player = city_player_current();
    if (player->ra_trade_bonus_months >= 0) {
        player->ra_trade_bonus_months--;
    }
    if (player->ra_trade_penalty_months >= 0) {
        player->ra_trade_penalty_months--;
    }
    if (player->ra_trade_minor_penalty_months >= 0) {
        player->ra_trade_minor_penalty_months--;
    }
    if (player->ra_exports_bonus_months >= 0) {
        player->ra_exports_bonus_months--;
    }
    if (player->ra_no_traders_months >= 0) {
        player->ra_no_traders_months--;
    }
}

int32_t city_gods_ra_trade_modifier(void)
{
    const city_player *player = city_player_current();
    int32_t modifier = player->ra_trade_bonus_months > 0;
    if (player->ra_trade_penalty_months > 0) {
        modifier -= 2;
    }
    if (player->ra_trade_minor_penalty_months > 0) {
        modifier -= 1;
    }
    return modifier;
}

int32_t city_gods_ra_adjust_percentage(int32_t percentage)
{
    int32_t modifier = city_gods_ra_trade_modifier();
    if (percentage == 0) {
        return 0;
    }
    if (modifier == 1) {
        if (percentage == 25) {
            return 40;
        }
        if (percentage == 15) {
            return 25;
        }
        return (percentage > 14) ? percentage : 15;
    }
    if (modifier > 1) {
        if (percentage == 25) {
            return 40;
        }
        if (percentage == 15) {
            return 40;
        }
        return (percentage > 14) ? percentage : 25;
    }
    if (modifier == -1) {
        if (percentage == 40) {
            return 25;
        }
        if (percentage == 25) {
            return 15;
        }
        if (percentage == 15) {
            return 0;
        }
        return percentage;
    }
    if (modifier > -2) {
        return percentage;
    }
    if (percentage == 40) {
        return 15;
    }
    if (percentage != 25 && percentage != 15) {
        return percentage;
    }
    return 0;
}

int32_t city_gods_with_status(int32_t status)
{
    for (int32_t god = 0; god < GOD_COUNT; god++) {
        if (scenario_data.god_status[god] == status) {
            return god;
        }
    }
    return -1;
}

int32_t city_gods_find_least_happy(void)
{
    city_player *player = city_player_current();
    int32_t found = 0;
    int8_t max_wrath = 0;
    for (int32_t god = 0; god < GOD_COUNT; god++) {
        if (scenario_data.god_status[god] && player->god_wrath_bolts[god] > max_wrath) {
            found = god + 1;
            max_wrath = player->god_wrath_bolts[god];
        }
    }
    if (found > 0) {
        player->least_happy_god = found;
        return 1;
    }
    int8_t min_mood = 40;
    for (int32_t god = 0; god < GOD_COUNT; god++) {
        if (scenario_data.god_status[god] && player->god_mood[god] < min_mood) {
            found = god + 1;
            min_mood = player->god_mood[god];
        }
    }
    player->least_happy_god = found;
    return found > 0;
}

static const int32_t storage_yard_goods[6] = { RESOURCE_GEMS, RESOURCE_CLAY, RESOURCE_POTTERY, RESOURCE_FLAX, RESOURCE_LINEN, RESOURCE_LUXURY_GOODS };

int32_t city_gods_ptah_fill_storage_yard(void)
{
    int32_t least = 10000;
    int32_t granary = 0;
    for (int32_t id = 1; id < MAX_BUILDINGS; id++) {
        building *b = building_get(id);
        if (b->state != BUILDING_STATE_IN_USE || (b->type != BUILDING_STORAGE_YARD && b->type != BUILDING_STORAGE_YARD_UP)) {
            continue;
        }
        int32_t total = 0;
        for (int32_t i = 0; i < 6; i++) {
            total += building_storageyard_get_amount(id, storage_yard_goods[i]);
        }
        if (total != 0 && total < least) {
            least = total;
            granary = id;
        }
    }
    if (granary == 0 || least == 0) {
        return granary;
    }
    int32_t food = 0;
    int32_t amount = 10000;
    for (int32_t i = 0; i < 6; i++) {
        int32_t stored = building_storageyard_get_amount(granary, storage_yard_goods[i]);
        if (stored != 0 && stored < amount) {
            amount = stored;
            food = storage_yard_goods[i];
        }
    }
    if (food != 0) {
        int32_t room = storage_get(building_get(granary)->storage_id)->accept_amount[food] - amount;
        building_storageyard_add_resource(city_current_player, granary, food, (room < 0) ? 0 : room);
    }
    return granary;
}

static int32_t blessing_capacity(int32_t type)
{
    switch (type) {
        case BUILDING_SENET_HOUSE:
        case BUILDING_SCRIBAL_SCHOOL:
            return 300;
        case BUILDING_MORTUARY:
        case BUILDING_LIBRARY:
            return 500;
        case BUILDING_VILLAGE_PALACE:
        case BUILDING_TOWN_PALACE:
        case BUILDING_TAX_COLLECTOR:
        case BUILDING_TAX_COLLECTOR_UP:
        case BUILDING_COURTHOUSE:
        case BUILDING_VILLAGE_PALACE_UP:
        case BUILDING_TOWN_PALACE_UP:
        case BUILDING_CITY_PALACE:
            return 100000;
        default:
            return 200;
    }
}

static int32_t blessing_resource(int32_t subtype)
{
    static const int8_t resources[27] = { -1, RESOURCE_FLAX, -1, RESOURCE_STONE, RESOURCE_TIMBER, RESOURCE_CLAY, RESOURCE_LIMESTONE, RESOURCE_BRICKS,
        RESOURCE_WEAPONS, RESOURCE_BARLEY, RESOURCE_GEMS, RESOURCE_REEDS, RESOURCE_PAPYRUS, RESOURCE_STRAW, RESOURCE_CHARIOTS, RESOURCE_GOLD, RESOURCE_LINEN,
        RESOURCE_COPPER, RESOURCE_SANDSTONE, -1, RESOURCE_HENNA, RESOURCE_MEAT, RESOURCE_GAMEMEAT, RESOURCE_OIL, RESOURCE_POTTERY, RESOURCE_PAINT,
        RESOURCE_LAMPS };
    return (subtype >= 1 && subtype <= 26) ? resources[subtype] : -1;
}

int32_t city_gods_ptah_minor_blessing(void)
{
    int32_t types[3] = { BUILDING_SHIPWRIGHT, BUILDING_WEAVER, BUILDING_JEWELER };
    static const int32_t industries[3] = { -1, RESOURCE_LINEN, RESOURCE_LUXURY_GOODS };
    int32_t count = 0;
    for (int32_t i = 0; i < 3; i++) {
        if (building_count_total[types[i]] > 0) {
            count++;
        } else if (industries[i] > 0 && building_count_industry_active[industries[i]] > 0) {
            count++;
        } else {
            types[i] = 0;
        }
    }
    if (count == 0) {
        return 0;
    }
    int32_t pick = crt_rand() % count;
    int32_t k = 0;
    for (int32_t i = 0; i < 3; i++) {
        if (types[i] == 0) {
            continue;
        }
        if (k != pick) {
            k++;
            continue;
        }
        int32_t type = types[i];
        for (int32_t id = 1; id < MAX_BUILDINGS; id++) {
            building *b = building_get(id);
            if (b->state != BUILDING_STATE_IN_USE || b->type != type) {
                continue;
            }
            int32_t amount = blessing_capacity(type) - b->loads_stored;
            if (amount < 0) {
                amount = 0;
            }
            building_workshop_add_input(id, amount, blessing_resource(b->subtype));
        }
        return 1;
    }
    return 0;
}

static int32_t bazaar_slot(int32_t resource)
{
    switch (resource) {
        case RESOURCE_GRAIN:
            return 0;
        case RESOURCE_MEAT:
            return 1;
        case RESOURCE_LETTUCE:
            return 2;
        case RESOURCE_CHICKPEAS:
            return 3;
        case RESOURCE_POMEGRANATES:
            return 4;
        case RESOURCE_FIGS:
            return 5;
        case RESOURCE_FISH:
            return 6;
        case RESOURCE_GAMEMEAT:
            return 7;
        case RESOURCE_POTTERY:
            return 9;
        case RESOURCE_BEER:
            return 8;
        case RESOURCE_LINEN:
            return 10;
        case RESOURCE_LUXURY_GOODS:
            return 11;
        default:
            return -1;
    }
}

static int32_t house_goods_consumption(const house_model *model, int32_t slot)
{
    switch (slot) {
        case INVENTORY_BEER:
            return model->beer;
        case INVENTORY_POTTERY:
            return model->pottery;
        case INVENTORY_LINEN:
            return model->linen;
        default:
            return model->jewelry;
    }
}

void city_gods_bast_major_blessing(void)
{
    for (int32_t id = 0; id < MAX_BUILDINGS; id++) {
        building *b = building_get(id);
        if (b->state != BUILDING_STATE_IN_USE) {
            continue;
        }
        int32_t type = b->type;
        if (!((type >= BUILDING_HOUSE_CRUDE_HUT && type <= BUILDING_HOUSE_PALATIAL_ESTATE) || type == BUILDING_BAZAAR)) {
            continue;
        }
        int16_t *inventory = b->data.house.inventory;
        if (type >= BUILDING_HOUSE_CRUDE_HUT && type <= BUILDING_HOUSE_PALATIAL_ESTATE) {
            for (int32_t slot = 0; slot < 12; slot++) {
                if (inventory[slot] == 0) {
                    continue;
                }
                if (slot < 8) {
                    inventory[slot] = (int16_t) (b->house_population * 6);
                    continue;
                }
                int32_t level = b->subtype;
                int32_t value = house_goods_consumption(&house_models[level], slot);
                inventory[slot] = (int16_t) ((b->size > 1) ? (value * 8) : (value * 2));
            }
            continue;
        }
        int16_t *goods = b->data.market.inventory;
        for (int32_t i = 0; i < city_resources.count; i++) {
            int32_t resource = city_resources.list[i];
            int32_t slot = bazaar_slot(resource);
            if (!resource_is_bazaar_good(resource) || goods[slot] == 0) {
                continue;
            }
            int32_t amount = (resource >= RESOURCE_MIN_FOOD && resource <= RESOURCE_GAMEMEAT) ? ((resource != RESOURCE_GRAIN) ? 600 : 800) : 400;
            if ((goods[slot] * 2) > amount) {
                amount = goods[slot] * 2;
            }
            if (amount >= 20000) {
                amount = 20000;
            }
            goods[slot] = (int16_t) amount;
        }
    }
}
