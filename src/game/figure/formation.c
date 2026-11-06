// The formations of the legions and of the invaders: the per-tick update (figure lists, hit points, army totals,
// healing, morale, fight counters), the monthly morale with its rules, the invaders' regroup, advance and halt cycle
// towards their target, and the barracks sending recruits to the legions and sentries to the towers. A new fort gets a
// legion and its standard bearer here.
//
// The formation update keeps these side effects:
// - figures without a formation count into record 0, which nothing clears;
// - a retreating formation sends every slot home, the empty ones too (figure 0 loses its route);
// - the target found by the first formation of an invasion is kept for the later formations of the loop;
// - a legion on a ship counts as fighting;
// - the legion index search falls back to index 0 when all eight are taken.

#include "figure/formation.h"
#include "building/building_data.h"
#include "building/building_type.h"
#include "building/temple_complex.h"
#include "core/calc.h"
#include "figure/animals.h"
#include "figure/figure.h"
#include "figure/figure_action.h"
#include "figure/figure_data.h"
#include "figure/figure_type.h"
#include "figure/formation_enemy.h"
#include "figure/herds.h"
#include "figure/military_internal.h"
#include "figure/route.h"
#include "map/grid.h"
#include "map/map_data.h"
#include "map/road_access.h"
#include "map/routing_enemy.h"
#include "map/terrain.h"
#include "scenario/event_manager.h"
#include "scenario/scenario_internal.h"
#include "window/window_data.h"
#include <stdint.h>
#include <string.h>

int32_t formation_enemy_move_offsets[50];

uint8_t formation_empty_event_flags[52];

int32_t formation_empty_event_ids[50];

static uint8_t mission_reserved_legion_id(int32_t slot)
{
    return record_savings.started.legions[slot].legion_id;
}

int32_t formation_morale_rules[FORMATION_MORALE_RULE_COUNT] = {
    [FORMATION_MORALE_RULE_NEW_LEGION] = 50,
    [FORMATION_MORALE_RULE_CHARIOTEERS_MAX] = 80,
    [FORMATION_MORALE_RULE_TRAINED_CHARIOTEERS_MAX] = 100,
    [FORMATION_MORALE_RULE_KINGDOM_SOLDIERS_MAX] = 100,
    [FORMATION_MORALE_RULE_FOOT_SOLDIERS_MAX] = 60,
    [FORMATION_MORALE_RULE_TRAINED_FOOT_SOLDIERS_MAX] = 80,
    [FORMATION_MORALE_RULE_ENEMY_MAX] = 70,
    [FORMATION_MORALE_RULE_ENEMY_13_MAX] = 80,
    [FORMATION_MORALE_RULE_AT_FORT_GAIN] = 5,
    [FORMATION_MORALE_RULE_AWAY_LOSS] = 5,
    [FORMATION_MORALE_RULE_MONTHS_AWAY_WITHOUT_LOSS] = 3,
    [FORMATION_MORALE_RULE_LOW] = 20,
    [FORMATION_MORALE_RULE_VERY_LOW] = 10,
    [FORMATION_MORALE_RULE_ROUT_SWING] = 10,
    [FORMATION_MORALE_RULE_DEATH_SHARE_1] = 8,
    [FORMATION_MORALE_RULE_DEATH_SHARE_2] = 10,
    [FORMATION_MORALE_RULE_DEATH_SHARE_3] = 14,
    [FORMATION_MORALE_RULE_DEATH_SHARE_4] = 20,
    [FORMATION_MORALE_RULE_DEATH_SHARE_5] = 30,
    [FORMATION_MORALE_RULE_DEATH_LOSS_1] = 5,
    [FORMATION_MORALE_RULE_DEATH_LOSS_2] = 7,
    [FORMATION_MORALE_RULE_DEATH_LOSS_3] = 10,
    [FORMATION_MORALE_RULE_DEATH_LOSS_4] = 12,
    [FORMATION_MORALE_RULE_DEATH_LOSS_5] = 15,
    [FORMATION_MORALE_RULE_DEATH_LOSS_6] = 20,
};

static int32_t is_formation_figure(int32_t type)
{
    if ((type >= FIGURE_ARCHER && type <= FIGURE_CHARIOTEER) || (type >= FIGURE_ENEMY_ARCHER && type <= FIGURE_ENEMY_BARBARIAN_HEAVY_2) ||
        type == FIGURE_KINGDOM_SOLDIER) {
        return 1;
    }
    if (figure_is_herd_animal(type) || figure_is_predator_animal(type)) {
        return 1;
    }
    return type == FIGURE_BEDOUIN || type == FIGURE_ENEMY_EGYPTIAN_INFANTRY || type == FIGURE_ENEMY_EGYPTIAN_ARCHER || type == FIGURE_ENEMY_EGYPTIAN_CHARIOT;
}

void formation_calculate_legion_totals(void)
{
    int32_t count = 0;
    city_player *player = city_player_current();
    player->chariot_legions = 0;
    formation_id_last_legion = 0;
    formation_num_legions = 0;
    for (int32_t id = 1; id < MAX_FORMATIONS; id++) {
        formation *legion = formation_get(id);
        if (legion->in_use != 1 || !legion->is_legion) {
            continue;
        }
        count++;
        formation_id_last_legion = id;
        if (legion->figure_type == FIGURE_CHARIOTEER) {
            player->chariot_legions++;
        }
        if (legion->missile_attack_timeout > 0) {
            continue;
        }
        for (int32_t i = 0; i < FORMATION_MAX_FIGURES; i++) {
            int32_t figure_id = legion->figures[i];
            if (!figure_id) {
                break;
            }
            figure *f = figure_get(figure_id);
            if (f->state == FIGURE_STATE_ALIVE && f->action_state != FIGURE_ACTION_ON_FERRY && f->action_state != FIGURE_ACTION_WAIT_FOR_FERRY &&
                f->action_state != FIGURE_ACTION_SOLDIER_RECRUIT_GOING_TO_FORT && f->action_state != FIGURE_ACTION_SOLDIER_TRAINED_GOING_TO_FORT) {
                legion->x = f->x;
                legion->y = f->y;
                break;
            }
        }
    }
    formation_num_legions = count;
}

int32_t formation_legion_is_on_ship(int32_t formation_id)
{
    formation *legion = formation_get(formation_id);
    for (int32_t i = 0; i < FORMATION_MAX_FIGURES; i++) {
        int32_t figure_id = legion->figures[i];
        if (!figure_id) {
            return 0;
        }
        if (figure_get(figure_id)->action_state == FIGURE_ACTION_SOLDIER_ON_SHIP) {
            return 1;
        }
    }
    return 0;
}

static int32_t is_wounded_soldier_at_rest(const figure *f)
{
    int32_t is_soldier = f->type > FIGURE_FIREMAN && f->type < FIGURE_STANDARD_BEARER;
    return f->state == FIGURE_STATE_ALIVE && is_soldier && f->action_state == FIGURE_ACTION_SOLDIER_AT_REST && f->damage;
}

void formation_legion_decrease_damage(void)
{
    for (int32_t id = 1; id < MAX_FIGURES; id++) {
        figure *f = figure_get(id);
        if (is_wounded_soldier_at_rest(f)) {
            f->damage--;
        }
    }
}

void formation_change_morale(int32_t formation_id, int32_t amount)
{
    if (!formation_id) {
        return;
    }
    formation *m = formation_get(formation_id);
    int32_t type = m->figure_type;
    int32_t max_rule;
    if (type == FIGURE_CHARIOTEER) {
        max_rule = m->has_military_training ? FORMATION_MORALE_RULE_TRAINED_CHARIOTEERS_MAX : FORMATION_MORALE_RULE_CHARIOTEERS_MAX;
    } else if (type == FIGURE_KINGDOM_SOLDIER) {
        max_rule = FORMATION_MORALE_RULE_KINGDOM_SOLDIERS_MAX;
    } else if (type == FIGURE_INFANTRY || type == FIGURE_ARCHER) {
        max_rule = m->has_military_training ? FORMATION_MORALE_RULE_TRAINED_FOOT_SOLDIERS_MAX : FORMATION_MORALE_RULE_FOOT_SOLDIERS_MAX;
    } else {
        max_rule = (m->enemy_type != 13) ? FORMATION_MORALE_RULE_ENEMY_MAX : FORMATION_MORALE_RULE_ENEMY_13_MAX;
    }
    int32_t max_morale = formation_morale_rules[max_rule];
    m->morale += (int16_t) amount;
    if (m->morale > max_morale) {
        m->morale = (int16_t) max_morale;
    }
    if (m->morale < 0) {
        m->morale = 0;
    }
}

void formation_update_morale_after_death(int32_t formation_id)
{
    if (!formation_id) {
        return;
    }
    formation_calculate_figures();
    int32_t share = calc_percentage(1, formation_get(formation_id)->num_figures);
    int32_t step = 0;
    while (step < FORMATION_DEATH_SHARE_STEPS && share >= formation_morale_rules[FORMATION_MORALE_RULE_DEATH_SHARE_1 + step]) {
        step++;
    }
    formation_change_morale(formation_id, -formation_morale_rules[FORMATION_MORALE_RULE_DEATH_LOSS_1 + step]);
}

static void change_all_morale(int32_t amount, int32_t legions)
{
    for (int32_t id = 1; id < MAX_FORMATIONS; id++) {
        formation *m = formation_get(id);
        if (m->in_use == 1 && !m->herd_kind && (m->is_legion != 0) == legions) {
            formation_change_morale(id, amount);
        }
    }
}

void formation_update_monthly_morale_at_rest(void)
{
    for (int32_t id = 1; id < MAX_FORMATIONS; id++) {
        formation *m = formation_get(id);
        if (m->in_use != 1 || m->herd_kind) {
            continue;
        }
        if (!m->is_legion) {
            formation_change_morale(id, 0);
        } else if (m->is_at_fort) {
            m->months_from_home = 0;
            m->months_very_low_morale = 0;
            m->months_low_morale = 0;
            formation_change_morale(id, formation_morale_rules[FORMATION_MORALE_RULE_AT_FORT_GAIN]);
            if (m->layout == FORMATION_MOP_UP) {
                m->layout = m->prev_layout;
            }
        } else if (m->recent_fight == 0) {
            uint8_t months = ++m->months_from_home;
            if (months > formation_morale_rules[FORMATION_MORALE_RULE_MONTHS_AWAY_WITHOUT_LOSS]) {
                if (months > FORMATION_MONTHS_AWAY_MAX) {
                    m->months_from_home = FORMATION_MONTHS_AWAY_MAX;
                }
                formation_change_morale(id, -formation_morale_rules[FORMATION_MORALE_RULE_AWAY_LOSS]);
            }
        }
    }
}

void formation_update_monthly_morale_deployed(void)
{
    for (int32_t id = 1; id < MAX_FORMATIONS; id++) {
        formation *m = formation_get(id);
        if (m->in_use != 1 || m->herd_kind) {
            continue;
        }
        if (!m->is_legion) {
            if (m->morale <= formation_morale_rules[FORMATION_MORALE_RULE_LOW] && !m->months_low_morale && !m->months_very_low_morale) {
                formation_change_all_legion_morale(formation_morale_rules[FORMATION_MORALE_RULE_ROUT_SWING]);
                formation_change_all_enemy_morale(-formation_morale_rules[FORMATION_MORALE_RULE_ROUT_SWING]);
            }
        } else {
            if (m->is_at_fort || m->request_event_id >= 0) {
                continue;
            }
            if (m->morale <= formation_morale_rules[FORMATION_MORALE_RULE_LOW] && !m->months_low_morale && !m->months_very_low_morale) {
                formation_change_all_legion_morale(-formation_morale_rules[FORMATION_MORALE_RULE_ROUT_SWING]);
                formation_change_all_enemy_morale(formation_morale_rules[FORMATION_MORALE_RULE_ROUT_SWING]);
            }
        }
        if (m->morale <= formation_morale_rules[FORMATION_MORALE_RULE_VERY_LOW]) {
            m->months_very_low_morale++;
        } else if (m->morale <= formation_morale_rules[FORMATION_MORALE_RULE_LOW]) {
            m->months_low_morale++;
        }
    }
}

void formation_legion_set_max_figures(void)
{
    for (int32_t id = 1; id < MAX_FORMATIONS; id++) {
        formation *m = formation_get(id);
        if (m->in_use == 1 && m->is_legion) {
            m->max_figures = FORMATION_MAX_FIGURES;
        }
    }
}

void formation_calculate_figures(void)
{
    int32_t num_events = 0;
    for (int32_t id = 1; id < MAX_FORMATIONS; id++) {
        formation *m = formation_get(id);
        for (int32_t i = 0; i < FORMATION_MAX_FIGURES; i++) {
            m->figures[i] = 0;
        }
        m->num_figures = 0;
        m->is_at_fort = 1;
        m->total_damage = 0;
        m->max_total_damage = 0;
    }
    for (int32_t figure_id = 1; figure_id < MAX_FIGURES; figure_id++) {
        figure *f = figure_get(figure_id);
        if (f->state != FIGURE_STATE_ALIVE || f->action_state == FIGURE_ACTION_CORPSE || !is_formation_figure(f->type)) {
            continue;
        }
        int32_t id = f->formation_id;
        formation *m = formation_get(id);
        m->num_figures++;
        m->max_total_damage += (int16_t) figure_properties[f->type].max_damage;
        m->total_damage += f->damage;
        if (f->formation_at_rest != 1) {
            m->is_at_fort = 0;
        }
        for (int32_t i = 0; i < FORMATION_MAX_FIGURES; i++) {
            if (m->figures[i] == 0) {
                f->index_in_formation = (uint8_t) i;
                m->figures[i] = (int16_t) figure_id;
                break;
            }
        }
    }
    enemy_army_totals.enemy_formations = 0;
    enemy_army_totals.enemy_strength = 0;
    enemy_army_totals.legion_formations = 0;
    enemy_army_totals.legion_strength = 0;
    for (int32_t id = 1; id < MAX_FORMATIONS; id++) {
        formation *m = formation_get(id);
        if (m->in_use != 1 || m->herd_kind) {
            continue;
        }
        int32_t num = m->num_figures;
        if (m->is_legion) {
            if (!num) {
                continue;
            }
            m->is_halted = 1;
            for (int32_t i = 0; i < m->num_figures; i++) {
                int32_t figure_id = m->figures[i];
                if (figure_id && figure_get(figure_id)->direction != DIRECTION_AT_DESTINATION) {
                    m->is_halted = 0;
                }
            }
            enemy_army_totals.legion_formations++;
            enemy_army_totals.legion_strength += m->num_figures;
            if (m->figure_type == FIGURE_CHARIOTEER) {
                enemy_army_totals.legion_strength += m->num_figures >> 1;
            }
        } else if (num) {
            enemy_army_totals.enemy_formations++;
            enemy_army_totals.enemy_strength += num;
        } else {
            int32_t event_id = m->invasion_event_id;
            int32_t known = 0;
            for (int32_t i = 0; i < num_events; i++) {
                if (formation_empty_event_ids[i] == event_id) {
                    known = 1;
                    break;
                }
            }
            if (!known) {
                formation_empty_event_ids[num_events] = event_id;
                formation_empty_event_flags[num_events] = 1;
                num_events++;
            }
            memset(m, 0, sizeof(formation));
        }
    }
    city_player *player = city_player_current();
    player->empire_service_legions = 0;
    player->total_soldiers = 0;
    player->total_legions = 0;
    player->legions_fighting = 0;
    player->legions_not_fighting = 0;
    player->empire_service_legions_fighting = 0;
    player->empire_service_legions_not_fighting = 0;
    for (int32_t id = 1; id < MAX_FORMATIONS; id++) {
        formation *m = formation_get(id);
        if (m->in_use != 1) {
            continue;
        }
        if (!m->is_legion) {
            int32_t event_id = m->invasion_event_id;
            for (int32_t i = 0; i < num_events; i++) {
                if (event_id == formation_empty_event_ids[i]) {
                    formation_empty_event_flags[i] = 0;
                }
            }
            continue;
        }
        int32_t fighting = formation_legion_is_on_ship(id) != 0;
        player->total_legions++;
        if (fighting) {
            player->legions_fighting++;
        } else {
            player->legions_not_fighting++;
        }
        if (m->empire_service && m->num_figures) {
            player->empire_service_legions++;
            if (fighting) {
                player->empire_service_legions_fighting++;
            } else {
                player->empire_service_legions_not_fighting++;
            }
        }
        player->total_soldiers += m->num_figures;
    }
    for (int32_t i = 0; i < num_events; i++) {
        int32_t event_id = formation_empty_event_ids[i];
        if (formation_empty_event_flags[i] && event_id >= 0 && event_id < scenario_events.count) {
            scenario_events.events[event_id].is_active = 0;
        }
    }
}

void formation_update_directions(void)
{
    for (int32_t id = 1; id < MAX_FORMATIONS; id++) {
        formation *m = formation_get(id);
        if (m->in_use != 1 || m->herd_kind) {
            continue;
        }
        int32_t x = m->x, y = m->y;
        int32_t prev_x = m->prev_x, prev_y = m->prev_y;
        int32_t dx = (x < prev_x) ? prev_x - x : x - prev_x;
        int32_t dy = (y < prev_y) ? prev_y - y : y - prev_y;
        if (m->direction_hold_ticks) {
            m->direction_hold_ticks--;
        } else if (m->missile_fired) {
            m->direction = (uint8_t) figure_get(m->figures[0])->direction;
        } else {
            int32_t layout = m->layout;
            int32_t turn = FORMATION_KEEPS_DIRECTION;
            if (layout == FORMATION_DOUBLE_LINE_1 || layout == FORMATION_SINGLE_LINE_1) {
                turn = FORMATION_TURNS_ALONG_Y;
            } else if (layout == FORMATION_DOUBLE_LINE_2 || layout == FORMATION_SINGLE_LINE_2) {
                turn = FORMATION_TURNS_ALONG_X;
            } else if (layout == FORMATION_COLUMN || layout == FORMATION_TORTOISE) {
                turn = (dx <= dy) ? FORMATION_TURNS_ALONG_Y : FORMATION_TURNS_ALONG_X;
            }
            if (turn == FORMATION_TURNS_ALONG_Y) {
                if (y < prev_y) {
                    m->direction = DIRECTION_TOP;
                } else if (y > prev_y) {
                    m->direction = DIRECTION_BOTTOM;
                }
            } else if (turn == FORMATION_TURNS_ALONG_X) {
                if (x < prev_x) {
                    m->direction = DIRECTION_LEFT;
                } else if (x > prev_x) {
                    m->direction = DIRECTION_RIGHT;
                }
            }
        }
        m->prev_x = (int16_t) x;
        m->prev_y = (int16_t) y;
    }
}

void formation_legion_update(void)
{
    const city_player *player = city_player_current();
    for (int32_t id = 1; id <= FORMATION_MAX_LEGIONS; id++) {
        formation *legion = formation_get(id);
        if (legion->in_use != 1 || !legion->is_legion) {
            continue;
        }
        if (legion->cursed_by_seth) {
            legion->cursed_by_seth--;
        }
        if (legion->missile_fired) {
            legion->missile_fired--;
        }
        if (legion->missile_attack_timeout) {
            legion->missile_attack_timeout--;
        }
        if (legion->recent_fight) {
            legion->recent_fight--;
        }
        if (player->enemies < 1) {
            legion->recent_fight = 0;
            legion->missile_attack_timeout = 0;
            legion->missile_fired = 0;
        }
        for (int32_t i = 0; i < FORMATION_MAX_FIGURES; i++) {
            if (figure_get(legion->figures[i])->action_state == FIGURE_ACTION_ATTACK) {
                legion->recent_fight = FORMATION_RECENT_FIGHT_TICKS;
            }
        }
        if (!legion->months_very_low_morale && !legion->months_low_morale) {
            if (legion->layout != FORMATION_MOP_UP) {
                continue;
            }
            if (player->rioters + player->attacking_natives + enemy_army_totals.enemy_formations < 1) {
                legion->layout = legion->prev_layout;
                continue;
            }
            for (int32_t i = 0; i < FORMATION_MAX_FIGURES; i++) {
                int32_t figure_id = legion->figures[i];
                if (figure_id > 0) {
                    figure *f = figure_get(figure_id);
                    if (f->action_state != FIGURE_ACTION_ATTACK && f->action_state != FIGURE_ACTION_CORPSE) {
                        f->action_state = FIGURE_ACTION_SOLDIER_MOPPING_UP;
                    }
                }
            }
        } else {
            for (int32_t i = 0; i < FORMATION_MAX_FIGURES; i++) {
                int32_t figure_id = legion->figures[i];
                figure *f = figure_get(figure_id);
                if (f->action_state != FIGURE_ACTION_ATTACK && f->action_state != FIGURE_ACTION_CORPSE && f->action_state != FIGURE_ACTION_SOLDIER_RETREATING) {
                    f->action_state = FIGURE_ACTION_SOLDIER_RETREATING;
                    figure_route_remove(figure_id);
                }
            }
        }
    }
}

static void formation_rout(int32_t id)
{
    formation *enemy = formation_get(id);
    for (int32_t i = 0; i < FORMATION_MAX_FIGURES; i++) {
        int32_t figure_id = enemy->figures[i];
        figure *f = figure_get(figure_id);
        if (f->action_state != FIGURE_ACTION_ATTACK && f->action_state != FIGURE_ACTION_CORPSE && f->action_state != FIGURE_ACTION_ENEMY_RETREATING) {
            f->action_state = FIGURE_ACTION_ENEMY_RETREATING;
            figure_route_remove(figure_id);
        }
    }
}

void formation_enemy_update(void)
{
    const city_player *player = city_player_current();
    int32_t target_found = ENEMY_TARGET_NONE;
    if (enemy_army_totals.enemy_formations < 1) {
        for (scenario_event *inv = scenario_invasion_next(&scenario_events, 0, 1); inv; inv = scenario_invasion_next(&scenario_events, inv, 1)) {
            inv->data.invasion.ignore_soldiers = 0;
        }
        formation_set_enemy_destination(0);
        return;
    }
    if (++enemy_army_totals.update_counter > 4) {
        enemy_army_totals.update_counter = 0;
        formation_update_soldier_strength();
    }
    for (scenario_event *inv = scenario_invasion_next(&scenario_events, 0, 1); inv; inv = scenario_invasion_next(&scenario_events, inv, 1)) {
        inv->data.invasion.first_formation_id = 0;
        inv->data.invasion.num_formations = 0;
    }
    for (int32_t id = 1; id < MAX_FORMATIONS; id++) {
        formation *enemy = formation_get(id);
        if (enemy->in_use != 1 || enemy->is_legion || enemy->herd_kind) {
            continue;
        }
        int32_t event_id = enemy->invasion_event_id;
        scenario_event *inv = scenario_invasion_event_at(&scenario_events, event_id);
        if ((enemy_army_totals.enemy_strength > (enemy_army_totals.legion_strength * 2)) && enemy->figure_type != FIGURE_ARCHER) {
            inv->data.invasion.ignore_soldiers = 1;
        }
        if (enemy->missile_fired) {
            enemy->missile_fired--;
        }
        if (enemy->missile_attack_timeout) {
            enemy->missile_attack_timeout--;
        }
        if (enemy->recent_fight) {
            enemy->recent_fight--;
        }
        if (player->soldiers < 1) {
            enemy->recent_fight = 0;
            enemy->missile_attack_timeout = 0;
            enemy->missile_fired = 0;
        }
        for (int32_t i = 0; i < FORMATION_MAX_FIGURES; i++) {
            figure *f = figure_get(enemy->figures[i]);
            if (f->action_state == FIGURE_ACTION_ATTACK) {
                figure *opponent = figure_get(f->opponent_id);
                if (opponent->state == FIGURE_STATE_ALIVE && opponent->action_state != FIGURE_ACTION_CORPSE && opponent->type >= FIGURE_ARCHER &&
                    opponent->type <= FIGURE_CHARIOTEER) {
                    enemy->recent_fight = FORMATION_RECENT_FIGHT_TICKS;
                }
            }
        }
        if (enemy->months_very_low_morale || enemy->months_low_morale) {
            formation_rout(id);
            continue;
        }
        int32_t first_id = enemy->figures[0];
        const figure *first = figure_get(first_id);
        if (first_id && first->state == FIGURE_STATE_ALIVE) {
            enemy->x = first->x;
            enemy->y = first->y;
        }
        inv = scenario_invasion_event_at(&scenario_events, event_id);
        if (inv->data.invasion.first_formation_id == 0) {
            inv->data.invasion.first_formation_id = (int16_t) id;
            inv->data.invasion.home_x = enemy->x;
            inv->data.invasion.home_y = enemy->y;
            inv->data.invasion.layout = enemy->layout;
            target_found = ENEMY_TARGET_NONE;
            map_routing_noncitizen_can_travel_over_land(enemy->x, enemy->y, -2, -2, 0, ENEMY_ROUTE_ANY_DISTANCE);
            if (map_soldier_strength_get_max(enemy->x, enemy->y, ENEMY_SOLDIERS_NEAR_RADIUS)) {
                target_found = ENEMY_TARGET_SOLDIERS_NEAR;
            } else if (map_soldier_strength_get_max(enemy->x, enemy->y, ENEMY_SOLDIERS_FAR_RADIUS)) {
                target_found = ENEMY_TARGET_SOLDIERS_FAR;
            }
            if (inv->data.invasion.ignore_soldiers == 0 && target_found == ENEMY_TARGET_SOLDIERS_NEAR) {
                inv->data.invasion.destination_x = (int16_t) map_result_x;
                inv->data.invasion.destination_y = (int16_t) map_result_y;
                inv->data.invasion.destination_building_id = 0;
            } else {
                if (inv->data.invasion.ignore_soldiers) {
                    target_found = ENEMY_TARGET_NONE;
                }
                if (formation_enemy_set_target_building(id)) {
                    enemy->ticks_without_target = 0;
                } else if (++enemy->ticks_without_target > ENEMY_FORMATION_MAX_TICKS_WITHOUT_TARGET) {
                    enemy->months_low_morale = 1;
                }
                inv->data.invasion.destination_x = enemy->destination_x;
                inv->data.invasion.destination_y = enemy->destination_y;
                inv->data.invasion.destination_building_id = enemy->destination_building_id;
            }
        }
        enemy->invasion_formation_index = inv->data.invasion.num_formations;
        inv->data.invasion.num_formations++;
        enemy->wait.ticks++;
        enemy->destination_x = inv->data.invasion.destination_x;
        enemy->destination_y = inv->data.invasion.destination_y;
        enemy->destination_building_id = inv->data.invasion.destination_building_id;

        int32_t hold_position = 0;
        int32_t move_to_spot = 0;
        int32_t follow_target = 0;
        int32_t target_formation = 0;
        int16_t *regroup = &enemy->regroup_ticks;
        int16_t *advance = &enemy->advance_ticks;
        int16_t *halt = &enemy->halt_ticks;
        if (enemy->missile_fired) {
            hold_position = 1;
        } else if (enemy->missile_attack_timeout) {
            follow_target = 1;
            target_formation = enemy->missile_attack_formation_id;
        } else if (enemy->wait.ticks < ENEMY_FORMATION_GATHER_TICKS) {
            move_to_spot = 1;
            *regroup = ENEMY_CYCLE_GATHER_REGROUP;
        } else if (inv->data.invasion.ignore_soldiers) {
            move_to_spot = 0;
        } else {
            int32_t next_halt, next_regroup, next_advance;
            int32_t layout = inv->data.invasion.layout;
            if (layout == FORMATION_ENEMY_MOB || layout == FORMATION_ENEMY12) {
                int32_t index = enemy->invasion_formation_index;
                next_advance = ENEMY_MOB_CYCLE_ADVANCE;
                if (index == 0 || index == 1) {
                    next_regroup = ENEMY_MOB_CYCLE_FRONT_REGROUP;
                    next_halt = ENEMY_MOB_CYCLE_FRONT_HALT;
                } else if (index == 2 || index == 3) {
                    next_regroup = ENEMY_MOB_CYCLE_MIDDLE_REGROUP;
                    next_halt = ENEMY_MOB_CYCLE_MIDDLE_HALT;
                } else {
                    next_regroup = ENEMY_MOB_CYCLE_BACK_REGROUP;
                    next_halt = ENEMY_MOB_CYCLE_BACK_HALT;
                }
                if (target_found == ENEMY_TARGET_NONE) {
                    next_regroup += ENEMY_MOB_CYCLE_NO_TARGET_EXTRA_REGROUP;
                    next_halt--;
                    next_advance = ENEMY_CYCLE_NO_TARGET_ADVANCE;
                }
            } else if (target_found == ENEMY_TARGET_NONE) {
                next_advance = ENEMY_CYCLE_NO_TARGET_ADVANCE;
                next_regroup = ENEMY_CYCLE_NO_TARGET_REGROUP;
                next_halt = ENEMY_CYCLE_NO_TARGET_HALT;
            } else {
                next_advance = ENEMY_CYCLE_ADVANCE;
                next_regroup = ENEMY_CYCLE_REGROUP;
                next_halt = ENEMY_CYCLE_HALT;
            }
            if (*halt) {
                hold_position = 1;
                *regroup = 0;
                *advance = 0;
                if (--*halt <= 0) {
                    *advance = (int16_t) next_advance;
                    formation_enemy_set_figures_to_initial(id);
                }
            } else if (*advance) {
                *regroup = 0;
                *halt = 0;
                move_to_spot = 1;
                if (--*advance <= 0) {
                    *regroup = (int16_t) next_regroup;
                    formation_enemy_set_figures_to_initial(id);
                    move_to_spot = 0;
                }
            } else {
                *advance = 0;
                *halt = 0;
                if (--*regroup <= 0) {
                    *halt = (int16_t) next_halt;
                    formation_enemy_set_figures_to_initial(id);
                    hold_position = 1;
                }
            }
        }
        if (enemy->wait.ticks > ENEMY_FORMATION_FLEE_CHECK_TICKS) {
            formation_enemy_set_fleeing();
        }
        if (hold_position) {
            enemy->destination_x = enemy->x;
            enemy->destination_y = enemy->y;
        } else if (follow_target) {
            if (target_formation <= 0) {
                enemy->destination_x = inv->data.invasion.destination_x;
                enemy->destination_y = inv->data.invasion.destination_y;
            } else {
                const formation *target = formation_get(target_formation);
                if (target->num_figures) {
                    enemy->destination_x = target->x;
                    enemy->destination_y = target->y;
                }
            }
        } else if (move_to_spot) {
            const formation_layout_spot *spots = enemy_formation_spots[inv->data.invasion.layout][enemy->orientation >> 1];
            const formation_layout_spot *spot = &spots[enemy->invasion_formation_index];
            if (formation_enemy_move_formation_to(id, spot->x + inv->data.invasion.home_x, spot->y + inv->data.invasion.home_y)) {
                enemy->destination_x = (int16_t) formation_move_result_x;
                enemy->destination_y = (int16_t) formation_move_result_y;
            }
        }
    }
    formation_set_enemy_destination(0);
}

void formation_set_enemy_destination(int32_t formation_id)
{
    formation *m = formation_get(formation_id);
    map_routing_noncitizen_can_travel_through_everything(m->x, m->y, -2, -2);
    int32_t best_id = 0;
    int32_t best_distance = 10000;
    const city_player *player = city_player_current();
    for (int32_t id = 1; id < MAX_BUILDINGS; id++) {
        const building *b = building_get(id);
        if (b->state != BUILDING_STATE_IN_USE) {
            continue;
        }
        int32_t type = b->type;
        if (type == BUILDING_MISSION_POST || type == BUILDING_NATIVE_HUT || type == BUILDING_NATIVE_MEETING || type == BUILDING_NATIVE_CROPS ||
            type == BUILDING_STORAGE_YARD || type == BUILDING_STORAGE_YARD_UP || type == BUILDING_FORT) {
            continue;
        }
        if (routing_grids.distance[b->grid_offset] <= 0) {
            continue;
        }
        int32_t distance = calc_maximum_distance(player->main_native_meeting_x, player->main_native_meeting_y, b->x, b->y);
        if (distance < best_distance) {
            best_id = id;
            best_distance = distance;
        }
    }
    if (best_id > 0) {
        const building *target = building_get(best_id);
        m->destination_x = target->x;
        m->destination_y = target->y;
        m->destination_building_id = (int16_t) best_id;
    }
}

int32_t formation_enemy_move_formation_to(int32_t formation_id, int32_t x, int32_t y)
{
    formation *m = formation_get(formation_id);
    int32_t num = m->num_figures;
    const formation_layout_spot *layout = enemy_formation_layout_offsets[m->layout];
    int32_t base = layout[0].x + (layout[0].y * GRID_SIZE);
    formation_enemy_move_offsets[0] = 0;
    for (int32_t i = 1; i < num; i++) {
        formation_enemy_move_offsets[i] = (layout[i].y * GRID_SIZE) - base + layout[i].x;
    }
    map_routing_noncitizen_can_travel_over_land(x, y, -1, -1, 0, ENEMY_PLACEMENT_ROUTE_TILES);
    for (int32_t r = 0; r <= ENEMY_PLACEMENT_MAX_RADIUS; r++) {
        int32_t x_min = x - r, y_min = y - r, x_max = x + r, y_max = y + r;
        if (x_min < 0) {
            x_min = 0;
        }
        if (y_min < 0) {
            y_min = 0;
        }
        if (x_max >= map_info.grid_width) {
            x_max = map_info.grid_width - 1;
        }
        if (y_max >= map_info.grid_height) {
            y_max = map_info.grid_height - 1;
        }
        for (int32_t yy = y_min; yy <= y_max; yy++) {
            int32_t row = map_info.grid_start + (yy * GRID_SIZE);
            for (int32_t xx = x_min; xx <= x_max; xx++) {
                int32_t ok = 1;
                for (int32_t i = 0; i < num; i++) {
                    int32_t grid_offset = row + xx + formation_enemy_move_offsets[i];
                    uint32_t terrain = map_grids.terrain[grid_offset];
                    if ((terrain & TERRAIN_WATER) || (terrain & (TERRAIN_ELEVATION | TERRAIN_RUBBLE)) || routing_grids.distance[grid_offset] <= 0) {
                        ok = 0;
                        break;
                    }
                    int32_t figure_id = map_grids.figure_ids[grid_offset];
                    if (figure_id && figure_get(figure_id)->formation_id != formation_id) {
                        ok = 0;
                        break;
                    }
                }
                if (ok) {
                    formation_move_result_x = xx;
                    formation_move_result_y = yy;
                    return 1;
                }
            }
        }
    }
    return 0;
}

int32_t formation_find_closest_academy(int32_t formation_id)
{
    formation *legion = formation_get(formation_id);
    const building *fort = building_get(legion->building_id);
    int32_t best_id = 0;
    int32_t best_distance = 10000;
    for (int32_t id = 1; id < MAX_BUILDINGS; id++) {
        const building *b = building_get(id);
        int32_t is_academy = b->type == BUILDING_MILITARY_ACADEMY || b->type == BUILDING_MILITARY_ACADEMY_2 || b->type == BUILDING_MILITARY_ACADEMY_3;
        if (b->state != BUILDING_STATE_IN_USE || !is_academy) {
            continue;
        }
        if (building_model_table[b->type].laborers > b->num_workers) {
            continue;
        }
        int32_t distance = calc_maximum_distance(b->x, b->y, fort->x, fort->y);
        if (distance < best_distance) {
            best_distance = distance;
            best_id = id;
        }
    }
    return best_id;
}

int32_t building_barracks_create_soldier(int32_t building_id)
{
    building *barracks = building_get(building_id);
    int32_t no_stock_a = barracks->loads_stored <= 0;
    int32_t no_stock_b = barracks->second_input_stock <= 0;
    int32_t best_id = 0;
    int32_t best_type = LEGION_RECRUIT_NONE;
    int32_t best_distance = 10000;
    for (int32_t id = 1; id < MAX_FORMATIONS; id++) {
        formation *legion = formation_get(id);
        if (legion->in_use != 1 || !legion->is_legion || legion->request_event_id >= 0) {
            continue;
        }
        int32_t type = legion->legion_recruit_type;
        if (type <= LEGION_RECRUIT_NONE || legion->ship_figure_id != 0) {
            continue;
        }
        if ((type == LEGION_RECRUIT_CHARIOTEER && no_stock_b) || (type == LEGION_RECRUIT_INFANTRY && no_stock_a)) {
            continue;
        }
        const building *fort = building_get(legion->building_id);
        int32_t distance = calc_maximum_distance(barracks->x, barracks->y, fort->x, fort->y);
        if (type > best_type || (type == best_type && distance < best_distance)) {
            best_id = id;
            best_type = type;
            best_distance = distance;
        }
    }
    formation *recruiting = formation_get(best_id);
    int32_t academy_id = 0;
    if (best_id > 0) {
        int32_t type = recruiting->figure_type;
        int32_t figure_id = figure_create(barracks->player, type, map_result_x, map_result_y, 0);
        figure *f = figure_get(figure_id);
        f->formation_id = (int16_t) best_id;
        f->formation_at_rest = 1;
        if (type == FIGURE_CHARIOTEER) {
            if (barracks->second_input_stock > 0) {
                barracks->second_input_stock -= BARRACKS_STOCK_PER_RECRUIT;
            }
        } else if (type == FIGURE_INFANTRY) {
            if (barracks->loads_stored > 0) {
                barracks->loads_stored -= BARRACKS_STOCK_PER_RECRUIT;
            }
        }
        academy_id = formation_find_closest_academy(best_id);
        if (!academy_id) {
            f->action_state = FIGURE_ACTION_SOLDIER_RECRUIT_GOING_TO_FORT;
        } else {
            const building *academy = building_get(academy_id);
            if (!map_has_road_access(academy->x, academy->y, academy->size)) {
                f->action_state = FIGURE_ACTION_SOLDIER_GOING_TO_FORT;
            } else {
                f->terrain_usage = TERRAIN_USAGE_ANY_OR_FORT;
                f->action_state = FIGURE_ACTION_SOLDIER_GOING_TO_ACADEMY;
                f->destination_x = (int16_t) map_result_x;
                f->destination_y = (int16_t) map_result_y;
                f->destination_grid_offset = map_info.grid_start + f->destination_x + ((int16_t) map_result_y * GRID_SIZE);
            }
        }
    }
    formation_calculate_figures();
    int32_t training = city_military_training_bonus(academy_id);
    if (!best_id) {
        return 0;
    }
    int32_t experience = recruiting->experience;
    int32_t others = recruiting->num_figures - 1;
    if (experience < LEGION_ACADEMY_EXPERIENCE && training == LEGION_ACADEMY_EXPERIENCE) {
        recruiting->experience = (int16_t) calc_divide_round_up((others * experience) + LEGION_ACADEMY_EXPERIENCE, others + 1);
        return 1;
    }
    int32_t total = (((others * 2) + 1) * experience) + training;
    int32_t average = calc_divide_round_up(total, (others * 2) + 2);
    if (average < LEGION_MAX_EXPERIENCE) {
        recruiting->experience = (int16_t) average;
    } else {
        recruiting->experience = LEGION_MAX_EXPERIENCE;
    }
    return 1;
}

int32_t building_barracks_create_tower_sentry(int32_t building_id)
{
    if (building_barracks_tower_sentry_request <= 0) {
        return 0;
    }
    building *barracks = building_get(building_id);
    for (int32_t id = 1; id < building_highest_id; id++) {
        building *tower = building_get(id);
        int32_t is_working_tower = tower->state == BUILDING_STATE_IN_USE && tower->type == BUILDING_TOWER && tower->num_workers > 0;
        if (!is_working_tower || tower->road_network_id != barracks->road_network_id || tower->figure_id != 0) {
            continue;
        }
        int32_t figure_id = figure_create(barracks->player, FIGURE_TOWER_SENTRY, map_result_x, map_result_y, 0);
        figure *f = figure_get(figure_id);
        f->action_state = FIGURE_ACTION_TOWER_SENTRY_GOING_TO_TOWER;
        if (!map_has_road_access(tower->x, tower->y, tower->size)) {
            f->state = FIGURE_STATE_DEAD;
        }
        f->destination_x = (int16_t) map_result_x;
        f->destination_y = (int16_t) map_result_y;
        f->building_id = (int16_t) id;
        tower->figure_id = (int16_t) figure_id;
        return 1;
    }
    return 0;
}

void formation_legion_update_recruit_type(int32_t building_id)
{
    const building *fort = building_get(building_id);
    int32_t id = fort->formation_id;
    formation *legion = formation_get(id);
    legion->legion_recruit_type = LEGION_RECRUIT_NONE;
    if (!legion->is_at_fort || legion->cursed_by_seth != 0) {
        return;
    }
    int32_t num = legion->num_figures;
    int32_t max = legion->max_figures;
    if (num == max) {
        return;
    }
    if (num < max) {
        int32_t type = fort->subtype;
        if (type == FIGURE_CHARIOTEER) {
            legion->legion_recruit_type = LEGION_RECRUIT_CHARIOTEER;
        }
        if (type == FIGURE_ARCHER) {
            legion->legion_recruit_type = LEGION_RECRUIT_ARCHER;
        }
        if (type == FIGURE_INFANTRY) {
            legion->legion_recruit_type = LEGION_RECRUIT_INFANTRY;
        }
        return;
    }
    int32_t excess = num - max;
    for (int32_t i = FORMATION_MAX_FIGURES - 1; i >= 0; i--) {
        int32_t figure_id = legion->figures[i];
        if (figure_id > 0) {
            figure_get(figure_id)->action_state = FIGURE_ACTION_SOLDIER_RETURNING_TO_BARRACKS;
            if (--excess < 1) {
                break;
            }
        }
    }
    formation_calculate_figures();
}

void formation_update_all(int32_t skip_morale)
{
    formation_calculate_legion_totals();
    formation_calculate_figures();
    formation_update_directions();
    formation_legion_decrease_damage();
    if (!skip_morale) {
        formation_update_monthly_morale_deployed();
    }
    formation_legion_set_max_figures();
    formation_legion_update();
    formation_enemy_update();
    formation_herd_update();
}

void formation_change_all_legion_morale(int32_t amount)
{
    change_all_morale(amount, 1);
}

void formation_change_all_enemy_morale(int32_t amount)
{
    change_all_morale(amount, 0);
}

uint8_t *formation_memory_past_records(int32_t offset)
{
    int32_t *const counters[] = { &figure_roaming_enabled, &figure_search_best_distance, &figure_highest_id, &figure_unused_counter };
    int32_t counter_bytes = (int32_t) sizeof(counters) / (int32_t) sizeof(counters[0]) * (int32_t) sizeof(int32_t);
    if (offset < counter_bytes) {
        return (uint8_t *) counters[offset / (int32_t) sizeof(int32_t)] + (offset % (int32_t) sizeof(int32_t));
    }
    offset -= counter_bytes;
    if (offset < (int32_t) sizeof(city_figures)) {
        return (uint8_t *) &city_figures + offset;
    }
    return 0;
}

int32_t formation_create_legion(int32_t building_id)
{
    formation_calculate_legion_totals();
    uint8_t player = (uint8_t) city_current_player;
    uint8_t used[LEGION_IDS] = { 0 };
    for (int32_t slot = 0; slot < MISSION_LEGION_SLOTS; slot++) {
        uint8_t reserved_id = mission_reserved_legion_id(slot);
        if (reserved_id < LEGION_IDS) {
            used[reserved_id] = 1;
        }
    }
    for (int32_t id = 0; id < MAX_FORMATIONS; id++) {
        formation *m = formation_get(id);
        if (m->in_use != 0 && m->is_legion == 1) {
            used[m->legion_id] = 1;
        }
    }
    int32_t index;
    for (index = 0; index < LEGION_IDS; index++) {
        if (!used[index]) {
            break;
        }
    }
    if (index >= LEGION_IDS) {
        index = 0;
    }
    int32_t id;
    for (id = 1; id < MAX_FORMATIONS; id++) {
        if (formation_get(id)->in_use == 0) {
            break;
        }
    }
    if (id >= MAX_FORMATIONS) {
        return 0;
    }
    const building *b = building_get(building_id);
    formation *legion = formation_get(id);
    legion->in_use = 1;
    legion->is_legion = 1;
    legion->faction = player;
    legion->figure_type = b->subtype;
    legion->building_id = (int16_t) building_id;
    legion->layout = FORMATION_DOUBLE_LINE_1;
    legion->morale = (int16_t) formation_morale_rules[FORMATION_MORALE_RULE_NEW_LEGION];
    legion->is_at_fort = 1;
    legion->request_event_id = LEGION_NO_REQUEST;
    legion->alternate_layout = 0;
    legion->legion_id = (uint8_t) index;
    int16_t x = (int16_t) (b->x + LEGION_STANDARD_FORT_DX);
    int16_t y = (int16_t) (b->y + LEGION_STANDARD_FORT_DY);
    legion->x = x;
    legion->y = y;
    legion->order_x = x;
    legion->order_y = y;
    legion->home_x = x;
    legion->home_y = y;
    legion->has_move_order = 0;
    legion->move_order_ticks = 0;
    int32_t standard_bearer_id = figure_create(b->player, FIGURE_STANDARD_BEARER, 0, 0, 0);
    legion->standard_figure_id = (int16_t) standard_bearer_id;
    figure *standard_bearer = figure_get(standard_bearer_id);
    standard_bearer->building_id = (int16_t) building_id;
    standard_bearer->formation_id = (int16_t) id;
    formation_num_legions++;
    if (id > formation_id_last_in_use) {
        formation_id_last_in_use = id;
    }
    return id;
}
