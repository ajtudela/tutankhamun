// The movement functions of figure/movement.c: walking a route, roaming, following, the cross-country line and the
// missile line check.

#ifndef FIGURE_MOVEMENT_H
#define FIGURE_MOVEMENT_H

#include <stdint.h>

enum movement_step {
    MOVEMENT_STEPS_PER_TILE = 15,
    MOVEMENT_TILE_CHANGE_STEP = 8,
    MOVEMENT_BRIDGE_CLIMB_TICKS = 18,
    MOVEMENT_IN_BUILDING_WAIT_TICKS = 8,
    MOVEMENT_FERRY_PASS_TILES = 3,
    MOVEMENT_MISSILE_STEPS_PER_TILE = 8,
    MOVEMENT_MISSILE_MAX_TILES = 1000,
    MOVEMENT_MISSILE_WALL_FREE_TILES = 6,
};

enum movement_action {
    MOVEMENT_ACTION_START_OVER = 8,
};

enum movement_rule {
    MOVEMENT_RULE_BUILDING_MAX_DAMAGE,
    MOVEMENT_RULE_WALL_MAX_DAMAGE,
    MOVEMENT_RULE_GATEHOUSE_MAX_DAMAGE,
    MOVEMENT_RULE_ROAM_START_DISTANCE,
    MOVEMENT_RULE_ROAM_ROAD_RADIUS,
    MOVEMENT_RULE_COUNT,
};

extern int32_t movement_rules[MOVEMENT_RULE_COUNT];

enum movement_roam {
    MOVEMENT_ROAM_TURN_CLOCKWISE = 2,
    MOVEMENT_ROAM_TURN_COUNTER_CLOCKWISE = -2,
    MOVEMENT_ROAM_TURN_TICKS = 5,
    MOVEMENT_ROAM_STRAIGHT_TICKS = 100,
    MOVEMENT_ROAM_NO_TURN_YET = -1,
    MOVEMENT_ROAM_NO_DIRECTION = -1,
    MOVEMENT_ROAM_ROAD_DIRECTIONS = 4,
    MOVEMENT_ROAM_EVEN_DIRECTION_MASK = 6,
};

enum bridge_height {
    BRIDGE_HEIGHT_GROUND = 0,
    BRIDGE_HEIGHT_RAMP_BACK = 14,
    BRIDGE_HEIGHT_RAMP_FRONT = 15,
    BRIDGE_HEIGHT_LOW_SPAN_ALONG_X = 27,
    BRIDGE_HEIGHT_LOW_SPAN_ALONG_Y = 30,
    BRIDGE_HEIGHT_SHIP_SPAN = 36,
};

enum cross_country_axis {
    CROSS_COUNTRY_ALONG_X = 1,
    CROSS_COUNTRY_ALONG_Y = 2,
};

void figure_movement_walk_ticks(int32_t figure_id, int32_t num_ticks);
void figure_movement_move_ticks(int32_t figure_id, int32_t speed);
void figure_movement_roam_ticks(int32_t figure_id, int32_t num_ticks);
void figure_movement_init_roaming(int32_t figure_id);
void figure_movement_follow_ticks(int32_t figure_id, int32_t leader_id, int32_t num_ticks);
void figure_movement_roam_set_direction(int32_t figure_id);
void figure_movement_cross_country_update_delta(int32_t figure_id);
void figure_movement_cross_country_advance_x(int32_t figure_id);
void figure_movement_cross_country_advance_y(int32_t figure_id);
void figure_movement_advance_tick(int32_t figure_id);
void figure_movement_move_to_next_tile(int32_t figure_id);
void figure_movement_set_next_route_tile_direction(int32_t figure_id);
void figure_movement_advance_route_tile(int32_t figure_id);
int32_t figure_movement_move_ticks_cross_country(int32_t figure_id, int32_t num_ticks);
void figure_movement_set_cross_country_direction(int32_t figure_id, int32_t x_src, int32_t y_src, int32_t x_dst, int32_t y_dst, int32_t is_missile);
int32_t figure_bridge_height(int32_t sprite);
int32_t figure_movement_can_launch_cross_country_missile(int32_t x_src, int32_t y_src, int32_t x_dst, int32_t y_dst);

#endif
