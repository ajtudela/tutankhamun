// The industry panels of the building info window (building_info_industry.c), the farms' crop line and the month of a
// farm's next harvest.

#ifndef WINDOW_BUILDING_INFO_INDUSTRY_H
#define WINDOW_BUILDING_INFO_INDUSTRY_H

#include <stdint.h>

void window_building_draw_work_camp(void);
void window_building_draw_barley_farm(void);
void window_building_draw_grain_farm(void);
void window_building_draw_lettuce_farm(void);
void window_building_draw_fig_farm(void);
void window_building_draw_pomegranate_farm(void);
void window_building_draw_flax_farm(void);
void window_building_draw_henna_farm(void);
void window_building_draw_chickpea_farm(void);
void window_building_draw_reed_gatherer(void);
void window_building_draw_cattle_ranch(void);
void window_building_draw_stone_quarry(void);
void window_building_draw_gold_mine(void);
void window_building_draw_copper_mine(void);
void window_building_draw_gemstone_mine(void);
void window_building_draw_wood_cutter(void);
void window_building_draw_clay_pit(void);
void window_building_draw_brewery(void);
void window_building_draw_weaver(void);
void window_building_draw_papyrus_maker(void);
void window_building_draw_lamp_maker(void);
void window_building_draw_paint_maker(void);
void window_building_draw_chariot_maker(void);
void window_building_draw_brickworks(void);
void window_building_draw_weaponsmith(void);
void window_building_draw_jeweler(void);
void window_building_draw_potter(void);
void window_building_draw_carpenters_guild(void);
void window_building_draw_bricklayers_guild(void);
void window_building_draw_stonemasons_guild(void);
void window_building_draw_artisans_guild(void);
void window_building_draw_hunting_lodge(void);
void window_building_draw_quarry(int32_t resource);
void window_building_draw_farm_crop(int32_t group);
int32_t farm_next_harvest_month(int32_t building_id);

#endif
