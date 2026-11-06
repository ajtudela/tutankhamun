// The obelisks (building/obelisk.c): their progress, work spots, scripts and workers.

#ifndef BUILDING_OBELISK_H
#define BUILDING_OBELISK_H

#include <stdint.h>

enum obelisk_progress {
    OBELISK_PROGRESS_THIRD = 33,
    OBELISK_PROGRESS_START = OBELISK_PROGRESS_THIRD,
    OBELISK_PROGRESS_DONE = 100,
};

enum obelisk_scripts {
    OBELISK_SMALL_SCRIPTS = 4,
    OBELISK_LARGE_SCRIPTS = 6,
};

void obelisk_load_script_for_slot(int32_t slot);
void obelisk_load_scripts_all_slots(void);
int32_t obelisk_find_work_for_figure(int32_t figure_id);
void obelisk_figure_work_tick(int32_t building_id, int32_t figure_id, uint8_t next_action);
void obelisk_count_assigned_figures(int32_t figure_type, int32_t *count, int32_t building_id);
int32_t obelisk_figure_is_working_on(int32_t figure_type, int32_t building_id);
int32_t obelisk_has_free_spot_for_type(int32_t figure_type, int32_t building_id);
int32_t obelisk_init_on_place(int32_t building_id);
void obelisk_update_image(int32_t building_id, int32_t x, int32_t y);
int32_t obelisk_find_incomplete(void);
void obelisk_reset_work_spots(int32_t building_id);
void obelisk_draw_overlay(int32_t building_id, int32_t x, int32_t y);

#endif
