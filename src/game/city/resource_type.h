// Resource ids in the game's numbering. 1..8 are the foods (RESOURCE_MIN_FOOD up to RESOURCE_MAX_FOOD, exclusive),
// 1..35 the goods (RESOURCE_MIN up to RESOURCE_MAX, exclusive: the size of every per-resource table); 27 has no good.
// Deben and troops are not goods: the scenario's requests and the advisors use 36 and 37 for them.

#ifndef CITY_RESOURCE_TYPE_H
#define CITY_RESOURCE_TYPE_H

typedef enum resource_type {
    RESOURCE_NONE = 0,
    RESOURCE_GRAIN = 1,
    RESOURCE_MEAT = 2,
    RESOURCE_LETTUCE = 3,
    RESOURCE_CHICKPEAS = 4,
    RESOURCE_POMEGRANATES = 5,
    RESOURCE_FIGS = 6,
    RESOURCE_FISH = 7,
    RESOURCE_GAMEMEAT = 8,
    RESOURCE_STRAW = 9,
    RESOURCE_WEAPONS = 10,
    RESOURCE_CLAY = 11,
    RESOURCE_BRICKS = 12,
    RESOURCE_POTTERY = 13,
    RESOURCE_BARLEY = 14,
    RESOURCE_BEER = 15,
    RESOURCE_FLAX = 16,
    RESOURCE_LINEN = 17,
    RESOURCE_GEMS = 18,
    RESOURCE_LUXURY_GOODS = 19,
    RESOURCE_TIMBER = 20,
    RESOURCE_GOLD = 21,
    RESOURCE_REEDS = 22,
    RESOURCE_PAPYRUS = 23,
    RESOURCE_STONE = 24,
    RESOURCE_LIMESTONE = 25,
    RESOURCE_GRANITE = 26,
    RESOURCE_UNUSED = 27,
    RESOURCE_CHARIOTS = 28,
    RESOURCE_COPPER = 29,
    RESOURCE_SANDSTONE = 30,
    RESOURCE_OIL = 31,
    RESOURCE_HENNA = 32,
    RESOURCE_PAINT = 33,
    RESOURCE_LAMPS = 34,
    RESOURCE_MARBLE = 35,
    RESOURCE_DEBEN = 36,
    RESOURCE_TROOPS = 37,
    RESOURCE_MIN = 1,
    RESOURCE_MAX = 36,
    RESOURCE_MIN_FOOD = 1,
    RESOURCE_MAX_FOOD = 9,
} resource_type;

#endif
