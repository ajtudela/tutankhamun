// The gods' curses that destroy or take away: every building of one industry type collapses, the fullest storage yard
// collapses or loses loads, the most experienced legion's fort collapses and its soldiers die, and up to twenty of the
// highest houses collapse.
//
// Two curses reach further than they look:
// - Seth's curse takes its soldiers from the formation whose index is the fort's building id, not from the fort's
//   own formation, as the original does. A building id of MAX_FORMATIONS or more reads past the formations, through
//   the memory that follows them in the original (formation_memory_past_records: the four figure counters, then the
//   bytes of city_figures). The ids found there die: a figure id past the records sets the byte of the route paths
//   where that figure's state would be, and a write past the route paths is dropped;
// - a storage yard part id of 0 or less is followed as it is for the rest of the eight steps.

#include "building/building_data.h"
#include "building/building_figure_helpers.h"
#include "building/building_image_group.h"
#include "building/building_maintenance.h"
#include "building/building_type.h"
#include "city/city_data.h"
#include "city/city_internal.h"
#include "city/gods.h"
#include "city/resource_type.h"
#include "core/crt_compat.h"
#include "figure/figure.h"
#include "figure/formation.h"
#include "graphics/image_collection.h"
#include "map/routing_terrain.h"
#include "sound/sound_play.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

static void collapse_building(int32_t id)
{
    building_destroy_by_fire(id, 0);
    building_destroy_linked_parts(id, 1);
    sound_effect_play(SOUND_EFFECT_BUILDING_DESTROYED);
    const building *b = building_get(id);
    int32_t size = b->size;
    map_routing_update_land_citizen_rect(b->x - size, b->y - size, b->x + size, b->y + size, 1);
    size = b->size;
    map_routing_update_land_noncitizen_rect(b->x - size, b->y - size, b->x + size, b->y + size);
}

int32_t city_gods_ptah_major_curse(void)
{
    int32_t types[PTAH_CURSE_INDUSTRIES] = { BUILDING_GOLD_MINE, BUILDING_GEMSTONE_MINE, BUILDING_CLAY_PIT, BUILDING_SHIPWRIGHT, BUILDING_WEAVER,
        BUILDING_JEWELER };
    static const int32_t industries[PTAH_CURSE_INDUSTRIES] = { RESOURCE_GOLD, RESOURCE_GEMS, RESOURCE_CLAY, PTAH_CURSE_NO_INDUSTRY, RESOURCE_LINEN,
        RESOURCE_LUXURY_GOODS };
    int32_t count = 0;
    for (int32_t i = 0; i < PTAH_CURSE_INDUSTRIES; i++) {
        if (building_count_total[types[i]] > 0) {
            count++;
        } else if (industries[i] > 0 && building_count_industry_active[industries[i]] > 0) {
            count++;
        } else {
            types[i] = 0;
        }
    }
    if (count == 0) {
        return 0;
    }
    int32_t chosen = crt_rand() % count;
    int32_t seen = 0;
    int32_t type = 0;
    int32_t i;
    for (i = 0; i < PTAH_CURSE_INDUSTRIES; i++) {
        if (types[i] != 0) {
            if (seen == chosen) {
                break;
            }
            seen++;
        }
    }
    if (i == PTAH_CURSE_INDUSTRIES) {
        return 0;
    }
    type = types[i];
    if (type == -1) {
        return 0;
    }
    for (int32_t id = 1; id < MAX_BUILDINGS; id++) {
        const building *b = building_get(id);
        if (b->state == BUILDING_STATE_IN_USE && b->type == type) {
            building_destroy_by_fire(id, 0);
            building_destroy_linked_parts(id, 1);
        }
    }
    city_message_posted_once = 0;
    sound_effect_play(SOUND_EFFECT_BUILDING_DESTROYED);
    map_routing_update_land_citizen();
    map_routing_update_land_noncitizen();
    return 1;
}

void city_gods_ptah_minor_curse(int32_t destroy)
{
    int32_t best = 0;
    int32_t most = 0;
    for (int32_t id = 1; id < MAX_BUILDINGS; id++) {
        const building *b = building_get(id);
        if (b->state != BUILDING_STATE_IN_USE || (b->type != BUILDING_STORAGE_YARD && b->type != BUILDING_STORAGE_YARD_UP)) {
            continue;
        }
        int32_t total = 0;
        for (int32_t r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
            total += building_storageyard_get_amount(id, r);
        }
        if (most <= total) {
            most = total;
            best = id;
        }
    }
    if (best == 0) {
        return;
    }
    if (destroy == 1) {
        city_message_posted_once = 0;
        collapse_building(best);
        return;
    }
    int32_t type = building_get(best)->type;
    if (type == BUILDING_STORAGE_YARD || type == BUILDING_STORAGE_YARD_UP) {
        building_storageyard_remove_resource_curse(best, PTAH_CURSE_LOADS);
    }
}

void building_storageyard_remove_resource_curse(int32_t building_id, int32_t amount)
{
    int32_t type = building_get(building_id)->type;
    if (type != BUILDING_STORAGE_YARD && type != BUILDING_STORAGE_YARD_UP) {
        return;
    }
    city_player *player = city_player_current();
    int32_t id = building_id;
    for (int32_t i = 0; i < STORAGE_YARD_ROOMS; i++) {
        if (amount <= 0) {
            return;
        }
        id = building_get(id)->next_part_building_id;
        if (id <= 0) {
            continue;
        }
        building *room = building_get(id);
        int16_t loads = room->loads_stored;
        if (loads <= 0) {
            continue;
        }
        int32_t resource = room->subtype;
        if (amount < loads) {
            room->loads_stored = (int16_t) (loads - amount);
            player->storage_space[resource] += amount;
            player->storage_stored[resource] -= amount;
            amount = 0;
        } else {
            amount -= loads;
            player->storage_space[resource] += loads;
            player->storage_stored[resource] -= loads;
            room->loads_stored = 0;
            room->subtype = 0;
        }
        int32_t grid_offset = room->grid_offset;
        int32_t image;
        if (room->loads_stored > 0) {
            image = (resource < RESOURCE_OIL) ? image_group(IMAGE_GROUP_STORAGE_YARD_GOODS) + ((resource - RESOURCE_MIN) * STORAGE_ROOM_IMAGES_PER_RESOURCE)
                                              : image_group(IMAGE_GROUP_STORAGE_YARD_GOODS_LATE) + ((resource - RESOURCE_OIL) * STORAGE_ROOM_IMAGES_PER_RESOURCE);
            image += storage_image_level(room->loads_stored);
        } else {
            image = image_group(IMAGE_GROUP_STORAGE_YARD_SPACE);
        }
        map_grids.images[grid_offset] = image;
    }
}

static int32_t seth_curse_soldier_id(int32_t formation_index, int32_t slot)
{
    if (formation_index < MAX_FORMATIONS) {
        return formation_get(formation_index)->figures[slot];
    }
    const uint8_t *past = formation_memory_past_records(((formation_index - MAX_FORMATIONS) * (int32_t) sizeof(formation)) +
        (int32_t) offsetof(formation, figures) + (slot * (int32_t) sizeof(int16_t)));
    if (!past) {
        return 0;
    }
    int16_t id;
    memcpy(&id, past, sizeof(id));
    return id;
}

static void seth_curse_kill(int32_t figure_id)
{
    if (figure_id < MAX_FIGURES) {
        figure_get(figure_id)->state = FIGURE_STATE_DEAD;
        return;
    }
    int32_t path_byte = ((figure_id - MAX_FIGURES) * (int32_t) sizeof(figure)) + (int32_t) offsetof(figure, state);
    if (path_byte < (int32_t) sizeof(city_figures.route_paths)) {
        city_figures.route_paths[path_byte] = FIGURE_STATE_DEAD;
    }
}

int32_t city_gods_seth_minor_curse(void)
{
    int32_t most = 0;
    int32_t fort = 0;
    for (int32_t id = 1; id < MAX_FORMATIONS; id++) {
        formation *f = formation_get(id);
        if (f->in_use == 1 && f->is_legion != 0) {
            int32_t experience = f->experience;
            if (experience >= most) {
                fort = f->building_id;
                most = experience;
            }
        }
    }
    if (fort == 0) {
        return 0;
    }
    city_message_posted_once = 0;
    for (int32_t i = 0; i < FORMATION_MAX_FIGURES; i++) {
        int32_t figure_id = seth_curse_soldier_id(fort, i);
        if (figure_id == 0) {
            break;
        }
        if (figure_id > 0) {
            seth_curse_kill(figure_id);
        }
    }
    collapse_building(fort);
    return 1;
}

int city_gods_house_level_compare(const void *a, const void *b)
{
    int32_t level_a = building_get(*(const int32_t *) a)->subtype;
    int32_t level_b = building_get(*(const int32_t *) b)->subtype;
    if (level_a < level_b) {
        return -1;
    }
    return level_a > level_b;
}

void city_gods_bast_major_curse(void)
{
    int32_t houses[BAST_CURSE_HOUSES];
    int32_t count = 0;
    int32_t insert_at = BAST_CURSE_HOUSES - 1;
    for (int32_t id = 1; id < MAX_BUILDINGS; id++) {
        const building *b = building_get(id);
        if (b->type >= BUILDING_HOUSE_CRUDE_HUT && b->type <= BUILDING_HOUSE_PALATIAL_ESTATE && b->state == BUILDING_STATE_IN_USE) {
            houses[count++] = id;
            if (count >= BAST_CURSE_HOUSES) {
                break;
            }
        }
    }
    if (count >= BAST_CURSE_HOUSES) {
        crt_qsort(houses, BAST_CURSE_HOUSES, sizeof(houses[0]), city_gods_house_level_compare);
        int32_t last = houses[BAST_CURSE_HOUSES - 1];
        for (int32_t id = BAST_CURSE_HOUSES + 1; id < MAX_BUILDINGS; id++) {
            const building *b = building_get(id);
            if (b->type < BUILDING_HOUSE_CRUDE_HUT || b->type > BUILDING_HOUSE_PALATIAL_ESTATE || b->state != BUILDING_STATE_IN_USE) {
                continue;
            }
            int16_t level = b->subtype;
            if (level <= building_get(last)->subtype) {
                continue;
            }
            for (int32_t i = 0; i < BAST_CURSE_HOUSES; i++) {
                if (building_get(houses[i])->subtype < level) {
                    insert_at = i;
                    break;
                }
            }
            for (int32_t i = BAST_CURSE_HOUSES - 1; i > insert_at; i--) {
                houses[i] = houses[i - 1];
            }
            houses[insert_at] = id;
            last = houses[BAST_CURSE_HOUSES - 1];
        }
    }
    for (int32_t i = 0; i < count; i++) {
        city_message_posted_once = 0;
        collapse_building(houses[i]);
    }
}
