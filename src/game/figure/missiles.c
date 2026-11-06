// Missiles: arrows, bolts and spears flying cross country and hitting a figure or a ship on their tile, with the
// defense rules of the legions' layouts and the morale of the hit formation, and the javelin target of a soldier.
// The game itself creates only spears (the hunter's, the enemies' and the ships').
//
// A spear takes the enemy image only when its own type is the enemy archer, so every spear draws the player's.

#include "figure/missiles.h"
#include "building/building_data.h"
#include "core/calc.h"
#include "figure/figure.h"
#include "figure/figure_action.h"
#include "figure/figure_core.h"
#include "figure/figure_data.h"
#include "figure/figure_type.h"
#include "figure/formation.h"
#include "figure/image_groups_animals.h"
#include "figure/image_groups_enemies.h"
#include "figure/image_groups_military.h"
#include "figure/military.h"
#include "graphics/image_collection.h"
#include <stdint.h>

int32_t missile_hit_target(int32_t figure_id)
{
    int32_t id = map_grids.figure_ids[figure_get(figure_id)->grid_offset];
    while (id > 0) {
        const figure *f = figure_get(id);
        uint8_t type = f->type;
        if (f->action_state != FIGURE_ACTION_CORPSE && type != FIGURE_NONE && type != FIGURE_EXPLOSION && type != FIGURE_STANDARD_BEARER &&
            type != FIGURE_MAP_FLAG && type != FIGURE_FLOTSAM && (type < FIGURE_NATIVE || type >= FIGURE_TRANSPORT_SHIP) && type <= FIGURE_WARSHIP) {
            return id;
        }
        id = f->next_figure_id_on_same_tile;
    }
    return 0;
}

static int32_t missile_defense(const figure *target, int32_t formation_id, int32_t defense)
{
    if (defense >= MISSILE_MAX_DEFENSE) {
        defense = MISSILE_MAX_DEFENSE;
    }
    const formation *legion = formation_get(formation_id);
    if (legion->is_halted != 0) {
        int32_t type = legion->figure_type;
        if (type >= FIGURE_ARCHER && type <= FIGURE_INFANTRY) {
            if (!attack_is_same_direction(target->attack_direction, legion->direction)) {
                defense -= military_rules[MILITARY_RULE_MISSILE_FACING_AWAY_PENALTY];
            } else {
                int32_t layout = legion->layout;
                if (layout >= FORMATION_SINGLE_LINE_1 && layout <= FORMATION_SINGLE_LINE_2) {
                    defense += military_rules[MILITARY_RULE_MISSILE_SINGLE_LINE_BONUS];
                } else if (layout == FORMATION_MOP_UP) {
                    defense -= military_rules[MILITARY_RULE_MISSILE_MOP_UP_PENALTY];
                }
            }
        } else if (type == FIGURE_CHARIOTEER) {
            if (!attack_is_same_direction(target->attack_direction, legion->direction)) {
                defense -= military_rules[MILITARY_RULE_MISSILE_FACING_AWAY_PENALTY];
            } else if (legion->layout == FORMATION_MOP_UP) {
                defense -= military_rules[MILITARY_RULE_MISSILE_MOP_UP_PENALTY];
            }
        }
    }
    if (defense < 0) {
        defense = 0;
    }
    if (defense > MISSILE_MAX_DEFENSE) {
        defense = MISSILE_MAX_DEFENSE;
    }
    return defense;
}

static int32_t missile_damage(int32_t attack, int32_t defense)
{
    if (attack == 0) {
        return 0;
    }
    int32_t damage = ((MISSILE_DAMAGE_BASE - defense) / MISSILE_MAX_DEFENSE) * attack;
    return (damage < 0) ? 0 : damage;
}

static void missile_damage_figure(int32_t target_id, figure *target, int32_t formation_id, int32_t max_damage, int32_t damage)
{
    int32_t total = target->damage + damage;
    if (total > max_damage) {
        target->damage = (uint8_t) (max_damage + 1);
        target->action_state = FIGURE_ACTION_CORPSE;
        target->wait_ticks = 0;
        figure_play_die_sound(target_id);
        formation_update_morale_after_death(formation_id);
    } else {
        target->damage = (uint8_t) total;
    }
}

static int32_t experience_bonus(int32_t formation_id)
{
    return (formation_get(formation_id)->experience + 10) / 20;
}

static void missile_set_image(int32_t offset)
{
    figure *f = figure_get(figure_current_id);
    figure_image_direction = f->direction - map_info.orientation * 2;
    if (figure_image_direction < 0) {
        figure_image_direction += MISSILE_IMAGE_DIRECTIONS;
    }
    f->image_id = image_group(IMAGE_GROUP_FIGURE_MISSILES) + offset + figure_image_direction;
}

static int32_t missile_fly(void)
{
    figure *f = figure_get(figure_current_id);
    f->use_cross_country = 1;
    if (++f->progress_on_tile > MISSILE_FLIGHT_TICKS) {
        f->state = FIGURE_STATE_DEAD;
    }
    return figure_movement_move_ticks_cross_country(figure_current_id, MISSILE_SPEED);
}

void figure_arrow_action(void)
{
    int32_t ended = missile_fly();
    int32_t target_id = missile_hit_target(figure_current_id);
    figure *arrow = figure_get(figure_current_id);
    if (target_id == 0) {
        if (ended) {
            arrow->state = FIGURE_STATE_DEAD;
        }
        missile_set_image(MISSILE_IMAGE_ARROW);
        return;
    }
    figure *target = figure_get(target_id);
    int32_t formation_id = target->formation_id;
    int32_t bonus = (target->faction_id == city_current_player) ? experience_bonus(formation_id) : 0;
    int32_t max_damage = figure_properties[target->type].max_damage;
    int32_t defense = missile_defense(target, formation_id, figure_properties[target->type].missile_defense + bonus);
    int32_t attack = figure_properties[arrow->type].missile_attack;
    missile_damage_figure(target_id, target, formation_id, max_damage, missile_damage(attack, defense));
    arrow->state = FIGURE_STATE_DEAD;
    if (formation_id != 0) {
        formation *hit = formation_get(formation_id);
        hit->missile_attack_timeout = (int16_t) military_rules[MILITARY_RULE_MISSILE_ATTACK_TIMEOUT];
        hit->missile_attack_formation_id = figure_get(arrow->building_id)->formation_id;
    }
    missile_set_image(MISSILE_IMAGE_ARROW);
}

void figure_bolt_action(void)
{
    int32_t ended = missile_fly();
    int32_t target_id = map_figure_find_target(figure_current_id);
    figure *bolt = figure_get(figure_current_id);
    if (target_id == 0) {
        if (ended) {
            bolt->state = FIGURE_STATE_DEAD;
        }
        missile_set_image(MISSILE_IMAGE_BOLT);
        return;
    }
    figure *target = figure_get(target_id);
    int32_t formation_id = target->formation_id;
    int32_t bonus = (target->faction_id == city_current_player) ? experience_bonus(formation_id) : 0;
    int32_t max_damage = figure_properties[target->type].max_damage;
    int32_t defense = missile_defense(target, formation_id, figure_properties[target->type].missile_defense + bonus);
    int32_t attack = figure_properties[bolt->type].missile_attack;
    missile_damage_figure(target_id, target, formation_id, max_damage, missile_damage(attack, defense));
    bolt->state = FIGURE_STATE_DEAD;
    missile_set_image(MISSILE_IMAGE_BOLT);
}

static void spear_hit(figure *spear, int32_t target_id, int32_t shooter_id, int32_t shooter_formation)
{
    figure *target = figure_get(target_id);
    int32_t formation_id = target->formation_id;
    int32_t bonus = 0;
    int32_t max_damage;
    int32_t type = target->type;
    if (target->faction_id == city_current_player && type == FIGURE_TRANSPORT_SHIP) {
        max_damage = target->ship_max_hit_points;
    } else if (target->faction_id == city_current_player && type == FIGURE_WARSHIP) {
        max_damage = target->ship_max_hit_points;
        if (target->action_state == FIGURE_ACTION_WARSHIP_DOCKED) {
            bonus = military_rules[MILITARY_RULE_MISSILE_DOCKED_WARSHIP_BONUS];
        }
    } else {
        if (target->faction_id == city_current_player && type >= FIGURE_ARCHER && type <= FIGURE_CHARIOTEER) {
            bonus = experience_bonus(formation_id);
        }
        max_damage = figure_properties[type].max_damage;
    }
    int32_t defense = bonus + figure_properties[type].missile_defense;
    if (defense >= MISSILE_MAX_DEFENSE) {
        defense = MISSILE_MAX_DEFENSE;
    }
    int32_t is_ship = (type == FIGURE_WARSHIP) || (type == FIGURE_TRANSPORT_SHIP) || (type == FIGURE_FISHING_BOAT);
    int32_t attack_type;
    if (is_ship) {
        const figure *shooter = figure_get(shooter_id);
        if (shooter->state == FIGURE_STATE_DEAD) {
            spear->state = FIGURE_STATE_DEAD;
            return;
        }
        attack_type = shooter->type;
    } else {
        attack_type = spear->type;
    }
    int32_t attack = figure_properties[attack_type].missile_attack;
    defense = missile_defense(target, formation_id, defense);
    int32_t damage = missile_damage(attack, defense);
    if (is_ship) {
        int32_t strength = target->ship_hit_points - damage;
        if (strength <= 0) {
            target->ship_hit_points = 0;
            target->action_state = FIGURE_ACTION_CORPSE;
            target->wait_ticks = 0;
            figure_play_die_sound(target_id);
        } else {
            target->ship_hit_points = strength;
        }
    } else {
        missile_damage_figure(target_id, target, formation_id, max_damage, damage);
        formation *hit = formation_get(formation_id);
        hit->missile_attack_timeout = (int16_t) military_rules[MILITARY_RULE_MISSILE_ATTACK_TIMEOUT];
        hit->missile_attack_formation_id = (int16_t) shooter_formation;
    }
    spear->state = FIGURE_STATE_DEAD;
}

static void spear_set_image(figure *spear, int32_t shooter_formation)
{
    figure_image_direction = spear->direction - map_info.orientation * 4;
    if (figure_image_direction < 0) {
        figure_image_direction += SPEAR_IMAGE_DIRECTIONS;
    }
    if (spear->type == FIGURE_ENEMY_ARCHER && formation_get(shooter_formation)->enemy_type == 4) {
        spear->image_id = image_group(IMAGE_GROUP_FIGURE_ENEMY_SPEAR) + figure_image_direction;
        spear->cart_image_id = image_group(IMAGE_GROUP_FIGURE_ENEMY_SPEAR_SHADOW) + figure_image_direction;
        return;
    }
    spear->image_id = image_group(IMAGE_GROUP_FIGURE_HUNTER_SPEAR) + figure_image_direction;
    spear->cart_image_id = image_group(IMAGE_GROUP_FIGURE_HUNTER_SPEAR_SHADOW) + figure_image_direction;
}

void figure_spear_action(void)
{
    int32_t ended = missile_fly();
    int32_t target_id = missile_hit_target(figure_current_id);
    figure *spear = figure_get(figure_current_id);
    int32_t shooter_id = spear->building_id;
    int32_t shooter_formation = figure_get(shooter_id)->formation_id;
    if (target_id != 0) {
        spear_hit(spear, target_id, shooter_id, shooter_formation);
    } else if (ended) {
        spear->state = FIGURE_STATE_DEAD;
    }
    spear_set_image(spear, shooter_formation);
}

int32_t soldier_find_enemy(int32_t x, int32_t y, int32_t max_distance, int32_t figure_id)
{
    int32_t result = 0;
    int32_t attack_formation = formation_get(figure_get(figure_id)->formation_id)->attack_formation_id;
    figure_search_best_distance = 10000;
    for (int32_t id = 1; id < MAX_FIGURES; id++) {
        const figure *f = figure_get(id);
        if (f->state != FIGURE_STATE_ALIVE || f->action_state == FIGURE_ACTION_CORPSE) {
            continue;
        }
        uint8_t type = f->type;
        if (!((type >= FIGURE_ENEMY_ARCHER && type <= FIGURE_KINGDOM_SOLDIER) || type == FIGURE_BEDOUIN || figure_is_predator_animal(type))) {
            if (type != FIGURE_NATIVE || f->action_state != FIGURE_ACTION_NATIVE_ATTACKING) {
                continue;
            }
        }
        int32_t distance = calc_maximum_distance(x, y, f->x, f->y);
        if (attack_formation > 0 && f->formation_id != attack_formation) {
            continue;
        }
        if (distance > max_distance) {
            continue;
        }
        if (f->targeted_by_figure_id != 0) {
            distance *= 2;
        }
        if (distance < figure_search_best_distance) {
            figure_search_best_distance = distance;
            result = id;
        }
    }
    return result;
}
