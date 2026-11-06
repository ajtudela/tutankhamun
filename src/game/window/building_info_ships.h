// The ship panels of the building info window (building_info_ships.c) and the thresholds of the hull texts
// (ship_hull_text_rules).

#ifndef WINDOW_BUILDING_INFO_SHIPS_H
#define WINDOW_BUILDING_INFO_SHIPS_H

#include <stdint.h>

enum ship_order_button {
    SHIP_ORDER_FIRST = 0,
    SHIP_ORDER_SECOND = 1,
    SHIP_ORDER_THIRD = 2,
    SHIP_ORDER_REPAIR = 3,
    SHIP_ORDER_RETURN = 4,
    SHIP_ORDER_BUTTONS = 5,
};

enum ship_hull_text_rule {
    SHIP_HULL_TEXT_RULE_WEAK,
    SHIP_HULL_TEXT_RULE_FAIR,
    SHIP_HULL_TEXT_RULE_AVERAGE,
    SHIP_HULL_TEXT_RULE_GOOD,
    SHIP_HULL_TEXT_RULE_STRONG,
    SHIP_HULL_TEXT_RULE_COUNT,
};

typedef struct {
    int32_t transport;
    int32_t warship;
} ship_hull_text_rule;

extern ship_hull_text_rule ship_hull_text_rules[SHIP_HULL_TEXT_RULE_COUNT];

void window_building_draw_boat(void);
void window_building_draw_ship(void);
void window_building_draw_boat_orders(int32_t figure_id);
void window_building_draw_ship_orders(int32_t figure_id);
void transport_ship_button(void);
void warship_button(void);
void transport_target_click(void);
void warship_target_click(void);
void legion_fort_click(void);
int32_t window_building_ship_damage_text(int32_t figure_id);
int32_t window_building_ship_crew_text(int32_t figure_id);
void window_building_draw_shipwright(void);
void window_building_draw_dock(void);
void window_building_draw_fishing_wharf(void);
void window_building_draw_transport_wharf(void);
void window_building_draw_warship_wharf(void);
void window_building_draw_ferry(void);

#endif
