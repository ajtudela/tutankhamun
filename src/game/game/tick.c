// The game tick: what runs once per tick, the dispatcher over the 51 ticks of a day (gods, trade, formations,
// roads, granaries, population, culture, buildings, figures, ...), and the changes of day, month and year with their
// updates; after a map change the water and marsh tiles are redrawn in strips.
//
// The monthly water redraw passes the map's width and height the other way round than the strip redraw, and a royal
// tomb's material request is held by any laborer with the right action, even a dead one.

#include "game/tick.h"

#include "building/building.h"
#include "building/building_figure.h"
#include "building/building_figure_helpers.h"
#include "building/building_industry.h"
#include "building/building_maintenance.h"
#include "building/building_state.h"
#include "building/building_stocks.h"
#include "building/building_type.h"
#include "building/house_evolve.h"
#include "building/house_population.h"
#include "building/house_services.h"
#include "building/monument_spots.h"
#include "building/pyramid_level.h"
#include "building/royal_tomb.h"
#include "building/temple_complex.h"
#include "city/city_player.h"
#include "city/culture.h"
#include "city/finance.h"
#include "city/gods.h"
#include "city/health.h"
#include "city/kingdom.h"
#include "city/labor.h"
#include "city/locusts.h"
#include "city/message.h"
#include "city/population.h"
#include "city/ratings.h"
#include "city/resources.h"
#include "city/sentiment.h"
#include "core/random.h"
#include "empire/empire.h"
#include "figure/action.h"
#include "figure/carriers.h"
#include "figure/figure.h"
#include "figure/figure_action.h"
#include "figure/figure_core.h"
#include "figure/figure_type.h"
#include "figure/formation.h"
#include "flood/flood_internal.h"
#include "flood/flood_types.h"
#include "game/discord_presence.h"
#include "game/game_data.h"
#include "game/game_internal.h"
#include "game/time.h"
#include "game/tutorial.h"
#include "game/undo.h"
#include "io/game_file.h"
#include "map/desirability.h"
#include "map/grid.h"
#include "map/orientation.h"
#include "map/road_network.h"
#include "map/routing_terrain.h"
#include "map/service_visit.h"
#include "map/tiles.h"
#include "map/tiles_road.h"
#include "map/vegetation.h"
#include "map/water_supply.h"
#include "scenario/event_manager.h"
#include "sound/sound_data.h"
#include "sound/sound_lib.h"
#include "widget/widget_data.h"
#include "window/window_id.h"

#include <stdint.h>

enum tick_rule {
    TICK_POPULATION_AGES = 100,
    TICK_BAST_CULTURE_DECAY_SKIP = 10,
    TICK_SENTIMENT_DAY = 8,
    TICK_HALF_YEAR_MONTH = 6,
    TICK_TUTORIAL_POPULATION = 150,
    TICK_OSIRIS_FLOOD_FARMS_DESTROYED = 2,
    TICK_TOMB_KINDS = 4,
    TICK_LABORER_TARGETS = 2,
};

void game_tick_run(void)
{
    random_generate_next();
    game_animation_update_timers();
    game_undo_reduce_time_available();
    if (!editor_active) {
        advance_tick();
        floods_tick_update(&floods_data);
        city_player *player = city_player_current();
        if (floods_data.state == FLOOD_STATE_PEAK && player->osiris_flood_destroys_farms == TICK_OSIRIS_FLOOD_FARMS_DESTROYED) {
            player->osiris_flood_destroys_farms = 0;
        }
    }
    figure_action_handle();
    monuments_animate_all_spots();
    scenario_earthquake_process(&scenario_events);
    city_victory_check();
}

static void update_population(void)
{
    city_player *player = city_player_current();
    player->population = 0;
    for (int32_t age = 0; age < TICK_POPULATION_AGES; age++) {
        player->population += player->population_at_age[age];
    }
    if (player->population > player->population_highest_ever) {
        player->population_highest_ever = player->population;
    }
}

static void decay_culture(void)
{
    city_player *player = city_player_current();
    if (!player->has_temple_complex || building_temple_god(player->temple_complex_building_id) != GOD_BAST) {
        house_service_decay_culture();
    } else if (player->bast_culture_decay_skip) {
        player->bast_culture_decay_skip = 0;
    } else {
        house_service_decay_culture();
        player->bast_culture_decay_skip = TICK_BAST_CULTURE_DECAY_SKIP;
    }
}

static void count_down_music(int32_t *countdown)
{
    if (*countdown > 0 && --*countdown == 0) {
        sound_lib_stop_stream(&sound_manager, SOUND_STREAM_CITY);
        sound_music_playing_special = 0;
    }
}

static void update_map_sweep(void)
{
    if (--map_sweep_countdown == 0) {
        map_tiles_update_region_water(0, 0, map_info.grid_height - 1, map_info.grid_width - 1, 1);
        map_tiles_update_region_marshland(0, 0, map_info.grid_height - 1, map_info.grid_width - 1);
        map_orientation_update_buildings();
        for (int32_t id = 1; id < MAX_FIGURES; id++) {
            figure *f = figure_get(id);
            if (f->state && f->type == FIGURE_FISHING_SPOT) {
                map_figure_add(id);
            }
        }
    } else {
        int32_t step = (map_sweep_total - map_sweep_countdown) / 2;
        if (step < GRID_SIZE) {
            map_tiles_update_region_water(GRID_SIZE + 1 - step, 0, GRID_SIZE + 2 - step, map_info.grid_height - 1, 1);
            map_tiles_update_region_marshland(GRID_SIZE + 1 - step, 0, GRID_SIZE + 2 - step, map_info.grid_height - 1);
            map_orientation_update_buildings();
        }
    }
    window_redraw_request = 1;
}

void advance_tick(void)
{
    switch (game_time_tick) {
        case 1:
            city_gods_calculate_moods(1);
            break;
        case 2:
            empire_update_export_keep();
            break;
        case 3:
            minimap_invalidate_flag = 1;
            break;
        case 4:
            city_emperor_update();
            break;
        case 5:
            formation_update_all(0);
            break;
        case 7:
            map_road_network_update();
            break;
        case 8:
            building_granaries_calculate_stocks();
            break;
        case 10:
            building_update_highest_id();
            break;
        case 11:
            update_population();
            break;
        case 12:
            house_service_decay_houses_covered();
            break;
        case 16:
            city_resource_calculate_warehouse_stocks();
            break;
        case 17:
            city_resource_calculate_food_stocks_and_supply();
            break;
        case 18:
            city_resource_calculate_workshop_stocks();
            break;
        case 19:
            building_dock_update_open_water_access();
            break;
        case 20:
            building_industry_update_production();
            map_vegetation_growth_update();
            break;
        case 21:
            building_maintenance_check_kingdom_access();
            break;
        case 22:
            house_population_update_room();
            break;
        case 23:
            house_population_update_migration();
            break;
        case 24:
            house_population_evict_overcrowded();
            break;
        case 25:
            city_labor_calculate_workers_needed_per_category();
            city_labor_check_employment();
            city_labor_set_building_worker_weight();
            city_labor_allocate_workers_to_buildings();
            break;
        case 27:
            map_water_supply_update_reservoir_fountain();
            break;
        case 28:
            map_water_supply_update_houses();
            break;
        case 29:
            formation_update_all(1);
            break;
        case 30:
            minimap_invalidate_flag = 1;
            break;
        case 31:
            building_figure_generate();
            break;
        case 32:
            empire_city_generate_trader();
            monuments_check_completion();
            break;
        case 33:
            building_count_update();
            city_culture_update_coverage();
            break;
        case 34:
            building_update_money_values();
            break;
        case 35:
            decay_culture();
            break;
        case 36:
            house_service_calculate_culture_aggregates();
            break;
        case 37:
            map_desirability_update_buildings();
            map_desirability_update_terrain();
            break;
        case 38:
            building_update_desirability();
            break;
        case 39:
            building_house_process_evolve_and_consume_goods();
            break;
        case 40:
            building_update_state();
            break;
        case 43:
            building_maintenance_update_burning_ruins();
            break;
        case 44:
            building_maintenance_check_fire_collapse();
            break;
        case 45:
            figure_generate_criminals();
            map_service_visit_decay();
            break;
        case 47:
            building_reset_tick47_counters();
            break;
        case 48:
            house_service_decay_tax_collector();
            break;
        case 49:
            city_culture_calculate();
            break;
    }
    count_down_music(&city_locusts_countdown);
    count_down_music(&city_frogs_countdown);
    if (window_current_id == WINDOW_CITY) {
        count_down_music(&city_hailstorm_countdown);
    }
    if (map_sweep_countdown > 0) {
        update_map_sweep();
    }
    if (pyramid_speedup_countdown > 0 && --pyramid_speedup_countdown == 0) {
        pyramid_speedup_active = 0;
    }
    if (++game_time_tick >= TICKS_PER_DAY) {
        game_time_tick = 0;
        game_tick_advance_day();
    }
    if (pyramid_speedup_active) {
        pyramid_speedup_tick();
    }
}

static void advance_year(void)
{
    game_time_month = 0;
    game_fast_forward_year = 0;
    game_time_year++;
    undo_available = 0;
    city_population_request_yearly_update();
    city_finance_handle_year_change();
    city_finance_year_change_wages();
    city_finance_year_change_trade();
    city_finance_year_change_construction();
    city_finance_year_change_interest();
    city_finance_year_change_salary();
    city_finance_year_change_sundries();
    city_finance_year_change_requests_festivals();
    city_finance_pay_tribute();
    empire_city_reset_yearly_trade_amounts();
    building_maintenance_update_fire_direction();
    city_ratings_update(1);
    temple_complex_ra_kingdom_bonus();
}

static void advance_month(void)
{
    city_player *player = city_player_current();
    player->migration_newcomers_last_month = player->migration_newcomers;
    player->migration_newcomers = 0;
    player->crimes_this_month = 0;
    player->gold_stolen_this_month = 0;
    player->months_since_festival++;
    city_health_update();
    city_finance_collect_monthly_taxes();
    city_finance_pay_monthly_wages();
    city_finance_pay_monthly_interest();
    city_finance_pay_monthly_salary();
    city_finance_update_debt_months();
    empire_update_yearly_trade_stats();
    city_resource_consume_food();
    scenario_invasions_process();
    scenario_events_process_all(&scenario_events);
    city_victory_update_months_to_govern();
    formation_update_monthly_morale_at_rest();
    city_message_decrease_delays();

    map_tiles_update_all_roads();
    map_tiles_update_region_water(0, 0, map_info.grid_width - 1, map_info.grid_height - 1, 0);
    map_routing_update_land_citizen();
    city_message_sort_and_compact();

    game_time_month++;
    city_locusts_update();
    figure_count_by_destination();
    game_tick_update_tomb_material_requests();
    if (game_time_month >= MONTHS_PER_YEAR) {
        advance_year();
    } else {
        city_ratings_update(0);
    }
    if (game_time_month == 0 || game_time_month == TICK_HALF_YEAR_MONTH) {
        player->mission_saved_game_state = MISSION_SAVE_DUE;
    }
    city_population_record_monthly();
    city_festival_update();
    if (!game_fast_forward_year && settings_data.monthly_autosave == 1) {
        game_file_io_write_saved_game(autosave_file_name);
    }
    discord_presence_update();
}

void game_tick_advance_day(void)
{
    game_time_total_days++;
    if (++game_time_day >= DAYS_PER_MONTH) {
        game_time_day = 0;
        advance_month();
    }
    if (game_time_day == 0 || game_time_day == TICK_SENTIMENT_DAY) {
        city_sentiment_update();
        if (game_time_day == TICK_SENTIMENT_DAY) {
            floods_post_flood_prediction_message(&floods_data);
        }
    }
    city_sentiment_update_house_happiness();
    city_player *player = city_player_current();
    if (tutorial_flags_a[TUTORIAL_FLAG_FIRE] && !player->tutorial_flag_seen_a) {
        player->tutorial_flag_seen_a = 1;
        city_message_use_popup = 1;
    }
    if (tutorial_flags_a[TUTORIAL_FLAG_DISEASE] && !player->tutorial_flag_seen_g) {
        player->tutorial_flag_seen_g = 1;
        city_message_use_popup = 1;
        city_message_post(MESSAGE_TUTORIAL_DISEASE, 0, 0, NULL, 0);
    }
    if (!tutorial_flags_a[TUTORIAL_FLAG_POPULATION_150] && player->population >= TICK_TUTORIAL_POPULATION) {
        tutorial_set_flag(TUTORIAL_FLAG_POPULATION_150);
    }
}

static int32_t laborer_works_for_tomb(const figure *f)
{
    if (f->type != FIGURE_LABORER) {
        return 0;
    }
    return f->action_state == FIGURE_ACTION_LABORER_TO_STORAGE_YARD || f->action_state == FIGURE_ACTION_LABORER_AT_STORAGE_YARD ||
        f->action_state == FIGURE_ACTION_LABORER_SLED_TO_TOMB;
}

void game_tick_update_tomb_material_requests(void)
{
    uint8_t *flags[TICK_TOMB_KINDS] = {
        &small_royal_tomb_object.base.material_requested,
        &medium_royal_tomb_object.base.material_requested,
        &large_royal_tomb_object.base.material_requested,
        &grand_royal_tomb_object.base.material_requested,
    };
    static const int16_t tomb_types[TICK_TOMB_KINDS] = {
        BUILDING_SMALL_ROYAL_TOMB, BUILDING_MEDIUM_ROYAL_TOMB, BUILDING_LARGE_ROYAL_TOMB, BUILDING_GRAND_ROYAL_TOMB
    };
    uint8_t laborer_found[TICK_TOMB_KINDS] = { 0, 0, 0, 0 };
    uint8_t requested[TICK_TOMB_KINDS];
    for (int32_t i = 0; i < TICK_TOMB_KINDS; i++) {
        requested[i] = *flags[i] != 0;
    }
    for (int32_t id = 0; id < MAX_FIGURES; id++) {
        const figure *f = figure_get(id);
        if (!laborer_works_for_tomb(f)) {
            continue;
        }
        int16_t targets[TICK_LABORER_TARGETS] = { building_get(f->tomb_building_id)->type, building_get(f->destination_building_id)->type };
        for (int32_t t = 0; t < TICK_LABORER_TARGETS; t++) {
            for (int32_t i = 0; i < TICK_TOMB_KINDS; i++) {
                if (targets[t] == tomb_types[i]) {
                    laborer_found[i] = 1;
                }
            }
        }
    }
    for (int32_t i = 0; i < TICK_TOMB_KINDS; i++) {
        if (requested[i] && !laborer_found[i]) {
            *flags[i] = 0;
        }
    }
}
