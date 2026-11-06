// Road access (map/road_access.c): the road next to or near a building and the roads open to a roaming walker.

#ifndef MAP_ROAD_ACCESS_H
#define MAP_ROAD_ACCESS_H

#include <stdint.h>

enum road_access_rank {
    ROAD_RANK_RADIUS_NETWORKS = 5,
    ROAD_RANK_UNLISTED_NETWORK = 11,
    ROAD_RANK_NONE = 12,
};

enum road_access_rule {
    ROAD_ACCESS_RING_TILES = 24,
    ROAD_ACCESS_NEARBY_TILES = 8,
    ROAD_ACCESS_TRIPLE_STEP = 3,
    ROAD_ACCESS_TRIPLE_SHIFT_END = 7,
    ROAD_ACCESS_TEMPLE_RING_SHORT = 9,
    ROAD_ACCESS_TEMPLE_RING_LONG = 15,
};

int32_t map_road_within_radius(int32_t x, int32_t y, int32_t size, int32_t radius, int32_t kind);
int32_t map_has_road_access(int32_t x, int32_t y, int32_t size);
int32_t map_has_road_access_granary(int32_t x, int32_t y);
int32_t map_has_road_access_nearby(int32_t x, int32_t y);
int32_t map_has_road_access_temple_complex(int32_t x, int32_t y, int32_t orientation);
int32_t map_reachable_road_around(int32_t x, int32_t y, int32_t size);
int32_t map_reachable_road_triple(int32_t x, int32_t y, int32_t size);
int32_t map_get_adjacent_road_tiles_for_roaming(int32_t grid_offset, int32_t figure_id);
int32_t map_closest_road_within_radius(int32_t x, int32_t y, int32_t size, int32_t max_radius);

#endif
