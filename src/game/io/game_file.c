// Saved games and scenarios: the chunk tables, reading and writing the chunks, converting the chunks of older files
// whose layout changed, and what loading rebuilds (grid geometry, graphics, routing, road networks, coverage, the
// view). Also the campaign missions packed into mission<n>.pak and the mission autosave. Files before the oldest
// supported version are refused.
//
// What loading and saving keep:
// - converters run on the read buffer, before the move for a smaller layout and after it for a larger one, so they
//   change nothing that stays;
// - the route paths are drawn again after a save with the empire map's line drawing, so they can differ from the
//   paths after loading that save;
// - the scenario points outside the map are checked twice.

#include "building/building.h"
#include "building/building_image_group.h"
#include "building/building_maintenance.h"
#include "building/building_state.h"
#include "building/building_stocks.h"
#include "building/building_storage.h"
#include "building/building_type.h"
#include "building/ferry_landing.h"
#include "building/model_load.h"
#include "building/monument.h"
#include "building/monument_city.h"
#include "building/monument_data.h"
#include "building/monument_object.h"
#include "building/monument_spots.h"
#include "building/obelisk.h"
#include "building/royal_tomb.h"
#include "building/shipwright.h"
#include "city/city_player.h"
#include "city/culture.h"
#include "city/kingdom.h"
#include "city/message.h"
#include "city/ratings.h"
#include "city/resource_type.h"
#include "city/resources.h"
#include "core/calc.h"
#include "editor/editor_allowed_buildings.h"
#include "editor/editor_data.h"
#include "editor/editor_terrain_tiles.h"
#include "empire/empire.h"
#include "empire/empire_data.h"
#include "figure/boats.h"
#include "figure/carriers.h"
#include "figure/enemy.h"
#include "figure/enemy_ships.h"
#include "figure/figure_core.h"
#include "figure/route.h"
#include "flood/flood_internal.h"
#include "game/discord_presence.h"
#include "game/game_data.h"
#include "game/player_record.h"
#include "game/scenario_start.h"
#include "game/undo.h"
#include "graphics/image_collection.h"
#include "io/empire_file.h"
#include "io/file.h"
#include "io/file_write.h"
#include "io/game_file_internal.h"
#include "io/game_file_relayout.h"
#include "io/io_data.h"
#include "map/grid.h"
#include "map/grid_size.h"
#include "map/map_data.h"
#include "map/moisture.h"
#include "map/orientation.h"
#include "map/road_network.h"
#include "map/routing_citizen.h"
#include "map/routing_terrain.h"
#include "map/terrain.h"
#include "map/tiles.h"
#include "map/vegetation.h"
#include "platform/platform_file.h"
#include "scenario/event_manager.h"
#include "sound/city_view.h"
#include "sound/sound_data.h"
#include "sound/sound_lib.h"
#include "sound/sound_lib_samples.h"
#include "text/lang.h"
#include "text/text_lookup.h"
#include "widget/city_view.h"
#include "widget/widget_data.h"
#include "window/building_menu.h"
#include "window/hotkeys.h"
#include "window/main_menu.h"
#include "window/message_box.h"
#include "window/mission_flow.h"
#include "window/player_selection.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint8_t game_loaded_flag;

enum game_file_chunk {
    SAVED_NAME_COUNTER_FIRST_GROUP = 275,
    TUTORIAL_FLAGS_A_SAVED = 0x1A,
    TUTORIAL_FLAGS_B_SAVED = 0xF,
    MISSION_START_RECORDS_SAVED = 12,
    TRADE_PRICE_SIZE = 2 * sizeof(int32_t),
    SAVEGAME_CHUNKS_STATIC = 147,
    SCENARIO_CHUNKS_STATIC = 21,
    SAVEGAME_CHUNK_MONUMENT_SPOT_STATE = 175,
};

enum game_file_limit {
    INVASION_POINT_CHECKS = 2,
    ALLOWED_BUILDINGS_RESET = 100,
    ALLOWED_BUILDINGS_RESET_ON = 50,
    SCENARIO_ALLOWS_CLEARED_SIZE = 0x6E,
    NATIVE_IMAGE_MEETING_OFFSET = 2,
    MISSION_ID_NONE = 0x7F,
    CITY_VIEW_ORIENTATION_MAX = 6,
    AUTOSAVE_PATH_SIZE = 100,
    OBELISK_SMALL_SPOTS = 10,
    OBELISK_LARGE_SPOTS = 18,
    SPHINX_SPOTS = 22,
    SUN_TEMPLE_SPOTS = 13,
};

enum game_file_text {
    POPUP_TEXT_OLD_FILE = 24,
    POPUP_TEXT_OLD_FILE_TITLE = 25,
};

static const chunk_entry savegame_chunks_ext[SAVEGAME_CHUNKS - SAVEGAME_CHUNKS_STATIC] = {
    { 1, &floods_data.settings, sizeof(floodplain_settings), 1, 0 },
    { 1, map_service_visit_grid, sizeof(uint32_t), GRID_CELLS, 0 },
    { 0, sun_temple_spot_objects, sizeof(monument_spot), sizeof(sun_temple_spot_objects) / sizeof(sun_temple_spot_objects[0]), 0 },
    { 0, &figure_name_counter_table.by_slot[FIGURE_NAME_GROUP_COUNTER_BASE - (SAVED_NAME_COUNTER_FIRST_GROUP + 0)], sizeof(int32_t), 1, 0 },
    { 0, &figure_name_counter_table.by_slot[FIGURE_NAME_GROUP_COUNTER_BASE - (SAVED_NAME_COUNTER_FIRST_GROUP + 1)], sizeof(int32_t), 1, 0 },
    { 0, &figure_name_counter_table.by_slot[FIGURE_NAME_GROUP_COUNTER_BASE - (SAVED_NAME_COUNTER_FIRST_GROUP + 2)], sizeof(int32_t), 1, 0 },
    { 0, &figure_name_counter_table.by_slot[FIGURE_NAME_GROUP_COUNTER_BASE - (SAVED_NAME_COUNTER_FIRST_GROUP + 3)], sizeof(int32_t), 1, 0 },
    { 0, &figure_name_counter_table.by_slot[FIGURE_NAME_GROUP_COUNTER_BASE - (SAVED_NAME_COUNTER_FIRST_GROUP + 4)], sizeof(int32_t), 1, 0 },
    { 0, &figure_name_counter_table.by_slot[FIGURE_NAME_GROUP_COUNTER_BASE - (SAVED_NAME_COUNTER_FIRST_GROUP + 5)], sizeof(int32_t), 1, 0 },
    { 0, &figure_name_counter_table.by_slot[FIGURE_NAME_GROUP_COUNTER_BASE - (SAVED_NAME_COUNTER_FIRST_GROUP + 6)], sizeof(int32_t), 1, 0 },
    { 0, &figure_name_counter_table.by_slot[FIGURE_NAME_GROUP_COUNTER_BASE - (SAVED_NAME_COUNTER_FIRST_GROUP + 7)], sizeof(int32_t), 1, 0 },
    { 0, &figure_name_counter_table.by_slot[FIGURE_NAME_GROUP_COUNTER_BASE - (SAVED_NAME_COUNTER_FIRST_GROUP + 8)], sizeof(int32_t), 1, 0 },
    { 0, &figure_name_counter_table.by_slot[FIGURE_NAME_GROUP_COUNTER_BASE - (SAVED_NAME_COUNTER_FIRST_GROUP + 9)], sizeof(int32_t), 1, 0 },
    { 0, &figure_name_counter_table.by_slot[FIGURE_NAME_GROUP_COUNTER_BASE - (SAVED_NAME_COUNTER_FIRST_GROUP + 10)], sizeof(int32_t), 1, 0 },
    { 0, &figure_name_counter_table.by_slot[FIGURE_NAME_GROUP_COUNTER_BASE - (SAVED_NAME_COUNTER_FIRST_GROUP + 11)], sizeof(int32_t), 1, 0 },
    { 0, &figure_name_counter_table.by_slot[FIGURE_NAME_GROUP_COUNTER_BASE - (SAVED_NAME_COUNTER_FIRST_GROUP + 12)], sizeof(int32_t), 1, 0 },
    { 0, &figure_name_counter_table.by_slot[FIGURE_NAME_GROUP_COUNTER_BASE - (SAVED_NAME_COUNTER_FIRST_GROUP + 13)], sizeof(int32_t), 1, 0 },
    { 0, &figure_name_counter_table.by_slot[FIGURE_NAME_GROUP_COUNTER_BASE - (SAVED_NAME_COUNTER_FIRST_GROUP + 14)], sizeof(int32_t), 1, 0 },
    { 0, &figure_name_counter_table.by_slot[FIGURE_NAME_GROUP_COUNTER_BASE - (SAVED_NAME_COUNTER_FIRST_GROUP + 15)], sizeof(int32_t), 1, 0 },
    { 0, tutorial_flags_a, sizeof(uint8_t), TUTORIAL_FLAGS_A_SAVED, 0 },
    { 0, tutorial_flags_b, sizeof(uint8_t), TUTORIAL_FLAGS_B_SAVED, 0 },
    { 1, map_grids.rubble_building_type, sizeof(uint8_t), GRID_CELLS, 0 },
    { 0, &game_is_replay, sizeof(game_is_replay), 1, 0 },
    { 1, map_grids.moisture, sizeof(uint8_t), GRID_CELLS, 0 },
    { 0, obelisk_small_spot_objects, sizeof(monument_spot), sizeof(obelisk_small_spot_objects) / sizeof(obelisk_small_spot_objects[0]), 0 },
    { 0, obelisk_large_spot_objects, sizeof(monument_spot), sizeof(obelisk_large_spot_objects) / sizeof(obelisk_large_spot_objects[0]), 0 },
    { 0, &mummies_max, sizeof(mummies_max), 1, 0 },
    { 0, &settings_data.difficulty, sizeof(settings_data.difficulty), 1, 0 },
    { 0, &record_savings.started, MISSION_START_RECORDS_SAVED, 1, 0 },
    { 0, library_spot_objects, sizeof(monument_spot), sizeof(library_spot_objects) / sizeof(library_spot_objects[0]), 0 },
    { 0, caesareum_spot_objects, sizeof(monument_spot), sizeof(caesareum_spot_objects) / sizeof(caesareum_spot_objects[0]), 0 },
    { 0, lighthouse_spot_objects, sizeof(monument_spot), sizeof(lighthouse_spot_objects) / sizeof(lighthouse_spot_objects[0]), 0 },
    { 0, abu_simbel_spot_objects, sizeof(monument_spot), sizeof(abu_simbel_spot_objects) / sizeof(abu_simbel_spot_objects[0]), 0 },
    { 0, royal_tomb_chamber_spot_objects, sizeof(monument_spot), sizeof(royal_tomb_chamber_spot_objects) / sizeof(royal_tomb_chamber_spot_objects[0]), 0 },
};

static const chunk_entry scenario_chunks_ext[SCENARIO_CHUNKS - SCENARIO_CHUNKS_STATIC] = {
    { 1, &floods_data.settings, sizeof(floodplain_settings), 1, 0 },
    { 0, trade_prices, TRADE_PRICE_SIZE, RESOURCE_MAX, 0 },
    { 1, map_grids.moisture, sizeof(uint8_t), GRID_CELLS, 0 },
};

static void init_table_ext(chunk_entry *table, int32_t first, const chunk_entry *ext, int32_t count)
{
    for (int32_t i = 0; i < count; i++) {
        chunk_entry *c = &table[first + i];
        if (i > 0) {
            c->compressed = ext[i].compressed;
        }
        c->data = ext[i].data;
        c->element_size = ext[i].element_size;
        c->count = ext[i].count;
        c->converter = ext[i].converter;
    }
}

void game_file_init_save_chunk_table_ext(void)
{
    init_table_ext(savegame_chunk_table, SAVEGAME_CHUNKS_STATIC, savegame_chunks_ext, SAVEGAME_CHUNKS - SAVEGAME_CHUNKS_STATIC);
}

void game_file_init_map_chunk_table_ext(void)
{
    init_table_ext(scenario_chunk_table, SCENARIO_CHUNKS_STATIC, scenario_chunks_ext, SCENARIO_CHUNKS - SCENARIO_CHUNKS_STATIC);
}

static void read_raw_or_compressed(int32_t compressed, int32_t fd, void *buffer, int32_t size)
{
    if (compressed) {
        io_read_compressed_chunk(fd, buffer, size);
    } else {
        platform_file_read(fd, buffer, (uint32_t) size);
    }
}

static void convert_elements(const chunk_entry *c, int32_t converter, uint8_t *elements, int32_t stride, int32_t from_file, int32_t count)
{
    int32_t i = 0;
    for (; i < from_file; i++) {
        c->converter[converter].convert(elements + i * stride, 1);
    }
    for (; i < count; i++) {
        c->converter[converter].convert(elements + i * stride, 0);
    }
}

static int32_t read_chunk_converted(chunk_entry *c, const chunk_schema_record *s, int32_t converter, int32_t fd)
{
    int32_t size = c->element_size;
    int32_t count = c->count;
    int32_t old_size = s->element_size;
    int32_t old_count = s->count;
    int32_t old_bytes = old_size * old_count;
    if (size * count < old_bytes) {
        uint8_t *buffer = malloc((size_t) old_bytes);
        if (!buffer) {
            return 0;
        }
        memset(buffer, 0, (size_t) old_bytes);
        read_raw_or_compressed(s->compressed, fd, buffer, old_bytes);
        if (c->converter && size < old_size) {
            convert_elements(c, converter, buffer, old_size, old_count, count);
        }
        for (int32_t i = count - 1; i >= 0; i--) {
            uint8_t *element = (uint8_t *) c->data + i * size;
            memmove(element, buffer + i * old_size, (size_t) old_size);
            if (size > old_size) {
                memset(element + old_size, 0, (size_t) (size - old_size));
            }
        }
        if (c->converter && size >= old_size) {
            convert_elements(c, converter, buffer, old_size, old_count, count);
        }
        if (!c->converter && count > old_count) {
            memset((uint8_t *) c->data + old_count * size, 0, (size_t) ((count - old_count) * size));
        }
        free(buffer);
        return 1;
    }
    memset(c->data, 0, (size_t) old_bytes);
    read_raw_or_compressed(s->compressed, fd, c->data, old_bytes);
    for (int32_t i = old_count - 1; i >= 0; i--) {
        uint8_t *element = (uint8_t *) c->data + i * size;
        memmove(element, (uint8_t *) c->data + i * old_size, (size_t) old_size);
        if (size - old_size > 0) {
            memset(element + old_size, 0, (size_t) (size - old_size));
        }
    }
    if (c->converter) {
        convert_elements(c, converter, c->data, size, old_count, c->count);
    } else if (c->count > old_count) {
        memset((uint8_t *) c->data + old_count * size, 0, (size_t) ((c->count - old_count) * size));
    }
    return 1;
}

static void show_old_file_error(void)
{
    const char *text = lang_get_string(LANG_GROUP_POPUP, POPUP_TEXT_OLD_FILE);
    const char *title = lang_get_string(LANG_GROUP_POPUP, POPUP_TEXT_OLD_FILE_TITLE);
    window_message_box_show(title, text, 0);
}

int32_t game_file_io_read_chunk(int32_t index, chunk_entry *table, int32_t fd)
{
    chunk_entry *c = &table[index];
    const chunk_schema_record *s = &savegame_chunk_schema[index];
    int32_t layout_changed = savegame_schema_chunk_count && c->data != (void *) savegame_chunk_schema &&
        (c->element_size != s->element_size || c->count != s->count);
    int32_t converter = 0;
    int32_t convert = 0;
    if (c->converter) {
        while (c->converter[converter].version && c->converter[converter].version < savegame_file_version) {
            converter++;
        }
        convert = c->converter[converter].version != 0;
    }
    if (convert || layout_changed) {
        if (!read_chunk_converted(c, s, converter, fd)) {
            return 0;
        }
    } else {
        read_raw_or_compressed(c->compressed, fd, c->data, c->element_size * c->count);
    }
    if (savegame_file_version < SAVE_VERSION_OLDEST) {
        show_old_file_error();
        return 0;
    }
    return 1;
}

static void free_empire_route_paths(void)
{
    empire_map_routes.routes[0].num_points = 0;
    empire_map_routes.routes[0].path_length = 0;
    empire_route_free_paths();
}

static void build_empire_route_paths(void)
{
    empire_map_routes.routes[0].num_points = 0;
    empire_map_routes.routes[0].path_length = 0;
    for (empire_route *r = empire_map_routes.routes; r < empire_map_routes.routes + EMPIRE_ROUTES; r++) {
        if (empire_route_is_valid(r)) {
            game_file_build_empire_route_path(r);
        } else {
            empire_route_reset(r);
        }
    }
}

static void rebuild_empire_route_paths_after_save(void)
{
    for (empire_route *r = empire_map_routes.routes; r < empire_map_routes.routes + EMPIRE_ROUTES; r++) {
        if (empire_route_is_valid(r)) {
            empire_route_redraw_path(r);
        } else {
            empire_route_reset(r);
        }
    }
}

static void reset_missing_monument_work_spots(void)
{
    int32_t caesareum = 0, lighthouse = 0, library = 0, abu_simbel = 0, royal_tomb = 0;
    for (int32_t id = 1; id < MAX_BUILDINGS; id++) {
        const building *b = building_get(id);
        if (b->state != BUILDING_STATE_IN_USE) {
            continue;
        }
        switch (b->type) {
            case BUILDING_CAESAREUM:
                caesareum = 1;
                break;
            case BUILDING_LIGHTHOUSE:
                lighthouse = 1;
                break;
            case BUILDING_ALEXANDRIA_LIBRARY:
                library = 1;
                break;
            case BUILDING_ABU_SIMBEL:
                abu_simbel = 1;
                break;
            case BUILDING_SMALL_ROYAL_TOMB:
            case BUILDING_MEDIUM_ROYAL_TOMB:
            case BUILDING_LARGE_ROYAL_TOMB:
            case BUILDING_GRAND_ROYAL_TOMB:
                royal_tomb = 1;
                break;
            default:
                break;
        }
    }
    if (!caesareum) {
        caesareum_reset_work_spots();
    }
    if (!lighthouse) {
        lighthouse_reset_work_spots();
    }
    if (!library) {
        library_reset_work_spots();
    }
    if (!abu_simbel) {
        abu_simbel_reset_work_spots();
    }
    if (!royal_tomb) {
        royal_tomb_reset_work_spots();
    }
}

static void reset_allowed_buildings(void)
{
    for (int32_t i = 0; i < ALLOWED_BUILDINGS_RESET; i++) {
        scenario_data.allowed_buildings[i] = (uint16_t) (i < ALLOWED_BUILDINGS_RESET_ON);
    }
}

static void restore_spot_scripts(monument_spot *spots, const monument_spot *kept, int32_t count)
{
    for (int32_t i = 0; i < count; i++) {
        spots[i].script = kept[i].script;
    }
}

void game_file_io_read_saved_game_state(int32_t fd)
{
    monument_spot kept_obelisk_small[OBELISK_SMALL_SPOTS];
    monument_spot kept_obelisk_large[OBELISK_LARGE_SPOTS];
    monument_spot kept_sphinx[SPHINX_SPOTS];
    monument_spot kept_sun_temple[SUN_TEMPLE_SPOTS];
    savegame_file_version = SAVEGAME_VERSION;
    memcpy(kept_obelisk_small, obelisk_small_spot_objects, sizeof(kept_obelisk_small));
    savegame_schema_chunk_count = 0;
    memcpy(kept_obelisk_large, obelisk_large_spot_objects, sizeof(kept_obelisk_large));
    memcpy(kept_sphinx, sphinx_spot_objects, sizeof(kept_sphinx));
    memcpy(kept_sun_temple, sun_temple_spot_objects, sizeof(kept_sun_temple));
    empire_route_free_paths();

    for (int32_t i = 0; i < SAVEGAME_CHUNKS; i++) {
        if ((savegame_file_version < SAVE_VERSION_MONUMENT_SPOT_STATE) && (i == SAVEGAME_CHUNK_MONUMENT_SPOT_STATE)) {
            game_file_reset_mission_records();
            break;
        }
        if (!game_file_io_read_chunk(i, savegame_chunk_table, fd)) {
            return;
        }
    }
    build_empire_route_paths();

    if (savegame_file_version < SAVE_VERSION_ALLOWED_BUILDINGS) {
        reset_allowed_buildings();
    }
    if (savegame_file_version < SAVE_VERSION_PREDATOR_VARIANT) {
        scenario_data.predator_variant = 0;
    }
    if (savegame_file_version < SAVE_VERSION_EMPIRE_OBJECT_STATES) {
        game_file_upgrade_empire_object_states();
    }
    if (savegame_file_version < SAVE_VERSION_FORMATION_FIELD) {
        game_file_upgrade_clear_formation_field();
    }
    if (savegame_file_version < SAVE_VERSION_MESSAGE_FIELDS) {
        game_file_upgrade_clear_message_fields();
    }
    if (savegame_file_version < SAVE_VERSION_PLAYER_BLOCK) {
        game_file_upgrade_copy_player_block();
    }
    if (savegame_file_version < SAVE_VERSION_PLAYER_COUNTS) {
        game_file_upgrade_widen_player_counts();
    }
    if (savegame_file_version < SAVE_VERSION_RECORD_RELAYOUT) {
        game_file_relayout_empire_cities();
        game_file_relayout_players();
        game_file_relayout_buildings();
        game_file_relayout_storages();
    }
    if (savegame_file_version < SAVE_VERSION_FLOODPLAIN_SETTINGS) {
        floodplain_settings_reset(&floods_data.settings);
    }
    if (savegame_file_version < SAVE_VERSION_FLOOD_NEXT_QUALITY) {
        floodplain_settings_reset_next_quality(&floods_data.settings);
    }
    game_file_transfer_records(1);
    if (savegame_file_version < SAVE_VERSION_FIGURE_FIELDS) {
        game_file_upgrade_clear_figure_fields();
    }
    if (savegame_file_version < SAVE_VERSION_SERVICE_VISIT_GRID) {
        memset(map_service_visit_grid, 0, GRID_CELLS * sizeof(uint32_t));
    }
    if (savegame_file_version < SAVE_VERSION_MESSAGE_TEXT_IDS) {
        game_file_upgrade_message_text_ids();
    }
    game_file_assign_figure_name_groups();
    if (savegame_file_version < SAVE_VERSION_REPLAY_FLAG) {
        game_is_replay = 0;
    }
    if (savegame_file_version < SAVE_VERSION_PLAYER_DIFFICULTY) {
        city_player_current()->difficulty = settings_data.difficulty;
    }
    if (savegame_file_version < SAVE_VERSION_MESSAGE_RELAYOUT) {
        game_file_upgrade_obelisk_work_spots();
    }
    if (savegame_file_version < SAVE_VERSION_FIGURE_TARGETS) {
        game_file_upgrade_figure_targets();
    }
    if (savegame_file_version < SAVE_VERSION_BOOKMARKS) {
        map_bookmarks_reset();
    }
    if (savegame_file_version < SAVE_VERSION_EVENT_FIELDS) {
        game_file_upgrade_event_fields();
    }
    if (savegame_file_version < SAVE_VERSION_MONUMENT_FIELDS) {
        game_file_upgrade_monument_fields();
        game_file_upgrade_clear_formation_flags();
    }
    if (savegame_file_version < SAVE_VERSION_LAST_FLOOD_QUALITY) {
        floods_data.settings.quality_last = floods_data.settings.quality;
    }
    if (savegame_file_version < SAVE_VERSION_FORMATION_FIGURES) {
        game_file_upgrade_formation_figures();
    }
    if (savegame_file_version < SAVE_VERSION_STORAGE_FLAGS) {
        game_file_upgrade_building_storage_flags();
    }
    if (savegame_file_version < SAVE_VERSION_PLAYER_FIELDS) {
        game_file_reset_player_fields();
    }
    if (savegame_file_version < SAVE_VERSION_SCENARIO_EVENTS) {
        game_file_upgrade_scenario_events();
    }
    if (savegame_file_version < SAVE_VERSION_EMPIRE_OBJECT_FLAGS) {
        game_file_upgrade_empire_object_flags();
    }
    if (savegame_file_version < SAVE_VERSION_ZOO) {
        game_file_upgrade_clear_building_field();
        game_file_upgrade_clear_player_fields();
        memset(&scenario_data.allowed_buildings[ALLOWED_ZOO], 0, SCENARIO_ALLOWS_CLEARED_SIZE);
    }
    if (savegame_file_version < SAVE_VERSION_PLAYER_COUNTS_2) {
        game_file_upgrade_widen_player_counts2();
    }
    if (savegame_file_version < SAVE_VERSION_COUNTDOWNS) {
        map_sweep_countdown = 0;
        city_frogs_countdown = 0;
        pyramid_speedup_countdown = 0;
        scenario_data.animation_set = MONUMENT_ERA_PYRAMIDS;
    }
    if (savegame_file_version < SAVE_VERSION_SWEEP_TOTAL) {
        map_sweep_total = 0;
    }
    if (savegame_file_version < SAVE_VERSION_SOLDIER_IMAGES) {
        scenario_data.soldier_images_alt = 0;
    }
    if (savegame_file_version < SAVE_VERSION_BUILDING_ORIENTATION) {
        city_locusts_countdown = 0;
        game_file_upgrade_building_orientation();
        mummies_max = 10;
        city_hailstorm_countdown = 0;
    }
    reset_missing_monument_work_spots();

    restore_spot_scripts(obelisk_small_spot_objects, kept_obelisk_small, OBELISK_SMALL_SPOTS);
    restore_spot_scripts(obelisk_large_spot_objects, kept_obelisk_large, OBELISK_LARGE_SPOTS);
    restore_spot_scripts(sphinx_spot_objects, kept_sphinx, SPHINX_SPOTS);
    restore_spot_scripts(sun_temple_spot_objects, kept_sun_temple, SUN_TEMPLE_SPOTS);
    pyramid_speedup_active = pyramid_speedup_countdown > 0;
}

void game_file_io_read_scenario_state(int32_t fd)
{
    chunk_entry *magic = &scenario_chunk_table[0];
    read_raw_or_compressed(magic->compressed, fd, magic->data, magic->element_size * magic->count);
    if (platform_text_compare_nocase(scenario_file_magic, scenario_file_magic_expected) != 0) {
        show_old_file_error();
        return;
    }
    savegame_file_version = SAVEGAME_VERSION;
    savegame_schema_chunk_count = 0;
    empire_route_free_paths();
    for (int32_t i = 1; i < SCENARIO_CHUNKS; i++) {
        if (!game_file_io_read_chunk(i, scenario_chunk_table, fd)) {
            return;
        }
    }
    build_empire_route_paths();

    int32_t version = savegame_file_version;
    if (version < SAVE_VERSION_ALLOWED_BUILDINGS) {
        reset_allowed_buildings();
    }
    if (version < SAVE_VERSION_PREDATOR_VARIANT) {
        scenario_data.predator_variant = 0;
    }
    if (version < SAVE_VERSION_EMPIRE_OBJECT_STATES) {
        game_file_upgrade_empire_object_states();
        version = savegame_file_version;
    }
    if (version < SAVE_VERSION_FLOODPLAIN_SETTINGS) {
        floodplain_settings_reset(&floods_data.settings);
        version = savegame_file_version;
    }
    if (version < SAVE_VERSION_FIGURE_FIELDS) {
        game_file_upgrade_clear_figure_fields();
        version = savegame_file_version;
    }
    if (version < SAVE_VERSION_EVENT_FIELDS) {
        game_file_upgrade_event_fields();
        version = savegame_file_version;
    }
    if (version < SAVE_VERSION_SCENARIO_EVENTS) {
        game_file_upgrade_scenario_events();
        version = savegame_file_version;
    }
    if (version < SAVE_VERSION_EMPIRE_OBJECT_FLAGS) {
        game_file_upgrade_empire_object_flags();
        version = savegame_file_version;
    }
    if (version < SAVE_VERSION_ZOO) {
        game_file_upgrade_clear_building_field();
        game_file_upgrade_clear_player_fields();
        memset(&scenario_data.allowed_buildings[ALLOWED_ZOO], 0, SCENARIO_ALLOWS_CLEARED_SIZE);
        version = savegame_file_version;
    }
    if (version < SAVE_VERSION_PLAYER_COUNTS_2) {
        game_file_upgrade_widen_player_counts2();
        version = savegame_file_version;
    }
    if (version < SAVE_VERSION_COUNTDOWNS) {
        map_sweep_countdown = 0;
        city_frogs_countdown = 0;
        pyramid_speedup_countdown = 0;
        scenario_data.animation_set = MONUMENT_ERA_PYRAMIDS;
    }
    if (version < SAVE_VERSION_SWEEP_TOTAL) {
        map_sweep_total = 0;
        scenario_data.soldier_images_alt = 0;
        mummies_max = 10;
        city_hailstorm_countdown = 0;
    }
}

static void fill_schema(const chunk_entry *table, int32_t count)
{
    memset(savegame_chunk_schema, 0, sizeof(savegame_chunk_schema));
    for (int32_t i = 0; i < count; i++) {
        chunk_schema_record *s = &savegame_chunk_schema[i];
        s->compressed = table[i].compressed;
        s->element_size = table[i].element_size;
        s->count = table[i].count;
    }
    savegame_schema_chunk_count = count;
}

static void write_chunks(int32_t fd, const chunk_entry *table, int32_t count)
{
    fill_schema(table, count);
    for (int32_t i = 0; i < count; i++) {
        const chunk_entry *c = &table[i];
        if (c->compressed) {
            io_write_compressed_chunk(fd, c->data, c->element_size * c->count);
        } else {
            platform_file_write(fd, c->data, (uint32_t) (c->element_size * c->count));
        }
    }
}

int32_t game_file_io_write_saved_game(const char *path)
{
    game_file_transfer_records(0);
    strcpy(scenario_file_magic, scenario_file_magic_expected);
    savegame_file_version = SAVEGAME_VERSION;
    strcpy(city_player_names[1].name, settings_data.player_name);
    map_terrain_shrub_to_tree();
    free_empire_route_paths();
    int32_t fd = platform_file_open(path, PLATFORM_OPEN_REPLACE);
    if (fd == -1) {
        return 1;
    }
    write_chunks(fd, savegame_chunk_table, SAVEGAME_CHUNKS);
    platform_file_close(fd);
    city_warning_show(WARNING_GAME_SAVED);
    rebuild_empire_route_paths_after_save();
    return 0;
}

int32_t game_file_io_delete_saved_game(const char *path)
{
    if (!platform_file_remove(path)) {
        return 1;
    }
    dir_list_files(0);
    return 0;
}

static int32_t load_saved_game(const char *path, int32_t keep_difficulty)
{
    int32_t difficulty = settings_data.difficulty;
    floodplain_settings_reset(&floods_data.settings);
    map_grids_clear_all();
    game_loaded_flag = 0;
    int32_t fd = platform_file_open(path, PLATFORM_OPEN_READ);
    if (fd == -1) {
        return 1;
    }
    sound_lib_stop_stream(&sound_manager, SOUND_STREAM_CITY);
    sound_music_playing_special = 0;
    game_file_io_read_saved_game_state(fd);
    platform_file_close(fd);
    if (keep_difficulty) {
        settings_data.difficulty = difficulty;
        city_player *player = city_player_current();
        int32_t treasury = calc_adjust_with_percentage(player->treasury, difficulty_treasury_percentage[difficulty]);
        game_loaded_flag = 1;
        player->treasury = treasury;
    } else {
        game_loaded_flag = 1;
    }
    game_file_initialize_saved_game();
    building_storage_reset_building_ids();
    building_ferry_records_update();
    building_repair_slots_update();
    strcpy(city_player_names[1].name, settings_data.player_name);
    discord_presence_update();
    return 0;
}

int32_t game_file_load_saved_game(const char *path)
{
    return load_saved_game(path, 0);
}

int32_t game_file_load_saved_game_keep_difficulty(const char *path)
{
    return load_saved_game(path, 1);
}

void game_file_initialize_saved_game(void)
{
    memset(map_grids.images, 0, GRID_CELLS * sizeof(int32_t));
    scenario_climate_current = scenario_data.climate;
    map_info.grid_width = scenario_data.map_width;
    map_info.grid_height = scenario_data.map_height;
    map_info.grid_start = scenario_data.grid_start;
    map_info.grid_border_size = scenario_data.grid_border_size;
    map_grid_init();
    building_model_load();
    figure_model_load();
    tax_sentiment_model_load();
    monument_load_animation_scripts();
    monument_load_extra_scripts();
    figure_enemy_properties_init();
    map_terrain_shrub_to_tree();
    for (int32_t i = 0; i < SCENARIO_INVASION_POINTS; i++) {
        for (int32_t check = 0; check < INVASION_POINT_CHECKS; check++) {
            if (!map_grid_is_inside(map_info.diamond_x_min + scenario_data.invasion_point_x[i], map_info.diamond_y_min + scenario_data.invasion_point_y[i])) {
                scenario_data.invasion_point_x[i] = -1;
                scenario_data.invasion_point_y[i] = -1;
            }
        }
    }
    building_count_update();
    city_culture_update_coverage();
    if ((city_view_orientation < 0) || (city_view_orientation > CITY_VIEW_ORIENTATION_MAX)) {
        city_view_orientation = 0;
    }
    map_info.orientation = city_view_orientation;
    obelisk_load_scripts_all_slots();
    game_load_restore_buildings_and_figure_lists();
    if (savegame_file_version < SAVE_VERSION_PYRAMID_REDRAW) {
        map_pyramid_redraw_pending = 1;
        map_pyramid_redraw();
    }
    if (savegame_file_version < SAVE_VERSION_FARM_FIELDS) {
        game_file_upgrade_farm_fields();
    }
    map_tiles_update_all();
    floods_build(&floods_data, 0, 0, map_info.grid_width - 1, map_info.grid_height - 1);
    city_view_calculate_lookup(city_view_orientation);
    city_view_calculate_map_bounds();
    city_view_check_camera_boundaries();
    map_routing_clear_land_citizen();
    map_routing_update_water();
    map_routing_update_land_citizen();
    map_routing_update_land_noncitizen();
    map_routing_update_walls();
    figure_route_clean();
    map_road_network_update();
    building_maintenance_check_kingdom_access();
    building_granaries_calculate_stocks();
    building_menu_update();
    sidebar_build_buttons_update();
    city_message_init_problem_areas();
    sound_city_init();
    undo_available = 0;
    city_overlay = 0;
    city_overlay_previous = 0;
    mission_briefing_set_started();
    city_player *player = city_player_current();
    player->tutorial_flag_seen_g = 1;
    player->tutorial_flag_seen_a = 1;
    map_terrain_mark_gardens();
    map_orientation_update_buildings();
    city_message_scroll_position = 0;
    city_message_list_scroll = 0;
    settings_data.paused = 0;
    figure_count_by_destination();
    figure_count_traders();
    city_health_recompute_distances = 1;
    empire_object_set_images();
    empire_object_set_city_images();
    building_warship_wharf_count = building_count_warship_wharves();
    game_file_update_granary_stock();
    game_file_update_festival_square();
    if (settings_data.is_custom_scenario) {
        scenario_start_flags[SCENARIO_START_FLAG_EMPIRE_LOCKED] = 0;
    }
    savegame_file_version = SAVEGAME_VERSION;
    empire_update_export_keep();
    city_resource_determine_available();
    building_update_highest_id();
    city_rebuild_monument_slots();
    ferry_check_passengers();
}

int32_t game_file_io_write_scenario(const char *path)
{
    scenario_data.native_image_hut = image_group(IMAGE_GROUP_NATIVE_HUT);
    scenario_data.native_image_meeting = image_group(IMAGE_GROUP_NATIVE_HUT) + NATIVE_IMAGE_MEETING_OFFSET;
    scenario_data.native_image_crops = image_group(IMAGE_GROUP_GRAIN_CROPS);
    scenario_data.flotsam_enabled = 0;
    scenario_data.kingdom_supplies_grain = 0;
    strcpy(scenario_file_magic, scenario_file_magic_expected);
    savegame_file_version = SAVEGAME_VERSION;
    int32_t fd = platform_file_open(path, PLATFORM_OPEN_REPLACE);
    if (fd == -1) {
        return 1;
    }
    editor_fix_all();
    map_pyramid_redraw();
    farm_update_map();
    map_terrain_shrub_to_tree();
    free_empire_route_paths();
    write_chunks(fd, scenario_chunk_table, SCENARIO_CHUNKS);
    rebuild_empire_route_paths_after_save();
    platform_file_close(fd);
    return 0;
}

int32_t game_file_load_scenario_data(const char *path, int32_t skip_scripts)
{
    floodplain_settings_reset(&floods_data.settings);
    int32_t fd = platform_file_open(path, PLATFORM_OPEN_READ);
    if (fd == -1) {
        return 1;
    }
    game_file_io_read_scenario_state(fd);
    platform_file_close(fd);
    scenario_climate_current = scenario_data.climate;
    if (!skip_scripts) {
        monument_load_animation_scripts();
        monument_load_extra_scripts();
        figure_enemy_properties_init();
    }
    if (editor_active == 1) {
        scenario_data.win_monument = (int32_t) (float) city_ratings_calculate_monument(1);
    }
    return 0;
}

static const char campaign_paks[5][20] = {
    "mission1.pak",
    "mission2.pak",
    "mission3.pak",
    "mission4.pak",
    "mission5.pak",
};

int32_t game_file_load_campaign_mission(int32_t pak, int32_t skip_initialize)
{
    int32_t difficulty = settings_data.difficulty;
    if (settings_data.mission_id == MISSION_ID_NONE) {
        return 0;
    }
    const char *path = campaign_paks[pak];
    int32_t offset;
    if (!io_read_file_part(path, &offset, sizeof(offset), settings_data.mission_id * (int32_t) sizeof(offset))) {
        return 0;
    }
    if (offset <= 0) {
        return 0;
    }
    int32_t fd = platform_file_open(path, PLATFORM_OPEN_READ);
    if (fd == -1) {
        return 0;
    }
    platform_file_seek(fd, offset, SEEK_SET);
    game_file_io_read_saved_game_state(fd);
    platform_file_close(fd);
    settings_data.difficulty = difficulty;
    city_player_current()->cheats_used = 0;
    if (!skip_initialize) {
        floodplain_settings_init(&floods_data.settings);
        game_file_initialize_saved_game();
    }
    return 1;
}

void game_file_write_mission_saved_game(void)
{
    if (editor_active == 1) {
        return;
    }
    int32_t *state = &city_player_current()->mission_saved_game_state;
    if (((*state == MISSION_SAVE_DUE) && (settings_data.autosave != 0)) || (*state == MISSION_SAVE_WRITTEN)) {
        return;
    }
    const char *name;
    if (game_is_replay == 1) {
        name = (*state != MISSION_SAVE_NONE) ? "autosave_history.sav" : "autosave_replay.sav";
    } else {
        name = "autosave.sav";
    }
    *state = MISSION_SAVE_WRITTEN;
    char path[AUTOSAVE_PATH_SIZE];
    strcpy(path, dir_save_path());
    strcat(path, "\\");
    strcat(path, name);
    platform_dir_create(dir_save_path());
    game_file_io_write_saved_game(path);
    if (game_is_replay == 1) {
        strcpy(campaign_object.campaign.saved_game_path, path);
        player_data_write_file(&campaign_object, player_record_path());
    }
}

void game_file_initialize_scenario(void)
{
    memset(map_grids.images, 0, GRID_CELLS * sizeof(int32_t));
    map_terrain_shrub_to_tree();
    floodplain_settings_init(&floods_data.settings);
    if (savegame_file_version < SAVE_VERSION_SCENARIO_PYRAMID_REDRAW) {
        map_pyramid_redraw_pending = 1;
        map_pyramid_redraw();
    }
    memset(map_grids.vegetation_growth, 0xFF, GRID_CELLS);
    map_vegetation_growth_update();
    floods_build(&floods_data, 0, 0, map_info.grid_width - 1, map_info.grid_height - 1);
    for (int32_t i = 0; i < scenario_events.count; i++) {
        scenario_event_reset(&scenario_events.events[i]);
    }
    for (int32_t i = 0; i < SCENARIO_MAX_EVENTS; i++) {
        scenario_events.events[i].id = (int16_t) i;
    }
    obelisk_load_scripts_all_slots();
    city_player *player = city_player_current();
    player->festival_square_placed = 0;
    player->festival_square_x = -1;
    player->festival_square_y = -1;
    empire_object_set_images();
    empire_object_set_city_images();
    for (int32_t i = 0; i < EMPIRE_OBJECT_COUNT; i++) {
        empire_object *o = &empire_map_objects[i];
        if (o->in_use && o->type == EMPIRE_OBJECT_CITY) {
            o->is_open = 0;
        }
    }
    for (int32_t i = 1; i < EMPIRE_CITY_COUNT; i++) {
        empire_cities[i].is_open = 0;
    }
    savegame_file_version = SAVEGAME_VERSION;
    city_view_calculate_map_bounds();
    building_warship_wharf_count = building_count_warship_wharves();
    game_file_reset_player_fields();
}
