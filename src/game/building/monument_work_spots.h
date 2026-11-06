// The work spots of the scripted monuments while they are built (building/monument_work_spots.c).

#ifndef BUILDING_MONUMENT_WORK_SPOTS_H
#define BUILDING_MONUMENT_WORK_SPOTS_H

#include "building/monument_tables.h"
#include <stdbool.h>
#include <stdint.h>

void library_check_stage_done(int32_t building_id);
int32_t library_has_free_spot(int32_t building_id, int32_t figure_id);
void library_advance_stage(monument_object *self, int32_t building_id);
void abu_simbel_check_stage_done(int32_t building_id);
void abu_simbel_figure_work_tick_spot(int32_t building_id, int32_t figure_id, uint8_t done_action);
int32_t abu_simbel_has_free_spot(int32_t building_id, int32_t figure_id);
void abu_simbel_advance_stage(monument_object *self, int32_t building_id);
bool abu_simbel_get_work_tile(monument_object *self, int32_t building_id, int32_t *x, int32_t *y);
bool abu_simbel_part_has_active_worker(monument_object *self, int32_t building_id);
bool abu_simbel_check_work_no_assign(monument_object *self, int32_t building_id, int32_t figure_id, bool assign);
void caesareum_check_stage_done(int32_t building_id);
void caesareum_figure_work_tick_spot(int32_t building_id, int32_t figure_id, uint8_t done_action);
int32_t caesareum_has_free_spot(int32_t building_id, int32_t figure_id);
void caesareum_advance_stage(monument_object *self, int32_t building_id);
void lighthouse_check_stage_done(int32_t building_id);
void lighthouse_figure_work_tick_spot(int32_t building_id, int32_t figure_id, uint8_t done_action);
int32_t lighthouse_has_free_spot(int32_t building_id, int32_t figure_id);
void lighthouse_advance_stage(monument_object *self, int32_t building_id);
int32_t library_spot_table_get_variant(void);
void library_spot_table_set_variant(int32_t variant);
void library_draw_spots(int32_t x, int32_t y);
void abu_simbel_draw_spots(int32_t x, int32_t y);
void caesareum_draw_spots(int32_t x, int32_t y);
void lighthouse_draw_spots(int32_t x, int32_t y);
bool library_has_free_spot_for_type(int32_t figure_type, int32_t stage);
bool abu_simbel_has_free_spot_for_type(int32_t figure_type, int32_t stage);
bool caesareum_has_free_spot_for_type(int32_t figure_type, int32_t stage);
bool lighthouse_has_free_spot_for_type(int32_t figure_type, int32_t stage);

#endif
