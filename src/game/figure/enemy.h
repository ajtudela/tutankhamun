// The enemy actions of figure/enemy.c, the invader's properties and the enemy check of a building site. enum
// enemy_type is the invading nation, in the order of the game's nation names and enemy graphics files.

#ifndef FIGURE_ENEMY_H
#define FIGURE_ENEMY_H

#include "map/grid.h"
#include <stdint.h>

enum enemy_type {
    ENEMY_HYKSOS = 0,
    ENEMY_SEA_PEOPLE = 1,
    ENEMY_HITTITE = 2,
    ENEMY_MITANI = 3,
    ENEMY_KUSHITE = 4,
    ENEMY_LIBYAN = 5,
    ENEMY_NUBIAN = 6,
    ENEMY_CANAANITE = 7,
    ENEMY_ASSYRIAN = 8,
    ENEMY_ROMAN = 9,
    ENEMY_PHOENICIAN = 10,
    ENEMY_PERSIAN = 11,
    ENEMY_EGYPTIAN = 12,
    ENEMY_BEDOUIN = 13,
};

enum enemy_target_priority {
    ENEMY_TARGET_OTHER = 10,
    ENEMY_TARGET_TRANSPORT_SHIP = 20,
    ENEMY_TARGET_WARSHIP = 30,
    ENEMY_TARGET_TRANSPORT_WITH_TROOPS = 40,
    ENEMY_TARGET_SOLDIER = 50,
};

enum enemy_limit {
    ENEMY_LAST_FRAME = 11,
    ENEMY_FIGHT_WINDUP = 12,
    ENEMY_LAYOUT_SPOTS = 16,
    ENEMY_ORDER_WAIT_TICKS = 50,
    ENEMY_MISSILE_FIRED_TICKS = 6,
    ENEMY_SHOOT_TICKS = 101,
    ENEMY_MARCH_SOUND_RESET = -56,
    ENEMY_OTHER_TARGETS_MAX_SOLDIERS = 4,
    ENEMY_OTHER_TARGET_DISTANCE = 5,
    ENEMY_BUILDING_SITE_DISTANCE = 13,
    ENEMY_NO_ATTACK_SOUND = -1,
};

enum enemy_foot_kind {
    ENEMY_FOOT_ARCHER = 0,
    ENEMY_FOOT_INFANTRY = 1,
    ENEMY_FOOT_CHARIOT = 2,
    ENEMY_FOOT_EGYPTIAN_INFANTRY = 3,
    ENEMY_FOOT_EGYPTIAN_ARCHER = 4,
    ENEMY_FOOT_EGYPTIAN_CHARIOT = 5,
    ENEMY_FOOT_BEDOUIN = 6,
    ENEMY_FOOT_KINDS = 7,
};

enum enemy_image {
    ENEMY_IMAGE_KINGDOM_SOLDIER_WALK = 48,
    ENEMY_IMAGE_RIDER_MISSILE = 96,
    ENEMY_IMAGE_CHARIOT_FIGHT = 96,
    ENEMY_IMAGE_ELEPHANT_CORPSE = 104,
    ENEMY_IMAGE_RIDER_CORPSE = 144,
    ENEMY_IMAGE_CHARIOT_CORPSE = 144,
    ENEMY_IMAGE_KINGDOM_SOLDIER_CORPSE = 152,
};

typedef struct enemy_cart_offset {
    int32_t dx;
    int32_t dy;
} enemy_cart_offset;

extern enemy_cart_offset enemy_cart_offsets[DIRECTION_COUNT];

typedef struct enemy_foot_rule {
    int32_t speed_factor;
    int32_t attack_sound;
} enemy_foot_rule;

extern enemy_foot_rule enemy_foot_rules[ENEMY_FOOT_KINDS];

void enemy_action(void);
void figure_enemy_properties_init(void);
int32_t enemy_find_soldier(int32_t x, int32_t y);
int32_t figure_combat_get_missile_target_for_enemy(int32_t figure_id, int32_t max_distance);
int32_t figure_enemy_near_building_site(int32_t x1, int32_t y1, int32_t x2, int32_t y2);
void figure_enemy_archer_action(void);
void figure_enemy_infantry_action(void);
void figure_enemy_chariot_action(void);
void figure_enemy_camel_action(void);
void figure_enemy_elephant_action(void);
void figure_enemy_barbarian_chariot_action(void);
void figure_enemy_barbarian_light_action(void);
void figure_enemy_barbarian_heavy_action(void);
void figure_enemy_barbarian_archer_action(void);
void figure_enemy_barbarian_mounted_action(void);
void figure_enemy_barbarian_heavy_2_action(void);
void figure_enemy_egyptian_infantry_action(void);
void figure_enemy_egyptian_archer_action(void);
void figure_enemy_egyptian_chariot_action(void);
void figure_kingdom_soldier_action(void);
void figure_bedouin_action(void);

#endif
