// The start of an invasion: enemy formations and figures at a land entry point, or warships and transport ships at
// a sea entry point, by nation (Egyptian, Bedouin or the scenario's enemy). The soldiers are capped and split into up
// to three groups of types, each into up to three formations; at sea warships come first, then transports that land
// at a random landing point. Also the set-up of an enemy warship and the message of a landing.

#include "scenario/invasion.h"

#include "building/destroy_enemy.h"
#include "building/shipwright.h"
#include "city/city_player.h"
#include "city/difficulty.h"
#include "core/calc.h"
#include "core/random.h"
#include "editor/editor_data.h"
#include "figure/enemy.h"
#include "figure/enemy_ships.h"
#include "figure/figure.h"
#include "figure/figure_action.h"
#include "figure/figure_core.h"
#include "figure/figure_type.h"
#include "figure/formation.h"
#include "game/game_data.h"
#include "map/grid.h"
#include "map/terrain.h"
#include "map/water.h"
#include "platform/log.h"
#include "scenario/scenario_internal.h"

#include <stdbool.h>
#include <stdint.h>

enum invasion_limits {
    INVASION_MAX_SOLDIERS = 150,
    INVASION_FORMATION_SIZE = 16,
    INVASION_GROUPS = 3,
    INVASION_GROUP_FORMATIONS = 3,
    INVASION_POINT_RANDOM = 8,
    INVASION_FIRST_SEA_POINT = 9,
    ENEMY_SHIP_FAST_RAM_PERCENT = 20,
};

enum invasion_waits {
    INVASION_FIRST_WAIT = 10,
    INVASION_WARSHIP_WAIT_STEP = 25,
    INVASION_TRANSPORT_WAIT_STEP = 40,
    INVASION_SOLDIER_WAIT_STEP = 10,
    INVASION_FORMATION_WAIT_SOLDIERS = 20,
};

static const int32_t egyptian_types[INVASION_GROUPS] = { FIGURE_ENEMY_EGYPTIAN_INFANTRY, FIGURE_ENEMY_EGYPTIAN_ARCHER, FIGURE_ENEMY_EGYPTIAN_CHARIOT };
static const int32_t bedouin_types[INVASION_GROUPS] = { FIGURE_BEDOUIN, FIGURE_BEDOUIN, FIGURE_BEDOUIN };
static const int32_t enemy_types[INVASION_GROUPS] = { FIGURE_ENEMY_INFANTRY, FIGURE_ENEMY_ARCHER, FIGURE_ENEMY_CHARIOT };

void figure_enemy_ship_init(int32_t figure_id)
{
    figure *f = figure_get(figure_id);
    const figure_type_properties *properties = &figure_properties[f->type];
    f->reserved_9 = 0;
    f->ship_hit_points = properties->max_damage;
    f->ship_max_hit_points = properties->max_damage;
    f->has_troops_on_board = 0;
    f->ship.moves = 0;
    f->ship.steps = 0;
    f->ship.speed = properties->speed;
    f->ship.max_straight_steps = properties->ship_steps;
    f->ship.model_column_m = properties->column_m;
    f->ship.model_column_n = properties->column_n;
    f->ram_damage_slow = properties->attack;
    f->ship_move_mode = 0;
    f->ram_damage_fast = calc_adjust_with_percentage(properties->attack, ENEMY_SHIP_FAST_RAM_PERCENT);
    city_player_current()->enemy_warships++;
}

static int32_t invasion_orientation(int32_t x, int32_t y)
{
    if (y == 0) {
        return DIRECTION_BOTTOM;
    }
    if (y >= scenario_data.map_height - 1) {
        return DIRECTION_TOP;
    }
    if (x == 0) {
        return DIRECTION_RIGHT;
    }
    return (x < scenario_data.map_width - 1) ? DIRECTION_BOTTOM : DIRECTION_LEFT;
}

static int32_t invasion_split(int32_t count, int32_t sizes[INVASION_GROUP_FORMATIONS])
{
    if (count <= 0) {
        return 0;
    }
    if (count <= INVASION_FORMATION_SIZE) {
        sizes[0] = count;
        return 1;
    }
    if (count <= 2 * INVASION_FORMATION_SIZE) {
        sizes[1] = count / 2;
        sizes[0] = count - (count / 2);
        return 2;
    }
    sizes[1] = sizes[2] = count / 3;
    sizes[0] = count - (2 * (count / 3));
    return 3;
}

static void invasion_next_id(void)
{
    if (++invasion_last_id > INVASION_LAST_ID_MAX) {
        invasion_last_id = 1;
    }
}

static int32_t invasion_sea_point(int32_t point)
{
    int32_t index = point - 1;
    for (int32_t tries = SCENARIO_LAND_INVASION_POINTS; tries < SCENARIO_INVASION_POINTS; tries++) {
        if (scenario_data.invasion_point_x[index] != -1) {
            break;
        }
        if (++index >= SCENARIO_INVASION_POINTS) {
            index = SCENARIO_LAND_INVASION_POINTS;
        }
    }
    return index;
}

static void invasion_ship_launch(figure *ship, int32_t figure_id, int32_t action_state, int32_t wait_ticks)
{
    ship->is_friendly = 0;
    ship->action_state = (uint8_t) action_state;
    ship->wait_ticks = (int16_t) wait_ticks;
    figure_name_set(figure_id);
    ship->is_ghost = 1;
    ship->is_boat = 1;
    ship->cart_image_id = 0;
}

static int32_t invasion_start_sea(int32_t nation, int32_t size, int32_t point, int32_t formation_param, int32_t event_id, const scenario_event *event)
{
    if (size >= INVASION_MAX_SOLDIERS) {
        size = INVASION_MAX_SOLDIERS;
    }
    invasion_next_id();
    int32_t landing_x[SCENARIO_INVASION_LANDINGS] = { 0 };
    int32_t landing_y[SCENARIO_INVASION_LANDINGS] = { 0 };
    int32_t landings = 0;
    for (int32_t i = 0; i < SCENARIO_INVASION_LANDINGS; i++) {
        if (scenario_data.invasion_landing_x[i] != -1) {
            landing_x[landings] = scenario_data.invasion_landing_x[i];
            landing_y[landings] = scenario_data.invasion_landing_y[i];
            landings++;
        }
    }
    int32_t index = invasion_sea_point(point);
    int32_t x = scenario_data.invasion_point_x[index];
    int32_t y = scenario_data.invasion_point_y[index];
    if (x == -1 || y == -1) {
        x = scenario_data.exit_point_x;
        y = scenario_data.exit_point_y;
    }
    int32_t orientation = invasion_orientation(x, y);
    int32_t grid_offset = map_grid_offset(x, y);
    int32_t egyptian = nation == ENEMY_EGYPTIAN;
    int32_t warships = (event_id == SCENARIO_CHEAT_INVASION_EVENT) ? (size / 4) : event->phrase_var;
    for (int32_t i = 0; i < warships; i++) {
        int32_t ship_id;
        if (egyptian) {
            ship_id = figure_create(0, FIGURE_ENEMY_EGYPTIAN_WARSHIP, x, y, orientation);
        } else {
            int32_t ship_x = x;
            int32_t ship_y = y;
            map_water_find_free_tile(&ship_x, &ship_y);
            ship_id = figure_create(0, FIGURE_ENEMY_WARSHIP, ship_x, ship_y, orientation);
        }
        figure *ship = figure_get(ship_id);
        invasion_ship_launch(ship, ship_id, FIGURE_ACTION_ENEMY_WARSHIP_WAITING, (i * INVASION_WARSHIP_WAIT_STEP) + INVASION_FIRST_WAIT);
        figure_enemy_ship_init(ship_id);
        ship->ship_leave_month = (uint8_t) game_time_month;
    }
    int32_t landing = 0;
    if (landings > 0) {
        random_generate_next();
        landing = random_generator_state.random1_15bit % landings;
    }
    if (size == 0) {
        return grid_offset;
    }
    int16_t land_x = (int16_t) landing_x[landing];
    int16_t land_y = (int16_t) landing_y[landing];
    int32_t transport_type = egyptian ? FIGURE_ENEMY_EGYPTIAN_TRANSPORT_SHIP : FIGURE_ENEMY_TRANSPORT_SHIP;
    for (int32_t n = 0; size != 0; n++) {
        int32_t soldiers = (size >= ENEMY_TRANSPORT_FORMATION_SIZE) ? ENEMY_TRANSPORT_FORMATION_SIZE : size;
        size -= soldiers;
        int32_t ship_x = x;
        int32_t ship_y = y;
        map_water_find_free_tile(&ship_x, &ship_y);
        int32_t ship_id = figure_create(0, transport_type, ship_x, ship_y, orientation);
        figure *ship = figure_get(ship_id);
        ship->ship_landing_x = land_x;
        ship->ship_landing_y = land_y;
        invasion_ship_launch(ship, ship_id, FIGURE_ACTION_ENEMY_TRANSPORT_WAITING, (n * INVASION_TRANSPORT_WAIT_STEP) + INVASION_FIRST_WAIT);
        boat_init_transport(ship_id);
        ship->has_troops_on_board = 1;
        ship->invasion_soldiers = soldiers;
        ship->invasion_id = invasion_last_id;
        ship->invasion_nation = nation;
        ship->invasion_formation_param = formation_param;
        ship->invasion_event_id = event_id;
    }
    return grid_offset;
}

static int32_t invasion_land_point(int32_t point)
{
    int32_t points = 0;
    for (int32_t i = 0; i < SCENARIO_LAND_INVASION_POINTS; i++) {
        if (scenario_data.invasion_point_x[i] != -1) {
            points++;
        }
    }
    int32_t index;
    if (point == INVASION_POINT_RANDOM) {
        if (points <= 2) {
            index = random_generator_state.random1_7bit & 1;
        } else if (points <= 4) {
            index = random_generator_state.random1_7bit & 3;
        } else {
            index = random_generator_state.random1_7bit & 7;
        }
    } else {
        index = point - 1;
    }
    for (int32_t tries = 0; tries < SCENARIO_LAND_INVASION_POINTS; tries++) {
        if (scenario_data.invasion_point_x[index] != -1) {
            break;
        }
        if (++index >= SCENARIO_LAND_INVASION_POINTS) {
            index = 0;
        }
    }
    return index;
}

static const int32_t *invasion_land_types(int32_t nation)
{
    if (nation == ENEMY_EGYPTIAN) {
        return egyptian_types;
    }
    return (nation == ENEMY_BEDOUIN) ? bedouin_types : enemy_types;
}

static int32_t invasion_group_soldiers(int32_t nation, int32_t total, int32_t type)
{
    int32_t percentage = (nation == ENEMY_BEDOUIN) ? 0 : figure_properties[type].invasion_percentage;
    return calc_adjust_with_percentage(total, percentage);
}

static int32_t invasion_land_blocked(int32_t x, int32_t y, int32_t grid_offset)
{
    uint32_t terrain = map_grids.terrain[grid_offset];
    if (terrain & (TERRAIN_TREE | TERRAIN_ROCK | TERRAIN_ELEVATION)) {
        return 1;
    }
    if (terrain & TERRAIN_WATER) {
        if (!(terrain & TERRAIN_ROAD)) {
            return 1;
        }
    } else if (terrain & (TERRAIN_BUILDING | TERRAIN_CANAL | TERRAIN_WALL | TERRAIN_GATEHOUSE)) {
        building_destroy_by_enemy(x, y, grid_offset);
    }
    return 0;
}

int32_t scenario_invasion_start(int32_t nation, int32_t size, int32_t point, int32_t formation_param, int32_t event_id)
{
    platform_log("invasion: start nation %d size %d point %d param %d event %d", nation, size, point, formation_param, event_id);
    scenario_event *event = 0;
    if (event_id >= 0 && event_id < scenario_events.count) {
        event = &scenario_events.events[event_id];
        event->is_active = 1;
    }
    if (point >= INVASION_FIRST_SEA_POINT && point <= SCENARIO_INVASION_POINTS && nation != ENEMY_BEDOUIN) {
        return invasion_start_sea(nation, size, point, formation_param, event_id, event);
    }
    int32_t total = calc_adjust_with_percentage(size, difficulty_sentiment_base_table[DIFFICULTY_COUNT + settings_data.difficulty]);
    if (total >= INVASION_MAX_SOLDIERS) {
        total = INVASION_MAX_SOLDIERS;
    }
    invasion_next_id();
    const int32_t *types = invasion_land_types(nation);
    int32_t layout = (nation == ENEMY_BEDOUIN) ? FORMATION_ENEMY_MOB : FORMATION_ENEMY_DOUBLE_LINE;
    int32_t second = invasion_group_soldiers(nation, total, types[1]);
    int32_t third = invasion_group_soldiers(nation, total, types[2]);
    int32_t formations[INVASION_GROUPS];
    int32_t sizes[INVASION_GROUPS][INVASION_GROUP_FORMATIONS] = { { 0 } };
    formations[0] = invasion_split(total - third - second, sizes[0]);
    formations[1] = invasion_split(second, sizes[1]);
    formations[2] = invasion_split(third, sizes[2]);

    int32_t x;
    int32_t y;
    if (nation == ENEMY_PERSIAN) {
        x = scenario_data.entry_point_x;
        y = scenario_data.entry_point_y;
    } else {
        int32_t index = invasion_land_point(point);
        x = scenario_data.invasion_point_x[index];
        y = scenario_data.invasion_point_y[index];
    }
    if (x == -1 || y == -1) {
        x = scenario_data.exit_point_x;
        y = scenario_data.exit_point_y;
    }
    int32_t orientation = invasion_orientation(x, y);
    int32_t grid_offset = map_grid_offset(x, y);
    if (invasion_land_blocked(x, y, grid_offset)) {
        return -1;
    }
    int32_t created = 0;
    for (int32_t group = 0; group < INVASION_GROUPS; group++) {
        int32_t type = types[group];
        for (int32_t k = 0; k < formations[group]; k++) {
            int32_t formation_id = formation_create(type, layout, orientation, x, y);
            if (formation_id <= 0) {
                continue;
            }
            formation *m = formation_get(formation_id);
            m->attack_type = (int16_t) formation_param;
            m->orientation = (uint8_t) orientation;
            m->enemy_type = (uint8_t) nation;
            m->invasion_event_id = (int16_t) event_id;
            m->invasion_sequence = invasion_last_id;
            for (int32_t i = 0; i < sizes[group][k]; i++) {
                int32_t figure_id = figure_create(0, type, x, y, orientation);
                figure *f = figure_get(figure_id);
                f->is_friendly = 0;
                f->action_state = FIGURE_ACTION_ENEMY_INITIAL;
                f->wait_ticks = (int16_t) ((i + 1 + (created * INVASION_FORMATION_WAIT_SOLDIERS)) * INVASION_SOLDIER_WAIT_STEP);
                f->formation_id = (int16_t) formation_id;
                figure_name_set(figure_id);
                f->is_ghost = 1;
            }
            created++;
        }
    }
    return grid_offset;
}

void scenario_invasion_landing_message(int32_t event_id, int32_t grid_offset)
{
    if (event_id < 0 || event_id >= scenario_events.count) {
        return;
    }
    scenario_event *event = &scenario_events.events[event_id];
    if (event->type != EVENT_TYPE_INVASION) {
        return;
    }
    int16_t phrases[16];
    int16_t subtype = event->item.value;
    switch (subtype) {
        case 1:
            phrases[1] = 182;
            phrases[8] = 190;
            break;
        case 2:
            phrases[1] = 172;
            phrases[8] = (int16_t) ((scenario_event_parent_by_favour(event) != 0) + 180);
            break;
        case 3:
            phrases[1] = 162;
            phrases[8] = (int16_t) ((scenario_event_parent_by_favour(event) != 0) + 170);
            break;
        case 4:
            phrases[1] = 0xFF;
            phrases[8] = 263;
            break;
        default:
            return;
    }
    scenario_event_fill_phrases(event, phrases);
    switch (subtype) {
        case 1:
            phrases[0] = 296;
            break;
        case 2:
            phrases[0] = 295;
            break;
        case 4:
            phrases[0] = 297;
            break;
        default:
            return;
    }
    city_message_use_popup = 1;
    city_message_post(MESSAGE_ATTACK_ALERT, invasion_last_id, grid_offset, phrases, 0);
}
