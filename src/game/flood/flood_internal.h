// The Nile flood object: a pointer per map tile to its flood record, the floodplain settings, thirty lists of flood
// records by depth (sorted by the tile's random value) and the state of the current flood. The flood advances in units
// of 25 ticks; it is imminent less than 28 units before and resting less than 28 units after, and a map without
// floodplain stays idle.

#ifndef FLOOD_FLOOD_INTERNAL_H
#define FLOOD_FLOOD_INTERNAL_H

#include "flood/flood_types.h"
#include "map/grid.h"

#include <stdbool.h>
#include <stdint.h>

#define FLOOD_TICKS_PER_UNIT 25

void floodplain_settings_reset(floodplain_settings *s);
void floodplain_settings_init(floodplain_settings *s);
void floodplain_settings_randomize_next_quality(floodplain_settings *s);
void floods_update_next_flood_params(floodplain_settings *s);
void floodplain_settings_reset_season(floodplain_settings *s);
int32_t floodplain_settings_get_quality(floodplain_settings *s);
void floodplain_settings_reset_next_quality(floodplain_settings *s);
void floods_adjust_next_quality(floodplain_settings *s, int32_t delta);
void flood_tile_init(
    flood_tile *tile, int32_t grid_offset, int32_t x, int32_t y, int32_t depth, flood_list *list, int32_t is_floodplain);
void flood_list_construct(flood_list *list);
void flood_list_clear(flood_list *list);
void flood_list_destruct(flood_list *list);
flood_tile *flood_list_insert(flood_list *list, int32_t grid_offset, int32_t x, int32_t y, int32_t depth, int32_t is_floodplain);
void flood_list_process(flood_list *list, bool flooding, int32_t percent, bool all);
void flood_tile_apply(flood_tile *tile, bool flooding);
void flood_list_recede_pass(flood_list *list);
void floods_reset(floods *self);
void floods_build(floods *self, int32_t x0, int32_t y0, int32_t x1, int32_t y1);
void floods_prepare_next_flood(floods *self);
void floods_move_boat_to_water(int32_t figure_id);
void floods_end(floods *self);
void floods_post_flood_prediction_message(floods *self);
void floods_tick_update(floods *self);
void floods_fill_canals(floods *self);
void flood_tile_fill_canal(flood_tile *tile);
int32_t floods_get_month_before(floods *self);
void floods_worsen_next_quality(floods *self);
void floods_improve_next_quality(floods *self);
void floods_construct(floods *self);

void floods_static_init_units(void);

#endif
