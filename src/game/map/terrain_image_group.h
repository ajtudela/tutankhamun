// The image groups of the terrain (an index into the image collection's group table): grass, leveled ground, the
// ghost mask tile, water (shallow, deep, after the blood river), roads, canals and bridges.

#ifndef MAP_TERRAIN_IMAGE_GROUP_H
#define MAP_TERRAIN_IMAGE_GROUP_H

enum terrain_image_group {
    IMAGE_GROUP_LEVELED_GROUND = 500,
    IMAGE_GROUP_CANAL = 506,
    IMAGE_GROUP_GRASS = 507,
    IMAGE_GROUP_WATER_SHALLOW = 516,
    IMAGE_GROUP_FLAT_TILE = 518,
    IMAGE_GROUP_CANAL_ROAD = 528,
    IMAGE_GROUP_ROAD_PAVED = 530,
    IMAGE_GROUP_ROAD = 540,
    IMAGE_GROUP_FLOODPLAIN_ROAD = 548,
    IMAGE_GROUP_WATER_DEEP = 558,
    IMAGE_GROUP_BRIDGE = 560,
    IMAGE_GROUP_WATER_DEEP_SWEPT = 718,
};

#endif
