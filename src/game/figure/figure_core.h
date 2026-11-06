// The functions of figure/figure_core.c that other files call, the name counters per text group and the name groups by
// figure type and by enemy type.

#ifndef FIGURE_FIGURE_CORE_H
#define FIGURE_FIGURE_CORE_H

#include <stdbool.h>
#include <stdint.h>

enum figure_name_counter_slots {
    FIGURE_NAME_COUNTERS = 41,
    FIGURE_NAME_GROUP_FIRST = 254,
    FIGURE_NAME_GROUP_COUNTER_BASE = 294,
    FIGURE_NAME_GROUP_ENEMY_EXTRA_FIRST = 300,
    FIGURE_NAME_GROUP_ENEMY_EXTRA_LAST = 303,
};

typedef struct {
    int32_t by_slot[FIGURE_NAME_COUNTERS];
} figure_name_counter_record;

typedef struct {
    int32_t group;
    int32_t count;
} figure_name_group;

extern figure_name_group figure_name_groups[];
extern figure_name_group enemy_name_groups[];

int32_t map_figure_count_at(int32_t grid_offset);
int32_t map_figure_find_target(int32_t figure_id);
int32_t figure_create(int32_t player, int32_t type, int32_t x, int32_t y, int32_t direction);
void map_figure_delete(int32_t figure_id);
void map_figure_add(int32_t figure_id);
void figure_delete(int32_t figure_id);
void map_figure_update(int32_t figure_id);
void figure_name_set(int32_t figure_id);
bool figure_id_is_reserved(int32_t figure_id);
int32_t map_figure_of_type_at(int32_t figure_id, int32_t type);
bool figure_is_herd_animal(int32_t type);
bool figure_is_predator_animal(int32_t type);
bool figure_is_boat(int32_t type);

#endif
