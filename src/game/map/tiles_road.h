// Road tile images (map/tiles_road.c): dirt and paved roads, roads on the floodplain and over canals, and the road
// image a canal tile takes under a road.

#ifndef MAP_TILES_ROAD_H
#define MAP_TILES_ROAD_H

#include <stdint.h>

enum canal_road_image {
    CANAL_ROAD_IMAGE_DIRT = 15,
    CANAL_ROAD_IMAGE_DIRT_OTHER = 16,
    CANAL_ROAD_IMAGE_CROSSING = 36,
    CANAL_ROAD_IMAGE_PAVED = 42,
    CANAL_ROAD_IMAGE_PAVED_OTHER = 43,
};

enum canal_image_set {
    CANAL_WATER_IMAGES = 48,
};

enum canal_piece {
    CANAL_PIECE_STRAIGHT = 0,
    CANAL_PIECE_STRAIGHT_OTHER = 1,
    CANAL_PIECE_CROSSING = 2,
    CANAL_PIECE_CORNER = 3,
    CANAL_PIECE_OTHER = 4,
};

void map_tiles_update_region_walls(int32_t x, int32_t y, int32_t size);
void map_tiles_update_region_canal_images(int32_t x0, int32_t y0, int32_t x1, int32_t y1);
void map_tiles_set_road(int32_t x, int32_t y, int32_t set);
void map_tiles_set_canal(int32_t x, int32_t y, int32_t full, int32_t set);
int32_t map_image_context_set_tiles_gatehouse(void);
void map_image_context_add_tiles_wall(void);
void map_tiles_update_region_roads(int32_t x0, int32_t y0, int32_t x1, int32_t y1);
int32_t map_get_canal_with_road_image(int32_t grid_offset);
void map_tiles_set_canal_image(int32_t full);
void map_tiles_update_region_canals(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t full);
void map_tiles_update_all_roads(void);
void map_image_context_set_canal_road_tiles(void);
void map_image_context_add_gatehouse_tiles(void);
void map_image_context_add_canal_side(int32_t dir, int32_t *count);
int32_t map_tiles_road_is_paved(void);
void map_tiles_set_road_image(void);
void map_tiles_update_all_walls(void);

#endif
