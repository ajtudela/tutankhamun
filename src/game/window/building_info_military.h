// The military panels of the building info window (building_info_military.c) and the damage thresholds of the legion
// damage texts (legion_damage_text_rules).

#ifndef WINDOW_BUILDING_INFO_MILITARY_H
#define WINDOW_BUILDING_INFO_MILITARY_H

#include <stdint.h>

enum legion_order_button {
    LEGION_ORDER_HOLD_TIGHT = 0,
    LEGION_ORDER_HOLD_LOOSE = 1,
    LEGION_ORDER_ENGAGE = 2,
    LEGION_ORDER_MOP_UP = 3,
    LEGION_ORDER_RETURN = 4,
    LEGION_ORDER_BUTTONS = 5,
};

enum legion_damage_text_rule {
    LEGION_DAMAGE_TEXT_RULE_PERFECT,
    LEGION_DAMAGE_TEXT_RULE_VERY_GOOD,
    LEGION_DAMAGE_TEXT_RULE_GOOD,
    LEGION_DAMAGE_TEXT_RULE_AVERAGE,
    LEGION_DAMAGE_TEXT_RULE_POOR,
    LEGION_DAMAGE_TEXT_RULE_VERY_POOR,
    LEGION_DAMAGE_TEXT_RULE_COUNT,
};

extern int32_t legion_damage_text_rules[LEGION_DAMAGE_TEXT_RULE_COUNT];

void legion_order_button(void);
void legion_return_button(void);
void legion_order_button_toggle(void);
void legion_target_handle_input(void);
void warship_order_button(void);
void transport_ship_order_button(void);
void window_building_draw_legion(int32_t formation_id);
void window_building_draw_legion_foreground(int32_t formation_id);
void window_building_draw_legion_lines(int32_t formation_id, int32_t soldiers_y, int32_t damage_y, int32_t experience_y, int32_t morale_y);
void window_building_draw_fort(void);
void window_building_draw_mud_gatehouse(void);
void window_building_draw_brick_gatehouse(void);
void window_building_draw_clay_gatehouse(void);
void window_building_draw_roadblock(void);
void window_building_draw_mud_tower(void);
void window_building_draw_brick_tower(void);
void window_building_draw_clay_tower(void);
void window_building_draw_military_academy(void);
void window_building_draw_recruiter(void);

#endif
