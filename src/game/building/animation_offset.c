// The animation frame of a building tile, asked for every animated tile drawn each frame.
//
// The frame advances on a tile without a building or when its building is working: it has workers and, by type,
// its inputs, water access, a show running, or a finished lighthouse; a well animates while some house around it
// lacks water. Reversible animations run forth and back, the others wrap to the first frame.

#include "building/animation_offset.h"
#include "building/building.h"
#include "building/building_figure_helpers.h"
#include "building/building_industry.h"
#include "building/building_type.h"
#include "building/entertainment.h"
#include "building/lighthouse.h"
#include "building/shipwright.h"
#include "city/finance.h"
#include "figure/figure_type.h"
#include "game/game_data.h"
#include "graphics/image_collection.h"
#include "map/water_supply.h"
#include <stdint.h>

static int32_t image_index(int32_t image)
{
    return (image < image_collection.image_count) ? image : 0;
}

static int32_t advance_sprite(int32_t image, int32_t grid, int32_t sprite)
{
    int32_t indexed = image_index(image);
    if (animation_should_advance[image_record_animation_speed_id(indexed)] == 0) {
        return sprite & SPRITE_FRAME_MASK;
    }
    int32_t direction = 0;
    if (image_record_animation_can_reverse(indexed) != 0) {
        direction = (sprite & SPRITE_RUNS_BACK) ? 1 : 0;
        sprite &= SPRITE_FRAME_MASK;
        if (direction == 0) {
            sprite++;
            int32_t frames = image_record_animation_frames(indexed);
            if (sprite > frames) {
                sprite = frames;
                direction = 1;
            }
        } else {
            sprite--;
            if (sprite < 1) {
                sprite = 1;
                direction = 0;
            }
        }
    } else {
        sprite++;
        if (sprite > image_record_animation_frames(indexed)) {
            sprite = 1;
        }
    }
    map_grids.sprite[grid] = (uint8_t) sprite;
    if (direction) {
        map_grids.sprite[grid] = (uint8_t) (sprite | SPRITE_RUNS_BACK);
    }
    return sprite;
}

static int32_t workers_then_advance(int32_t workers, int32_t image, int32_t grid, int32_t sprite)
{
    if (workers > 0) {
        return advance_sprite(image, grid, sprite);
    }
    return 0;
}

static int32_t special_types(int32_t type)
{
    return type == BUILDING_SENET_HOUSE || type == BUILDING_MORTUARY || type == BUILDING_SCRIBAL_SCHOOL || type == BUILDING_LIBRARY ||
        type == BUILDING_SHIPWRIGHT || type == BUILDING_JEWELER || type == BUILDING_CARPENTERS_GUILD || type == BUILDING_BRICKLAYERS_GUILD ||
        type == BUILDING_VILLAGE_PALACE_UP || type == BUILDING_TOWN_PALACE_UP || type == BUILDING_CITY_PALACE || type == BUILDING_CATTLE_RANCH ||
        type == BUILDING_PAPYRUS_MAKER || type == BUILDING_BRICKWORKS || type == BUILDING_CHARIOT_MAKER || type == BUILDING_ZOO ||
        type == BUILDING_ARTISANS_GUILD || type == BUILDING_LAMP_MAKER || type == BUILDING_PAINT_MAKER || (type >= BUILDING_BREWERY && type <= BUILDING_POTTER);
}

static int32_t listed_worker_type(int32_t type)
{
    switch (type) {
        case BUILDING_POLICE_STATION:
        case BUILDING_ARCHITECT_POST:
        case BUILDING_BAZAAR:
        case BUILDING_BAZAAR_UP:
        case BUILDING_STORAGE_YARD:
        case BUILDING_STORAGE_YARD_UP:
        case BUILDING_DOCK:
        case BUILDING_DOCK_UP:
        case BUILDING_GOLD_MINE:
        case BUILDING_COPPER_MINE:
        case BUILDING_GEMSTONE_MINE:
        case BUILDING_STONE_QUARRY:
        case BUILDING_SANDSTONE_QUARRY:
        case BUILDING_LIMESTONE_QUARRY:
        case BUILDING_GRANITE_QUARRY:
        case BUILDING_WOOD_CUTTER:
        case BUILDING_CLAY_PIT:
        case BUILDING_CONSERVATORY:
        case BUILDING_DANCE_SCHOOL:
        case BUILDING_JUGGLER_SCHOOL:
        case BUILDING_BULLFIGHT_SCHOOL:
            return 1;
        default:
            return 0;
    }
}

int32_t building_animation_offset(int32_t image, int32_t grid_offset)
{
    int32_t building_id = map_grids.building_ids[grid_offset];
    int32_t sprite = map_grids.sprite[grid_offset];
    if (building_id == 0) {
        return advance_sprite(image, grid_offset, sprite);
    }
    const building *b = building_get(building_id);
    int32_t workers = b->num_workers;
    if (building_is_floodplain_farm(building_id)) {
        return (workers > 0) ? advance_sprite(image, grid_offset, sprite) : 0;
    }
    int32_t type = b->type;
    if (type == BUILDING_LAMP_MAKER) {
        if (workers > 0 && b->loads_stored > 0 && b->second_input_stock > 0) {
            return advance_sprite(image, grid_offset, sprite);
        }
        return 0;
    }
    if (type == BUILDING_WELL) {
        int32_t water = map_houses_water_in_radius(building_id, WELL_RADIUS);
        if (water == HOUSES_WATER_ALL || water == HOUSES_WATER_NO_HOUSES) {
            return 0;
        }
        return 1;
    }
    if (type == BUILDING_WATER_SUPPLY || type == BUILDING_WATER_LIFT) {
        if (workers > 0 && b->has_water_access != 0) {
            return advance_sprite(image, grid_offset, sprite);
        }
        return 0;
    }
    int32_t palace = building_is_palace(building_id);
    if (!palace && special_types(type)) {
        if (cheat_allow_all) {
            return workers_then_advance(workers, image, grid_offset, sprite);
        }
        int32_t allow_empty = 0;
        if (type == BUILDING_SHIPWRIGHT) {
            uint32_t packed = b->data.shipyard.boat_order;
            if ((packed & BOAT_ORDER_KIND_MASK) == BOAT_ORDER_FOR_CITY && (packed & BOAT_ORDER_TYPE_MASK) == FIGURE_FISHING_BOAT) {
                allow_empty = 1;
            }
        }
        if (b->loads_stored <= 0 && !allow_empty) {
            return 0;
        }
        if (type == BUILDING_BRICKWORKS && b->second_input_stock <= 0) {
            return 0;
        }
        if (type == BUILDING_ZOO && b->second_input_stock <= 0) {
            return 0;
        }
        return workers_then_advance(workers, image, grid_offset, sprite);
    }
    if (listed_worker_type(type) || type == BUILDING_GRANARY || type == BUILDING_GRANARY_UP) {
        return workers_then_advance(workers, image, grid_offset, sprite);
    }
    if (type == BUILDING_SENET_HOUSE || (type == BUILDING_ZOO && palace)) {
        return workers_then_advance(workers, image, grid_offset, sprite);
    }
    if (type == BUILDING_BOOTH || type == BUILDING_BANDSTAND || type == BUILDING_PAVILION) {
        const building *venue = building_get(building_main(building_id));
        if (venue->num_workers <= 0) {
            return 0;
        }
        int32_t show = (type == BUILDING_BOOTH) ? SHOW_JUGGLERS : (type == BUILDING_BANDSTAND) ? SHOW_MUSICIANS : SHOW_DANCERS;
        if (venue->data.entertainment.show_days_left[show] != 0) {
            return advance_sprite(image, grid_offset, sprite);
        }
        return 0;
    }
    if (type == BUILDING_SPHINX) {
        return advance_sprite(image, grid_offset, sprite);
    }
    if (type == BUILDING_LIGHTHOUSE) {
        if (b->data.monument.stage == LIGHTHOUSE_STAGE_COMPLETE) {
            return advance_sprite(image, grid_offset, sprite);
        }
        return 0;
    }
    if (workers > 0 || building_model_table[type].laborers == 0) {
        return advance_sprite(image, grid_offset, sprite);
    }
    return 0;
}
