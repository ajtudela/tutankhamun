// Citizen routing over land: the citizen land grid reset, the reachability test, and the deletion of the first wall,
// canal or garden that blocks a search (used to reopen a blocked kingdom road). Also two canal image helpers: the
// canal pieces under a road and the floodplain variant of a canal image.

#include "map/routing_citizen.h"

#include "building/building_data.h"
#include "map/grid.h"
#include "map/map_data.h"
#include "map/routing_internal.h"
#include "map/routing_terrain.h"

#include <string.h>

void map_routing_clear_land_citizen(void)
{
    memset(routing_grids.land_citizen, ROUTING_CITIZEN_BLOCKED, GRID_CELLS);
}

static int32_t is_citizen_land(int32_t next, int32_t current)
{
    (void) current;
    return routing_grids.land_citizen[next] >= ROUTING_CITIZEN_ROAD;
}

int32_t map_routing_citizen_can_travel_over_land(int32_t src_x, int32_t src_y, int32_t dst_x, int32_t dst_y)
{
    int32_t dst_offset = map_grid_offset(dst_x, dst_y);
    routing_route_queue(src_x, src_y, dst_offset, is_citizen_land);
    return routing_grids.distance[dst_offset] != 0;
}

static void clear_wall_or_canal(int32_t offset)
{
    map_grids.edge[offset] |= MAP_EDGE_DRAW_TILE;
    map_grids.terrain[offset] &= TERRAIN_KEPT_BY_BUILDING;
    map_grids.bitfields[offset] &= ~MAP_BITFIELD_SIZE;
}

int32_t map_routing_delete_first_wall_or_canal(int32_t src_x, int32_t src_y, int32_t dst_x, int32_t dst_y)
{
    routing_queue_start(src_x, src_y);
    int32_t dst = map_grid_offset(dst_x, dst_y);
    do {
        int32_t offset = routing_grids.queue_items[routing_queue_head];
        if (offset == dst) {
            break;
        }
        int32_t distance = routing_grids.distance[offset] + 1;
        for (int32_t i = 0; i < ROUTING_NEIGHBOUR_COUNT; i++) {
            int32_t next = offset + routing_neighbour_offsets[i];
            if (routing_grids.distance[next] != 0) {
                continue;
            }
            if (routing_grids.land_citizen[next] < ROUTING_CITIZEN_ROAD) {
                if (map_grids.terrain[next] & TERRAIN_WALL_CANAL_OR_GARDEN) {
                    clear_wall_or_canal(next);
                    return 0;
                }
            } else {
                routing_enqueue(next, distance);
            }
        }
        routing_queue_advance();
    } while (routing_queue_head != routing_queue_tail);
    return routing_grids.distance[dst] != 0;
}

int32_t map_canal_with_road_image(int32_t image_offset)
{
    switch (image_offset) {
        case CANAL_IMAGE_STRAIGHT_C:
            return CANAL_IMAGE_ROAD_CROSSING_C;
        case CANAL_IMAGE_STRAIGHT_A:
            return CANAL_IMAGE_ROAD_CROSSING_A;
        case CANAL_IMAGE_STRAIGHT_D:
            return CANAL_IMAGE_ROAD_CROSSING_D;
        case CANAL_IMAGE_STRAIGHT_B:
            return CANAL_IMAGE_ROAD_CROSSING_B;
        default:
            return image_offset;
    }
}

int32_t map_canal_floodplain_image(int32_t image_id)
{
    return image_id + CANAL_IMAGE_FLOODPLAIN_OFFSET;
}
