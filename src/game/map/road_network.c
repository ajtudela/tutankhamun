// Road networks: every connected set of road tiles gets an id once a day, the tiles are cut into segments, and the
// largest networks of the current player are kept in its city record.
//
// Network ids are a byte in the grid, so ids above 255 wrap; of two equal networks the later one stays out of the list
// of the largest.

#include "map/road_network.h"

#include "city/city_player.h"
#include "map/map_data.h"
#include "map/map_layers.h"
#include "map/routing_terrain.h"

#include <stddef.h>
#include <string.h>

int32_t road_network_size;

int32_t road_segment_tile_count;

int32_t road_segment_next_id;

void map_road_network_mark(int32_t grid_offset, int32_t network_id)
{
    int32_t tail = 0;
    memset(road_network_queue, 0, MAP_BFS_QUEUE_SIZE * sizeof(road_network_queue[0]));
    road_network_queue_head = 0;
    road_network_queue_tail = 0;
    int32_t segment_id = road_segment_next_id;
    int32_t guard = 1;
    for (;;) {
        map_grids.road_segment[grid_offset] = (uint8_t) segment_id;
        map_grids.road_network[grid_offset] = (uint8_t) network_id;
        int32_t next_offset = -1;
        for (int32_t i = 0; i < 4; i++) {
            int32_t new_offset = grid_offset + road_network_cardinal_delta[i];
            int8_t routing = routing_grids.land_citizen[new_offset];
            if (routing < ROUTING_CITIZEN_ROAD || routing > ROUTING_CITIZEN_PASSABLE) {
                continue;
            }
            if (routing == ROUTING_CITIZEN_PASSABLE && !(map_grids.terrain[new_offset] & TERRAIN_ACCESS_RAMP)) {
                continue;
            }
            if (map_grids.road_network[new_offset]) {
                continue;
            }
            map_grids.road_segment[new_offset] = (uint8_t) segment_id;
            map_grids.road_network[new_offset] = (uint8_t) network_id;
            road_network_size++;
            if (next_offset == -1) {
                next_offset = new_offset;
                road_segment_tile_count++;
            } else {
                road_network_queue[tail++] = new_offset;
                if (tail >= MAP_BFS_QUEUE_SIZE) {
                    tail = 0;
                }
            }
        }
        if (next_offset == -1) {
            if (road_network_queue_head == tail) {
                break;
            }
            next_offset = road_network_queue[road_network_queue_head++];
            if (road_network_queue_head >= MAP_BFS_QUEUE_SIZE) {
                road_network_queue_head = 0;
            }
        }
        if (road_segment_tile_count > ROAD_SEGMENT_MAX_TILES) {
            segment_id++;
            road_segment_tile_count = 0;
        }
        if (next_offset <= -1) {
            break;
        }
        grid_offset = next_offset;
        if (guard++ >= GRID_CELLS) {
            break;
        }
    }
    road_network_queue_tail = tail;
    road_segment_next_id = segment_id;
}

void map_road_network_update(void)
{
    road_network_entry *largest = city_player_current()->largest_road_networks;
    for (int32_t i = 0; i < PLAYER_LARGEST_ROAD_NETWORKS; i++) {
        largest[i].id = 0;
        largest[i].size = 0;
    }
    memset(map_grids.road_network, 0, GRID_CELLS);
    memset(map_grids.road_segment, 0, GRID_CELLS);
    road_segment_next_id = 1;
    road_segment_tile_count = 0;
    int32_t network_id = 1;
    map_tile_cursor_offset = map_info.grid_start;
    for (int32_t y = 0; y < map_info.grid_height; y++, map_tile_cursor_offset += map_info.grid_border_size) {
        for (int32_t x = 0; x < map_info.grid_width; x++, map_tile_cursor_offset++) {
            int32_t grid_offset = map_tile_cursor_offset;
            if ((map_grids.terrain[grid_offset] & (TERRAIN_ROAD | TERRAIN_WATER)) != TERRAIN_ROAD || map_grids.road_network[grid_offset]) {
                continue;
            }
            road_network_size = 1;
            map_road_network_mark(grid_offset, network_id);
            int32_t size = road_network_size;
            for (int32_t n = 0; n < PLAYER_LARGEST_ROAD_NETWORKS; n++) {
                if (size > largest[n].size) {
                    for (int32_t m = PLAYER_LARGEST_ROAD_NETWORKS - 1; m > n; m--) {
                        largest[m] = largest[m - 1];
                    }
                    largest[n].id = network_id;
                    largest[n].size = size;
                    break;
                }
            }
            network_id++;
        }
    }
}
