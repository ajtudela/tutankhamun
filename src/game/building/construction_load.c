// Adding buildings to the map, when a building is placed and while a save loads: the tiles and images of every
// building type with the extra parts a new building creates (storage yard rooms, fort standard, sphinx parts,
// monuments through their objects) and the city records it marks; with the venue plaza tiles, the empty-land
// redraw around a building and the migration warning.
//
// Three details differ from what the code seems to intend:
// - the call that would add a farm image to a storage room is not made;
// - every palace sets palace_placed, but only the first one is remembered;
// - the ruin of a small house is a single image.

#include "building/construction_load.h"
#include "building/abu_simbel.h"
#include "building/building.h"
#include "building/building_create.h"
#include "building/building_figure_helpers.h"
#include "building/building_image_group.h"
#include "building/building_maintenance.h"
#include "building/building_storage.h"
#include "building/building_type.h"
#include "building/caesareum.h"
#include "building/construction_place.h"
#include "building/ferry_landing.h"
#include "building/fort.h"
#include "building/house_evolve.h"
#include "building/house_size.h"
#include "building/library.h"
#include "building/lighthouse.h"
#include "building/mausoleum.h"
#include "building/monument.h"
#include "building/monument_city.h"
#include "building/monument_object.h"
#include "building/obelisk.h"
#include "building/pyramid_parts.h"
#include "building/royal_tomb.h"
#include "building/sphinx.h"
#include "building/sun_temple.h"
#include "building/temple_complex.h"
#include "city/finance_estimates.h"
#include "city/message.h"
#include "city/resource_type.h"
#include "city/resources.h"
#include "core/crt_compat.h"
#include "core/queue.h"
#include "figure/figure_type.h"
#include "figure/formation.h"
#include "game/game_internal.h"
#include "game/undo_build.h"
#include "graphics/image_collection.h"
#include "map/building_tiles.h"
#include "map/farm.h"
#include "map/grid.h"
#include "map/map_data.h"
#include "map/orientation.h"
#include "map/terrain.h"
#include "map/tiles.h"
#include "map/tiles_internal.h"
#include "map/tiles_road.h"
#include "map/water.h"
#include "window/advisor_comments.h"
#include "window/window_input.h"
#include <stdbool.h>
#include <stdint.h>

int32_t migration_warning_rules[MIGRATION_WARNING_RULE_COUNT] = {
    [MIGRATION_WARNING_RULE_MIN_POPULATION] = 20,
    [MIGRATION_WARNING_RULE_QUIET_MISSIONS] = 2,
    [MIGRATION_WARNING_RULE_MAX_ARMED_FIGURES] = 3,
    [MIGRATION_WARNING_RULE_FEW_NEWCOMERS] = 5,
    [MIGRATION_WARNING_RULE_LOW_PERCENTAGE] = 80,
};

void venue_orientation_images(int32_t which, int32_t *main_image, int32_t *next_image)
{
    int32_t base = image_group(IMAGE_GROUP_BANDSTAND_GATE);
    switch (map_info.orientation) {
        case 0:
            if (which == 0) {
                *next_image = base;
                *main_image = base + 1;
                return;
            }
            if (which == 1) {
                *next_image = base + 3;
                *main_image = base + 2;
                return;
            }
            break;
        case 2:
            if (which == 0) {
                *next_image = base + 3;
                *main_image = base + 2;
                return;
            }
            if (which == 1) {
                *next_image = base + 1;
                *main_image = base;
                return;
            }
            break;
        case 4:
            if (which == 0) {
                *next_image = base + 1;
                *main_image = base;
                return;
            }
            if (which == 1) {
                *next_image = base + 2;
                *main_image = base + 3;
                return;
            }
            break;
        case 6:
            if (which == 0) {
                *next_image = base + 2;
                *main_image = base + 3;
                return;
            }
            if (which == 1) {
                *next_image = base;
                *main_image = base + 1;
            }
            break;
    }
}

void venue_place_adjacent_tiles(int32_t building_id, int32_t x, int32_t y)
{
    building *b = building_get(building_id);
    int32_t which = b->venue_orientation;
    int32_t next_id = b->next_part_building_id;
    int32_t main_image = 0;
    int32_t next_image = 0;
    venue_orientation_images(which, &main_image, &next_image);
    int32_t x2 = x;
    int32_t y2 = y + 1;
    if (which != 0) {
        x2 = x + 1;
        y2 = y;
    }
    map_building_tiles_add(building_id, x, y, 1, 1, main_image, TERRAIN_BUILDING);
    map_building_tiles_add(next_id, x2, y2, 1, 1, next_image, TERRAIN_BUILDING);
}

static int32_t cause_quiet_in_first_missions(int32_t cause)
{
    return cause == NO_IMMIGRATION_LOW_WAGES || cause == NO_IMMIGRATION_NO_JOBS || cause == NO_IMMIGRATION_HIGH_TAXES || cause == NO_IMMIGRATION_LOW_MOOD;
}

static void show_cause_warning(int32_t warning)
{
    if (warning != 0) {
        city_warning_show(warning);
    }
}

void city_migration_post_message(int32_t on_change)
{
    const city_player *player = city_player_current();
    if (editor_active == 1 || player->population < migration_warning_rules[MIGRATION_WARNING_RULE_MIN_POPULATION]) {
        return;
    }
    int32_t cause = player->no_immigration_cause;
    if (settings_data.is_custom_scenario == 0 && settings_data.mission_id < migration_warning_rules[MIGRATION_WARNING_RULE_QUIET_MISSIONS] &&
        cause_quiet_in_first_missions(cause)) {
        return;
    }
    int32_t direction = player->migration_direction;
    int32_t refused = player->refused_immigrants_today;
    int32_t room = player->room_in_houses;
    if (direction == CITY_MIGRATION_NONE) {
        if ((player->kingdom_soldiers + player->enemies) > migration_warning_rules[MIGRATION_WARNING_RULE_MAX_ARMED_FIGURES]) {
            city_warning_show(WARNING_NO_IMMIGRATION_WITH_ENEMIES);
            return;
        }
        if (player->migration_newcomers < migration_warning_rules[MIGRATION_WARNING_RULE_FEW_NEWCOMERS]) {
            if (refused != 0 || room < 1) {
                if (on_change == 0) {
                    city_warning_show(WARNING_IMMIGRATION_NO_HOUSING);
                }
                return;
            }
            if (player->migration_percentage < migration_warning_rules[MIGRATION_WARNING_RULE_LOW_PERCENTAGE]) {
                show_cause_warning(WARNING_IMMIGRATION_LOW_WAGES_DETER + cause);
                return;
            }
        }
        if (on_change == 0) {
            city_warning_show(WARNING_IMMIGRATION_PEOPLE_COMING);
        }
        return;
    }
    if (direction != CITY_MIGRATION_IMMIGRATION) {
        return;
    }
    if (refused != 0 || room < 1) {
        if (on_change == 0) {
            city_warning_show(WARNING_EMIGRATION_NO_HOUSING);
        }
        return;
    }
    show_cause_warning(WARNING_EMIGRATION_LOW_WAGES + cause);
}

static int32_t plaza_sprite(int32_t x, int32_t y, int32_t size)
{
    switch (map_info.orientation) {
        case 0:
            return (y * size) + x;
        case 2:
            return ((size - 1 - x) * size) + y;
        case 4:
            return ((size - 1 - y) * size) + (size - 1 - x);
        default:
            return (x * size) + (size - 1 - y);
    }
}

static int32_t venue_plaza_size(int32_t type)
{
    switch (type) {
        case BUILDING_BOOTH:
            return 2;
        case BUILDING_BANDSTAND:
            return 3;
        case BUILDING_PAVILION:
            return 4;
        case BUILDING_FESTIVAL_SQUARE:
            return 5;
        default:
            return 1;
    }
}

static int32_t is_venue_stage_image(int32_t image)
{
    int32_t gate = image_group(IMAGE_GROUP_BANDSTAND_GATE);
    return image == image_group(IMAGE_GROUP_BOOTH) || (image >= gate && image <= (gate + 3)) || image == image_group(IMAGE_GROUP_PAVILION);
}

static int32_t plaza_image(int32_t type, int32_t x, int32_t y, int32_t size)
{
    switch (type) {
        case BUILDING_BANDSTAND:
            return image_group(IMAGE_GROUP_BANDSTAND_PLAZA) + plaza_sprite(x, y, size);
        case BUILDING_BOOTH:
            return image_group(IMAGE_GROUP_BOOTH_PLAZA) + plaza_sprite(x, y, size);
        case BUILDING_PAVILION:
            return image_group(IMAGE_GROUP_PAVILION_PLAZA) + plaza_sprite(x, y, size);
        case BUILDING_FESTIVAL_SQUARE:
            return image_group(IMAGE_GROUP_FESTIVAL_SQUARE) + plaza_sprite(x, y, size);
        default:
            return image_group(IMAGE_GROUP_PLAZA_PLAIN);
    }
}

void map_add_venue_plaza_tiles(int32_t offset, int32_t type, int32_t building_id)
{
    int32_t size = venue_plaza_size(type);
    for (int32_t y = 0; y < size; y++) {
        for (int32_t x = 0; x < size; x++) {
            int32_t tile = offset + x + (y * GRID_SIZE);
            if (type != BUILDING_FESTIVAL_SQUARE && is_venue_stage_image(map_grids.images[tile])) {
                continue;
            }
            uint32_t placed = (map_grids.terrain[tile] & (TERRAIN_KEPT_BY_BUILDING | TERRAIN_ROAD)) | TERRAIN_BUILDING;
            map_grids.building_ids[tile] = (int16_t) building_id;
            map_grids.bitfields[tile] &= (MAP_BITFIELD_ALTERNATE_TERRAIN | MAP_BITFIELD_DELETED);
            map_grids.terrain[tile] = placed;
            if ((placed & TERRAIN_ROAD) == 0 && type != BUILDING_FESTIVAL_SQUARE) {
                map_grids.terrain[tile] = placed | TERRAIN_GARDEN;
                map_grids.images[tile] = image_group(GROUP_TERRAIN_GARDEN) + (map_grids.random[tile] & (PLAZA_GARDEN_IMAGES - 1));
            } else {
                map_grids.images[tile] = plaza_image(type, x, y, size);
                routing_grids.land_citizen[tile] = 0;
            }
            map_grids.edge[tile] |= MAP_EDGE_DRAW_TILE;
        }
    }
}

static void place_square(int32_t id, int32_t x, int32_t y, int32_t size, int32_t image, int32_t terrain)
{
    map_building_tiles_add(id, x, y, size, size, image, terrain);
}

static void clear_construction_cursor(void)
{
    construction_building_type = BUILDING_NONE;
    construction_in_progress = 0;
    construction_end_x = 0;
    construction_start_x = 0;
    construction_end_y = 0;
    construction_start_y = 0;
}

static const int32_t *plain_image_group(int32_t type)
{
    switch (type) {
        case BUILDING_SENET_HOUSE:
        case BUILDING_BULLFIGHT_SCHOOL:
            return image_group_entry(IMAGE_GROUP_SENET_HOUSE);
        case BUILDING_CONSERVATORY:
            return image_group_entry(IMAGE_GROUP_CONSERVATORY);
        case BUILDING_DANCE_SCHOOL:
            return image_group_entry(IMAGE_GROUP_DANCE_SCHOOL);
        case BUILDING_JUGGLER_SCHOOL:
            return image_group_entry(IMAGE_GROUP_JUGGLER_SCHOOL);
        case BUILDING_APOTHECARY:
            return image_group_entry(IMAGE_GROUP_APOTHECARY);
        case BUILDING_MORTUARY:
            return image_group_entry(IMAGE_GROUP_MORTUARY);
        case BUILDING_DENTIST:
            return image_group_entry(IMAGE_GROUP_DENTIST);
        case BUILDING_SCRIBAL_SCHOOL:
            return image_group_entry(IMAGE_GROUP_SCRIBAL_SCHOOL);
        case BUILDING_LIBRARY:
            return image_group_entry(IMAGE_GROUP_LIBRARY);
        case BUILDING_POLICE_STATION:
            return image_group_entry(IMAGE_GROUP_POLICE_STATION);
        case BUILDING_TEMPLE_OSIRIS:
        case BUILDING_TEMPLE_GOD_5:
        case BUILDING_TEMPLE_GOD_6:
        case BUILDING_TEMPLE_GOD_7:
        case BUILDING_TEMPLE_GOD_8:
        case BUILDING_TEMPLE_GOD_9:
            return image_group_entry(IMAGE_GROUP_TEMPLE_OSIRIS);
        case BUILDING_TEMPLE_RA:
            return image_group_entry(IMAGE_GROUP_TEMPLE_RA);
        case BUILDING_TEMPLE_PTAH:
            return image_group_entry(IMAGE_GROUP_TEMPLE_PTAH);
        case BUILDING_TEMPLE_SETH:
            return image_group_entry(IMAGE_GROUP_TEMPLE_SETH);
        case BUILDING_TEMPLE_BAST:
            return image_group_entry(IMAGE_GROUP_TEMPLE_BAST);
        case BUILDING_MISSION_POST:
            return image_group_entry(IMAGE_GROUP_MISSION_POST);
        case BUILDING_ARCHITECT_POST:
            return image_group_entry(IMAGE_GROUP_ARCHITECT_POST);
        case BUILDING_TAX_COLLECTOR:
            return image_group_entry(IMAGE_GROUP_TAX_COLLECTOR);
        case BUILDING_WELL:
            return image_group_entry(IMAGE_GROUP_WELL);
        case BUILDING_STONE_QUARRY:
            return image_group_entry(IMAGE_GROUP_STONE_QUARRY);
        case BUILDING_WOOD_CUTTER:
            return image_group_entry(IMAGE_GROUP_WOOD_CUTTER);
        case BUILDING_CLAY_PIT:
            return image_group_entry(IMAGE_GROUP_CLAY_PIT);
        case BUILDING_BREWERY:
            return image_group_entry(IMAGE_GROUP_BREWERY);
        case BUILDING_WEAVER:
            return image_group_entry(IMAGE_GROUP_WEAVER);
        case BUILDING_WEAPONSMITH:
            return image_group_entry(IMAGE_GROUP_WEAPONSMITH);
        case BUILDING_JEWELER:
            return image_group_entry(IMAGE_GROUP_JEWELER);
        case BUILDING_POTTER:
            return image_group_entry(IMAGE_GROUP_POTTER);
        case BUILDING_HUNTING_LODGE:
            return image_group_entry(IMAGE_GROUP_HUNTING_LODGE);
        case BUILDING_GOLD_MINE:
            return image_group_entry(IMAGE_GROUP_GOLD_MINE);
        case BUILDING_GEMSTONE_MINE:
            return image_group_entry(IMAGE_GROUP_GEMSTONE_MINE);
        case BUILDING_FIREHOUSE:
            return image_group_entry(IMAGE_GROUP_FIREHOUSE);
        case BUILDING_CARPENTERS_GUILD:
            return image_group_entry(IMAGE_GROUP_CARPENTERS_GUILD);
        case BUILDING_BRICKLAYERS_GUILD:
            return image_group_entry(IMAGE_GROUP_BRICKLAYERS_GUILD);
        case BUILDING_STONEMASONS_GUILD:
            return image_group_entry(IMAGE_GROUP_STONEMASONS_GUILD);
        case BUILDING_WATER_SUPPLY:
            return image_group_entry(IMAGE_GROUP_WATER_SUPPLY);
        case BUILDING_COURTHOUSE:
            return image_group_entry(IMAGE_GROUP_COURTHOUSE);
        case BUILDING_WORK_CAMP:
            return image_group_entry(IMAGE_GROUP_WORK_CAMP);
        case BUILDING_PAPYRUS_MAKER:
            return image_group_entry(IMAGE_GROUP_PAPYRUS_MAKER);
        case BUILDING_BRICKWORKS:
            return image_group_entry(IMAGE_GROUP_BRICKWORKS);
        case BUILDING_CHARIOT_MAKER:
            return image_group_entry(IMAGE_GROUP_CHARIOT_MAKER);
        case BUILDING_PHYSICIAN:
            return image_group_entry(IMAGE_GROUP_PHYSICIAN);
        case BUILDING_COPPER_MINE:
            return image_group_entry(IMAGE_GROUP_COPPER_MINE);
        case BUILDING_SANDSTONE_QUARRY:
            return image_group_entry(IMAGE_GROUP_SANDSTONE_QUARRY);
        case BUILDING_ZOO:
            return image_group_entry(IMAGE_GROUP_ZOO);
        case BUILDING_ARTISANS_GUILD:
            return image_group_entry(IMAGE_GROUP_ARTISANS_GUILD);
        case BUILDING_LAMP_MAKER:
            return image_group_entry(IMAGE_GROUP_LAMP_MAKER);
        case BUILDING_PAINT_MAKER:
            return image_group_entry(IMAGE_GROUP_PAINT_MAKER);
        default:
            return NULL;
    }
}

static void remember_city_building(int32_t id, int32_t x, int32_t y, int32_t *building_id, int16_t *record_x, int16_t *record_y, int32_t *grid_offset)
{
    if (*grid_offset != 0) {
        return;
    }
    *building_id = id;
    *record_x = (int16_t) x;
    *record_y = (int16_t) y;
    *grid_offset = map_grid_offset(x, y);
}

static void place_palace(int32_t id, int32_t x, int32_t y, int32_t size, int32_t image_group_id)
{
    city_player *player = city_player_current();
    player->palace_placed = 1;
    place_square(id, x, y, size, image_group(image_group_id), TERRAIN_BUILDING);
    remember_city_building(id, x, y, &player->palace_building_id, &player->palace_x, &player->palace_y, &player->palace_grid_offset);
}

static void place_water(int32_t id, int32_t x, int32_t y, int32_t size3, const int32_t *normal_group, const int32_t *blood_group)
{
    if (size3) {
        map_water_determine_orientation_size3(x, y, 0);
    } else {
        map_water_determine_orientation_size2(x, y, 0, 0);
    }
    building_get(id)->data.dock.orientation = (uint8_t) water_orient_raw;
    int32_t image = *(map_sweep_has_passed(x) ? blood_group : normal_group) + map_water_orientation_relative;
    if (size3) {
        map_building_tiles_add_water_size3(id, x, y, image);
    } else {
        map_building_tiles_add_water_size2(id, x, y, image);
    }
}

static void finish_monument_place(void (*reset_work_spots)(void), int32_t clear_cursor, int32_t cancel)
{
    if (reset_work_spots) {
        reset_work_spots();
    }
    if (cancel) {
        construction_cancel();
    }
    if (clear_cursor) {
        clear_construction_cursor();
    }
}

static void place_royal_tomb(tomb_object *tomb, void (*place)(tomb_object *, int32_t, int32_t, int32_t, bool), int32_t x, int32_t y,
    int32_t id, bool no_slot, int32_t creating, int32_t cancel)
{
    if (creating) {
        place(tomb, x, y, id, no_slot);
        finish_monument_place(royal_tomb_reset_work_spots, 0, cancel);
        return;
    }
    royal_tomb_update_image(tomb, id);
}

static void place_storage_yard(int32_t player, int32_t id, int32_t x, int32_t y, int32_t size, int32_t creating)
{
    static const int32_t dx[STORAGE_YARD_ROOMS + 1] = { 0, 1, 2, 0, 1, 2, 0, 1, 2 };
    static const int32_t dy[STORAGE_YARD_ROOMS + 1] = { 0, 0, 0, 1, 1, 1, 2, 2, 2 };
    if (creating) {
        building *yard = building_get(id);
        yard->storage_id = building_storage_create(id);
        yard->prev_part_building_id = 0;
    }
    int32_t current = id;
    for (int32_t i = 0; i <= STORAGE_YARD_ROOMS; i++) {
        building *part = building_get(current);
        int32_t image = image_group((i == 0) ? IMAGE_GROUP_STORAGE_YARD : IMAGE_GROUP_STORAGE_YARD_SPACE);
        if (i > 0 && !creating && part->subtype != RESOURCE_NONE) {
            image = storage_room_image(part->subtype, part->loads_stored);
        }
        place_square(current, x + dx[i], y + dy[i], size, image, TERRAIN_BUILDING);
        if (i < STORAGE_YARD_ROOMS) {
            if (creating) {
                int32_t next_id = building_create(player, BUILDING_STORAGE_ROOM, x + dx[i + 1], y + dy[i + 1], 0, 0);
                game_undo_add_building(next_id);
                building_get(next_id)->prev_part_building_id = (int16_t) current;
                building_get(current)->next_part_building_id = (int16_t) next_id;
                current = next_id;
            } else {
                current = building_get(current)->next_part_building_id;
            }
        }
    }
}

static mission_legion_record *mission_legion_slot_get(int32_t slot)
{
    return &record_savings.started.legions[slot];
}

static int32_t mission_legion_slot_of(int32_t fort_type)
{
    if (fort_type == BUILDING_FORT_CHARIOTEERS) {
        return MISSION_LEGION_CHARIOTEERS;
    }
    return (fort_type == BUILDING_FORT_ARCHERS) ? MISSION_LEGION_ARCHERS : MISSION_LEGION_INFANTRY;
}

static void copy_formation_soldiers(int32_t formation_id, int32_t x, int32_t y, mission_legion_record *slot)
{
    if (slot->soldiers == 0) {
        return;
    }
    formation *m = formation_get(formation_id);
    m->legion_id = slot->legion_id;
    formation_add_soldiers(slot->soldiers, formation_id, x + FORT_SOLDIERS_DX, y + FORT_SOLDIERS_DY);
    m->experience = (int16_t) slot->experience;
    slot->experience = -1;
    slot->soldiers = 0;
    slot->legion_id = MISSION_LEGION_NONE;
}

static void place_fort(int32_t player, int32_t type, int32_t id, int32_t x, int32_t y, int32_t size, int32_t creating)
{
    place_square(id, x, y, size, image_group(IMAGE_GROUP_FORT), TERRAIN_BUILDING);
    building *fort = building_get(id);
    int32_t standard_id;
    if (creating) {
        int32_t formation_id = formation_create_legion(id);
        fort->prev_part_building_id = 0;
        fort->formation_id = (int16_t) formation_id;
        int32_t kind = (type == BUILDING_FORT_CHARIOTEERS) ? FIGURE_CHARIOTEER : (type == BUILDING_FORT_ARCHERS) ? FIGURE_ARCHER : FIGURE_INFANTRY;
        fort->subtype = (int16_t) kind;
        formation_get(formation_id)->figure_type = (int16_t) kind;
        copy_formation_soldiers(formation_id, x, y, mission_legion_slot_get(mission_legion_slot_of(type)));
        standard_id = building_create(player, BUILDING_FORT_GROUND, x + LEGION_STANDARD_FORT_DX, y + LEGION_STANDARD_FORT_DY, 0, 0);
        game_undo_add_building(standard_id);
        building *standard = building_get(standard_id);
        standard->formation_id = (int16_t) formation_id;
        standard->prev_part_building_id = (int16_t) id;
        fort->next_part_building_id = (int16_t) standard_id;
        standard->next_part_building_id = 0;
    } else {
        standard_id = fort->next_part_building_id;
    }
    map_building_tiles_add(standard_id, x + LEGION_STANDARD_FORT_DX, y + LEGION_STANDARD_FORT_DY, FORT_GROUND_SIZE, FORT_GROUND_SIZE, image_group(IMAGE_GROUP_FORT) + 1,
        TERRAIN_BUILDING);
}

static void place_storage_building(int32_t id, int32_t x, int32_t y, int32_t size, int32_t image_group_id, int32_t creating)
{
    if (creating) {
        city_resource_determine_available();
        building_get(id)->storage_id = building_storage_create(id);
    }
    place_square(id, x, y, size, image_group(image_group_id), TERRAIN_BUILDING);
}

static int32_t statue_image(int32_t type, int32_t orientation)
{
    int32_t direction = (orientation & STATUE_DATA_DIRECTION) - (map_info.orientation / 2) - 1;
    if (direction < 0) {
        direction += VIEW_TURNS;
    }
    int32_t variant = orientation >> STATUE_DATA_VARIANT_SHIFT;
    if (variant < STATUE_VARIANTS_IN_OWN_GROUP) {
        return image_group(building_property_get(type)->image_group) + (variant * VIEW_TURNS) + direction;
    }
    int32_t group = (type == BUILDING_SMALL_STATUE)  ? IMAGE_GROUP_SMALL_STATUE_VARIANTS
        : (type == BUILDING_MEDIUM_STATUE)           ? IMAGE_GROUP_MEDIUM_STATUE_VARIANTS
                                                     : IMAGE_GROUP_LARGE_STATUE_VARIANTS;
    return image_group(group) + ((variant - STATUE_VARIANTS_IN_OWN_GROUP) * VIEW_TURNS) + direction;
}

static void place_burning_ruin(int32_t id)
{
    building *b = building_get(id);
    if (b->state == BUILDING_STATE_RUBBLE) {
        map_building_tiles_set_rubble(id, b->x, b->y, b->size, b->size);
        return;
    }
    int32_t which = map_grids.random[b->grid_offset] & (BURNING_RUIN_VARIANTS - 1);
    int32_t image;
    if (b->house_size != 0 && b->subtype <= HOUSE_LEVEL_STURDY_HUT) {
        image = image_group(IMAGE_GROUP_RUIN_SMALL_HOUSE);
    } else {
        if (which > 2) {
            which = 3;
        }
        image = image_group(IMAGE_GROUP_RUINS) + (which * BURNING_RUIN_IMAGE_STEP);
    }
    map_building_tiles_add(id, b->x, b->y, 1, 1, image, TERRAIN_BUILDING);
}

static void place_ferry_landing(int32_t id, int32_t x, int32_t y, int32_t creating)
{
    if (creating) {
        building *b = building_get(id);
        int32_t which = (uint8_t) building_ferry_record_create(id);
        b->queue_index = (uint8_t) which;
        ring_queue_init(&ferry_queue_get(which)->queue);
        if (construction_ferry_state == FERRY_PLACEMENT_NONE) {
            b->linked_building_id = 0;
        }
    }
    place_water(id, x, y, 0, image_group_entry(IMAGE_GROUP_FERRY_LANDING), image_group_entry(IMAGE_GROUP_FERRY_LANDING_BLOOD));
    if (construction_ferry_state == FERRY_PLACEMENT_FIRST_PLACED) {
        game_undo_add_building(construction_ferry_first);
        game_undo_add_building(id);
    } else if (construction_ferry_state == FERRY_PLACEMENT_FIRST_EXISTING) {
        game_undo_add_building(id);
    }
}

static void create_sphinx_parts(int32_t player, int32_t id, int32_t x, int32_t y)
{
    city_player_current()->sphinx_stage = SPHINX_FIRST_STAGE;
    sphinx_reset_work_spots();
    int32_t side = map_info.orientation == 0 || map_info.orientation == 4;
    building *first = building_get(id);
    first->subtype = (int16_t) (side ? SPHINX_PART_LEFT : SPHINX_PART_LEFT_TURNED);
    first->prev_part_building_id = 0;
    int32_t second_id = building_create(player, BUILDING_SPHINX, x + SPHINX_PART_DX, y, 0, 0);
    game_undo_add_building(second_id);
    building *second = building_get(second_id);
    second->subtype = (int16_t) (side ? SPHINX_PART_MIDDLE : SPHINX_PART_MIDDLE_TURNED);
    second->prev_part_building_id = (int16_t) id;
    first->next_part_building_id = (int16_t) second_id;
    second->next_part_building_id = 0;
    int32_t third_id = building_create(player, BUILDING_SPHINX, x + (2 * SPHINX_PART_DX), y, 0, 0);
    game_undo_add_building(third_id);
    building *third = building_get(third_id);
    third->subtype = (int16_t) (side ? SPHINX_PART_RIGHT : SPHINX_PART_RIGHT_TURNED);
    third->prev_part_building_id = (int16_t) second_id;
    second->next_part_building_id = (int16_t) third_id;
    third->next_part_building_id = 0;
    monument_register(id);
}

static void place_sphinx_images(int32_t id, int32_t x, int32_t y, int32_t size)
{
    int32_t view = map_info.orientation;
    int32_t first = (view == 0) ? image_group(IMAGE_GROUP_SPHINX_END_A) + 1
        : (view == 4)           ? image_group(IMAGE_GROUP_SPHINX_END_B) + 1
        : (view == 2)           ? image_group(IMAGE_GROUP_SPHINX_END_B)
                                : image_group(IMAGE_GROUP_SPHINX_END_A);
    place_square(id, x, y, size, first, TERRAIN_BUILDING);
    int32_t second_id = building_get(id)->next_part_building_id;
    int32_t middle = ((view == 0) || (view == 4)) ? image_group(IMAGE_GROUP_SPHINX_MIDDLE) + 1 : image_group(IMAGE_GROUP_SPHINX_MIDDLE);
    place_square(second_id, x + SPHINX_PART_DX, y, size, middle, TERRAIN_BUILDING);
    int32_t last = (view == 0) ? image_group(IMAGE_GROUP_SPHINX_END_B) + 1
        : (view == 4)          ? image_group(IMAGE_GROUP_SPHINX_END_A) + 1
        : (view == 6)          ? image_group(IMAGE_GROUP_SPHINX_END_B)
                               : image_group(IMAGE_GROUP_SPHINX_END_A);
    place_square(building_get(second_id)->next_part_building_id, x + (2 * SPHINX_PART_DX), y, size, last, TERRAIN_BUILDING);
}

static void place_venue_plaza(int32_t id, int32_t type)
{
    building *b = building_get(id);
    if (building_main(id) == id && b->venue_has_plaza != 0) {
        map_add_venue_plaza_tiles(b->venue_plaza_grid_offset, type, id);
    }
}

void building_construction_add_to_map(int32_t player, int32_t type, int32_t id, int32_t x, int32_t y, int32_t size, int32_t creating, bool no_slot)
{
    building *b = building_get(id);
    if (type == BUILDING_FORT) {
        int32_t subtype = b->subtype;
        type = (subtype == FIGURE_CHARIOTEER) ? BUILDING_FORT_CHARIOTEERS : BUILDING_FORT_ARCHERS + (subtype != FIGURE_ARCHER);
    }
    switch (type) {
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
            if (b->data.house.is_sick == 0 && b->house_population == 0) {
                building_house_change_to_vacant_lot(id);
            } else {
                building_house_change_to(id, type);
            }
            return;
        case BUILDING_BANDSTAND:
            if (building_get(b->next_part_building_id)->type == BUILDING_BANDSTAND) {
                venue_place_adjacent_tiles(id, x, y);
            }
            place_venue_plaza(id, BUILDING_BANDSTAND);
            return;
        case BUILDING_BOOTH:
            place_square(id, x, y, size, image_group(IMAGE_GROUP_BOOTH), TERRAIN_BUILDING);
            place_venue_plaza(id, BUILDING_BOOTH);
            return;
        case BUILDING_PAVILION:
            map_building_tiles_add(id, x, y, PAVILION_STAGE_TILES, PAVILION_STAGE_TILES, image_group(IMAGE_GROUP_PAVILION), TERRAIN_BUILDING);
            place_venue_plaza(id, BUILDING_PAVILION);
            return;
        case BUILDING_SMALL_STATUE:
        case BUILDING_MEDIUM_STATUE:
        case BUILDING_LARGE_STATUE:
            place_square(id, x, y, size, statue_image(type, b->data.oriented.orientation), TERRAIN_BUILDING);
            return;
        case BUILDING_FORT_CHARIOTEERS:
        case BUILDING_FORT_ARCHERS:
        case BUILDING_FORT_INFANTRY:
            place_fort(player, type, id, x, y, size, creating);
            return;
        case BUILDING_DISTRIBUTION_CENTER: {
            city_player *city = city_player_current();
            city->distribution_center_placed = 1;
            place_square(id, x, y, size, image_group(IMAGE_GROUP_RECRUITER), TERRAIN_BUILDING);
            remember_city_building(id, x, y, &city->distribution_center_building_id, &city->distribution_center_x, &city->distribution_center_y,
                &city->distribution_center_grid_offset);
            return;
        }
        case BUILDING_BAZAAR:
        case BUILDING_BAZAAR_UP:
            place_storage_building(id, x, y, size, IMAGE_GROUP_BAZAAR, creating);
            return;
        case BUILDING_GRANARY:
        case BUILDING_GRANARY_UP:
            place_storage_building(id, x, y, size, IMAGE_GROUP_GRANARY, creating);
            return;
        case BUILDING_STORAGE_YARD:
        case BUILDING_STORAGE_YARD_UP:
            place_storage_yard(player, id, x, y, size, creating);
            return;
        case BUILDING_SHIPWRIGHT:
            place_water(id, x, y, 1, image_group_entry(IMAGE_GROUP_SHIPWRIGHT), image_group_entry(IMAGE_GROUP_SHIPWRIGHT_BLOOD));
            return;
        case BUILDING_DOCK:
        case BUILDING_DOCK_UP:
            if (creating) {
                building_count_total[BUILDING_DOCK]++;
            }
            place_water(id, x, y, 1, image_group_entry(IMAGE_GROUP_DOCK), image_group_entry(IMAGE_GROUP_DOCK_BLOOD));
            return;
        case BUILDING_FISHING_WHARF:
            place_water(id, x, y, 0, image_group_entry(IMAGE_GROUP_FISHING_WHARF), image_group_entry(IMAGE_GROUP_FISHING_WHARF_BLOOD));
            return;
        case BUILDING_PERSONAL_MANSION:
            city_player_current()->mansion_placed = 1;
            place_square(id, x, y, size, image_group(IMAGE_GROUP_PERSONAL_MANSION), TERRAIN_BUILDING);
            return;
        case BUILDING_FAMILY_MANSION:
            city_player_current()->mansion_placed = 1;
            place_square(id, x, y, size, image_group(IMAGE_GROUP_FAMILY_MANSION), TERRAIN_BUILDING);
            return;
        case BUILDING_DYNASTY_MANSION:
            city_player_current()->mansion_placed = 1;
            place_square(id, x, y, size, image_group(IMAGE_GROUP_DYNASTY_MANSION), TERRAIN_BUILDING);
            return;
        case BUILDING_VILLAGE_PALACE:
        case BUILDING_TOWN_PALACE:
        case BUILDING_VILLAGE_PALACE_UP:
            place_palace(id, x, y, size, IMAGE_GROUP_VILLAGE_PALACE);
            return;
        case BUILDING_TOWN_PALACE_UP:
            place_palace(id, x, y, size, IMAGE_GROUP_TOWN_PALACE);
            return;
        case BUILDING_CITY_PALACE:
            place_palace(id, x, y, size, IMAGE_GROUP_CITY_PALACE);
            return;
        case BUILDING_NATIVE_HUT:
            place_square(id, x, y, size, (crt_rand() & 1) + image_group(IMAGE_GROUP_NATIVE_HUT), TERRAIN_BUILDING);
            return;
        case BUILDING_NATIVE_MEETING:
            place_square(id, x, y, size, image_group(IMAGE_GROUP_NATIVE_HUT) + NATIVE_MEETING_IMAGE_OFFSET, TERRAIN_BUILDING);
            return;
        case BUILDING_WATER_LIFT:
            if (creating) {
                map_water_determine_orientation_size2(x, y, 0, 1);
                b->data.oriented.orientation = (uint8_t) water_orient_raw;
            }
            map_building_update_water_lift_image(id);
            return;
        case BUILDING_NATIVE_CROPS:
            place_square(id, x, y, size, image_group(IMAGE_GROUP_GRAIN_CROPS), TERRAIN_BUILDING);
            return;
        case BUILDING_MILITARY_ACADEMY:
            city_player_current()->military_academy_placed = 1;
            place_square(id, x, y, size, image_group(IMAGE_GROUP_MILITARY_ACADEMY), TERRAIN_BUILDING);
            return;
        case BUILDING_MILITARY_ACADEMY_2:
        case BUILDING_MILITARY_ACADEMY_3:
            place_square(id, x, y, size, image_group(IMAGE_GROUP_MILITARY_ACADEMY), TERRAIN_BUILDING);
            return;
        case BUILDING_RECRUITER: {
            city_player *city = city_player_current();
            place_square(id, x, y, size, image_group(IMAGE_GROUP_RECRUITER), TERRAIN_BUILDING);
            remember_city_building(id, x, y, &city->recruiter_building_id, &city->recruiter_x, &city->recruiter_y, &city->recruiter_grid_offset);
            return;
        }
        case BUILDING_BURNING_RUIN:
            place_burning_ruin(id);
            return;
        case BUILDING_BARLEY_FARM:
        case BUILDING_FLAX_FARM:
        case BUILDING_GRAIN_FARM:
        case BUILDING_LETTUCE_FARM:
        case BUILDING_POMEGRANATES_FARM:
        case BUILDING_CHICKPEAS_FARM:
        case BUILDING_CATTLE_RANCH:
        case BUILDING_REED_GATHERER:
        case BUILDING_FIG_FARM:
        case BUILDING_HENNA_FARM:
            map_building_place_farm(id, 0);
            return;
        case BUILDING_LIMESTONE_QUARRY:
            place_square(id, x, y, size, image_group(quarry_image_group(RESOURCE_LIMESTONE)), TERRAIN_BUILDING);
            return;
        case BUILDING_GRANITE_QUARRY:
            place_square(id, x, y, size, image_group(quarry_image_group(RESOURCE_GRANITE)), TERRAIN_BUILDING);
            return;
        case BUILDING_FERRY_LANDING:
            place_ferry_landing(id, x, y, creating);
            return;
        case BUILDING_ROADBLOCK:
            place_square(id, x, y, size, image_group(IMAGE_GROUP_ROADBLOCK),
                (int32_t) ((map_grids.terrain[map_grid_offset(x, y)] & TERRAIN_CANAL) | (TERRAIN_BUILDING | TERRAIN_ROAD)));
            return;
        case BUILDING_SHRINE_OSIRIS:
        case BUILDING_SHRINE_RA:
        case BUILDING_SHRINE_PTAH:
        case BUILDING_SHRINE_SETH:
        case BUILDING_SHRINE_BAST:
        case BUILDING_SHRINE_GOD_5:
        case BUILDING_SHRINE_GOD_6:
        case BUILDING_SHRINE_GOD_7:
        case BUILDING_SHRINE_GOD_8:
        case BUILDING_SHRINE_GOD_9:
            place_square(id, x, y, size, image_group(building_property_get(type)->image_group), TERRAIN_BUILDING);
            return;
        case BUILDING_TEMPLE_COMPLEX_OSIRIS:
        case BUILDING_TEMPLE_COMPLEX_RA:
        case BUILDING_TEMPLE_COMPLEX_PTAH:
        case BUILDING_TEMPLE_COMPLEX_SETH:
        case BUILDING_TEMPLE_COMPLEX_BAST:
        case BUILDING_TEMPLE_COMPLEX_GOD_5:
        case BUILDING_TEMPLE_COMPLEX_GOD_6:
        case BUILDING_TEMPLE_COMPLEX_GOD_7:
        case BUILDING_TEMPLE_COMPLEX_GOD_8:
        case BUILDING_TEMPLE_COMPLEX_GOD_9:
            image_collection_load_file(temple_complex_god_script_names[building_temple_god(id)], TEMPLE_COMPLEX_SCRIPT_SLOT);
            building_temple_complex_add_to_map(id, x, y, player, type, creating);
            return;
        case BUILDING_GATEHOUSE:
        case BUILDING_GATEHOUSE_UP:
            building_gatehouse_update_image(id);
            return;
        case BUILDING_TOWER:
            map_terrain_and_area(x, y, TOWER_SIZE, 0, ~(uint32_t) TERRAIN_WALL_KINDS);
            place_square(id, x, y, size, image_group(IMAGE_GROUP_TOWER), (TERRAIN_BUILDING | TERRAIN_GATEHOUSE));
            map_tiles_update_region_walls(x, y, TOWER_WALL_UPDATE_RADIUS);
            return;
        case BUILDING_TRANSPORT_WHARF:
            place_water(id, x, y, 0, image_group_entry(IMAGE_GROUP_TRANSPORT_WHARF), image_group_entry(IMAGE_GROUP_TRANSPORT_WHARF_BLOOD));
            return;
        case BUILDING_WARSHIP_WHARF:
            place_water(id, x, y, 1, image_group_entry(IMAGE_GROUP_WARSHIP_WHARF), image_group_entry(IMAGE_GROUP_WARSHIP_WHARF_BLOOD));
            if (creating) {
                building_warship_wharf_count++;
            }
            return;
        case BUILDING_PYRAMID:
            if (creating) {
                pyramid_create_parts(player, x, y, id);
            }
            if (building_get(id)->prev_part_building_id == 0) {
                pyramid_update_all_part_images(id);
            }
            if (creating) {
                construction_cancel();
            }
            return;
        case BUILDING_SMALL_OBELISK:
        case BUILDING_LARGE_OBELISK:
            if (creating) {
                obelisk_init_on_place(id);
            }
            obelisk_update_image(id, x, y);
            return;
        case BUILDING_FESTIVAL_SQUARE:
            map_add_venue_plaza_tiles(b->venue_plaza_grid_offset, BUILDING_FESTIVAL_SQUARE, id);
            city_player_current()->festival_square_placed = 1;
            return;
        case BUILDING_SPHINX:
            if (creating) {
                create_sphinx_parts(player, id, x, y);
            }
            if (building_main(id) == id) {
                place_sphinx_images(id, x, y, size);
            }
            sphinx_load_stage_scripts();
            return;
        case BUILDING_TEMPLE_COMPLEX_ALTAR:
        case BUILDING_TEMPLE_COMPLEX_ORACLE: {
            int32_t temple_id = city_player_current()->temple_complex_building_id;
            if (creating) {
                building *temple = building_get(temple_id);
                temple->data.temple_complex.upgrades = (uint8_t) (temple->data.temple_complex.upgrades + (type != BUILDING_TEMPLE_COMPLEX_ALTAR) + 1);
            }
            building_temple_complex_update_image(temple_id, 0);
            return;
        }
        case BUILDING_SUN_TEMPLE:
            if (creating) {
                sun_temple_place(x, y, id);
                sun_temple_reset_work_spots();
                clear_construction_cursor();
            } else {
                sun_temple_update_image(id);
            }
            return;
        case BUILDING_PYRAMID_ENTRY_TILE:
            place_square(id, x, y, size, image_group(IMAGE_GROUP_PYRAMID_ENTRY_TILE), (TERRAIN_BUILDING | TERRAIN_ROAD));
            return;
        case BUILDING_MORTUARY_TEMPLE:
            place_square(id, x, y, size, ((map_info.orientation / 2) & 1) + image_group(IMAGE_GROUP_MORTUARY_TEMPLE), TERRAIN_BUILDING);
            return;
        case BUILDING_CAUSEWAY:
            place_square(id, x, y, size, ((map_info.orientation / 2) & 1) + image_group(IMAGE_GROUP_CAUSEWAY), TERRAIN_BUILDING);
            return;
        case BUILDING_VALLEY_TEMPLE:
            map_building_tiles_add_water_size2(id, x, y, (map_info.orientation / 2) + image_group(IMAGE_GROUP_VALLEY_TEMPLE));
            return;
        case BUILDING_MAUSOLEUM:
            if (creating) {
                mausoleum_place(x, y, id, no_slot);
                clear_construction_cursor();
            } else {
                mausoleum_update_image(id);
            }
            return;
        case BUILDING_ALEXANDRIA_LIBRARY:
            if (creating) {
                library_place(&library_object.base, x, y, id, no_slot);
                finish_monument_place(library_reset_work_spots, 1, 0);
            } else {
                library_update_image(&library_object.base, id);
            }
            return;
        case BUILDING_CAESAREUM:
            if (creating) {
                caesareum_place(&caesareum_object.base, x, y, id, 0);
                finish_monument_place(caesareum_reset_work_spots, 0, 1);
            } else {
                caesareum_update_image(&caesareum_object.base, id);
            }
            return;
        case BUILDING_LIGHTHOUSE:
            if (creating) {
                lighthouse_place(&lighthouse_object.base, x, y, id, no_slot);
                finish_monument_place(lighthouse_reset_work_spots, 1, 0);
            } else {
                lighthouse_update_image(&lighthouse_object.base, id);
            }
            return;
        case BUILDING_ABU_SIMBEL:
            if (creating) {
                abu_simbel_place(&abu_simbel_object.base, x, y, id, no_slot);
                finish_monument_place(abu_simbel_reset_work_spots, 1, 0);
            } else {
                abu_simbel_update_image(&abu_simbel_object.base, id);
            }
            return;
        case BUILDING_SMALL_ROYAL_TOMB:
            place_royal_tomb(&small_royal_tomb_object.base, small_royal_tomb_place, x, y, id, no_slot, creating, 0);
            if (creating) {
                construction_building_type = BUILDING_NONE;
            }
            return;
        case BUILDING_MEDIUM_ROYAL_TOMB:
            place_royal_tomb(&medium_royal_tomb_object.base, medium_royal_tomb_place, x, y, id, no_slot, creating, 0);
            if (creating) {
                construction_building_type = BUILDING_NONE;
            }
            return;
        case BUILDING_LARGE_ROYAL_TOMB:
            place_royal_tomb(&large_royal_tomb_object.base, large_royal_tomb_place, x, y, id, no_slot, creating, 0);
            if (creating) {
                construction_building_type = BUILDING_NONE;
            }
            return;
        case BUILDING_GRAND_ROYAL_TOMB:
            place_royal_tomb(&grand_royal_tomb_object.base, grand_royal_tomb_place, x, y, id, 0, creating, 1);
            return;
        default:
            break;
    }
    const int32_t *group = plain_image_group(type);
    if (group != NULL) {
        place_square(id, x, y, size, *group, TERRAIN_BUILDING);
    }
}
