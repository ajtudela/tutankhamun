// Figure core: creating and deleting figures (and clearing everything that refers to a deleted one), their names from
// the name groups, the figure lists of the map tiles, the attack target on a tile and the small type predicates.
//
// Creating and deleting figures touches more than it should:
// - deleting a cart reads the amount and destination of figure_current_id instead of the deleted cart;
// - a ship's formation id shares its byte with the flee / wait / reroute counters, so an enemy transport ship can name a
//   formation of 50 or more; removing it then clears four bytes where that record would be, past the formations
//   (formation_memory_past_records: in the figure records that follow them in the original);
// - a figure missing from its tile list ends the unlink walk on the scratch figure 0;
// - the Mitani soldiers' names wrap one name later.

#include "figure/figure_core.h"
#include "building/building.h"
#include "building/building_data.h"
#include "building/building_industry.h"
#include "building/building_type.h"
#include "building/ferry_landing.h"
#include "city/city_player.h"
#include "city/resource_type.h"
#include "core/crt_compat.h"
#include "core/queue.h"
#include "empire/empire.h"
#include "figure/enemy.h"
#include "figure/figure.h"
#include "figure/figure_action.h"
#include "figure/figure_data.h"
#include "figure/figure_type.h"
#include "figure/formation.h"
#include "figure/movement_internal.h"
#include "figure/route.h"
#include "figure/ships_player.h"
#include "figure/trade.h"
#include "io/io_data.h"
#include "map/grid.h"
#include <stdbool.h>
#include <string.h>

static void clear_formation_ship(int32_t formation_id)
{
    if (formation_id < MAX_FORMATIONS) {
        formation_get(formation_id)->ship_figure_id = 0;
        return;
    }
    uint8_t *past = formation_memory_past_records(((formation_id - MAX_FORMATIONS) * (int32_t) sizeof(formation)) + (int32_t) offsetof(formation, ship_figure_id));
    if (past) {
        memset(past, 0, sizeof(int32_t));
    }
}

bool figure_id_is_reserved(int32_t figure_id)
{
    for (int32_t i = 0; i < FERRY_QUEUES; i++) {
        ring_queue *queue = &ferry_queue_get(i)->queue;
        if (queue->in_use && ring_queue_contains(queue, figure_id)) {
            return 1;
        }
    }
    for (int32_t i = 0; i < FERRY_PATH_RECORD_COUNT; i++) {
        const ferry_path *record = &ferry_path_records[i];
        if (record->in_use) {
            for (int32_t j = 0; j < FERRY_PATH_FIGURES; j++) {
                if (record->passengers[j] == figure_id) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

int32_t figure_create(int32_t player, int32_t type, int32_t x, int32_t y, int32_t direction)
{
    for (int32_t id = 1; id < MAX_FIGURES; id++) {
        figure *f = figure_get(id);
        if (f->state || figure_id_is_reserved(id)) {
            continue;
        }
        memset(f, 0, sizeof(figure));
        f->state = FIGURE_STATE_ALIVE;
        f->faction_id = (uint8_t) player;
        f->type = (uint8_t) type;
        f->use_cross_country = 0;
        f->is_friendly = 1;
        f->request_event_id = -1;
        f->created_sequence = (int16_t) figure_created_sequence;
        f->direction = (int8_t) direction;
        figure_created_sequence++;
        f->destination_x = f->previous_tile_x = f->x = f->source_x = (int16_t) x;
        f->destination_y = f->previous_tile_y = f->y = f->source_y = (int16_t) y;
        f->grid_offset = map_info.grid_start + (y * GRID_SIZE) + x;
        f->cross_country.x = (int16_t) (f->source_x * 15);
        f->cross_country.y = (int16_t) (f->source_y * 15);
        f->progress_on_tile = 15;
        f->ship_formation_id = 0;
        int32_t r = crt_rand();
        f->phrase_sequence_city = (uint8_t) (r & 3);
        f->phrase_sequence_exact = (uint8_t) (r & 3);
        figure_name_set(id);
        map_figure_add(id);
        if (type == FIGURE_TRADE_CARAVAN || type == FIGURE_TRADE_SHIP) {
            trader_create(id);
        }
        if (id > figure_highest_id) {
            figure_highest_id = id;
        }
        return id;
    }
    return 0;
}

static int32_t inventory_resource(int32_t item)
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
        default:
            return -1;
    }
}

static int32_t counts_carts(int32_t type)
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

static void delete_cart(const figure *f)
{
    const figure *current = figure_get(figure_current_id);
    building *own = building_get(f->destination_building_id);
    building *b = building_get(current->destination_building_id);
    if (counts_carts(own->type) && current->resource_amount) {
        int32_t resource = inventory_resource(b->second_input_resource);
        if (own->type == BUILDING_BRICKWORKS && f->resource_id == resource) {
            own->incoming_loads--;
        } else {
            own->incoming_carts--;
        }
        return;
    }
    int16_t amount = current->resource_amount;
    if (b->type == BUILDING_GRANARY && amount) {
        b->incoming_carts--;
        b->incoming_loads -= amount;
    }
}

static void delete_references(int32_t figure_id, figure *f)
{
    city_player *player = city_player_current();
    int32_t type = f->type;
    if (type == FIGURE_KINGDOM_SOLDIER) {
        player->kingdom_soldiers_removed++;
    }
    if (type == FIGURE_ENEMY_WARSHIP || type == FIGURE_ENEMY_EGYPTIAN_WARSHIP) {
        player->enemy_warships--;
    }
    if ((type == FIGURE_ENEMY_TRANSPORT_SHIP || type == FIGURE_ENEMY_EGYPTIAN_TRANSPORT_SHIP || type == FIGURE_TRANSPORT_SHIP) && f->ship_formation_id) {
        clear_formation_ship(f->ship_formation_id);
    }
    building *home = building_get(f->building_id);
    switch (type) {
        case FIGURE_LABOR_SEEKER:
        case FIGURE_MARKET_BUYER:
            if (f->building_id) {
                home->figure_id2 = 0;
            }
            return;
        case FIGURE_WARSHIP:
        case FIGURE_TRANSPORT_SHIP:
            ship_leave_dock_queue(figure_id);
            return;
        case FIGURE_BALLISTA:
            home->figure_id4 = 0;
            return;
        case FIGURE_EXPLOSION:
        case FIGURE_STANDARD_BEARER:
        case FIGURE_ARROW:
        case FIGURE_JAVELIN:
        case FIGURE_BOLT:
        case FIGURE_SPEAR:
        case FIGURE_SEAGULLS:
        case FIGURE_BIRDS:
        case FIGURE_CROCODILE:
        case FIGURE_ANTELOPE:
        case FIGURE_HIPPO:
        case FIGURE_HYENA:
        case FIGURE_LION:
        case FIGURE_SCORPION:
        case FIGURE_ASP:
        case FIGURE_MUMMY:
        case FIGURE_DELIVERY_BOY:
        case FIGURE_PATRICIAN:
            return;
        case FIGURE_LUMBERJACK:
        case FIGURE_REED_GATHERER:
            home->data.industry.worker_count--;
            return;
        case FIGURE_HUNTER:
            if (home->data.industry.worker_count) {
                home->data.industry.worker_count--;
                return;
            }
            break;
    }
    if (type == FIGURE_PLAGUED_CITIZEN) {
        player->plague_count = (player->plague_count > 0) ? (player->plague_count - 1) : 0;
        return;
    }
    if (type == FIGURE_JUGGLER) {
        home->data.entertainment.juggler_id = 0;
        return;
    }
    if (type == FIGURE_MUSICIAN) {
        home->data.entertainment.musician_id = 0;
        return;
    }
    if (type == FIGURE_DANCER) {
        home->data.entertainment.dancer_id = 0;
        return;
    }
    if (type == FIGURE_LABORER) {
        int32_t farm_id = f->destination_building_id;
        if (building_is_floodplain_farm(farm_id)) {
            building_get(farm_id)->data.industry.worker_figure_id = 0;
        }
        return;
    }
    if (type == FIGURE_DOCKER) {
        for (int32_t i = 0; i < BUILDING_DOCK_DOCKERS; i++) {
            int16_t *slot = &home->data.dock.docker_ids[i];
            if (*slot == figure_id) {
                *slot = 0;
            }
        }
        return;
    }
    if (!f->building_id) {
        return;
    }
    if (home->figure_id == figure_id) {
        home->figure_id = 0;
    } else if (home->data.storage_yard.second_cart_id == figure_id) {
        home->data.storage_yard.second_cart_id = 0;
    }
    if (type == FIGURE_CART_PUSHER || type == FIGURE_STORAGE_YARD_CART) {
        delete_cart(f);
    }
}

void figure_delete(int32_t figure_id)
{
    figure *f = figure_get(figure_id);
    delete_references(figure_id, f);
    if (f->empire_city_id) {
        empire_city *city = empire_city_edit(f->empire_city_id);
        city->trader_blocked = 0;
        if (city->trader_figures[0] == figure_id) {
            city->trader_figures[0] = 0;
        } else if (city->trader_figures[1] == figure_id) {
            city->trader_figures[1] = 0;
        } else if (city->trader_figures[2] == figure_id) {
            city->trader_figures[2] = 0;
        }
    }
    if (f->ferry_path_index) {
        ferry_path_records[f->ferry_path_index].in_use = 0;
    }
    if (f->immigrant_building_id) {
        building_get(f->building_id)->immigrant_figure_id = 0;
    }
    figure_route_remove(figure_id);
    map_figure_delete(figure_id);
    memset(f, 0, sizeof(figure));
}

static int32_t *name_counter(int32_t group)
{
    int32_t slot = (group >= FIGURE_NAME_GROUP_ENEMY_EXTRA_FIRST) ? (FIGURE_NAME_GROUP_ENEMY_EXTRA_LAST - group) : (FIGURE_NAME_GROUP_COUNTER_BASE - group);
    return &figure_name_counter_table.by_slot[slot];
}

figure_name_group figure_name_groups[FIGURE_TYPE_COUNT] = {
    [FIGURE_IMMIGRANT] = { 0xFE, 0x80 },
    [FIGURE_EMIGRANT] = { 0xFE, 0x80 },
    [FIGURE_HOMELESS] = { 0xFF, 0x20 },
    [FIGURE_CART_PUSHER] = { 0xFE, 0x80 },
    [FIGURE_LABOR_SEEKER] = { 0xFE, 0x80 },
    [FIGURE_TAX_COLLECTOR] = { 0x115, 0x10 },
    [FIGURE_ARCHITECT] = { 0x113, 0x10 },
    [FIGURE_STORAGE_YARD_CART] = { 0xFE, 0x80 },
    [FIGURE_FIREMAN] = { 0x112, 0x10 },
    [FIGURE_JUGGLER] = { 0x107, 0x20 },
    [FIGURE_MUSICIAN] = { 0x108, 0x20 },
    [FIGURE_DANCER] = { 0x109, 0x10 },
    [FIGURE_SENET_PLAYER] = { 0x107, 0x20 },
    [FIGURE_TRADE_CARAVAN] = { 0x106, 0x10 },
    [FIGURE_TRADE_SHIP] = { 0x105, 0x10 },
    [FIGURE_TRADE_CARAVAN_DONKEY] = { 0x106, 0x10 },
    [FIGURE_PROTESTER] = { 0x100, 0x10 },
    [FIGURE_CRIMINAL] = { 0x100, 0x10 },
    [FIGURE_MARKET_TRADER] = { 0x103, 0x20 },
    [FIGURE_TEACHER] = { 0x10F, 0x20 },
    [FIGURE_LIBRARIAN] = { 0x10F, 0x20 },
    [FIGURE_DENTIST] = { 0x110, 0x20 },
    [FIGURE_PHYSICIAN] = { 0x110, 0x20 },
    [FIGURE_HERBALIST] = { 0x110, 0x20 },
    [FIGURE_EMBALMER] = { 0x111, 0x10 },
    [FIGURE_WORKER] = { 0xFE, 0x80 },
    [FIGURE_DOCKER] = { 0xFE, 0x80 },
    [FIGURE_MARKET_BUYER] = { 0x103, 0x20 },
    [FIGURE_PATRICIAN] = { 0x101, 0x20 },
    [FIGURE_TOWER_SENTRY] = { 0x118, 0x20 },
    [FIGURE_ENEMY_EGYPTIAN_INFANTRY] = { 0xFE, 0x80 },
    [FIGURE_ENEMY_EGYPTIAN_ARCHER] = { 0xFE, 0x80 },
    [FIGURE_ENEMY_EGYPTIAN_CHARIOT] = { 0xFE, 0x80 },
    [FIGURE_DELIVERY_BOY] = { 0x104, 0x10 },
    [FIGURE_HUNTER] = { 0x102, 0x10 },
    [FIGURE_LUMBERJACK] = { 0xFE, 0x80 },
    [FIGURE_FERRY_BOAT] = { 0x117, 0x10 },
    [FIGURE_CARPENTER] = { 0xFE, 0x80 },
    [FIGURE_BRICKLAYER] = { 0xFE, 0x80 },
    [FIGURE_STONEMASON] = { 0xFE, 0x80 },
    [FIGURE_LABORER] = { 0xFE, 0x80 },
    [FIGURE_WATER_CARRIER] = { 0xFE, 0x80 },
    [FIGURE_CONSTABLE] = { 0x114, 0x10 },
    [FIGURE_MAGISTRATE] = { 0x116, 0x10 },
    [FIGURE_REED_GATHERER] = { 0xFE, 0x80 },
    [FIGURE_ENEMY_TRANSPORT_SHIP] = { 0x121, 0x10 },
    [FIGURE_ENEMY_WARSHIP] = { 0x121, 0x10 },
    [FIGURE_SLED_PULLER] = { 0xFE, 0x80 },
    [FIGURE_PLAGUED_CITIZEN] = { 0xFE, 0x80 },
    [FIGURE_BEDOUIN] = { 0x122, 0x10 },
    [FIGURE_ENEMY_EGYPTIAN_WARSHIP] = { 0x121, 0x10 },
    [FIGURE_ENEMY_EGYPTIAN_TRANSPORT_SHIP] = { 0x121, 0x10 },
    [FIGURE_ZOOKEEPER] = { 0x107, 0x20 },
    [FIGURE_TOMB_ARTISAN] = { 0xFE, 0x80 },
};

figure_name_group enemy_name_groups[ENEMY_EGYPTIAN] = {
    [ENEMY_HYKSOS] = { 0x11D, 0x10 },
    [ENEMY_SEA_PEOPLE] = { 0x11F, 0x10 },
    [ENEMY_HITTITE] = { 0x11E, 0x10 },
    [ENEMY_MITANI] = { 0x120, 0x11 },
    [ENEMY_KUSHITE] = { 0x11C, 0x10 },
    [ENEMY_LIBYAN] = { 0x11B, 0x10 },
    [ENEMY_NUBIAN] = { 0x11A, 0x10 },
    [ENEMY_CANAANITE] = { 0x119, 0x10 },
    [ENEMY_ASSYRIAN] = { 0x12C, 0x10 },
    [ENEMY_ROMAN] = { 0x12D, 0x10 },
    [ENEMY_PHOENICIAN] = { 0x12E, 0x10 },
    [ENEMY_PERSIAN] = { 0x12F, 0x10 },
};

static const figure_name_group *usable(const figure_name_group *group)
{
    int32_t text = group->group;
    int32_t has_counter = (text >= FIGURE_NAME_GROUP_FIRST && text <= FIGURE_NAME_GROUP_COUNTER_BASE) ||
        (text >= FIGURE_NAME_GROUP_ENEMY_EXTRA_FIRST && text <= FIGURE_NAME_GROUP_ENEMY_EXTRA_LAST);
    return (group->count > 0 && has_counter) ? group : 0;
}

static const figure_name_group *name_group_of(int32_t type, const figure *f)
{
    if (type == FIGURE_ENEMY_ARCHER || type == FIGURE_ENEMY_INFANTRY || type == FIGURE_ENEMY_CHARIOT) {
        int32_t enemy = formation_get(f->formation_id)->enemy_type;
        return (enemy < ENEMY_EGYPTIAN) ? usable(&enemy_name_groups[enemy]) : 0;
    }
    if (type < 0 || type >= FIGURE_TYPE_COUNT) {
        return 0;
    }
    return usable(&figure_name_groups[type]);
}

void figure_name_set(int32_t figure_id)
{
    figure *f = figure_get(figure_id);
    int32_t type = f->type;
    if (type == FIGURE_EXPLOSION || type == FIGURE_STANDARD_BEARER || type == FIGURE_SEAGULLS || type == FIGURE_FISHING_SPOT || type == FIGURE_CREATURE ||
        type == FIGURE_CHARIOT_RACER || type == FIGURE_SHOWMAN) {
        return;
    }
    const figure_name_group *group = name_group_of(type, f);
    if (!group) {
        if (type >= FIGURE_ENEMY_ARCHER && type <= FIGURE_ENEMY_CHARIOT) {
            f->name_id = -1;
            f->name_group = -1;
        } else {
            f->name_group = -2;
        }
        return;
    }
    int32_t *counter = name_counter(group->group);
    int32_t name = *counter;
    f->name_id = (int16_t) name;
    f->name_group = group->group;
    *counter = ((name + 1) < group->count) ? (name + 1) : 0;
}

void map_figure_add(int32_t figure_id)
{
    figure *f = figure_get(figure_id);
    if (f->grid_offset < 0) {
        return;
    }
    int32_t id = map_grids.figure_ids[f->grid_offset];
    f->index_on_same_tile = 0;
    if (!id) {
        map_grids.figure_ids[f->grid_offset] = (int16_t) figure_id;
        return;
    }
    f->index_on_same_tile = 1;
    figure *last = figure_get(id);
    while (last->next_figure_id_on_same_tile) {
        id = last->next_figure_id_on_same_tile;
        if (id == figure_id) {
            return;
        }
        f->index_on_same_tile++;
        last = figure_get(id);
    }
    if (f->index_on_same_tile > FIGURE_TILE_INDEX_MAX) {
        f->index_on_same_tile = FIGURE_TILE_INDEX_MAX;
    }
    last->next_figure_id_on_same_tile = (int16_t) figure_id;
}

void map_figure_update(int32_t figure_id)
{
    figure *f = figure_get(figure_id);
    f->index_on_same_tile = 0;
    int32_t id = map_grids.figure_ids[f->grid_offset];
    while (id) {
        if (id == figure_id) {
            return;
        }
        f->index_on_same_tile++;
        id = figure_get(id)->next_figure_id_on_same_tile;
    }
    if (f->index_on_same_tile > FIGURE_TILE_INDEX_MAX) {
        f->index_on_same_tile = FIGURE_TILE_INDEX_MAX;
    }
}

void map_figure_delete(int32_t figure_id)
{
    figure *f = figure_get(figure_id);
    if (f->grid_offset < 0) {
        return;
    }
    int32_t head = map_grids.figure_ids[f->grid_offset];
    if (!head) {
        return;
    }
    if (head == figure_id) {
        map_grids.figure_ids[f->grid_offset] = f->next_figure_id_on_same_tile;
        f->next_figure_id_on_same_tile = 0;
        return;
    }
    int32_t prev_id = head;
    figure *prev = figure_get(prev_id);
    int32_t next = prev->next_figure_id_on_same_tile;
    while (next != figure_id) {
        if (!prev_id) {
            return;
        }
        prev_id = next;
        prev = figure_get(prev_id);
        next = prev->next_figure_id_on_same_tile;
    }
    prev->next_figure_id_on_same_tile = f->next_figure_id_on_same_tile;
    f->next_figure_id_on_same_tile = 0;
}

int32_t map_figure_of_type_at(int32_t figure_id, int32_t type)
{
    int32_t id = map_grids.figure_ids[figure_get(figure_id)->grid_offset];
    while (id > 0) {
        const figure *other = figure_get(id);
        if (other->type == type) {
            return id;
        }
        id = other->next_figure_id_on_same_tile;
    }
    return 0;
}

int32_t map_figure_count_at(int32_t grid_offset)
{
    int32_t count = 0;
    int32_t id = map_grids.figure_ids[grid_offset];
    while (id != 0) {
        count++;
        id = city_figures.records[id].next_figure_id_on_same_tile;
    }
    return count;
}

static int32_t is_attack_target(const figure *f)
{
    if (f->action_state == FIGURE_ACTION_CORPSE) {
        return 0;
    }
    if (f->type >= FIGURE_ENEMY_ARCHER && f->type <= FIGURE_KINGDOM_SOLDIER) {
        return 1;
    }
    switch (f->type) {
        case FIGURE_NATIVE:
            return f->action_state == FIGURE_ACTION_NATIVE_ATTACKING;
        case FIGURE_CROCODILE:
        case FIGURE_HYENA:
        case FIGURE_HIPPO:
        case FIGURE_ENEMY_TRANSPORT_SHIP:
        case FIGURE_ENEMY_WARSHIP:
        case FIGURE_BEDOUIN:
        case FIGURE_ENEMY_EGYPTIAN_WARSHIP:
        case FIGURE_ENEMY_EGYPTIAN_TRANSPORT_SHIP:
        case FIGURE_ASP:
        case FIGURE_LION:
        case FIGURE_SCORPION:
        case FIGURE_MUMMY:
            return 1;
    }
    return 0;
}

int32_t map_figure_find_target(int32_t figure_id)
{
    int32_t id = map_grids.figure_ids[figure_get(figure_id)->grid_offset];
    while (id > 0) {
        const figure *f = figure_get(id);
        if (is_attack_target(f)) {
            return id;
        }
        id = f->next_figure_id_on_same_tile;
    }
    return 0;
}

bool figure_is_herd_animal(int32_t type)
{
    return type >= FIGURE_BIRDS && type <= FIGURE_ANTELOPE;
}

bool figure_is_predator_animal(int32_t type)
{
    switch (type) {
        case FIGURE_CROCODILE:
        case FIGURE_HYENA:
        case FIGURE_HIPPO:
        case FIGURE_ASP:
        case FIGURE_LION:
        case FIGURE_SCORPION:
            return 1;
    }
    return 0;
}

bool figure_is_boat(int32_t type)
{
    switch (type) {
        case FIGURE_FISHING_BOAT:
        case FIGURE_TRANSPORT_SHIP:
        case FIGURE_WARSHIP:
        case FIGURE_ENEMY_TRANSPORT_SHIP:
        case FIGURE_ENEMY_WARSHIP:
        case FIGURE_ENEMY_EGYPTIAN_WARSHIP:
        case FIGURE_ENEMY_EGYPTIAN_TRANSPORT_SHIP:
            return 1;
    }
    return 0;
}
