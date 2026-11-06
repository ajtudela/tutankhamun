// Loading helpers for old files: the record re-layouts of older saves (empire cities, city players,
// buildings, storages), the farm field fix of older saves and the farm update a scenario write runs. Each
// re-layout moves every record through a zeroed temporary record, field by field.

#include "io/game_file_relayout.h"

#include "building/building.h"
#include "building/building_industry.h"
#include "building/building_storage.h"
#include "building/building_type.h"
#include "city/city_player.h"
#include "empire/empire_data.h"
#include "empire/empire_types.h"
#include "map/soil.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef struct {
    uint16_t dst;
    uint16_t src;
    uint16_t size;
} relayout_move;

enum {
    EMPIRE_CITY_BEFORE_RELAYOUT_RESOURCES = 30,
    EMPIRE_CITY_BEFORE_RELAYOUT_SELLS = offsetof(empire_city, buys) + EMPIRE_CITY_BEFORE_RELAYOUT_RESOURCES,
    EMPIRE_CITY_BEFORE_RELAYOUT_COST_TO_OPEN = EMPIRE_CITY_BEFORE_RELAYOUT_SELLS + EMPIRE_CITY_BEFORE_RELAYOUT_RESOURCES,
};

enum relayout_rule {
    CITY_PLAYERS_RELAID = 2,
    FARM_PROGRESS_FACTOR = 10,
    RELAYOUT_MAX = sizeof(city_player),
};

static const relayout_move relayout_empire_city[] = {
    { 0, 0, EMPIRE_CITY_BEFORE_RELAYOUT_SELLS },
    { offsetof(empire_city, sells), EMPIRE_CITY_BEFORE_RELAYOUT_SELLS, EMPIRE_CITY_BEFORE_RELAYOUT_RESOURCES },
    { offsetof(empire_city, cost_to_open), EMPIRE_CITY_BEFORE_RELAYOUT_COST_TO_OPEN, sizeof(empire_city) - offsetof(empire_city, cost_to_open) },
};

static const relayout_move relayout_player[] = {
    { 0x0000, 0x0000, 0x27D2 },
    { 0x27DE, 0x27D2, 0x3C },
    { 0x2826, 0x280E, 0x3C },
    { 0x286E, 0x284A, 0x3C },
    { 0x28B6, 0x2886, 0x3C },
    { 0x28FE, 0x28FE, 0x2 },
    { 0x2900, 0x28C4, 0x44 },
    { 0x2944, 0x2930, 0x28 },
    { 0x296C, 0x2980, 0xC },
    { 0x2978, 0x2978, 0x110 },
    { 0x2A88, 0x2A9C, 0x78 },
    { 0x2B18, 0x2B14, 0x114C },
    { 0x3C64, 0x3C64, 0x140 },
    { 0x3DA4, 0x3C60, 0x1E0 },
    { 0x3F84, 0x3F80, 0x35C },
    { 0x42E4, 0x42E0, 0x1B8 },
    { 0x44A0, 0x449C, 0x28 },
    { 0x44C8, 0x44C8, 0x8 },
    { 0x44D0, 0x44CC, 0x190 },
    { 0x4660, 0x4660, 0x7 },
    { 0x4667, 0x4663, 0x1 },
    { 0x4668, 0x4668, 0x4 },
    { 0x466C, 0x4668, 0xC4 },
    { 0x4730, 0x4730, 0x88 },
    { 0x47B8, 0x472C, 0x5C },
    { 0x4814, 0x4810, 0xC4 },
    { 0x48F0, 0x48D4, 0x78 },
    { 0x4980, 0x494C, 0xA },
};

static const relayout_move relayout_building_kept[] = {
    { 0x0000, 0x0000, 0xB8 },
    { 0x00B8, 0x00AC, 0x12 },
    { 0x00CA, 0x00CA, 0x2 },
    { 0x00CC, 0x00C0, 0x34 },
};

static const relayout_move relayout_building_moved[] = {
    { 0x0000, 0x0000, 0x92 },
    { 0x009E, 0x0092, 0xD },
    { 0x00AB, 0x00AB, 0x1 },
    { 0x00AC, 0x00A0, 0x1E },
    { 0x00CA, 0x00CA, 0x2 },
    { 0x00CC, 0x00C0, 0x34 },
};

static const relayout_move relayout_storage[] = {
    { 0x0000, 0x0000, 0x28 },
    { 0x002E, 0x0028, 0x42 },
};

typedef struct {
    const relayout_move *moves;
    size_t count;
} relayout_table;

static const relayout_table relayout_empire_city_table = { relayout_empire_city, sizeof(relayout_empire_city) / sizeof(relayout_empire_city[0]) };
static const relayout_table relayout_player_table = { relayout_player, sizeof(relayout_player) / sizeof(relayout_player[0]) };
static const relayout_table relayout_building_kept_table = { relayout_building_kept, sizeof(relayout_building_kept) / sizeof(relayout_building_kept[0]) };
static const relayout_table relayout_building_moved_table = { relayout_building_moved, sizeof(relayout_building_moved) / sizeof(relayout_building_moved[0]) };
static const relayout_table relayout_storage_table = { relayout_storage, sizeof(relayout_storage) / sizeof(relayout_storage[0]) };

static void relayout(void *record, int32_t size, const relayout_table *table)
{
    uint8_t *bytes = record;
    uint8_t relaid[RELAYOUT_MAX];
    memset(relaid, 0, (size_t) size);
    for (size_t i = 0; i < table->count; i++) {
        const relayout_move *move = &table->moves[i];
        memcpy(relaid + move->dst, bytes + move->src, move->size);
    }
    memcpy(bytes, relaid, (size_t) size);
}

void game_file_relayout_empire_cities(void)
{
    for (int32_t i = 0; i < EMPIRE_CITY_COUNT; i++) {
        relayout(&empire_cities[i], sizeof(empire_city), &relayout_empire_city_table);
    }
}

void game_file_relayout_players(void)
{
    for (int32_t p = 0; p < CITY_PLAYERS_RELAID; p++) {
        relayout(city_player_get(p), sizeof(city_player), &relayout_player_table);
    }
}

static int32_t building_keeps_layout_before_relayout(int32_t type)
{
    return ((type >= BUILDING_HOUSE_CRUDE_HUT) && (type <= BUILDING_HOUSE_PALATIAL_ESTATE)) || (type == BUILDING_PYRAMID) || (type == BUILDING_SPHINX);
}

void game_file_relayout_buildings(void)
{
    for (int32_t id = 0; id < MAX_BUILDINGS; id++) {
        building *b = building_get(id);
        if (building_keeps_layout_before_relayout(b->type)) {
            relayout(b, sizeof(building), &relayout_building_kept_table);
        } else {
            relayout(b, sizeof(building), &relayout_building_moved_table);
        }
    }
}

void game_file_relayout_storages(void)
{
    for (int32_t i = 0; i < STORAGE_RECORDS; i++) {
        relayout(storage_get(i), sizeof(building_storage), &relayout_storage_table);
    }
}

void game_file_upgrade_farm_fields(void)
{
    for (int32_t id = 1; id < MAX_BUILDINGS; id++) {
        if (building_is_farm(id)) {
            building *b = building_get(id);
            b->data.industry.progress = (int16_t) (FARM_PROGRESS_FACTOR * b->data.industry.progress);
        }
    }
}

void farm_update_map(void)
{
    map_soil_fertility_update_rect(0, 0, map_info.grid_width - 1, map_info.grid_height - 1, 1);
}
