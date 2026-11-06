// Adding buildings to the map (building/construction_load.c), the venue plaza helpers, and the migration
// warning rules: the population it starts from, the missions with quiet causes and the thresholds it warns at.

#ifndef BUILDING_CONSTRUCTION_LOAD_H
#define BUILDING_CONSTRUCTION_LOAD_H

#include <stdbool.h>
#include <stdint.h>

enum migration_warning_rule {
    MIGRATION_WARNING_RULE_MIN_POPULATION,
    MIGRATION_WARNING_RULE_QUIET_MISSIONS,
    MIGRATION_WARNING_RULE_MAX_ARMED_FIGURES,
    MIGRATION_WARNING_RULE_FEW_NEWCOMERS,
    MIGRATION_WARNING_RULE_LOW_PERCENTAGE,
    MIGRATION_WARNING_RULE_COUNT,
};

enum migration_message_caller {
    MIGRATION_MESSAGE_AFTER_NEW_HOUSES = 10,
};

enum construction_load_layout {
    PLAZA_GARDEN_IMAGES = 4,
    PAVILION_STAGE_TILES = 2,
    FORT_GROUND_SIZE = 4,
    FORT_SOLDIERS_DX = 4,
    FORT_SOLDIERS_DY = 1,
    NATIVE_MEETING_IMAGE_OFFSET = 2,
    TOWER_SIZE = 2,
    TOWER_WALL_UPDATE_RADIUS = 10,
    TEMPLE_COMPLEX_SCRIPT_SLOT = 10,
    SPHINX_PART_DX = 6,
    SPHINX_FIRST_STAGE = 1,
};

enum sphinx_part {
    SPHINX_PART_LEFT = 0,
    SPHINX_PART_MIDDLE = 1,
    SPHINX_PART_RIGHT = 2,
    SPHINX_PART_LEFT_TURNED = 3,
    SPHINX_PART_MIDDLE_TURNED = 4,
    SPHINX_PART_RIGHT_TURNED = 5,
};

extern int32_t migration_warning_rules[MIGRATION_WARNING_RULE_COUNT];

void map_add_venue_plaza_tiles(int32_t offset, int32_t type, int32_t building_id);
void venue_place_adjacent_tiles(int32_t building_id, int32_t x, int32_t y);
void city_migration_post_message(int32_t on_change);
void building_construction_add_to_map(int32_t player, int32_t type, int32_t id, int32_t x, int32_t y, int32_t size, int32_t creating, bool no_slot);
void venue_orientation_images(int32_t which, int32_t *main_image, int32_t *next_image);

#endif
