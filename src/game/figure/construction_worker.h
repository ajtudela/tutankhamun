// The construction guild workers of figure/construction_worker.c, their rules (guild_worker_rules) and the task chains.

#ifndef FIGURE_CONSTRUCTION_WORKER_H
#define FIGURE_CONSTRUCTION_WORKER_H

#include <stdint.h>

enum guild_worker_constants {
    GUILD_WORKER_ANIMATION_FRAMES = 12,
    OBELISK_SMALL_PROGRESS_DIVISOR = 10,
    OBELISK_LARGE_PROGRESS_DIVISOR = 18,
};

enum tomb_artisan_task_stage {
    TOMB_ARTISAN_PAINT_STAGE = 2,
    TOMB_ARTISAN_LAMP_STAGE = 3,
};

enum guild_worker_rule {
    GUILD_WORKER_RULE_MAX_ROAM_TICKS = 0,
    GUILD_WORKER_RULE_FERRY_WAIT_TICKS = 1,
    GUILD_WORKER_RULE_GHOST_TICKS = 2,
    GUILD_WORKER_RULE_TASK_SEARCH_BASE_TICKS = 3,
    GUILD_WORKER_RULE_TASK_SEARCH_RANDOM_TICKS = 4,
    GUILD_WORKER_RULE_GUILD_ROAD_RADIUS = 5,
    GUILD_WORKER_RULE_MAUSOLEUM_HEAD_START_TICKS = 6,
    GUILD_WORKER_RULE_ABU_SIMBEL_SPOTS_IN_A_ROW = 7,
    GUILD_WORKER_RULE_LOAD = 8,
    GUILD_WORKER_RULE_CARPENTER_PROGRESS = 9,
    GUILD_WORKER_RULE_COUNT = 10,
};

extern int32_t guild_worker_rules[GUILD_WORKER_RULE_COUNT];

void building_artisans_guild_spawn_figure(int32_t building_id);
void building_bricklayers_guild_spawn_figure(int32_t building_id);
void building_carpenters_guild_spawn_figure(int32_t building_id);
void building_stonemasons_guild_spawn_figure(int32_t building_id);
int32_t figure_tomb_artisan_find_paint_task(int32_t x, int32_t y, int32_t figure_id);
int32_t figure_tomb_artisan_find_lamp_task(int32_t x, int32_t y, int32_t figure_id);
int32_t monument_find_task_carpenter(int32_t x, int32_t y, int32_t figure_id);
int32_t monument_find_task_stonemason(int32_t x, int32_t y, int32_t figure_id);
int32_t figure_bricklayer_find_monument_task(int32_t x, int32_t y, int32_t figure_id);
void figure_carpenter_action(void);
void figure_bricklayer_action(void);
void figure_stonemason_action(void);
void figure_tomb_artisan_action(void);

#endif
