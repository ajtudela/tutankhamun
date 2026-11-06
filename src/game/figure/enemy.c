// Enemies of an invasion: the invading nation's figure properties and graphics, the common enemy action (taking the
// spot in the formation's layout, marching, shooting, fighting the nearest soldier, retreating), the image choice of
// each enemy figure type, the kingdom soldier, and the check whether an enemy is near a building site. The chariot
// carts' offsets and the foot types' speeds and sounds come from config files.
//
// Two leftovers in the invaders' data:
// - a nation past the enemy models (the Egyptians, the bedouins) uses the Persian models;
// - the march sounds are never played, while their countdowns still run (reset to -56, 200 cut to a byte).

#include "figure/enemy.h"
#include "building/building_data.h"
#include "core/calc.h"
#include "figure/figure.h"
#include "figure/figure_action.h"
#include "figure/figure_core.h"
#include "figure/figure_data.h"
#include "figure/figure_image_group.h"
#include "figure/figure_type.h"
#include "figure/formation.h"
#include "figure/military.h"
#include "figure/military_internal.h"
#include "figure/movement.h"
#include "figure/route.h"
#include "graphics/image_collection.h"
#include "map/grid.h"
#include "map/routing.h"
#include "map/terrain.h"
#include <stdint.h>

int32_t enemy_find_soldier(int32_t x, int32_t y)
{
    int32_t best_id = 0;
    figure_search_best_distance = 10000;
    for (int32_t id = 1; id < MAX_FIGURES; id++) {
        figure *s = figure_get(id);
        if (s->state != FIGURE_STATE_ALIVE || s->action_state == FIGURE_ACTION_CORPSE || s->targeted_by_figure_id != 0 || s->type <= FIGURE_FIREMAN ||
            s->type >= FIGURE_STANDARD_BEARER) {
            continue;
        }
        int32_t distance = calc_maximum_distance(x, y, s->x, s->y);
        if (distance < figure_search_best_distance) {
            figure_search_best_distance = distance;
            best_id = id;
        }
    }
    if (best_id) {
        return best_id;
    }
    for (int32_t id = 1; id < MAX_FIGURES; id++) {
        figure *s = figure_get(id);
        if (s->state == FIGURE_STATE_ALIVE && s->action_state != FIGURE_ACTION_CORPSE && s->type > FIGURE_FIREMAN && s->type < FIGURE_STANDARD_BEARER) {
            return id;
        }
    }
    return 0;
}

int32_t figure_combat_get_missile_target_for_enemy(int32_t figure_id, int32_t max_distance)
{
    figure *f = figure_get(figure_id);
    int32_t target_level = city_player_current()->soldiers;
    int32_t y = f->y;
    int32_t x = f->x;
    int32_t best_id = 0;
    uint32_t best_priority = 0;
    figure_search_best_distance = max_distance;
    for (int32_t id = 1; id < MAX_FIGURES; id++) {
        figure *t = figure_get(id);
        if (t->state != FIGURE_STATE_ALIVE || t->action_state == FIGURE_ACTION_CORPSE) {
            continue;
        }
        uint32_t priority;
        int32_t distance;
        switch ((int8_t) t->type) {
            case FIGURE_NONE:
            case FIGURE_EXPLOSION:
            case FIGURE_STANDARD_BEARER:
            case FIGURE_MAP_FLAG:
            case FIGURE_FLOTSAM:
            case FIGURE_NATIVE:
            case FIGURE_ENEMY_ARCHER:
            case FIGURE_ENEMY_INFANTRY:
            case FIGURE_ENEMY_CHARIOT:
            case FIGURE_ENEMY_EGYPTIAN_INFANTRY:
            case FIGURE_ENEMY_EGYPTIAN_ARCHER:
            case FIGURE_ENEMY_EGYPTIAN_CHARIOT:
            case FIGURE_NATIVE_TRADER:
            case FIGURE_ARROW:
            case FIGURE_JAVELIN:
            case FIGURE_BOLT:
            case FIGURE_CREATURE:
            case FIGURE_SEAGULLS:
            case FIGURE_SHIPWRECK:
            case FIGURE_BIRDS:
            case FIGURE_ANTELOPE:
            case FIGURE_SPEAR:
            case FIGURE_CROCODILE:
            case FIGURE_HYENA:
            case FIGURE_HIPPO:
            case FIGURE_SLED:
            case FIGURE_ENEMY_TRANSPORT_SHIP:
            case FIGURE_ENEMY_WARSHIP:
            case FIGURE_FISHING_SPOT:
            case FIGURE_SHOWMAN:
            case FIGURE_BEDOUIN:
            case FIGURE_ENEMY_EGYPTIAN_WARSHIP:
            case FIGURE_ENEMY_EGYPTIAN_TRANSPORT_SHIP:
            case FIGURE_ASP:
            case FIGURE_LION:
            case FIGURE_SCORPION:
            case FIGURE_MUMMY:
                continue;
            case FIGURE_ARCHER:
            case FIGURE_INFANTRY:
            case FIGURE_CHARIOTEER:
            case FIGURE_TOWER_SENTRY:
            case FIGURE_BALLISTA:
                distance = calc_maximum_distance(x, y, t->x, t->y);
                priority = ENEMY_TARGET_SOLDIER;
                break;
            case FIGURE_TRANSPORT_SHIP:
                priority = t->has_troops_on_board ? ENEMY_TARGET_TRANSPORT_WITH_TROOPS : ENEMY_TARGET_TRANSPORT_SHIP;
                distance = calc_maximum_distance(x, y, t->x, t->y);
                break;
            case FIGURE_WARSHIP:
                distance = calc_maximum_distance(x, y, t->x, t->y);
                priority = ENEMY_TARGET_WARSHIP;
                break;
            default:
                if (target_level >= ENEMY_OTHER_TARGETS_MAX_SOLDIERS) {
                    continue;
                }
                distance = calc_maximum_distance(x, y, t->x, t->y) + ENEMY_OTHER_TARGET_DISTANCE;
                priority = ENEMY_TARGET_OTHER;
                break;
        }
        if (best_priority < priority && distance < figure_search_best_distance && figure_movement_can_launch_cross_country_missile(x, y, t->x, t->y)) {
            best_id = id;
            figure_search_best_distance = distance;
            best_priority = priority;
        }
    }
    if (!best_id) {
        return 0;
    }
    const figure *best = figure_get(best_id);
    map_result_y = best->y;
    map_result_x = best->x;
    return best_id;
}

int32_t figure_enemy_near_building_site(int32_t x1, int32_t y1, int32_t x2, int32_t y2)
{
    for (int32_t id = 1; id < MAX_FIGURES; id++) {
        const figure *f = figure_get(id);
        if (f->state != FIGURE_STATE_ALIVE || !((f->type > FIGURE_TOWER_SENTRY && f->type < FIGURE_NATIVE_TRADER) || f->type == FIGURE_BEDOUIN)) {
            continue;
        }
        int32_t fx = f->x;
        int32_t fy = f->y;
        int32_t dx = fx > x1 ? fx - x1 : x1 - fx;
        int32_t dy = fy > y1 ? fy - y1 : y1 - fy;
        if (dx < ENEMY_BUILDING_SITE_DISTANCE && dy < ENEMY_BUILDING_SITE_DISTANCE) {
            return 1;
        }
        dx = fx > x2 ? fx - x2 : x2 - fx;
        dy = fy > y2 ? fy - y2 : y2 - fy;
        if (dx < ENEMY_BUILDING_SITE_DISTANCE && dy < ENEMY_BUILDING_SITE_DISTANCE) {
            return 1;
        }
    }
    return 0;
}

void figure_enemy_properties_init(void)
{
    int32_t enemy = scenario_data.enemy_type;
    if (enemy >= ENEMY_MODEL_NATIONS) {
        enemy = ENEMY_PERSIAN;
        scenario_data.enemy_type = ENEMY_PERSIAN;
    }
    image_collection_load_file(enemy_graphics_names[enemy], 2);
    const figure_type_properties *model = enemy_model[enemy];
    figure_properties[FIGURE_ENEMY_INFANTRY] = model[ENEMY_MODEL_INFANTRY];
    figure_properties[FIGURE_ENEMY_ARCHER] = model[ENEMY_MODEL_ARCHER];
    figure_properties[FIGURE_ENEMY_CHARIOT] = model[ENEMY_MODEL_CHARIOT];
    figure_properties[FIGURE_ENEMY_WARSHIP] = model[ENEMY_MODEL_WARSHIP];
    figure_properties[FIGURE_ENEMY_TRANSPORT_SHIP] = model[ENEMY_MODEL_TRANSPORT_SHIP];
}

void enemy_action(void)
{
    int32_t figure_id = figure_current_id;
    figure *f = figure_get(figure_id);
    formation *m = formation_get(f->formation_id);
    city_player *player = city_player_current();
    f->terrain_usage = TERRAIN_USAGE_ENEMY;
    int32_t speed = figure_properties[f->type].speed;
    player->enemies++;
    const formation_layout_spot *spot = &enemy_formation_layout_offsets[m->layout][f->index_in_formation];
    f->formation_position_x = (int16_t) spot->x;
    f->formation_position_y = (int16_t) spot->y;

    switch (f->action_state) {
        case FIGURE_ACTION_ATTACK:
            figure_combat_handle_attack(figure_id);
            return;
        case FIGURE_ACTION_CORPSE:
            figure_combat_handle_corpse(figure_id);
            return;
        case FIGURE_ACTION_ENEMY_HOLDING:
            f->wait_ticks = 0;
            return;
        case FIGURE_ACTION_ENEMY_INITIAL: {
            map_figure_update(figure_id);
            f->animation_frame = 0;
            figure_route_remove(figure_id);
            if (--f->wait_ticks < 1) {
                f->wait_ticks = ENEMY_ORDER_WAIT_TICKS;
                f->is_ghost = 0;
                if (m->recent_fight == 0) {
                    f->destination_x = m->destination_x + f->formation_position_x;
                    f->destination_y = m->destination_y + f->formation_position_y;
                    uint8_t dir = (uint8_t) calc_general_direction(f->x, f->y, f->destination_x, f->destination_y);
                    if (dir < DIRECTION_COUNT) {
                        f->action_state = FIGURE_ACTION_ENEMY_MARCHING;
                    }
                } else {
                    f->action_state = FIGURE_ACTION_ENEMY_ATTACKING;
                }
            }
            int32_t type = f->type;
            if (type != FIGURE_ENEMY_ARCHER && type != FIGURE_ENEMY_CAMEL && type != FIGURE_ENEMY_BARBARIAN_ARCHER && type != FIGURE_ENEMY_BARBARIAN_MOUNTED &&
                type != FIGURE_ENEMY_EGYPTIAN_ARCHER) {
                return;
            }
            uint8_t counter = (uint8_t) (f->wait_ticks_missile + 1);
            f->wait_ticks_missile = counter;
            if (figure_properties[type].missile_delay < (int32_t) counter) {
                f->wait_ticks_missile = 0;
                if (figure_combat_get_missile_target_for_enemy(figure_id, figure_properties[type].missile_range) == 0) {
                    f->attack_image_offset = 0;
                } else {
                    f->attack_image_offset = 1;
                    f->direction = (int8_t) calc_missile_shooter_direction(f->x, f->y, map_result_x, map_result_y);
                }
            }
            if (f->attack_image_offset == 0) {
                return;
            }
            if (f->attack_image_offset == 1) {
                figure_create_missile(figure_id, f->x, f->y, map_result_x, map_result_y, FIGURE_SPEAR);
                m->missile_fired = ENEMY_MISSILE_FIRED_TICKS;
            }
            if (++f->attack_image_offset >= ENEMY_SHOOT_TICKS) {
                f->attack_image_offset = 0;
            }
            return;
        }
        case FIGURE_ACTION_ENEMY_RETREATING:
            f->destination_x = f->source_x;
            f->destination_y = f->source_y;
            figure_movement_move_ticks(figure_id, speed);
            if (f->direction == DIRECTION_REROUTE || f->direction == DIRECTION_LOST || f->direction == DIRECTION_AT_DESTINATION) {
                f->state = FIGURE_STATE_DEAD;
            }
            return;
        case FIGURE_ACTION_ENEMY_IDLE:
            map_figure_update(figure_id);
            return;
        case FIGURE_ACTION_ENEMY_MARCHING:
            if (--f->wait_ticks < 1) {
                f->wait_ticks = ENEMY_ORDER_WAIT_TICKS;
                int32_t spot_x = m->destination_x + f->formation_position_x;
                int32_t spot_y = m->destination_y + f->formation_position_y;
                if (f->destination_x != spot_x || f->destination_y != spot_y) {
                    f->destination_x = (int16_t) spot_x;
                    f->destination_y = (int16_t) spot_y;
                    int8_t dir = (int8_t) calc_general_direction(f->x, f->y, f->destination_x, f->destination_y);
                    if (dir == DIRECTION_AT_DESTINATION) {
                        f->action_state = FIGURE_ACTION_ENEMY_INITIAL;
                        return;
                    }
                    f->destination_building_id = m->destination_building_id;
                    figure_route_remove(figure_id);
                }
            }
            figure_movement_move_ticks(figure_id, speed);
            if (f->direction == DIRECTION_REROUTE || f->direction == DIRECTION_LOST || f->direction == DIRECTION_AT_DESTINATION) {
                f->action_state = FIGURE_ACTION_ENEMY_INITIAL;
            }
            return;
        case FIGURE_ACTION_ENEMY_ATTACKING:
            break;
        case FIGURE_ACTION_ENEMY_13:
            return;
        default:
            f->action_state = FIGURE_ACTION_ENEMY_INITIAL;
            return;
    }

    if (m->recent_fight == 0) {
        f->action_state = FIGURE_ACTION_ENEMY_INITIAL;
    }
    int32_t type = f->type;
    if (type != FIGURE_ENEMY_CAMEL && type != FIGURE_ENEMY_ELEPHANT) {
        if (type == FIGURE_ENEMY_BARBARIAN_MOUNTED || type == FIGURE_ENEMY_BARBARIAN_CHARIOT) {
            if (--player->sound_march_horse < 1) {
                player->sound_march_horse = ENEMY_MARCH_SOUND_RESET;
            }
        } else if (--player->sound_march_enemy < 1) {
            player->sound_march_enemy = ENEMY_MARCH_SOUND_RESET;
        }
    }
    int32_t target_id = f->target_figure_id;
    if (figure_get(target_id)->state != FIGURE_STATE_ALIVE) {
        f->target_figure_id = 0;
        target_id = 0;
    }
    int32_t target_dying = figure_get(target_id)->action_state == FIGURE_ACTION_CORPSE;
    if (target_dying) {
        f->target_figure_id = 0;
    }
    if (target_dying || target_id <= 0) {
        target_id = enemy_find_soldier(f->x, f->y);
        if (target_id == 0) {
            f->action_state = FIGURE_ACTION_ENEMY_INITIAL;
            f->wait_ticks = ENEMY_ORDER_WAIT_TICKS;
            return;
        }
        figure *target = figure_get(target_id);
        f->destination_x = target->x;
        f->destination_y = target->y;
        f->target_figure_id = (int16_t) target_id;
        target->targeted_by_figure_id = (int16_t) figure_id;
        f->target_figure_created_sequence = target->created_sequence;
        figure_route_remove(figure_id);
    }
    figure_movement_move_ticks(figure_id, speed);
    if (f->direction == DIRECTION_REROUTE || f->direction == DIRECTION_LOST) {
        f->action_state = FIGURE_ACTION_ENEMY_INITIAL;
        f->target_figure_id = 0;
        return;
    }
    if (f->direction == DIRECTION_AT_DESTINATION) {
        const figure *target = figure_get(f->target_figure_id);
        f->destination_x = target->x;
        f->destination_y = target->y;
        figure_route_remove(figure_id);
    }
}

static int32_t enemy_begin(int32_t drowns, int32_t speed_factor)
{
    figure *f = figure_get(figure_current_id);
    if (drowns && (map_grids.terrain[f->grid_offset] & (TERRAIN_WATER | TERRAIN_FLOODPLAIN)) == (TERRAIN_WATER | TERRAIN_FLOODPLAIN)) {
        figure_set_state_dead(figure_current_id);
        return 0;
    }
    uint8_t frame = (uint8_t) (f->animation_frame + 1);
    f->animation_frame = frame;
    if (frame > ENEMY_LAST_FRAME) {
        f->animation_frame = 0;
    }
    f->cart_image_id = 0;
    f->speed_multiplier = (uint8_t) speed_factor;
    return 1;
}

static int32_t enemy_image_direction(const figure *f, int32_t missiles, int32_t formation_id)
{
    int32_t dir;
    if (f->action_state == FIGURE_ACTION_ATTACK) {
        dir = f->attack_direction;
    } else if (missiles && formation_get(formation_id)->missile_fired != 0) {
        dir = f->direction;
    } else if (f->direction < DIRECTION_COUNT) {
        dir = f->direction;
    } else {
        dir = f->previous_tile_direction;
    }
    figure_image_direction = dir - map_info.orientation;
    if (figure_image_direction < 0) {
        figure_image_direction += DIRECTION_COUNT;
    }
    return figure_image_direction;
}

static int32_t enemy_attack_image(const figure *f, int32_t base, int32_t dir)
{
    if (f->attack_image_offset < ENEMY_FIGHT_WINDUP) {
        return base + dir;
    }
    return base + (((int32_t) (f->attack_image_offset - ENEMY_FIGHT_WINDUP) / 2) * 8) + dir;
}

static void enemy_images_foot(figure *f, int32_t attack, int32_t corpse, int32_t walk, int32_t missile_frames)
{
    int32_t dir = figure_image_direction;
    if (f->action_state == FIGURE_ACTION_ATTACK) {
        f->image_id = enemy_attack_image(f, attack, dir);
    } else if (missile_frames && f->action_state == FIGURE_ACTION_ENEMY_INITIAL) {
        f->image_id = (int32_t) (dir + figure_missile_frames[f->attack_image_offset >> 1] * 8u + attack);
    } else if (f->action_state == FIGURE_ACTION_CORPSE) {
        f->image_id = (int32_t) (figure_corpse_offsets[f->wait_ticks / 2] + corpse);
    } else if (f->direction == DIRECTION_ATTACK) {
        f->image_id = (int32_t) (dir + (f->animation_frame >> 1) * 8u + attack);
    } else {
        f->image_id = (int32_t) (dir + f->animation_frame * 8u + walk);
    }
}

enemy_cart_offset enemy_cart_offsets[DIRECTION_COUNT] = {
    [DIRECTION_TOP] = { .dx = 22, .dy = -18 },
    [DIRECTION_TOP_RIGHT] = { .dx = 27, .dy = -3 },
    [DIRECTION_RIGHT] = { .dx = 23, .dy = 17 },
    [DIRECTION_BOTTOM_RIGHT] = { .dx = 0, .dy = 11 },
    [DIRECTION_BOTTOM] = { .dx = -21, .dy = 17 },
    [DIRECTION_BOTTOM_LEFT] = { .dx = -25, .dy = -1 },
    [DIRECTION_LEFT] = { .dx = -17, .dy = -17 },
    [DIRECTION_TOP_LEFT] = { .dx = 0, .dy = -12 },
};

static void enemy_set_cart(figure *f, int32_t dir)
{
    f->cart_image_id = (int32_t) (image_group(IMAGE_GROUP_FIGURE_CHARIOT_CART) + f->animation_frame * 8u + dir);
    f->cart_offset_x = (int8_t) enemy_cart_offsets[dir].dx;
    f->cart_offset_y = (int8_t) enemy_cart_offsets[dir].dy;
}

static void enemy_images_rider(figure *f, int32_t base)
{
    int32_t dir = figure_image_direction;
    if (f->direction != DIRECTION_ATTACK) {
        if (f->action_state == FIGURE_ACTION_ATTACK) {
            f->image_id = dir + base;
            return;
        }
        if (f->action_state == FIGURE_ACTION_ENEMY_INITIAL) {
            f->image_id = (int32_t) (dir + ENEMY_IMAGE_RIDER_MISSILE + figure_missile_frames[f->attack_image_offset >> 1] * 8u + base);
            return;
        }
        if (f->action_state == FIGURE_ACTION_CORPSE) {
            f->image_id = (int32_t) (figure_corpse_offsets[f->wait_ticks / 2] + ENEMY_IMAGE_RIDER_CORPSE + base);
            return;
        }
    }
    f->image_id = (int32_t) (dir + f->animation_frame * 8u + base);
}

typedef struct {
    uint8_t follows_missiles;
    uint8_t pulls_cart;
    uint8_t sound_before_direction;
    int16_t attack_alt_group, attack_group, corpse_alt_group, corpse_group, walk_alt_group, walk_group;
} enemy_foot;

enemy_foot_rule enemy_foot_rules[ENEMY_FOOT_KINDS] = {
    [ENEMY_FOOT_ARCHER] = { .speed_factor = 1, .attack_sound = ENEMY_NO_ATTACK_SOUND },
    [ENEMY_FOOT_INFANTRY] = { .speed_factor = 1, .attack_sound = ENEMY_NO_ATTACK_SOUND },
    [ENEMY_FOOT_CHARIOT] = { .speed_factor = 1, .attack_sound = ENEMY_NO_ATTACK_SOUND },
    [ENEMY_FOOT_EGYPTIAN_INFANTRY] = { .speed_factor = 2, .attack_sound = 0x0C },
    [ENEMY_FOOT_EGYPTIAN_ARCHER] = { .speed_factor = 2, .attack_sound = 0x0B },
    [ENEMY_FOOT_EGYPTIAN_CHARIOT] = { .speed_factor = 2, .attack_sound = 0x0D },
    [ENEMY_FOOT_BEDOUIN] = { .speed_factor = 2, .attack_sound = 0x0A },
};

static const enemy_foot enemy_foot_types[ENEMY_FOOT_KINDS] = {
    [ENEMY_FOOT_ARCHER] = { .follows_missiles = 1,
        .pulls_cart = 0,
        .sound_before_direction = 0,
        .attack_alt_group = IMAGE_GROUP_FIGURE_ENEMY_ALT_ARCHER_ATTACK,
        .attack_group = IMAGE_GROUP_FIGURE_ENEMY_ARCHER_ATTACK,
        .corpse_alt_group = IMAGE_GROUP_FIGURE_ENEMY_ALT_ARCHER_DEATH,
        .corpse_group = IMAGE_GROUP_FIGURE_ENEMY_ARCHER_DEATH,
        .walk_alt_group = IMAGE_GROUP_FIGURE_ENEMY_ALT_ARCHER,
        .walk_group = IMAGE_GROUP_FIGURE_ENEMY_ARCHER },
    [ENEMY_FOOT_INFANTRY] = { .follows_missiles = 0,
        .pulls_cart = 0,
        .sound_before_direction = 0,
        .attack_alt_group = IMAGE_GROUP_FIGURE_ENEMY_ALT_INFANTRY_ATTACK,
        .attack_group = IMAGE_GROUP_FIGURE_ENEMY_INFANTRY_ATTACK,
        .corpse_alt_group = IMAGE_GROUP_FIGURE_ENEMY_ALT_INFANTRY_DEATH,
        .corpse_group = IMAGE_GROUP_FIGURE_ENEMY_INFANTRY_DEATH,
        .walk_alt_group = IMAGE_GROUP_FIGURE_ENEMY_ALT_INFANTRY,
        .walk_group = IMAGE_GROUP_FIGURE_ENEMY_INFANTRY },
    [ENEMY_FOOT_CHARIOT] = { .follows_missiles = 0,
        .pulls_cart = 1,
        .sound_before_direction = 0,
        .attack_alt_group = IMAGE_GROUP_FIGURE_ENEMY_ALT_CHARIOT_ATTACK,
        .attack_group = IMAGE_GROUP_FIGURE_ENEMY_CHARIOT_ATTACK,
        .corpse_alt_group = IMAGE_GROUP_FIGURE_ENEMY_ALT_CHARIOT_DEATH,
        .corpse_group = IMAGE_GROUP_FIGURE_ENEMY_CHARIOT_DEATH,
        .walk_alt_group = IMAGE_GROUP_FIGURE_ENEMY_ALT_CHARIOT,
        .walk_group = IMAGE_GROUP_FIGURE_ENEMY_CHARIOT },
    [ENEMY_FOOT_EGYPTIAN_INFANTRY] = { .follows_missiles = 0,
        .pulls_cart = 0,
        .sound_before_direction = 1,
        .attack_alt_group = IMAGE_GROUP_FIGURE_ENEMY_EGYPTIAN_INFANTRY_ATTACK,
        .attack_group = IMAGE_GROUP_FIGURE_ENEMY_EGYPTIAN_INFANTRY_ATTACK,
        .corpse_alt_group = IMAGE_GROUP_FIGURE_ENEMY_EGYPTIAN_INFANTRY_DEATH,
        .corpse_group = IMAGE_GROUP_FIGURE_ENEMY_EGYPTIAN_INFANTRY_DEATH,
        .walk_alt_group = IMAGE_GROUP_FIGURE_ENEMY_EGYPTIAN_INFANTRY,
        .walk_group = IMAGE_GROUP_FIGURE_ENEMY_EGYPTIAN_INFANTRY },
    [ENEMY_FOOT_EGYPTIAN_ARCHER] = { .follows_missiles = 0,
        .pulls_cart = 0,
        .sound_before_direction = 0,
        .attack_alt_group = IMAGE_GROUP_FIGURE_ENEMY_EGYPTIAN_ARCHER_ATTACK,
        .attack_group = IMAGE_GROUP_FIGURE_ENEMY_EGYPTIAN_ARCHER_ATTACK,
        .corpse_alt_group = IMAGE_GROUP_FIGURE_ENEMY_EGYPTIAN_ARCHER_DEATH,
        .corpse_group = IMAGE_GROUP_FIGURE_ENEMY_EGYPTIAN_ARCHER_DEATH,
        .walk_alt_group = IMAGE_GROUP_FIGURE_ENEMY_EGYPTIAN_ARCHER,
        .walk_group = IMAGE_GROUP_FIGURE_ENEMY_EGYPTIAN_ARCHER },
    [ENEMY_FOOT_EGYPTIAN_CHARIOT] = { .follows_missiles = 0,
        .pulls_cart = 1,
        .sound_before_direction = 0,
        .attack_alt_group = IMAGE_GROUP_FIGURE_ENEMY_EGYPTIAN_CHARIOT_ATTACK,
        .attack_group = IMAGE_GROUP_FIGURE_ENEMY_EGYPTIAN_CHARIOT_ATTACK,
        .corpse_alt_group = IMAGE_GROUP_FIGURE_ENEMY_EGYPTIAN_CHARIOT_DEATH,
        .corpse_group = IMAGE_GROUP_FIGURE_ENEMY_EGYPTIAN_CHARIOT_DEATH,
        .walk_alt_group = IMAGE_GROUP_FIGURE_ENEMY_EGYPTIAN_CHARIOT,
        .walk_group = IMAGE_GROUP_FIGURE_ENEMY_EGYPTIAN_CHARIOT },
    [ENEMY_FOOT_BEDOUIN] = { .follows_missiles = 0,
        .pulls_cart = 0,
        .sound_before_direction = 0,
        .attack_alt_group = IMAGE_GROUP_FIGURE_BEDOUIN_ATTACK,
        .attack_group = IMAGE_GROUP_FIGURE_BEDOUIN_ATTACK,
        .corpse_alt_group = IMAGE_GROUP_FIGURE_BEDOUIN_DEATH,
        .corpse_group = IMAGE_GROUP_FIGURE_BEDOUIN_DEATH,
        .walk_alt_group = IMAGE_GROUP_FIGURE_BEDOUIN,
        .walk_group = IMAGE_GROUP_FIGURE_BEDOUIN },
};

static int32_t enemy_image(int32_t alt_group, int32_t group)
{
    return scenario_data.soldier_images_alt ? image_group(alt_group) : image_group(group);
}

static void enemy_foot_action(enum enemy_foot_kind kind)
{
    const enemy_foot *t = &enemy_foot_types[kind];
    const enemy_foot_rule *rule = &enemy_foot_rules[kind];
    figure *f = figure_get(figure_current_id);
    int32_t formation_id = f->formation_id;
    if (!enemy_begin(1, rule->speed_factor)) {
        return;
    }
    if (kind == ENEMY_FOOT_BEDOUIN && formation_get(formation_id)->herd_kind) {
        f->state = FIGURE_STATE_DEAD;
    }
    enemy_action();
    int32_t plays_attack_sound = rule->attack_sound != ENEMY_NO_ATTACK_SOUND && f->action_state == FIGURE_ACTION_ATTACK;
    if (plays_attack_sound && t->sound_before_direction) {
        sound_figure_play(figure_current_id, rule->attack_sound, 2);
    }
    int32_t dir = enemy_image_direction(f, t->follows_missiles, formation_id);
    if (plays_attack_sound && !t->sound_before_direction) {
        sound_figure_play(figure_current_id, rule->attack_sound, 2);
        dir = figure_image_direction;
    }
    int32_t attack = enemy_image(t->attack_alt_group, t->attack_group);
    int32_t corpse = enemy_image(t->corpse_alt_group, t->corpse_group);
    enemy_images_foot(f, attack, corpse, enemy_image(t->walk_alt_group, t->walk_group), t->follows_missiles);
    if (t->pulls_cart) {
        enemy_set_cart(f, dir);
    }
}

static void enemy_rider_action(int32_t drowns, int32_t speed_factor)
{
    figure *f = figure_get(figure_current_id);
    int32_t formation_id = f->formation_id;
    if (!enemy_begin(drowns, speed_factor)) {
        return;
    }
    enemy_action();
    enemy_image_direction(f, 1, formation_id);
    enemy_images_rider(f, image_group(IMAGE_GROUP_FIGURE_ENEMY_RIDER));
}

void figure_enemy_elephant_action(void)
{
    enemy_begin(0, 1);
    enemy_action();
    figure *f = figure_get(figure_current_id);
    int32_t dir = enemy_image_direction(f, 0, 0);
    if (f->direction != DIRECTION_ATTACK && f->action_state == FIGURE_ACTION_CORPSE) {
        f->image_id = (int32_t) (figure_corpse_offsets[f->wait_ticks / 2] + ENEMY_IMAGE_ELEPHANT_CORPSE + image_group(IMAGE_GROUP_FIGURE_ENEMY_ELEPHANT));
    } else {
        f->image_id = (int32_t) (dir + f->animation_frame * 8u + image_group(IMAGE_GROUP_FIGURE_ENEMY_ELEPHANT));
    }
}

void figure_enemy_barbarian_chariot_action(void)
{
    enemy_begin(0, 3);
    enemy_action();
    figure *f = figure_get(figure_current_id);
    int32_t dir = enemy_image_direction(f, 0, 0);
    int32_t base = image_group(IMAGE_GROUP_FIGURE_ENEMY_CHARIOT);
    if (f->direction == DIRECTION_ATTACK || f->action_state == FIGURE_ACTION_ATTACK) {
        f->image_id = (int32_t) (dir + ENEMY_IMAGE_CHARIOT_FIGHT + (f->animation_frame >> 1) * 8u + base);
    } else if (f->action_state == FIGURE_ACTION_CORPSE) {
        f->image_id = (int32_t) (figure_corpse_offsets[f->wait_ticks / 2] + ENEMY_IMAGE_CHARIOT_CORPSE + base);
    } else {
        f->image_id = (int32_t) (dir + f->animation_frame * 8u + base);
    }
}

static void enemy_plain_action(int32_t drowns, int32_t speed_factor, int32_t missiles)
{
    figure *f = figure_get(figure_current_id);
    int32_t formation_id = f->formation_id;
    if (!enemy_begin(drowns, speed_factor)) {
        return;
    }
    enemy_action();
    enemy_image_direction(f, missiles, formation_id);
}

void figure_kingdom_soldier_action(void)
{
    figure *f = figure_get(figure_current_id);
    if ((map_grids.terrain[f->grid_offset] & (TERRAIN_WATER | TERRAIN_FLOODPLAIN)) == (TERRAIN_WATER | TERRAIN_FLOODPLAIN)) {
        figure_set_state_dead(figure_current_id);
        return;
    }
    city_player_current()->kingdom_soldiers++;
    enemy_begin(0, 1);
    enemy_action();
    int32_t base = image_group(IMAGE_GROUP_FIGURE_KINGDOM_SOLDIER);
    int32_t dir = enemy_image_direction(f, 0, 0);
    if (f->action_state == FIGURE_ACTION_ATTACK) {
        f->image_id = enemy_attack_image(f, base, dir);
    } else if (f->action_state == FIGURE_ACTION_ENEMY_ATTACKING) {
        f->image_id = image_group(IMAGE_GROUP_FIGURE_ENEMY_ALT_CHARIOT) + dir;
    } else if (f->action_state == FIGURE_ACTION_CORPSE) {
        f->image_id = (int32_t) (figure_corpse_offsets[f->wait_ticks / 2] + ENEMY_IMAGE_KINGDOM_SOLDIER_CORPSE + base);
    } else {
        f->image_id = (int32_t) (base + ENEMY_IMAGE_KINGDOM_SOLDIER_WALK + f->animation_frame * 8u + dir);
    }
}

void figure_enemy_archer_action(void)
{
    enemy_foot_action(ENEMY_FOOT_ARCHER);
}
void figure_enemy_infantry_action(void)
{
    enemy_foot_action(ENEMY_FOOT_INFANTRY);
}
void figure_enemy_chariot_action(void)
{
    enemy_foot_action(ENEMY_FOOT_CHARIOT);
}
void figure_enemy_camel_action(void)
{
    enemy_rider_action(0, 1);
}
void figure_enemy_barbarian_light_action(void)
{
    enemy_plain_action(0, 2, 0);
}
void figure_enemy_barbarian_heavy_action(void)
{
    enemy_plain_action(0, 1, 0);
}
void figure_enemy_barbarian_archer_action(void)
{
    enemy_plain_action(0, 2, 1);
}
void figure_enemy_barbarian_mounted_action(void)
{
    enemy_rider_action(1, 3);
}
void figure_enemy_barbarian_heavy_2_action(void)
{
    enemy_plain_action(1, 1, 0);
}
void figure_enemy_egyptian_infantry_action(void)
{
    enemy_foot_action(ENEMY_FOOT_EGYPTIAN_INFANTRY);
}
void figure_enemy_egyptian_archer_action(void)
{
    enemy_foot_action(ENEMY_FOOT_EGYPTIAN_ARCHER);
}
void figure_enemy_egyptian_chariot_action(void)
{
    enemy_foot_action(ENEMY_FOOT_EGYPTIAN_CHARIOT);
}
void figure_bedouin_action(void)
{
    enemy_foot_action(ENEMY_FOOT_BEDOUIN);
}
