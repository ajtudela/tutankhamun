// House size changes (building/house_size.c): merging, expanding, splitting and devolving houses, and the area
// sizes, merge chance and devolve shares they use.

#ifndef BUILDING_HOUSE_SIZE_H
#define BUILDING_HOUSE_SIZE_H

#include <stdint.h>

enum house_road_reach {
    HOUSE_ROAD_RADIUS = 2,
};

enum house_area_tiles {
    HOUSE_TILES_2X2 = 4,
    HOUSE_TILES_3X3 = 9,
    HOUSE_TILES_4X4 = 16,
};

enum house_expand_search {
    HOUSE_EXPAND_DIRECTIONS = 4,
    HOUSE_EXPAND_PASS_EMPTY_LAND = 1,
    HOUSE_EXPAND_PASS_GARDENS = 2,
    HOUSE_EXPAND_PASSES = 3,
};

enum house_merge_chance {
    HOUSE_MERGE_RANDOM_MASK = 7,
    HOUSE_MERGE_RANDOM_BELOW = 5,
};

enum house_merged_image {
    HOUSE_MERGED_IMAGE = 4,
    HOUSE_MERGED_IMAGE_WITH_OFFSET = 5,
};

enum house_devolve_share {
    HOUSE_DEVOLVE_3X3_SHARE = 6,
    HOUSE_DEVOLVE_4X4_SHARE = 8,
};

int32_t building_house_can_expand(int32_t building_id, int32_t num_tiles);
int32_t building_house_merge(int32_t building_id);
int32_t building_house_prepare_for_merge(int32_t building_id, int32_t num_tiles);
void building_house_change_to(int32_t building_id, int32_t type);
void building_house_change_to_vacant_lot(int32_t building_id);
void building_house_check_for_corruption(int32_t building_id);
void building_house_devolve_from_common_manor(int32_t building_id);
void building_house_devolve_from_modest_estate(int32_t building_id);
void building_house_expand_to_common_manor(int32_t building_id, int32_t population);
void building_house_expand_to_common_residence(int32_t building_id, int32_t population);
void building_house_expand_to_modest_estate(int32_t building_id, int32_t population);
void building_house_merge_to_size2(int32_t building_id, int32_t population);
void building_house_split_houses_in_area(int32_t building_id, int32_t num_tiles);
void building_house_split_merged(int32_t building_id);
void building_house_split_size2(int32_t building_id);
void building_house_split_size3(int32_t building_id);

#endif
