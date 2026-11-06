// The sphinx (building/sphinx.c): its stages, work spots, workers and work percentages.

#ifndef BUILDING_SPHINX_H
#define BUILDING_SPHINX_H

#include <stdbool.h>
#include <stdint.h>

enum sphinx_stage {
    SPHINX_STAGE_COUNT = 16,
    SPHINX_SCRIPTS_PER_STAGE = 3,
};

int32_t sphinx_get_work_percentages(
    int32_t *carpenter_percent, int32_t *mason_percent, int32_t *carpenters, int32_t *carpenters_done, int32_t *masons, int32_t *masons_done);
void sphinx_figure_work_tick(int32_t building_id, int32_t figure_id, uint8_t next_action);
int32_t sphinx_find_work_for_figure(int32_t figure_id);
void sphinx_count_assigned(int32_t figure_type, int32_t *count);
int32_t sphinx_figure_is_working_on(int32_t figure_type);
bool sphinx_has_free_spot_for_type(int32_t figure_type);

void sphinx_reset_work_spots(void);
void sphinx_load_stage_scripts(void);
void sphinx_draw_spots(int32_t x, int32_t y);

#endif
