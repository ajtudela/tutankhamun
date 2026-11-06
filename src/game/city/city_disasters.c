// The disasters of the gods' curses and of the scenario events: health changes, all ships sunk, the plague, the
// hailstorm, the frogs, the river of blood, a building collapse and the mummies.

#include "city/city_disasters.h"

#include "building/building_data.h"
#include "building/building_type.h"
#include "building/house_population.h"
#include "city/city_data.h"
#include "city/city_internal.h"
#include "city/health.h"
#include "city/locusts.h"
#include "city/sentiment.h"
#include "core/random.h"
#include "figure/animals.h"
#include "figure/figure.h"
#include "figure/figure_action.h"
#include "figure/figure_type.h"
#include "game/game_data.h"
#include "map/building_tiles.h"
#include "map/map_data.h"
#include "map/road_access.h"
#include "map/routing_terrain.h"
#include "map/terrain.h"
#include "platform/platform_system.h"
#include "sound/sound_data.h"
#include "sound/sound_lib.h"
#include "sound/sound_play.h"
#include "widget/widget_data.h"

#include <math.h>
#include <stdbool.h>
#include <stdio.h>

static void play_disaster_sound(const char *file)
{
    if (sound_lib_stream_is_playing(&sound_manager, SOUND_STREAM_MUSIC)) {
        sound_lib_stop_stream(&sound_manager, SOUND_STREAM_MUSIC);
    }
    sound_music_playing_special = 1;
    if (sound_lib_stream_is_playing(&sound_manager, SOUND_STREAM_CITY)) {
        sound_lib_stop_stream(&sound_manager, SOUND_STREAM_CITY);
    }
    sound_lib_play_stream(&sound_manager, file, SOUND_STREAM_CITY, 1);
}

void city_health_change(int32_t amount)
{
    city_player *player = city_player_current();
    int32_t health = player->health + amount;
    player->health = health;
    if (health > 100) {
        player->health = 100;
    }
    if (player->health < 0) {
        player->health = 0;
    }
}

void city_locusts_start(void)
{
    city_locusts_countdown = LOCUST_COUNTDOWN_RUN_OUT;
    city_sentiment_change_happiness(-DISASTER_HAPPINESS_LOSS);
}

void figure_sink_all_ships(void)
{
    for (int32_t id = 1; id < MAX_FIGURES; id++) {
        figure *ship = figure_get(id);
        uint8_t type = ship->type;
        if (ship->state == FIGURE_STATE_ALIVE && (type == FIGURE_FISHING_BOAT || type == FIGURE_TRANSPORT_SHIP || type == FIGURE_WARSHIP)) {
            int32_t building_id = ship->building_id;
            ship->building_id = 0;
            ship->type = FIGURE_SHIPWRECK;
            ship->wait_ticks = 0;
            building_get(building_id)->data.dock.ship_id = 0;
        }
    }
}

static int32_t house_can_catch_plague(const building *b)
{
    return b->state != BUILDING_STATE_UNUSED && b->house_population != 0 && b->house_size != 0;
}

int32_t city_health_find_plague_house(void)
{
    int32_t best = 0;
    int32_t best_value = 0;
    for (int32_t id = 1; id <= building_highest_id; id++) {
        building *b = building_get(id);
        if (house_can_catch_plague(b) && b->house_plague_risk > best_value) {
            best = id;
            best_value = b->house_plague_risk;
        }
    }
    return best;
}

bool city_health_start_plague_at_house(int32_t building_id)
{
    if (building_id == 0) {
        return 0;
    }
    city_health_infect_house(building_id);
    city_player_current()->plague_count++;
    const building *b = building_get(building_id);
    int32_t figure_id = figure_create(city_current_player, FIGURE_PLAGUED_CITIZEN, b->x, b->y, 0);
    if (figure_id != 0) {
        figure *plagued = figure_get(figure_id);
        plagued->wait_ticks = PLAGUE_FIGURE_WAIT_TICKS;
        plagued->building_id = (int16_t) building_id;
        plagued->action_state = FIGURE_ACTION_PLAGUED_CITIZEN_CREATED;
        figure_movement_init_roaming(figure_id);
    }
    return 1;
}

bool city_health_start_plague(bool random_house)
{
    int32_t chosen = city_health_find_plague_house();
    if (random_house && chosen == 0) {
        int32_t count = 0;
        for (int32_t id = 1; id <= building_highest_id; id++) {
            if (!house_can_catch_plague(building_get(id))) {
                continue;
            }
            count++;
            random_generate_next();
            if (((int64_t) (uint32_t) random_generator_state.random1_15bit % count) == 0) {
                chosen = id;
            }
        }
    }
    if (!city_health_start_plague_at_house(chosen)) {
        return 0;
    }
    city_view.disease_message_other_building = chosen;
    city_view.disease_message_other = 1;
    return 1;
}

void city_hailstorm_start(void)
{
    for (int32_t id = 1; id < MAX_FIGURES; id++) {
        figure *f = figure_get(id);
        if (f->state == FIGURE_STATE_NONE) {
            continue;
        }
        random_generate_next();
        int32_t roll = (int32_t) ((uint32_t) random_generator_state.random1_15bit % 100);
        int32_t chance;
        uint8_t type = f->type;
        if (f->action_state == HAILSTORM_ACTION_CREATED) {
            chance = HAILSTORM_CHANCE_CREATED;
        } else if (type == FIGURE_ARCHER || type == FIGURE_INFANTRY || type == FIGURE_CHARIOTEER ||
            (type > FIGURE_TOWER_SENTRY && type < FIGURE_NATIVE_TRADER) || type == FIGURE_BEDOUIN) {
            chance = HAILSTORM_CHANCE_FIGHTER;
        } else if (figure_is_predator_animal(type)) {
            chance = HAILSTORM_CHANCE_FIGHTER;
        } else {
            chance = figure_is_boat(f->type) ? HAILSTORM_CHANCE_FIGHTER : HAILSTORM_CHANCE_OTHER;
        }
        if (roll <= chance) {
            f->action_state = FIGURE_ACTION_CORPSE;
        }
    }
    play_disaster_sound(str_hailstorm_mp3);
    city_sentiment_change_happiness(-DISASTER_HAPPINESS_LOSS);
    city_hailstorm_countdown = HAILSTORM_COUNTDOWN;
}

static int32_t find_nearest_terrain(int32_t x, int32_t y, uint32_t terrain, int32_t *out_x, int32_t *out_y, int32_t rings)
{
    return animal_find_nearest_figure_of_types(x, y, 0, NULL, terrain, out_x, out_y, rings, 0);
}

int32_t city_frogs_find_water(int32_t building_id, int32_t *x, int32_t *y)
{
    int32_t limit = (map_info.grid_width < map_info.grid_height) ? map_info.grid_height : map_info.grid_width;
    const building *b = building_get(building_id);
    int32_t marsh_x;
    int32_t marsh_y;
    int32_t water_x;
    int32_t water_y;
    int32_t marsh = limit;
    if (find_nearest_terrain(b->x, b->y, TERRAIN_MARSH, &marsh_x, &marsh_y, limit)) {
        int32_t dy = b->y - marsh_y;
        int32_t dx = b->x - marsh_x;
        marsh = crt_ftol_double(sqrt((double) ((dy * dy) + (dx * dx))));
    }
    int32_t water = limit;
    int32_t found = find_nearest_terrain(b->x, b->y, TERRAIN_WATER, &water_x, &water_y, limit);
    if (found) {
        int32_t dy = b->y - water_y;
        int32_t dx = b->x - water_x;
        water = crt_ftol_double(sqrt((double) ((dy * dy) + (dx * dx))));
    }
    if (marsh > water && water < limit) {
        *x = water_x;
        *y = water_y;
        return 1;
    }
    if (marsh < water && marsh < limit) {
        *x = marsh_x;
        *y = marsh_y;
        return 1;
    }
    return 0;
}

void city_frogs_create(int32_t building_id, int32_t x, int32_t y)
{
    int32_t figure_id = figure_create(city_current_player, FIGURE_FROG, x, y, 0);
    figure *frog = figure_get(figure_id);
    frog->action_state = FIGURE_ACTION_FROG_CREATED;
    frog->immigrant_building_id = (int16_t) building_id;
    building_get(building_id)->immigrant_figure_id = (int16_t) figure_id;
    random_generate_next();
    frog->wait_ticks = (int16_t) ((uint32_t) random_generator_state.random1_15bit % FROG_MAX_START_DELAY);
    frog->migrant_num_people = 0;
    frog->terrain_usage = TERRAIN_USAGE_AMPHIBIA;
    char text[500];
    sprintf(text, frog_debug_format, figure_id, x, y, building_id);
    platform_debug_output(text);
}

void city_frogs_infest_house(int32_t building_id)
{
    random_generate_next();
    uint32_t r = (uint32_t) random_generator_state.random1_15bit;
    int32_t x;
    int32_t y;
    if ((r % 100) < FROG_HOUSE_CHANCE && city_frogs_find_water(building_id, &x, &y)) {
        city_frogs_create(building_id, x, y);
    }
}

void city_frogs_start(int32_t amount)
{
    int32_t sum = 0;
    int32_t level = 0;
    int32_t people = 0;
    city_frogs_countdown = amount * DISASTER_TICKS_PER_AMOUNT;
    house_population_fill_building_list_with_houses();
    for (int32_t i = 0; i < building_list_large_size; i++) {
        const building *b = building_get(building_list_large[i]);
        if (b->type > level) {
            level = b->type;
        }
        people += b->house_population;
    }
    int32_t target = (people * FROGS_PEOPLE_PERCENT) / 100;
    for (; level >= BUILDING_HOUSE_CRUDE_HUT && sum < target; level--) {
        for (int32_t i = 0; i < building_list_large_size; i++) {
            int32_t id = building_list_large[i];
            const building *b = building_get(id);
            if (b->type == level) {
                sum += b->house_population;
                if (sum <= target) {
                    city_frogs_infest_house(id);
                }
            }
        }
    }
    play_disaster_sound(frogs_sound_file);
    city_sentiment_change_happiness(-DISASTER_HAPPINESS_LOSS);
}

void city_blood_river_start(int32_t amount)
{
    if (amount < BLOOD_RIVER_MIN_AMOUNT) {
        amount = BLOOD_RIVER_MIN_AMOUNT;
    } else if (amount > BLOOD_RIVER_MAX_AMOUNT) {
        amount = BLOOD_RIVER_MAX_AMOUNT;
    }
    for (int32_t id = 1; id <= building_highest_id; id++) {
        building *b = building_get(id);
        if (b->house_size == 0) {
            continue;
        }
        int32_t distance = b->house_water_distance;
        int32_t risk = BLOOD_RIVER_RISK / (distance + 1);
        b->house_malaria_risk += risk;
        b->house_plague_risk += risk;
        int32_t water_supply_cap = distance * BLOOD_RIVER_WATER_SUPPLY_PER_DISTANCE;
        if (water_supply_cap < (int32_t) b->data.house.water_supply) {
            b->data.house.water_supply = (uint8_t) water_supply_cap;
        }
    }
    for (int32_t id = 1; id < MAX_FIGURES; id++) {
        figure *f = figure_get(id);
        if (f->state == FIGURE_STATE_NONE) {
            continue;
        }
        uint8_t type = f->type;
        int32_t boat_is_out = (f->action_state == FIGURE_ACTION_FISHING_BOAT_FISHING) || (f->action_state == FIGURE_ACTION_FISHING_BOAT_GOING_TO_FISH);
        if (type == FIGURE_FISHING_SPOT) {
            map_figure_delete(id);
        } else if (type == FIGURE_FISHING_BOAT && boat_is_out) {
            f->wait_ticks = 0;
            f->action_state = FIGURE_ACTION_FISHING_BOAT_RETURNING_WITH_FISH;
            f->destination_x = f->source_x;
            f->destination_y = f->source_y;
            figure_route_remove(id);
        }
    }
    map_sweep_total = amount * DISASTER_TICKS_PER_AMOUNT;
    map_sweep_countdown = map_sweep_total;
    city_sentiment_change_happiness(-DISASTER_HAPPINESS_LOSS);
}

int32_t building_destroy_first_of_type(int32_t type)
{
    for (int32_t id = 1; id <= building_highest_id; id++) {
        building *b = building_get(id);
        if (b->state != BUILDING_STATE_IN_USE || b->type != type) {
            continue;
        }
        undo_available = 0;
        int32_t grid_offset = b->grid_offset;
        b->state = BUILDING_STATE_RUBBLE;
        map_building_tiles_set_rubble(id, b->x, b->y, b->size, b->size);
        sound_effect_play(SOUND_EFFECT_BUILDING_DESTROYED);
        int32_t size = b->size;
        map_routing_update_land_citizen_rect(b->x - size, b->y - size, b->x + size, b->y + size, 1);
        size = b->size;
        map_routing_update_land_noncitizen_rect(b->x - size, b->y - size, b->x + size, b->y + size);
        return grid_offset;
    }
    return 0;
}

bool city_mummies_is_source_building(int32_t building_id)
{
    const building *b = building_get(building_id);
    int32_t type = b->type;
    if ((type > BUILDING_MUD_TOWER && type < BUILDING_TEMPLE_COMPLEX_OSIRIS) || type == BUILDING_MORTUARY || type == BUILDING_PYRAMID ||
        type == BUILDING_MAUSOLEUM || type == BUILDING_SMALL_ROYAL_TOMB || type == BUILDING_MEDIUM_ROYAL_TOMB || type == BUILDING_LARGE_ROYAL_TOMB ||
        type == BUILDING_GRAND_ROYAL_TOMB) {
        if (map_closest_road_within_radius(b->road_access_x, b->road_access_y, 1, MUMMY_ROAD_RADIUS)) {
            return 1;
        }
    }
    return 0;
}

void city_mummies_start(int32_t amount)
{
    mummies_max = amount;
    int32_t count = 0;
    for (int32_t id = 1; id <= building_highest_id; id++) {
        if (city_mummies_is_source_building(id)) {
            count++;
        }
    }
    if (count <= 0) {
        return;
    }
    random_generate_next();
    int32_t wanted = (int32_t) ((uint32_t) random_generator_state.random1_15bit % (uint32_t) count) + 1;
    int32_t seen = 0;
    int32_t id = 1;
    for (; id <= building_highest_id; id++) {
        if (city_mummies_is_source_building(id) && (++seen == wanted)) {
            break;
        }
    }
    if (id > building_highest_id) {
        return;
    }
    const building *b = building_get(id);
    map_closest_road_within_radius(b->road_access_x, b->road_access_y, 1, MUMMY_ROAD_RADIUS);
    int32_t figure_id = figure_create(city_current_player, FIGURE_MUMMY, map_result_x, map_result_y, 0);
    figure *mummy = figure_get(figure_id);
    mummy->building_id = (int16_t) id;
    mummy->action_state = FIGURE_ACTION_MUMMY_ROAMING;
    mummy->wait_ticks = 0;
    figure_movement_init_roaming(figure_id);
    mummy->roam_length = 0;
}
