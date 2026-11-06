// The tables of map/map_data.c: tile offsets, rings, image contexts and terrain lookups.

#ifndef MAP_MAP_DATA_H
#define MAP_MAP_DATA_H

#include <stdint.h>
#include "flood/flood_types.h"
#include "map/map_data_types.h"
#include "map/ring_types.h"
#include "map/routing_grid_types.h"

extern ring_tile_table ring_tile_count;

extern int32_t editor_brush_tiles[82];

extern building_tile_offset_tables building_tile_offsets;

extern int32_t water_delta2[9];

extern int32_t water_delta3[9];

extern int32_t road_network_cardinal_delta[4];

extern water_lift_side_tables water_lift_sides;

extern ring_tile *const ring_tiles_by_distance[12];

extern int32_t water_pattern2[16];

extern int32_t water_adj2_a[8];

extern int32_t water_adj2_b[8];

extern int32_t water_adj2_c[8];

extern int32_t water_pattern3[36];

extern int32_t water_adj3[8];

extern int32_t map_grid_direction_delta[8];

extern int32_t direction_xy_delta[16];

extern int32_t building_ring_xy[336];

extern ring_offset_tables ring_offsets_by_size;

extern int16_t ring_offsets[624];

extern int32_t road_network_queue_tail;

extern int32_t road_network_queue_head;

extern int32_t road_network_queue[1000];


extern int32_t map_sweep_total;

extern int32_t map_sweep_countdown;

extern uint32_t map_service_visit_grid[51984];

extern int32_t rotation_direction;

extern int32_t map_water_orientation_relative;

extern int32_t ferry_route_direction;

extern int32_t ferry_route_length;


extern int32_t building_tiles_size_y;

extern int32_t building_tiles_size_x;

extern int32_t context_item_offset;

extern int32_t context_canal_offset;

extern int32_t context_group_offset;

extern int32_t map_tile_cursor_y;

extern int32_t map_tile_cursor_x;

extern int32_t map_tile_cursor_offset;

extern int32_t map_adjacent_tiles[16];

extern int32_t map_row_start_offset[228];

extern int32_t map_row_x_end[228];

extern int32_t map_row_x_start[228];

extern uint8_t water_lift_flooded;

extern floods floods_data;

extern int32_t routing_stats_total_routes_calculated;

extern int32_t routing_stats_enemy_routes_calculated;

extern int32_t routing_source_offset;

extern int32_t routing_queue_tail;

extern int32_t routing_queue_head;

extern routing_grid_set routing_grids;

#endif
