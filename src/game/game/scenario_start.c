// Starting a mission: a campaign mission from the campaign file or a custom map from maps\, then the favour,
// savings, salary, tutorial stages, build menus, messages, records of the last mission and the difficulty. Also the
// set-up of a new custom map, the map read and the figure and fishing spot reset; the natives' buildings are placed by
// map/natives.c.
//
// Left as the mission start does it: the invasion points outside the map are cleared twice.

#include "game/scenario_start.h"

#include "building/building.h"
#include "building/building_create.h"
#include "building/building_image_group.h"
#include "building/building_state.h"
#include "building/building_storage.h"
#include "building/building_type.h"
#include "building/ferry_landing.h"
#include "building/fort.h"
#include "city/city_init.h"
#include "city/city_player.h"
#include "core/calc.h"
#include "core/crt_compat.h"
#include "core/random.h"
#include "core/static_init.h"
#include "core/text.h"
#include "editor/editor_data.h"
#include "editor/editor_new_scenario.h"
#include "empire/empire_data.h"
#include "empire/empire_helpers.h"
#include "figure/animals_wild.h"
#include "figure/figure.h"
#include "figure/figure_core.h"
#include "figure/figure_type.h"
#include "figure/natives.h"
#include "figure/route.h"
#include "figure/ships_player.h"
#include "game/discord_presence.h"
#include "game/game_data.h"
#include "game/game_internal.h"
#include "game/startup_load.h"
#include "game/time.h"
#include "game/tutorial.h"
#include "graphics/image_collection.h"
#include "input/keyboard_capture.h"
#include "io/empire_file.h"
#include "io/file.h"
#include "io/game_file.h"
#include "map/building_tiles.h"
#include "map/grid.h"
#include "map/natives.h"
#include "map/routing_citizen.h"
#include "map/routing_terrain.h"
#include "map/terrain.h"
#include "map/tiles.h"
#include "platform/platform_time.h"
#include "scenario/campaign.h"
#include "scenario/event_manager.h"
#include "sound/city_view.h"
#include "widget/city_view.h"
#include "widget/widget_data.h"
#include "window/building_menu.h"
#include "window/empire_scroll.h"
#include "window/file_list.h"
#include "window/hotkeys.h"
#include "window/main_menu.h"
#include "window/mission_flow.h"
#include "window/window_id.h"
#include "window/window_image_group.h"

#include <stdint.h>
#include <string.h>

enum scenario_start_rule {
    SCENARIO_MAP_EXTENSION_LENGTH = 3,
    SCENARIO_UNLIMITED_YEARS = 1000000,
    SCENARIO_SALARY_RANK_MAX = 10,
    SCENARIO_LAST_EMPIRE_LOCKED_MISSION = 4,
    SCENARIO_REPLAY_MAP_SELECTION = 3,
    SCENARIO_INVASION_POINT_CHECKS = 2,
    SCENARIO_FISHING_FRAME_MASK = 0x1F,
    SCENARIO_NAME_COUNTER_MASK = 0xF,
    SCENARIO_NAME_INPUT_SLOT = 5,
    SCENARIO_NAME_INPUT_LENGTH = 24,
    SCENARIO_NAME_INPUT_WIDTH = 280,
    SCENARIO_EDITOR_NAME_INPUT_SLOT = 6,
    SCENARIO_EDITOR_NAME_INPUT_LENGTH = 500,
    SCENARIO_EDITOR_NAME_INPUT_WIDTH = 400,
    CAMPAIGN_LINK_NONE_BYTE = 0xFF,
};

void figure_action_profile_init(void)
{
    uint64_t frequency;
    figure_profile_qpc_available = platform_time_frequency(&frequency);
    figure_profile_frequency.low_part = (uint32_t) frequency;
    figure_profile_frequency.high_part = (int32_t) (frequency >> 32);
    memset(figure_action_profile, 0, sizeof(figure_action_profile));
}

int32_t scenario_map_exists(char *name)
{
    file_remove_extension(name);
    memcpy(file_extension, map_file_extension, SCENARIO_MAP_EXTENSION_LENGTH);
    file_append_extension(name);
    int32_t exists = file_exists(name);
    file_remove_extension(name);
    return exists != 0;
}

void scenario_load_map(void)
{
    char path[100];
    map_grids_clear_all();
    file_remove_extension(scenario_map_name);
    memcpy(file_extension, map_file_extension, SCENARIO_MAP_EXTENSION_LENGTH);
    file_append_extension(scenario_map_name);
    strcpy(path, maps_directory);
    strcat(path, scenario_map_name);
    game_file_load_scenario_data(path, 0);
    file_remove_extension(scenario_map_name);
    empire_objects_fix_trade_amounts();
    map_info.grid_width = scenario_data.map_width;
    map_info.grid_height = scenario_data.map_height;
    map_info.grid_start = scenario_data.grid_start;
    map_info.grid_border_size = scenario_data.grid_border_size;
    map_grid_init();
    for (int32_t i = 0; i < SCENARIO_INVASION_POINTS; i++) {
        for (int32_t pass = 0; pass < SCENARIO_INVASION_POINT_CHECKS; pass++) {
            if (map_grid_is_inside(map_info.diamond_x_min + scenario_data.invasion_point_x[i], map_info.diamond_y_min + scenario_data.invasion_point_y[i]) == 0) {
                scenario_data.invasion_point_x[i] = -1;
                scenario_data.invasion_point_y[i] = -1;
            }
        }
    }
    keyboard_start_capture(SCENARIO_NAME_INPUT_SLOT, scenario_data.name, SCENARIO_NAME_INPUT_LENGTH, SCENARIO_NAME_INPUT_WIDTH, 0, image_group(IMAGE_GROUP_FONT_SMALL));
    keyboard_start_capture(SCENARIO_EDITOR_NAME_INPUT_SLOT, scenario_data.briefing, SCENARIO_EDITOR_NAME_INPUT_LENGTH,
        SCENARIO_EDITOR_NAME_INPUT_WIDTH, 0, image_group(IMAGE_GROUP_FONT_SMALL));
    city_view_calculate_lookup(city_view_orientation);
    if (editor_active == 1) {
        figure_create_editor_flags();
        if (scenario_data.river_entry_x != -1 && scenario_data.river_entry_y != -1 && scenario_data.river_exit_x != -1 && scenario_data.river_exit_y != -1) {
            figure_create_flotsam(scenario_data.river_entry_x, scenario_data.river_entry_y, 1);
        }
    }
    figure_create_fishing_points();
    figure_create_herds();
    game_file_initialize_scenario();
    map_tiles_update_all();
    map_natives_init();
    city_view_calculate_map_bounds();
    city_view_check_camera_boundaries();
    map_routing_clear_land_citizen();
    map_routing_update_water();
    map_routing_update_land_citizen();
    map_routing_update_land_noncitizen();
    map_routing_update_walls();
}

void scenario_init_custom(void)
{
    city_current_player = CITY_PLAYER_HUMAN;
    scenario_load_map();
    if (editor_active == 0) {
        int32_t river = 0;
        if (scenario_data.river_entry_x != -1 && scenario_data.river_entry_y != -1 && scenario_data.river_exit_x != -1 && scenario_data.river_exit_y != -1) {
            river = 1;
        }
        figure_create_flotsam(scenario_data.river_entry_x, scenario_data.river_entry_y, river);
    }
    map_routing_update_water();
    map_routing_update_land_citizen();
    map_routing_update_land_noncitizen();
    map_routing_update_walls();
    int16_t entry_x = scenario_data.entry_point_x;
    int16_t entry_y;
    if (entry_x == -1 || (entry_y = scenario_data.entry_point_y) == -1) {
        entry_x = (int16_t) (map_info.grid_width - 1);
        entry_y = (int16_t) (map_info.grid_height / 2);
        scenario_data.entry_point_x = entry_x;
        scenario_data.entry_point_y = entry_y;
    }
    city_player *player = city_player_current();
    player->entry_point_x = (uint8_t) scenario_data.entry_point_x;
    player->entry_point_y = (uint8_t) scenario_data.entry_point_y;
    player->entry_point_grid_offset = map_grid_offset(player->entry_point_x, player->entry_point_y);
    if (scenario_data.exit_point_x == -1 || scenario_data.exit_point_y == -1) {
        scenario_data.exit_point_x = entry_x;
        scenario_data.exit_point_y = entry_y;
    }
    player->exit_point_x = (uint8_t) scenario_data.exit_point_x;
    player->exit_point_y = (uint8_t) scenario_data.exit_point_y;
    player->exit_point_grid_offset = map_grid_offset(player->exit_point_x, player->exit_point_y);
    int32_t treasury = calc_adjust_with_percentage(scenario_data.initial_funds, difficulty_treasury_percentage[settings_data.difficulty]);
    player->treasury = treasury;
    player->balance_last_year = treasury;
    game_time_year = scenario_data.start.year;
    if (scenario_data.time_limit != 0) {
        scenario_max_year = game_time_year + scenario_data.time_limit_years;
    } else if (scenario_data.survival != 0) {
        scenario_max_year = game_time_year + scenario_data.survival_years;
    } else {
        scenario_max_year = game_time_year + SCENARIO_UNLIMITED_YEARS;
    }
    scenario_climate_current = scenario_data.climate;
    empire_object_init_cities();
    traders_clear();
    empire_scroll_x = empire_header[scenario_climate_current].scroll_x;
    empire_scroll_y = empire_header[scenario_climate_current].scroll_y;
    empire_check_scroll_boundaries();
    building_menu_update();
    sidebar_build_buttons_update();
}

void scenario_start_custom(const char *path)
{
    city_victory_state = 0;
    city_victory_forced = 0;
    city_victory_cheated = 0;
    construction_building_type = BUILDING_NONE;
    city_data_init();
    city_current_player = CITY_PLAYER_HUMAN;
    city_players[CITY_PLAYER_HUMAN].enabled = 1;
    city_player_get(CITY_PLAYER_HUMAN)->in_use = 1;
    city_message_init_scenario();
    game_state_init();
    game_animation_init();
    sound_city_init();
    building_menu_enable_all();
    building_clear_all();
    building_storage_clear_all();
    ferry_queues_clear_all();
    ferry_paths_clear_all();
    building_repair_queue_clear();
    figure_init_scenario();
    scenario_invasion_amounts_clear();
    figure_name_init();
    formation_clear_all();
    figure_route_clear_all();
    game_time_init();
    if (editor_active == 1) {
        scenario_climate_current = 0;
        game_file_editor_create_scenario(3);
        empire_file_load(1);
        empire_object_init_cities();
        traders_clear();
        figure_create_editor_flags();
    }
    scenario_init_custom();
    city_current_player = CITY_PLAYER_HUMAN;
    int32_t difficulty = settings_data.difficulty;
    strcpy(city_player_names[1].name, settings_data.player_name);
    city_players[CITY_PLAYER_HUMAN].enabled = 1;
    city_player_get(CITY_PLAYER_HUMAN)->in_use = 1;
    city_player_faction[0].bytes.started = 1;
    city_player_current()->treasury = calc_adjust_with_percentage(city_players[CITY_PLAYER_HUMAN].treasury, difficulty_treasury_percentage[difficulty]);
}

void game_file_start_scenario(const char *name, bool replay)
{
    char path[100];
    int32_t savings = settings_data.personal_savings_last_mission;
    strcpy(path, maps_directory);
    strcat(path, name);
    int32_t mission = settings_data.mission_id;
    uint8_t period = settings_data.campaign;
    uint8_t replay_mode = game_is_replay;
    uint8_t mission_count = (uint8_t) settings_data.current_mission_id;
    int8_t link = settings_data.campaign_link;
    uint8_t previous = settings_data.previous_campaign_link;
    tutorial_legions_message_posted = 1;
    map_bookmarks_reset();
    if (settings_data.is_custom_scenario != 0 && !replay) {
        if (scenario_map_exists(path) == 0) {
            window_current_id = WINDOW_CITY;
            window_redraw_request = 1;
            return;
        }
        scenario_start_custom(path);
    } else {
        strcpy(city_player_names[1].name, settings_data.player_name);
        if (game_file_load_campaign_mission(0, 0) == 0) {
            return;
        }
        settings_data.is_custom_scenario = 0;
        city_player *loaded = city_player_current();
        loaded->treasury = calc_adjust_with_percentage(loaded->treasury, difficulty_treasury_percentage[settings_data.difficulty]);
        strcpy(city_player_names[1].name, settings_data.player_name);
    }
    settings_data.current_mission_id = mission_count;
    settings_data.mission_id = (int8_t) mission;
    settings_data.campaign = period;
    game_is_replay = replay_mode;
    settings_data.personal_savings_last_mission = savings;
    if (replay_mode == SCENARIO_REPLAY_MAP_SELECTION) {
        int8_t found = (int8_t) (uint8_t) campaign_link_of_mission(&campaign_object.campaign, mission);
        settings_data.campaign_link = found;
        if (found < 0) {
            settings_data.campaign_link = 0;
        }
        settings_data.previous_campaign_link = CAMPAIGN_LINK_NONE_BYTE;
    } else {
        settings_data.campaign_link = link;
        settings_data.previous_campaign_link = previous;
    }
    int32_t custom = settings_data.is_custom_scenario;
    settings_data.starting_favor = difficulty_starting_favor[settings_data.difficulty];
    city_player *player = city_player_current();
    player->rating_kingdom = settings_data.starting_favor;
    player->personal_savings = settings_data.personal_savings_last_mission;
    if (custom != 0) {
        player->personal_savings = 0;
    }
    int32_t rank = scenario_data.initial_rank;
    player->player_rank = rank;
    player->salary_rank = rank;
    if (rank > SCENARIO_SALARY_RANK_MAX) {
        player->salary_rank = SCENARIO_SALARY_RANK_MAX;
    }
    saved_reserved_1 = 0;
    scenario_start_flags[SCENARIO_START_FLAG_EMPIRE_LOCKED] = 0;
    player->salary_amount = salary_by_rank[player->salary_rank];
    player->tutorial_flag_seen_a = 1;
    scenario_start_flags[5] = 1;
    scenario_start_flags[7] = 1;
    scenario_start_flags[4] = 1;
    scenario_start_flags[2] = 1;
    scenario_start_flags[3] = 1;
    scenario_start_flags[1] = 1;
    scenario_start_flags[0] = 1;
    player->tutorial_flag_seen_g = 1;
    int8_t m = settings_data.mission_id;
    if (custom == 0) {
        if (m == 0) {
            saved_reserved_1 = 1;
            scenario_start_flags[SCENARIO_START_FLAG_EMPIRE_LOCKED] = 1;
        } else if (m <= SCENARIO_LAST_EMPIRE_LOCKED_MISSION) {
            scenario_start_flags[SCENARIO_START_FLAG_EMPIRE_LOCKED] = 1;
        }
    }
    if (m <= TUTORIAL_LAST_MISSION && custom == 0) {
        memset(tutorial_flags_b, 0, TUTORIAL_MENU_FLAGS);
        for (int32_t i = 0; i < TUTORIAL_COUNT; i++) {
            const tutorial_entry *stage = &tutorial_briefing_tables.tutorial.entries[i];
            uint8_t open = m > stage->mission;
            tutorial_flags_a[i] = open;
            for (int32_t k = 0; k < stage->menu_flag_count; k++) {
                tutorial_flags_b[stage->menu_flags[k]] = open;
            }
        }
    } else {
        memset(tutorial_flags_a, 1, TUTORIAL_COUNT);
        memset(tutorial_flags_b, 1, TUTORIAL_MENU_FLAGS);
    }
    building_menu_update();
    sidebar_build_buttons_update();
    city_message_init_scenario();
    mission_records_apply();
    if (settings_data.is_custom_scenario == 0) {
        player->rating_kingdom = settings_data.starting_favor;
        player->personal_savings = settings_data.personal_savings_last_mission;
    }
    player->difficulty = settings_data.difficulty;
    scenario_data.soldier_images_alt = scenario_data.soldier_images_alt == 1;
    figure_action_profile_init();
    discord_presence_update();
}

void figure_init_scenario(void)
{
    for (int32_t id = 0; id < MAX_FIGURES; id++) {
        memset(figure_get(id), 0, sizeof(figure));
    }
    figure_highest_id = 0;
    figure_unused_counter = 0;
}

void scenario_invasion_amounts_clear(void)
{
    for (scenario_event *e = scenario_invasion_next(&scenario_events, NULL, 1); e; e = scenario_invasion_next(&scenario_events, e, 1)) {
        scenario_event_clear_amounts(e);
    }
    enemy_army_totals.enemy_formations = 0;
    enemy_army_totals.enemy_strength = 0;
    enemy_army_totals.legion_formations = 0;
    enemy_army_totals.legion_strength = 0;
    enemy_army_totals.update_counter = 0;
}

void figure_name_init(void)
{
    for (int32_t i = FIGURE_NAME_COUNTERS - 1; i >= 0; i--) {
        figure_name_counter_table.by_slot[i] = crt_rand() & SCENARIO_NAME_COUNTER_MASK;
    }
}

void figure_create_fishing_points(void)
{
    if (editor_active != 0) {
        return;
    }
    for (int32_t i = 0; i < SCENARIO_FISHING_POINTS; i++) {
        int16_t x = scenario_data.fishing_point_x[i];
        if (x <= 0) {
            continue;
        }
        random_generate_next();
        int32_t id = figure_create(CITY_PLAYER_HUMAN, FIGURE_FISHING_SPOT, x, scenario_data.fishing_point_y[i], 0);
        uint8_t r = (uint8_t) crt_rand();
        figure_get(id)->animation_frame = r & SCENARIO_FISHING_FRAME_MASK;
    }
}

void traders_clear(void)
{
    memset(empire_map_routes.traders, 0, EMPIRE_TRADERS * sizeof(empire_trader));
    empire_map_routes.trader_next_index = 0;
}
