// The monument framework (building/monument.c): city slots, definitions, resource demand, and the pyramid's
// progress report (phase, height, blocks, percent, workers, material), part creation and entry tile.

#ifndef BUILDING_MONUMENT_H
#define BUILDING_MONUMENT_H

#include <stdbool.h>
#include <stdint.h>

enum monument_progress {
    MONUMENT_PROGRESS_COMPLETE = 100,
};

enum monument_slots {
    MONUMENT_SLOTS = 3,
    MONUMENT_DEFINITIONS = 38,
};

enum monument_created_sequence {
    MONUMENT_KEPT_FROM_EARLIER_MISSION = 0xFFFF,
};

enum pyramid_phase {
    PYRAMID_PHASE_SITE_PREPARATION = 1,
    PYRAMID_PHASE_BLOCKS = 2,
    PYRAMID_PHASE_FINISHING = 3,
    PYRAMID_PHASE_COMPLETE = 4,
};

enum pyramid_progress_units {
    PYRAMID_PREPARATION_PERCENT = 5,
    PYRAMID_UNITS_PER_BLOCK = 2,
    PYRAMID_MATERIAL_PER_WAITING = 200,
    PYRAMID_PREP_MIN_STAGE_NONE = 999,
    PYRAMID_PART_SIZE = 2,
};

typedef struct {
    int32_t phase;
    int32_t height;
    int32_t height_next;
    int32_t layer_count;
    int32_t main_placed;
    int32_t casing_placed;
    int32_t frontier_main;
    int32_t frontier_casing;
    int32_t main_remaining;
    int32_t casing_remaining;
    int32_t main_total;
    int32_t casing_total;
    int32_t phase_percent;
    int32_t percent;
    int32_t prep_min_stage;
    int32_t prep_max_stage;
    int32_t prep_done;
    int32_t prep_total;
    int32_t stonemasons;
    int32_t bricklayers;
    int32_t waiting_main;
    int32_t waiting_casing;
    int32_t carpenters;
    int32_t waiting;
    int32_t block_workers;
    int32_t laborers;
    int32_t material_main;
    int32_t material_casing;
} pyramid_progress;

void city_set_monument_building_id(int32_t building_id, int32_t slot);
void city_clear_monument_building_id(int32_t building_id, int32_t slot);
int32_t city_find_monument_slot(int32_t building_id);
void monument_unregister(int32_t building_id);
int32_t monument_get_def_id(int32_t building_id);
int32_t monument_register(int32_t building_id);
bool city_monuments_need_resource(int32_t resource, bool force);
int32_t scenario_monument_slot_obelisk_type(int32_t slot);
int32_t scenario_monument_slot_is_pyramid(int32_t slot);
int32_t guild_has_monument_demand(int32_t guild_type);
int32_t guild_serves_monument_def(int32_t guild_type, int32_t def_id);
int32_t monument_def_uses_resource(int32_t def_id, int32_t resource);
int32_t monument_needs_resource(int32_t building_id, int32_t resource);
void city_rebuild_monument_slots(void);

void pyramid_get_progress_info(int32_t building_id, pyramid_progress *out);
void pyramid_create_parts(int32_t player, int32_t x, int32_t y, int32_t building_id);
void pyramid_part_mark_terrain(int32_t building_id);
void pyramid_create_entry_tile(int32_t building_id);
void pyramid_remove_entry_tile(int32_t building_id, bool delete_it);
void pyramid_get_entry_tile(int32_t building_id, int32_t *grid_offset, int32_t *x, int32_t *y);
void pyramid_get_entry_road_access(int32_t building_id, int32_t *x, int32_t *y);
int32_t pyramid_set_figure_destination_entry(int32_t figure_id, int32_t building_id);
int32_t pyramid_find_by_complex_building(int32_t building_id);
int32_t pyramid_has_complex_temple(int32_t size);
void pyramid_clear_complex_area(int32_t building_id);


int32_t monument_defs_count(void);
bool building_is_not_monument(int32_t building_id);
bool building_type_is_monument(int32_t type);

#endif
