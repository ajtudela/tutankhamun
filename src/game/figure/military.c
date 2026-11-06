// Soldiers and combat: the figure sounds of deaths and hits, melee combat between figures that meet (who attacks
// whom by category, the fight, a charge's hit, experience and morale), missiles and the soldiers' target search, the
// explosion cloud, the thief and the tomb robber, the legions' standard, tower sentries and ballistas, the player's
// warships and transport ships, and the soldiers of the legions. The combat balance and timings are military_rules
// and military_difficulties; the chariot's cart offsets, the transports' escapes and the soldiers' alternative spots
// are config tables too.
//
// The combat and ship code has these known oddities:
// - the experience bonus to a defender comes from the attacker's formation;
// - a charge's hit uses the attacker's own defense;
// - the explosion of a building larger than 5 reads past the two size tables as the original does: the tile offset
//   of size 6 and up comes from the cloud offsets, drifts and speeds that follow military_size_tile_offsets, and the
//   cloud offset of size 6 and up from the drifts and speeds after explosion_size_cloud_offsets (both tables' sizes
//   are EXPLOSION_SIZES; no building reaches size 54, where the original's run went on into other tables);
// - the ship actions call figure_next_route_direction and drop its result.

#include "figure/military.h"
#include "building/building_data.h"
#include "building/building_figure_helpers.h"
#include "building/building_state.h"
#include "building/building_type.h"
#include "building/mausoleum.h"
#include "building/monument.h"
#include "building/monument_access.h"
#include "building/royal_tomb.h"
#include "city/city_data.h"
#include "city/finance.h"
#include "city/gods.h"
#include "city/kingdom.h"
#include "city/message.h"
#include "city/ratings.h"
#include "city/resource_type.h"
#include "city/resources.h"
#include "core/calc.h"
#include "core/crt_compat.h"
#include "core/random.h"
#include "figure/boats.h"
#include "figure/carriers.h"
#include "figure/enemy_ships.h"
#include "figure/figure.h"
#include "figure/figure_action.h"
#include "figure/figure_core.h"
#include "figure/figure_data.h"
#include "figure/figure_image_group.h"
#include "figure/figure_type.h"
#include "figure/military_internal.h"
#include "figure/missiles.h"
#include "figure/movement.h"
#include "figure/ships_player.h"
#include "flood/flood_internal.h"
#include "graphics/image_collection.h"
#include "map/grid.h"
#include "map/map_data.h"
#include "map/routing.h"
#include "map/routing_enemy.h"
#include "map/terrain.h"
#include "map/tiles.h"
#include "map/water.h"
#include "sound/sound_play.h"
#include <stdint.h>

int32_t military_rules[MILITARY_RULE_COUNT] = {
    [MILITARY_RULE_MELEE_HIT_TICKS] = 24,
    [MILITARY_RULE_FLANK_ATTACK_BONUS] = 4,
    [MILITARY_RULE_LINE_ATTACK_BONUS] = 4,
    [MILITARY_RULE_MOP_UP_ATTACK_BONUS] = 2,
    [MILITARY_RULE_LINE_DEFENSE_BONUS] = 4,
    [MILITARY_RULE_MOP_UP_DEFENSE_PENALTY] = 2,
    [MILITARY_RULE_FACING_AWAY_DEFENSE_PENALTY] = 4,
    [MILITARY_RULE_KINGDOM_SOLDIER_LINE_BONUS] = 4,
    [MILITARY_RULE_MAX_EXPERIENCE] = 100,
    [MILITARY_RULE_CHARGE_MIN_STEPS] = 5,
    [MILITARY_RULE_CHARGE_MAX_STEPS] = 20,
    [MILITARY_RULE_CHARGE_ATTACK_FACTOR] = 4,
    [MILITARY_RULE_JAVELIN_DAMAGE] = 10,
    [MILITARY_RULE_BOLT_DAMAGE] = 60,
    [MILITARY_RULE_MISSILE_ATTACK_TIMEOUT] = 6,
    [MILITARY_RULE_MISSILE_FACING_AWAY_PENALTY] = 4,
    [MILITARY_RULE_MISSILE_SINGLE_LINE_BONUS] = 4,
    [MILITARY_RULE_MISSILE_MOP_UP_PENALTY] = 2,
    [MILITARY_RULE_MISSILE_DOCKED_WARSHIP_BONUS] = 2,
    [MILITARY_RULE_TARGET_PRIORITY_ENEMY_SOLDIER] = 50,
    [MILITARY_RULE_TARGET_PRIORITY_ANIMAL] = 10,
    [MILITARY_RULE_TARGET_PRIORITY_LOADED_TRANSPORT] = 40,
    [MILITARY_RULE_TARGET_PRIORITY_TRANSPORT] = 20,
    [MILITARY_RULE_TARGET_PRIORITY_WARSHIP] = 30,
    [MILITARY_RULE_SENTRY_REST_TICKS] = 40,
    [MILITARY_RULE_SENTRY_SEARCH_TICKS] = 40,
    [MILITARY_RULE_SENTRY_SHOOT_DISTANCE] = 10,
    [MILITARY_RULE_BALLISTA_SEARCH_TICKS] = 20,
    [MILITARY_RULE_BALLISTA_AIM_TICKS] = 10,
    [MILITARY_RULE_BALLISTA_RELOAD_TICKS] = 23,
    [MILITARY_RULE_WARSHIP_HUNT_RANGE] = 228,
    [MILITARY_RULE_WARSHIP_PATROL_EXTRA_RANGE] = 10,
    [MILITARY_RULE_WARSHIP_PATROL_RADIUS] = 10,
    [MILITARY_RULE_SHIP_DAMAGED_PERCENTAGE] = 15,
    [MILITARY_RULE_TRANSPORT_EVADE_TICKS] = 75,
    [MILITARY_RULE_TRANSPORT_EVADE_RANGE] = 18,
    [MILITARY_RULE_SOLDIER_ENGAGE_RADIUS] = 6,
    [MILITARY_RULE_SOLDIER_MOP_UP_RANGE] = 20,
    [MILITARY_RULE_SOLDIER_ATTACK_FORMATION_RANGE] = 100,
    [MILITARY_RULE_CRIMINAL_WAIT_TICKS] = 40,
    [MILITARY_RULE_CRIMINAL_SPEED_FACTOR] = 2,
    [MILITARY_RULE_CART_THIEF_FLEE_TICKS] = 10,
    [MILITARY_RULE_CART_THEFT_AMOUNT] = 100,
    [MILITARY_RULE_TOMB_ROBBER_KILLED_RATING] = 1,
};

military_difficulty military_difficulties[DIFFICULTY_COUNT] = {
    [DIFFICULTY_VERY_EASY] = { 3, -2 },
    [DIFFICULTY_EASY] = { 6, -4 },
    [DIFFICULTY_NORMAL] = { 9, -6 },
    [DIFFICULTY_HARD] = { 12, -8 },
    [DIFFICULTY_VERY_HARD] = { 15, -10 },
};

military_offset chariot_cart_offsets[DIRECTION_COUNT] = {
    [DIRECTION_TOP] = { 22, -18 },
    [DIRECTION_TOP_RIGHT] = { 27, -3 },
    [DIRECTION_RIGHT] = { 23, 17 },
    [DIRECTION_BOTTOM_RIGHT] = { 0, 11 },
    [DIRECTION_BOTTOM] = { -21, 17 },
    [DIRECTION_BOTTOM_LEFT] = { -25, -1 },
    [DIRECTION_LEFT] = { -23, -17 },
    [DIRECTION_TOP_LEFT] = { 0, -12 },
};

transport_escape transport_escapes[DIRECTION_COUNT] = {
    [DIRECTION_TOP] = { { 0, 10, -10 }, { -10, -10, -10 } },
    [DIRECTION_TOP_RIGHT] = { { 0, 10, 10 }, { -10, -10, 0 } },
    [DIRECTION_RIGHT] = { { 10, 10, 10 }, { -10, 0, 10 } },
    [DIRECTION_BOTTOM_RIGHT] = { { 10, 10, 0 }, { 0, 10, 10 } },
    [DIRECTION_BOTTOM] = { { 10, 0, -10 }, { 10, 10, 10 } },
    [DIRECTION_BOTTOM_LEFT] = { { 0, -10, -10 }, { 10, 0, 10 } },
    [DIRECTION_LEFT] = { { -10, -10, -10 }, { 0, 10, -10 } },
    [DIRECTION_TOP_LEFT] = { { -10, -10, 0 }, { 0, -10, -10 } },
};

military_offset soldier_alternative_spots[SOLDIER_ALTERNATIVE_SPOTS] = { { 0, -1 }, { 1, -1 }, { 1, 0 }, { 1, 1 }, { 0, 1 }, { -1, 1 }, { -1, 0 }, { -1, -1 },
    { 0, -2 }, { 1, -2 }, { 2, -2 }, { 2, -1 }, { 2, 0 }, { 2, 1 }, { 2, 2 }, { 1, 2 }, { 0, 2 }, { -1, 2 }, { -2, 2 }, { -2, 1 }, { -2, 0 }, { -2, -1 },
    { -2, -2 }, { -1, -2 }, { 0, -3 }, { 1, -3 }, { 2, -3 }, { 3, -3 }, { 3, -2 }, { 3, -1 }, { 3, 0 }, { 3, 1 }, { 3, 2 }, { 3, 3 }, { 2, 3 }, { 1, 3 },
    { 0, 3 }, { -1, 3 }, { -2, 3 }, { -3, 3 }, { -3, 2 }, { -3, 1 }, { -3, 0 }, { -3, -1 }, { -3, -2 }, { -3, -3 }, { -2, -3 }, { -1, -3 }, { 0, -4 },
    { 1, -4 }, { 2, -4 }, { 3, -4 }, { 4, -4 }, { 4, -3 }, { 4, -2 }, { 4, -1 }, { 4, 0 }, { 4, 1 }, { 4, 2 }, { 4, 3 }, { 4, 4 }, { 3, 4 }, { 2, 4 }, { 1, 4 },
    { 0, 4 }, { -1, 4 }, { -2, 4 }, { -3, 4 }, { -4, 4 }, { -4, 3 }, { -4, 2 }, { -4, 1 }, { -4, 0 }, { -4, -1 }, { -4, -2 }, { -4, -3 }, { -4, -4 },
    { -3, -4 }, { -2, -4 }, { -1, -4 }, { 0, -5 }, { 1, -5 }, { 2, -5 }, { 3, -5 }, { 4, -5 }, { 5, -5 }, { 5, -4 }, { 5, -3 }, { 5, -2 }, { 5, -1 }, { 5, 0 },
    { 5, 1 }, { 5, 2 }, { 5, 3 }, { 5, 4 }, { 5, 5 }, { 4, 5 }, { 3, 5 }, { 2, 5 }, { 1, 5 }, { 0, 5 }, { -1, 5 }, { -2, 5 }, { -3, 5 }, { -4, 5 }, { -5, 5 },
    { -5, 4 }, { -5, 3 }, { -5, 2 }, { -5, 1 }, { -5, 0 }, { -5, -1 }, { -5, -2 }, { -5, -3 }, { -5, -4 }, { -5, -5 }, { -4, -5 }, { -3, -5 }, { -2, -5 },
    { -1, -5 }, { 0, -6 }, { 1, -6 }, { 2, -6 }, { 3, -6 }, { 4, -6 }, { 5, -6 }, { 6, -6 }, { 6, -5 }, { 6, -4 }, { 6, -3 }, { 6, -2 }, { 6, -1 }, { 6, 0 },
    { 6, 1 }, { 6, 2 }, { 6, 3 }, { 6, 4 }, { 6, 5 }, { 6, 6 }, { 5, 6 }, { 4, 6 }, { 3, 6 }, { 2, 6 }, { 1, 6 }, { 0, 6 }, { -1, 6 }, { -2, 6 }, { -3, 6 },
    { -4, 6 }, { -5, 6 }, { -6, 6 }, { -6, 5 }, { -6, 4 }, { -6, 3 }, { -6, 2 }, { -6, 1 }, { -6, 0 }, { -6, -1 }, { -6, -2 }, { -6, -3 }, { -6, -4 },
    { -6, -5 }, { -6, -6 }, { -5, -6 }, { -4, -6 }, { -3, -6 }, { -2, -6 }, { -1, -6 } };

void figure_play_die_sound(int32_t figure_id)
{
    int32_t sound;
    switch (figure_get(figure_id)->type) {
        case FIGURE_CHARIOTEER:
        case FIGURE_ENEMY_CHARIOT:
        case FIGURE_ENEMY_EGYPTIAN_CHARIOT:
            sound = FIGURE_SOUND_CHARIOT;
            break;
        case FIGURE_MARKET_TRADER:
            sound = FIGURE_SOUND_MARKET_TRADER + ((crt_rand() * FIGURE_SOUND_MARKET_TRADER_COUNT) / 32768);
            break;
        case FIGURE_BIRDS:
            sound = FIGURE_SOUND_BIRDS;
            break;
        case FIGURE_OSTRICH:
            sound = FIGURE_SOUND_OSTRICH;
            break;
        case FIGURE_ANTELOPE:
            sound = FIGURE_SOUND_ANTELOPE;
            break;
        case FIGURE_TRANSPORT_SHIP:
        case FIGURE_WARSHIP:
        case FIGURE_ENEMY_TRANSPORT_SHIP:
        case FIGURE_ENEMY_WARSHIP:
        case FIGURE_ENEMY_EGYPTIAN_WARSHIP:
        case FIGURE_ENEMY_EGYPTIAN_TRANSPORT_SHIP:
            sound = FIGURE_SOUND_SHIP;
            break;
        case FIGURE_CROCODILE:
            sound = FIGURE_SOUND_CROCODILE;
            break;
        case FIGURE_HYENA:
            sound = FIGURE_SOUND_HYENA;
            break;
        case FIGURE_HIPPO:
            sound = FIGURE_SOUND_HIPPO;
            break;
        case FIGURE_ASP:
            sound = FIGURE_SOUND_ASP;
            break;
        case FIGURE_LION:
            sound = FIGURE_SOUND_LION;
            break;
        case FIGURE_SCORPION:
            sound = FIGURE_SOUND_SCORPION;
            break;
        default:
            sound = FIGURE_SOUND_CITIZEN + ((crt_rand() * FIGURE_SOUND_CITIZEN_COUNT) / 32768);
            break;
    }
    sound_figure_play(figure_id, sound, FIGURE_SOUND_VARIANT_DIE);
}

void figure_play_hit_sound(int32_t figure_id)
{
    switch (figure_get(figure_id)->type) {
        case FIGURE_ARCHER:
        case FIGURE_ENEMY_ARCHER:
        case FIGURE_ENEMY_EGYPTIAN_ARCHER:
            sound_figure_play(figure_id, FIGURE_SOUND_ARCHER, FIGURE_SOUND_VARIANT_HIT);
            return;
        case FIGURE_INFANTRY:
        case FIGURE_ENEMY_INFANTRY:
        case FIGURE_ENEMY_EGYPTIAN_INFANTRY:
            sound_figure_play(figure_id, FIGURE_SOUND_INFANTRY, FIGURE_SOUND_VARIANT_HIT);
            return;
        case FIGURE_CHARIOTEER:
        case FIGURE_ENEMY_CHARIOT:
        case FIGURE_ENEMY_EGYPTIAN_CHARIOT:
        case FIGURE_BEDOUIN:
            sound_figure_play(figure_id, FIGURE_SOUND_CHARIOT, FIGURE_SOUND_VARIANT_HIT);
            return;
        case FIGURE_WARSHIP:
        case FIGURE_ENEMY_WARSHIP:
        case FIGURE_ENEMY_EGYPTIAN_WARSHIP:
            sound_figure_play(figure_id, FIGURE_SOUND_SHIP, FIGURE_SOUND_VARIANT_HIT);
            return;
    }
}

void figure_set_state_dead(int32_t figure_id)
{
    figure_get(figure_id)->state = FIGURE_STATE_DEAD;
}

int32_t attack_is_same_direction(int32_t dir1, int32_t dir2)
{
    if (dir1 == dir2) {
        return 1;
    }
    int32_t left = dir2 - 1;
    if (left < 0) {
        left = DIRECTION_TOP_LEFT;
    }
    if (dir1 == left) {
        return 1;
    }
    int32_t right = dir2 + 1;
    if (right >= DIRECTION_COUNT) {
        right = 0;
    }
    return dir1 == right;
}

void figure_combat_handle_corpse(int32_t figure_id)
{
    figure *f = figure_get(figure_id);
    if (f->wait_ticks < 0) {
        f->wait_ticks = 0;
    }
    if (++f->wait_ticks > FIGURE_CORPSE_LAST_TICK) {
        f->wait_ticks = FIGURE_CORPSE_LAST_TICK;
        f->state = FIGURE_STATE_DEAD;
    }
}

void figure_combat_resume_activity_after_attack(int32_t figure_id)
{
    figure *f = figure_get(figure_id);
    f->num_attackers = 0;
    f->action_state = f->action_state_before_attack;
    f->opponent_id = 0;
    f->attacker_id1 = 0;
    f->attacker_id2 = 0;
    figure_route_remove(figure_id);
}

static int32_t category_attacks(int32_t category, int32_t opponent_category, int32_t opponent_action)
{
    switch (category) {
        case 1:
            return opponent_category == 7;
        case 2:
            if (opponent_category == 5) {
                return opponent_action == FIGURE_ACTION_NATIVE_ATTACKING;
            }
            return opponent_category == 4 || opponent_category == 3 || opponent_category == 7;
        case 3:
            return opponent_category == 1 || opponent_category == 2 || opponent_category == 4 || opponent_category == 7;
        case 4:
            return opponent_category == 2 || opponent_category == 3 || opponent_category == 7;
        case 6:
            return opponent_category == 7;
        case 7:
            return opponent_category == 1 || opponent_category == 2 || opponent_category == 3 || opponent_category == 4 || opponent_category == 6;
    }
    return 0;
}

void figure_combat_attack_figure_at(int32_t figure_id, int32_t opponent_id)
{
    figure *f = figure_get(figure_id);
    int32_t category = figure_properties[f->type].category;
    if (category <= 0 || category >= 8) {
        return;
    }
    if (f->action_state == FIGURE_ACTION_SOLDIER_CHARGE) {
        figure_combat_hit_opponent(figure_id, opponent_id);
        return;
    }
    if (figure_get(opponent_id)->action_state == FIGURE_ACTION_SOLDIER_CHARGE || f->action_state == FIGURE_ACTION_ATTACK) {
        return;
    }
    int32_t attack = 0;
    city_player *player = city_player_current();
    for (int32_t guard = 1; opponent_id > 0;) {
        figure *opponent = figure_get(opponent_id);
        if (opponent_id != figure_id) {
            switch (opponent->type) {
                case FIGURE_FISHING_BOAT:
                case FIGURE_TRANSPORT_SHIP:
                case FIGURE_WARSHIP:
                case FIGURE_ENEMY_TRANSPORT_SHIP:
                case FIGURE_ENEMY_WARSHIP:
                case FIGURE_ENEMY_EGYPTIAN_WARSHIP:
                case FIGURE_ENEMY_EGYPTIAN_TRANSPORT_SHIP:
                    break;
                default:
                    if (opponent->state != FIGURE_STATE_ALIVE || opponent->action_state == FIGURE_ACTION_CORPSE) {
                        attack = 0;
                    } else if (category_attacks(category, figure_properties[opponent->type].category, opponent->action_state)) {
                        attack = 1;
                    }
                    if (f->type == FIGURE_CRIMINAL && opponent->type == FIGURE_CART_PUSHER && opponent->resource_id == RESOURCE_GOLD) {
                        f->action_state = FIGURE_ACTION_CRIMINAL_FLEEING;
                        f->wait_ticks = 0;
                        opponent->state = FIGURE_STATE_DEAD;
                        player->thefts++;
                        return;
                    }
                    if (f->type == FIGURE_CART_PUSHER && f->resource_id == RESOURCE_GOLD && opponent->type == FIGURE_CRIMINAL) {
                        opponent->action_state = FIGURE_ACTION_CRIMINAL_FLEEING;
                        opponent->wait_ticks = 0;
                        f->state = FIGURE_STATE_DEAD;
                        return;
                    }
                    if (f->type == FIGURE_HERBALIST && opponent->type == FIGURE_PLAGUED_CITIZEN) {
                        opponent->state = FIGURE_STATE_DEAD;
                        return;
                    }
                    if (f->type == FIGURE_PLAGUED_CITIZEN && opponent->type == FIGURE_HERBALIST) {
                        f->state = FIGURE_STATE_DEAD;
                        return;
                    }
                    if (player->has_temple_complex && building_temple_god(player->temple_complex_building_id) == GOD_BAST &&
                        building_temple_complex_has_upgrade(player->temple_complex_building_id, 1)) {
                        if (f->type == FIGURE_PRIEST && building_temple_god(f->building_id) == GOD_BAST && opponent->type == FIGURE_PLAGUED_CITIZEN) {
                            opponent->state = FIGURE_STATE_DEAD;
                            return;
                        }
                        if (f->type == FIGURE_PLAGUED_CITIZEN && opponent->type == FIGURE_PRIEST && building_temple_god(opponent->building_id) == GOD_BAST) {
                            f->state = FIGURE_STATE_DEAD;
                            return;
                        }
                    }
                    if (attack) {
                        if (opponent->action_state != FIGURE_ACTION_ATTACK || opponent->num_attackers < 2) {
                            f->action_state_before_attack = f->action_state;
                            f->action_state = FIGURE_ACTION_ATTACK;
                            f->opponent_id = (int16_t) opponent_id;
                            f->attacker_id1 = (int16_t) opponent_id;
                            f->num_attackers = 1;
                            f->attack_image_offset = 12;
                            int32_t direction;
                            if (opponent->x == opponent->destination_x && opponent->y == opponent->destination_y) {
                                direction = calc_general_direction(f->previous_tile_x, f->previous_tile_y, opponent->x, opponent->y);
                            } else {
                                direction = calc_general_direction(
                                    f->previous_tile_x, f->previous_tile_y, opponent->previous_tile_x, opponent->previous_tile_y);
                            }
                            f->attack_direction = (int8_t) direction;
                            if (f->attack_direction >= DIRECTION_COUNT) {
                                f->attack_direction = 0;
                            }
                            if (opponent->action_state != FIGURE_ACTION_ATTACK) {
                                opponent->action_state_before_attack = opponent->action_state;
                                opponent->action_state = FIGURE_ACTION_ATTACK;
                                opponent->attack_image_offset = 0;
                                int8_t back = (int8_t) (f->attack_direction + (DIRECTION_COUNT / 2));
                                opponent->attack_direction = back;
                                if (back >= DIRECTION_COUNT) {
                                    opponent->attack_direction = (int8_t) (back - DIRECTION_COUNT);
                                }
                            }
                            if (opponent->num_attackers == 0) {
                                opponent->attacker_id1 = (int16_t) figure_id;
                                opponent->opponent_id = (int16_t) figure_id;
                                opponent->num_attackers = 1;
                            } else if (opponent->num_attackers == 1) {
                                opponent->attacker_id2 = (int16_t) figure_id;
                                opponent->num_attackers = 2;
                            }
                            return;
                        }
                        attack = 0;
                    }
                    break;
            }
        }
        opponent_id = opponent->next_figure_id_on_same_tile;
        if (++guard >= 1000) {
            return;
        }
    }
}

void figure_combat_attack_adjacent(int32_t figure_id)
{
    int32_t offset = city_figures.records[figure_id].grid_offset;
    for (int32_t d = 0; d < DIRECTION_COUNT; d++) {
        if (city_figures.records[figure_id].action_state == FIGURE_ACTION_ATTACK) {
            return;
        }
        switch (city_figures.records[figure_id].type) {
            case FIGURE_FISHING_BOAT:
            case FIGURE_TRANSPORT_SHIP:
            case FIGURE_WARSHIP:
            case FIGURE_ENEMY_TRANSPORT_SHIP:
            case FIGURE_ENEMY_WARSHIP:
            case FIGURE_ENEMY_EGYPTIAN_WARSHIP:
            case FIGURE_ENEMY_EGYPTIAN_TRANSPORT_SHIP:
                break;
            default:
                figure_combat_attack_figure_at(figure_id, map_grids.figure_ids[map_grid_direction_delta[d] + offset]);
                break;
        }
    }
}

int32_t figure_target_is_alive(int32_t figure_id)
{
    const figure *f = figure_get(figure_id);
    int32_t target_id = f->target_figure_id;
    if (target_id <= 0) {
        return 0;
    }
    const figure *target = figure_get(target_id);
    if (target->state != FIGURE_STATE_ALIVE || target->action_state == FIGURE_ACTION_CORPSE) {
        return 0;
    }
    return target->created_sequence == f->target_figure_created_sequence;
}

static int32_t formation_faces(int32_t formation_id, int32_t direction)
{
    return attack_is_same_direction(direction, formation_get(formation_id)->direction);
}

void figure_combat_handle_attack(int32_t figure_id)
{
    figure *f = figure_get(figure_id);
    int32_t formation_id = f->formation_id;
    formation *legion = formation_get(formation_id);
    if (f->progress_on_tile <= 5) {
        f->progress_on_tile++;
        figure_movement_advance_tick(figure_id);
    }
    int32_t num_attackers = f->num_attackers;
    if (num_attackers == 0) {
        figure_combat_resume_activity_after_attack(figure_id);
        return;
    }
    if (num_attackers == 1) {
        figure *opponent = figure_get(f->opponent_id);
        if (opponent->state != FIGURE_STATE_ALIVE || opponent->action_state == FIGURE_ACTION_CORPSE) {
            figure_combat_resume_activity_after_attack(figure_id);
            return;
        }
    } else if (num_attackers == 2) {
        int32_t opponent_id = f->opponent_id;
        figure *opponent = figure_get(opponent_id);
        if (opponent->state != FIGURE_STATE_ALIVE || opponent->action_state == FIGURE_ACTION_CORPSE) {
            if (opponent_id == f->attacker_id1) {
                f->opponent_id = f->attacker_id2;
            } else if (opponent_id == f->attacker_id2) {
                f->opponent_id = f->attacker_id1;
            }
            opponent_id = f->opponent_id;
            opponent = figure_get(opponent_id);
            if (opponent->state != FIGURE_STATE_ALIVE || opponent->action_state == FIGURE_ACTION_CORPSE) {
                figure_combat_resume_activity_after_attack(figure_id);
                return;
            }
            f->num_attackers = 1;
            f->attacker_id1 = (int16_t) opponent_id;
            f->attacker_id2 = 0;
        }
    }
    if (++f->attack_image_offset < military_rules[MILITARY_RULE_MELEE_HIT_TICKS]) {
        return;
    }
    int32_t opponent_id = f->opponent_id;
    figure *opponent = figure_get(opponent_id);
    int32_t opponent_formation = opponent->formation_id;
    formation *opponent_legion = formation_get(opponent_formation);
    int32_t opponent_category = figure_properties[opponent->type].category;
    f->attack_image_offset = opponent_category == 1 || opponent_category == 4 ? 12 : 0;
    int32_t experience_bonus = 0;
    if (opponent->faction_id == city_current_player) {
        experience_bonus = (legion->experience + 10) / 20;
    }
    int32_t attack = figure_properties[f->type].attack;
    int32_t defense = figure_properties[opponent->type].defense + experience_bonus;
    if (defense >= 20) {
        defense = 20;
    }
    if (opponent->opponent_id != figure_id && attack_is_same_direction(f->attack_direction, opponent->attack_direction) &&
        (legion->figure_type == FIGURE_INFANTRY || legion->figure_type == FIGURE_CHARIOTEER)) {
        attack += military_rules[MILITARY_RULE_FLANK_ATTACK_BONUS];
    }
    if (legion->is_halted) {
        int32_t type = legion->figure_type;
        if (type == FIGURE_ARCHER || type == FIGURE_CHARIOTEER) {
            if (formation_faces(formation_id, f->attack_direction) && legion->layout == FORMATION_MOP_UP) {
                attack += military_rules[MILITARY_RULE_MOP_UP_ATTACK_BONUS];
            }
        } else if (type == FIGURE_INFANTRY) {
            if (formation_faces(formation_id, f->attack_direction)) {
                int32_t layout = legion->layout;
                if (layout > FORMATION_COLUMN) {
                    if (layout <= FORMATION_DOUBLE_LINE_2) {
                        attack += military_rules[MILITARY_RULE_LINE_ATTACK_BONUS];
                    } else if (layout == FORMATION_MOP_UP) {
                        attack += military_rules[MILITARY_RULE_MOP_UP_ATTACK_BONUS];
                    }
                }
            }
        }
    }
    if (opponent_legion->is_halted) {
        int32_t type = opponent_legion->figure_type;
        if (type == FIGURE_INFANTRY) {
            if (formation_faces(opponent_formation, opponent->attack_direction)) {
                int32_t layout = opponent_legion->layout;
                if (layout > FORMATION_COLUMN) {
                    if (layout <= FORMATION_DOUBLE_LINE_2) {
                        defense += military_rules[MILITARY_RULE_LINE_DEFENSE_BONUS];
                    } else if (layout == FORMATION_MOP_UP) {
                        defense -= military_rules[MILITARY_RULE_MOP_UP_DEFENSE_PENALTY];
                    }
                }
            } else {
                defense -= military_rules[MILITARY_RULE_FACING_AWAY_DEFENSE_PENALTY];
            }
        } else if (type == FIGURE_ARCHER || type == FIGURE_CHARIOTEER) {
            if (formation_faces(opponent_formation, opponent->attack_direction)) {
                if (opponent_legion->layout == FORMATION_MOP_UP) {
                    defense -= military_rules[MILITARY_RULE_MOP_UP_DEFENSE_PENALTY];
                }
            } else {
                defense -= military_rules[MILITARY_RULE_FACING_AWAY_DEFENSE_PENALTY];
            }
        }
    }
    if (legion->figure_type == FIGURE_KINGDOM_SOLDIER && formation_faces(formation_id, f->attack_direction)) {
        int32_t layout = legion->layout;
        if (layout > FORMATION_COLUMN && layout <= FORMATION_DOUBLE_LINE_2) {
            attack += military_rules[MILITARY_RULE_KINGDOM_SOLDIER_LINE_BONUS];
        }
    }
    if (opponent_legion->figure_type == FIGURE_KINGDOM_SOLDIER && formation_faces(opponent_formation, opponent->attack_direction)) {
        int32_t layout = opponent_legion->layout;
        if (layout > FORMATION_COLUMN && layout <= FORMATION_DOUBLE_LINE_2) {
            defense += military_rules[MILITARY_RULE_KINGDOM_SOLDIER_LINE_BONUS];
        }
    }
    int32_t max_damage = figure_properties[opponent->type].max_damage;
    if (defense < 0) {
        defense = 0;
    }
    if (defense > 20) {
        defense = 20;
    }
    int32_t damage = 0;
    if (attack != 0) {
        damage = ((20 - defense) * attack) / 20;
        if (damage < 0) {
            damage = 0;
        }
    }
    opponent->damage = (uint8_t) (opponent->damage + damage);
    if (opponent->damage <= max_damage) {
        figure_play_hit_sound(figure_id);
        return;
    }
    if (!figure_is_predator_animal(opponent->type) && !figure_is_herd_animal(opponent->type) && opponent->type != FIGURE_CRIMINAL &&
        f->faction_id == city_current_player) {
        int32_t experience = legion->experience + 1;
        if (experience >= military_rules[MILITARY_RULE_MAX_EXPERIENCE]) {
            experience = military_rules[MILITARY_RULE_MAX_EXPERIENCE];
        }
        legion->experience = (int16_t) experience;
    }
    opponent->action_state = FIGURE_ACTION_CORPSE;
    opponent->wait_ticks = 0;
    figure_play_die_sound(opponent_id);
    formation_update_morale_after_death(opponent_formation);
}

static int32_t charge_is_at_full_strength(int32_t steps)
{
    return (steps > military_rules[MILITARY_RULE_CHARGE_MIN_STEPS]) && (steps < military_rules[MILITARY_RULE_CHARGE_MAX_STEPS]);
}

void figure_combat_hit_opponent(int32_t figure_id, int32_t opponent_id)
{
    figure *f = figure_get(figure_id);
    int32_t formation_id = f->formation_id;
    figure *opponent = figure_get(opponent_id);
    int32_t opponent_formation = opponent->formation_id;
    formation *legion = formation_get(formation_id);
    if (figure_id == opponent_id || opponent->faction_id == city_current_player) {
        return;
    }
    int32_t attack = figure_properties[f->type].attack;
    int32_t defense = figure_properties[f->type].defense;
    int32_t charge = f->charge.steps;
    if (charge_is_at_full_strength(charge)) {
        attack *= military_rules[MILITARY_RULE_CHARGE_ATTACK_FACTOR];
    }
    int32_t damage = 0;
    if (attack != 0) {
        damage = ((30 - defense) / 20) * attack;
        if (damage < 0) {
            damage = 0;
        }
    }
    opponent->damage = (uint8_t) (opponent->damage + damage);
    if (opponent->damage <= figure_properties[opponent->type].max_damage) {
        figure_play_hit_sound(figure_id);
        return;
    }
    if (!figure_is_predator_animal(opponent->type) && !figure_is_herd_animal(opponent->type) && opponent->type != FIGURE_CRIMINAL &&
        f->faction_id == city_current_player) {
        int32_t experience = legion->experience + 1;
        if (experience >= military_rules[MILITARY_RULE_MAX_EXPERIENCE]) {
            experience = military_rules[MILITARY_RULE_MAX_EXPERIENCE];
        }
        legion->experience = (int16_t) experience;
    }
    opponent->action_state = FIGURE_ACTION_CORPSE;
    opponent->wait_ticks = 0;
    figure_play_die_sound(opponent_id);
    formation_update_morale_after_death(opponent_formation);
}

int32_t figure_create_missile(int32_t figure_id, int32_t x, int32_t y, int32_t dst_x, int32_t dst_y, int32_t type)
{
    int32_t missile_id = figure_create(city_current_player, type, x, y, 0);
    if (!missile_id) {
        return 0;
    }
    figure_play_hit_sound(figure_id);
    figure *m = figure_get(missile_id);
    m->missile_damage = (uint8_t) ((type == FIGURE_BOLT) ? military_rules[MILITARY_RULE_BOLT_DAMAGE] : military_rules[MILITARY_RULE_JAVELIN_DAMAGE]);
    m->building_id = (int16_t) figure_id;
    m->destination_x = (int16_t) dst_x;
    m->destination_y = (int16_t) dst_y;
    figure_movement_set_cross_country_direction(missile_id, m->cross_country.x, m->cross_country.y, m->destination_x * 15, (int16_t) dst_y * 15, 1);
    m->direction = (int8_t) calc_missile_angle(m->x, m->y, m->destination_x, m->destination_y);
    return missile_id;
}

int32_t figure_combat_get_missile_target_for_soldier(int32_t figure_id, int32_t max_distance)
{
    figure *f = figure_get(figure_id);
    int32_t x = f->x;
    int32_t y = f->y;
    int32_t formation_id = f->formation_id;
    int32_t best_id = 0;
    int32_t best_priority = 0;
    figure_search_best_distance = max_distance;
    for (int32_t id = 1; id < MAX_FIGURES; id++) {
        figure *target = figure_get(id);
        if (target->state != FIGURE_STATE_ALIVE || target->action_state == FIGURE_ACTION_CORPSE) {
            continue;
        }
        int32_t type = target->type;
        if (type != FIGURE_ENEMY_TRANSPORT_SHIP && type != FIGURE_ENEMY_EGYPTIAN_TRANSPORT_SHIP && type != FIGURE_ENEMY_WARSHIP &&
            type != FIGURE_ENEMY_EGYPTIAN_WARSHIP) {
            int32_t target_formation = formation_get(formation_id)->attack_formation_id;
            if (target_formation > 0 && target->formation_id != target_formation) {
                continue;
            }
        }
        int32_t priority;
        switch (type) {
            case FIGURE_ENEMY_ARCHER:
            case FIGURE_ENEMY_INFANTRY:
            case FIGURE_ENEMY_CHARIOT:
            case FIGURE_ENEMY_EGYPTIAN_INFANTRY:
            case FIGURE_ENEMY_EGYPTIAN_ARCHER:
            case FIGURE_ENEMY_EGYPTIAN_CHARIOT:
            case FIGURE_BEDOUIN:
                priority = military_rules[MILITARY_RULE_TARGET_PRIORITY_ENEMY_SOLDIER];
                break;
            case FIGURE_CROCODILE:
            case FIGURE_HYENA:
            case FIGURE_HIPPO:
            case FIGURE_ASP:
            case FIGURE_LION:
            case FIGURE_SCORPION:
            case FIGURE_MUMMY:
                priority = military_rules[MILITARY_RULE_TARGET_PRIORITY_ANIMAL];
                break;
            case FIGURE_ENEMY_TRANSPORT_SHIP:
            case FIGURE_ENEMY_EGYPTIAN_TRANSPORT_SHIP:
                priority = military_rules[target->has_troops_on_board ? MILITARY_RULE_TARGET_PRIORITY_LOADED_TRANSPORT
                                                                      : MILITARY_RULE_TARGET_PRIORITY_TRANSPORT];
                break;
            case FIGURE_ENEMY_WARSHIP:
            case FIGURE_ENEMY_EGYPTIAN_WARSHIP:
                priority = military_rules[MILITARY_RULE_TARGET_PRIORITY_WARSHIP];
                break;
            default:
                continue;
        }
        int32_t distance = calc_maximum_distance(x, y, target->x, target->y);
        if (best_priority < priority && distance < figure_search_best_distance &&
            (f->action_state == FIGURE_ACTION_SOLDIER_ATTACKING_FORMATION || figure_movement_can_launch_cross_country_missile(x, y, target->x, target->y))) {
            best_id = id;
            figure_search_best_distance = distance;
            best_priority = priority;
        }
    }
    if (!best_id) {
        return 0;
    }
    map_result_y = figure_get(best_id)->y;
    map_result_x = figure_get(best_id)->x;
    return best_id;
}

static int32_t view_direction(const figure *f)
{
    int32_t dir = f->direction - map_info.orientation;
    if (dir < 0) {
        dir += DIRECTION_COUNT;
    }
    return dir;
}

static int32_t explosion_cloud_word(int32_t index)
{
    if (index < EXPLOSION_SIZES) {
        return explosion_size_cloud_offsets[index];
    }
    index -= EXPLOSION_SIZES;
    if (index < EXPLOSION_CLOUDS * 2) {
        return explosion_cloud_drifts[index / 2][index % 2];
    }
    index -= EXPLOSION_CLOUDS * 2;
    if (index < EXPLOSION_CLOUDS) {
        return explosion_cloud_speeds[index];
    }
    return 0;
}

static int32_t explosion_tile_offset(int32_t size)
{
    if (size < EXPLOSION_SIZES) {
        return military_size_tile_offsets[size];
    }
    return explosion_cloud_word(size - EXPLOSION_SIZES);
}

void figure_create_explosion_cloud(int32_t x, int32_t y, int32_t size, bool with_sound)
{
    int32_t tile_offset = explosion_tile_offset(size);
    int32_t cc_offset = explosion_cloud_word(size);
    int32_t count = with_sound ? EXPLOSION_CLOUDS : EXPLOSION_CLOUDS_SILENT;
    for (int32_t i = 0; i < count; i++) {
        int32_t figure_id = figure_create(city_current_player, FIGURE_EXPLOSION, x + tile_offset, y + tile_offset, 0);
        if (!figure_id) {
            continue;
        }
        figure *f = figure_get(figure_id);
        f->cross_country.x += (int16_t) cc_offset;
        f->cross_country.y += (int16_t) cc_offset;
        f->destination_x += (int16_t) explosion_cloud_drifts[i][0];
        f->destination_y += (int16_t) explosion_cloud_drifts[i][1];
        figure_movement_set_cross_country_direction(
            figure_id, f->cross_country.x, f->cross_country.y, cc_offset + f->destination_x * 15, cc_offset + f->destination_y * 15, 0);
        uint8_t speed = (uint8_t) explosion_cloud_speeds[i];
        f->speed_multiplier = speed;
        f->warship_secondary_action = speed < 3;
    }
    if (with_sound) {
        sound_effect_play(SOUND_EFFECT_BUILDING_DESTROYED);
    }
}

void figure_explosion_cloud_action(void)
{
    int32_t figure_id = figure_current_id;
    figure *f = figure_get(figure_id);
    f->use_cross_country = 1;
    int32_t group, frames, ticks_per_frame;
    if (f->warship_secondary_action) {
        group = IMAGE_GROUP_FIGURE_EXPLOSION_BIG;
        frames = 20;
        ticks_per_frame = (figure_id & 1) + 1;
    } else {
        group = IMAGE_GROUP_FIGURE_EXPLOSION;
        frames = 15;
        ticks_per_frame = 3;
    }
    if (++f->progress_on_tile >= ticks_per_frame * frames) {
        f->state = FIGURE_STATE_DEAD;
    }
    figure_movement_move_ticks_cross_country(figure_id, f->speed_multiplier);
    f->image_id = (f->progress_on_tile / ticks_per_frame) + image_group(group);
}

static void criminal_start(figure *f)
{
    f->action_state = FIGURE_ACTION_CRIMINAL_WAITING;
    random_generate_next();
    f->wait_ticks = (int16_t) (random_generator_state.random1_15bit % 10);
}

void figure_criminal_action(void)
{
    int32_t figure_id = figure_current_id;
    figure *f = figure_get(figure_id);
    city_player *player = city_player_current();
    f->terrain_usage = TERRAIN_USAGE_ROADS;
    f->cart_image_id = 0;
    int32_t speed = figure_properties[f->type].speed;
    if (++f->animation_frame >= 12) {
        f->animation_frame = 0;
    }
    switch (f->action_state) {
        case FIGURE_ACTION_CORPSE:
            figure_combat_handle_corpse(figure_id);
            break;
        case FIGURE_ACTION_ATTACK:
            figure_combat_handle_attack(figure_id);
            break;
        default:
            criminal_start(f);
            break;
        case FIGURE_ACTION_CRIMINAL_WAITING:
            if (++f->wait_ticks <= military_rules[MILITARY_RULE_CRIMINAL_WAIT_TICKS]) {
                break;
            }
            {
                int32_t building_id = criminal_find_rich_building(figure_id);
                if (!building_id) {
                    f->state = FIGURE_STATE_DEAD;
                    break;
                }
                const building *b = building_get(building_id);
                f->action_state = FIGURE_ACTION_CRIMINAL_GOING_TO_ROB;
                f->destination_x = b->road_access_x;
                f->destination_y = b->road_access_y;
                f->destination_building_id = (int16_t) building_id;
                f->wait_ticks = 0;
            }
            break;
        case FIGURE_ACTION_CRIMINAL_GOING_TO_ROB: {
            figure_movement_move_ticks(figure_id, speed * military_rules[MILITARY_RULE_CRIMINAL_SPEED_FACTOR]);
            if (f->direction != DIRECTION_AT_DESTINATION) {
                if (f->direction == DIRECTION_REROUTE || f->direction == DIRECTION_LOST) {
                    f->state = FIGURE_STATE_DEAD;
                }
                break;
            }
            int32_t building_id = f->destination_building_id;
            const building *target = building_get(building_id);
            if (building_id && target->state == BUILDING_STATE_IN_USE) {
                if (building_is_mansion(building_id)) {
                    int32_t amount = calc_adjust_with_percentage(player->personal_savings, military_difficulties[settings_data.difficulty].theft_percentage);
                    if (amount) {
                        player->crimes_this_month++;
                        player->personal_savings -= amount;
                        player->savings_lost += amount;
                        city_warning_show(WARNING_THIEF_STOLE_SAVINGS);
                        player->thefts++;
                    }
                } else if (building_is_palace(building_id)) {
                    int32_t amount = calc_adjust_with_percentage(target->tax_income_or_storage, military_difficulties[settings_data.difficulty].theft_percentage);
                    if (amount) {
                        building_update_money_values();
                        city_message_use_popup = 1;
                        player->palace_robberies++;
                        player->crimes_this_month++;
                        player->stolen_this_year += amount;
                        player->gold_stolen_this_month += amount;
                        player->treasury -= amount;
                        switch (target->type) {
                            case BUILDING_VILLAGE_PALACE:
                            case BUILDING_TOWN_PALACE:
                            case BUILDING_TAX_COLLECTOR:
                            case BUILDING_TAX_COLLECTOR_UP:
                                city_warning_show(197);
                                break;
                            case BUILDING_COURTHOUSE:
                                city_warning_show(198);
                                break;
                            case BUILDING_VILLAGE_PALACE_UP:
                            case BUILDING_TOWN_PALACE_UP:
                            case BUILDING_CITY_PALACE:
                                city_warning_show(WARNING_PALACE_ROBBED);
                                break;
                        }
                        player->thefts++;
                    }
                }
            }
            f->state = FIGURE_STATE_DEAD;
            break;
        }
        case FIGURE_ACTION_CRIMINAL_FLEEING:
            if (++f->wait_ticks <= military_rules[MILITARY_RULE_CART_THIEF_FLEE_TICKS]) {
                break;
            }
            player->palace_robberies++;
            player->crimes_this_month++;
            city_message_use_popup = 1;
            player->stolen_this_year += military_rules[MILITARY_RULE_CART_THEFT_AMOUNT];
            player->gold_stolen_this_month += military_rules[MILITARY_RULE_CART_THEFT_AMOUNT];
            city_warning_show(WARNING_GOLD_MINER_ROBBED);
            f->state = FIGURE_STATE_DEAD;
            break;
    }
    int32_t dir = view_direction(f);
    if (f->action_state != FIGURE_ACTION_CORPSE) {
        f->image_id = image_group(IMAGE_GROUP_FIGURE_PROTESTOR) + f->animation_frame * 8 + dir;
    } else {
        f->image_id = figure_corpse_offsets[f->wait_ticks / 2] + image_group(IMAGE_GROUP_FIGURE_PROTESTOR_DEATH);
    }
}

int32_t tomb_robber_find_target(int32_t figure_id)
{
    route_candidates.count = 0;
    int32_t any = 0;
    for (int32_t i = -1; i < 35; i++) {
        switch (i) {
            case 0:
            case 7:
            case 9:
            case 12:
            case 14:
            case 16:
            case 17:
            case 18:
            case 19:
            case 22:
            case 23:
            case 24:
            case 25:
            case 27:
            case 29:
                if (scenario_data.provisions_sent[i + 1] > 0) {
                    any = 1;
                }
                break;
        }
        if (any) {
            break;
        }
    }
    for (int32_t id = 1; id <= building_highest_id; id++) {
        const building *b = building_get(id);
        if (!b->state) {
            continue;
        }
        int32_t unfinished = (int16_t) b->created_sequence == -1;
        int32_t x, y;
        switch (b->type) {
            case BUILDING_MAUSOLEUM:
                if (!unfinished && !any) {
                    continue;
                }
                route_candidates.buildings[route_candidates.count] = id;
                mausoleum_get_entry_tile(id, &x, &y);
                break;
            case BUILDING_PYRAMID:
                if (!any) {
                    continue;
                }
                route_candidates.buildings[route_candidates.count] = id;
                pyramid_get_entry_road_access(id, &x, &y);
                break;
            case BUILDING_SMALL_ROYAL_TOMB:
            case BUILDING_MEDIUM_ROYAL_TOMB:
            case BUILDING_LARGE_ROYAL_TOMB:
            case BUILDING_GRAND_ROYAL_TOMB:
                if (b->subtype != 0 || (!unfinished && !any)) {
                    continue;
                }
                route_candidates.buildings[route_candidates.count] = id;
                if (b->type == BUILDING_SMALL_ROYAL_TOMB) {
                    small_royal_tomb_get_access_tile(&small_royal_tomb_object.base, id, &x, &y);
                } else if (b->type == BUILDING_MEDIUM_ROYAL_TOMB) {
                    medium_royal_tomb_get_entry_tile(&medium_royal_tomb_object.base, id, &x, &y);
                } else if (b->type == BUILDING_LARGE_ROYAL_TOMB) {
                    large_royal_tomb_get_entry_tile(&large_royal_tomb_object.base, id, &x, &y);
                } else {
                    grand_royal_tomb_get_entry_tile(&grand_royal_tomb_object.base, id, &x, &y);
                }
                break;
            default:
                continue;
        }
        route_candidates.tiles[route_candidates.count] = map_info.grid_start + y * GRID_SIZE + x;
        route_candidates.count++;
    }
    if (route_candidates.count) {
        int32_t index = figure_route_add_to_closest_of(figure_id, route_candidates.tiles, route_candidates.count);
        if (index > 0) {
            return route_candidates.buildings[index - 1];
        }
    }
    return 0;
}

void figure_tomb_robber_robbed_monument(void)
{
    int32_t best_price = 0;
    int32_t best = 0;
    for (int32_t r = 0; r < 36; r++) {
        switch (r) {
            case 1:
            case 8:
            case 10:
            case 13:
            case 15:
            case 17:
            case 18:
            case 19:
            case 20:
            case 23:
            case 24:
            case 25:
            case 26:
            case 28:
            case 30:
                if (scenario_data.provisions_sent[r] != 0 && trade_price_get(r)->buy_price > best_price) {
                    best_price = trade_price_get(r)->buy_price;
                    best = r;
                }
                break;
        }
    }
    if (best > 0) {
        scenario_data.provisions_sent[best]--;
    }
    if (city_victory_goals_reached()) {
        return;
    }
    for (int32_t id = 1; id < building_highest_id; id++) {
        switch (building_get(id)->type) {
            case BUILDING_SMALL_ROYAL_TOMB:
                royal_tomb_update_image(&small_royal_tomb_object.base, id);
                break;
            case BUILDING_MEDIUM_ROYAL_TOMB:
                royal_tomb_update_image(&medium_royal_tomb_object.base, id);
                break;
            case BUILDING_LARGE_ROYAL_TOMB:
                royal_tomb_update_image(&large_royal_tomb_object.base, id);
                break;
            case BUILDING_GRAND_ROYAL_TOMB:
                royal_tomb_update_image(&grand_royal_tomb_object.base, id);
                break;
        }
    }
}

void figure_tomb_robber_action(void)
{
    int32_t figure_id = figure_current_id;
    figure *f = figure_get(figure_id);
    city_player *player = city_player_current();
    f->terrain_usage = TERRAIN_USAGE_PREFER_ROADS;
    f->cart_image_id = 0;
    int32_t speed = figure_properties[f->type].speed;
    if (++f->animation_frame >= 12) {
        f->animation_frame = 0;
    }
    switch (f->action_state) {
        case FIGURE_ACTION_CORPSE:
            figure_combat_handle_corpse(figure_id);
            if (f->state == FIGURE_STATE_DEAD && f->opponent_id > 0) {
                city_ratings_change_kingdom(military_rules[MILITARY_RULE_TOMB_ROBBER_KILLED_RATING]);
                city_warning_show(WARNING_TOMB_ROBBER_CAUGHT);
            }
            break;
        case FIGURE_ACTION_ATTACK:
            figure_combat_handle_attack(figure_id);
            break;
        default:
            criminal_start(f);
            break;
        case FIGURE_ACTION_TOMB_ROBBER_WAITING:
            if (++f->wait_ticks <= military_rules[MILITARY_RULE_CRIMINAL_WAIT_TICKS]) {
                break;
            }
            {
                int32_t building_id = tomb_robber_find_target(figure_id);
                if (!building_id) {
                    f->state = FIGURE_STATE_DEAD;
                    break;
                }
                int32_t x = 0, y = 0;
                f->action_state = FIGURE_ACTION_TOMB_ROBBER_GOING_TO_TOMB;
                f->destination_building_id = (int16_t) building_id;
                switch (building_get(building_id)->type) {
                    case BUILDING_MAUSOLEUM:
                        mausoleum_get_entry_tile(building_id, &x, &y);
                        break;
                    case BUILDING_PYRAMID:
                        pyramid_get_entry_road_access(building_id, &x, &y);
                        break;
                    case BUILDING_SMALL_ROYAL_TOMB:
                        small_royal_tomb_get_access_tile(&small_royal_tomb_object.base, building_id, &x, &y);
                        break;
                    case BUILDING_MEDIUM_ROYAL_TOMB:
                        medium_royal_tomb_get_entry_tile(&medium_royal_tomb_object.base, building_id, &x, &y);
                        break;
                    case BUILDING_LARGE_ROYAL_TOMB:
                        large_royal_tomb_get_entry_tile(&large_royal_tomb_object.base, building_id, &x, &y);
                        break;
                    case BUILDING_GRAND_ROYAL_TOMB:
                        grand_royal_tomb_get_entry_tile(&grand_royal_tomb_object.base, building_id, &x, &y);
                        break;
                }
                f->destination_x = (int16_t) x;
                f->destination_y = (int16_t) y;
                f->wait_ticks = 0;
            }
            break;
        case FIGURE_ACTION_TOMB_ROBBER_GOING_TO_TOMB: {
            figure_movement_move_ticks(figure_id, speed * military_rules[MILITARY_RULE_CRIMINAL_SPEED_FACTOR]);
            if (f->direction != DIRECTION_AT_DESTINATION) {
                if (f->direction == DIRECTION_REROUTE || f->direction == DIRECTION_LOST) {
                    f->state = FIGURE_STATE_DEAD;
                }
                break;
            }
            int32_t building_id = f->destination_building_id;
            const building *b = building_get(building_id);
            if (building_id && b->state == BUILDING_STATE_IN_USE) {
                int32_t type = b->type;
                int32_t unfinished = (int16_t) b->created_sequence == -1;
                int32_t rating = 0;
                if (type == BUILDING_MAUSOLEUM && unfinished) {
                    city_warning_show(WARNING_MAUSOLEUM_PLUNDERED);
                    rating = military_difficulties[settings_data.difficulty].tomb_robbery_rating;
                } else if ((type == BUILDING_MAUSOLEUM || type == BUILDING_SMALL_ROYAL_TOMB || type == BUILDING_LARGE_ROYAL_TOMB ||
                               type == BUILDING_GRAND_ROYAL_TOMB || type == BUILDING_MEDIUM_ROYAL_TOMB) &&
                    unfinished) {
                    city_warning_show(WARNING_TOMB_ROBBERS_PLUNDERED);
                    rating = military_difficulties[settings_data.difficulty].tomb_robbery_rating;
                } else if (type == BUILDING_PYRAMID || type == BUILDING_MAUSOLEUM || type == BUILDING_SMALL_ROYAL_TOMB || type == BUILDING_LARGE_ROYAL_TOMB ||
                    type == BUILDING_GRAND_ROYAL_TOMB || type == BUILDING_MEDIUM_ROYAL_TOMB) {
                    city_warning_show(WARNING_TOMB_ROBBERS_STOLE_PROVISIONS);
                    figure_tomb_robber_robbed_monument();
                    rating = military_difficulties[settings_data.difficulty].tomb_robbery_rating / 2;
                }
                player->palace_robberies++;
                player->crimes_this_month++;
                player->thefts++;
                city_ratings_change_kingdom(rating);
            }
            f->state = FIGURE_STATE_DEAD;
            break;
        }
    }
    int32_t dir = view_direction(f);
    if (f->action_state == FIGURE_ACTION_CORPSE) {
        f->image_id = figure_corpse_offsets[f->wait_ticks / 2] + image_group(IMAGE_GROUP_FIGURE_TOMB_ROBBER_DEATH);
    } else {
        f->image_id = image_group(IMAGE_GROUP_FIGURE_TOMB_ROBBER) + f->animation_frame * 8 + dir;
    }
}


void figure_military_standard_action(void)
{
    int32_t figure_id = figure_current_id;
    figure *f = figure_get(figure_id);
    int32_t formation_id = f->formation_id;
    formation *legion = formation_get(formation_id);
    f->terrain_usage = TERRAIN_USAGE_ANY;
    if (++f->animation_frame > 15) {
        f->animation_frame = 0;
    }
    map_figure_delete(figure_id);
    int32_t ship_id = legion->ship_figure_id;
    if (!legion->is_at_fort && ship_id) {
        const figure *ship = figure_get(ship_id);
        f->x = ship->x;
        f->y = ship->y;
        f->grid_offset = map_info.grid_start + f->x + ship->y * GRID_SIZE;
        f->cross_country.x = ship->cross_country.x;
        f->cross_country.y = ship->cross_country.y;
        f->progress_on_tile = ship->progress_on_tile;
        f->direction = ship->direction;
    } else {
        if (legion->is_at_fort) {
            f->x = legion->home_x;
            f->y = legion->home_y;
        } else {
            f->x = legion->order_x;
            f->y = legion->order_y;
        }
        f->grid_offset = map_info.grid_start + f->x + f->y * GRID_SIZE;
        f->cross_country.x = (int16_t) (f->x * 15);
        f->cross_country.y = (int16_t) (f->y * 15);
        f->cross_country.x += 7;
        f->cross_country.y += 7;
    }
    map_figure_add(figure_id);
    f->image_id = image_group(IMAGE_GROUP_FIGURE_STANDARD) - (legion->morale / 5) + 20;
    int32_t flag = image_group(IMAGE_GROUP_FIGURE_STANDARD_FLAG);
    int32_t halted = legion->is_halted != 0;
    int32_t waving = f->animation_frame >> 1;
    int32_t type = legion->figure_type;
    if (type == FIGURE_INFANTRY) {
        f->cart_image_id = halted ? flag + 8 : waving + flag;
    } else if (type != FIGURE_CHARIOTEER) {
        f->cart_image_id = halted ? flag + 0x11 : waving + 9 + flag;
    } else {
        f->cart_image_id = halted ? flag + 0x1A : waving + 0x12 + flag;
    }
}

void figure_movement_move_ticks_tower_sentry(int32_t figure_id, int32_t num_ticks)
{
    figure *f = figure_get(figure_id);
    for (; num_ticks > 0; num_ticks--) {
        if (++f->progress_on_tile < 15) {
            figure_movement_advance_tick(figure_id);
        } else {
            f->progress_on_tile = 15;
        }
    }
}

int32_t tower_sentry_find_target(int32_t figure_id)
{
    if (enemy_army_totals.enemy_formations < 1) {
        return 0;
    }
    figure *f = figure_get(figure_id);
    if (f->action_state == FIGURE_ACTION_ATTACK || f->action_state == FIGURE_ACTION_CORPSE || f->in_building_wait_ticks) {
        return 0;
    }
    if (++f->wait_ticks_next_target < military_rules[MILITARY_RULE_SENTRY_SEARCH_TICKS]) {
        return 0;
    }
    f->wait_ticks_next_target = 0;
    if (figure_combat_get_missile_target_for_soldier(figure_id, figure_properties[f->type].missile_range) < 1) {
        return 0;
    }
    if (figure_search_best_distance > military_rules[MILITARY_RULE_SENTRY_SHOOT_DISTANCE]) {
        return 0;
    }
    f->action_state = FIGURE_ACTION_TOWER_SENTRY_FIRING;
    f->destination_x = f->x;
    f->destination_y = f->y;
    return 1;
}

static void patrol_offset(int32_t direction, int32_t distance, int32_t *x, int32_t *y)
{
    if (direction == 0) {
        *y -= distance;
    } else if (direction == 2) {
        *x += distance;
    } else if (direction == 4) {
        *y += distance;
    } else if (direction == 6) {
        *x -= distance;
    }
    if (*x < 0) {
        *x = 0;
    }
    if (*y < 0) {
        *y = 0;
    }
    if (*x >= map_info.grid_width) {
        *x = map_info.grid_width - 1;
    }
    if (*y >= map_info.grid_height) {
        *y = map_info.grid_height - 1;
    }
}

int32_t tower_sentry_init_patrol(int32_t figure_id)
{
    building *tower = building_get(figure_get(figure_id)->building_id);
    uint8_t *patrol_direction = &tower->figure_roam_direction;
    int32_t x = tower->x;
    int32_t y = tower->y;
    patrol_offset(*patrol_direction, 8, &x, &y);
    if (map_routing_wall_within_radius(x, y, 1, 6)) {
        *patrol_direction += 2;
        if (*patrol_direction > 6) {
            *patrol_direction = 0;
        }
        return 1;
    }
    for (int32_t tries = 0;;) {
        int32_t direction = *patrol_direction;
        *patrol_direction += 2;
        if (*patrol_direction > 6) {
            *patrol_direction = 0;
        }
        x = tower->x;
        y = tower->y;
        patrol_offset(direction, 3, &x, &y);
        if (map_routing_wall_within_radius(x, y, 1, 6)) {
            return 1;
        }
        if (++tries > 3) {
            return 0;
        }
    }
}

void figure_tower_sentry_action(void)
{
    int32_t figure_id = figure_current_id;
    figure *f = figure_get(figure_id);
    f->terrain_usage = TERRAIN_USAGE_WALLS;
    f->use_cross_country = 0;
    f->is_ghost = 1;
    f->height_adjusted_ticks = 10;
    f->max_roam_length = 800;
    int32_t speed = figure_properties[f->type].speed;
    building *tower = building_get(f->building_id);
    if (tower->state != BUILDING_STATE_IN_USE || tower->figure_id != figure_id) {
        f->state = FIGURE_STATE_DEAD;
    }
    if (++f->animation_frame >= 12) {
        f->animation_frame = 0;
    }
    if (f->action_state != FIGURE_ACTION_TOWER_SENTRY_GOING_TO_TOWER && f->action_state != FIGURE_ACTION_WAIT_FOR_FERRY &&
        f->action_state != FIGURE_ACTION_ON_FERRY) {
        tower_sentry_find_target(figure_id);
    }
    switch (f->action_state) {
        case FIGURE_ACTION_ATTACK:
            sound_figure_play(figure_id, FIGURE_SOUND_TOWER_SENTRY, FIGURE_SOUND_VARIANT_HIT);
            figure_combat_handle_attack(figure_id);
            break;
        case FIGURE_ACTION_CORPSE:
            figure_combat_handle_corpse(figure_id);
            break;
        case FIGURE_ACTION_TOWER_SENTRY_AT_REST:
            f->animation_frame = 0;
            if (++f->wait_ticks <= military_rules[MILITARY_RULE_SENTRY_REST_TICKS]) {
                break;
            }
            f->wait_ticks = 0;
            if (tower_sentry_init_patrol(figure_id)) {
                f->action_state = FIGURE_ACTION_TOWER_SENTRY_PATROLLING;
                f->destination_x = (int16_t) map_result_x;
                f->destination_y = (int16_t) map_result_y;
                figure_route_remove(figure_id);
            }
            break;
        case FIGURE_ACTION_TOWER_SENTRY_GOING_TO_TOWER:
            f->terrain_usage = TERRAIN_USAGE_ANY_OR_FORT;
            f->is_ghost = 0;
            f->height_adjusted_ticks = 0;
            figure_movement_move_ticks(figure_id, speed);
            if (f->direction == DIRECTION_REROUTE || f->direction == DIRECTION_LOST) {
                f->state = FIGURE_STATE_DEAD;
            } else if (f->direction == DIRECTION_AT_DESTINATION) {
                map_figure_delete(figure_id);
                f->x = tower->x;
                f->source_x = tower->x;
                f->y = tower->y;
                f->source_y = tower->y;
                f->grid_offset = map_info.grid_start + f->x + f->y * GRID_SIZE;
                map_figure_add(figure_id);
                f->action_state = FIGURE_ACTION_TOWER_SENTRY_AT_REST;
                figure_route_remove(figure_id);
            }
            break;
        case FIGURE_ACTION_TOWER_SENTRY_PATROLLING:
            figure_movement_move_ticks(figure_id, speed);
            if (f->direction == DIRECTION_REROUTE || f->direction == DIRECTION_LOST) {
                f->action_state = FIGURE_ACTION_TOWER_SENTRY_AT_REST;
            } else if (f->direction == DIRECTION_AT_DESTINATION) {
                f->action_state = FIGURE_ACTION_TOWER_SENTRY_RETURNING;
                f->destination_x = f->source_x;
                f->destination_y = f->source_y;
                figure_route_remove(figure_id);
            }
            break;
        case FIGURE_ACTION_TOWER_SENTRY_RETURNING:
            figure_movement_move_ticks(figure_id, speed);
            if (f->direction == DIRECTION_REROUTE || f->direction == DIRECTION_LOST) {
                f->state = FIGURE_STATE_DEAD;
            } else if (f->direction == DIRECTION_AT_DESTINATION) {
                f->action_state = FIGURE_ACTION_TOWER_SENTRY_AT_REST;
            }
            break;
        case FIGURE_ACTION_TOWER_SENTRY_FIRING:
            figure_movement_move_ticks_tower_sentry(figure_id, 1);
            if (++f->wait_ticks_missile <= figure_properties[f->type].missile_delay) {
                break;
            }
            if (figure_combat_get_missile_target_for_soldier(figure_id, figure_properties[f->type].missile_range)) {
                sound_figure_play(figure_id, FIGURE_SOUND_TOWER_SENTRY, FIGURE_SOUND_VARIANT_HIT);
                f->direction = (int8_t) calc_missile_shooter_direction(f->x, f->y, map_result_x, map_result_y);
                f->wait_ticks_missile = 0;
                figure_create_missile(figure_id, f->x, f->y, map_result_x, map_result_y, FIGURE_JAVELIN);
            } else {
                f->action_state = FIGURE_ACTION_TOWER_SENTRY_RETURNING;
                f->destination_x = f->source_x;
                f->destination_y = f->source_y;
                figure_route_remove(figure_id);
            }
            break;
        case FIGURE_ACTION_ON_FERRY:
            f->terrain_usage = TERRAIN_USAGE_ANY_OR_FORT;
            f->image_id = 0;
            f->cart_image_id = 0;
            break;
        case FIGURE_ACTION_WAIT_FOR_FERRY:
            f->terrain_usage = TERRAIN_USAGE_ANY_OR_FORT;
            f->animation_frame = 0;
            if (++f->wait_ticks > 100) {
                f->wait_ticks = 0;
                if (!figure_is_queued_for_ferry(figure_id)) {
                    f->state = FIGURE_STATE_DEAD;
                }
            }
            break;
        default:
            f->action_state = FIGURE_ACTION_TOWER_SENTRY_AT_REST;
            break;
    }
    uint32_t terrain = map_grids.terrain[f->grid_offset];
    if (terrain & TERRAIN_WALL) {
        f->current_height = 18;
    } else if (terrain & TERRAIN_GATEHOUSE) {
        f->in_building_wait_ticks = 24;
    } else if (f->action_state != FIGURE_ACTION_TOWER_SENTRY_GOING_TO_TOWER && f->action_state != FIGURE_ACTION_WAIT_FOR_FERRY &&
        f->action_state != FIGURE_ACTION_ON_FERRY) {
        f->state = FIGURE_STATE_DEAD;
    }
    if (f->in_building_wait_ticks) {
        f->in_building_wait_ticks--;
        f->height_adjusted_ticks = 0;
    }
    figure_image_direction = f->direction - map_info.orientation;
    if (figure_image_direction < 0) {
        figure_image_direction += DIRECTION_COUNT;
    }
    if (f->action_state == FIGURE_ACTION_CORPSE) {
        f->image_id = figure_corpse_offsets[f->wait_ticks / 2] + image_group(IMAGE_GROUP_FIGURE_TOWER_SENTRY_DEATH);
    } else if (f->action_state == FIGURE_ACTION_TOWER_SENTRY_FIRING) {
        int32_t frame = figure_corpse_offsets[FIGURE_SENTRY_ATTACK_FRAMES + (f->wait_ticks_missile >> 1)];
        f->image_id = image_group(IMAGE_GROUP_FIGURE_TOWER_SENTRY_ATTACK) + (frame * 8) + figure_image_direction;
    } else {
        f->image_id = image_group(IMAGE_GROUP_FIGURE_TOWER_SENTRY) + f->animation_frame * 8 + figure_image_direction;
    }
}

static int32_t is_ship_type(int32_t type)
{
    return type == FIGURE_ENEMY_WARSHIP || type == FIGURE_ENEMY_TRANSPORT_SHIP || type == FIGURE_ENEMY_EGYPTIAN_WARSHIP ||
        type == FIGURE_ENEMY_EGYPTIAN_TRANSPORT_SHIP;
}

static void javelin_set_image(figure *f, const figure *shooter)
{
    figure_image_direction = f->direction - map_info.orientation * 4;
    if (figure_image_direction < 0) {
        figure_image_direction += 32;
    }
    if (shooter->type == FIGURE_TOWER_SENTRY || shooter->type == FIGURE_BALLISTA) {
        f->image_id = image_group(IMAGE_GROUP_FIGURE_ENEMY_SPEAR) + figure_image_direction;
        f->cart_image_id = image_group(IMAGE_GROUP_FIGURE_ENEMY_SPEAR_SHADOW) + figure_image_direction;
    } else {
        f->image_id = image_group(IMAGE_GROUP_FIGURE_JAVELIN) + figure_image_direction;
        f->cart_image_id = image_group(IMAGE_GROUP_FIGURE_JAVELIN_SHADOW) + figure_image_direction;
    }
}

void figure_javelin_action(void)
{
    int32_t figure_id = figure_current_id;
    figure *f = figure_get(figure_id);
    f->use_cross_country = 1;
    if (++f->progress_on_tile > 120) {
        f->state = FIGURE_STATE_DEAD;
    }
    int32_t moved = figure_movement_move_ticks_cross_country(figure_id, 4);
    int32_t target_id = map_figure_find_target(figure_id);
    figure *shooter = figure_get(f->building_id);
    int32_t shooter_formation = shooter->formation_id;
    formation *shooter_legion = formation_get(shooter_formation);
    if (target_id) {
        figure *target = figure_get(target_id);
        int32_t target_formation = target->formation_id;
        int32_t attack_type;
        if (shooter->type == FIGURE_BALLISTA) {
            attack_type = 62;
        } else if (shooter->type == FIGURE_WARSHIP) {
            if (shooter->state == FIGURE_STATE_DEAD) {
                f->state = FIGURE_STATE_DEAD;
                javelin_set_image(f, shooter);
                return;
            }
            attack_type = 78;
        } else {
            attack_type = shooter_legion->figure_type;
        }
        int32_t type = target->type;
        int32_t max_damage = is_ship_type(type) ? target->ship_max_hit_points : figure_properties[type].max_damage;
        int32_t defense = figure_properties[type].missile_defense;
        if (defense >= 20) {
            defense = 20;
        }
        int32_t attack = figure_properties[attack_type].missile_attack;
        int32_t damage = 0;
        if (attack > 0) {
            damage = ((20 - defense) * attack) / 20;
            if (damage < 0) {
                damage = 0;
            }
        }
        if (is_ship_type(type)) {
            int32_t hit_points = target->ship_hit_points - damage;
            if (hit_points <= 0) {
                target->ship_hit_points = 0;
                target->action_state = FIGURE_ACTION_CORPSE;
                target->wait_ticks = 0;
                figure_play_die_sound(target_id);
                formation_update_morale_after_death(target_formation);
            } else {
                target->ship_hit_points = hit_points;
            }
        } else if (target->damage + damage > max_damage) {
            target->damage = (uint8_t) (max_damage + 1);
            target->action_state = FIGURE_ACTION_CORPSE;
            target->wait_ticks = 0;
            figure_play_die_sound(target_id);
            formation_update_morale_after_death(target_formation);
            if (!figure_is_predator_animal(target->type) && !figure_is_herd_animal(target->type) && target->type != FIGURE_CRIMINAL &&
                shooter->faction_id == city_current_player) {
                int32_t experience = shooter_legion->experience + 1;
                if (experience >= military_rules[MILITARY_RULE_MAX_EXPERIENCE]) {
                    experience = military_rules[MILITARY_RULE_MAX_EXPERIENCE];
                }
                shooter_legion->experience = (int16_t) experience;
            }
        } else {
            target->damage = (uint8_t) (target->damage + damage);
        }
        if (target_formation) {
            formation_get(target_formation)->missile_attack_timeout = (int16_t) military_rules[MILITARY_RULE_MISSILE_ATTACK_TIMEOUT];
            formation_get(target_formation)->missile_attack_formation_id = (int16_t) shooter_formation;
        }
        f->state = FIGURE_STATE_DEAD;
    } else if (moved) {
        f->state = FIGURE_STATE_DEAD;
    }
    javelin_set_image(f, shooter);
}

void figure_ballista_action(void)
{
    int32_t figure_id = figure_current_id;
    figure *f = figure_get(figure_id);
    int32_t shooting = 0;
    f->terrain_usage = TERRAIN_USAGE_WALLS;
    f->use_cross_country = 0;
    f->is_ghost = 1;
    f->height_adjusted_ticks = 10;
    f->current_height = 45;
    int32_t tower_id = f->building_id;
    const building *tower = building_get(tower_id);
    if (tower->state != BUILDING_STATE_IN_USE || tower->figure_id4 != figure_id) {
        f->state = FIGURE_STATE_DEAD;
    }
    if (tower->num_workers < 1) {
        f->state = FIGURE_STATE_DEAD;
    }
    if (tower->figure_id < 1) {
        f->state = FIGURE_STATE_DEAD;
    }
    map_figure_delete(figure_id);
    switch (map_info.orientation) {
        case 0:
            f->x = tower->x;
            f->y = tower->y;
            break;
        case 2:
            f->x = tower->x + 1;
            f->y = tower->y;
            break;
        case 4:
            f->x = tower->x + 1;
            f->y = tower->y + 1;
            break;
        case 6:
            f->x = tower->x;
            f->y = tower->y + 1;
            break;
    }
    f->grid_offset = map_info.grid_start + f->x + f->y * GRID_SIZE;
    map_figure_add(figure_id);
    switch (f->action_state) {
        case FIGURE_ACTION_CORPSE:
            f->state = FIGURE_STATE_DEAD;
            break;
        case FIGURE_ACTION_BALLISTA_READY:
            if (++f->wait_ticks <= military_rules[MILITARY_RULE_BALLISTA_SEARCH_TICKS]) {
                break;
            }
            f->wait_ticks = 0;
            if (figure_combat_get_missile_target_for_soldier(figure_id, figure_properties[f->type].missile_range)) {
                f->action_state = FIGURE_ACTION_BALLISTA_FIRING;
                f->wait_ticks_missile = (uint8_t) figure_properties[f->type].missile_delay;
            }
            break;
        case FIGURE_ACTION_BALLISTA_FIRING:
            if (++f->wait_ticks_missile <= figure_properties[f->type].missile_delay) {
                break;
            }
            shooting = 1;
            if (!figure_combat_get_missile_target_for_soldier(figure_id, figure_properties[f->type].missile_range)) {
                f->action_state = FIGURE_ACTION_BALLISTA_READY;
                break;
            }
            f->wait_ticks++;
            f->direction = (int8_t) calc_missile_shooter_direction(f->x, f->y, map_result_x, map_result_y);
            if (f->wait_ticks == military_rules[MILITARY_RULE_BALLISTA_AIM_TICKS]) {
                figure_create_missile(figure_id, f->x, f->y, map_result_x, map_result_y, FIGURE_JAVELIN);
            } else if (f->wait_ticks > military_rules[MILITARY_RULE_BALLISTA_RELOAD_TICKS]) {
                f->wait_ticks = 0;
                f->wait_ticks_missile = 0;
            }
            break;
        default:
            f->action_state = FIGURE_ACTION_BALLISTA_READY;
            break;
    }
    figure_image_direction = f->direction - map_info.orientation;
    if (figure_image_direction < 0) {
        figure_image_direction += DIRECTION_COUNT;
    }
    if (f->action_state == FIGURE_ACTION_BALLISTA_FIRING && shooting) {
        f->image_id = image_group(IMAGE_GROUP_FIGURE_BALLISTA_FIRE) + ((f->wait_ticks / 2) * 8) + figure_image_direction;
    } else {
        f->image_id = image_group(IMAGE_GROUP_FIGURE_BALLISTA) + figure_image_direction;
    }
}

int32_t ship_is_docked(int32_t figure_id)
{
    int32_t building_id = figure_get(figure_id)->building_id;
    if (!building_id) {
        return 0;
    }
    return building_get(building_id)->data.dock.ship_id == figure_id;
}

static void ship_update_dock(int32_t figure_id, int32_t building_id)
{
    building *b = building_get(building_id);
    map_water_tile_next_to(b->x, b->y, b->size);
    const figure *f = figure_get(figure_id);
    if (map_result_x == f->x && map_result_y == f->y) {
        b->data.dock.ship_id = (int16_t) figure_id;
    } else {
        b->data.dock.ship_id = 0;
    }
}

void ship_move_to_dock(int32_t figure_id, int32_t *moving)
{
    figure *f = figure_get(figure_id);
    int32_t building_id = f->building_id;
    if (f->x == f->destination_x && f->y == f->destination_y) {
        f->direction = DIRECTION_AT_DESTINATION;
    } else {
        figure_movement_move_ticks(figure_id, figure_properties[f->type].speed);
    }
    if (f->direction == DIRECTION_REROUTE) {
        figure_route_remove(figure_id);
    } else if (f->direction == DIRECTION_LOST) {
        figure_route_remove(figure_id);
    } else if (f->direction == DIRECTION_AT_DESTINATION) {
        *moving = 0;
        if (f->action_state != FIGURE_ACTION_TRANSPORT_SHIP_DOCKED && f->action_state != FIGURE_ACTION_TRANSPORT_SHIP_GOING_TO_DOCK &&
            (uint8_t) ship_is_docked(figure_id)) {
            f->action_state = FIGURE_ACTION_TRANSPORT_SHIP_EVADING;
            f->warship_secondary_action = SHIP_MODE_WAITING;
        }
    }
    if (building_id) {
        ship_update_dock(figure_id, building_id);
    }
}

void ship_evade(int32_t figure_id, int32_t *moving)
{
    figure *f = figure_get(figure_id);
    if (f->warship_secondary_action == SHIP_MODE_MOVING) {
        ship_move_to_dock(figure_id, moving);
        if (f->direction == DIRECTION_AT_DESTINATION) {
            f->warship_secondary_action = SHIP_MODE_WAITING;
            *moving = 0;
        }
        return;
    }
    if (f->warship_secondary_action != SHIP_MODE_WAITING) {
        return;
    }
    *moving = 0;
    if (++f->wait_ticks < military_rules[MILITARY_RULE_TRANSPORT_EVADE_TICKS]) {
        return;
    }
    f->wait_ticks = 0;
    int32_t enemy_id = ship_find_closest(figure_id, military_rules[MILITARY_RULE_TRANSPORT_EVADE_RANGE], 0);
    if (!enemy_id) {
        return;
    }
    const figure *enemy = figure_get(enemy_id);
    int32_t direction = (uint8_t) calc_general_direction(enemy->x, enemy->y, f->x, f->y);
    if (direction > DIRECTION_LEFT) {
        direction = DIRECTION_TOP_LEFT;
    }
    const transport_escape *escape = &transport_escapes[direction];
    int32_t x;
    int32_t y;
    ship_pick_escape_point(&x, &y, escape->dx[0], escape->dy[0], escape->dx[1], escape->dy[1], escape->dx[2], escape->dy[2], figure_id);
    f->destination_x = (int16_t) x;
    f->destination_y = (int16_t) y;
    figure_route_remove(figure_id);
    f->warship_secondary_action = SHIP_MODE_MOVING;
    f->animation_frame = 0;
}

int32_t ship_find_closest(int32_t figure_id, int32_t size, int32_t any)
{
    const figure *f = figure_get(figure_id);
    int32_t best_id = 0;
    int32_t best_distance = 500000;
    int32_t row = f->x + map_info.grid_start + f->y * GRID_SIZE - ((size / 2) * (GRID_SIZE + 1));
    for (int32_t dy = size; dy > 0; dy--) {
        int32_t grid_offset = row;
        for (int32_t dx = size; dx > 0; dx--) {
            int32_t id = any ? ship_find_ship_at(grid_offset, figure_id) : ship_find_target_at(grid_offset, figure_id);
            if (id) {
                const figure *other = figure_get(id);
                int32_t ddx = other->x - f->x;
                int32_t ddy = other->y - f->y;
                int32_t distance = ddx * ddx + ddy * ddy;
                if (distance < best_distance) {
                    best_id = id;
                    best_distance = distance;
                }
            }
            grid_offset++;
        }
        row += GRID_SIZE;
    }
    return best_id;
}

void ship_move(int32_t figure_id, int32_t cautious)
{
    figure *f = figure_get(figure_id);
    if (f->x == f->destination_x && f->y == f->destination_y && f->direction == DIRECTION_AT_DESTINATION) {
        return;
    }
    int32_t *counter = &f->ship.moves;
    int32_t *steps = &f->ship.steps;
    switch (f->ship_move_mode) {
        case SHIP_MOVE_FULL_SPEED: {
            int32_t speed;
            if (cautious) {
                if (++*counter >= figure_properties[f->type].ship_moves) {
                    *counter = 0;
                    f->ship_move_mode = SHIP_MOVE_RESTING;
                    return;
                }
                if (f->previous_tile_direction != f->direction) {
                    *steps = 0;
                }
                if (f->x != f->formation_position_x || f->y != f->formation_position_y) {
                    f->formation_position_x = f->x;
                    f->formation_position_y = f->y;
                    ++*steps;
                }
                if (*steps >= figure_properties[f->type].ship_steps) {
                    figure_movement_move_ticks(figure_id, figure_properties[f->type].speed);
                    return;
                }
            }
            speed = figure_properties[f->type].speed * 2;
            if (speed >= 18) {
                speed = 18;
            }
            figure_movement_move_ticks(figure_id, speed);
            return;
        }
        case SHIP_MOVE_SLOW:
            figure_movement_move_ticks(figure_id, calc_adjust_with_percentage(figure_properties[f->type].speed, 40));
            if (++*counter >= 200) {
                *counter = 0;
                f->ship_move_mode = SHIP_MOVE_FULL_SPEED;
            }
            return;
        case SHIP_MOVE_RESTING:
            if (++*counter >= 200) {
                *counter = 0;
                f->ship_move_mode = SHIP_MOVE_SLOW;
            }
            return;
    }
}

int32_t ship_find_target_at(int32_t grid_offset, int32_t figure_id)
{
    int32_t id = map_grids.figure_ids[grid_offset];
    while (id) {
        if (id != figure_id) {
            int32_t type = figure_get(id)->type;
            if (type == FIGURE_ENEMY_TRANSPORT_SHIP || type == FIGURE_ENEMY_WARSHIP || type == FIGURE_ENEMY_EGYPTIAN_TRANSPORT_SHIP ||
                type == FIGURE_ENEMY_EGYPTIAN_WARSHIP) {
                return id;
            }
        }
        id = figure_get(id)->next_figure_id_on_same_tile;
    }
    return 0;
}

static void warship_fire_missile(int32_t figure_id, figure *f, int32_t target_id)
{
    if (f->attack_image_offset) {
        if (f->attack_image_offset == 1) {
            const figure *target = figure_get(target_id);
            figure_create_missile(figure_id, f->x, f->y, target->x, target->y, FIGURE_JAVELIN);
        }
        if (++f->attack_image_offset > 100) {
            f->attack_image_offset = 0;
        }
    }
}

void warship_patrol(int32_t figure_id, int32_t *moving)
{
    figure *f = figure_get(figure_id);
    switch (f->warship_secondary_action) {
        case SHIP_MODE_MOVING: {
            int32_t target_id = f->target_figure_id;
            if (f->action_state != FIGURE_ACTION_WARSHIP_HUNTING) {
                f->warship_secondary_action = SHIP_MODE_PATROLLING;
            }
            const figure *target = figure_get(target_id);
            int32_t distance = calc_maximum_distance(f->x, f->y, target->x, target->y);
            int32_t shoot = 0;
            if (distance < figure_properties[f->type].missile_range && ++f->wait_ticks_missile > figure_properties[f->type].missile_delay) {
                f->wait_ticks_missile = 0;
                shoot = target_id != 0;
            }
            f->attack_image_offset = (uint8_t) shoot;
            warship_fire_missile(figure_id, f, target_id);
            warship_move_attack(figure_id, moving);
            if (f->direction == DIRECTION_AT_DESTINATION) {
                f->warship_secondary_action = SHIP_MODE_WAITING;
                figure_route_remove(figure_id);
            }
            return;
        }
        case SHIP_MODE_WAITING: {
            if (f->action_state != FIGURE_ACTION_WARSHIP_HUNTING) {
                f->warship_secondary_action = SHIP_MODE_PATROLLING;
            }
            int32_t target_id = figure_warship_find_enemy(figure_id, military_rules[MILITARY_RULE_WARSHIP_HUNT_RANGE]);
            if (target_id) {
                f->warship_secondary_action = SHIP_MODE_MOVING;
                f->direction = f->previous_tile_direction;
                f->destination_x = figure_get(target_id)->x;
                f->destination_y = figure_get(target_id)->y;
                f->target_figure_id = (int16_t) target_id;
                figure_route_remove(figure_id);
                return;
            }
            ship_move(figure_id, 0);
            if (f->direction == DIRECTION_AT_DESTINATION) {
                *moving = 0;
            }
            return;
        }
        case SHIP_MODE_PATROL_ATTACK: {
            if (f->action_state != FIGURE_ACTION_WARSHIP_PATROLLING) {
                f->warship_secondary_action = SHIP_MODE_WAITING;
            }
            if (calc_maximum_distance(f->patrol_x, f->patrol_y, f->x, f->y) < military_rules[MILITARY_RULE_WARSHIP_PATROL_RADIUS]) {
                int32_t target_id = f->target_figure_id;
                const figure *target = figure_get(target_id);
                if (calc_maximum_distance(target->x, target->y, f->x, f->y) < figure_properties[f->type].missile_range) {
                    if (++f->wait_ticks_missile > figure_properties[f->type].missile_delay) {
                        f->wait_ticks_missile = 0;
                        f->attack_image_offset = target_id != 0;
                    }
                    warship_fire_missile(figure_id, f, target_id);
                    warship_move_attack(figure_id, moving);
                    if (f->direction != DIRECTION_AT_DESTINATION) {
                        return;
                    }
                    f->warship_secondary_action = SHIP_MODE_PATROLLING;
                    f->destination_x = f->patrol_x;
                    f->destination_y = f->patrol_y;
                    figure_route_remove(figure_id);
                    return;
                }
            }
            f->warship_secondary_action = SHIP_MODE_PATROLLING;
            f->destination_x = f->patrol_x;
            f->destination_y = f->patrol_y;
            figure_route_remove(figure_id);
            return;
        }
        case SHIP_MODE_PATROLLING: {
            int32_t range = figure_properties[f->type].missile_range + military_rules[MILITARY_RULE_WARSHIP_PATROL_EXTRA_RANGE];
            int32_t target_id = figure_warship_find_enemy(figure_id, range);
            if (f->action_state != FIGURE_ACTION_WARSHIP_PATROLLING) {
                f->warship_secondary_action = SHIP_MODE_WAITING;
            }
            if (target_id) {
                const figure *target = figure_get(target_id);
                if (calc_maximum_distance(f->patrol_x, f->patrol_y, target->x, target->y) < military_rules[MILITARY_RULE_WARSHIP_PATROL_RADIUS]) {
                    f->warship_secondary_action = SHIP_MODE_PATROL_ATTACK;
                    f->target_figure_id = (int16_t) target_id;
                    f->direction = f->previous_tile_direction;
                    f->destination_x = target->x;
                    f->destination_y = target->y;
                    figure_route_remove(figure_id);
                    return;
                }
            }
            ship_move(figure_id, 0);
            if (f->direction == DIRECTION_AT_DESTINATION) {
                *moving = 0;
            }
            return;
        }
    }
}

int32_t figure_next_route_direction(int32_t figure_id)
{
    const figure *f = figure_get(figure_id);
    if (f->routing_path_id < 1) {
        int32_t direction = (uint8_t) calc_general_direction(f->x, f->y, f->destination_x, f->destination_y);
        return (direction != DIRECTION_AT_DESTINATION) ? DIRECTION_LOST : direction;
    }
    if (f->routing_path_current_tile >= f->routing_path_length) {
        return 8;
    }
    return city_figures.route_paths[(f->routing_path_id * 500) + f->routing_path_current_tile];
}

static int32_t ship_image_direction(const figure *f)
{
    int32_t dir = f->direction >= DIRECTION_COUNT ? f->previous_tile_direction : f->direction;
    dir -= map_info.orientation;
    if (dir < 0) {
        dir += DIRECTION_COUNT;
    }
    return dir;
}

static int32_t ship_sinking_image(const figure *f, int32_t dir, int32_t base)
{
    if (f->wait_ticks < 33) {
        return dir / 8 + base + f->wait_ticks / 3;
    }
    int32_t frame = f->wait_ticks - 33;
    if (frame >= 23) {
        frame = 22;
    }
    return frame + image_group(IMAGE_GROUP_FIGURE_DEATH_IN_WATER);
}

static void ship_sink(figure *f)
{
    if (f->action_state != FIGURE_ACTION_CORPSE) {
        f->wait_ticks = -1;
    }
    f->action_state = FIGURE_ACTION_CORPSE;
}

void figure_warship_action(void)
{
    int32_t figure_id = figure_current_id;
    figure *f = figure_get(figure_id);
    if (f->request_event_id >= 0) {
        if (f->action_state == FIGURE_ACTION_WARSHIP_AWAY_ON_REQUEST) {
            f->is_ghost = 1;
            return;
        }
        if (f->action_state != FIGURE_ACTION_WARSHIP_GOING_TO_REQUEST) {
            figure_route_remove(figure_id);
            f->routing_path_current_tile = 0;
            f->progress_on_tile = 0;
            f->action_state = FIGURE_ACTION_WARSHIP_GOING_TO_REQUEST;
        }
    }
    int32_t speed = figure_properties[f->type].speed;
    int32_t building_id = f->building_id;
    int32_t moving = 1;
    city_player_current()->soldiers++;
    if (building_id) {
        ship_update_dock(figure_id, building_id);
    }
    f->is_ghost = 0;
    f->is_boat = 1;
    f->cart_image_id = 0;
    if (++f->animation_frame >= 12) {
        f->animation_frame = 0;
    }
    if (building_get(building_id)->state != BUILDING_STATE_IN_USE) {
        ship_sink(f);
    }
    if (f->ship_hit_points <= 0) {
        ship_sink(f);
    }
    int32_t health = calc_percentage(f->ship_hit_points, f->ship_max_hit_points);
    if (f->action_state != FIGURE_ACTION_WARSHIP_GOING_TO_DOCK && f->action_state != FIGURE_ACTION_WARSHIP_QUEUED_AT_DOCK &&
        f->action_state != FIGURE_ACTION_CORPSE && health <= military_rules[MILITARY_RULE_SHIP_DAMAGED_PERCENTAGE] && health > 0) {
        ship_go_to_best_dock(figure_id);
    }
    switch (f->action_state) {
        case FIGURE_ACTION_CORPSE:
            enemy_ship_sink(figure_id);
            break;
        case FIGURE_ACTION_WARSHIP_WAITING:
            warship_wait_at_dock(figure_id, &moving);
            warship_shoot(figure_id);
            break;
        case FIGURE_ACTION_WARSHIP_STRAIGHT:
            warship_move_attack(figure_id, &moving);
            break;
        case FIGURE_ACTION_WARSHIP_HUNTING:
        case FIGURE_ACTION_WARSHIP_PATROLLING:
            warship_patrol(figure_id, &moving);
            break;
        case FIGURE_ACTION_WARSHIP_SAILING:
            warship_sail_to_destination(figure_id, &moving);
            warship_shoot(figure_id);
            break;
        case FIGURE_ACTION_WARSHIP_EVADING:
            warship_evading(figure_id, &moving);
            warship_shoot(figure_id);
            break;
        case FIGURE_ACTION_WARSHIP_GOING_TO_DOCK:
            warship_sail_to_dock(figure_id, &moving);
            break;
        case FIGURE_ACTION_WARSHIP_QUEUED_AT_DOCK:
            moving = 0;
            break;
        case FIGURE_ACTION_WARSHIP_GOING_TO_REQUEST:
            f->destination_x = (uint8_t) scenario_data.river_entry_x;
            f->destination_y = (uint8_t) scenario_data.river_entry_y;
            figure_movement_move_ticks(figure_id, speed);
            if (f->direction == DIRECTION_AT_DESTINATION) {
                f->action_state = FIGURE_ACTION_WARSHIP_AWAY_ON_REQUEST;
                figure_route_remove(figure_id);
            } else if (f->direction == DIRECTION_REROUTE) {
                figure_route_remove(figure_id);
            } else if (f->direction == DIRECTION_LOST) {
                ship_sink(f);
            }
            break;
        case FIGURE_ACTION_WARSHIP_AWAY_ON_REQUEST:
            f->is_ghost = 1;
            break;
        case FIGURE_ACTION_WARSHIP_RETURNING_FROM_REQUEST:
            f->is_ghost = 0;
            f->wait_ticks = 0;
            figure_movement_move_ticks(figure_id, speed);
            if (f->direction == DIRECTION_AT_DESTINATION) {
                figure_movement_move_ticks(figure_id, speed);
                if (f->direction == DIRECTION_AT_DESTINATION) {
                    f->action_state = FIGURE_ACTION_WARSHIP_WAITING;
                } else if (f->direction == DIRECTION_REROUTE) {
                    figure_route_remove(figure_id);
                } else if (f->direction == DIRECTION_LOST) {
                    ship_sink(f);
                }
            }
            break;
        case FIGURE_ACTION_WARSHIP_DOCKED:
            warship_sail_to_destination(figure_id, &moving);
            warship_shoot(figure_id);
            warship_docked(figure_id);
            break;
        case FIGURE_ACTION_WARSHIP_ATTACKING_SHIP:
            warship_attack_ship(figure_id, &moving);
            break;
        default:
            ship_sink(f);
            break;
    }
    figure_next_route_direction(figure_id);
    int32_t dir = ship_image_direction(f);
    if (f->action_state == FIGURE_ACTION_CORPSE) {
        f->image_id = ship_sinking_image(f, dir, image_group(IMAGE_GROUP_FIGURE_WARSHIP_SINKING));
    } else if (moving && f->ship_move_mode != SHIP_MOVE_RESTING) {
        f->image_id = image_group(IMAGE_GROUP_FIGURE_WARSHIP_MOVING) + ((f->animation_frame / 3) * 8) + dir;
    } else {
        f->image_id = image_group(IMAGE_GROUP_FIGURE_WARSHIP_STILL) + dir;
    }
    if (selected_warship_id == figure_id && selected_warship_active) {
        f->cart_image_id = ((f->animation_frame << 3) / 12) + image_group(IMAGE_GROUP_FIGURE_SHIP_WAKE);
    }
}

void figure_transport_ship_action(void)
{
    int32_t figure_id = figure_current_id;
    floods_move_boat_to_water(figure_id);
    int32_t moving = 1;
    figure *f = figure_get(figure_id);
    int32_t building_id = f->building_id;
    int32_t speed = figure_properties[f->type].speed;
    if (building_id) {
        ship_update_dock(figure_id, building_id);
    }
    f->is_ghost = 0;
    f->is_boat = 1;
    f->cart_image_id = 0;
    if (++f->animation_frame >= 12) {
        f->animation_frame = 0;
    }
    if (building_get(building_id)->state != BUILDING_STATE_IN_USE) {
        f->action_state = FIGURE_ACTION_CORPSE;
    }
    if (f->ship_hit_points <= 0) {
        f->action_state = FIGURE_ACTION_CORPSE;
    }
    int32_t health = calc_percentage(f->ship_hit_points, f->ship_max_hit_points);
    if (f->action_state != FIGURE_ACTION_TRANSPORT_SHIP_GOING_TO_DOCK && f->action_state != FIGURE_ACTION_TRANSPORT_SHIP_QUEUED_AT_DOCK &&
        f->action_state != FIGURE_ACTION_CORPSE && health <= military_rules[MILITARY_RULE_SHIP_DAMAGED_PERCENTAGE] && health > 0) {
        ship_go_to_best_dock(figure_id);
    }
    switch (f->action_state) {
        case FIGURE_ACTION_CORPSE:
            transport_sink(figure_id);
            break;
        case FIGURE_ACTION_TRANSPORT_SHIP_WAITING:
            transport_wait_at_dock(figure_id, building_id, &moving);
            break;
        case FIGURE_ACTION_TRANSPORT_SHIP_SAILING:
            ship_move_to_dock(figure_id, &moving);
            break;
        case FIGURE_ACTION_TRANSPORT_SHIP_EVADING:
            ship_evade(figure_id, &moving);
            break;
        case FIGURE_ACTION_TRANSPORT_SHIP_GOING_TO_DOCK:
            transport_sail_to_dock(figure_id, &moving);
            break;
        case FIGURE_ACTION_TRANSPORT_SHIP_GOING_TO_LEGION:
            transport_sail_to_legion(figure_id, &moving);
            break;
        case FIGURE_ACTION_TRANSPORT_SHIP_GOING_TO_LANDING:
            transport_sail_to_landing(figure_id, &moving);
            break;
        case FIGURE_ACTION_TRANSPORT_SHIP_RETURNING_TO_LANDING:
        case FIGURE_ACTION_TRANSPORT_SHIP_RETURNING_FROM_REQUEST:
            f->is_ghost = 0;
            f->wait_ticks = 0;
            transport_sail_to_landing(figure_id, &moving);
            break;
        case FIGURE_ACTION_TRANSPORT_SHIP_QUEUED_AT_DOCK:
            moving = 0;
            break;
        case FIGURE_ACTION_TRANSPORT_SHIP_GOING_TO_REQUEST:
            f->destination_x = (uint8_t) scenario_data.river_entry_x;
            f->destination_y = (uint8_t) scenario_data.river_entry_y;
            figure_movement_move_ticks(figure_id, speed);
            if (f->direction == DIRECTION_AT_DESTINATION) {
                f->action_state = FIGURE_ACTION_TRANSPORT_SHIP_AWAY_ON_REQUEST;
                figure_route_remove(figure_id);
            } else if (f->direction == DIRECTION_REROUTE) {
                figure_route_remove(figure_id);
            } else if (f->direction == DIRECTION_LOST) {
                f->action_state = FIGURE_ACTION_CORPSE;
            }
            break;
        case FIGURE_ACTION_TRANSPORT_SHIP_AWAY_ON_REQUEST:
            f->is_ghost = 1;
            break;
        case FIGURE_ACTION_TRANSPORT_SHIP_DOCKED:
            ship_move_to_dock(figure_id, &moving);
            transport_docked(figure_id);
            break;
        default:
            f->action_state = FIGURE_ACTION_CORPSE;
            break;
    }
    figure_image_direction = ship_image_direction(f);
    int32_t dir = figure_image_direction;
    if (f->action_state == FIGURE_ACTION_CORPSE) {
        if (f->wait_ticks < 33) {
            int32_t base = image_group(
                map_sweep_has_passed(f->x) ? IMAGE_GROUP_FIGURE_TRANSPORT_SHIP_SWEPT_SINKING : IMAGE_GROUP_FIGURE_TRANSPORT_SHIP_SINKING);
            f->image_id = dir / 8 + base + f->wait_ticks / 3;
        } else {
            f->image_id = ship_sinking_image(f, dir, 0);
        }
    } else if (moving) {
        int32_t water = map_sweep_has_passed(f->x);
        int32_t group = water ? IMAGE_GROUP_FIGURE_TRANSPORT_SHIP_SWEPT_MOVING : IMAGE_GROUP_FIGURE_TRANSPORT_SHIP_MOVING;
        f->image_id = image_group(group) + ((f->animation_frame / 3) * 8) + figure_image_direction;
    } else {
        int32_t water = map_sweep_has_passed(f->x);
        int32_t group = water ? IMAGE_GROUP_FIGURE_TRANSPORT_SHIP_SWEPT_STILL : IMAGE_GROUP_FIGURE_TRANSPORT_SHIP_STILL;
        f->image_id = image_group(group) + figure_image_direction;
    }
    if (selected_transport_id == figure_id && selected_transport_active) {
        f->cart_image_id = ((f->animation_frame << 3) / 12) + image_group(IMAGE_GROUP_FIGURE_SHIP_WAKE);
    }
}

static void soldier_layout_offset(const figure *f, int32_t layout, int32_t check_layout, int16_t *dx, int16_t *dy)
{
    if (f->type == FIGURE_CHARIOTEER && check_layout == 1) {
        layout = 13;
    } else if (f->type == FIGURE_CHARIOTEER && check_layout == 2) {
        layout = 14;
    }
    const formation_layout_spot *spot = &enemy_formation_layout_offsets[layout][f->index_in_formation];
    *dx = (int16_t) spot->x;
    *dy = (int16_t) spot->y;
}

static void soldier_set_destination_spot(figure *f, int32_t formation_id, int32_t layout)
{
    int16_t dx, dy;
    soldier_layout_offset(f, layout, layout, &dx, &dy);
    f->destination_x = (int16_t) (dx + formation_get(formation_id)->order_x);
    f->destination_y = (int16_t) (dy + formation_get(formation_id)->order_y);
    if (f->alternative_location_index) {
        f->destination_x += (int16_t) soldier_alternative_spots[f->alternative_location_index - 1].dx;
        f->destination_y += (int16_t) soldier_alternative_spots[f->alternative_location_index - 1].dy;
    }
}

static void soldier_set_destination_grid(figure *f)
{
    f->destination_grid_offset = map_info.grid_start + f->destination_x + f->destination_y * GRID_SIZE;
}

static void soldier_walk_to_formation_spot(int32_t figure_id, figure *f, int32_t speed)
{
    figure_movement_move_ticks(figure_id, speed);
    if (f->direction == DIRECTION_AT_DESTINATION) {
        f->action_state = FIGURE_ACTION_SOLDIER_AT_REST;
    } else if (f->direction == DIRECTION_REROUTE) {
        figure_route_remove(figure_id);
    } else if (f->direction == DIRECTION_LOST) {
        f->state = FIGURE_STATE_DEAD;
    }
}

static void soldier_take_target(int32_t figure_id, int32_t target_id)
{
    figure *f = figure_get(figure_id);
    figure *target = figure_get(target_id);
    f->destination_x = target->x;
    f->destination_y = target->y;
    f->target_figure_id = (int16_t) target_id;
    target->targeted_by_figure_id = (int16_t) figure_id;
    f->target_figure_created_sequence = target->created_sequence;
}

static void soldier_aim(int32_t figure_id, int32_t target_id)
{
    figure *f = figure_get(figure_id);
    const figure *target = figure_get(target_id);
    f->direction = (int8_t) calc_missile_shooter_direction(f->x, f->y, target->x, target->y);
}

static int32_t soldier_reload_and_look(int32_t figure_id, int32_t extra_range, int32_t previous)
{
    figure *f = figure_get(figure_id);
    if (++f->wait_ticks_missile <= figure_properties[f->type].missile_delay) {
        return previous;
    }
    f->wait_ticks_missile = 0;
    int32_t target_id = figure_combat_get_missile_target_for_soldier(figure_id, figure_properties[f->type].missile_range + extra_range);
    if (!target_id) {
        f->attack_image_offset = 0;
    } else {
        f->attack_image_offset = 1;
        soldier_aim(figure_current_id, target_id);
    }
    return target_id;
}

static void soldier_throw(int32_t formation_id, int32_t target_id)
{
    figure *f = figure_get(figure_current_id);
    if (f->attack_image_offset == 7) {
        const figure *target = figure_get(target_id);
        figure_create_missile(figure_current_id, f->x, f->y, target->x, target->y, FIGURE_JAVELIN);
        formation_get(formation_id)->missile_fired = 6;
    }
}

static void soldier_advance_attack_frame(void)
{
    figure *f = figure_get(figure_current_id);
    if (++f->attack_image_offset > 100) {
        f->attack_image_offset = 0;
    }
}

static int32_t soldier_engage(int32_t formation_id, int32_t target_id, int32_t speed)
{
    figure *f = figure_get(figure_current_id);
    int32_t in_range = 0;
    if (f->type == FIGURE_ARCHER || f->type == FIGURE_CHARIOTEER) {
        const figure *target = figure_get(target_id);
        in_range = calc_maximum_distance(f->x, f->y, target->x, target->y) <= figure_properties[f->type].missile_range;
    }
    if (!target_id) {
        return in_range;
    }
    if (!in_range) {
        figure_movement_move_ticks(figure_current_id, speed);
    }
    if (f->direction == DIRECTION_REROUTE || f->direction == DIRECTION_LOST) {
        f->action_state = FIGURE_ACTION_SOLDIER_AT_STANDARD;
        f->target_figure_id = 0;
        f->animation_frame = 0;
        return in_range;
    }
    if (in_range) {
        soldier_aim(figure_current_id, target_id);
        soldier_throw(formation_id, target_id);
        soldier_advance_attack_frame();
        figure_route_remove(figure_current_id);
    } else if (f->direction == DIRECTION_AT_DESTINATION) {
        const figure *target = figure_get(f->target_figure_id);
        f->destination_x = target->x;
        f->destination_y = target->y;
        figure_route_remove(figure_current_id);
    }
    return in_range;
}

static int32_t soldier_check_target(figure *f)
{
    int32_t target_id = f->target_figure_id;
    if (figure_get(target_id)->state != FIGURE_STATE_ALIVE) {
        target_id = 0;
        f->target_figure_id = 0;
    }
    if (figure_get(target_id)->action_state == FIGURE_ACTION_CORPSE) {
        f->target_figure_id = 0;
        target_id = 0;
        return -1;
    }
    return target_id;
}

static void soldier_lost_tries(figure *f)
{
    if (++f->alternative_location_index > 168) {
        f->state = FIGURE_STATE_DEAD;
    }
    f->animation_frame = 0;
}

void figure_soldier_action(void)
{
    int32_t figure_id = figure_current_id;
    figure *f = figure_get(figure_id);
    if ((map_grids.terrain[f->grid_offset] & (TERRAIN_WATER | TERRAIN_FLOODPLAIN)) == (TERRAIN_WATER | TERRAIN_FLOODPLAIN)) {
        figure_set_state_dead(figure_id);
        return;
    }
    int32_t in_range = 0;
    int32_t formation_id = f->formation_id;
    city_player *player = city_player_current();
    formation *legion = formation_get(formation_id);
    player->soldiers++;
    f->terrain_usage = TERRAIN_USAGE_NO_FERRY;
    if (++f->animation_frame >= 12) {
        f->animation_frame = 0;
    }
    f->cart_image_id = 0;
    if (legion->in_use != 1) {
        f->action_state = FIGURE_ACTION_CORPSE;
    }
    int32_t speed = figure_properties[f->type].speed;
    if (legion->has_move_order && legion->move_order_ticks++ > 2000) {
        legion->has_move_order = 0;
    }
    int32_t layout = legion->layout;
    int32_t spot_layout = f->formation_at_rest || f->action_state == FIGURE_ACTION_SOLDIER_GOING_TO_FORT ? FORMATION_AT_REST : layout;
    {
        int16_t dx, dy;
        soldier_layout_offset(f, spot_layout, spot_layout, &dx, &dy);
        f->formation_position_x = (int16_t) (dx + legion->home_x);
        f->formation_position_y = (int16_t) (dy + legion->home_y);
    }
    switch (f->action_state) {
        case FIGURE_ACTION_ATTACK:
            figure_combat_handle_attack(figure_id);
            break;
        case FIGURE_ACTION_CORPSE:
            figure_combat_handle_corpse(figure_id);
            break;
        case FIGURE_ACTION_SOLDIER_ON_SHIP:
            f->wait_ticks = 0;
            f->formation_at_rest = 0;
            break;
        case FIGURE_ACTION_SOLDIER_AT_REST:
            map_figure_update(figure_id);
            f->wait_ticks = 0;
            f->formation_at_rest = 1;
            f->animation_frame = 0;
            if (f->x != f->formation_position_x || f->y != f->formation_position_y) {
                f->action_state = FIGURE_ACTION_SOLDIER_GOING_TO_FORT;
            }
            break;
        case FIGURE_ACTION_SOLDIER_RETREATING:
            if (legion->ship_figure_id) {
                break;
            }
        case FIGURE_ACTION_SOLDIER_GOING_TO_FORT:
            f->wait_ticks = 0;
            f->formation_at_rest = 1;
            f->destination_x = f->formation_position_x;
            f->destination_y = f->formation_position_y;
            soldier_set_destination_grid(f);
            soldier_walk_to_formation_spot(figure_id, f, speed);
            break;
        case FIGURE_ACTION_SOLDIER_RETURNING_TO_BARRACKS:
            f->formation_at_rest = 1;
            f->destination_x = f->source_x;
            f->destination_y = f->source_y;
            figure_movement_move_ticks(figure_id, speed);
            if (f->direction == DIRECTION_AT_DESTINATION) {
                f->state = FIGURE_STATE_DEAD;
            } else if (f->direction == DIRECTION_REROUTE) {
                figure_route_remove(figure_id);
            } else if (f->direction == DIRECTION_LOST) {
                f->state = FIGURE_STATE_DEAD;
            }
            break;
        case FIGURE_ACTION_SOLDIER_GOING_TO_STANDARD:
            f->formation_at_rest = 0;
            soldier_set_destination_spot(f, formation_id, layout);
            soldier_set_destination_grid(f);
            figure_movement_move_ticks(figure_id, speed);
            if (f->direction == DIRECTION_AT_DESTINATION) {
                f->action_state = (legion->layout == FORMATION_CHARGE) ? FIGURE_ACTION_SOLDIER_ENGAGING : FIGURE_ACTION_SOLDIER_AT_STANDARD;
                f->animation_frame = 0;
            } else if (f->direction == DIRECTION_REROUTE) {
                figure_route_remove(figure_id);
            } else if (f->direction == DIRECTION_LOST) {
                soldier_lost_tries(f);
            }
            break;
        case FIGURE_ACTION_SOLDIER_CHARGE: {
            f->formation_at_rest = 0;
            soldier_set_destination_spot(f, formation_id, layout);
            soldier_set_destination_grid(f);
            if (f->x != f->charge.tile_x || f->y != f->charge.tile_y) {
                f->charge.tile_x = f->x;
                f->charge.tile_y = f->y;
                f->charge.steps++;
            }
            int32_t charge = f->charge.steps;
            figure_movement_move_ticks(figure_id, charge_is_at_full_strength(charge) ? (speed * 2) : speed);
            if (f->direction == DIRECTION_AT_DESTINATION) {
                f->action_state = FIGURE_ACTION_SOLDIER_ENGAGING;
                f->animation_frame = 0;
                legion->layout = FORMATION_CHARGE;
                f->charge.steps = 0;
            } else if (f->direction == DIRECTION_REROUTE) {
                figure_route_remove(figure_id);
            } else if (f->direction == DIRECTION_LOST) {
                soldier_lost_tries(f);
            } else {
                legion->layout = FORMATION_COLUMN;
            }
            break;
        }
        case FIGURE_ACTION_SOLDIER_ENGAGING: {
            f->formation_at_rest = 0;
            const formation_layout_spot *spot = &enemy_formation_layout_offsets[layout][f->index_in_formation];
            int32_t spot_x = legion->order_x + spot->x;
            int32_t spot_y = legion->order_y + spot->y;
            if (f->alternative_location_index) {
                spot_x += soldier_alternative_spots[f->alternative_location_index - 1].dx;
                spot_y += soldier_alternative_spots[f->alternative_location_index - 1].dy;
            }
            int32_t target_id = f->target_figure_id;
            if (figure_get(target_id)->state != FIGURE_STATE_ALIVE) {
                target_id = 0;
                f->target_figure_id = 0;
            }
            if (figure_get(target_id)->action_state == FIGURE_ACTION_CORPSE) {
                f->target_figure_id = 0;
                target_id = 0;
            }
            int32_t target_type = figure_get(target_id)->type;
            if (target_type == FIGURE_ARCHER || target_type == FIGURE_INFANTRY || target_type == FIGURE_CHARIOTEER) {
                target_id = 0;
                f->target_figure_id = 0;
            }
            if (calc_maximum_distance(f->x, f->y, spot_x, spot_y) >= military_rules[MILITARY_RULE_SOLDIER_ENGAGE_RADIUS]) {
                f->action_state = FIGURE_ACTION_SOLDIER_AT_STANDARD;
                f->target_figure_id = 0;
                f->animation_frame = 0;
                break;
            }
            if (target_id <= 0) {
                if (f->type == FIGURE_ARCHER || f->type == FIGURE_CHARIOTEER) {
                    target_id = soldier_reload_and_look(figure_id, military_rules[MILITARY_RULE_SOLDIER_ENGAGE_RADIUS], target_id);
                } else {
                    target_id = soldier_find_enemy(f->x, f->y, military_rules[MILITARY_RULE_SOLDIER_ENGAGE_RADIUS], figure_id);
                }
                if (target_id) {
                    soldier_take_target(figure_id, target_id);
                } else {
                    if (spot_x == f->x && spot_y == f->y) {
                        f->animation_frame = 0;
                    } else {
                        f->action_state = FIGURE_ACTION_SOLDIER_AT_STANDARD;
                    }
                }
                figure_route_remove(figure_id);
            }
            in_range = soldier_engage(formation_id, target_id, speed);
            break;
        }
        case FIGURE_ACTION_SOLDIER_AT_STANDARD: {
            f->formation_at_rest = 0;
            f->animation_frame = 0;
            soldier_set_destination_spot(f, formation_id, layout);
            if (f->x == f->destination_x && f->y == f->destination_y) {
                if ((int16_t) layout == FORMATION_CHARGE) {
                    f->action_state = FIGURE_ACTION_SOLDIER_GOING_TO_STANDARD;
                    f->alternative_location_index = 0;
                }
            } else if (legion->missile_fired <= 0 && legion->recent_fight <= 0 && legion->missile_attack_timeout <= 0) {
                f->action_state = FIGURE_ACTION_SOLDIER_GOING_TO_STANDARD;
                f->alternative_location_index = 0;
            }
            if (f->type == FIGURE_ARCHER) {
                if (f->action_state == FIGURE_ACTION_SOLDIER_GOING_TO_STANDARD) {
                    break;
                }
                int32_t target_id = f->target_figure_id;
                if (++f->wait_ticks_missile > figure_properties[f->type].missile_delay) {
                    f->wait_ticks_missile = 0;
                    target_id = figure_combat_get_missile_target_for_soldier(figure_id, figure_properties[f->type].missile_range);
                    if (!target_id) {
                        f->attack_image_offset = 0;
                        f->target_figure_id = 0;
                        break;
                    }
                    f->attack_image_offset = 1;
                    soldier_aim(figure_id, target_id);
                }
                if (!target_id) {
                    break;
                }
                figure *target = figure_get(target_id);
                f->target_figure_id = (int16_t) target_id;
                in_range = 0;
                target->targeted_by_figure_id = (int16_t) figure_id;
                f->target_figure_created_sequence = target->created_sequence;
                int32_t x = f->x, y = f->y;
                if (calc_maximum_distance(x, y, target->x, target->y) > figure_properties[f->type].missile_range) {
                    break;
                }
                in_range = 1;
                f->direction = (int8_t) calc_missile_shooter_direction(x, y, target->x, target->y);
                soldier_throw(formation_id, target_id);
                soldier_advance_attack_frame();
                figure_route_remove(figure_id);
            } else if (f->type == FIGURE_CHARIOTEER) {
                if (f->action_state == FIGURE_ACTION_SOLDIER_GOING_TO_STANDARD) {
                    break;
                }
                if (++f->wait_ticks_missile > figure_properties[f->type].missile_delay) {
                    f->wait_ticks_missile = 0;
                    int32_t target_id = figure_combat_get_missile_target_for_soldier(figure_id, figure_properties[f->type].missile_range);
                    f->target_figure_id = (int16_t) target_id;
                    if (target_id) {
                        f->attack_image_offset = 1;
                        soldier_aim(figure_id, target_id);
                    } else {
                        f->attack_image_offset = 0;
                    }
                }
                if (!f->attack_image_offset) {
                    figure_combat_attack_adjacent(figure_id);
                    break;
                }
                int32_t target_id = f->target_figure_id;
                if (target_id) {
                    soldier_aim(figure_id, target_id);
                    soldier_throw(formation_id, target_id);
                }
                soldier_advance_attack_frame();
            }
            break;
        }
        case FIGURE_ACTION_SOLDIER_MOPPING_UP: {
            f->formation_at_rest = 0;
            int32_t target_id = soldier_check_target(f);
            if (target_id <= 0) {
                target_id = 0;
                if (f->type == FIGURE_ARCHER || f->type == FIGURE_CHARIOTEER) {
                    target_id = soldier_reload_and_look(figure_id, military_rules[MILITARY_RULE_SOLDIER_MOP_UP_RANGE], 0);
                } else {
                    target_id = soldier_find_enemy(f->x, f->y, military_rules[MILITARY_RULE_SOLDIER_MOP_UP_RANGE], figure_id);
                }
                if (target_id) {
                    soldier_take_target(figure_id, target_id);
                } else {
                    f->action_state = FIGURE_ACTION_SOLDIER_AT_STANDARD;
                    f->animation_frame = 0;
                }
                figure_route_remove(figure_id);
            }
            in_range = soldier_engage(formation_id, target_id, speed);
            break;
        }
        case FIGURE_ACTION_SOLDIER_ATTACKING_FORMATION: {
            f->formation_at_rest = 0;
            int32_t target_id = f->target_figure_id;
            if (figure_get(target_id)->state != FIGURE_STATE_ALIVE) {
                target_id = 0;
                f->target_figure_id = 0;
            }
            if (figure_get(target_id)->action_state == FIGURE_ACTION_CORPSE) {
                target_id = 0;
                f->target_figure_id = 0;
            }
            int32_t target_formation = legion->attack_formation_id;
            int32_t search = 0;
            if (figure_get(target_id)->formation_id != target_formation) {
                f->target_figure_id = 0;
                search = 1;
            } else if (target_id <= 0) {
                search = 1;
            }
            if (search) {
                if (f->type == FIGURE_INFANTRY) {
                    target_id = soldier_find_enemy(f->x, f->y, military_rules[MILITARY_RULE_SOLDIER_ATTACK_FORMATION_RANGE], figure_id);
                } else {
                    target_id = figure_combat_get_missile_target_for_soldier(figure_id, military_rules[MILITARY_RULE_SOLDIER_ATTACK_FORMATION_RANGE]);
                }
                target_formation = legion->attack_formation_id;
                if (target_formation == -1 || !target_id) {
                    target_id = 0;
                } else {
                    soldier_take_target(figure_id, target_id);
                }
            }
            if (target_formation == -1 || !target_id) {
                legion->attack_formation_id = -1;
                f->action_state = FIGURE_ACTION_SOLDIER_GOING_TO_STANDARD;
                f->animation_frame = 0;
                break;
            }
            if (f->type == FIGURE_ARCHER || f->type == FIGURE_CHARIOTEER) {
                const figure *target = figure_get(target_id);
                if (calc_maximum_distance(f->x, f->y, target->x, target->y) <= figure_properties[f->type].missile_range) {
                    f->animation_frame = 0;
                    in_range = 1;
                    soldier_aim(figure_id, target_id);
                    soldier_throw(formation_id, target_id);
                    soldier_advance_attack_frame();
                    figure_route_remove(figure_id);
                    break;
                }
            }
            in_range = 0;
            figure_movement_move_ticks(figure_id, speed);
            if (f->direction == DIRECTION_AT_DESTINATION) {
                const figure *target = figure_get(f->target_figure_id);
                f->destination_x = target->x;
                f->destination_y = target->y;
                figure_route_remove(figure_id);
            }
            break;
        }
        case FIGURE_ACTION_SOLDIER_GOING_TO_ACADEMY:
            f->terrain_usage = TERRAIN_USAGE_ANY_OR_FORT;
            legion->has_military_training = 1;
            f->formation_at_rest = 1;
            figure_movement_move_ticks(figure_id, speed);
            if (f->direction == DIRECTION_AT_DESTINATION) {
                f->action_state = FIGURE_ACTION_SOLDIER_TRAINED_GOING_TO_FORT;
            } else if (f->direction == DIRECTION_REROUTE) {
                figure_route_remove(figure_id);
            } else if (f->direction == DIRECTION_LOST) {
                f->state = FIGURE_STATE_DEAD;
            }
            break;
        case FIGURE_ACTION_SOLDIER_GOING_TO_REQUEST:
            f->formation_at_rest = 0;
            f->destination_x = player->exit_point_x;
            f->destination_y = player->exit_point_y;
            figure_movement_move_ticks(figure_id, speed);
            if (f->direction == DIRECTION_AT_DESTINATION || f->direction == DIRECTION_REROUTE || f->direction == DIRECTION_LOST) {
                f->action_state = FIGURE_ACTION_SOLDIER_AWAY_ON_REQUEST;
                figure_route_remove(figure_id);
                map_figure_delete(figure_id);
            }
            break;
        case FIGURE_ACTION_SOLDIER_AWAY_ON_REQUEST:
            f->is_ghost = 1;
            f->formation_at_rest = 1;
            break;
        case FIGURE_ACTION_SOLDIER_RETURNING_FROM_REQUEST:
            f->is_ghost = 0;
            f->wait_ticks = 0;
            f->formation_at_rest = 1;
            f->destination_x = f->formation_position_x;
            f->destination_y = f->formation_position_y;
            soldier_set_destination_grid(f);
            soldier_walk_to_formation_spot(figure_id, f, speed);
            break;
        case FIGURE_ACTION_SOLDIER_RECRUIT_GOING_TO_FORT:
        case FIGURE_ACTION_SOLDIER_TRAINED_GOING_TO_FORT:
            f->terrain_usage = TERRAIN_USAGE_ANY_OR_FORT;
            f->wait_ticks = 0;
            f->formation_at_rest = 1;
            f->destination_x = f->formation_position_x;
            f->destination_y = f->formation_position_y;
            soldier_set_destination_grid(f);
            soldier_walk_to_formation_spot(figure_id, f, speed);
            break;
        case FIGURE_ACTION_ON_FERRY:
            f->terrain_usage = TERRAIN_USAGE_ANY_OR_FORT;
            f->image_id = 0;
            f->cart_image_id = 0;
            break;
        case FIGURE_ACTION_WAIT_FOR_FERRY:
            f->terrain_usage = TERRAIN_USAGE_ANY_OR_FORT;
            f->animation_frame = 0;
            if (++f->wait_ticks > 100) {
                f->wait_ticks = 0;
                if (!figure_is_queued_for_ferry(figure_id)) {
                    f->state = FIGURE_STATE_DEAD;
                }
            }
            break;
        default:
            f->action_state = FIGURE_ACTION_SOLDIER_AT_REST;
            break;
    }

    int32_t javelin_dir = 0;
    int32_t dir;
    if (f->type == FIGURE_ARCHER) {
        int32_t target_id = f->target_figure_id;
        if (target_id && f->action_state == FIGURE_ACTION_SOLDIER_AT_STANDARD) {
            soldier_aim(figure_id, target_id);
        }
        dir = f->direction >= DIRECTION_COUNT ? f->previous_tile_direction : f->direction;
        dir -= map_info.orientation;
        figure_image_direction = dir;
        javelin_dir = dir + 2;
        if (javelin_dir > DIRECTION_TOP_LEFT) {
            javelin_dir -= DIRECTION_COUNT;
        }
    } else {
        if (f->action_state == FIGURE_ACTION_ATTACK) {
            dir = f->attack_direction;
        } else {
            int32_t missile_fired = legion->missile_fired;
            dir = -100;
            if (missile_fired && f->direction < DIRECTION_COUNT) {
                dir = f->direction;
            } else if (missile_fired && f->previous_tile_direction < 8) {
                dir = f->previous_tile_direction;
            } else if (f->action_state == FIGURE_ACTION_SOLDIER_AT_STANDARD && legion->direction < DIRECTION_COUNT && !f->attack_image_offset) {
                dir = legion->direction;
            } else if (f->action_state == FIGURE_ACTION_SOLDIER_AT_STANDARD && legion->direction_hold_ticks < DIRECTION_COUNT && !f->attack_image_offset) {
                dir = legion->direction_hold_ticks;
            } else {
                dir = f->direction >= DIRECTION_COUNT ? f->previous_tile_direction : f->direction;
            }
        }
        dir -= map_info.orientation;
        figure_image_direction = dir;
    }
    if (dir < 0) {
        dir += DIRECTION_COUNT;
        figure_image_direction = dir;
    }
    if (dir >= DIRECTION_COUNT) {
        dir -= DIRECTION_COUNT;
        figure_image_direction = dir;
    }
    int32_t alt = scenario_data.soldier_images_alt != 0;
    int32_t frame = f->animation_frame;
    int32_t attack = f->attack_image_offset;
    int32_t corpse = figure_corpse_offsets[f->wait_ticks / 2];
    if (f->type == FIGURE_CHARIOTEER) {
        f->cart_image_id = image_group(IMAGE_GROUP_FIGURE_CHARIOT_CART) + frame * 8 + dir;
        f->cart_offset_x = (int8_t) chariot_cart_offsets[dir].dx;
        f->cart_offset_y = (int8_t) chariot_cart_offsets[dir].dy;
        int32_t fight = image_group(alt ? IMAGE_GROUP_FIGURE_ENEMY_CHARIOT_ATTACK : IMAGE_GROUP_FIGURE_CHARIOTEER_ATTACK);
        int32_t walk = image_group(alt ? IMAGE_GROUP_FIGURE_ENEMY_CHARIOT : IMAGE_GROUP_FIGURE_CHARIOTEER);
        if (f->action_state == FIGURE_ACTION_ATTACK) {
            f->image_id = (attack < 12) ? (fight + dir) : (fight + (((attack - 12) / 2) * 8) + dir);
        } else if (f->action_state == FIGURE_ACTION_SOLDIER_AT_STANDARD) {
            f->image_id = walk + dir;
        } else if (f->action_state == FIGURE_ACTION_SOLDIER_ENGAGING && attack) {
            f->image_id = fight + figure_missile_frames[attack >> 1] * 8 + dir;
        } else if (f->action_state == FIGURE_ACTION_CORPSE) {
            f->image_id = corpse + image_group(alt ? IMAGE_GROUP_FIGURE_ENEMY_CHARIOT_DEATH : IMAGE_GROUP_FIGURE_CHARIOTEER_DEATH);
        } else {
            f->image_id = walk + frame * 8 + dir;
        }
    } else if (f->type == FIGURE_INFANTRY) {
        f->cart_image_id = 0;
        int32_t fight = image_group(alt ? IMAGE_GROUP_FIGURE_ENEMY_INFANTRY_ATTACK : IMAGE_GROUP_FIGURE_INFANTRY_ATTACK);
        if (f->action_state == FIGURE_ACTION_ATTACK) {
            f->image_id = (attack < 12) ? (fight + dir) : (fight + (((attack - 12) / 2) * 8) + dir);
        } else if (f->action_state == FIGURE_ACTION_CORPSE) {
            f->image_id = corpse + image_group(alt ? IMAGE_GROUP_FIGURE_ENEMY_INFANTRY_DEATH : IMAGE_GROUP_FIGURE_INFANTRY_DEATH);
        } else {
            f->image_id = image_group(alt ? IMAGE_GROUP_FIGURE_ENEMY_INFANTRY : IMAGE_GROUP_FIGURE_INFANTRY) + frame * 8 + dir;
        }
    } else if (f->type == FIGURE_ARCHER) {
        f->cart_image_id = 0;
        int32_t throw_base = image_group(alt ? IMAGE_GROUP_FIGURE_ENEMY_ARCHER_ATTACK : IMAGE_GROUP_FIGURE_ARCHER_ATTACK);
        int32_t walk = image_group(alt ? IMAGE_GROUP_FIGURE_ENEMY_ARCHER : IMAGE_GROUP_FIGURE_ARCHER) + frame * 8 + dir;
        if (f->action_state == FIGURE_ACTION_ATTACK) {
            f->image_id = attack < 12 ? throw_base + attack * 8 + javelin_dir : throw_base + javelin_dir;
        } else if (f->action_state == FIGURE_ACTION_SOLDIER_AT_STANDARD) {
            if (f->target_figure_id == 0) {
                f->image_id = walk;
            }
            if (attack && attack < 12) {
                f->image_id = throw_base + attack * 8 + javelin_dir;
            } else {
                f->image_id = throw_base + javelin_dir + 88;
            }
        } else if (f->action_state == FIGURE_ACTION_SOLDIER_ENGAGING && in_range) {
            if (attack && attack < 12) {
                f->image_id = throw_base + attack * 8 + javelin_dir;
            } else {
                f->image_id = throw_base + javelin_dir + 88;
            }
        } else if (f->action_state == FIGURE_ACTION_CORPSE) {
            f->image_id = corpse + image_group(alt ? IMAGE_GROUP_FIGURE_ENEMY_ARCHER_DEATH : IMAGE_GROUP_FIGURE_ARCHER_DEATH);
        } else {
            f->image_id = walk;
        }
    }
    if (f->action_state == FIGURE_ACTION_SOLDIER_ON_SHIP) {
        f->image_id = 0;
        f->cart_image_id = 0;
    }
}
