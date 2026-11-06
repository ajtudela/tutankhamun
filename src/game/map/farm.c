// Farms on the map.
//
// Placing a farm on its tiles, the average fertility of a rectangle, a farm's fertility and expected produce, the
// monthly warning that the granaries cannot hold the floodplain harvest, and the drawing of crop farms on the meadow
// and on the floodplain (crops by growth progress, and the worker sprite chosen from the flood's progress).
//
// Two drawing oddities stay: a floodplain farm beside water is not drawn when it is placed, and in a view orientation
// other than 0, 2, 4 and 6 the floodplain worker is looked up at grid offset 0.

#include "map/farm.h"

#include "building/building.h"
#include "building/building_image_group.h"
#include "building/building_industry.h"
#include "building/building_type.h"
#include "city/city_player.h"
#include "city/message.h"
#include "city/resource_type.h"
#include "figure/image_groups_workers.h"
#include "flood/flood_types.h"
#include "game/game_data.h"
#include "graphics/image_collection.h"
#include "graphics/image_draw.h"
#include "map/building_tiles.h"
#include "map/grid.h"
#include "map/map_data.h"
#include "map/soil.h"
#include "map/terrain.h"
#include "map/tiles_internal.h"
#include "map/water.h"
#include "widget/widget_data.h"

#include <stdint.h>

static const double FARM_PRODUCE_PER_WORKER = 0.16;
static const double FARM_FERTILITY_SCALE = 1.0 / 99;
static const double FARM_PRODUCE_SCALE = 10.0;
static const double FARM_SOWING_WORKER_SLOTS = 9.0;

void map_building_place_farm(int32_t building_id, bool extra)
{
    building *b = building_get(building_id);
    map_building_tiles_add_farm(building_id, b->x, b->y, extra);
}

int32_t map_fertility_average(int32_t x, int32_t y, int32_t width, int32_t height)
{
    int32_t origin = map_info.grid_start + (y * GRID_SIZE) + x;
    map_tile_cursor_offset = origin;
    double divisor = 0.0;
    int32_t sum = 0;
    if (height > 0) {
        for (int32_t row = 0; row < height; row++) {
            if (width > 0) {
                for (int32_t col = 0; col < width; col++) {
                    int32_t tile = origin + building_square_tile_offset(row, col);
                    divisor += 1.0;
                    sum += map_grids.soil_fertility[tile] + 1;
                }
            }
        }
    }
    return (int32_t) ((double) sum / divisor) + 1;
}

int32_t building_farm_fertility(int32_t building_id)
{
    building *b = building_get(building_id);
    int32_t value = map_fertility_average(b->x, b->y, FARM_SIZE, FARM_SIZE);
    if (b->has_water_access && building_is_crop_farm(building_id)) {
        if (building_is_on_floodplain(building_id)) {
            value += FARM_FERTILITY_FLOODPLAIN_WATER_BONUS;
        } else {
            value += FARM_FERTILITY_MEADOW_WATER_BONUS;
        }
    }
    return (value > SOIL_FERTILITY_MAX) ? SOIL_FERTILITY_MAX : value;
}

int32_t building_farm_expected_produce(int32_t building_id, int32_t use_model)
{
    building *b = building_get(building_id);
    int32_t workers = use_model ? building_model_table[b->type].laborers : (int32_t) b->num_workers;
    int32_t fertility = building_farm_fertility(building_id);
    double value = (double) fertility * (FARM_PRODUCE_PER_WORKER * FARM_FERTILITY_SCALE);
    value *= (double) workers;
    value *= FARM_PRODUCE_SCALE;
    if (b->data.industry.labor_days_left[2] && building_is_farm(building_id) && (b->type == BUILDING_BARLEY_FARM || b->type == BUILDING_GRAIN_FARM)) {
        value += value;
    }
    int32_t result = (int32_t) value;
    return (result < 1) ? 1 : result;
}

static int32_t farm_crop_group(int32_t type)
{
    switch (type) {
        case BUILDING_BARLEY_FARM:
            return IMAGE_GROUP_BARLEY_CROPS;
        case BUILDING_FLAX_FARM:
            return IMAGE_GROUP_FLAX_CROPS;
        case BUILDING_GRAIN_FARM:
            return IMAGE_GROUP_GRAIN_CROPS;
        case BUILDING_LETTUCE_FARM:
            return IMAGE_GROUP_LETTUCE_CROPS;
        case BUILDING_POMEGRANATES_FARM:
            return IMAGE_GROUP_POMEGRANATES_CROPS;
        case BUILDING_CHICKPEAS_FARM:
            return IMAGE_GROUP_CHICKPEAS_CROPS;
        case BUILDING_FIG_FARM:
            return IMAGE_GROUP_FIG_CROPS;
        case BUILDING_HENNA_FARM:
            return IMAGE_GROUP_HENNA_CROPS;
        default:
            return -1;
    }
}

void map_building_tiles_add_farm(int32_t building_id, int32_t x, int32_t y, bool extra)
{
    building *b = building_get(building_id);
    if (b->type == BUILDING_REED_GATHERER) {
        map_building_tiles_add(building_id, x, y, REED_GATHERER_SIZE, REED_GATHERER_SIZE, image_group(IMAGE_GROUP_REED_GATHERER), TERRAIN_BUILDING);
        return;
    }
    if (b->type == BUILDING_CATTLE_RANCH) {
        map_building_tiles_add(building_id, x, y, FARM_SIZE, FARM_SIZE, image_group(IMAGE_GROUP_CATTLE_RANCH), TERRAIN_BUILDING);
        return;
    }
    int32_t group = -1;
    if (building_is_farm(building_id)) {
        group = farm_crop_group(b->type);
    }
    if (group < 0) {
        return;
    }
    int32_t image = image_group(group);
    int32_t parts = (int32_t) b->data.industry.progress / FARM_PROGRESS_PER_PART;
    if (building_is_on_floodplain(building_id)) {
        if (!building_near_water(building_id)) {
            map_farm_tiles_floodplain(building_id, x, y, image, parts, extra);
        }
        return;
    }
    map_farm_tiles_meadow(building_id, x, y, image, parts, extra);
}

void building_farm_food_warning(void)
{
    const city_player *player = city_player_current();
    int32_t produced = 0;
    int32_t stored = 0;
    for (int32_t id = 1; id < MAX_BUILDINGS; id++) {
        building *b = building_get(id);
        if (b->state != BUILDING_STATE_IN_USE) {
            continue;
        }
        if (building_is_floodplain_farm(id)) {
            int32_t amount = (building_farm_expected_produce(id, 1) + farm_growth_percentage_of(id)) * FARM_FOOD_WARNING_PRODUCE_FACTOR;
            if (amount < FARM_FOOD_WARNING_MIN_PRODUCE) {
                amount = FARM_FOOD_WARNING_MIN_PRODUCE;
            }
            if (b->type == BUILDING_BARLEY_FARM || b->type == BUILDING_FLAX_FARM || b->type == BUILDING_HENNA_FARM) {
                continue;
            }
            if (player->osiris_double_farm_yield != 0) {
                amount += amount;
            }
            produced += amount;
        } else if (b->type == BUILDING_GRANARY || b->type == BUILDING_GRANARY_UP) {
            stored += b->data.granary.resource_stored[RESOURCE_NONE];
        }
    }
    int32_t gap = produced - stored;
    if (gap > FARM_FOOD_WARNING_GRANARIES_GAP) {
        city_warning_show(WARNING_BUILD_GRANARIES);
    } else if (gap > FARM_FOOD_WARNING_GRANARY_GAP) {
        city_warning_show(WARNING_BUILD_GRANARY);
    }
}

static int32_t image_table_index(int32_t image)
{
    return image < image_collection.image_count ? image : 0;
}

static void draw_isometric(int32_t image, int32_t pixel_x, int32_t pixel_y)
{
    int32_t indexed = image_table_index(image);
    pixel_y += (city_view.tile_height * 2) - image_record_height(indexed);
    image_draw(image, pixel_x, pixel_y);
}

void map_farm_tiles_meadow(int32_t building_id, int32_t x, int32_t y, int32_t image, int32_t parts, bool extra)
{
    if (!extra) {
        int32_t dry = farm_has_water_access_of(building_id) == 0;
        map_building_tiles_add(building_id, x, y, FARM_SIZE, FARM_SIZE, image_group(IMAGE_GROUP_MEADOW_FARM) + dry, TERRAIN_BUILDING);
        return;
    }
    static const int32_t meadow_x[5] = { -2, -1, 0, 1, 2 };
    static const int32_t meadow_y[5] = { 0, 1, 2, 1, 0 };
    static const int32_t meadow_order[5] = { 0, 4, 1, 3, 2 };
    int32_t step = parts / 10;
    int32_t crops[5];
    for (int32_t i = 0; i < 5; i++) {
        crops[i] = image + (step < 4 ? step : 5);
        step -= 4;
        if (step < 0) {
            step = 0;
        }
    }
    for (int32_t i = 0; i < 5; i++) {
        int32_t part = meadow_order[i];
        draw_isometric(crops[part], x + (meadow_x[part] * city_view.tile_half_width), y + (meadow_y[part] * city_view.tile_half_height));
    }
}

static int32_t flood_farm_tile(building *b)
{
    int32_t grid = map_info.grid_start + (b->y * GRID_SIZE) + b->x;
    int32_t size = b->size;
    int32_t delta;
    switch (map_info.orientation) {
        case DIRECTION_TOP:
            delta = building_square_tile_offset(size - 1, 0);
            break;
        case DIRECTION_RIGHT:
            delta = building_square_tile_offset(0, 0);
            break;
        case DIRECTION_BOTTOM:
            delta = building_square_tile_offset(0, size - 1);
            break;
        case DIRECTION_LEFT:
            delta = building_square_tile_offset(size - 1, size - 1);
            break;
        default:
            return 0;
    }
    return grid + delta;
}

void map_farm_tiles_floodplain(int32_t building_id, int32_t x, int32_t y, int32_t image, int32_t parts, bool extra)
{
    if (!extra) {
        int32_t fertility = map_fertility_average(x, y, FARM_SIZE, FARM_SIZE);
        map_building_tiles_add(building_id, x, y, FARM_SIZE, FARM_SIZE, (fertility / 14) + image_group(IMAGE_GROUP_FLOODPLAIN_FARM), TERRAIN_BUILDING);
        return;
    }
    static const int32_t flood_x[9] = { 0, 1, 2, -1, 0, 1, -2, -1, 0 };
    static const int32_t flood_y[9] = { -2, -1, 0, -1, 0, 1, 0, 1, 2 };
    building *b = building_get(building_id);
    int32_t tile = flood_farm_tile(b);
    int32_t sprite = map_grids.sprite[tile];
    int32_t scaled = 10 * parts;
    int32_t kind = FARM_WORKER_NONE;
    int32_t slot = -1;
    if (sprite != 0 && b->num_workers > 0) {
        if (scaled < (farm_max_progress / 5)) {
            kind = FARM_WORKER_GROWN;
            slot = 4;
        } else if (floods_data.state == FLOOD_STATE_IMMINENT) {
            kind = FARM_WORKER_HARVEST;
            slot = (int32_t) map_grids.random[game_time_day + tile] % 7;
        } else {
            kind = FARM_WORKER_SOWING;
            slot = (int32_t) (((double) scaled / (double) (farm_max_progress + 1)) * FARM_SOWING_WORKER_SLOTS);
        }
    }
    int32_t step = parts / 10;
    for (int32_t i = 0; i < 9; i++) {
        int32_t pixel_x = x + (flood_x[i] * city_view.tile_half_width);
        int32_t pixel_y = y + (flood_y[i] * city_view.tile_half_height);
        int32_t crop = image + (step < 5 ? step : 5);
        step -= 2;
        if (step < 0) {
            step = 0;
        }
        pixel_y += (city_view.tile_height * 2) - image_record_height(image_table_index(crop));
        image_draw(crop, pixel_x, pixel_y);
        if (kind == FARM_WORKER_NONE || slot != i || city_locusts_countdown > 0) {
            continue;
        }
        int32_t frame;
        int32_t base;
        if (kind == FARM_WORKER_GROWN) {
            frame = ((game_time_tick >> 3) + building_id) & 7;
            base = image_group(IMAGE_GROUP_FIGURE_LABORER_GROWN_CROPS);
        } else if (kind == FARM_WORKER_SOWING) {
            frame = ((int32_t) map_grids.random[tile] + i) % 7;
            base = image_group(IMAGE_GROUP_FIGURE_LABORER_WORK);
        } else {
            frame = (int32_t) map_grids.random[tile] % 7;
            base = image_group(IMAGE_GROUP_FIGURE_LABORER_HARVEST);
        }
        int32_t worker = base + frame + (sprite * 8) - 8;
        int32_t indexed = image_table_index(worker);
        int32_t worker_x = pixel_x + city_view.tile_half_width - image_record_sprite_offset_x(indexed);
        int32_t worker_y = pixel_y + (city_view.tile_half_height * 3) - image_record_sprite_offset_y(indexed) - 5;
        image_draw(worker, worker_x, worker_y);
    }
}
