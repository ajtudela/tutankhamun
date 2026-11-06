// Road networks (map/road_network.c): marking one network and the update of all of them.

#ifndef MAP_ROAD_NETWORK_H
#define MAP_ROAD_NETWORK_H

#include <stdint.h>

enum map_bfs_queue {
    MAP_BFS_QUEUE_SIZE = 1000,
};

enum road_network_rule {
    ROAD_SEGMENT_MAX_TILES = 30,
};

void map_road_network_mark(int32_t grid_offset, int32_t network_id);
void map_road_network_update(void);

#endif
