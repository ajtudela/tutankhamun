// Carriers: cart pushers, storage yard and granary carts, bazaar buyers with their delivery boys, homeless and
// immigrants; the choice of where a load goes, the monthly count of the loads heading for each workshop, granary and
// recruiter, and the palace or mansion a criminal heads for.
//
// The carriers read some state from the wrong figure or stop early:
// - a part taken from a granary records the amount left at the source, not the amount taken;
// - buyer_take_goods reads the slot and loads of figure_current_id;
// - a buyer that follows a leader is removed when it would buy;
// - a storage yard cart without a resource to fetch dies without an image update.

#include "figure/carriers.h"
#include "building/building.h"
#include "building/building_data.h"
#include "building/building_figure_helpers.h"
#include "building/building_industry.h"
#include "building/building_storage.h"
#include "building/building_type.h"
#include "building/house_evolve.h"
#include "building/house_population.h"
#include "building/house_size.h"
#include "city/city_player.h"
#include "city/finance.h"
#include "city/population.h"
#include "city/resource_type.h"
#include "empire/empire.h"
#include "figure/boats.h"
#include "figure/carriers_internal.h"
#include "figure/figure.h"
#include "figure/figure_action.h"
#include "figure/figure_data.h"
#include "figure/figure_image_group.h"
#include "figure/figure_type.h"
#include "figure/military.h"
#include "figure/movement.h"
#include "figure/movement_internal.h"
#include "flood/flood_types.h"
#include "graphics/image_collection.h"
#include "map/grid.h"
#include "map/map_data.h"
#include "map/road_access.h"
#include "map/routing_terrain.h"
#include "map/terrain.h"
#include "map/tiles.h"
#include <stdbool.h>
#include <stdint.h>

int32_t granary_take_resource;

static int32_t input_resource(int32_t item)
{
    switch (item) {
        case 1:
            return RESOURCE_FLAX;
        case 3:
            return RESOURCE_STONE;
        case 4:
            return RESOURCE_TIMBER;
        case 5:
            return RESOURCE_CLAY;
        case 6:
            return RESOURCE_LIMESTONE;
        case 7:
            return RESOURCE_BRICKS;
        case 8:
            return RESOURCE_WEAPONS;
        case 9:
            return RESOURCE_BARLEY;
        case 10:
            return RESOURCE_GEMS;
        case 11:
            return RESOURCE_REEDS;
        case 12:
            return RESOURCE_PAPYRUS;
        case 13:
            return RESOURCE_STRAW;
        case 14:
            return RESOURCE_CHARIOTS;
        case 15:
            return RESOURCE_GOLD;
        case 16:
            return RESOURCE_LINEN;
        case 17:
            return RESOURCE_COPPER;
        case 18:
            return RESOURCE_SANDSTONE;
        case 20:
            return RESOURCE_HENNA;
        case 21:
            return RESOURCE_MEAT;
        case 22:
            return RESOURCE_GAMEMEAT;
        case 23:
            return RESOURCE_OIL;
        case 24:
            return RESOURCE_POTTERY;
        case 25:
            return RESOURCE_PAINT;
        case 26:
            return RESOURCE_LAMPS;
    }
    return -1;
}

static int32_t takes_booked_input(int32_t type)
{
    if (type >= BUILDING_BREWERY && type <= BUILDING_POTTER) {
        return 1;
    }
    switch (type) {
        case BUILDING_SENET_HOUSE:
        case BUILDING_MORTUARY:
        case BUILDING_SCRIBAL_SCHOOL:
        case BUILDING_LIBRARY:
        case BUILDING_SHIPWRIGHT:
        case BUILDING_JEWELER:
        case BUILDING_CARPENTERS_GUILD:
        case BUILDING_BRICKLAYERS_GUILD:
        case BUILDING_VILLAGE_PALACE_UP:
        case BUILDING_TOWN_PALACE_UP:
        case BUILDING_CITY_PALACE:
        case BUILDING_CATTLE_RANCH:
        case BUILDING_PAPYRUS_MAKER:
        case BUILDING_BRICKWORKS:
        case BUILDING_CHARIOT_MAKER:
        case BUILDING_ZOO:
        case BUILDING_ARTISANS_GUILD:
        case BUILDING_LAMP_MAKER:
        case BUILDING_PAINT_MAKER:
            return 1;
    }
    return 0;
}

static void unbook_destination(const figure *f)
{
    building *b = building_get(f->destination_building_id);
    if (takes_booked_input(b->type)) {
        int32_t resource = input_resource(b->second_input_resource);
        if (b->type == BUILDING_BRICKWORKS && f->resource_id == resource) {
            b->incoming_loads--;
        } else {
            b->incoming_carts--;
        }
    } else if (b->type == BUILDING_GRANARY) {
        int16_t amount = f->resource_amount;
        b->incoming_carts--;
        b->incoming_loads = (int16_t) (b->incoming_loads - amount);
    }
}

static int32_t cart_state_for_destination(int32_t type)
{
    switch (type) {
        case BUILDING_GRANARY:
        case BUILDING_GRANARY_UP:
            return FIGURE_ACTION_CART_TO_GRANARY;
        case BUILDING_STORAGE_YARD:
        case BUILDING_STORAGE_ROOM:
        case BUILDING_STORAGE_YARD_UP:
            return FIGURE_ACTION_CART_TO_STORAGEYARD;
        case BUILDING_RECRUITER:
            return FIGURE_ACTION_CART_TO_RECRUITER;
    }
    return FIGURE_ACTION_CART_TO_WORKSHOP;
}

int32_t cart_image_group(int32_t resource)
{
    switch (resource) {
        case RESOURCE_GRAIN:
            return IMAGE_GROUP_FIGURE_CART_GRAIN;
        case RESOURCE_MEAT:
            return IMAGE_GROUP_FIGURE_CART_MEAT;
        case RESOURCE_LETTUCE:
            return IMAGE_GROUP_FIGURE_CART_LETTUCE;
        case RESOURCE_CHICKPEAS:
            return IMAGE_GROUP_FIGURE_CART_CHICKPEAS;
        case RESOURCE_POMEGRANATES:
            return IMAGE_GROUP_FIGURE_CART_POMEGRANATES;
        case RESOURCE_FIGS:
            return IMAGE_GROUP_FIGURE_CART_FIGS;
        case RESOURCE_FISH:
            return IMAGE_GROUP_FIGURE_CART_FISH;
        case RESOURCE_GAMEMEAT:
            return IMAGE_GROUP_FIGURE_CART_GAMEMEAT;
        case RESOURCE_STRAW:
            return IMAGE_GROUP_FIGURE_CART_STRAW;
        case RESOURCE_WEAPONS:
            return IMAGE_GROUP_FIGURE_CART_WEAPONS;
        case RESOURCE_CLAY:
            return IMAGE_GROUP_FIGURE_CART_CLAY;
        case RESOURCE_BRICKS:
            return IMAGE_GROUP_FIGURE_CART_BRICKS;
        case RESOURCE_POTTERY:
            return IMAGE_GROUP_FIGURE_CART_POTTERY;
        case RESOURCE_BARLEY:
            return IMAGE_GROUP_FIGURE_CART_BARLEY;
        case RESOURCE_BEER:
            return IMAGE_GROUP_FIGURE_CART_BEER;
        case RESOURCE_FLAX:
            return IMAGE_GROUP_FIGURE_CART_FLAX;
        case RESOURCE_LINEN:
            return IMAGE_GROUP_FIGURE_CART_LINEN;
        case RESOURCE_GEMS:
            return IMAGE_GROUP_FIGURE_CART_GEMS;
        case RESOURCE_LUXURY_GOODS:
            return IMAGE_GROUP_FIGURE_CART_LUXURY_GOODS;
        case RESOURCE_TIMBER:
            return IMAGE_GROUP_FIGURE_CART_TIMBER;
        case RESOURCE_GOLD:
            return IMAGE_GROUP_FIGURE_CART_GOLD;
        case RESOURCE_REEDS:
            return IMAGE_GROUP_FIGURE_CART_REEDS;
        case RESOURCE_PAPYRUS:
            return IMAGE_GROUP_FIGURE_CART_PAPYRUS;
        case RESOURCE_STONE:
            return IMAGE_GROUP_FIGURE_CART_STONE;
        case RESOURCE_LIMESTONE:
            return IMAGE_GROUP_FIGURE_CART_LIMESTONE;
        case RESOURCE_GRANITE:
            return IMAGE_GROUP_FIGURE_CART_GRANITE;
        case RESOURCE_CHARIOTS:
            return IMAGE_GROUP_FIGURE_CART_CHARIOTS;
        case RESOURCE_COPPER:
            return IMAGE_GROUP_FIGURE_CART_COPPER;
        case RESOURCE_SANDSTONE:
            return IMAGE_GROUP_FIGURE_CART_SANDSTONE;
        case RESOURCE_OIL:
            return IMAGE_GROUP_FIGURE_CART_OIL;
        case RESOURCE_HENNA:
            return IMAGE_GROUP_FIGURE_CART_HENNA;
        case RESOURCE_PAINT:
            return IMAGE_GROUP_FIGURE_CART_PAINT;
        case RESOURCE_LAMPS:
            return IMAGE_GROUP_FIGURE_CART_LAMPS;
        case RESOURCE_MARBLE:
            return IMAGE_GROUP_FIGURE_CART_MARBLE;
    }
    return IMAGE_GROUP_FIGURE_CART_EMPTY;
}

void cart_image(int32_t figure_id)
{
    figure *f = figure_get(figure_id);
    int16_t amount = f->resource_amount;
    int32_t offset;
    if (amount == 0) {
        offset = -1;
    } else if (amount <= 200) {
        offset = 0;
    } else {
        offset = amount > 400 ? 16 : 8;
    }
    int32_t resource = f->resource_id;
    if (carrier_is_big_resource(resource) && offset != -1) {
        offset = 0;
    }
    int32_t loaded;
    if (f->type == FIGURE_DOCKER) {
        int32_t state = f->action_state;
        int32_t empty = state == FIGURE_ACTION_DOCKER_EXPORT_AT_STORAGEYARD || state == FIGURE_ACTION_DOCKER_EXPORT_TO_STORAGEYARD ||
            (state == FIGURE_ACTION_WAIT_FOR_FERRY &&
                (f->action_state_before_attack == FIGURE_ACTION_DOCKER_EXPORT_AT_STORAGEYARD ||
                    f->action_state_before_attack == FIGURE_ACTION_DOCKER_EXPORT_TO_STORAGEYARD));
        if (!empty) {
            offset = 0;
        }
        loaded = !empty;
    } else {
        loaded = offset >= 0;
    }
    if (loaded) {
        f->cart_image_id = image_group(cart_image_group(resource)) + offset;
        return;
    }
    if (carrier_is_big_resource(resource) || (resource == RESOURCE_NONE && carrier_is_big_resource(f->collecting_item_id))) {
        f->cart_image_id = image_group(IMAGE_GROUP_FIGURE_CART_EMPTY_BIG);
    } else {
        f->cart_image_id = image_group(IMAGE_GROUP_FIGURE_CART_EMPTY);
    }
}

static void cart_update_image(int32_t figure_id)
{
    figure *f = figure_get(figure_id);
    int32_t direction = f->direction;
    if (direction >= DIRECTION_AT_DESTINATION) {
        direction = f->previous_tile_direction;
    }
    int32_t dir = carrier_image_direction(direction);
    if (f->action_state == FIGURE_ACTION_CORPSE) {
        f->image_id = image_group(IMAGE_GROUP_FIGURE_CART_PUSHER_DEATH) + figure_corpse_offsets[f->wait_ticks / 2];
        f->cart_image_id = 0;
    } else if (f->action_state != FIGURE_ACTION_ON_FERRY) {
        f->image_id = image_group(IMAGE_GROUP_FIGURE_CART_PUSHER) + (f->animation_frame * DIRECTION_COUNT) + dir;
    }
    if (!f->cart_image_id) {
        return;
    }
    f->cart_image_id += dir;
    int32_t offset_dir = dir;
    int32_t resource = f->resource_id;
    if (carrier_is_big_resource(resource) || (resource == RESOURCE_NONE && carrier_is_big_resource(f->collecting_item_id))) {
        offset_dir = dir + (DIRECTION_COUNT / 2);
        if (offset_dir >= DIRECTION_COUNT) {
            offset_dir = dir - (DIRECTION_COUNT / 2);
        }
    }
    f->cart_offset_x = figure_cart_offsets[offset_dir * 2];
    f->cart_offset_y = figure_cart_offsets[(offset_dir * 2) + 1];
}

int32_t cartpusher_determine_destination(int32_t figure_id, int32_t road_network)
{
    figure *f = figure_get(figure_id);
    int32_t dest = building_recruiter_for_weapons(f->x, f->y, f->resource_id, road_network);
    if (!dest) {
        dest = building_workshop_for_raw_material(figure_id, f->resource_id, road_network);
    }
    if (!dest) {
        dest = building_storageyard_for_storing(0, figure_id, f->resource_id, road_network);
    }
    if (!dest) {
        dest = building_granary_for_storing(0, figure_id, f->resource_id, road_network);
    }
    if (!dest) {
        dest = building_granary_for_storing(1, figure_id, f->resource_id, road_network);
    }
    f->destination_building_id = (int16_t) dest;
    return dest;
}

void cartpusher_delivered(int32_t amount, int32_t figure_id, int32_t road_network)
{
    figure *f = figure_get(figure_id);
    unbook_destination(f);
    if (amount == f->resource_amount) {
        f->action_state = FIGURE_ACTION_CART_RETURNING;
        f->wait_ticks = 0;
        f->destination_x = f->source_x;
        f->destination_y = f->source_y;
        f->resource_amount = 0;
        return;
    }
    f->resource_amount = (int16_t) (f->resource_amount - amount);
    int32_t dest = cartpusher_determine_destination(figure_id, road_network);
    if (dest) {
        f->action_state = (uint8_t) cart_state_for_destination(building_get(dest)->type);
        f->wait_ticks = 0;
        f->destination_x = map_result_x;
        f->destination_y = map_result_y;
        return;
    }
    f->action_state = FIGURE_ACTION_CART_INITIAL;
    f->wait_ticks = 0;
}

void warehouseman_delivered(int32_t amount, int32_t figure_id, int32_t road_network, bool done_dies)
{
    figure *f = figure_get(figure_id);
    unbook_destination(f);
    if (amount == f->resource_amount) {
        if (done_dies) {
            f->state = FIGURE_STATE_DEAD;
            f->wait_ticks = 0;
            return;
        }
        f->action_state = FIGURE_ACTION_STORAGE_CART_RETURNING_EMPTY;
        f->wait_ticks = 0;
        f->destination_x = f->source_x;
        f->destination_y = f->source_y;
        f->resource_amount = 0;
        return;
    }
    f->resource_amount = (int16_t) (f->resource_amount - amount);
    if (cartpusher_determine_destination(figure_id, road_network)) {
        f->action_state = FIGURE_ACTION_STORAGE_CART_DELIVERING;
        f->wait_ticks = 0;
        f->destination_x = map_result_x;
        f->destination_y = map_result_y;
        return;
    }
    f->action_state = FIGURE_ACTION_STORAGE_CART_INITIAL;
    f->min_max_seen = 2;
    f->wait_ticks = 0;
}

static int32_t cart_walk(int32_t figure_id, int32_t speed)
{
    cart_image(figure_id);
    figure_movement_move_ticks(figure_current_id, speed);
    return figure_get(figure_current_id)->direction;
}

static void cart_reroute(int32_t figure_id)
{
    figure_route_remove(figure_id);
    figure *f = figure_get(figure_current_id);
    if (routing_grids.land_citizen[f->grid_offset] != ROUTING_CITIZEN_PASSABLE) {
        f->action_state = FIGURE_ACTION_CART_INITIAL;
    }
    f->wait_ticks = 0;
}

void figure_cartpusher_action(void)
{
    int32_t figure_id = figure_current_id;
    figure *f = figure_get(figure_id);
    uint8_t frame = (uint8_t) (f->animation_frame + 1);
    f->animation_frame = frame;
    int32_t speed = figure_properties[f->type].speed;
    if (frame >= CARRIER_ANIMATION_FRAMES) {
        f->animation_frame = 0;
    }
    f->cart_image_id = 0;
    int32_t start_grid_offset = f->grid_offset;
    f->terrain_usage = TERRAIN_USAGE_ROADS;
    int32_t road_network = map_grids.road_network[start_grid_offset];
    switch (f->action_state) {
        case FIGURE_ACTION_ATTACK:
            figure_combat_handle_attack(figure_id);
            break;
        case FIGURE_ACTION_CORPSE:
            figure_combat_handle_corpse(figure_id);
            break;
        case FIGURE_ACTION_CART_INITIAL: {
            int32_t home_id = f->building_id;
            cart_image(figure_id);
            int8_t land = routing_grids.land_citizen[start_grid_offset];
            if (land < ROUTING_CITIZEN_ROAD || land > ROUTING_CITIZEN_PASSABLE) {
                f->state = FIGURE_STATE_DEAD;
            }
            building *home = building_get(home_id);
            if (home->state != BUILDING_STATE_IN_USE || (home->figure_id != figure_id && home->data.industry.second_cart_id != figure_id)) {
                f->state = FIGURE_STATE_DEAD;
            }
            if (++f->wait_ticks <= 30) {
                f->animation_frame = 0;
                break;
            }
            storage_staff_missing = 0;
            int32_t resource = home->output_resource;
            int32_t exported = 0;
            const city_player *player = city_player_current();
            int16_t status = player->trade_status[resource];
            if (status == TRADE_STATUS_EXPORT || status == TRADE_STATUS_EXPORT_SURPLUS) {
                exported = 1;
            }
            if (player->stockpiled[resource] == 0 && !exported) {
                figure_route_remove(figure_id);
            }
            int32_t dest = cartpusher_determine_destination(figure_id, road_network);
            if (dest) {
                f->action_state = (uint8_t) cart_state_for_destination(building_get(dest)->type);
                f->wait_ticks = 0;
                f->destination_x = map_result_x;
                f->destination_y = map_result_y;
            } else {
                f->wait_ticks = 0;
                f->min_max_seen = storage_staff_missing ? 2 : 1;
            }
            f->animation_frame = 0;
            break;
        }
        case FIGURE_ACTION_CART_TO_STORAGEYARD:
        case FIGURE_ACTION_CART_TO_GRANARY: {
            int32_t to_granary = f->action_state == FIGURE_ACTION_CART_TO_GRANARY;
            int32_t dir = cart_walk(figure_id, speed);
            if (dir == DIRECTION_AT_DESTINATION) {
                f->action_state = to_granary ? FIGURE_ACTION_CART_AT_GRANARY : FIGURE_ACTION_CART_AT_STORAGEYARD;
            } else if (dir == DIRECTION_REROUTE) {
                cart_reroute(figure_id);
            } else if (dir == DIRECTION_LOST) {
                if (to_granary) {
                    f->action_state = FIGURE_ACTION_CART_INITIAL;
                    f->wait_ticks = 0;
                } else {
                    f->state = FIGURE_STATE_DEAD;
                }
            }
            if (building_get(f->destination_building_id)->state != BUILDING_STATE_IN_USE) {
                f->state = FIGURE_STATE_DEAD;
            }
            break;
        }
        case FIGURE_ACTION_CART_TO_WORKSHOP:
        case FIGURE_ACTION_CART_TO_RECRUITER: {
            int32_t to_workshop = f->action_state == FIGURE_ACTION_CART_TO_WORKSHOP;
            int32_t dir = cart_walk(figure_id, speed);
            if (dir == DIRECTION_AT_DESTINATION) {
                f->action_state = to_workshop ? FIGURE_ACTION_CART_AT_WORKSHOP : FIGURE_ACTION_CART_AT_RECRUITER;
            } else if (dir == DIRECTION_REROUTE) {
                cart_reroute(figure_id);
            } else if (dir == DIRECTION_LOST) {
                f->state = FIGURE_STATE_DEAD;
            }
            break;
        }
        case FIGURE_ACTION_CART_AT_STORAGEYARD:
            cart_image(figure_id);
            if (++f->wait_ticks > 10) {
                building_main(f->destination_building_id);
                int32_t amount = building_storageyard_add_resource(f->faction_id, f->destination_building_id, f->resource_id, f->resource_amount);
                cartpusher_delivered(amount, figure_id, road_network);
            }
            f->animation_frame = 0;
            break;
        case FIGURE_ACTION_CART_AT_GRANARY:
            cart_image(figure_id);
            if (++f->wait_ticks > 5) {
                int32_t amount = building_granary_add_resource(f->destination_building_id, f->resource_id, 1, f->resource_amount);
                cartpusher_delivered(amount, figure_id, road_network);
            }
            f->animation_frame = 0;
            break;
        case FIGURE_ACTION_CART_AT_WORKSHOP:
            cart_image(figure_id);
            if (++f->wait_ticks > 5) {
                int32_t amount = building_workshop_add_input(f->destination_building_id, f->resource_amount, f->resource_id);
                cartpusher_delivered(amount, figure_id, road_network);
            }
            f->animation_frame = 0;
            break;
        case FIGURE_ACTION_CART_AT_RECRUITER:
            cart_image(figure_id);
            if (++f->wait_ticks > 5) {
                int32_t amount = building_recruiter_add_input(f->destination_building_id, f->resource_amount, f->resource_id);
                cartpusher_delivered(amount, figure_id, road_network);
            }
            f->animation_frame = 0;
            break;
        case FIGURE_ACTION_CART_RETURNING: {
            int32_t home_id = f->building_id;
            bool flood_comes = floods_is_running(&floods_data);
            if (building_is_farm(home_id) && building_is_on_floodplain(home_id) && flood_comes) {
                figure_route_remove(figure_id);
                f->state = FIGURE_STATE_DEAD;
                return;
            }
            int32_t dir = cart_walk(figure_id, speed);
            if (dir == DIRECTION_AT_DESTINATION) {
                f->action_state = FIGURE_ACTION_CART_INITIAL;
                f->state = FIGURE_STATE_DEAD;
            } else if (dir == DIRECTION_REROUTE) {
                figure_route_remove(figure_id);
            } else if (dir == DIRECTION_LOST) {
                f->state = FIGURE_STATE_DEAD;
            }
            break;
        }
        case FIGURE_ACTION_WAIT_FOR_FERRY:
            f->animation_frame = 0;
            cart_image(figure_id);
            f->image_id = image_group(IMAGE_GROUP_FIGURE_CART_PUSHER) + 1 + figure_image_direction;
            if (++f->wait_ticks > 100) {
                f->wait_ticks = 0;
                if (!figure_is_queued_for_ferry(figure_id)) {
                    f->action_state = FIGURE_ACTION_CART_INITIAL;
                }
            }
            break;
        case FIGURE_ACTION_ON_FERRY:
            f->image_id = 0;
            if (f->cart_image_id != 0) {
                f->cart_image_id = 0;
            }
            break;
        default:
            f->action_state = FIGURE_ACTION_CART_INITIAL;
            break;
    }
    cart_update_image(figure_id);
}

static int32_t warehouseman_walk(int32_t speed)
{
    cart_image(figure_current_id);
    figure_movement_move_ticks(figure_current_id, speed);
    return figure_get(figure_current_id)->direction;
}

static void warehouseman_walk_failed(int32_t dir)
{
    if (dir == DIRECTION_REROUTE) {
        figure_route_remove(figure_current_id);
    } else if (dir == DIRECTION_LOST) {
        figure_get(figure_current_id)->state = FIGURE_STATE_DEAD;
    }
}

static int32_t warehouseman_fetch_amount(int32_t resource)
{
    if (carrier_is_big_resource(resource) || resource == RESOURCE_CHARIOTS || resource == RESOURCE_WEAPONS) {
        return 100;
    }
    return 400;
}

static void warehouseman_took(int32_t amount, int32_t state)
{
    figure *f = figure_get(figure_current_id);
    f->resource_amount = (int16_t) amount;
    if (f->resource_amount == 0) {
        f->resource_id = RESOURCE_NONE;
        f->action_state = FIGURE_ACTION_STORAGE_CART_INITIAL;
        figure_route_remove(figure_current_id);
        return;
    }
    f->resource_id = f->collecting_item_id;
    f->action_state = (uint8_t) state;
    f->wait_ticks = 0;
    f->destination_x = f->source_x;
    f->destination_y = f->source_y;
    figure_route_remove(figure_current_id);
}

static void warehouseman_initial_granary(int32_t home_id, int32_t road_network)
{
    figure *f = figure_get(figure_current_id);
    building *home = building_get(home_id);
    int32_t resource = f->resource_id;
    if (resource == RESOURCE_NONE) {
        int32_t dest = building_granary_supplier(home_id);
        int32_t state;
        if (dest) {
            state = FIGURE_ACTION_STORAGE_CART_TO_GRANARY_GETTING;
        } else {
            dest = building_storageyard_for_getting(home_id, f->collecting_item_id, 0);
            if (!dest) {
                f->action_state = FIGURE_ACTION_STORAGE_CART_INITIAL;
                cart_image(figure_current_id);
                return;
            }
            state = building_get(dest)->type == BUILDING_STORAGE_YARD ? FIGURE_ACTION_STORAGE_CART_TO_YARD_GETTING_FOOD
                                                                      : FIGURE_ACTION_STORAGE_CART_TO_GRANARY_GETTING;
        }
        f->destination_building_id = (int16_t) dest;
        f->action_state = (uint8_t) state;
        f->wait_ticks = 0;
        f->destination_x = map_result_x;
        f->destination_y = map_result_y;
        return;
    }
    int32_t dest = building_granary_for_storing(0, figure_current_id, resource, road_network);
    if (!dest) {
        dest = building_storageyard_for_storing(0, figure_current_id, f->resource_id, road_network);
    }
    if (!dest) {
        dest = building_granary_for_storing(1, figure_current_id, f->resource_id, road_network);
    }
    if (dest) {
        f->destination_building_id = (int16_t) dest;
        f->action_state = FIGURE_ACTION_STORAGE_CART_DELIVERING;
        f->wait_ticks = 0;
        f->destination_x = map_result_x;
        f->destination_y = map_result_y;
        return;
    }
    cart_image(figure_current_id);
    int32_t storage_state = storage_get(home->storage_id)->resource_state[f->resource_id];
    if (storage_state == STORAGE_STATE_EMPTY || storage_state == STORAGE_STATE_GET) {
        f->action_state = FIGURE_ACTION_STORAGE_CART_INITIAL;
    } else {
        f->destination_building_id = (int16_t) home_id;
        f->action_state = FIGURE_ACTION_STORAGE_CART_DELIVERING;
        f->wait_ticks = 0;
        f->destination_x = home->x;
        f->destination_y = home->y;
    }
}

static int32_t warehouseman_initial_yard(int32_t home_id, int32_t road_network)
{
    figure *f = figure_get(figure_current_id);
    building *home = building_get(home_id);
    if (f->resource_id == RESOURCE_NONE) {
        int32_t resource1 = RESOURCE_NONE;
        int32_t resource2 = RESOURCE_NONE;
        int32_t percent1 = 100;
        int32_t percent2 = 100;
        int32_t storage1 = 0;
        int32_t storage2 = 0;
        building_storage_wanted_resources(home_id, &resource1, &resource2, &percent1, &percent2, &storage1, &storage2);
        int32_t resource = resource1;
        if (home->data.storage_yard.second_cart_id == figure_current_id && resource2 != RESOURCE_NONE) {
            resource = resource2;
        }
        int32_t dest = building_storageyard_for_getting(home_id, resource, 0);
        if (!dest) {
            f->state = FIGURE_STATE_DEAD;
            return 0;
        }
        f->collecting_item_id = (uint8_t) resource;
        f->resource_amount = 0;
        f->destination_building_id = (int16_t) dest;
        f->action_state = building_get(dest)->type == BUILDING_STORAGE_YARD ? FIGURE_ACTION_STORAGE_CART_TO_YARD_GETTING
                                                                            : FIGURE_ACTION_STORAGE_CART_TO_GRANARY_GETTING;
        f->wait_ticks = 0;
        f->destination_x = map_result_x;
        f->destination_y = map_result_y;
        f->terrain_usage = TERRAIN_USAGE_PREFER_ROADS;
        return 1;
    }
    int32_t dest = building_recruiter_for_weapons(f->x, f->y, f->resource_id, road_network);
    if (!dest) {
        dest = building_workshop_for_raw_material(figure_current_id, f->resource_id, road_network);
    }
    if (!dest) {
        dest = building_granary_for_storing(0, figure_current_id, f->resource_id, road_network);
    }
    if (!dest) {
        dest = building_granary_getting_food(figure_current_id, f->resource_id, road_network);
    }
    if (!dest) {
        dest = building_storageyard_for_storing(home_id, figure_current_id, f->resource_id, road_network);
    }
    if (!dest) {
        dest = building_workshop_for_raw_material_any(figure_current_id, f->resource_id, road_network);
    }
    if (dest) {
        f->destination_building_id = (int16_t) dest;
        f->action_state = FIGURE_ACTION_STORAGE_CART_DELIVERING;
        f->wait_ticks = 0;
        f->destination_x = map_result_x;
        f->destination_y = map_result_y;
        return 1;
    }
    cart_image(figure_current_id);
    f->action_state = FIGURE_ACTION_STORAGE_CART_INITIAL;
    return 1;
}

void figure_warehouseman_action(void)
{
    int32_t figure_id = figure_current_id;
    figure *f = figure_get(figure_id);
    f->terrain_usage = TERRAIN_USAGE_ROADS;
    uint8_t frame = (uint8_t) (f->animation_frame + 1);
    int32_t speed = figure_properties[f->type].speed;
    f->animation_frame = frame;
    if (frame >= CARRIER_ANIMATION_FRAMES) {
        f->animation_frame = 0;
    }
    f->cart_image_id = 0;
    int32_t road_network = map_grids.road_network[f->grid_offset];
    switch (f->action_state) {
        case FIGURE_ACTION_ATTACK:
            figure_combat_handle_attack(figure_id);
            break;
        case FIGURE_ACTION_CORPSE:
            figure_combat_handle_corpse(figure_id);
            break;
        case FIGURE_ACTION_STORAGE_CART_INITIAL: {
            int32_t home_id = f->building_id;
            building *home = building_get(home_id);
            int32_t is_home_cart = home->figure_id == figure_id || home->figure_id2 == figure_id || home->data.storage_yard.second_cart_id == figure_id;
            if (home->state != BUILDING_STATE_IN_USE || !is_home_cart) {
                f->state = FIGURE_STATE_DEAD;
            }
            if (++f->wait_ticks > 2) {
                if (home->type == BUILDING_GRANARY || home->type == BUILDING_GRANARY_UP) {
                    warehouseman_initial_granary(home_id, road_network);
                } else if (!warehouseman_initial_yard(home_id, road_network)) {
                    return;
                }
            }
            f->animation_frame = 0;
            cart_image(figure_id);
            break;
        }
        case FIGURE_ACTION_STORAGE_CART_DELIVERING: {
            int32_t dir = warehouseman_walk(speed);
            if (dir == DIRECTION_AT_DESTINATION) {
                f->action_state = FIGURE_ACTION_STORAGE_CART_AT_DELIVERY;
            } else {
                warehouseman_walk_failed(dir);
            }
            break;
        }
        case FIGURE_ACTION_STORAGE_CART_AT_DELIVERY: {
            int32_t dest = f->destination_building_id;
            building_main(dest);
            cart_image(figure_id);
            if (++f->wait_ticks > 4) {
                cart_image(figure_id);
                int32_t amount;
                switch (building_get(dest)->type) {
                    case BUILDING_STORAGE_YARD:
                    case BUILDING_STORAGE_ROOM:
                    case BUILDING_STORAGE_YARD_UP:
                        amount = building_storageyard_add_resource(city_current_player, dest, f->resource_id, f->resource_amount);
                        break;
                    case BUILDING_GRANARY:
                    case BUILDING_GRANARY_UP:
                        amount = building_granary_add_resource(dest, f->resource_id, 0, f->resource_amount);
                        break;
                    case BUILDING_RECRUITER:
                        amount = building_recruiter_add_input(dest, f->resource_amount, f->resource_id);
                        break;
                    default:
                        amount = building_workshop_add_input(dest, f->resource_amount, f->resource_id);
                        break;
                }
                warehouseman_delivered(amount, figure_id, road_network, 0);
            }
            f->animation_frame = 0;
            break;
        }
        case FIGURE_ACTION_STORAGE_CART_RETURNING_EMPTY: {
            int32_t dir = warehouseman_walk(speed);
            if (dir == DIRECTION_AT_DESTINATION) {
                f->state = FIGURE_STATE_DEAD;
            } else {
                warehouseman_walk_failed(dir);
            }
            break;
        }
        case FIGURE_ACTION_STORAGE_CART_GETTING_FOOD_RETURN: {
            int32_t dir = warehouseman_walk(speed);
            if (dir == DIRECTION_AT_DESTINATION) {
                f->action_state = FIGURE_ACTION_STORAGE_CART_TAKE_FOOD;
            } else {
                warehouseman_walk_failed(dir);
            }
            break;
        }
        case FIGURE_ACTION_STORAGE_CART_TAKE_FOOD: {
            int32_t home_id = f->building_id;
            int32_t dest = f->destination_building_id;
            if (++f->wait_ticks > 4) {
                int32_t amount = building_granary_take_wanted(home_id, dest);
                f->resource_amount = (int16_t) amount;
                f->resource_id = (uint8_t) granary_take_resource;
                f->action_state = FIGURE_ACTION_STORAGE_CART_RETURNING_WITH_FOOD;
                f->wait_ticks = 0;
                f->destination_x = f->source_x;
                f->destination_y = f->source_y;
                figure_route_remove(figure_id);
            }
            f->animation_frame = 0;
            break;
        }
        case FIGURE_ACTION_STORAGE_CART_TO_GRANARY_GETTING: {
            int32_t dir = warehouseman_walk(speed);
            if (dir == DIRECTION_AT_DESTINATION) {
                f->action_state = FIGURE_ACTION_STORAGE_CART_AT_GRANARY_GETTING;
            } else {
                warehouseman_walk_failed(dir);
            }
            break;
        }
        case FIGURE_ACTION_STORAGE_CART_AT_GRANARY_GETTING: {
            cart_image(figure_id);
            int32_t home_id = f->building_id;
            int32_t dest = f->destination_building_id;
            if (++f->wait_ticks > 4) {
                int32_t left = building_granary_remove_resource(city_current_player, dest, f->collecting_item_id, 400);
                int32_t amount = left == 400 ? 0 : left == 0 ? 400 : left;
                warehouseman_took(amount,
                    building_get(home_id)->type == BUILDING_GRANARY ? FIGURE_ACTION_STORAGE_CART_RETURNING_WITH_FOOD
                                                                    : FIGURE_ACTION_STORAGE_CART_RETURNING_WITH_GOODS);
            }
            f->animation_frame = 0;
            break;
        }
        case FIGURE_ACTION_STORAGE_CART_TO_YARD_GETTING_FOOD: {
            int32_t dir = warehouseman_walk(speed);
            if (dir == DIRECTION_AT_DESTINATION) {
                f->action_state = FIGURE_ACTION_STORAGE_CART_AT_YARD_GETTING_FOOD;
            } else {
                warehouseman_walk_failed(dir);
            }
            break;
        }
        case FIGURE_ACTION_STORAGE_CART_AT_YARD_GETTING_FOOD:
        case FIGURE_ACTION_STORAGE_CART_AT_YARD_GETTING: {
            int32_t food = f->action_state == FIGURE_ACTION_STORAGE_CART_AT_YARD_GETTING_FOOD;
            if (!food) {
                f->terrain_usage = TERRAIN_USAGE_PREFER_ROADS;
            }
            int32_t dest = f->destination_building_id;
            if (++f->wait_ticks > 4) {
                if (!food) {
                    f->resource_amount = 0;
                }
                int32_t resource = f->collecting_item_id;
                int32_t want = warehouseman_fetch_amount(resource);
                int32_t left = building_storageyard_remove_resource(city_current_player, dest, resource, want);
                int32_t amount = left == want ? 0 : left == 0 ? want : want - left;
                warehouseman_took(amount, food ? FIGURE_ACTION_STORAGE_CART_RETURNING_WITH_FOOD : FIGURE_ACTION_STORAGE_CART_RETURNING_WITH_GOODS);
            }
            f->animation_frame = 0;
            break;
        }
        case FIGURE_ACTION_STORAGE_CART_RETURNING_WITH_FOOD: {
            int32_t home_id = f->building_id;
            int32_t dir = warehouseman_walk(speed);
            if (dir == DIRECTION_AT_DESTINATION) {
                int32_t amount = building_granary_add_resource(home_id, f->resource_id, 0, f->resource_amount);
                warehouseman_delivered(amount, figure_id, road_network, 1);
            } else {
                warehouseman_walk_failed(dir);
            }
            break;
        }
        case FIGURE_ACTION_STORAGE_CART_TO_YARD_GETTING: {
            f->terrain_usage = TERRAIN_USAGE_PREFER_ROADS;
            int32_t dir = warehouseman_walk(speed);
            if (dir == DIRECTION_AT_DESTINATION) {
                f->action_state = FIGURE_ACTION_STORAGE_CART_AT_YARD_GETTING;
            } else {
                warehouseman_walk_failed(dir);
            }
            break;
        }
        case FIGURE_ACTION_STORAGE_CART_RETURNING_WITH_GOODS: {
            f->terrain_usage = TERRAIN_USAGE_PREFER_ROADS;
            int32_t home_id = f->building_id;
            int32_t dir = warehouseman_walk(speed);
            if (dir == DIRECTION_AT_DESTINATION) {
                int32_t amount = building_storageyard_add_resource(city_current_player, home_id, f->resource_id, f->resource_amount);
                warehouseman_delivered(amount, figure_id, road_network, 1);
            } else {
                warehouseman_walk_failed(dir);
            }
            break;
        }
        case FIGURE_ACTION_WAIT_FOR_FERRY:
            f->animation_frame = 0;
            cart_image(figure_id);
            f->image_id = image_group(IMAGE_GROUP_FIGURE_CART_PUSHER) + 1 + figure_image_direction;
            if (++f->wait_ticks > 100) {
                f->wait_ticks = 0;
                if (!figure_is_queued_for_ferry(figure_id)) {
                    f->action_state = FIGURE_ACTION_STORAGE_CART_INITIAL;
                }
            }
            break;
        case FIGURE_ACTION_ON_FERRY:
            f->image_id = 0;
            if (f->cart_image_id != 0) {
                f->cart_image_id = 0;
            }
            break;
        default:
            f->action_state = FIGURE_ACTION_STORAGE_CART_INITIAL;
            break;
    }
    cart_update_image(figure_id);
}

int32_t building_granary_take_wanted(int32_t building_id, int32_t src_building_id)
{
    const building *b = building_get(building_id);
    const building *src = building_get(src_building_id);
    int32_t storage = b->storage_id;
    int32_t src_storage = src->storage_id;
    int32_t resource = RESOURCE_NONE;
    int32_t amount = 0;
    for (int32_t r = RESOURCE_MIN_FOOD; r <= RESOURCE_GAMEMEAT; r++) {
        int32_t wanted = storage_get(storage)->resource_state[r] == STORAGE_STATE_GET && storage_get(src_storage)->resource_state[r] != STORAGE_STATE_GET;
        if (wanted && src->data.granary.resource_stored[r] > amount) {
            amount = src->data.granary.resource_stored[r];
            resource = r;
        }
    }
    if (amount > GRANARY_CART_MAX_TAKE) {
        amount = GRANARY_CART_MAX_TAKE;
    }
    if (amount > b->data.granary.resource_stored[RESOURCE_NONE]) {
        amount = b->data.granary.resource_stored[RESOURCE_NONE];
    }
    building_granary_remove_resource(city_current_player, src_building_id, resource, amount);
    granary_take_resource = resource;
    return amount;
}

static int32_t is_workshop_type(int32_t type)
{
    if (type >= BUILDING_BREWERY && type <= BUILDING_POTTER) {
        return 1;
    }
    switch (type) {
        case BUILDING_SENET_HOUSE:
        case BUILDING_MORTUARY:
        case BUILDING_SCRIBAL_SCHOOL:
        case BUILDING_LIBRARY:
        case BUILDING_SHIPWRIGHT:
        case BUILDING_JEWELER:
        case BUILDING_CARPENTERS_GUILD:
        case BUILDING_BRICKLAYERS_GUILD:
        case BUILDING_VILLAGE_PALACE_UP:
        case BUILDING_TOWN_PALACE_UP:
        case BUILDING_CITY_PALACE:
        case BUILDING_CATTLE_RANCH:
        case BUILDING_PAPYRUS_MAKER:
        case BUILDING_BRICKWORKS:
        case BUILDING_CHARIOT_MAKER:
        case BUILDING_ZOO:
        case BUILDING_ARTISANS_GUILD:
        case BUILDING_LAMP_MAKER:
        case BUILDING_PAINT_MAKER:
            return 1;
        default:
            return 0;
    }
}

void figure_count_by_destination(void)
{
    for (int32_t id = 1; id <= MAX_BUILDINGS - 1; id++) {
        if (city_buildings.records[id].state != BUILDING_STATE_IN_USE) {
            continue;
        }
        int16_t type = city_buildings.records[id].type;
        if (is_workshop_type(type) || type == BUILDING_GRANARY || type == BUILDING_RECRUITER) {
            city_buildings.records[id].incoming_carts = 0;
            city_buildings.records[id].incoming_loads = 0;
        }
    }
    for (int32_t f = 1; f <= MAX_FIGURES - 1; f++) {
        if (!city_figures.records[f].state) {
            continue;
        }
        int16_t amount = city_figures.records[f].resource_amount;
        if (amount == 0) {
            continue;
        }
        int32_t b = city_figures.records[f].destination_building_id;
        int16_t type = city_buildings.records[b].type;
        if (!is_workshop_type(type)) {
            if (type == BUILDING_GRANARY) {
                int16_t *incoming_loads = &city_buildings.records[b].incoming_loads;
                city_buildings.records[b].incoming_carts++;
                *incoming_loads += amount;
            }
            continue;
        }
        uint8_t kind = city_figures.records[f].resource_id;
        if (type == BUILDING_RECRUITER) {
            if (kind == RESOURCE_CHARIOTS) {
                city_buildings.records[b].incoming_loads++;
            } else if (kind == RESOURCE_WEAPONS) {
                city_buildings.records[b].incoming_carts++;
            }
        } else if (type == BUILDING_BRICKWORKS) {
            if (kind == RESOURCE_STRAW) {
                city_buildings.records[b].incoming_loads++;
            } else if (kind == RESOURCE_CLAY) {
                city_buildings.records[b].incoming_carts++;
            }
        } else {
            city_buildings.records[b].incoming_carts++;
        }
    }
}

static int32_t bazaar_slot_resource(int32_t slot)
{
    switch (slot) {
        case INVENTORY_GRAIN:
            return RESOURCE_GRAIN;
        case INVENTORY_MEAT:
            return RESOURCE_MEAT;
        case INVENTORY_LETTUCE:
            return RESOURCE_LETTUCE;
        case INVENTORY_CHICKPEAS:
            return RESOURCE_CHICKPEAS;
        case INVENTORY_POMEGRANATES:
            return RESOURCE_POMEGRANATES;
        case INVENTORY_FIGS:
            return RESOURCE_FIGS;
        case INVENTORY_FISH:
            return RESOURCE_FISH;
        case INVENTORY_GAMEMEAT:
            return RESOURCE_GAMEMEAT;
        case INVENTORY_BEER:
            return RESOURCE_BEER;
        case INVENTORY_POTTERY:
            return RESOURCE_POTTERY;
        case INVENTORY_LINEN:
            return RESOURCE_LINEN;
        case INVENTORY_JEWELRY:
            return RESOURCE_LUXURY_GOODS;
    }
    return -1;
}

static int32_t bazaar_resource_slot(int32_t resource)
{
    switch (resource) {
        case RESOURCE_GRAIN:
            return INVENTORY_GRAIN;
        case RESOURCE_MEAT:
            return INVENTORY_MEAT;
        case RESOURCE_LETTUCE:
            return INVENTORY_LETTUCE;
        case RESOURCE_CHICKPEAS:
            return INVENTORY_CHICKPEAS;
        case RESOURCE_POMEGRANATES:
            return INVENTORY_POMEGRANATES;
        case RESOURCE_FIGS:
            return INVENTORY_FIGS;
        case RESOURCE_FISH:
            return INVENTORY_FISH;
        case RESOURCE_GAMEMEAT:
            return INVENTORY_GAMEMEAT;
        case RESOURCE_POTTERY:
            return INVENTORY_POTTERY;
        case RESOURCE_BEER:
            return INVENTORY_BEER;
        case RESOURCE_LINEN:
            return INVENTORY_LINEN;
        case RESOURCE_LUXURY_GOODS:
            return INVENTORY_JEWELRY;
    }
    return BAZAAR_NO_SLOT;
}

static int32_t stock_loads(int32_t stock, int32_t max_loads)
{
    int32_t loads = 0;
    for (int32_t level = max_loads; level >= 1; level--) {
        if (stock >= level * 100) {
            loads = level;
            break;
        }
    }
    if (!loads) {
        loads = stock > 0;
    }
    return loads;
}

static int32_t buyer_loads(int32_t want, int32_t available)
{
    if (want < 1) {
        return 0;
    }
    int32_t max_want = want / 100 + 1;
    int32_t loads = available;
    if (max_want < loads) {
        loads = max_want;
    }
    return loads < 1 ? 0 : loads;
}

int32_t buyer_go_to_storage(int32_t figure_id, int32_t storage_id)
{
    figure *f = figure_get(figure_id);
    f->action_state = FIGURE_ACTION_MARKET_BUYER_TO_STORAGE;
    f->destination_building_id = (int16_t) storage_id;
    f->direction = 0;
    building *b = building_get(storage_id);
    if (map_has_road_access(b->x, b->y, b->size)) {
        f->destination_x = map_result_x;
        f->destination_y = map_result_y;
        return 0;
    }
    if (!map_has_road_access(b->x, b->y, 3)) {
        f->action_state = FIGURE_ACTION_MARKET_BUYER_RETURNING;
        f->destination_x = f->source_x;
        f->destination_y = f->source_y;
        return 0;
    }
    f->destination_x = map_result_x;
    f->destination_y = map_result_y;
    return 0;
}

int32_t buyer_take_food(int32_t figure_id, int32_t home_id, int32_t storage_id, int32_t index)
{
    figure *f = figure_get(figure_id);
    return buyer_buy_food(figure_id, home_id, storage_id, f->buyer_resources[index], f->buyer_amounts_bought[index], index);
}

int32_t buyer_take_goods(int32_t figure_id, int32_t home_id, int32_t storage_id, int32_t index)
{
    figure *f = figure_get(figure_current_id);
    return buyer_buy_goods(figure_id, home_id, storage_id, f->buyer_resources[index], f->buyer_amounts_bought[index], index);
}

void buyer_return_home(int32_t figure_id)
{
    figure *f = figure_get(figure_id);
    f->action_state = FIGURE_ACTION_MARKET_BUYER_RETURNING;
    f->destination_x = f->source_x;
    f->destination_y = f->source_y;
    figure_route_remove(figure_id);
}

bool buyer_has_bought(int32_t figure_id)
{
    const int32_t *bought = figure_get(figure_id)->buyer_amounts_bought;
    for (int32_t i = 0; i < 4; i++) {
        if (bought[i] != 0) {
            return 1;
        }
    }
    return 0;
}

int32_t buyer_best_slot(int32_t *wants)
{
    int32_t best = -1;
    int32_t max = 0;
    for (int32_t i = 0; i < 4; i++) {
        if (wants[i] / 100 > max) {
            best = i;
            max = wants[i] / 100;
        }
    }
    return best;
}

void buyer_return_or_die(int32_t figure_id)
{
    if (buyer_has_bought(figure_id)) {
        buyer_return_home(figure_id);
        return;
    }
    figure_get(figure_id)->state = FIGURE_STATE_DEAD;
}

static void buyer_next_storage(int32_t figure_id, int32_t home_id, int32_t storage_id, int32_t food)
{
    figure *f = figure_get(figure_id);
    if (f->buyer_storages_visited < 3) {
        int32_t wants[4];
        const building *home = building_get(home_id);
        for (int32_t i = 0; i < 4; i++) {
            int32_t item = f->buyer_resources[i];
            wants[i] = 0;
            if (item != -1) {
                int32_t max = food ? (item == 0 ? 800 : 600) : 400;
                wants[i] = max - home->data.market.inventory[item] - f->buyer_amounts_bought[i];
            }
        }
        int32_t index = buyer_best_slot(wants);
        building_market_update_buyer_storages(figure_id, storage_id);
        while (index != -1) {
            int32_t item = figure_get(figure_id)->buyer_resources[index];
            if (city_buildings.market_nearest_storage[item] > 0) {
                buyer_go_to_storage(figure_id, city_buildings.market_nearest_storage[item]);
                return;
            }
            wants[index] = 0;
            index = buyer_best_slot(wants);
        }
    }
    buyer_return_or_die(figure_id);
}

void buyer_next_goods_storage(int32_t figure_id, int32_t home_id, int32_t storage_id)
{
    buyer_next_storage(figure_id, home_id, storage_id, 0);
}

void buyer_next_food_storage(int32_t figure_id, int32_t home_id, int32_t storage_id)
{
    buyer_next_storage(figure_id, home_id, storage_id, 1);
}

void buyer_at_storage(int32_t figure_id, int32_t home_id, int32_t storage_id)
{
    int32_t any_food = 0;
    for (int32_t i = 0; i < 4; i++) {
        uint32_t item = (uint32_t) figure_get(figure_current_id)->buyer_resources[i];
        if (item <= 7) {
            buyer_take_food(figure_id, home_id, storage_id, i);
            any_food = 1;
        } else if (item <= 11) {
            buyer_take_goods(figure_id, home_id, storage_id, i);
        }
    }
    figure *f = figure_get(figure_id);
    f->buyer_storage_ids[f->buyer_storages_visited] = storage_id;
    f->buyer_storages_visited++;
    if (any_food) {
        buyer_next_food_storage(figure_id, home_id, storage_id);
    } else {
        buyer_next_goods_storage(figure_id, home_id, storage_id);
    }
}

int32_t buyer_buy_food(int32_t figure_id, int32_t home_id, int32_t storage_id, int32_t slot, int32_t bought, int32_t index)
{
    int32_t resource = bazaar_slot_resource(slot);
    if (city_player_current()->stockpiled[resource] != 0) {
        return 0;
    }
    int32_t type = building_get(storage_id)->type;
    int32_t loads;
    if (type == BUILDING_GRANARY || type == BUILDING_GRANARY_UP) {
        loads = buyer_food_loads_granary(figure_id, resource, home_id, storage_id, bought, slot);
    } else {
        loads = buyer_food_loads_yard(figure_id, resource, home_id, storage_id, bought, slot);
    }
    if (loads <= 0) {
        return 0;
    }
    int32_t leader = buyer_check_no_leader(figure_id);
    if (leader == 0) {
        return 0;
    }
    buyer_send_food_boys(resource, loads, figure_id, home_id, storage_id, leader);
    figure_get(figure_id)->buyer_amounts_bought[index] += loads * 100;
    return 1;
}

int32_t buyer_buy_goods(int32_t figure_id, int32_t home_id, int32_t storage_id, int32_t slot, int32_t bought, int32_t index)
{
    int32_t resource = bazaar_slot_resource(slot);
    if (city_player_current()->stockpiled[resource] != 0) {
        return 0;
    }
    int32_t loads = buyer_goods_loads(resource, figure_id, home_id, storage_id, bought, slot);
    if (loads <= 0) {
        return 0;
    }
    int32_t leader = buyer_check_no_leader(figure_id);
    if (leader == 0) {
        return 0;
    }
    buyer_send_goods_boys(resource, loads, figure_id, home_id, storage_id, leader);
    figure_get(figure_id)->buyer_amounts_bought[index] += loads * 100;
    return 1;
}

int32_t buyer_food_loads_yard(int32_t figure_id, int32_t resource, int32_t home_id, int32_t storage_id, int32_t bought, int32_t slot)
{
    (void) figure_id;
    int32_t amount = building_storageyard_get_amount(storage_id, resource);
    int32_t max = slot != 0 ? 600 : 800;
    int32_t want = max - building_get(home_id)->data.market.inventory[slot] - bought;
    if (want < 1) {
        return 0;
    }
    return buyer_loads(want, stock_loads(amount, 8));
}

int32_t buyer_food_loads_granary(int32_t figure_id, int32_t resource, int32_t home_id, int32_t storage_id, int32_t bought, int32_t slot)
{
    (void) figure_id;
    int32_t max = slot != 0 ? 600 : 800;
    int32_t want = max - building_get(home_id)->data.market.inventory[slot] - bought;
    if (want < 1) {
        return 0;
    }
    return buyer_loads(want, stock_loads(building_get(storage_id)->data.granary.resource_stored[resource], 8));
}

int32_t buyer_goods_loads(int32_t resource, int32_t figure_id, int32_t home_id, int32_t storage_id, int32_t bought, int32_t slot)
{
    (void) figure_id;
    int32_t want = 400 - building_get(home_id)->data.market.inventory[slot] - bought;
    if (want < 1) {
        return 0;
    }
    int32_t amount = building_storageyard_get_amount(storage_id, resource);
    return buyer_loads(want, stock_loads(amount, 4));
}

int32_t buyer_check_no_leader(int32_t figure_id)
{
    figure *f = figure_get(figure_id);
    if (f->leading_figure_id != 0) {
        f->state = FIGURE_STATE_DEAD;
        return 0;
    }
    return figure_id;
}

static void buyer_send_boys(int32_t resource, int32_t loads, int32_t figure_id, int32_t leader_id, int32_t max)
{
    int32_t slot = bazaar_resource_slot(resource);
    int32_t leader = leader_id;
    for (int32_t i = 0; i < max && (i == 0 || i < loads); i++) {
        const figure *buyer = figure_get(figure_id);
        int32_t boy_id = figure_create(city_current_player, FIGURE_DELIVERY_BOY, buyer->x, buyer->y, 0);
        figure *boy = figure_get(boy_id);
        boy->leading_figure_id = (int16_t) leader;
        boy->collecting_item_id = (uint8_t) slot;
        boy->building_id = figure_get(figure_id)->building_id;
        leader = boy_id;
    }
}

int32_t buyer_send_food_boys(int32_t resource, int32_t loads, int32_t figure_id, int32_t home_id, int32_t storage_id, int32_t leader_id)
{
    (void) home_id;
    int32_t type = building_get(storage_id)->type;
    if (type == BUILDING_GRANARY || type == BUILDING_GRANARY_UP) {
        building_granary_remove_resource(city_current_player, storage_id, resource, loads * 100);
    } else {
        building_storageyard_remove_resource(city_current_player, storage_id, resource, loads * 100);
    }
    buyer_send_boys(resource, loads, figure_id, leader_id, 8);
    return 1;
}

int32_t buyer_send_goods_boys(int32_t resource, int32_t loads, int32_t figure_id, int32_t home_id, int32_t storage_id, int32_t leader_id)
{
    (void) home_id;
    building_storageyard_remove_resource(city_current_player, storage_id, resource, loads * 100);
    buyer_send_boys(resource, loads, figure_id, leader_id, 4);
    return 1;
}

void figure_market_buyer_action(void)
{
    int32_t figure_id = figure_current_id;
    figure *f = figure_get(figure_id);
    f->terrain_usage = TERRAIN_USAGE_ROADS;
    f->use_cross_country = 0;
    f->max_roam_length = 800;
    int32_t speed = figure_properties[f->type].speed;
    int32_t home_id = f->building_id;
    building *home = building_get(home_id);
    if (home->state != BUILDING_STATE_IN_USE) {
        f->state = FIGURE_STATE_DEAD;
    }
    if (home->figure_id2 != figure_id && home->data.market.second_buyer_id != figure_id) {
        f->state = FIGURE_STATE_DEAD;
    }
    uint8_t frame = (uint8_t) (f->animation_frame + 1);
    f->animation_frame = frame;
    if (frame >= CARRIER_ANIMATION_FRAMES) {
        f->animation_frame = 0;
    }
    switch (f->action_state) {
        case FIGURE_ACTION_CORPSE:
            figure_combat_handle_corpse(figure_id);
            break;
        case FIGURE_ACTION_ATTACK:
            figure_combat_handle_attack(figure_id);
            break;
        case FIGURE_ACTION_ON_FERRY:
            f->image_id = 0;
            break;
        case FIGURE_ACTION_WAIT_FOR_FERRY:
            f->animation_frame = 0;
            if (++f->wait_ticks > 100) {
                f->wait_ticks = 0;
                if (!figure_is_queued_for_ferry(figure_id)) {
                    f->action_state = FIGURE_ACTION_MARKET_BUYER_RETURNING;
                }
            }
            break;
        case FIGURE_ACTION_MARKET_BUYER_TO_STORAGE: {
            figure_movement_move_ticks(figure_id, speed);
            if (f->direction == DIRECTION_AT_DESTINATION) {
                buyer_at_storage(figure_id, home_id, f->destination_building_id);
            } else if (f->direction == DIRECTION_REROUTE || f->direction == DIRECTION_LOST) {
                f->action_state = FIGURE_ACTION_MARKET_BUYER_RETURNING;
                f->destination_x = f->source_x;
                f->destination_y = f->source_y;
                figure_route_remove(figure_id);
            }
            break;
        }
        case FIGURE_ACTION_MARKET_BUYER_RETURNING:
            figure_movement_move_ticks(figure_id, speed);
            if (f->direction == DIRECTION_AT_DESTINATION || f->direction == DIRECTION_LOST) {
                f->state = FIGURE_STATE_DEAD;
            } else if (f->direction == DIRECTION_REROUTE) {
                figure_route_remove(figure_id);
            }
            break;
        default:
            f->action_state = FIGURE_ACTION_MARKET_BUYER_RETURNING;
            f->destination_x = f->source_x;
            f->destination_y = f->source_y;
            figure_route_remove(figure_id);
            break;
    }
    int32_t dir = carrier_image_direction(f->direction);
    if (f->action_state == FIGURE_ACTION_CORPSE) {
        f->image_id = image_group(IMAGE_GROUP_FIGURE_MARKET_BUYER_DEATH) + figure_corpse_offsets[f->wait_ticks / 2];
    } else {
        f->image_id = image_group(IMAGE_GROUP_FIGURE_MARKET_BUYER) + (f->animation_frame * DIRECTION_COUNT) + dir;
    }
}

int32_t delivery_boy_find_buyer(int32_t figure_id)
{
    int32_t id = figure_get(figure_id)->leading_figure_id;
    for (int32_t i = 0; i < 8; i++) {
        const figure *f = figure_get(id);
        if (f->type == FIGURE_MARKET_BUYER) {
            return id;
        }
        id = f->leading_figure_id;
    }
    return 0;
}

void figure_delivery_boy_action(void)
{
    figure *f = figure_get(figure_current_id);
    int32_t leader_id = f->leading_figure_id;
    int32_t home_id = f->building_id;
    int32_t buyer_id = delivery_boy_find_buyer(figure_current_id);
    int32_t figure_id = figure_current_id;
    f = figure_get(figure_id);
    f->is_ghost = 0;
    f->terrain_usage = TERRAIN_USAGE_ROADS;
    uint8_t frame = (uint8_t) (f->animation_frame + 1);
    f->animation_frame = frame;
    if (frame >= CARRIER_ANIMATION_FRAMES) {
        f->animation_frame = 0;
    }
    f->cart_image_id = 0;
    if (leader_id <= 0) {
        f->state = FIGURE_STATE_DEAD;
    } else {
        const figure *leader = figure_get(leader_id);
        if (leader->action_state == FIGURE_ACTION_CORPSE) {
            f->state = FIGURE_STATE_DEAD;
        } else if (leader->state != FIGURE_STATE_ALIVE) {
            f->state = FIGURE_STATE_DEAD;
            building_get(home_id)->data.market.inventory[f->collecting_item_id] += 100;
        } else if (leader->type != FIGURE_MARKET_BUYER && leader->type != FIGURE_DELIVERY_BOY) {
            f->state = FIGURE_STATE_DEAD;
        } else {
            int32_t buyer_state = figure_get(buyer_id)->action_state;
            if (buyer_state != FIGURE_ACTION_WAIT_FOR_FERRY && buyer_state != FIGURE_ACTION_ON_FERRY) {
                figure_movement_follow_ticks(figure_id, f->leading_figure_id, 1);
            }
        }
    }
    int32_t buyer_state = figure_get(buyer_id)->action_state;
    if (buyer_state == FIGURE_ACTION_WAIT_FOR_FERRY) {
        f->animation_frame = 0;
    }
    if (figure_get(buyer_id)->action_state == FIGURE_ACTION_ON_FERRY) {
        f->is_ghost = 1;
    }
    if (figure_get(leader_id)->is_ghost) {
        f->is_ghost = 1;
    }
    int32_t direction = f->direction;
    if (direction >= DIRECTION_AT_DESTINATION) {
        direction = f->previous_tile_direction;
    }
    int32_t dir = carrier_image_direction(direction);
    if (f->action_state == FIGURE_ACTION_CORPSE) {
        f->image_id = image_group(IMAGE_GROUP_FIGURE_DELIVERY_BOY) + DELIVERY_BOY_DEATH_IMAGES + figure_corpse_offsets[f->wait_ticks / 2];
    } else {
        f->image_id = image_group(IMAGE_GROUP_FIGURE_DELIVERY_BOY) + (f->animation_frame * DIRECTION_COUNT) + dir;
    }
}

static int32_t migrant_is_swimming(const figure *f)
{
    return (map_grids.terrain[f->grid_offset] & TERRAIN_WATER) && map_grids.sprite[f->grid_offset] == 0;
}

static void migrant_swimming_image(int32_t dying)
{
    figure *f = figure_get(figure_current_id);
    if (dying) {
        if (f->wait_ticks >= MIGRANT_DROWNING_TICKS) {
            int32_t frame = f->wait_ticks - MIGRANT_DROWNING_TICKS;
            if (frame > MIGRANT_LAST_BUBBLE_FRAME) {
                frame = MIGRANT_LAST_BUBBLE_FRAME;
            }
            f->image_id = image_group(IMAGE_GROUP_FIGURE_DEATH_IN_WATER) + frame;
            f->cart_image_id = 0;
            return;
        }
        int32_t group = map_sweep_has_passed(f->x) ? IMAGE_GROUP_FIGURE_EMIGRANT_SWEPT_DEATH : IMAGE_GROUP_FIGURE_FERRY_BOAT_SINKING;
        f->image_id = (figure_image_direction / DIRECTION_COUNT) + image_group(group) + (f->wait_ticks / MIGRANT_SWIM_FRAME_TICKS);
        f->cart_image_id = 0;
        return;
    }
    int32_t group = map_sweep_has_passed(f->x) ? IMAGE_GROUP_FIGURE_EMIGRANT_SWEPT : IMAGE_GROUP_FIGURE_FERRY_BOAT;
    f->image_id = image_group(group) + ((f->animation_frame / MIGRANT_SWIM_FRAME_TICKS) * DIRECTION_COUNT) + figure_image_direction;
    f->cart_image_id = 0;
}

static void migrant_enter_house(int32_t house_id, int32_t homeless)
{
    building *house = building_get(house_id);
    figure *f = figure_get(figure_current_id);
    int32_t max_people = house_models[house->subtype].max_people;
    if (house->house_is_merged) {
        max_people <<= 2;
    }
    int32_t room = max_people - house->house_population;
    if (room < 0) {
        room = 0;
    }
    if (room < f->migrant_num_people) {
        f->migrant_num_people = (uint8_t) room;
    }
    if (house->house_population == 0 && (homeless || houses_frozen == 0)) {
        building_house_change_to(house_id, BUILDING_HOUSE_CRUDE_HUT);
    }
    if (homeless || !house->data.house.is_sick) {
        int32_t people = f->migrant_num_people;
        house->house_population = (int16_t) (house->house_population + people);
        house->house_population_room = (int16_t) (max_people - house->house_population);
        if (homeless) {
            city_population_add_homeless(city_current_player, people);
        } else {
            city_population_add(city_current_player, people);
        }
    }
    house->immigrant_figure_id = 0;
}

void figure_immigrant_action(void)
{
    int32_t figure_id = figure_current_id;
    figure *f = figure_get(figure_id);
    f->terrain_usage = TERRAIN_USAGE_ANY;
    f->cart_image_id = 0;
    int32_t house_id = f->immigrant_building_id;
    int32_t speed = figure_properties[f->type].speed;
    int32_t swimming = migrant_is_swimming(f);
    building *house = building_get(house_id);
    if (house->state != BUILDING_STATE_IN_USE || house->immigrant_figure_id != figure_id || house->house_size == 0) {
        f->state = FIGURE_STATE_DEAD;
        return;
    }
    uint8_t frame = (uint8_t) (f->animation_frame + 1);
    f->animation_frame = frame;
    if (frame >= CARRIER_ANIMATION_FRAMES) {
        f->animation_frame = 0;
    }
    switch (f->action_state) {
        case FIGURE_ACTION_ATTACK:
            figure_combat_handle_attack(figure_id);
            break;
        case FIGURE_ACTION_CORPSE:
            figure_combat_handle_corpse(figure_id);
            break;
        case FIGURE_ACTION_IMMIGRANT_CREATED:
            f->is_ghost = 1;
            f->animation_frame = 0;
            if (--f->wait_ticks <= 0) {
                if (!map_closest_road_within_radius(house->x, house->y, house->size, 2)) {
                    f->state = FIGURE_STATE_DEAD;
                } else {
                    f->action_state = FIGURE_ACTION_IMMIGRANT_ARRIVING;
                    f->destination_x = map_result_x;
                    f->destination_y = map_result_y;
                    f->roam_length = 0;
                }
            }
            break;
        case FIGURE_ACTION_IMMIGRANT_ARRIVING:
            f->is_ghost = 0;
            figure_movement_move_ticks(figure_id, speed);
            if (f->direction == DIRECTION_REROUTE) {
                f->action_state = FIGURE_ACTION_IMMIGRANT_CREATED;
                f->wait_ticks = 1;
                figure_route_remove(figure_id);
            } else if (f->direction == DIRECTION_LOST) {
                house->immigrant_figure_id = 0;
                house->distance_from_entry = 0;
                f->state = FIGURE_STATE_DEAD;
            } else if (f->direction == DIRECTION_AT_DESTINATION) {
                f->action_state = FIGURE_ACTION_IMMIGRANT_ENTERING_HOUSE;
                f->destination_x = house->x;
                f->destination_y = house->y;
                figure_movement_set_cross_country_direction(
                    figure_id, f->cross_country.x, f->cross_country.y, f->destination_x * 15, f->destination_y * 15, 0);
                f->roam_length = 0;
            }
            break;
        case FIGURE_ACTION_IMMIGRANT_ENTERING_HOUSE:
            f->use_cross_country = 1;
            f->is_ghost = 1;
            if (figure_movement_move_ticks_cross_country(figure_id, 1) == 1) {
                f->state = FIGURE_STATE_DEAD;
                migrant_enter_house(house_id, 0);
            }
            f->is_ghost = f->in_building_wait_ticks ? 1 : 0;
            break;
        default:
            f->action_state = FIGURE_ACTION_IMMIGRANT_CREATED;
            break;
    }
    carrier_image_direction(f->direction);
    if (f->action_state == FIGURE_ACTION_CORPSE) {
        if (swimming) {
            migrant_swimming_image(1);
        } else {
            f->image_id = image_group(IMAGE_GROUP_FIGURE_IMMIGRANT_DEATH) + figure_corpse_offsets[f->wait_ticks / 2];
        }
        return;
    }
    if (swimming) {
        migrant_swimming_image(0);
        return;
    }
    f->image_id = image_group(IMAGE_GROUP_FIGURE_IMMIGRANT) + (f->animation_frame * DIRECTION_COUNT) + figure_image_direction;
    f->cart_image_id = 0;
}

static int32_t homeless_go_to_house(int32_t house_id)
{
    building *house = building_get(house_id);
    if (!map_closest_road_within_radius(house->x, house->y, house->size, 2)) {
        return 0;
    }
    figure *f = figure_get(figure_current_id);
    house->immigrant_figure_id = (int16_t) figure_current_id;
    f->immigrant_building_id = (int16_t) house_id;
    f->action_state = FIGURE_ACTION_HOMELESS_GOING_TO_HOUSE;
    f->destination_x = map_result_x;
    f->destination_y = map_result_y;
    f->roam_length = 0;
    return 1;
}

void figure_homeless_action(void)
{
    int32_t figure_id = figure_current_id;
    figure *f = figure_get(figure_id);
    int32_t speed = figure_properties[f->type].speed;
    int32_t swimming = migrant_is_swimming(f);
    uint8_t frame = (uint8_t) (f->animation_frame + 1);
    f->animation_frame = frame;
    if (frame >= CARRIER_ANIMATION_FRAMES) {
        f->animation_frame = 0;
    }
    f->terrain_usage = TERRAIN_USAGE_PREFER_ROADS;
    switch (f->action_state) {
        case FIGURE_ACTION_ATTACK:
            figure_combat_handle_attack(figure_id);
            break;
        case FIGURE_ACTION_CORPSE:
            figure_combat_handle_corpse(figure_id);
            break;
        case FIGURE_ACTION_HOMELESS_CREATED:
            f->animation_frame = 0;
            if (++f->wait_ticks > 51) {
                int32_t house_id = house_population_closest_house_with_room(f->x, f->y);
                if (house_id == 0) {
                    f->action_state = FIGURE_ACTION_HOMELESS_LEAVING;
                    const city_player *player = city_player_current();
                    f->destination_x = player->exit_point_x;
                    f->destination_y = player->exit_point_y;
                    f->roam_length = 0;
                    f->wait_ticks = 0;
                } else if (!homeless_go_to_house(house_id)) {
                    f->state = FIGURE_STATE_DEAD;
                }
            }
            break;
        case FIGURE_ACTION_HOMELESS_GOING_TO_HOUSE: {
            f->is_ghost = 0;
            figure_movement_move_ticks(figure_id, speed);
            building *house = building_get(f->immigrant_building_id);
            if (f->direction == DIRECTION_REROUTE || f->direction == DIRECTION_LOST) {
                f->state = FIGURE_STATE_DEAD;
                house->immigrant_figure_id = 0;
            } else if (f->direction == DIRECTION_AT_DESTINATION) {
                f->action_state = FIGURE_ACTION_HOMELESS_ENTERING_HOUSE;
                f->destination_x = house->x;
                f->destination_y = house->y;
                figure_movement_set_cross_country_direction(
                    figure_id, f->cross_country.x, f->cross_country.y, f->destination_x * 15, f->destination_y * 15, 0);
                f->roam_length = 0;
            }
            break;
        }
        case FIGURE_ACTION_HOMELESS_LEAVING:
            figure_movement_move_ticks(figure_id, speed);
            if (f->direction == DIRECTION_AT_DESTINATION || f->direction == DIRECTION_LOST) {
                f->state = FIGURE_STATE_DEAD;
            } else if (f->direction == DIRECTION_REROUTE) {
                figure_route_remove(figure_id);
            }
            if (++f->wait_ticks > 30) {
                f->wait_ticks = 0;
                int32_t house_id = house_population_closest_house_with_room(f->x, f->y);
                if (house_id > 0 && homeless_go_to_house(house_id)) {
                    figure_route_remove(figure_id);
                }
            }
            break;
        case FIGURE_ACTION_HOMELESS_ENTERING_HOUSE: {
            int32_t house_id = f->immigrant_building_id;
            f->use_cross_country = 1;
            f->is_ghost = 1;
            if (figure_movement_move_ticks_cross_country(figure_id, 1) == 1) {
                f->state = FIGURE_STATE_DEAD;
                if (house_id != 0) {
                    building *house = building_get(house_id);
                    if (house->type >= BUILDING_HOUSE_CRUDE_HUT && house->type <= BUILDING_HOUSE_PALATIAL_ESTATE && !house->data.house.is_sick) {
                        migrant_enter_house(house_id, 1);
                    }
                }
            }
            break;
        }
        default:
            f->action_state = FIGURE_ACTION_HOMELESS_CREATED;
            break;
    }
    carrier_image_direction(f->direction);
    if (f->action_state == FIGURE_ACTION_CORPSE) {
        f->image_id = image_group(IMAGE_GROUP_FIGURE_HOMELESS_DEATH) + figure_corpse_offsets[f->wait_ticks / 2];
        if (swimming) {
            migrant_swimming_image(1);
        }
        return;
    }
    if (swimming) {
        migrant_swimming_image(0);
        return;
    }
    f->image_id = image_group(IMAGE_GROUP_FIGURE_HOMELESS) + (f->animation_frame * DIRECTION_COUNT) + figure_image_direction;
}

int32_t criminal_find_rich_building(int32_t figure_id)
{
    route_candidates.count = 0;
    const city_player *player = city_player_current();
    int32_t has_savings = player->personal_savings > 0;
    int32_t has_treasury = player->treasury > 0;
    if (!has_savings && !has_treasury) {
        return 0;
    }
    for (int32_t id = 1; id < MAX_BUILDINGS; id++) {
        building *b = building_get(id);
        if (!b->state || !building_is_palace_or_mansion(id)) {
            continue;
        }
        if (building_is_mansion(id) && !has_savings) {
            continue;
        }
        if (building_is_palace(id) && !has_treasury) {
            continue;
        }
        if (b->tax_income_or_storage <= 0) {
            continue;
        }
        route_candidates.buildings[route_candidates.count] = id;
        route_candidates.count++;
        route_candidates.tiles[route_candidates.count - 1] = map_info.grid_start + b->road_access_x + b->road_access_y * GRID_SIZE;
    }
    if (route_candidates.count) {
        int32_t index = figure_route_add_to_closest_of(figure_id, route_candidates.tiles, route_candidates.count);
        if (index > 0) {
            return route_candidates.buildings[index - 1];
        }
    }
    return 0;
}
