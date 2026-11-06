// The Nile flood: the floodplain lists built from the map, the scenario's floodplain settings, the yearly state
// machine of the flood (imminent, rising, peak, receding, resting, idle), the flooding and drying of tiles with their
// effects on farms, roads, canals and figures, and the message that predicts the flood.

#include "building/building.h"
#include "building/building_figure_helpers.h"
#include "building/building_industry.h"
#include "building/building_type.h"
#include "building/construction_load.h"
#include "city/city_player.h"
#include "city/gods.h"
#include "city/message.h"
#include "core/crt_compat.h"
#include "core/random.h"
#include "core/sorted_list.h"
#include "core/static_init.h"
#include "figure/action.h"
#include "figure/figure.h"
#include "figure/figure_action.h"
#include "figure/figure_core.h"
#include "figure/figure_type.h"
#include "figure/military.h"
#include "flood/flood_data.h"
#include "flood/flood_internal.h"
#include "game/game_data.h"
#include "game/game_internal.h"
#include "map/farm.h"
#include "map/image_context.h"
#include "map/map_data.h"
#include "map/routing_terrain.h"
#include "map/terrain.h"
#include "map/tiles.h"
#include "map/tiles_road.h"
#include "map/water.h"
#include "map/water_supply.h"
#include "scenario/invasion.h"
#include "widget/widget_data.h"
#include "window/window_data.h"

#include <stdlib.h>
#include <string.h>

#define TERRAIN_FLOOD_BUILD_CLEARED (TERRAIN_DUNE | TERRAIN_MARSH | TERRAIN_MEADOW)
#define TERRAIN_DRY_CLEARED (TERRAIN_DEEPWATER | TERRAIN_WATER)
#define TERRAIN_FLOODPLAIN_OR_DIKE (TERRAIN_FLOODPLAIN | TERRAIN_DIKE)

#define TICKS_PER_YEAR 9792.0

enum flood_limits {
    FLOOD_NO_LIST = FLOOD_LIST_COUNT,
    FLOOD_NEAR_STATE_UNITS = 28,
    OSIRIS_COMPLEX_QUALITY_BONUS = 10,
    FLOOD_QUALITY_MAX = 100,
    FLOOD_QUALITY_PER_MESSAGE = 20,
};

static const int32_t ADJACENT_OFFSETS[8] = { -GRID_SIZE, -GRID_SIZE + 1, 1, GRID_SIZE + 1, GRID_SIZE, GRID_SIZE - 1, -1, -GRID_SIZE - 1 };

void floodplain_settings_reset(floodplain_settings *s)
{
    s->season = 180;
    s->duration = 70;
    s->duration_initial = 70;
    s->season_initial = 180;
    s->quality_next = 100;
    s->quality = 100;
    s->quality_initial = 100;
    s->reserved_1 = 0;
}

void floodplain_settings_init(floodplain_settings *s)
{
    s->season = s->season_initial;
    s->duration = s->duration_initial;
    s->quality = s->quality_initial;
    s->quality_next = s->quality_initial;
    s->reserved_1 = 0;
}

void floodplain_settings_randomize_next_quality(floodplain_settings *s)
{
    static const int32_t steps[11][2] = {
        { 2, 100 },
        { 3, 80 },
        { 5, 60 },
        { 10, 40 },
        { 15, 20 },
        { 30, 0 },
        { 15, -20 },
        { 10, -40 },
        { 5, -60 },
        { 3, -80 },
        { 2, -100 },
    };
    random_generate_next();
    int32_t roll = random_generator_state.random1_15bit % 99 + 1;
    int32_t delta = 9999;
    int32_t sum = 0;
    for (int32_t i = 0; delta == 9999 && i < 11; i++) {
        sum += steps[i][0];
        if (roll < sum) {
            delta = steps[i][1];
        }
    }
    int32_t quality = s->quality_next + delta;
    if (quality >= 100) {
        quality = 100;
    }
    s->quality_next = quality > 0 ? quality : 0;
}

void floods_update_next_flood_params(floodplain_settings *s)
{
    s->duration = s->duration_initial;
    s->season = s->season_initial;
    s->quality_next = s->quality_initial;
    s->quality_last = floodplain_settings_get_quality(s);
    floodplain_settings_randomize_next_quality(s);
}

void floodplain_settings_reset_season(floodplain_settings *s)
{
    s->season = s->season_initial;
    s->duration = s->duration_initial;
}

int32_t floodplain_settings_get_quality(floodplain_settings *s)
{
    const city_player *player = city_player_current();
    int32_t quality = s->quality;
    if (player->has_temple_complex && building_temple_god(player->temple_complex_building_id) == GOD_OSIRIS) {
        quality += OSIRIS_COMPLEX_QUALITY_BONUS;
    }
    return quality > FLOOD_QUALITY_MAX ? FLOOD_QUALITY_MAX : quality;
}

void floodplain_settings_reset_next_quality(floodplain_settings *s)
{
    s->quality_next = s->quality_initial;
}

void floods_adjust_next_quality(floodplain_settings *s, int32_t delta)
{
    s->quality_next += delta;
    if (s->quality_next < 0) {
        s->quality_next = 0;
    } else if (s->quality_next > 100) {
        s->quality_next = 100;
    }
}

void flood_tile_init(
    flood_tile *tile, int32_t grid_offset, int32_t x, int32_t y, int32_t depth, flood_list *list, int32_t is_floodplain)
{
    tile->grid_offset = grid_offset;
    tile->x = x;
    tile->y = y;
    tile->depth = depth;
    tile->link = 0;
    tile->list = list;
    tile->is_floodplain = (uint8_t) is_floodplain;
}

void flood_list_construct(flood_list *list)
{
    list->links.head = 0;
    list->links.tail = 0;
    list->count = 0;
    list->cursor = 0;
}

void flood_list_clear(flood_list *list)
{
    if (!list->count) {
        return;
    }
    sorted_link *link = list->links.head;
    flood_tile *next_tile = 0;
    if (link) {
        next_tile = link->item;
        link = link->next;
    }
    list->cursor = next_tile;
    while (list->cursor) {
        if (link) {
            next_tile = link->item;
            link = link->next;
        } else {
            next_tile = 0;
        }
        sorted_list_unlink(&list->links, list->cursor->link);
        free(list->cursor->link);
        free(list->cursor);
        list->cursor = next_tile;
    }
    list->count = 0;
}

void flood_list_destruct(flood_list *list)
{
    flood_list_clear(list);
}

flood_tile *flood_list_insert(flood_list *list, int32_t grid_offset, int32_t x, int32_t y, int32_t depth, int32_t is_floodplain)
{
    flood_tile *tile = malloc(sizeof(flood_tile));
    flood_tile_init(tile, grid_offset, x, y, depth, list, is_floodplain);
    sorted_link *link = malloc(sizeof(sorted_link));
    link->key = map_grids.random[grid_offset];
    link->item = tile;
    sorted_list_insert(&list->links, link);
    tile->link = link;
    list->count++;
    return tile;
}

static flood_tile *next_tile_of(const flood_tile *tile)
{
    sorted_link *next = tile->link->next;
    return next ? next->item : 0;
}

void flood_list_process(flood_list *list, bool flooding, int32_t percent, bool all)
{
    int32_t count = list->count;
    if (!count) {
        return;
    }
    if (!list->cursor) {
        list->cursor = list->links.head ? list->links.head->item : 0;
        if (list->cursor && percent) {
            int32_t skip = (percent < 0 ? 0 : percent) * count / 100;
            while (skip-- != 0) {
                list->cursor = next_tile_of(list->cursor);
                if (!list->cursor) {
                    break;
                }
            }
        }
    }
    flood_tile *tile = list->cursor;
    if (!tile) {
        return;
    }
    int32_t n = count;
    if (!all) {
        n = (int32_t) (count * 100.0 / (FLOOD_TICKS_PER_UNIT * 100.0)) + 1;
        if (n < 1) {
            n = 1;
        }
    }
    while (n-- != 0) {
        if (tile->is_floodplain) {
            flood_tile_apply(tile, flooding);
        }
        list->cursor = next_tile_of(list->cursor);
        tile = list->cursor;
        if (!tile) {
            break;
        }
    }
}

static void push_boat_off_tile(figure *f, int32_t figure_id, int32_t grid_offset)
{
    static const int32_t moves[8][3] = {
        { 0, -1, -GRID_SIZE },
        { 1, -1, -GRID_SIZE + 1 },
        { 1, 0, 1 },
        { 1, 1, GRID_SIZE + 1 },
        { 0, 1, GRID_SIZE },
        { -1, 1, GRID_SIZE - 1 },
        { -1, 0, -1 },
        { -1, -1, -GRID_SIZE - 1 },
    };
    map_tile_cursor_offset = grid_offset;
    map_image_context_set_tiles(TERRAIN_WATER);
    int32_t best_length = 0;
    int32_t best_start = -1;
    int32_t run_length = 0;
    int32_t run_start = -1;
    for (int32_t i = 0; i < 16; i++) {
        if (map_adjacent_tiles[i & 7]) {
            if (run_start < 0) {
                run_start = i;
            }
            run_length++;
            if (i != 15) {
                continue;
            }
        }
        if (run_length > best_length) {
            best_start = run_start;
            best_length = run_length;
            run_start = -1;
            run_length = 0;
        }
    }
    const int32_t *move = moves[(best_length / 2 + best_start) & 7];
    map_figure_delete(figure_id);
    f->cross_country.x += (int16_t) (move[0] * 32);
    f->x += (int16_t) (move[0] * 2);
    f->cross_country.y += (int16_t) (move[1] * 32);
    f->y += (int16_t) (move[1] * 2);
    f->grid_offset += move[2] * 2;
    map_figure_add(figure_id);
}

static int32_t is_boat_type(int32_t type)
{
    return type == FIGURE_TRADE_SHIP || type == FIGURE_FISHING_BOAT || type == FIGURE_TRANSPORT_SHIP || type == FIGURE_WARSHIP ||
        type == FIGURE_ENEMY_EGYPTIAN_WARSHIP || type == FIGURE_ENEMY_EGYPTIAN_TRANSPORT_SHIP;
}

static void restore_farm_tiles(building *b)
{
    int32_t offset = map_info.grid_start + b->y * GRID_SIZE + b->x;
    for (int32_t row = 0; row < 3; row++, offset += GRID_SIZE) {
        map_grids.bitfields[offset] = 2;
        map_grids.bitfields[offset + 1] = 2;
        map_grids.bitfields[offset + 2] = 2;
        for (int32_t col = 0; col < 3; col++) {
            map_grids.terrain[offset + col] |= TERRAIN_BUILDING;
        }
    }
}

static void flood_farm(building *b, const flood_tile *tile)
{
    int32_t offset = map_info.grid_start + b->y * GRID_SIZE + b->x;
    int32_t whole = 1;
    for (int32_t row = 0; whole && row < 3; row++) {
        for (int32_t col = 0; col < 3; col++, offset++) {
            uint32_t terrain = map_grids.terrain[offset];
            if (terrain & TERRAIN_BUILDING) {
                map_grids.bitfields[offset] = 0;
                map_grids.terrain[offset] = terrain & ~TERRAIN_BUILDING;
            } else {
                whole = 0;
            }
        }
        offset += GRID_SIZE - 3;
    }
    b->data.industry.progress = 0;
    b->num_workers = 0;
    b->data.industry.worker_count = 0;
    b->data.industry.labor_days_left[0] = 0;
    map_tiles_update_region_floodplain(tile->x - 2, tile->y - 2, tile->x + 2, tile->y + 2);
    map_tiles_update_region_water(tile->x - 2, tile->y - 2, tile->x + 2, tile->y + 2, 0);
}

static void flood_one_tile(flood_tile *tile, int32_t grid_offset)
{
    uint32_t terrain = map_grids.terrain[grid_offset];
    map_grids.sprite[grid_offset] = 0;
    terrain = (terrain & ~TERRAIN_RUBBLE) | TERRAIN_WATER;
    map_grids.terrain[grid_offset] = terrain;
    uint8_t fertility = tile->is_floodplain ? (uint8_t) (int32_t) (tile->list->depth * (1.0 / 30.0) * 99.0) : 0;
    int32_t building_id = map_grids.building_ids[grid_offset];
    map_grids.soil_fertility[grid_offset] = fertility;
    if (terrain & TERRAIN_ROAD) {
        map_grids.terrain[grid_offset] = (terrain & ~TERRAIN_ROAD) | TERRAIN_SUBMERGED_ROAD;
    }
    int32_t x = tile->x;
    int32_t y = tile->y;
    map_routing_update_water_rect(x - 1, y - 1, x + 1, y + 1);
    map_routing_update_land_citizen_rect(x - 1, y - 1, x + 1, y + 1, 0);
    map_routing_update_land_noncitizen_rect(x - 1, y - 1, x + 1, y + 1);
    map_tiles_update_region_floodplain(x - 1, y - 1, x + 1, y + 1);
    map_tiles_update_region_water(x - 2, y - 2, x + 2, y + 2, 0);
    map_tiles_update_region_roads(x - 2, y - 2, x + 2, y + 2);
    map_tiles_update_region_canals(x - 2, y - 2, x + 2, y + 2, 0);
    if ((terrain & TERRAIN_BUILDING) && building_id) {
        building *b = building_get(building_id);
        int32_t type = b->type;
        if (type != BUILDING_BURNING_RUIN && type != BUILDING_NATIVE_HUT && type != BUILDING_NATIVE_MEETING && type != BUILDING_NATIVE_CROPS &&
            type != BUILDING_ARCHITECT_POST && type != BUILDING_WATER_LIFT && type != BUILDING_ROADBLOCK && b->state != BUILDING_STATE_DELETED) {
            city_player *player = city_player_current();
            if (!building_is_farm(building_id)) {
                b->state = BUILDING_STATE_DELETED;
                b->is_deleted = 1;
            } else if (player->osiris_flood_destroys_farms) {
                player->osiris_flood_destroys_farms = 2;
                b->state = BUILDING_STATE_DELETED;
            } else {
                flood_farm(b, tile);
            }
        }
    }
    int32_t figure_id = map_grids.figure_ids[grid_offset];
    while (figure_id) {
        figure *f = figure_get(figure_id);
        if (f->next_figure_id_on_same_tile == figure_id) {
            break;
        }
        if (f->type != FIGURE_STANDARD_BEARER && f->type != FIGURE_BUBBLES) {
            figure_set_state_dead(figure_id);
        }
        figure_id = f->next_figure_id_on_same_tile;
    }
}

static void dry_one_tile(flood_tile *tile, int32_t grid_offset)
{
    int32_t building_id = map_grids.building_ids[grid_offset];
    uint32_t terrain = map_grids.terrain[grid_offset];
    int32_t x = tile->x;
    int32_t y = tile->y;
    if (terrain & TERRAIN_WATER) {
        terrain &= ~TERRAIN_DRY_CLEARED;
        map_grids.terrain[grid_offset] = terrain;
        if (terrain & TERRAIN_SUBMERGED_ROAD) {
            map_grids.terrain[grid_offset] = (terrain & ~TERRAIN_SUBMERGED_ROAD) | TERRAIN_ROAD;
        }
        map_tiles_update_region_canal_images(x - 3, y - 3, x + 3, y + 3);
        map_routing_update_water_rect(x - 1, y - 1, x + 1, y + 1);
        map_routing_update_land_citizen_rect(x - 1, y - 1, x + 1, y + 1, 0);
        map_routing_update_land_noncitizen_rect(x - 1, y - 1, x + 1, y + 1);
        map_tiles_update_region_floodplain(x - 2, y - 2, x + 2, y + 2);
        map_tiles_update_region_water(x - 2, y - 2, x + 2, y + 2, 0);
        map_tiles_update_region_roads(x - 3, y - 3, x + 3, y + 3);
        map_tiles_update_region_canals(x - 3, y - 3, x + 3, y + 3, 0);
        int32_t figure_id = map_grids.figure_ids[grid_offset];
        figure *f = figure_get(figure_id);
        if (f->next_figure_id_on_same_tile != figure_id) {
            while (figure_id) {
                if (is_boat_type(f->type)) {
                    push_boat_off_tile(f, figure_id, grid_offset);
                }
                figure_id = f->next_figure_id_on_same_tile;
                f = figure_get(figure_id);
                if (f->next_figure_id_on_same_tile == figure_id) {
                    break;
                }
            }
        }
    } else {
        map_tiles_update_region_floodplain(x, y, x + 1, y + 1);
        map_tiles_update_region_water(x - 1, y - 1, x + 1, y + 1, 0);
        map_tiles_update_region_roads(x - 2, y - 2, x + 2, y + 2);
        map_tiles_update_region_canals(x - 2, y - 2, x + 2, y + 2, 0);
    }
    if (building_is_farm(building_id) && !building_near_water(building_id)) {
        restore_farm_tiles(building_get(building_id));
        map_building_place_farm(building_id, 0);
    }
}

void flood_tile_apply(flood_tile *tile, bool flooding)
{
    if (flooding) {
        flood_one_tile(tile, tile->grid_offset);
    } else {
        dry_one_tile(tile, tile->grid_offset);
    }
}

void flood_list_recede_pass(flood_list *list)
{
    if (list->recede_passes < 5) {
        list->recede_passes++;
        flood_list_process(list, 0, 1, 1);
    }
}

void floods_reset(floods *self)
{
    memset(self->tiles, 0, sizeof(self->tiles));
    for (int32_t i = 0; i < FLOOD_LIST_COUNT;) {
        flood_list *list = &self->lists[i];
        flood_list_clear(list);
        i++;
        list->recede_passes = 0;
        list->depth = i;
        list->owner = self;
        list->cursor = 0;
        list->count = 0;
    }
    self->progress = FLOOD_NO_LIST;
    self->season_month = 0;
    self->has_floodplain = 0;
}

void floods_build(floods *self, int32_t x0, int32_t y0, int32_t x1, int32_t y1)
{
    static uint8_t depths[GRID_CELLS];
    floods_reset(self);
    memset(depths, 0, sizeof(depths));
    int32_t x_min = x0 <= x1 ? x0 : x1;
    int32_t x_max = x0 <= x1 ? x1 : x0;
    int32_t y_min = y0 <= y1 ? y0 : y1;
    int32_t y_max = y0 <= y1 ? y1 : y0;
    x_min--;
    x_max++;
    y_min--;
    y_max++;
    if (x_min < 0) {
        x_min = 0;
    }
    if (x_max >= map_info.grid_width) {
        x_max = map_info.grid_width - 1;
    }
    if (y_min < 0) {
        y_min = 0;
    }
    if (y_max >= map_info.grid_height) {
        y_max = map_info.grid_height - 1;
    }
    int32_t pass_depth = 30;
    int32_t changed;
    do {
        int32_t offset = map_info.grid_start + y_min * GRID_SIZE + x_min;
        changed = 0;
        for (int32_t y = y_min; y <= y_max; y++) {
            for (int32_t x = x_min; x <= x_max; x++, offset++) {
                if (depths[offset]) {
                    continue;
                }
                int32_t is_floodplain = (map_grids.terrain[offset] >> 16) & 1;
                int32_t floodplain_neighbours = map_terrain_count_neighbours_with(TERRAIN_FLOODPLAIN, offset);
                if (!is_floodplain && !floodplain_neighbours) {
                    continue;
                }
                int32_t depth = 30;
                int32_t water_neighbours = map_terrain_neighbor_count(TERRAIN_WATER, TERRAIN_FLOODPLAIN_OR_DIKE, offset);
                if (water_neighbours + floodplain_neighbours == 8 && (map_grids.terrain[offset] & (TERRAIN_FLOODPLAIN | TERRAIN_WATER)) == TERRAIN_WATER) {
                    continue;
                }
                if (!water_neighbours) {
                    depth = 0;
                    for (int32_t i = 0; i < 8; i++) {
                        int32_t next = offset + ADJACENT_OFFSETS[i];
                        if (!(map_grids.terrain[next] & TERRAIN_FLOODPLAIN)) {
                            continue;
                        }
                        int32_t d = depths[next];
                        if (d > depth && (d == 1 || d != pass_depth)) {
                            depth = d - 1;
                            if (depth < 1) {
                                depth = 1;
                            }
                        }
                    }
                    if (!depth) {
                        continue;
                    }
                }
                map_grids.terrain[offset] = (map_grids.terrain[offset] & ~TERRAIN_FLOOD_BUILD_CLEARED) | TERRAIN_GROUNDWATER;
                map_tiles_update_empty_land_around(x, y, 1);
                depths[offset] = (uint8_t) depth;
                floods_data.tiles[offset] = flood_list_insert(&floods_data.lists[depth - 1], offset, x, y, depth, is_floodplain);
                changed = 1;
                self->has_floodplain = 1;
            }
            offset += GRID_SIZE - (x_max - x_min + 1);
        }
        pass_depth--;
    } while (changed);
    self->width = 0;
    for (int32_t i = FLOOD_LIST_COUNT - 1; i >= 0 && self->lists[i].count; i--) {
        self->width++;
    }
    map_tiles_update_all();
}

void floods_prepare_next_flood(floods *self)
{
    floods_update_next_flood_params(&self->settings);
}

void floods_move_boat_to_water(int32_t figure_id)
{
    figure *f = figure_get(figure_id);
    if (routing_grids.water[f->grid_offset] >= 0) {
        return;
    }
    int32_t x = f->x;
    int32_t y = f->y;
    map_water_find_free_tile(&x, &y);
    int32_t dx = x - f->x;
    int32_t dy = y - f->y;
    if (!dx && !dy) {
        return;
    }
    map_figure_delete(figure_id);
    f->cross_country.x += (int16_t) (dx * 16);
    f->x = (int16_t) x;
    f->cross_country.y += (int16_t) (dy * 16);
    f->y = (int16_t) y;
    f->grid_offset += dx + dy * GRID_SIZE;
    map_figure_add(figure_id);
}

void floods_end(floods *self)
{
    floodplain_settings_reset_season(&self->settings);
    for (int32_t id = 1; id < MAX_FIGURES; id++) {
        figure *f = figure_get(id);
        switch (f->type) {
            case FIGURE_TRADE_SHIP:
                if (f->action_state == FIGURE_ACTION_TRADE_SHIP_DOCKED) {
                    break;
                }
            case FIGURE_FISHING_BOAT:
                if (f->action_state == FIGURE_ACTION_FISHING_BOAT_AT_WHARF) {
                    break;
                }
            case FIGURE_TRANSPORT_SHIP:
            case FIGURE_WARSHIP:
            case FIGURE_ENEMY_TRANSPORT_SHIP:
            case FIGURE_ENEMY_WARSHIP:
            case FIGURE_ENEMY_EGYPTIAN_WARSHIP:
            case FIGURE_ENEMY_EGYPTIAN_TRANSPORT_SHIP:
                floods_move_boat_to_water(id);
                break;
        }
    }
}

void floods_post_flood_prediction_message(floods *self)
{
    if (self->state != FLOOD_STATE_RESTING || !self->has_floodplain) {
        return;
    }
    city_message_use_popup = 1;
    int32_t gods_status = *scenario_data.god_status;
    switch ((floodplain_settings_get_quality(&self->settings) + (FLOOD_QUALITY_PER_MESSAGE - 1)) / FLOOD_QUALITY_PER_MESSAGE) {
        case 0:
            city_message_post(MESSAGE_NO_INUNDATION, 0, 0, 0, 0);
            break;
        case 1:
            city_message_post(gods_status ? MESSAGE_POOR_INUNDATION : MESSAGE_POOR_INUNDATION_SEERS, 0, 0, 0, 0);
            break;
        case 2:
            city_message_post(gods_status ? MESSAGE_MEDIOCRE_INUNDATION : MESSAGE_MEDIOCRE_INUNDATION_SEERS, 0, 0, 0, 0);
            break;
        case 3:
            city_message_post(MESSAGE_GOOD_INUNDATION, 0, 0, 0, 0);
            break;
        case 4:
            city_message_post(MESSAGE_EXCELLENT_INUNDATION, 0, 0, 0, 0);
            break;
        case 5:
            city_message_post(MESSAGE_PERFECT_INUNDATION, 0, 0, 0, 0);
            break;
    }
}

static void advance_flood(floods *self)
{
    floodplain_settings *s = &self->settings;
    if (!self->has_floodplain) {
        self->state = FLOOD_STATE_IDLE;
        s->quality = 0;
        return;
    }
    if (game_time_tick == 1 && self->state != FLOOD_STATE_FLOODING) {
        if (s->quality > s->quality_next) {
            int32_t quality = s->quality - 5;
            s->quality = quality > s->quality_next ? quality : s->quality_next;
        } else if (s->quality < s->quality_next) {
            int32_t quality = s->quality + 5;
            s->quality = quality < s->quality_next ? quality : s->quality_next;
        }
    }
    int32_t years = game_time_year - scenario_data.start.year;
    int32_t width = self->width;
    int32_t start = ((s->season * 105) / 100) + (int32_t) (years * floods_years_to_units) + 15;
    int32_t end = s->duration + width * 2 + start;
    int32_t rise = (int32_t) (((double) s->quality_last * width) * 0.01);
    int32_t now = (int32_t) (years * TICKS_PER_YEAR) + (game_time_month * DAYS_PER_MONTH + game_time_day) * TICKS_PER_DAY + game_time_tick;
    double now_ticks = now;
    int32_t previous_unit = (int32_t) ((now_ticks - 1.0) / FLOOD_TICKS_PER_UNIT);
    int32_t unit = (int32_t) (now_ticks / FLOOD_TICKS_PER_UNIT);
    if (floods_years_to_units * 0.5 < (double) (start - unit)) {
        int32_t year_units = (int32_t) floods_years_to_units;
        start -= year_units;
        end -= year_units;
    }
    self->season_month = s->season / 30;
    int32_t to_start = start - unit;
    int32_t since_end = unit - end;
    if (unit >= start && unit < end) {
        int32_t progress;
        if (unit < rise + start) {
            self->state = FLOOD_STATE_FLOODING;
            progress = to_start + 30;
        } else if (unit >= end - rise) {
            self->state = FLOOD_STATE_RECEDING;
            progress = since_end + 31;
        } else {
            self->state = FLOOD_STATE_PEAK;
            progress = 30 - rise;
        }
        progress--;
        if (progress > 30) {
            progress = 30;
        } else if (progress < 0) {
            progress = 0;
        }
        self->progress = progress;
    } else {
        self->progress = FLOOD_NO_LIST;
        if (to_start >= 0 && to_start < FLOOD_NEAR_STATE_UNITS) {
            self->state = FLOOD_STATE_IMMINENT;
        } else if (since_end >= 0 && since_end < FLOOD_NEAR_STATE_UNITS) {
            self->state = FLOOD_STATE_RESTING;
        } else {
            self->state = FLOOD_STATE_IDLE;
        }
    }
    if (unit != previous_unit) {
        if (unit < start || unit >= end) {
            if (to_start == 49) {
                building_farm_food_warning();
            } else if (to_start == 1) {
                floods_prepare_next_flood(self);
            } else if (since_end == 1) {
                floods_end(self);
            }
        } else if (unit < rise + start || unit >= end - rise) {
            city_health_recompute_distances = 1;
        } else if (unit == start + width) {
            for (int32_t i = 0; i < FLOOD_LIST_COUNT; i++) {
                self->lists[i].recede_passes = 0;
            }
        }
    }
    if (unit >= end - width) {
        int32_t slot_unit = width + since_end;
        if (slot_unit < 0) {
            slot_unit += (int32_t) floods_years_to_units;
        }
        int32_t slot = (int32_t) (now_ticks / (FLOOD_TICKS_PER_UNIT * (1.0 / 6.0))) % 6 + slot_unit * 6;
        int32_t slots = width * 60;
        int32_t chosen = slots < slot ? slots : slot;
        if (slot < slots && chosen != floods_last_recede_slot) {
            floods_last_recede_slot = chosen;
            int32_t back = chosen % width;
            if (back >= width - 1) {
                back = width - 1;
            }
            int32_t list = self->progress - back;
            if (list >= 0 && list < self->progress) {
                flood_list_recede_pass(&self->lists[list]);
            }
        }
        floods_recede_slot = slot;
    }
    int32_t percent = now % (int32_t) FLOOD_TICKS_PER_UNIT * 100 / (int32_t) FLOOD_TICKS_PER_UNIT;
    if (self->state == FLOOD_STATE_FLOODING) {
        flood_list_process(&self->lists[0] + self->progress, 1, percent, 0);
        undo_available = 0;
    } else if (self->state == FLOOD_STATE_RECEDING) {
        flood_list_process(&self->lists[0] + self->progress, 0, percent, 0);
        undo_available = 0;
    }
}

void floods_tick_update(floods *self)
{
    advance_flood(self);
}

static void fill_canal_at(int32_t grid_offset)
{
    if ((map_grids.terrain[grid_offset] & TERRAIN_CANAL) && !map_grids.canals[grid_offset]) {
        map_water_supply_fill_canals_from_offset(grid_offset);
    }
}

void floods_fill_canals(floods *self)
{
    const flood_list *list = self->progress < FLOOD_NO_LIST ? &self->lists[self->progress] : &self->lists[FLOOD_LIST_COUNT - 1];
    if (list->count) {
        for (sorted_link *link = list->links.head; link;) {
            flood_tile *tile = link->item;
            link = link->next;
            if (!tile) {
                break;
            }
            fill_canal_at(tile->grid_offset);
        }
    }
    for (int32_t i = 1; i <= 2; i++) {
        if (self->progress + i >= FLOOD_NO_LIST || !self->lists[self->progress + i].count) {
            continue;
        }
        for (sorted_link *link = self->lists[self->progress + i].links.head; link;) {
            flood_tile *tile = link->item;
            link = link->next;
            if (!tile) {
                break;
            }
            flood_tile_fill_canal(tile);
        }
    }
}

void flood_tile_fill_canal(flood_tile *tile)
{
    fill_canal_at(tile->grid_offset);
}

int32_t floods_get_month_before(floods *self)
{
    int32_t month = self->season_month - 1;
    if (month < 0) {
        month += 12;
    }
    return month;
}

void floods_worsen_next_quality(floods *self)
{
    int32_t r = crt_rand() % 8;
    floods_adjust_next_quality(&self->settings, (-3 - r) * 10);
}

void floods_improve_next_quality(floods *self)
{
    int32_t r = crt_rand() % 8;
    floods_adjust_next_quality(&self->settings, (r + 3) * 10);
}

void floods_construct(floods *self)
{
    floodplain_settings_reset(&self->settings);
    for (int32_t i = 0; i < FLOOD_LIST_COUNT; i++) {
        flood_list_construct(&self->lists[i]);
    }
}

void floods_static_init_units(void)
{
    floods_years_to_units = TICKS_PER_YEAR / FLOOD_TICKS_PER_UNIT;
}

