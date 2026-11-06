// Random city sounds: from time to time a sample of what lies under a random point of the city view (a building's
// channel, or the terrain's: trees, rock, water, meadow, floodplain), panned by its x in the view. Also the sound
// category of a building type, used for the footprints of the city view.
//
// BUILDING_EDITOR_CANAL's value is listed among the housing sound categories.

#include "sound/city_play.h"

#include "building/building.h"
#include "building/building_data.h"
#include "building/building_type.h"
#include "core/crt_compat.h"
#include "figure/figure_type.h"
#include "graphics/graphics_data.h"
#include "map/terrain.h"
#include "sound/ambient.h"
#include "sound/city_channel_types.h"
#include "sound/sound_data.h"
#include "sound/sound_lib.h"
#include "widget/city_view.h"
#include "widget/widget_data.h"
#include "window/window_data.h"

static int32_t city_channel_random(int32_t type)
{
    const city_channel *channel = &city_channel_base.types[type];
    return channel->variants[(crt_rand() * channel->variant_count) / CRT_RAND_RANGE];
}

static int32_t fort_channel(int32_t legion)
{
    if (legion == FIGURE_CHARIOTEER) {
        return city_channel_random(BUILDING_FORT_CHARIOTEERS);
    }
    if (legion == FIGURE_ARCHER) {
        return city_channel_random(BUILDING_FORT_ARCHERS);
    }
    return city_channel_random(BUILDING_FORT_INFANTRY);
}

static int32_t terrain_slot(uint32_t terrain)
{
    if (terrain & TERRAIN_TREE) {
        return AMBIENT_TERRAIN_FOREST;
    }
    if (terrain & TERRAIN_ROCK) {
        return AMBIENT_TERRAIN_ROCK;
    }
    if ((terrain & TERRAIN_WATER) && !(terrain & TERRAIN_DIKE)) {
        return AMBIENT_TERRAIN_WATER;
    }
    if (terrain & TERRAIN_MEADOW) {
        return AMBIENT_TERRAIN_FARM;
    }
    if (terrain & TERRAIN_FLOODPLAIN) {
        return AMBIENT_TERRAIN_FLOOD;
    }
    return -1;
}

void sound_city_play_random(void)
{
    if (!sound_digital_ok || !settings_data.sound_city_enabled) {
        return;
    }
    int32_t x;
    if (g_sidebar_collapsed) {
        x = (crt_rand() * city_view_width_collapsed) / CRT_RAND_RANGE;
    } else {
        x = (crt_rand() * city_view_width_expanded) / CRT_RAND_RANGE;
    }
    int32_t y = (crt_rand() * screen_height) / CRT_RAND_RANGE;
    int32_t grid_offset = city_view_pixels_to_grid_offset(x, y);
    if (grid_offset >= VIEW_LOOKUP_EMPTY) {
        return;
    }
    int32_t building_id = map_grids.building_ids[grid_offset];
    uint32_t terrain = map_grids.terrain[grid_offset];
    int32_t channel;
    if (building_id) {
        const building *b = building_get(building_id);
        channel = (b->type == BUILDING_FORT) ? fort_channel(b->subtype) : city_channel_random(b->type);
    } else {
        if (!terrain) {
            return;
        }
        int32_t kind = terrain_slot(terrain);
        if (kind < 0) {
            return;
        }
        const ambient_channel *terrain_channel = &ambient_channel_base.slots[kind];
        channel = terrain_channel->variants[(crt_rand() * terrain_channel->variant_count) / CRT_RAND_RANGE];
    }
    if (channel <= -1) {
        return;
    }
    int32_t width = g_sidebar_collapsed ? city_view_width_collapsed : city_view_width_expanded;
    sound_manager.request.pan = (int32_t) ((double) x * ((double) SOUND_PAN_RANGE / (double) width));
    sound_manager.request.volume = settings_data.sound_city_volume;
    sound_manager.request.name_index = channel;
    sound_manager.request.id = -1;
    sound_manager.request.loop_count = 1;
    sound_lib_play_sample(&sound_manager, &sound_manager.request);
}

int32_t building_type_sound_category(int32_t type)
{
    switch (type) {
        case BUILDING_MUD_WALL:
        case BUILDING_HOUSE_CRUDE_HUT:
        case BUILDING_HOUSE_STURDY_HUT:
        case BUILDING_HOUSE_MEAGER_SHANTY:
        case BUILDING_HOUSE_COMMON_SHANTY:
        case BUILDING_HOUSE_ROUGH_COTTAGE:
        case BUILDING_HOUSE_ORDINARY_COTTAGE:
        case BUILDING_HOUSE_MODEST_HOMESTEAD:
        case BUILDING_HOUSE_SPACIOUS_HOMESTEAD:
        case BUILDING_HOUSE_MODEST_APARTMENT:
        case BUILDING_HOUSE_SPACIOUS_APARTMENT:
        case BUILDING_HOUSE_COMMON_RESIDENCE:
        case BUILDING_HOUSE_SPACIOUS_RESIDENCE:
        case BUILDING_HOUSE_ELEGANT_RESIDENCE:
        case BUILDING_HOUSE_FANCY_RESIDENCE:
        case BUILDING_HOUSE_COMMON_MANOR:
        case BUILDING_HOUSE_SPACIOUS_MANOR:
        case BUILDING_HOUSE_ELEGANT_MANOR:
        case BUILDING_HOUSE_STATELY_MANOR:
        case BUILDING_HOUSE_MODEST_ESTATE:
        case BUILDING_HOUSE_PALATIAL_ESTATE:
        case BUILDING_BANDSTAND:
        case BUILDING_BOOTH:
        case BUILDING_SENET_HOUSE:
        case BUILDING_PAVILION:
        case BUILDING_CONSERVATORY:
        case BUILDING_DANCE_SCHOOL:
        case BUILDING_JUGGLER_SCHOOL:
        case BUILDING_BULLFIGHT_SCHOOL:
        case BUILDING_PLAZA:
        case BUILDING_GARDENS:
        case BUILDING_FORT_CHARIOTEERS:
        case BUILDING_SMALL_STATUE:
        case BUILDING_MEDIUM_STATUE:
        case BUILDING_LARGE_STATUE:
        case BUILDING_FORT_ARCHERS:
        case BUILDING_FORT_INFANTRY:
        case BUILDING_MORTUARY:
        case BUILDING_DENTIST:
        case BUILDING_SCRIBAL_SCHOOL:
        case BUILDING_LIBRARY:
        case BUILDING_POLICE_STATION:
        case BUILDING_MUD_GATEHOUSE:
        case BUILDING_MUD_TOWER:
        case BUILDING_BAZAAR:
        case BUILDING_PERSONAL_MANSION:
        case BUILDING_FAMILY_MANSION:
        case BUILDING_DYNASTY_MANSION:
        case BUILDING_ARCHITECT_POST:
        case BUILDING_WATER_LIFT:
        case BUILDING_WELL:
        case BUILDING_MILITARY_ACADEMY:
        case BUILDING_RECRUITER:
        case BUILDING_EDITOR_CANAL:
        case BUILDING_SHRINE_OSIRIS:
        case BUILDING_SHRINE_RA:
        case BUILDING_SHRINE_PTAH:
        case BUILDING_SHRINE_SETH:
        case BUILDING_SHRINE_BAST:
        case BUILDING_SHRINE_GOD_5:
        case BUILDING_TEMPLE_GOD_5:
        case BUILDING_TEMPLE_GOD_6:
        case BUILDING_TEMPLE_GOD_7:
        case BUILDING_TEMPLE_GOD_8:
        case BUILDING_TEMPLE_GOD_9:
        case BUILDING_TEMPLE_COMPLEX_GOD_5:
        case BUILDING_TEMPLE_COMPLEX_GOD_6:
        case BUILDING_TEMPLE_COMPLEX_GOD_7:
        case BUILDING_TEMPLE_COMPLEX_GOD_8:
        case BUILDING_TEMPLE_COMPLEX_GOD_9:
        case BUILDING_FIREHOUSE:
        case BUILDING_BRICK_WALL:
        case BUILDING_WALL:
        case BUILDING_BRICK_GATEHOUSE:
        case BUILDING_GATEHOUSE:
        case BUILDING_BRICK_TOWER:
        case BUILDING_TOWER:
        case BUILDING_COURTHOUSE:
        case BUILDING_PHYSICIAN:
        case BUILDING_ZOO:
            return AMBIENT_HOUSING;
        case BUILDING_GRANARY:
        case BUILDING_STORAGE_YARD:
        case BUILDING_STORAGE_ROOM:
        case BUILDING_SHIPWRIGHT:
        case BUILDING_DOCK:
        case BUILDING_FISHING_WHARF:
        case BUILDING_STONE_QUARRY:
        case BUILDING_LIMESTONE_QUARRY:
        case BUILDING_WOOD_CUTTER:
        case BUILDING_CLAY_PIT:
        case BUILDING_BREWERY:
        case BUILDING_WEAVER:
        case BUILDING_WEAPONSMITH:
        case BUILDING_JEWELER:
        case BUILDING_POTTER:
        case BUILDING_HUNTING_LODGE:
        case BUILDING_FERRY_LANDING:
        case BUILDING_GOLD_MINE:
        case BUILDING_GEMSTONE_MINE:
        case BUILDING_CARPENTERS_GUILD:
        case BUILDING_BRICKLAYERS_GUILD:
        case BUILDING_STONEMASONS_GUILD:
        case BUILDING_TRANSPORT_WHARF:
        case BUILDING_WARSHIP_WHARF:
        case BUILDING_BRICKWORKS:
        case BUILDING_CHARIOT_MAKER:
        case BUILDING_COPPER_MINE:
        case BUILDING_SANDSTONE_QUARRY:
        case BUILDING_ARTISANS_GUILD:
            return AMBIENT_INDUSTRIAL;
        case BUILDING_TAX_COLLECTOR:
        case BUILDING_TAX_COLLECTOR_UP:
        case BUILDING_FLAX_FARM:
        case BUILDING_GRAIN_FARM:
        case BUILDING_LETTUCE_FARM:
        case BUILDING_POMEGRANATES_FARM:
        case BUILDING_HENNA_FARM:
            return AMBIENT_MEADOW;
        case BUILDING_PYRAMID:
        case BUILDING_SPHINX:
            return AMBIENT_MONUMENT;
        default:
            return 0;
    }
}

