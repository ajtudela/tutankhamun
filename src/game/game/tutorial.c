// Tutorial flags set by the simulation (a building's first figure, the first fire, ...): a flag opens build menu
// items, redraws the sidebar and posts its message. In the tutorial missions the menus follow the flags; elsewhere all
// are on. Also the once-per-game message about the legions carried over, the reset of the animation timers and the
// game speed buttons.
//
// The legions' message passes a 15-entry phrase array of which only three entries are set.

#include "game/tutorial.h"

#include "city/city_internal.h"
#include "city/message.h"
#include "editor/editor_allowed_buildings.h"
#include "game/game_data.h"
#include "game/game_internal.h"
#include "game/mission_records.h"
#include "game/time.h"
#include "widget/sidebar.h"
#include "widget/widget_data.h"
#include "window/building_menu.h"
#include "window/window_data.h"
#include "window/window_id.h"

#include <stdint.h>

enum {
    TUTORIAL_SIDEBAR_BUTTONS = 12,
};

enum tutorial_legions_phrase {
    TUTORIAL_LEGIONS_PHRASE_TITLE = 585,
    TUTORIAL_LEGIONS_PHRASE_INTRO = 584,
    TUTORIAL_LEGIONS_PHRASE_INFANTRY = 586,
    TUTORIAL_LEGIONS_PHRASE_ARCHERS = 587,
    TUTORIAL_LEGIONS_PHRASE_CHARIOTEERS = 588,
    TUTORIAL_LEGIONS_PHRASE_INFANTRY_ARCHERS = 589,
    TUTORIAL_LEGIONS_PHRASE_INFANTRY_CHARIOTEERS = 590,
    TUTORIAL_LEGIONS_PHRASE_ARCHERS_CHARIOTEERS = 591,
    TUTORIAL_LEGIONS_PHRASE_ALL = 592,
};

enum tutorial_legions_message {
    TUTORIAL_LEGIONS_PHRASES = 16,
    TUTORIAL_LEGIONS_PHRASE_KINDS_SLOT = 8,
};

void tutorial_menu_counts_store(void)
{
    for (int32_t i = 0; i < TUTORIAL_SIDEBAR_BUTTONS; i++) {
        build_menu_group = buttons_expanded_build[i].parameter1;
        build_menu_item_counts[i] = building_menu_count_items();
    }
}

void tutorial_menu_counts_compare(void)
{
    for (int32_t i = 0; i < TUTORIAL_SIDEBAR_BUTTONS; i++) {
        build_menu_group = buttons_expanded_build[i].parameter1;
        int32_t count = building_menu_count_items();
        if (build_menu_item_counts[i] < count) {
            buttons_expanded_build[i].animation = 1;
        }
    }
}

void tutorial_mission_menu_flags(void)
{
    if (settings_data.mission_id > TUTORIAL_LAST_MISSION || settings_data.is_custom_scenario != 0) {
        for (int32_t i = 0; i < TUTORIAL_MENU_FLAGS; i++) {
            tutorial_flags_b[i] = 1;
        }
        return;
    }
    int32_t mission = settings_data.mission_id;
    for (int32_t i = 0; i < TUTORIAL_COUNT; i++) {
        const tutorial_entry *e = &tutorial_briefing_tables.tutorial.entries[i];
        if (mission != e->mission || e->menu_flag_count <= 0) {
            continue;
        }
        uint8_t on = tutorial_flags_a[i] != 0;
        for (int32_t k = 0; k < e->menu_flag_count; k++) {
            tutorial_flags_b[e->menu_flags[k]] = on;
        }
    }
}

void tutorial_set_flag(int32_t flag)
{
    tutorial_flags_a[flag] = 1;
    tutorial_menu_counts_store();
    building_menu_update();
    sidebar_build_buttons_update();
    tutorial_menu_counts_compare();
    tutorial_mission_menu_flags();
    if (window_current_id == WINDOW_CITY) {
        widget_sidebar_draw_background();
    }
    city_message_use_popup = 1;
    city_message_post(tutorial_briefing_tables.tutorial.entries[flag].message, 0, 0, NULL, 0);
}

void game_animation_init(void)
{
    for (int32_t i = 0; i < ANIMATION_TIMERS; i++) {
        animation_timers[i] = 0;
    }
    for (int32_t i = 0; i < ANIMATION_TIMERS; i++) {
        animation_should_advance[i] = 0;
    }
}

void settings_change_game_speed(void)
{
    if (editor_active == 1) {
        return;
    }
    if (button_parameter == GAME_SPEED_BUTTON_DOWN) {
        settings_data.game_speed -= GAME_SPEED_STEP;
    } else if (button_parameter == GAME_SPEED_BUTTON_UP) {
        settings_data.game_speed += GAME_SPEED_STEP;
    }
    if (settings_data.game_speed < GAME_SPEED_MIN) {
        settings_data.game_speed = GAME_SPEED_MIN;
    } else if (settings_data.game_speed > GAME_SPEED_MAX) {
        settings_data.game_speed = GAME_SPEED_MAX;
    }
    window_redraw_request = 1;
}

static int16_t legions_kinds_phrase(int32_t infantry, int32_t archers, int32_t charioteers)
{
    if (infantry) {
        if (archers) {
            return (int16_t) (charioteers ? TUTORIAL_LEGIONS_PHRASE_ALL : TUTORIAL_LEGIONS_PHRASE_INFANTRY_ARCHERS);
        }
        if (charioteers) {
            return TUTORIAL_LEGIONS_PHRASE_INFANTRY_CHARIOTEERS;
        }
    } else if (archers && charioteers) {
        return TUTORIAL_LEGIONS_PHRASE_ARCHERS_CHARIOTEERS;
    }
    if (infantry) {
        return TUTORIAL_LEGIONS_PHRASE_INFANTRY;
    }
    return (int16_t) (archers ? TUTORIAL_LEGIONS_PHRASE_ARCHERS : TUTORIAL_LEGIONS_PHRASE_CHARIOTEERS);
}

static int32_t legion_carried_over(int32_t kind)
{
    return mission_finished_records()->legions[kind].soldiers && scenario_data.allowed_buildings[ALLOWED_FORT_INFANTRY + kind];
}

void tutorial_update_mission_messages(void)
{
    if (settings_data.is_custom_scenario) {
        return;
    }
    for (int32_t flag = TUTORIAL_FIRST_MISSION_FLAG; flag <= TUTORIAL_LAST_MISSION_FLAG; flag++) {
        if (!tutorial_flags_a[flag] && tutorial_briefing_tables.tutorial.entries[flag].mission == settings_data.mission_id) {
            tutorial_set_flag(flag);
        }
    }
    if (tutorial_legions_message_posted) {
        return;
    }
    int32_t carried[MISSION_LEGION_KINDS];
    for (int32_t kind = 0; kind < MISSION_LEGION_KINDS; kind++) {
        carried[kind] = legion_carried_over(kind);
    }
    if (carried[MISSION_LEGION_INFANTRY] || carried[MISSION_LEGION_ARCHERS] || carried[MISSION_LEGION_CHARIOTEERS]) {
        int16_t phrases[TUTORIAL_LEGIONS_PHRASES] = { 0 };
        phrases[1] = TUTORIAL_LEGIONS_PHRASE_INTRO;
        phrases[0] = TUTORIAL_LEGIONS_PHRASE_TITLE;
        phrases[TUTORIAL_LEGIONS_PHRASE_KINDS_SLOT] =
            legions_kinds_phrase(carried[MISSION_LEGION_INFANTRY], carried[MISSION_LEGION_ARCHERS], carried[MISSION_LEGION_CHARIOTEERS]);
        city_message_use_popup = 1;
        city_message_post(MESSAGE_TEMPLATE_GENERAL, 0, 0, phrases, 0);
    }
    tutorial_legions_message_posted = 1;
}
