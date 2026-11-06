// The functions of empire_editor_object_panel.c and empire_editor_goods_slot_offered, the goods the sold goods slot
// list offers (empire_editor_data.c).

#ifndef EDITOR_EMPIRE_EDITOR_OBJECT_PANEL_H
#define EDITOR_EMPIRE_EDITOR_OBJECT_PANEL_H

#include "city/resource_type.h"
#include <stdint.h>

extern int32_t empire_editor_goods_slot_offered[RESOURCE_MAX];

void empire_editor_battle_path(void);
void empire_editor_battle_order(void);
void empire_editor_city_text_align(void);
void empire_editor_city_type(void);
void empire_editor_city_name(void);
void empire_editor_region_name(void);
void empire_editor_city_cost(void);
void empire_editor_city_route(void);
void empire_editor_trade_amounts(void);
void empire_editor_goods_slot(void);
void empire_editor_goods_sold_amount_slot(void);
void empire_editor_goods_chosen_sold(void);
void empire_editor_goods_chosen_bought(void);
void empire_editor_goods_bought_amount_slot(void);
void empire_editor_choose_image_kind(void);

#endif
