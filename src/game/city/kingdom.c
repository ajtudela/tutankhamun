// The kingdom side of the city: the bailout paid once when the treasury goes negative and the kingdom army at rating
// 0, the monthly festival countdown and its effect on the houses and the gods, and the victory and defeat checks of
// the mission every tick, with the messages, windows and video that follow.

#include "city/kingdom.h"

#include "building/building_figure_helpers.h"
#include "building/building_type.h"
#include "building/house_evolve.h"
#include "building/monument.h"
#include "city/city_data.h"
#include "city/city_init.h"
#include "city/city_internal.h"
#include "city/city_player.h"
#include "city/difficulty.h"
#include "city/finance.h"
#include "city/finance_estimates.h"
#include "city/gods.h"
#include "city/sentiment.h"
#include "game/game_data.h"
#include "game/tutorial.h"
#include "graphics/graphics_data.h"
#include "input/input_data.h"
#include "scenario/scenario_internal.h"
#include "video/smacker.h"
#include "video/video_data.h"
#include "widget/widget_data.h"
#include "window/intermezzo.h"
#include "window/mission_flow.h"
#include "window/window_data.h"
#include "window/window_id.h"

#include <stdbool.h>

const char victory_video_senate[24] = "smk\\victory_senate.smk";

const char victory_video_balcony[24] = "smk\\victory_balcony.smk";

const char victory_video_win_game[20] = "smk\\win_game.smk";

void city_emperor_update(void)
{
    city_player *player = city_player_current();
    if (player->treasury >= 0) {
        player->months_in_debt = 0;
        player->years_in_debt = 0;
        player->bailout_refused_message_shown = 0;
    } else if (player->bailout_taken == 0) {
        int32_t bailout = calc_adjust_with_percentage(scenario_data.bailout_amount, difficulty_treasury_percentage[settings_data.difficulty]);
        player->treasury += bailout;
        player->donated_this_year += bailout;
        city_finance_calculate_totals();
        city_finance_calculate_tribute();
        player->bailout_taken = 1;
        if (scenario_data.bailout_amount > 0) {
            city_message_use_popup = 1;
            if (player->player_rank != PLAYER_RANK_PHARAOH) {
                city_message_post(MESSAGE_OUT_OF_MONEY, 0, 0, NULL, 0);
                city_ratings_reduce_prosperity_after_bailout();
            } else {
                city_message_post(MESSAGE_OUT_OF_MONEY_AGAIN, 0, 0, NULL, 0);
            }
        }
    } else if (player->bailout_refused_message_shown == 0) {
        player->bailout_refused_message_shown = 1;
        city_message_use_popup = 1;
        city_message_post(MESSAGE_DEBT_AGAIN, 0, 0, NULL, 0);
    }
    if (player->rating_kingdom != 0) {
        return;
    }
    scenario_event *event = scenario_invasion_next(&scenario_events, NULL, 0);
    if (event == NULL) {
        return;
    }
    while ((event->flags & EVENT_FLAG_BY_FAVOUR) == 0) {
        event = scenario_invasion_next(&scenario_events, event, 0);
        if (event == NULL) {
            return;
        }
    }
    if ((event->flags & EVENT_FLAG_FIRED) != 0) {
        return;
    }
    event->year.fixed = 0;
    event->year.min = -1;
    event->year.max = -1;
    scenario_event_chain(event, event->id, (event->item.value != 3) ? 181 : 171);
    scenario_invasion_called_off(event);
    event->month = MONTHS_PER_YEAR;
}

void city_festival_update(void)
{
    city_player *player = city_player_current();
    if (player->first_festival_effect_months != 0) {
        player->first_festival_effect_months--;
    }
    if (player->second_festival_effect_months != 0) {
        player->second_festival_effect_months--;
    }
    if (player->festival_planned_size <= 0) {
        return;
    }
    if (--player->festival_planned_months_to_go > 0) {
        return;
    }
    if (player->first_festival_effect_months <= 0) {
        switch (player->festival_planned_size) {
            case FESTIVAL_SMALL:
                city_sentiment_change_happiness(6);
                player->god_mood[player->festival_planned_god] += 1;
                break;
            case FESTIVAL_LARGE:
                city_sentiment_change_happiness(10);
                player->god_mood[player->festival_planned_god] += 2;
                break;
            case FESTIVAL_GRAND:
                city_sentiment_change_happiness(15);
                player->god_mood[player->festival_planned_god] += 3;
                break;
            case FESTIVAL_ALL_GODS:
                city_sentiment_change_happiness(15);
                for (int32_t god = 0; god < GOD_COUNT; god++) {
                    if (god != GOD_BAST) {
                        player->god_mood[god] += 3;
                    }
                }
                break;
        }
    } else if (player->second_festival_effect_months <= 0) {
        int32_t happiness = 0;
        switch (player->festival_planned_size) {
            case FESTIVAL_SMALL:
                happiness = 2;
                player->god_mood[player->festival_planned_god] += 1;
                break;
            case FESTIVAL_LARGE:
                happiness = 3;
                player->god_mood[player->festival_planned_god] += 2;
                break;
            case FESTIVAL_GRAND:
                happiness = 5;
                player->god_mood[player->festival_planned_god] += 5;
                break;
        }
        if (happiness) {
            city_sentiment_change_happiness(happiness);
        }
    }
    city_message_use_popup = 1;
    player->months_since_festival = 1;
    player->god_months_since_festival[player->festival_planned_god] = 0;
    switch (player->festival_planned_size) {
        case FESTIVAL_SMALL:
            city_message_post(MESSAGE_COMMON_FESTIVAL, 0, 0, NULL, 0);
            break;
        case FESTIVAL_LARGE:
            city_message_post(MESSAGE_LAVISH_FESTIVAL, 0, 0, NULL, 0);
            break;
        case FESTIVAL_GRAND:
            city_message_post(MESSAGE_GRAND_FESTIVAL, 0, 0, NULL, 0);
            break;
    }
    building_festival_spawn_celebrants(player->festival_planned_size * CELEBRANTS_PER_FESTIVAL_SIZE);
    player->festival_planned_size = 0;
    player->festival_planned_months_to_go = 0;
}

void city_victory_update_months_to_govern(void)
{
    city_player *player = city_player_current();
    if (player->mission_has_won) {
        player->mission_continue_months_left--;
    }
}

int32_t city_victory_count_houses_of_level(int32_t min_level)
{
    int32_t count = 0;
    for (int32_t id = 1; id < MAX_BUILDINGS; id++) {
        building *b = building_get(id);
        if (b->state != 0 && b->house_size != 0 && b->house_population != 0 && min_level <= b->type - BUILDING_HOUSE_CRUDE_HUT) {
            count++;
        }
    }
    return count;
}

bool city_victory_goals_reached(void)
{
    for (int32_t resource = RESOURCE_MIN; resource <= BURIAL_GOAL_RESOURCES; resource++) {
        if (city_burial_goal_divisor(resource) == 0) {
            continue;
        }
        if (scenario_data.provisions_sent[resource] < scenario_data.provisions_needed[resource]) {
            return 0;
        }
    }
    return 1;
}

void city_victory_check(void)
{
    city_player *player = city_player_current();
    int32_t goals = 0;
    city_lost_by_population = 0;
    if (scenario_data.open_play != 0) {
        return;
    }
    city_victory_state = VICTORY_STATE_WON;
    if (scenario_data.win_culture_enabled) {
        goals = 1;
        if (player->rating_culture < scenario_data.win_culture) {
            city_victory_state = VICTORY_STATE_NONE;
        }
    }
    if (scenario_data.win_prosperity_enabled) {
        goals++;
        if (player->rating_prosperity < scenario_data.win_prosperity) {
            city_victory_state = VICTORY_STATE_NONE;
        }
    }
    if (scenario_data.win_population_enabled) {
        goals++;
        if (player->population < scenario_data.win_population) {
            city_victory_state = VICTORY_STATE_NONE;
        }
    }
    if (scenario_data.win_monument_enabled) {
        goals++;
        if (player->rating_monument < scenario_data.win_monument) {
            city_victory_state = VICTORY_STATE_NONE;
        }
        if (!city_victory_goals_reached()) {
            city_victory_state = VICTORY_STATE_NONE;
        }
        for (int32_t slot = 0; slot < BURIAL_GOAL_MONUMENT_SLOTS; slot++) {
            scenario_monument_slot_is_pyramid(slot);
        }
    }
    if (scenario_data.win_kingdom_enabled) {
        goals++;
        if (player->rating_kingdom < scenario_data.win_kingdom) {
            city_victory_state = VICTORY_STATE_NONE;
        }
    }
    if (scenario_data.win_houses != 0) {
        goals++;
        if (city_victory_count_houses_of_level(scenario_data.win_houses_level) < scenario_data.win_houses) {
            city_victory_state = VICTORY_STATE_NONE;
        }
    }
    if (settings_data.is_custom_scenario == 0 && settings_data.mission_id == tutorial_briefing_tables.tutorial.entries[TUTORIAL_FLAG_HOUSES_DONE].mission && tutorial_flags_a[TUTORIAL_FLAG_HOUSES_DONE] == 0 && tutorial_flags_a[TUTORIAL_FLAG_BEER_STORED] != 0 &&
        city_victory_count_houses_of_level(HOUSE_LEVEL_SPACIOUS_APARTMENT) > 0) {
        tutorial_set_flag(TUTORIAL_FLAG_HOUSES_DONE);
    }
    int32_t time_limit = scenario_data.time_limit;
    int32_t survival = scenario_data.survival;
    int32_t check_time = 1;
    if (goals <= 0) {
        city_victory_state = VICTORY_STATE_NONE;
        if (time_limit) {
            goals++;
        }
        if (survival) {
            goals++;
        }
    } else if (city_victory_state == VICTORY_STATE_WON && survival == 0) {
        check_time = 0;
    }
    if (check_time) {
        int32_t end_year = scenario_max_year;
        if (time_limit) {
            if (player->difficulty == DIFFICULTY_VERY_EASY) {
                end_year += EXTRA_YEARS_VERY_EASY;
            } else if (player->difficulty == DIFFICULTY_EASY) {
                end_year += EXTRA_YEARS_EASY;
            }
        }
        if (game_time_year >= end_year) {
            if (time_limit) {
                city_victory_state = VICTORY_STATE_LOST;
            } else if (survival && city_victory_state == VICTORY_STATE_NONE) {
                city_victory_state = VICTORY_STATE_LOST;
            }
        } else if (survival) {
            city_victory_state = VICTORY_STATE_NONE;
        }
    }
    int32_t emigrated = player->kingdom_soldiers + player->enemies;
    if (emigrated > player->soldiers + LOST_POPULATION_EXTRA_EMIGRANTS &&
        player->population < player->population_highest_ever / LOST_POPULATION_FRACTION_OF_HIGHEST) {
        city_victory_state = VICTORY_STATE_LOST;
        city_lost_by_population = 1;
    }
    if (emigrated > 0 && player->population <= 0) {
        city_victory_state = VICTORY_STATE_LOST;
        city_lost_by_population = 1;
    }
    if (editor_active == 1) {
        city_victory_state = VICTORY_STATE_NONE;
    }
    if (goals <= 0) {
        city_victory_state = VICTORY_STATE_NONE;
    }
    if (player->mission_has_won != 0) {
        city_victory_state = (player->mission_continue_months_left <= 0);
    }
    if (city_victory_forced) {
        city_victory_state = VICTORY_STATE_WON;
    } else if (city_victory_state == VICTORY_STATE_NONE) {
        return;
    }
    construction_building_type = BUILDING_NONE;
    game_fast_forward_year = 0;
    if (city_victory_state < VICTORY_STATE_WON) {
        if (player->fired_message_shown == 0 && (city_lost_by_population || time_limit == 0 || player->difficulty < DIFFICULTY_EASY)) {
            player->fired_message_shown = 1;
            city_message_use_popup = 1;
            city_message_post(MESSAGE_MISSION_DEFEAT, 0, 0, NULL, 0);
            city_victory_forced = 0;
            return;
        }
        city_victory_lost_cleanup();
        window_intermezzo_show(INTERMEZZO_FIRED, WINDOW_MISSION_END, INTERMEZZO_DURATION_MS);
        city_victory_forced = 0;
        return;
    }
    if (player->victory_message_shown == 0) {
        player->victory_message_shown = 1;
        city_message_use_popup = 1;
        window_current_id = WINDOW_VICTORY;
        window_redraw_request = 1;
        return;
    }
    mouse_buttons = 0;
    mouse_right_went_up = 0;
    mouse_left_went_up = 0;
    video_system_mode = VIDEO_SYSTEM_MODE_FULL_SCREEN;
    int32_t x = (g_screen_width - VICTORY_VIDEO_WIDTH) / 2;
    int32_t y = (g_screen_height - VICTORY_VIDEO_HEIGHT) / 2;
    if (settings_data.mission_id >= FIRST_MISSION_WITH_WIN_GAME_VIDEO && settings_data.is_custom_scenario == 0) {
        video_start(victory_video_win_game, x, y, WINDOW_VICTORY_VIDEO);
        city_victory_forced = 0;
        return;
    }
    const char *video = (settings_data.saved_with_game.victory_video_senate == 0) ? victory_video_balcony : victory_video_senate;
    video_start(video, x, y, WINDOW_VICTORY_VIDEO);
    city_victory_forced = 0;
    settings_data.saved_with_game.victory_video_senate = (settings_data.saved_with_game.victory_video_senate == 0);
}
