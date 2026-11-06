// The rarer disaster events of a scenario: the kingdom's reputation decrease, the collapse of a gold mine, the
// plagues (frogs, hailstorm, the river turned to blood), the crime wave and event type 29, each with its message and
// follow-up event; and the earthquake, whose four fronts move one tile every `delay` ticks, burning down the buildings
// on their way, until amount * 25 steps are done.

#include "building/building.h"
#include "building/building_maintenance.h"
#include "building/building_type.h"
#include "city/city_disasters.h"
#include "city/crime.h"
#include "figure/military.h"
#include "map/grid.h"
#include "map/routing_terrain.h"
#include "map/terrain.h"
#include "map/tiles.h"
#include "map/tiles_road.h"
#include "scenario/scenario_internal.h"
#include "scenario/scenario_rules.h"
#include "sound/sound_play.h"

#include <stdbool.h>
#include <stdint.h>

enum earthquake_state {
    EARTHQUAKE_STATE_DUE = 0,
    EARTHQUAKE_STATE_RUNNING = 1,
    EARTHQUAKE_STATE_OVER = 2,
};

enum earthquake_rules {
    EARTHQUAKE_STEPS_PER_AMOUNT = 25,
};

static void chain_completed(scenario_event *e)
{
    if (scenario_event_is_chain_target(e, e->on_completed)) {
        int32_t phrase = scenario_event_reason_phrase(e, 0);
        scenario_event_chain(e, e->on_completed, phrase);
    }
}

void scenario_event_process_reputation_decrease(scenario_event *e)
{
    if (!scenario_event_is_due(e, 0)) {
        return;
    }
    int16_t phrases[EVENT_PHRASES];
    scenario_event_init_phrases(e, phrases);
    city_message_use_popup = 1;
    city_message_post(MESSAGE_TEMPLATE_GENERAL, e->id, 0, phrases, 0);
    city_ratings_change_kingdom(-e->amount.value);
    city_player_current()->kingdom_ignored_request_penalty = e->amount.value;
    e->flags |= EVENT_FLAG_FIRED;
    e->is_active = 0;
    chain_completed(e);
}

void scenario_event_process_gold_mine_collapse(scenario_event *e)
{
    if (!scenario_event_is_due(e, 0)) {
        return;
    }
    e->flags |= EVENT_FLAG_FIRED;
    int32_t grid_offset = building_destroy_first_of_type(BUILDING_GOLD_MINE);
    if (grid_offset) {
        city_message_use_popup = 1;
        int16_t phrases[EVENT_PHRASES];
        scenario_event_init_phrases(e, phrases);
        city_message_post(MESSAGE_GOLD_MINE_COLLAPSE, 0, grid_offset, phrases, 0);
    }
    chain_completed(e);
}

static int32_t plague_fires(scenario_event *e)
{
    if (!scenario_event_is_due(e, 0)) {
        return 0;
    }
    e->flags |= EVENT_FLAG_FIRED;
    return 1;
}

static void plague_announce(int32_t message)
{
    city_message_use_popup = 1;
    city_message_post(message, 0, 0, NULL, 0);
}

void scenario_event_process_frogs(scenario_event *e)
{
    if (plague_fires(e)) {
        city_frogs_start(e->amount.value);
        plague_announce(MESSAGE_FROGS);
        chain_completed(e);
    }
}

void scenario_event_process_hailstorm(scenario_event *e)
{
    if (plague_fires(e)) {
        plague_announce(MESSAGE_HAILSTORM);
        city_hailstorm_start();
        chain_completed(e);
    }
}

void scenario_event_process_blood_river(scenario_event *e)
{
    if (plague_fires(e)) {
        city_blood_river_start(e->amount.value);
        plague_announce(MESSAGE_BLOOD_RIVER);
        chain_completed(e);
    }
}

void scenario_event_process_crime_wave(scenario_event *e)
{
    if (plague_fires(e)) {
        city_crime_wave(e->amount.value);
        plague_announce(MESSAGE_CRIME_WAVE);
        chain_completed(e);
    }
}

void scenario_event_process_mummies(scenario_event *e)
{
    if (plague_fires(e)) {
        city_mummies_start(e->amount.value);
        plague_announce(MESSAGE_MUMMY_ATTACK);
        chain_completed(e);
    }
}

bool scenario_earthquake_tile_free(int32_t x, int32_t y)
{
    return (map_grids.terrain[map_grid_offset(x, y)] & (TERRAIN_ROCK | TERRAIN_WATER | TERRAIN_ELEVATION)) == 0;
}

void scenario_earthquake_destroy_tile(int32_t x, int32_t y)
{
    int32_t grid_offset = map_grid_offset(x, y);
    int32_t building_id = map_grids.building_ids[grid_offset];
    if (building_id) {
        building_destroy_by_fire(building_id, 0);
        building_destroy_linked_parts(building_id, 1);
        sound_effect_play(SOUND_EFFECT_BUILDING_DESTROYED);
    }
    if (map_grids.building_ids[grid_offset]) {
        city_buildings.records[map_grids.building_ids[grid_offset]].state = BUILDING_STATE_DELETED_BY_GAME;
    }
    map_grids.terrain[grid_offset] = 0;
    map_tiles_set_fault_image(x, y, 1);
    map_tiles_update_all_gardens();
    map_tiles_update_all_roads();
    map_tiles_update_all_plazas(0, 0, map_info.grid_width - 1, map_info.grid_height - 1, 1);
    map_routing_update_land_citizen_rect(x - 1, y - 1, x + 1, y + 1, 1);
    map_routing_update_land_noncitizen_rect(x - 1, y - 1, x + 1, y + 1);
    map_routing_update_walls();
    figure_create_explosion_cloud(x, y, 1, 1);
}

static int32_t clamp_to(int32_t value, int32_t max)
{
    if (value < 0) {
        value = 0;
    }
    if (value > max) {
        value = max;
    }
    return value;
}

static bool earthquake_start(scenario_event *e)
{
    if (!scenario_event_is_due(e, 0)) {
        return 0;
    }
    scenario_earthquake_data *quake = &e->data.earthquake;
    e->is_active = 1;
    e->flags |= EVENT_FLAG_FIRED;
    e->state = EARTHQUAKE_STATE_RUNNING;
    quake->duration = 0;
    quake->ticks = 0;
    scenario_earthquake_destroy_tile(quake->locations[0], quake->locations[1]);
    city_message_use_popup = 1;
    int32_t grid_offset = map_grid_offset(quake->locations[0], quake->locations[1]);
    int16_t phrases[EVENT_PHRASES];
    scenario_event_init_phrases(e, phrases);
    city_message_post(MESSAGE_EARTHQUAKE, 0, grid_offset, phrases, 0);
    return 1;
}

bool scenario_earthquake_event_process(scenario_event *e)
{
    if (e->type != EVENT_TYPE_EARTHQUAKE) {
        return 0;
    }
    if (e->state == EARTHQUAKE_STATE_DUE) {
        return earthquake_start(e);
    }
    if (e->state == EARTHQUAKE_STATE_OVER) {
        return 0;
    }
    scenario_earthquake_data *quake = &e->data.earthquake;
    if (++quake->ticks < quake->delay) {
        return 1;
    }
    quake->ticks = 0;
    int32_t steps = e->amount.value * EARTHQUAKE_STEPS_PER_AMOUNT;
    if (steps < EARTHQUAKE_STEPS_PER_AMOUNT) {
        steps = EARTHQUAKE_STEPS_PER_AMOUNT;
    }
    if (++quake->duration >= steps) {
        e->state = EARTHQUAKE_STATE_OVER;
        e->is_active = 0;
    }
    int32_t r = random_generator_state.random1_7bit & (EARTHQUAKE_STEPS - 1);
    e->earthquake_step = (int16_t) r;
    const earthquake_step *step = &earthquake_steps[r];
    int16_t *front = &quake->locations[2 * step->front];
    int32_t x = clamp_to(front[0] + step->dx, map_info.grid_width - 1);
    int32_t y = clamp_to(front[1] + step->dy, map_info.grid_height - 1);
    if (scenario_earthquake_tile_free(x, y)) {
        front[0] = (int16_t) x;
        front[1] = (int16_t) y;
        scenario_earthquake_destroy_tile(x, y);
    }
    return 1;
}
