// Loading helpers: the upgrades of saves older than the current version, version by version, the chunk
// converters, the empire route paths drawn after loading, the dock queues kept in the save, and the fixes run after
// every load (figure name groups, granary capacity, the festival square).
//
// The statue upgrade never resets the old flag bit, so after the first statue that had it every later one gets the new
// flag too; the route path after loading is computed in a float and double mix of its own, so it differs from the empire
// map's.

#include "io/game_file_upgrade.h"

#include "building/building.h"
#include "building/building_figure_helpers.h"
#include "building/building_storage.h"
#include "building/building_type.h"
#include "building/monument_data.h"
#include "building/monument_tables.h"
#include "building/obelisk.h"
#include "city/city_player.h"
#include "city/gods.h"
#include "city/message.h"
#include "city/population.h"
#include "city/resource_type.h"
#include "core/queue.h"
#include "core/static_init.h"
#include "empire/empire_data.h"
#include "figure/enemy.h"
#include "figure/figure.h"
#include "figure/figure_action.h"
#include "figure/figure_core.h"
#include "figure/figure_type.h"
#include "figure/formation.h"
#include "figure/ships_player.h"
#include "game/mission_records.h"
#include "io/empire_file.h"
#include "io/game_file_internal.h"
#include "scenario/event_manager.h"
#include "scenario/scenario_event_types.h"

#include <assert.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

enum upgrade_rule {
    EVENT_REASON_BEFORE_EVENT_FIELDS = 6,
    RESOURCE_RENUMBERED_FROM_BEFORE_SCENARIO_EVENTS = 31,
    RESOURCE_RENUMBER_SHIFT_BEFORE_SCENARIO_EVENTS = 5,
    TRADE_PRICE_FIELDS = 2,
    STATUE_FLAG_BEFORE_BUILDING_ORIENTATION = 0x40,
    STATUE_FLAG = 0x10,
    STATUE_ORIENTATIONS = 4,
    CITY_PLAYERS = 2,
    STORAGE_AMOUNT_STEP = 800,
    STORAGE_AMOUNT_MAX = 3200,
    DOCK_QUEUE_RECORDS = 10,
};

enum monument_parts {
    SUN_TEMPLE_PARTS = 0x64,
    MAUSOLEUM_PARTS = 0x74,
    ALEXANDRIA_LIBRARY_PARTS = 0x19,
    SMALL_ROYAL_TOMB_PARTS = 0xA7,
    MEDIUM_ROYAL_TOMB_PARTS = 0x9E,
    LARGE_ROYAL_TOMB_PARTS = 0x1B5,
    GRAND_ROYAL_TOMB_PARTS = 0x201,
};

enum player_layout_before_relayout {
    PLAYER_BEFORE_PLAYER_BLOCK_COPIED_FROM = 0x2900,
    PLAYER_BEFORE_PLAYER_BLOCK_COPIED_TO = 0x483C,
    PLAYER_BEFORE_PLAYER_BLOCK_COPIED_SIZE = 7 * 4,
    PLAYER_BEFORE_PLAYER_COUNTS_A_FROM = 0x27DE,
    PLAYER_BEFORE_PLAYER_COUNTS_A_TO = 0x48F0,
    PLAYER_BEFORE_PLAYER_COUNTS_B_FROM = 0x2796,
    PLAYER_BEFORE_PLAYER_COUNTS_B_TO = 0x4860,
    PLAYER_BEFORE_PLAYER_COUNTS = 30,
    PLAYER_BEFORE_PLAYER_COUNTS_2_WORKSHOP_STORED = 0x291C,
    PLAYER_BEFORE_PLAYER_COUNTS_2_WORKSHOP_SPACE = 0x2944,
    PLAYER_BEFORE_PLAYER_COUNTS_2_WORKSHOPS = 20,
};

enum events_layout_before_event_relayout {
    EVENTS_BEFORE_EVENT_RELAYOUT_FIRST = 4,
    EVENT_BEFORE_EVENT_RELAYOUT_SIZE = 0x68,
    EVENTS_BEFORE_EVENT_RELAYOUT_RECORDS_END = EVENTS_BEFORE_EVENT_RELAYOUT_FIRST + (SCENARIO_MAX_EVENTS * EVENT_BEFORE_EVENT_RELAYOUT_SIZE),
    EVENTS_BEFORE_EVENT_RELAYOUT_EDITOR_SIZE = 8,
    EVENTS_BEFORE_EVENT_RELAYOUT_EARTHQUAKE_ACTIVE = EVENTS_BEFORE_EVENT_RELAYOUT_RECORDS_END + EVENTS_BEFORE_EVENT_RELAYOUT_EDITOR_SIZE,
};

typedef struct {
    int32_t param1;
    int16_t year;
    uint16_t param2;
    int16_t type;
    int16_t sequence;
    uint8_t is_read;
    uint8_t month;
    int16_t phrases[15];
} city_message_before_message_relayout;

static_assert(sizeof(city_message_before_message_relayout) == 0x2C, "city_message_before_message_relayout is an old savegame's record of 44 bytes");
static_assert(offsetof(city_message_before_message_relayout, phrases) == 0xE, "city_message_before_message_relayout is the old savegame's layout");
static_assert(offsetof(scenario_event, parent_item) == 0x66 && offsetof(scenario_event, parent_amount) == 0x68 && offsetof(scenario_event, reason) == 0x6A,
              "scenario_event is the savegame and scenario layout");
static_assert(offsetof(scenario_events_t, editor_selected_id) == 0x48AC, "scenario_events_t is the savegame and scenario layout");

static uint8_t *player_bytes(int32_t player)
{
    return (uint8_t *) city_player_get(player);
}

static void widen_counts(uint8_t *record, int32_t to_a, int32_t from_a, int32_t to_b, int32_t from_b, int32_t count)
{
    for (int32_t i = 0; i < count; i++) {
        int16_t a;
        int16_t b;
        memcpy(&a, record + from_a + (i * sizeof(int16_t)), sizeof(a));
        memcpy(&b, record + from_b + (i * sizeof(int16_t)), sizeof(b));
        int32_t wide_a = a;
        int32_t wide_b = b;
        memcpy(record + to_a + (i * sizeof(int32_t)), &wide_a, sizeof(wide_a));
        memcpy(record + to_b + (i * sizeof(int32_t)), &wide_b, sizeof(wide_b));
    }
}

static int32_t draw_route_segment(empire_route *r, int32_t to, int32_t at)
{
    int32_t x0 = r->points[to - 1].x;
    int32_t y0 = r->points[to - 1].y;
    int32_t dx = r->points[to].x - x0;
    int32_t dy = r->points[to].y - y0;
    float distance = (float) sqrt(((double) dx * dx) + ((double) dy * dy));
    if ((distance == 0.0f) || !r->step) {
        return 0;
    }
    float step_y = (float) ((double) dy / distance);
    float step_x = (float) ((double) dx / distance);
    float y0f = (float) y0;
    int32_t added = 0;
    int32_t t = 0;
    do {
        empire_route_point *p = &empire_route_path(r)[at + added];
        p->x = (int16_t) (int32_t) (((double) t * step_x) + (double) x0);
        p->y = (int16_t) (int32_t) (((double) t * step_y) + (double) y0f);
        p->segment = (uint8_t) (to - 1);
        t += r->step;
        added++;
    } while ((float) t <= distance);
    return added;
}

void game_file_upgrade_empire_object_states(void)
{
    for (int32_t i = 0; i < EMPIRE_OBJECT_COUNT; i++) {
        empire_object *o = &empire_map_objects[i];
        if (!o->in_use || o->type != EMPIRE_OBJECT_CITY) {
            continue;
        }
        switch (o->city_type) {
            case EMPIRE_CITY_BEFORE_OBJECT_STATES_EGYPTIAN_0:
            case EMPIRE_CITY_BEFORE_OBJECT_STATES_EGYPTIAN_3:
            case EMPIRE_CITY_BEFORE_OBJECT_STATES_EGYPTIAN_5:
                o->city_type = EMPIRE_CITY_EGYPTIAN;
                break;
            case EMPIRE_CITY_BEFORE_OBJECT_STATES_OURS:
                o->city_type = EMPIRE_CITY_OURS;
                break;
            case EMPIRE_CITY_BEFORE_OBJECT_STATES_PHARAOH_TRADING:
                o->city_type = EMPIRE_CITY_PHARAOH_TRADING;
                break;
            case EMPIRE_CITY_BEFORE_OBJECT_STATES_FOREIGN_4:
            case EMPIRE_CITY_BEFORE_OBJECT_STATES_FOREIGN_6:
                o->city_type = EMPIRE_CITY_FOREIGN;
                break;
            default:
                break;
        }
    }
}

void game_file_upgrade_clear_formation_field(void)
{
    for (int32_t id = 1; id < MAX_FORMATIONS; id++) {
        formation_get(id)->request_event_id = -1;
    }
}

void game_file_upgrade_clear_message_fields(void)
{
    for (int32_t i = 0; i < MAX_MESSAGES; i++) {
        city_message *m = &city_messages[i];
        m->phrases[0] = -1;
        m->phrases[1] = -1;
        m->phrases[2] = -1;
    }
}

void game_file_upgrade_copy_player_block(void)
{
    for (int32_t p = 0; p < CITY_PLAYERS; p++) {
        uint8_t *player = player_bytes(p);
        memcpy(player + PLAYER_BEFORE_PLAYER_BLOCK_COPIED_TO, player + PLAYER_BEFORE_PLAYER_BLOCK_COPIED_FROM, PLAYER_BEFORE_PLAYER_BLOCK_COPIED_SIZE);
    }
}

void game_file_upgrade_widen_player_counts(void)
{
    for (int32_t p = 0; p < CITY_PLAYERS; p++) {
        uint8_t *player = player_bytes(p);
        widen_counts(player, PLAYER_BEFORE_PLAYER_COUNTS_A_TO, PLAYER_BEFORE_PLAYER_COUNTS_A_FROM, PLAYER_BEFORE_PLAYER_COUNTS_B_TO, PLAYER_BEFORE_PLAYER_COUNTS_B_FROM,
            PLAYER_BEFORE_PLAYER_COUNTS);
    }
}

void game_file_upgrade_empire_object_flags(void)
{
    for (int32_t i = 0; i < EMPIRE_OBJECT_COUNT; i++) {
        empire_object *o = &empire_map_objects[i];
        const empire_object_before_object_flags *old = (const empire_object_before_object_flags *) o;
        empire_object upgraded;
        memset(&upgraded, 0, sizeof(upgraded));
        memcpy(&upgraded, old, sizeof(*old));
        for (int32_t resource = 0; resource < EMPIRE_OBJECT_BEFORE_OBJECT_FLAGS_MASKED_RESOURCES; resource++) {
            uint32_t bit = 1u << resource;
            uint8_t amount;
            if (old->resources_traded_high & bit) {
                amount = TRADE_AMOUNT_HIGH;
            } else if (old->resources_traded_medium & bit) {
                amount = TRADE_AMOUNT_MEDIUM;
            } else if (old->resources_traded_low & bit) {
                amount = TRADE_AMOUNT_LOW;
            } else {
                amount = TRADE_AMOUNT_NONE;
            }
            upgraded.trade_amount[resource] = amount;
        }
        int32_t unmasked = RESOURCE_MAX - EMPIRE_OBJECT_BEFORE_OBJECT_FLAGS_MASKED_RESOURCES;
        memset(&upgraded.trade_amount[EMPIRE_OBJECT_BEFORE_OBJECT_FLAGS_MASKED_RESOURCES], TRADE_AMOUNT_MEDIUM, unmasked);
        *o = upgraded;
    }
}

void game_file_upgrade_widen_player_counts2(void)
{
    for (int32_t p = 0; p < CITY_PLAYERS; p++) {
        city_player *player = city_player_get(p);
        widen_counts(player_bytes(p), offsetof(city_player, workshop_space), PLAYER_BEFORE_PLAYER_COUNTS_2_WORKSHOP_SPACE, offsetof(city_player, workshop_stored),
            PLAYER_BEFORE_PLAYER_COUNTS_2_WORKSHOP_STORED, PLAYER_BEFORE_PLAYER_COUNTS_2_WORKSHOPS);
        memset(&player->workshop_space[PLAYER_BEFORE_PLAYER_COUNTS_2_WORKSHOPS], 0, PLAYER_BEFORE_PLAYER_COUNTS_2_WORKSHOPS * sizeof(int32_t));
        memset(&player->workshop_stored[PLAYER_BEFORE_PLAYER_COUNTS_2_WORKSHOPS], 0, PLAYER_BEFORE_PLAYER_COUNTS_2_WORKSHOPS * sizeof(int32_t));
    }
}

void game_file_upgrade_message_text_ids(void)
{
    for (int32_t i = 0; i < MAX_MESSAGES; i++) {
        city_message *m = &city_messages[i];
        if (m->type == 0) {
            return;
        }
        m->type = (int16_t) city_message_get_text_id(m->type);
    }
}

void game_file_upgrade_clear_figure_fields(void)
{
    for (int32_t id = 1; id < MAX_FIGURES; id++) {
        figure *f = figure_get(id);
        f->answers_requests = 0;
        f->request_event_id = -1;
    }
}

void game_file_upgrade_obelisk_work_spots(void)
{
    int32_t id = obelisk_find_incomplete();
    if (id) {
        obelisk_reset_work_spots(id);
    }
}

static void reset_legion_records(mission_carry_records *records)
{
    for (int32_t kind = 0; kind < MISSION_LEGION_KINDS; kind++) {
        mission_legion_record *legion = &records->legions[kind];
        legion->experience = -1;
        legion->soldiers = 0;
        legion->legion_id = MISSION_LEGION_NONE;
    }
}

void game_file_reset_mission_records(void)
{
    mission_carry_records *started = mission_started_records();
    started->savings = 0;
    reset_legion_records(started);
    reset_legion_records(mission_finished_records());
}

static int32_t figure_is_alive(const figure *f)
{
    return (f->state == FIGURE_STATE_ALIVE) && (f->action_state != FIGURE_ACTION_CORPSE);
}

void game_file_upgrade_figure_targets(void)
{
    for (int32_t id = 1; id < MAX_FIGURES; id++) {
        figure *f = figure_get(id);
        if ((f->type == FIGURE_WARSHIP) && figure_is_alive(f)) {
            f->patrol_x = f->x;
            f->patrol_y = f->y;
        }
    }
}

static int32_t is_transport_ship(int32_t type)
{
    return (type == FIGURE_TRANSPORT_SHIP) || (type == FIGURE_ENEMY_TRANSPORT_SHIP) || (type == FIGURE_ENEMY_EGYPTIAN_TRANSPORT_SHIP);
}

void game_file_upgrade_formation_figures(void)
{
    for (int32_t i = 0; i < MAX_FORMATIONS; i++) {
        formation *m = formation_get(i);
        if (!is_transport_ship(figure_get(m->ship_figure_id)->type)) {
            m->ship_figure_id = 0;
        }
    }
    for (int32_t id = 1; id < MAX_FIGURES; id++) {
        const figure *f = figure_get(id);
        if ((f->type == FIGURE_TRANSPORT_SHIP) && figure_is_alive(f) && f->has_troops_on_board) {
            formation_get(f->ship_formation_id)->ship_figure_id = id;
        }
    }
}

void game_file_upgrade_event_fields(void)
{
    for (int32_t i = 0; i < SCENARIO_MAX_EVENTS; i++) {
        uint8_t *reason = scenario_events.events[i].reason;
        memset(reason, EVENT_REASON_BEFORE_EVENT_FIELDS, sizeof(scenario_events.events[i].reason));
    }
}

static int32_t pyramid_parts(const building *b)
{
    int32_t size = b->data.pyramid.size_variant;
    return (scenario_pyramid_style == PYRAMID_STYLE_MASTABA) ? mastaba_part_count[size] : pyramid_part_count[size];
}

static int32_t clear_monument_completion_shown(building *b)
{
    switch (b->type) {
        case BUILDING_PYRAMID:
            b->data.pyramid.completion_shown = 0;
            return pyramid_parts(b);
        case BUILDING_SMALL_OBELISK:
        case BUILDING_LARGE_OBELISK:
            b->data.obelisk.completion_shown = 0;
            return 1;
        case BUILDING_SPHINX:
            city_player_current()->sphinx_completed = 0;
            return 1;
        case BUILDING_SUN_TEMPLE:
            b->data.monument.completion_shown = 0;
            return SUN_TEMPLE_PARTS;
        case BUILDING_MAUSOLEUM:
            b->data.mausoleum.completion_shown = 0;
            return MAUSOLEUM_PARTS;
        case BUILDING_ALEXANDRIA_LIBRARY:
            b->data.monument.completion_shown = 0;
            return ALEXANDRIA_LIBRARY_PARTS;
        case BUILDING_CAESAREUM:
            b->data.monument.completion_shown = 0;
            return CAESAREUM_PARTS;
        case BUILDING_LIGHTHOUSE:
            b->data.monument.completion_shown = 0;
            return 1;
        case BUILDING_SMALL_ROYAL_TOMB:
            b->data.royal_tomb.completion_shown = 0;
            return SMALL_ROYAL_TOMB_PARTS;
        case BUILDING_ABU_SIMBEL:
            b->data.abu_simbel.completion_shown = 0;
            return ABU_SIMBEL_PARTS;
        case BUILDING_MEDIUM_ROYAL_TOMB:
            b->data.royal_tomb.completion_shown = 0;
            return MEDIUM_ROYAL_TOMB_PARTS;
        case BUILDING_LARGE_ROYAL_TOMB:
            b->data.royal_tomb.completion_shown = 0;
            return LARGE_ROYAL_TOMB_PARTS;
        case BUILDING_GRAND_ROYAL_TOMB:
            b->data.royal_tomb.completion_shown = 0;
            return GRAND_ROYAL_TOMB_PARTS;
        default:
            return 1;
    }
}

void game_file_upgrade_monument_fields(void)
{
    for (int32_t id = 1; id < MAX_BUILDINGS; id++) {
        building *b = building_get(id);
        if (b->state == BUILDING_STATE_IN_USE) {
            id += clear_monument_completion_shown(b) - 1;
        }
    }
}

void game_file_upgrade_clear_formation_flags(void)
{
    for (int32_t id = 1; id < MAX_FORMATIONS; id++) {
        formation_get(id)->ticks_without_target = 0;
    }
}

void game_file_upgrade_building_storage_flags(void)
{
    for (int32_t id = 1; id < MAX_BUILDINGS; id++) {
        building *b = building_get(id);
        uint8_t old_flag = b->storage_id;
        switch (b->type) {
            case BUILDING_BAZAAR:
            case BUILDING_GRANARY:
            case BUILDING_STORAGE_YARD:
            case BUILDING_BAZAAR_UP:
            case BUILDING_GRANARY_UP:
            case BUILDING_STORAGE_YARD_UP:
                b->queue_slot = 0;
                b->queue_index = 0;
                break;
            case BUILDING_SHIPWRIGHT:
                b->queue_slot = old_flag;
                b->storage_id = 0;
                b->queue_index = 0;
                break;
            case BUILDING_FERRY_LANDING:
                b->queue_index = old_flag;
                b->storage_id = 0;
                b->queue_slot = 0;
                break;
            default:
                b->storage_id = 0;
                b->queue_slot = 0;
                b->queue_index = 0;
                break;
        }
    }
}

void game_file_reset_player_fields(void)
{
    city_player *player = city_player_current();
    memset(player->food_known, 0, sizeof(player->food_known));
    memset(player->resource_known, 0, sizeof(player->resource_known));
}

void game_file_upgrade_clear_building_field(void)
{
    for (int32_t id = 0; id < MAX_BUILDINGS; id++) {
        building_get(id)->data.house.zoo = 0;
    }
}

void game_file_upgrade_clear_player_fields(void)
{
    for (int32_t p = 0; p < CITY_PLAYERS; p++) {
        city_player *player = city_player_get(p);
        player->zoo_shows = 0;
        player->zoo_no_shows_weighted = 0;
    }
}

static int16_t renumbered_resource(int16_t resource)
{
    return (resource >= RESOURCE_RENUMBERED_FROM_BEFORE_SCENARIO_EVENTS) ? (int16_t) (resource + RESOURCE_RENUMBER_SHIFT_BEFORE_SCENARIO_EVENTS) : resource;
}

static void move_up_over_unused(int16_t *first, int16_t *second, int16_t *third)
{
    if (*first < 0) {
        if (*second >= 0) {
            *first = *second;
            *second = -1;
        } else if (*third >= 0) {
            *first = *third;
            *third = -1;
        }
    }
    if ((*second < 0) && (*third >= 0)) {
        *second = *third;
        *third = -1;
    }
}

void game_file_upgrade_scenario_events(void)
{
    int32_t count = (int16_t) scenario_events.count;
    for (int32_t i = 0; i < count; i++) {
        scenario_event *e = &scenario_events.events[i];
        if (!scenario_event_type_uses_resource(e)) {
            continue;
        }
        scenario_value *item = &e->item;
        item->value = renumbered_resource(item->value);
        item->fixed = renumbered_resource(item->fixed);
        item->min = renumbered_resource(item->min);
        item->max = renumbered_resource(item->max);
        move_up_over_unused(&item->fixed, &item->min, &item->max);
    }
    for (int32_t resource = RESOURCE_RENUMBERED_FROM_BEFORE_SCENARIO_EVENTS; resource < RESOURCE_MAX; resource++) {
        for (int32_t field = 0; field < TRADE_PRICE_FIELDS; field++) {
            trade_prices[(resource * TRADE_PRICE_FIELDS) + field] = trade_default_prices[(resource * TRADE_PRICE_FIELDS) + field];
        }
    }
}

static int32_t is_statue(int32_t type)
{
    return (type == BUILDING_SMALL_STATUE) || (type == BUILDING_MEDIUM_STATUE) || (type == BUILDING_LARGE_STATUE);
}

void game_file_upgrade_building_orientation(void)
{
    int32_t flag = 0;
    for (int32_t id = 1; id < MAX_BUILDINGS; id++) {
        building *b = building_get(id);
        if ((b->state != BUILDING_STATE_IN_USE) || !is_statue(b->type)) {
            continue;
        }
        int32_t value = b->data.oriented.orientation;
        if (value & STATUE_FLAG_BEFORE_BUILDING_ORIENTATION) {
            flag = 1;
            value -= STATUE_FLAG_BEFORE_BUILDING_ORIENTATION;
        }
        value++;
        if (value == STATUE_ORIENTATIONS) {
            value = 0;
        }
        b->data.oriented.orientation = (uint8_t) ((flag * STATUE_FLAG) + value);
    }
}

static int32_t *name_counter(int32_t group)
{
    int32_t slot = (group >= FIGURE_NAME_GROUP_ENEMY_EXTRA_FIRST) ? (FIGURE_NAME_GROUP_ENEMY_EXTRA_LAST - group) : (FIGURE_NAME_GROUP_COUNTER_BASE - group);
    return &figure_name_counter_table.by_slot[slot];
}

static void set_name_group(figure *f, int32_t group, int32_t count)
{
    f->name_group = group;
    int32_t *counter = name_counter(group);
    if (*counter >= count) {
        *counter = 0;
    }
}

static void set_priest_name_group(figure *f)
{
    uint32_t god = (uint32_t) building_temple_god(f->building_id);
    if (god <= GOD_BAST) {
        set_name_group(f, PRIEST_NAME_GROUP_FIRST + (int32_t) god, PRIEST_NAME_COUNT);
    }
}

static void set_celebrant_name_group(figure *f)
{
    int32_t role = building_festival_celebrant_role(building_get(f->building_id)->type);
    f->resource_id = (uint8_t) role;
    if (role == FIGURE_PRIEST) {
        set_priest_name_group(f);
        return;
    }
    int32_t count;
    int32_t group = building_festival_celebrant_name_group(role, &count);
    set_name_group(f, group, count);
}

static void set_enemy_name_group(figure *f)
{
    uint32_t enemy = formation_get(f->formation_id)->enemy_type;
    if (enemy < ENEMY_EGYPTIAN) {
        set_name_group(f, enemy_name_groups[enemy].group, enemy_name_groups[enemy].count);
    } else {
        f->name_group = -1;
    }
}

static const figure_name_group *type_name_group(int32_t type)
{
    if (type == FIGURE_TOMB_ROBBER) {
        return &figure_name_groups[FIGURE_CRIMINAL];
    }
    return (type < FIGURE_TYPE_COUNT) ? &figure_name_groups[type] : 0;
}

static void set_type_name_group(figure *f)
{
    const figure_name_group *group = type_name_group(f->type);
    if (group && (group->group > 0)) {
        set_name_group(f, group->group, group->count);
    } else {
        f->name_group = -2;
        f->name_id = -1;
    }
}

void game_file_assign_figure_name_groups(void)
{
    for (int32_t id = 1; id < MAX_FIGURES; id++) {
        figure *f = figure_get(id);
        switch (f->type) {
            case FIGURE_PRIEST:
                set_priest_name_group(f);
                break;
            case FIGURE_FESTIVAL_PRIEST:
                set_celebrant_name_group(f);
                break;
            case FIGURE_ENEMY_ARCHER:
            case FIGURE_ENEMY_INFANTRY:
            case FIGURE_ENEMY_CHARIOT:
                set_enemy_name_group(f);
                break;
            default:
                set_type_name_group(f);
                break;
        }
    }
}

char game_file_build_empire_route_path(empire_route *r)
{
    empire_route_prepare_path(r);
    int32_t length = 0;
    for (int32_t i = 1; i < r->num_points; i++) {
        if (length > r->path_length) {
            return 0;
        }
        length += draw_route_segment(r, i, length);
    }
    r->path_length = length;
    empire_route_find_objects(r);
    return 1;
}

void game_file_transfer_records(char load)
{
    for (int32_t i = 0; i < DOCK_QUEUE_RECORDS; i++) {
        dyn_queue *queue = &dock_queue_get(i)->queue;
        if (load) {
            dyn_queue_load(queue, dock_queues.records[i]);
        } else {
            dyn_queue_save_to(queue, &dock_queues.records[i]);
        }
    }
}

static void round_up_storage_amount(int16_t *amount)
{
    for (int32_t level = STORAGE_AMOUNT_STEP; level <= STORAGE_AMOUNT_MAX; level += STORAGE_AMOUNT_STEP) {
        if (*amount <= level) {
            *amount = (int16_t) level;
            return;
        }
    }
}

void game_file_update_granary_stock(void)
{
    for (int32_t id = 0; id < MAX_BUILDINGS; id++) {
        building *b = building_get(id);
        if ((b->state != BUILDING_STATE_IN_USE) || (b->type != BUILDING_GRANARY)) {
            continue;
        }
        int16_t *stored = b->data.granary.resource_stored;
        int32_t sum = 0;
        for (int32_t r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
            sum += stored[r];
        }
        int32_t unused = GRANARY_CAPACITY - sum;
        stored[RESOURCE_NONE] = (int16_t) ((unused < 0) ? 0 : unused);
        building_storage *storage = storage_get(b->storage_id);
        for (int32_t r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
            round_up_storage_amount(&storage->accept_amount[r]);
            round_up_storage_amount(&storage->get_amount[r]);
        }
    }
}

void game_file_update_festival_square(void)
{
    city_player *player = city_player_current();
    for (int32_t id = 0; id < MAX_BUILDINGS; id++) {
        const building *b = building_get(id);
        if ((b->state == BUILDING_STATE_IN_USE) && (b->type == BUILDING_FESTIVAL_SQUARE)) {
            player->festival_square_x = b->x;
            player->festival_square_y = b->y;
            return;
        }
    }
    player->festival_square_x = -1;
    player->festival_square_y = -1;
}

void game_file_convert_scenario_events(void *element, int32_t has_data)
{
    static scenario_events_t events;
    scenario_events_construct(&events);
    if (has_data) {
        const uint8_t *old = element;
        memcpy(&events, old, EVENTS_BEFORE_EVENT_RELAYOUT_RECORDS_END);
        for (int32_t i = 0; i < SCENARIO_MAX_EVENTS; i++) {
            scenario_event *e = &events.events[i];
            memcpy(e, old + EVENTS_BEFORE_EVENT_RELAYOUT_FIRST + (i * EVENT_BEFORE_EVENT_RELAYOUT_SIZE), EVENT_BEFORE_EVENT_RELAYOUT_SIZE);
            e->parent_amount = 0;
            e->parent_item = 0;
        }
        memcpy(&events.editor_selected_id, old + EVENTS_BEFORE_EVENT_RELAYOUT_RECORDS_END, EVENTS_BEFORE_EVENT_RELAYOUT_EDITOR_SIZE);
        events.earthquake_active = old[EVENTS_BEFORE_EVENT_RELAYOUT_EARTHQUAKE_ACTIVE];
        events.reserved_1 = old[EVENTS_BEFORE_EVENT_RELAYOUT_EARTHQUAKE_ACTIVE + 1];
        memcpy(element, &events, SCENARIO_EVENTS_SAVED_BYTES);
    } else {
        memset(element, 0, SCENARIO_EVENTS_SAVED_BYTES);
    }
}

void game_file_convert_message(void *element, int32_t has_data)
{
    city_message *m = element;
    if (!has_data) {
        memset(m, 0, sizeof(city_message));
        return;
    }
    city_message_before_message_relayout old;
    memcpy(&old, m, sizeof(old));
    city_message upgraded;
    memset(&upgraded, 0, sizeof(upgraded));
    upgraded.param1 = old.param1;
    upgraded.param2 = old.param2;
    upgraded.year = old.year;
    upgraded.type = old.type;
    upgraded.sequence = old.sequence;
    upgraded.is_read = old.is_read;
    upgraded.month = old.month;
    memcpy(upgraded.phrases, old.phrases, sizeof(old.phrases));
    *m = upgraded;
}
