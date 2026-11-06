// The flow of a mission: replaying it, starting a campaign at a period, the end of a won mission (the next link and
// the records the player keeps: savings, the best legions, the kept monuments' positions), what a new mission takes
// over from the last one, the victory and defeat windows, the briefing, their buttons and the city intro's background.
// A mission's options are the characters of its text in group 307. A free play mission ends without any campaign
// record and returns to the front menu.
//
// A won mission whose next link is not in the campaign stays on its link.

#include "window/mission_flow.h"
#include "building/abu_simbel.h"
#include "building/building.h"
#include "building/building_create.h"
#include "building/building_type.h"
#include "building/caesareum.h"
#include "building/construction_load.h"
#include "building/library.h"
#include "building/lighthouse.h"
#include "building/mausoleum.h"
#include "building/monument.h"
#include "building/monument_city.h"
#include "building/monument_instant.h"
#include "building/royal_tomb.h"
#include "building/sun_temple.h"
#include "city/city_player.h"
#include "city/finance_estimates.h"
#include "city/kingdom.h"
#include "city/message.h"
#include "editor/editor_data.h"
#include "empire/empire.h"
#include "figure/figure_type.h"
#include "figure/formation.h"
#include "game/game_data.h"
#include "game/game_internal.h"
#include "game/mission_records.h"
#include "game/player_record.h"
#include "game/scenario_start.h"
#include "game/startup_load.h"
#include "graphics/button_border.h"
#include "graphics/graphics_data.h"
#include "graphics/image_collection.h"
#include "graphics/image_draw.h"
#include "graphics/label_animation_slots.h"
#include "graphics/panel_draw.h"
#include "graphics/rich_text.h"
#include "graphics/text_draw.h"
#include "graphics/ui_layout.h"
#include "io/file.h"
#include "io/game_file.h"
#include "map/tiles.h"
#include "scenario/campaign.h"
#include "scenario/campaign_load.h"
#include "scenario/scenario_data.h"
#include "text/event_text.h"
#include "text/lang.h"
#include "text/lang_message.h"
#include "text/text_data.h"
#include "text/text_lookup.h"
#include "video/bink.h"
#include "widget/widget_data.h"
#include "window/free_play.h"
#include "window/high_scores.h"
#include "window/intermezzo.h"
#include "window/main_menu.h"
#include "window/map_selection.h"
#include "window/player_selection.h"
#include "window/victory_dialog.h"
#include "window/window_data.h"
#include "window/window_id.h"
#include "window/window_image_group.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

enum mission_flow_text {
    VICTORY_TEXT_GROUP = 148,
    VICTORY_TEXT_CULTURE = 0,
    VICTORY_TEXT_PROSPERITY = 1,
    VICTORY_TEXT_KINGDOM = 3,
    VICTORY_TEXT_POPULATION = 4,
    VICTORY_TEXT_TREASURY = 5,
    VICTORY_TEXT_MONUMENTS = 6,
    VICTORY_TEXT_MONTHS = 7,
    VICTORY_TEXT_DIFFICULTY = 8,
    VICTORY_TEXT_SCORE = 9,
    VICTORY_TEXT_HIGH_SCORE_NEW = 10,
    VICTORY_TEXT_HIGH_SCORE_OWN_KEPT = 11,
    VICTORY_TEXT_HIGH_SCORE_OWN_BEATEN = 12,
    VICTORY_TEXT_HIGH_SCORE_OTHER_KEPT = 13,
    VICTORY_TEXT_HIGH_SCORE_OTHER_BEATEN = 14,
    VICTORY_TEXT_MONTHS_POSTFIX = 15,
    VICTORY_TEXT_CHEATED = 16,
    TRANSITION_TEXT_GROUP = 147,
    DEFEAT_TEXT_TITLE = 1,
    DEFEAT_TEXT_BODY = 16,
    DEFEAT_TEXT_TITLE_LOWER_DIFFICULTY = 38,
    DEFEAT_TEXT_BODY_LOWER_DIFFICULTY = 39,
    BRIEFING_TEXT_START = 7,
    BRIEFING_TEXT_GOALS = 10,
    BRIEFING_TEXT_POPULATION = 11,
    BRIEFING_TEXT_CULTURE = 12,
    BRIEFING_TEXT_PROSPERITY = 13,
    BRIEFING_TEXT_MONUMENTS = 14,
    BRIEFING_TEXT_KINGDOM = 15,
    MAP_TEXT_DIFFICULTY = 216,
    HOUSE_LEVEL_TEXT_PLURAL = 20,
    MONUMENT_NAMES_GROUP = 198,
    CITY_INTRO_TEXT_GROUP = 142,
    CITY_INTRO_TEXT_TITLE = 0,
    CITY_INTRO_TEXT_BODY = 1,
    CITY_INTRO_TEXT_HINT = 2,
    CITY_INTRO_TEXT_SEA_TRADE = 3,
    VICTORY_TEXT_LENGTH = 200,
};

enum mission_flow_layout {
    CAMPAIGN_VIDEO_PERIODS = 5,
    CAMPAIGN_PERIOD_GRAND_VICTORY = 5,
    MISSION_CHOICE = 0x7F,
    MISSION_LARGE_TOMB_KEPT = 40,
    MISSION_ALEXANDRIA_MONUMENTS = 51,
    MISSION_MEDIUM_TOMB_FIRST = 40,
    MISSION_LARGE_TOMB_FIRST = 41,
    VIDEO_HIGH_RESOLUTION_CPU = 233,
    VIDEO_WIDTH = 480,
    VIDEO_HEIGHT = 286,
    FULL_SCREEN_WIDTH = 1024,
    FULL_SCREEN_HEIGHT = 768,
    PLAYER_NAME_SIZE = 32,
    PATH_SIZE = 100,
    MAUSOLEUM_X_MASK = 0x1FFF,
    MAUSOLEUM_ORIENTATION_SHIFT = 13,
    BRIEFING_GOAL_LABEL = 6,
    BRIEFING_GOAL_WIDTH_BLOCKS = 15,
    BRIEFING_TUTORIAL_GOAL = 4,
    BRIEFING_TEXT_WIDTH_BLOCKS = 35,
    BRIEFING_TEXT_HEIGHT_BLOCKS = 15,
    BRIEFING_TEXT_LINES = 14,
    CONTINUE_TWO_YEARS = 1,
    CONTINUE_FIVE_YEARS = 2,
    CONTINUE_TWO_YEARS_MONTHS = 24,
    CONTINUE_FIVE_YEARS_MONTHS = 60,
    DEFEAT_EASIER_MAX_DIFFICULTY = 1,
    DEFEAT_BUTTONS_REPLAY = 0,
    DEFEAT_BUTTONS_LOWER_DIFFICULTY = 2,
    WHITE_SPACE_MAX = 0x20,
};

static const char *const period_videos[CAMPAIGN_VIDEO_PERIODS][2] = {
    { "Binks\\low\\pre_dynastic_small.bik", "Binks\\high\\pre_dynastic_big.bik" },
    { "Binks\\low\\archaic_small.bik", "Binks\\high\\archaic_big.bik" },
    { "Binks\\low\\old_kingdom_small.bik", "Binks\\high\\old_kingdom_big.bik" },
    { "Binks\\low\\middle_kingdom_small.bik", "Binks\\high\\middle_kingdom_big.bik" },
    { "Binks\\low\\new_king_small.bik", "Binks\\high\\new_king_big.bik" },
};

static mission_carry_records *start_records(void)
{
    return &record_savings.started;
}

static const char *mission_options(int32_t mission)
{
    return lang_get_string(MISSION_OPTIONS_GROUP, mission);
}

static bool video_high_resolution(void)
{
    return settings_data.cpu >= VIDEO_HIGH_RESOLUTION_CPU;
}

static void play_video(const char *file)
{
    video_play(video_player_ptr, file, (g_screen_width - VIDEO_WIDTH) / 2, (g_screen_height - VIDEO_HEIGHT) / 2);
}

static void replay_save_path(char *path)
{
    strcpy(path, dir_save_path());
    strcat(path, path_separator);
    strcat(path, str_replay_save);
}

bool mission_replay_available(void)
{
    char path[PATH_SIZE];
    if (settings_data.is_custom_scenario != 0) {
        strcpy(path, maps_directory);
        strcat(path, scenario_map_name);
        return scenario_map_exists(path) != 0;
    }
    if (game_is_replay == GAME_PLAY_CAMPAIGN) {
        replay_save_path(path);
        return file_exists(path);
    }
    return true;
}

void mission_replay(void)
{
    char path[PATH_SIZE];
    construction_building_type = BUILDING_NONE;
    if (settings_data.is_custom_scenario != 0) {
        game_is_replay = GAME_PLAY_CUSTOM_SCENARIO;
        game_file_start_scenario(scenario_map_name, 0);
    } else if (game_is_replay != GAME_PLAY_CAMPAIGN) {
        tutorial_legions_message_posted = 0;
        mission_start_intermezzo();
        return;
    } else {
        replay_save_path(path);
        if (!file_exists(path)) {
            window_main_menu_show();
            return;
        }
        editor_active = 0;
        overlay_show_all = 0;
        game_file_load_saved_game_keep_difficulty(path);
    }
    window_current_id = WINDOW_CITY;
    window_redraw_request = 1;
}

void mission_selection_start(void)
{
    int32_t mode = mission_select_mode;
    uint32_t period = (uint32_t) mission_selected;
    if (mode == 0 && (int32_t) period != campaign_period_reached) {
        return;
    }
    settings_data.mission_id = 0;
    editor_active = 0;
    overlay_show_all = 0;
    settings_data.is_custom_scenario = 0;
    empire_editor_route_dragging = 1;
    window_redraw_request = 1;
    game_is_replay = mode != 0 ? GAME_PLAY_CAMPAIGN_FROM_PERIOD : GAME_PLAY_CAMPAIGN;
    if (period < (uint32_t) CAMPAIGN_VIDEO_PERIODS) {
        play_video(period_videos[period][video_high_resolution()]);
    }
    settings_start_campaign(mission_selected);
    if (settings_data.mission_id == MISSION_CHOICE) {
        window_current_id = WINDOW_MISSION_CHOICE;
        return;
    }
    mission_start_intermezzo();
}

static int32_t legion_kind(const formation *legion)
{
    if (legion->figure_type == FIGURE_INFANTRY) {
        return MISSION_LEGION_INFANTRY;
    }
    if (legion->figure_type == FIGURE_ARCHER) {
        return MISSION_LEGION_ARCHERS;
    }
    return MISSION_LEGION_CHARIOTEERS;
}

static void store_best_legions(void)
{
    int32_t experience[MISSION_LEGION_KINDS] = { -1, -1, -1 };
    uint8_t soldiers[MISSION_LEGION_KINDS] = { 0, 0, 0 };
    int32_t best[MISSION_LEGION_KINDS] = { -1, -1, -1 };
    for (int32_t id = 1; id < MAX_FORMATIONS; id++) {
        const formation *legion = formation_get(id);
        if (legion->in_use != 1 || legion->is_legion == 0) {
            continue;
        }
        int32_t kind = legion_kind(legion);
        if (legion->experience > experience[kind] || (legion->experience == experience[kind] && legion->num_figures > soldiers[kind])) {
            experience[kind] = legion->experience;
            soldiers[kind] = legion->num_figures;
            best[kind] = id;
        }
    }
    for (int32_t kind = 0; kind < MISSION_LEGION_KINDS; kind++) {
        mission_legion_record *record = &mission_finished_records()->legions[kind];
        record->experience = experience[kind];
        record->soldiers = soldiers[kind];
        record->legion_id = best[kind] == -1 ? MISSION_LEGION_NONE : formation_get(best[kind])->legion_id;
    }
}

static void store_monument_position(const building *b, int32_t id)
{
    if (b->type == BUILDING_MAUSOLEUM) {
        campaign_object.monuments.mausoleum_y = (int16_t) b->data.mausoleum.origin_y;
        campaign_object.monuments.mausoleum_x_and_orientation = (uint16_t) (b->data.mausoleum.origin_x | (b->data.mausoleum.orientation << MAUSOLEUM_ORIENTATION_SHIFT));
    }
    if (b->type == BUILDING_ALEXANDRIA_LIBRARY) {
        campaign_object.monuments.library_x = (int16_t) (b->x - 4);
        campaign_object.monuments.library_y = (int16_t) (b->y + 2);
    }
    if (b->type == BUILDING_LIGHTHOUSE) {
        const building *main = &city_buildings.records[id - b->subtype];
        campaign_object.monuments.lighthouse_x = main->x;
        campaign_object.monuments.lighthouse_y = main->y;
    }
    if (b->type == BUILDING_SMALL_ROYAL_TOMB) {
        campaign_object.monuments.small_tomb_x = (int16_t) (b->x - 5);
        campaign_object.monuments.small_tomb_y = (int16_t) (b->y + 6);
    }
    if (b->type == BUILDING_MEDIUM_ROYAL_TOMB) {
        campaign_object.monuments.medium_tomb_x = (int16_t) (b->x - 6);
        campaign_object.monuments.medium_tomb_y = (int16_t) (b->y + 8);
    }
    if (b->type == BUILDING_LARGE_ROYAL_TOMB) {
        campaign_object.monuments.large_tomb_x = (int16_t) (b->x + 1);
        campaign_object.monuments.large_tomb_y = (int16_t) (b->y + 17);
    }
}

void mission_records_store(void)
{
    mission_finished_records()->savings = city_player_current()->personal_savings;
    store_best_legions();
    if (mission_options(settings_data.mission_id)[MISSION_OPTION_STORE_MONUMENTS] == '1') {
        for (int32_t id = 1; id < MAX_BUILDINGS; id++) {
            const building *b = &city_buildings.records[id];
            if (b->state == BUILDING_STATE_IN_USE) {
                store_monument_position(b, id);
            }
        }
    }
    player_data_write_file(&campaign_object, player_record_path());
}

static campaign_link *campaign_link_at(int32_t link)
{
    if (link < 0 || link >= campaign_object.campaign.link_count) {
        return 0;
    }
    return &campaign_object.campaign.links[link];
}

static void mark_link_played(void)
{
    campaign_link *link = campaign_link_at(settings_data.campaign_link);
    if (link != 0) {
        link->played = 1;
        player_data_write_file(&campaign_object, player_record_path());
    }
}

static bool move_to_next_link(int32_t next)
{
    const campaign_link *current = campaign_link_at(settings_data.campaign_link);
    if (current == 0) {
        return false;
    }
    if (current->choice_count >= 2) {
        return true;
    }
    const campaign_link *link = campaign_link_at(next);
    if (link == 0) {
        return false;
    }
    settings_data.previous_campaign_link = (uint8_t) settings_data.campaign_link;
    settings_data.campaign_link = (int8_t) next;
    settings_data.mission_id = (int8_t) link->mission_id;
    return false;
}

static void store_large_tomb_position(void)
{
    for (int32_t id = 1; id < MAX_BUILDINGS; id++) {
        const building *b = &city_buildings.records[id];
        if (b->type == BUILDING_LARGE_ROYAL_TOMB) {
            campaign_object.monuments.large_tomb_x = (int16_t) (b->x + 1);
            campaign_object.monuments.large_tomb_y = (int16_t) (b->y + 17);
        }
    }
    player_data_write_file(&campaign_object, player_record_path());
}

static void leave_city(void)
{
    undo_available = 0;
    city_overlay = 0;
    window_redraw_request = 1;
}

static bool open_next_period(void)
{
    campaign_period_reached = (int8_t) settings_data.campaign + 1;
    if (campaign_period_reached == CAMPAIGN_PERIOD_GRAND_VICTORY) {
        play_video(video_high_resolution() ? str_video_win_big : str_video_win_small);
        return false;
    }
    if (campaign_period_reached >= CAMPAIGN_PERIODS) {
        return false;
    }
    mission_selected = campaign_period_reached;
    window_current_id = WINDOW_MISSION_SELECTION;
    mission_select_mode = 0;
    return true;
}

static void free_play_mission_end(void)
{
    city_player *player = city_player_current();
    player->mission_has_won = 0;
    player->mission_continue_months_left = 0;
    player->mission_continue_months_chosen = 0;
    free_play_leave();
    leave_city();
}

void mission_end_advance(void)
{
    int32_t next = -1;
    uint8_t choice = 0;
    if (free_play_running()) {
        free_play_mission_end();
        return;
    }
    settings_data.starting_favor = city_player_current()->rating_kingdom;
    settings_data.personal_savings_last_mission = city_player_current()->personal_savings;
    settings_data.current_mission_id++;
    if (settings_data.is_custom_scenario == 0) {
        mark_link_played();
        next = campaign_find_link(&campaign_object.campaign, (int8_t) settings_data.campaign, settings_data.campaign_link, 0);
        if ((game_is_replay == GAME_PLAY_CAMPAIGN_FROM_PERIOD || game_is_replay == GAME_PLAY_CAMPAIGN) && next >= 0) {
            mission_records_store();
            choice = move_to_next_link(next);
        } else if (settings_data.mission_id == MISSION_LARGE_TOMB_KEPT) {
            store_large_tomb_position();
        }
    }
    city_player_current()->mission_has_won = 0;
    city_player_current()->mission_continue_months_left = 0;
    city_player_current()->mission_continue_months_chosen = 0;
    if (next >= 0 && settings_data.is_custom_scenario == 0) {
        if (game_is_replay != GAME_PLAY_MISSION_REPLAY) {
            if (choice) {
                window_current_id = WINDOW_MISSION_CHOICE;
                campaign_mission_choice.choice = 0;
            } else {
                mission_start_intermezzo();
            }
            leave_city();
            return;
        }
    } else if (settings_data.is_custom_scenario != 0) {
        editor_active = 0;
        overlay_show_all = 0;
        map_file_list.campaign = 0;
        map_file_list.selection_kind = MAP_SELECTION_CUSTOM;
        window_map_selection_show();
        leave_city();
        return;
    }
    if (game_is_replay == GAME_PLAY_CAMPAIGN && open_next_period()) {
        leave_city();
        return;
    }
    window_main_menu_show();
    leave_city();
}

static int32_t monument_recreate(int32_t type, int32_t x, int32_t y)
{
    int32_t id = building_create(city_current_player, type, x, y, 0, 0);
    building_construction_add_to_map(city_current_player, type, id, x, y, 1, 1, 1);
    return id;
}

static void recreate_alexander_monuments(void)
{
    uint16_t mausoleum = campaign_object.monuments.mausoleum_x_and_orientation;
    if (mausoleum != 0 && campaign_object.monuments.mausoleum_y != 0) {
        int32_t x = mausoleum & MAUSOLEUM_X_MASK;
        mausoleum_complete_instant(monument_recreate(BUILDING_MAUSOLEUM, x, campaign_object.monuments.mausoleum_y));
    }
    if (campaign_object.monuments.library_x != 0 && campaign_object.monuments.library_y != 0 && settings_data.mission_id == MISSION_ALEXANDRIA_MONUMENTS) {
        library_set_completed(&library_object, monument_recreate(BUILDING_ALEXANDRIA_LIBRARY, campaign_object.monuments.library_x, campaign_object.monuments.library_y));
    }
    if (campaign_object.monuments.lighthouse_x != 0 && campaign_object.monuments.lighthouse_y != 0 && settings_data.mission_id == MISSION_ALEXANDRIA_MONUMENTS) {
        lighthouse_complete_instant(&lighthouse_object, monument_recreate(BUILDING_LIGHTHOUSE, campaign_object.monuments.lighthouse_x, campaign_object.monuments.lighthouse_y));
    }
}

static void recreate_valley_monuments(void)
{
    if (campaign_object.monuments.small_tomb_x != 0 && campaign_object.monuments.small_tomb_y != 0) {
        royal_tomb_set_completed(&small_royal_tomb_object.base, monument_recreate(BUILDING_SMALL_ROYAL_TOMB, campaign_object.monuments.small_tomb_x, campaign_object.monuments.small_tomb_y));
    }
    if (campaign_object.monuments.medium_tomb_x != 0 && campaign_object.monuments.medium_tomb_y != 0 && settings_data.mission_id >= MISSION_MEDIUM_TOMB_FIRST) {
        royal_tomb_set_completed(&medium_royal_tomb_object.base, monument_recreate(BUILDING_MEDIUM_ROYAL_TOMB, campaign_object.monuments.medium_tomb_x, campaign_object.monuments.medium_tomb_y));
    }
    if (campaign_object.monuments.large_tomb_x != 0 && campaign_object.monuments.large_tomb_y != 0 && settings_data.mission_id >= MISSION_LARGE_TOMB_FIRST) {
        royal_tomb_set_completed(&large_royal_tomb_object.base, monument_recreate(BUILDING_LARGE_ROYAL_TOMB, campaign_object.monuments.large_tomb_x, campaign_object.monuments.large_tomb_y));
    }
}

static void clear_start_legions(void)
{
    for (int32_t kind = 0; kind < MISSION_LEGION_KINDS; kind++) {
        mission_legion_record *record = &start_records()->legions[kind];
        record->experience = -1;
        record->soldiers = 0;
        record->legion_id = MISSION_LEGION_NONE;
    }
}

void mission_records_apply(void)
{
    *start_records() = *mission_finished_records();
    const char *options = mission_options(settings_data.mission_id);
    settings_data.personal_savings_last_mission = options[MISSION_OPTION_KEEP_SAVINGS] == '1' ? start_records()->savings : 0;
    start_records()->savings = 0;
    if (options[MISSION_OPTION_MONUMENTS] == MISSION_MONUMENTS_ALEXANDER) {
        recreate_alexander_monuments();
    } else if (options[MISSION_OPTION_MONUMENTS] == MISSION_MONUMENTS_VALLEY) {
        recreate_valley_monuments();
    }
    if (options[MISSION_OPTION_KEEP_LEGIONS] != '1') {
        clear_start_legions();
        tutorial_legions_message_posted = 1;
    } else {
        tutorial_legions_message_posted = 0;
    }
    map_tiles_update_all();
}

static int32_t monument_progress(int32_t id)
{
    switch (city_buildings.records[id].type) {
        case BUILDING_PYRAMID: {
            pyramid_progress info;
            pyramid_get_progress_info(id, &info);
            return info.percent;
        }
        case BUILDING_SMALL_OBELISK:
        case BUILDING_LARGE_OBELISK:
            return city_buildings.records[id].data.obelisk.progress;
        case BUILDING_SPHINX:
            return city_player_current()->sphinx_stage * 100 / 16;
        case BUILDING_SUN_TEMPLE:
            return sun_temple_get_progress(id, 0);
        case BUILDING_MAUSOLEUM:
            return mausoleum_get_progress(id, 0);
        case BUILDING_ALEXANDRIA_LIBRARY:
            return library_get_progress(&library_object, id, 0);
        case BUILDING_CAESAREUM:
            return caesareum_get_progress(&caesareum_object, id, 0);
        case BUILDING_LIGHTHOUSE:
            return lighthouse_get_progress(&lighthouse_object, id, 0);
        case BUILDING_SMALL_ROYAL_TOMB:
            return royal_tomb_progress(BUILDING_SMALL_ROYAL_TOMB, id, 0);
        case BUILDING_ABU_SIMBEL:
            return abu_simbel_get_progress(&abu_simbel_object, id, 0);
        case BUILDING_MEDIUM_ROYAL_TOMB:
            return royal_tomb_progress(BUILDING_MEDIUM_ROYAL_TOMB, id, 0);
        case BUILDING_LARGE_ROYAL_TOMB:
            return royal_tomb_progress(BUILDING_LARGE_ROYAL_TOMB, id, 0);
        case BUILDING_GRAND_ROYAL_TOMB:
            return royal_tomb_progress(BUILDING_GRAND_ROYAL_TOMB, id, 0);
        default:
            return -1;
    }
}

int32_t victory_count_finished_monuments(void)
{
    int32_t count = 0;
    int32_t y = 180;
    for (int32_t slot = 0; slot < MONUMENT_SLOTS; slot++) {
        int32_t id = city_player_current()->monument_building_id[slot];
        if (id <= 0) {
            continue;
        }
        if (monument_progress(id) == MONUMENT_PROGRESS_COMPLETE) {
            y += 20;
            count++;
            lang_text_draw(MONUMENT_NAMES_GROUP, city_player_current()->monument_def_id[slot], victory_x + 275, victory_y + y, image_group(IMAGE_GROUP_FONT_BLACK), 0);
        }
    }
    return count;
}

static void draw_rating(int32_t text, int32_t value, int32_t dx, int32_t dy)
{
    g_text_width = 0;
    lang_text_draw(VICTORY_TEXT_GROUP, text, victory_x + dx, victory_y + dy, image_group(IMAGE_GROUP_FONT_BLACK), 0);
    text_draw_number(value, NUMBER_PREFIX_NONE, postfix_none, g_text_width + victory_x + dx, victory_y + dy, image_group(IMAGE_GROUP_FONT_BLACK), 0);
}

static void draw_high_score_text(int32_t text, const char *name)
{
    char saved[PLAYER_NAME_SIZE];
    char expanded[VICTORY_TEXT_LENGTH];
    const char *message = lang_get_string(VICTORY_TEXT_GROUP, text);
    strcpy(saved, settings_data.player_name);
    strcpy(settings_data.player_name, name);
    event_text_expand(message, expanded, VICTORY_TEXT_LENGTH);
    strcpy(settings_data.player_name, saved);
    text_draw_multiline(expanded, victory_x + 40, victory_y + 340, 544, image_group(IMAGE_GROUP_FONT_BLACK), 0);
}

static void draw_defeat(void)
{
    int32_t title = DEFEAT_TEXT_TITLE;
    int32_t text = DEFEAT_TEXT_BODY;
    if (city_defeat_options() >= DEFEAT_OPTIONS_LOWER_DIFFICULTY) {
        title = DEFEAT_TEXT_TITLE_LOWER_DIFFICULTY;
        text = DEFEAT_TEXT_BODY_LOWER_DIFFICULTY;
    }
    outer_panel_draw(victory_x, victory_y - 7, 34, 16);
    lang_text_draw_centered(LANG_GROUP_MISSION_GOALS, title, victory_x, victory_y + 16, 544, image_group(IMAGE_GROUP_FONT_LARGE), 0);
    g_draw_y_accum = 0;
    lang_text_draw_multiline(LANG_GROUP_MISSION_GOALS, text, victory_x + 25, victory_y + 48, 512, image_group(IMAGE_GROUP_FONT_BLACK), 0);
}

static void draw_high_score_result(void)
{
    switch (hs_result) {
        case HIGH_SCORE_RESULT_CHEATED:
            lang_text_draw_multiline(VICTORY_TEXT_GROUP, VICTORY_TEXT_CHEATED, victory_x + 40, victory_y + 340, 544, image_group(IMAGE_GROUP_FONT_BLACK), 0);
            break;
        case HIGH_SCORE_RESULT_NEW:
            lang_text_draw_multiline(VICTORY_TEXT_GROUP, VICTORY_TEXT_HIGH_SCORE_NEW, victory_x + 40, victory_y + 340, 544, image_group(IMAGE_GROUP_FONT_BLACK), 0);
            break;
        case HIGH_SCORE_RESULT_OWN_KEPT:
            lang_text_draw_multiline(VICTORY_TEXT_GROUP, VICTORY_TEXT_HIGH_SCORE_OWN_KEPT, victory_x + 40, victory_y + 340, 544, image_group(IMAGE_GROUP_FONT_BLACK), 0);
            break;
        case HIGH_SCORE_RESULT_OWN_BEATEN:
            lang_text_draw_multiline(VICTORY_TEXT_GROUP, VICTORY_TEXT_HIGH_SCORE_OWN_BEATEN, victory_x + 40, victory_y + 340, 544, image_group(IMAGE_GROUP_FONT_BLACK), 0);
            break;
        case HIGH_SCORE_RESULT_OTHER_BEATEN:
            draw_high_score_text(VICTORY_TEXT_HIGH_SCORE_OTHER_BEATEN, high_score_previous_name);
            break;
        case HIGH_SCORE_RESULT_OTHER_KEPT:
            draw_high_score_text(VICTORY_TEXT_HIGH_SCORE_OTHER_KEPT, highscores.missions[settings_data.mission_id].player_name);
            break;
    }
}

void window_victory_draw(void)
{
    victory_x = ui_origin_x + 48;
    victory_y = ui_origin_y + 16;
    image_view_x = (FULL_SCREEN_WIDTH - g_screen_width) / 2;
    image_view_width = g_screen_width;
    image_view_height = g_screen_height;
    image_view_y = (FULL_SCREEN_HEIGHT - g_screen_height) / 2;
    if (city_victory_state != VICTORY_STATE_WON) {
        draw_defeat();
        return;
    }
    int32_t years = scenario_data.start.year - game_time_year;
    if (years < 0) {
        years = -years;
    }
    int32_t months = game_time_month + years * MONTHS_PER_YEAR;
    int32_t score = city_victory_score(months);
    city_player *player = city_player_current();
    outer_panel_draw(victory_x, victory_y, 34, 27);
    lang_text_draw_centered(LANG_GROUP_MISSION_GOALS, VICTORY_TEXT_VICTORY, victory_x, victory_y + 16, 544, image_group(IMAGE_GROUP_FONT_LARGE), 0);
    inner_panel_draw(victory_x + 16, victory_y + 56, 32, 7);
    lang_text_draw_multiline(TRANSITION_TEXT_GROUP, mission_transition_text(), victory_x + 32, victory_y + 64, 496, image_group(IMAGE_GROUP_FONT_WHITE), 0);
    draw_rating(VICTORY_TEXT_CULTURE, player->rating_culture, 40, 180);
    draw_rating(VICTORY_TEXT_PROSPERITY, player->rating_prosperity, 40, 200);
    draw_rating(VICTORY_TEXT_KINGDOM, player->rating_kingdom, 40, 220);
    draw_rating(VICTORY_TEXT_TREASURY, player->treasury, 40, 240);
    draw_rating(VICTORY_TEXT_POPULATION, player->population, 40, 260);
    if (victory_count_finished_monuments() != 0) {
        lang_text_draw(VICTORY_TEXT_GROUP, VICTORY_TEXT_MONUMENTS, victory_x + 275, victory_y + 180, image_group(IMAGE_GROUP_FONT_BLACK), 0);
    }
    draw_rating(VICTORY_TEXT_MONTHS, months, 120, 280);
    lang_text_draw(VICTORY_TEXT_GROUP, VICTORY_TEXT_MONTHS_POSTFIX, g_text_width + 120 + victory_x, victory_y + 280, image_group(IMAGE_GROUP_FONT_BLACK), 0);
    g_text_width = 0;
    lang_text_draw(VICTORY_TEXT_GROUP, VICTORY_TEXT_DIFFICULTY, victory_x + 160, victory_y + 300, image_group(IMAGE_GROUP_FONT_BLACK), 0);
    lang_text_draw(LANG_GROUP_DIFFICULTY, city_player_current()->difficulty + 1, g_text_width + 160 + victory_x, victory_y + 300, image_group(IMAGE_GROUP_FONT_BLACK), 0);
    draw_rating(VICTORY_TEXT_SCORE, score, 200, 320);
    draw_high_score_result();
    lang_text_draw_centered(LANG_GROUP_WINDOW_HINTS, WINDOW_HINT_RIGHT_CLICK_TO_CONTINUE, victory_x + 16, victory_y + 396, 512, image_group(IMAGE_GROUP_FONT_BLACK), 0);
}

static const briefing_goal_position *goal_position(int32_t slot)
{
    return &tutorial_briefing_tables.briefing.goal_positions[slot];
}

static void draw_goal_label(int32_t slot, int32_t width_blocks)
{
    label_draw_animated(BRIEFING_GOAL_LABEL, ui_panel_x + goal_position(slot)->x, ui_panel_y + goal_position(slot)->y, width_blocks, 0, 1);
}

static void draw_goal(int32_t slot, int32_t text, int32_t value)
{
    int32_t x = goal_position(slot)->x;
    int32_t y = goal_position(slot)->y;
    draw_goal_label(slot, BRIEFING_GOAL_WIDTH_BLOCKS);
    g_text_width = 0;
    lang_text_draw(LANG_GROUP_MISSION_GOALS, text, ui_panel_x + x + 8, ui_panel_y + y + 3, image_group(IMAGE_GROUP_FONT_RED), 0);
    text_draw_number(value, NUMBER_PREFIX_NONE, postfix_none, g_text_width + ui_panel_x + x + 8, ui_panel_y + y + 3, image_group(IMAGE_GROUP_FONT_RED), 0);
}

static int32_t draw_goals(void)
{
    int32_t slot = 0;
    if (scenario_data.win_population_enabled != 0) {
        draw_goal(slot++, BRIEFING_TEXT_POPULATION, scenario_data.win_population);
    }
    if (scenario_data.win_houses != 0) {
        int32_t x = goal_position(slot)->x;
        int32_t y = goal_position(slot)->y;
        draw_goal_label(slot++, BRIEFING_GOAL_WIDTH_BLOCKS);
        g_text_width = 0;
        text_draw_number(scenario_data.win_houses, NUMBER_PREFIX_NONE, postfix_none, ui_panel_x + x + 8, ui_panel_y + y + 3, image_group(IMAGE_GROUP_FONT_RED), 0);
        lang_text_draw(
            LANG_GROUP_HOUSE_LEVELS, scenario_data.win_houses_level + HOUSE_LEVEL_TEXT_PLURAL, g_text_width + ui_panel_x + x + 8, ui_panel_y + y + 3, image_group(IMAGE_GROUP_FONT_RED), 0);
    }
    if (scenario_data.win_culture_enabled != 0) {
        draw_goal(slot++, BRIEFING_TEXT_CULTURE, scenario_data.win_culture);
    }
    if (scenario_data.win_prosperity_enabled != 0) {
        draw_goal(slot++, BRIEFING_TEXT_PROSPERITY, scenario_data.win_prosperity);
    }
    if (scenario_data.win_monument_enabled != 0) {
        draw_goal(slot++, BRIEFING_TEXT_MONUMENTS, scenario_data.win_monument);
    }
    if (scenario_data.win_kingdom_enabled != 0) {
        draw_goal(slot++, BRIEFING_TEXT_KINGDOM, scenario_data.win_kingdom);
    }
    return slot;
}

static void set_scroll_buttons_enabled(uint8_t enabled)
{
    message_scroll_up.button.buttons[0].enabled = enabled;
    message_scroll_down.button.buttons[0].enabled = enabled;
}

static void draw_briefing_text(const lang_message *message)
{
    const uint8_t *content = lang_message_data + message->content_text;
    inner_panel_draw(ui_panel_x + 16, ui_panel_y + 168, dialog_text_width_blocks - 2, BRIEFING_TEXT_HEIGHT_BLOCKS);
    rich_text_normal_font = image_group(IMAGE_GROUP_FONT_WHITE);
    rich_text_link_font = image_group(IMAGE_GROUP_FONT_RED);
    dialog_x_text = ui_panel_x - 6;
    dialog_y_text = ui_panel_y + 170;
    if (rich_line_count == 0) {
        dialog_text_height_blocks = BRIEFING_TEXT_HEIGHT_BLOCKS;
        rich_viewport_lines = BRIEFING_TEXT_LINES;
        set_scroll_buttons_enabled(1);
        rich_text_draw(content, ui_panel_x + 32, ui_panel_y + 184, dialog_text_width_blocks * 16 - 16, 1, 0);
        if (rich_line_count > rich_viewport_lines) {
            dialog_scroll_max = rich_line_count - rich_viewport_lines;
        } else {
            dialog_scroll_max = 0;
            set_scroll_buttons_enabled(0);
        }
    }
    if (rich_line_count > rich_viewport_lines) {
        inner_panel_draw(ui_panel_x + 557, ui_panel_y + 192, 2, 12);
    }
    g_clip_x0 = ui_panel_x + 19;
    g_clip_x1 = dialog_text_width_blocks * 16 + 19 + ui_panel_x;
    g_clip_y0 = ui_panel_y + 171;
    g_clip_y1 = ui_panel_y + 405;
    rich_text_draw(content, ui_panel_x + 32, ui_panel_y + 180, dialog_text_width_blocks * 16 - 16, 0, 0);
    g_clip_x0 = 0;
    g_clip_x1 = g_screen_width;
    g_clip_y0 = 0;
    g_clip_y1 = g_screen_height;
}

void window_briefing_draw(void)
{
    if (mission_briefing_started == 0) {
        mission_briefing_started = 1;
        game_file_start_scenario(scenario_map_name, 0);
        if (window_current_id == WINDOW_CITY) {
            return;
        }
    }
    dialog_text_id = mission_briefing_message();
    ui_panel_y = ui_origin_y + 16;
    ui_panel_x = ui_origin_x + 16;
    image_view_width = g_screen_width;
    image_view_height = g_screen_height;
    image_view_x = (FULL_SCREEN_WIDTH - g_screen_width) / 2;
    dialog_text_width_blocks = BRIEFING_TEXT_WIDTH_BLOCKS;
    image_view_y = (FULL_SCREEN_HEIGHT - g_screen_height) / 2;
    if (window_redraw_request == 1) {
        image_draw(image_group(IMAGE_GROUP_INTERMEZZO_BACKGROUND) + 1, 0, 0);
    }
    const lang_message *message = lang_message_get(dialog_text_id);
    outer_panel_draw(ui_panel_x, ui_panel_y, 38, 28);
    text_draw(lang_message_data + message->title_text, ui_panel_x + 16, ui_panel_y + 16, image_group(IMAGE_GROUP_FONT_LARGE), 0);
    text_draw(lang_message_data + message->subtitle_text, ui_panel_x + 16, ui_panel_y + 46, image_group(IMAGE_GROUP_FONT_BLACK), 0);
    lang_text_draw(LANG_GROUP_MISSION_GOALS, BRIEFING_TEXT_START, ui_panel_x + 476, ui_panel_y + 417, image_group(IMAGE_GROUP_FONT_BLACK), 0);
    if (window_current_id == WINDOW_BRIEFING && campaign_has_choices(&campaign_object.campaign) != 0) {
        lang_text_draw(LANG_GROUP_WINDOW_HINTS, WINDOW_HINT_CANCEL, ui_panel_x + 50, ui_panel_y + 419, image_group(IMAGE_GROUP_FONT_BLACK), 0);
    }
    g_text_width = 0;
    lang_text_draw(LANG_GROUP_EDITOR, MAP_TEXT_DIFFICULTY, ui_panel_x + 150, ui_panel_y + 417, image_group(IMAGE_GROUP_FONT_BLACK), 0);
    lang_text_draw(LANG_GROUP_DIFFICULTY, settings_data.difficulty + 1, g_text_width + ui_panel_x + 150, ui_panel_y + 417, image_group(IMAGE_GROUP_FONT_BLACK), 0);
    inner_panel_draw(ui_panel_x + 16, ui_panel_y + 64, dialog_text_width_blocks + 1, 6);
    lang_text_draw(LANG_GROUP_MISSION_GOALS, BRIEFING_TEXT_GOALS, ui_panel_x + 32, ui_panel_y + 72, image_group(IMAGE_GROUP_FONT_WHITE), 0);
    label_animation[LABEL_SLOT_REFRESH_COUNTDOWN] = 100;
    draw_goals();
    const char *hint = city_warning_tutorial_hint_text();
    if (hint) {
        draw_goal_label(BRIEFING_TUTORIAL_GOAL, dialog_text_width_blocks - 1);
        text_draw((const uint8_t *) hint, ui_panel_x + goal_position(BRIEFING_TUTORIAL_GOAL)->x + 8,
            ui_panel_y + goal_position(BRIEFING_TUTORIAL_GOAL)->y + 3, image_group(IMAGE_GROUP_FONT_RED), 0);
    }
    draw_briefing_text(message);
}

void victory_button_continue_years(void)
{
    int32_t months = 0;
    city_player *player = city_player_current();
    player->mission_has_won = 1;
    if (button_parameter == CONTINUE_TWO_YEARS) {
        months = CONTINUE_TWO_YEARS_MONTHS;
    } else if (button_parameter == CONTINUE_FIVE_YEARS) {
        months = CONTINUE_FIVE_YEARS_MONTHS;
    }
    if (months != 0) {
        player->mission_continue_months_left += months;
        player->mission_continue_months_chosen = months;
        player->salary_rank = 0;
        player->salary_amount = 0;
        city_finance_estimate_salary();
    }
    city_victory_forced = 0;
    city_victory_state = 0;
    city_victory_cheated = 0;
    window_current_id = WINDOW_CITY;
    window_redraw_request = 1;
}

void defeat_button_menu(void)
{
    city_player *player = city_player_current();
    player->mission_has_won = 0;
    player->mission_continue_months_left = 0;
    player->mission_continue_months_chosen = 0;
    if (free_play_running()) {
        free_play_leave();
    } else {
        window_main_menu_show();
    }
    undo_available = 0;
    window_redraw_request = 1;
}

void defeat_button_easier(void)
{
    int32_t *difficulty = &city_player_current()->difficulty;
    int32_t value = *difficulty - 1;
    if (value > DEFEAT_EASIER_MAX_DIFFICULTY) {
        value = DEFEAT_EASIER_MAX_DIFFICULTY;
    }
    settings_data.difficulty = value;
    *difficulty = value;
    window_current_id = WINDOW_CITY;
    window_redraw_request = 1;
}

generic_button *defeat_buttons(void)
{
    defeat_dialog_button_tables *buttons = &defeat_dialog_buttons;
    int32_t options = city_defeat_options();
    if (options == DEFEAT_OPTIONS_LOWER_DIFFICULTY) {
        return &buttons->choices[DEFEAT_BUTTONS_LOWER_DIFFICULTY];
    }
    if (options == DEFEAT_OPTIONS_REPLAY) {
        return &buttons->choices[DEFEAT_BUTTONS_REPLAY];
    }
    return buttons->menu;
}

void city_victory_lost_cleanup(void)
{
    mission_lost_can_replay = mission_replay_available();
}

void defeat_button_replay(void)
{
    mission_replay();
}

void string_trim(char *text)
{
    while (text[0] != 0 && (uint8_t) text[0] <= WHITE_SPACE_MAX) {
        for (char *p = text; *p != 0; p++) {
            p[0] = p[1];
        }
    }
    char *last = 0;
    for (char *p = text; *p != 0; p++) {
        if ((uint8_t) *p > WHITE_SPACE_MAX) {
            last = p;
        }
    }
    if (last != 0) {
        last[1] = 0;
    }
}

void mission_briefing_set_started(void)
{
    mission_briefing_started = 1;
}

void window_city_intro_draw_background(void)
{
    outer_panel_draw(ui_origin_x + 80, ui_origin_y + 64, 30, 14);
    lang_text_draw_centered_ui(CITY_INTRO_TEXT_GROUP, CITY_INTRO_TEXT_TITLE, ui_origin_x + 80, ui_origin_y + 80, 480, image_group(IMAGE_GROUP_FONT_LARGE), 0, ui_origin_x, ui_origin_y);
    if (empire_city_get(empire_selected_city)->is_sea_trade == 0) {
        lang_text_draw_multiline_ui(
            CITY_INTRO_TEXT_GROUP, CITY_INTRO_TEXT_BODY, ui_origin_x + 112, ui_origin_y + 152, 416, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    } else {
        lang_text_draw_multiline_ui(
            CITY_INTRO_TEXT_GROUP, CITY_INTRO_TEXT_BODY, ui_origin_x + 112, ui_origin_y + 120, 416, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
        lang_text_draw_multiline_ui(
            CITY_INTRO_TEXT_GROUP, CITY_INTRO_TEXT_SEA_TRADE, ui_origin_x + 112, ui_origin_y + 184, 416, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    }
    lang_text_draw_ui(CITY_INTRO_TEXT_GROUP, CITY_INTRO_TEXT_HINT, ui_origin_x + 128, ui_origin_y + 256, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
}
