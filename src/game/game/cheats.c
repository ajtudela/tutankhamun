// The cheats: the Alt keys of the city window (screenshots; with cheats enabled the cheat input window and unlocking
// every mission), and the cheats typed into the cheat input window (34 names stored XOR 3): the gods' blessings and
// curses, money, forced victory, disasters, a crime wave, mock invasions and a herd of hippos at the mouse tile.
//
// The hippos' spawn search stops at the first offset whose terrain predators may not use and spawns at the next one.

#include "game/cheats.h"

#include "building/pyramid_level.h"
#include "city/city_disasters.h"
#include "city/city_init.h"
#include "city/city_internal.h"
#include "city/city_player.h"
#include "city/crime.h"
#include "city/gods.h"
#include "city/message.h"
#include "core/random.h"
#include "editor/editor_data.h"
#include "figure/animals.h"
#include "figure/animals_internal.h"
#include "figure/enemy.h"
#include "figure/enemy_ships.h"
#include "figure/figure.h"
#include "figure/figure_action.h"
#include "figure/figure_core.h"
#include "figure/figure_data.h"
#include "figure/figure_type.h"
#include "figure/formation.h"
#include "game/game_data.h"
#include "game/game_internal.h"
#include "game/player_record.h"
#include "graphics/screenshot.h"
#include "map/grid.h"
#include "platform/platform_input.h"
#include "platform/platform_message.h"
#include "scenario/invasion.h"
#include "scenario/scenario_event_types.h"
#include "window/main_menu.h"
#include "window/popup_dialog.h"
#include "window/window_id.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

enum hippo_stomp_rule {
    HIPPO_STOMP_HIPPOS = 8,
    HIPPO_STOMP_HERD_WAIT_TICKS = 24,
    HIPPO_STOMP_HIPPO_WAIT_TICKS = 1000,
    HIPPO_STOMP_FRAME_COUNT = 7,
};

enum mock_attack_rule {
    MOCK_ATTACK_1_SIZE = 15,
    MOCK_ATTACK_1_POINT = 8,
    MOCK_ATTACK_2_SIZE = 16,
    MOCK_ATTACK_2_POINT = 9,
};

enum {
    CHEAT_INPUT_LENGTH = 100,
    CHEAT_NAME_XOR = 3,
    CHEAT_TREASURY_LIMIT = 15000,
    CHEAT_TREASURY_GIFT = 1000,
    CHEAT_FROGS = 6,
    CHEAT_BLOOD_RIVER_AMOUNT = 5,
    CHEAT_MUMMIES = 10,
    CHEAT_CRIME_WAVE_CRIMINALS = 3,
    CHEAT_PYRAMID_SPEEDUP = 2,
    CHEAT_KEY_PRESSED_MASK = 0xFF00,
};

void cheat_life_from_death(void)
{
    city_gods_perform_blessing_or_curse(GOD_OSIRIS, GOD_EVENT_MAJOR_BLESSING);
}
void cheat_bounty(void)
{
    city_gods_perform_blessing_or_curse(GOD_OSIRIS, GOD_EVENT_MINOR_BLESSING);
}
void cheat_mummys_curse(void)
{
    city_gods_perform_blessing_or_curse(GOD_OSIRIS, GOD_EVENT_MAJOR_CURSE);
}
void cheat_underworld(void)
{
    city_gods_perform_blessing_or_curse(GOD_OSIRIS, GOD_EVENT_MINOR_CURSE);
}
void cheat_pharaohs_glory(void)
{
    city_gods_perform_blessing_or_curse(GOD_RA, GOD_EVENT_MAJOR_BLESSING);
}
void cheat_sun_disk(void)
{
    city_gods_perform_blessing_or_curse(GOD_RA, GOD_EVENT_MINOR_BLESSING);
}
void cheat_mesektet(void)
{
    city_gods_perform_blessing_or_curse(GOD_RA, GOD_EVENT_MAJOR_CURSE);
}
void cheat_bird_of_prey(void)
{
    city_gods_perform_blessing_or_curse(GOD_RA, GOD_EVENT_MINOR_CURSE);
}
void cheat_supreme_craftsman(void)
{
    city_gods_perform_blessing_or_curse(GOD_PTAH, GOD_EVENT_MAJOR_BLESSING);
}
void cheat_noble_djed(void)
{
    city_gods_perform_blessing_or_curse(GOD_PTAH, GOD_EVENT_MINOR_BLESSING);
}
void cheat_big_dave(void)
{
    city_gods_perform_blessing_or_curse(GOD_PTAH, GOD_EVENT_MAJOR_CURSE);
}
void cheat_grenow(void)
{
    city_gods_perform_blessing_or_curse(GOD_PTAH, GOD_EVENT_MINOR_CURSE);
}
void cheat_spirit_of_typhon(void)
{
    city_gods_perform_blessing_or_curse(GOD_SETH, GOD_EVENT_MAJOR_BLESSING);
}
void cheat_typhonian_relief(void)
{
    city_gods_perform_blessing_or_curse(GOD_SETH, GOD_EVENT_MINOR_BLESSING);
}
void cheat_fury_of_seth(void)
{
    city_gods_perform_blessing_or_curse(GOD_SETH, GOD_EVENT_MAJOR_CURSE);
}
void cheat_seth_strikes(void)
{
    city_gods_perform_blessing_or_curse(GOD_SETH, GOD_EVENT_MINOR_CURSE);
}
void cheat_cat_nip(void)
{
    city_gods_perform_blessing_or_curse(GOD_BAST, GOD_EVENT_MAJOR_BLESSING);
}
void cheat_meow(void)
{
    city_gods_perform_blessing_or_curse(GOD_BAST, GOD_EVENT_MINOR_BLESSING);
}
void cheat_cat_fight(void)
{
    city_gods_perform_blessing_or_curse(GOD_BAST, GOD_EVENT_MAJOR_CURSE);
}
void cheat_kitty_litter(void)
{
    city_gods_perform_blessing_or_curse(GOD_BAST, GOD_EVENT_MINOR_CURSE);
}

void cheat_crop_busters(void)
{
    city_message_use_popup = 1;
    city_message_post(MESSAGE_LOCUSTS, 0, 0, NULL, 0);
    city_locusts_start();
}

void cheat_amphibious_assault(void)
{
    city_message_use_popup = 1;
    city_message_post(MESSAGE_FROGS, 0, 0, NULL, 0);
    city_frogs_start(CHEAT_FROGS);
}

void cheat_hail_to_the_chief(void)
{
    city_message_use_popup = 1;
    city_message_post(MESSAGE_HAILSTORM, 0, 0, NULL, 0);
    city_hailstorm_start();
}

void cheat_crimson_tide(void)
{
    city_message_use_popup = 1;
    city_message_post(MESSAGE_BLOOD_RIVER, 0, 0, NULL, 0);
    city_blood_river_start(CHEAT_BLOOD_RIVER_AMOUNT);
}

void cheat_ancient_astronauts(void)
{
    if (pyramid_speedup_level()) {
        pyramid_speedup_start(CHEAT_PYRAMID_SPEEDUP);
    }
}

void cheat_jail_break(void)
{
    city_crime_wave(CHEAT_CRIME_WAVE_CRIMINALS);
}

void cheat_mummys_revenge(void)
{
    if (editor_active != 1) {
        city_message_use_popup = 1;
        city_message_post(MESSAGE_MUMMY_ATTACK, 0, 0, NULL, 0);
        city_mummies_start(CHEAT_MUMMIES);
    }
    cheat_mummys_revenge_used = 1;
}

void cheat_side_show(void)
{
    cheat_side_show_enabled = cheat_side_show_enabled == 0;
}

void cheat_pharaohs_tomb(void)
{
    city_victory_forced = 1;
    city_victory_cheated = 1;
}

void cheat_treasure_chest(void)
{
    city_player *player = city_player_current();
    if (player->treasury < CHEAT_TREASURY_LIMIT) {
        player->treasury += CHEAT_TREASURY_GIFT;
        player->cheats_used += CHEAT_TREASURY_GIFT;
    }
}

static int32_t hippo_spawn_index(int32_t index)
{
    for (int32_t tries = 0; tries < HIPPO_STOMP_HIPPOS; tries++) {
        const formation_layout_spot *offset = &herd_spawn_offsets()[index];
        int32_t grid_offset = map_grid_offset(offset->x, offset->y) + map_info.cursor_tile_x + map_info.cursor_tile_y;
        uint8_t allowed = animal_terrain_allowed_climate(map_grids.terrain[grid_offset], 0);
        if (++index >= HIPPO_STOMP_HIPPOS) {
            index = 0;
        }
        if (!allowed) {
            break;
        }
    }
    return index;
}

static void hippo_create(int32_t formation_id, int32_t index_in_formation)
{
    random_generate_next();
    int32_t index = hippo_spawn_index(index_in_formation);
    random_generate_next();
    const formation_layout_spot *offset = &herd_spawn_offsets()[index];
    int32_t figure_id = figure_create(CITY_PLAYER_HUMAN, FIGURE_HIPPO, offset->x + map_info.cursor_tile_x, offset->y + map_info.cursor_tile_y,
        random_generator_state.random1_3bit);
    figure *hippo = &city_figures.records[figure_id];
    hippo->action_state = FIGURE_ACTION_HIPPO_CHARGE;
    hippo->formation_id = (int16_t) formation_id;
    hippo->wait_ticks = HIPPO_STOMP_HIPPO_WAIT_TICKS;
    hippo->index_in_formation = (uint8_t) index_in_formation;
    hippo->animal.frame_count = HIPPO_STOMP_FRAME_COUNT;
    predator_hunt_or_choose_action(figure_id);
}

void cheat_hippo_stomp(void)
{
    if (window_current_id != WINDOW_CITY) {
        return;
    }
    int32_t formation_id = formation_create(FIGURE_HIPPO, FORMATION_TORTOISE, 0, map_info.cursor_tile_x, map_info.cursor_tile_y);
    if (formation_id <= 0) {
        return;
    }
    formation *herd = formation_get(formation_id);
    herd->herd_kind = HERD_KIND_PREDATORS;
    herd->wait.ticks = HIPPO_STOMP_HERD_WAIT_TICKS;
    herd->max_figures = HIPPO_STOMP_HIPPOS;
    for (int32_t i = 0; i < HIPPO_STOMP_HIPPOS; i++) {
        hippo_create(formation_id, i);
    }
}

static void mock_attack(int32_t size, int32_t point)
{
    int32_t result = scenario_invasion_start(scenario_data.enemy_type, size, point, 0, SCENARIO_CHEAT_INVASION_EVENT);
    if (result == 0) {
        return;
    }
    city_message_use_popup = 1;
    city_message_post(scenario_data.enemy_type >= ENEMY_EGYPTIAN ? MESSAGE_ENEMY_ARMY_ATTACKS : MESSAGE_BARBARIANS_ATTACK,
        invasion_last_id, result, NULL, 0);
}

void cheat_mockattack1(void)
{
    mock_attack(MOCK_ATTACK_1_SIZE, MOCK_ATTACK_1_POINT);
}

void cheat_mockattack2(void)
{
    mock_attack(MOCK_ATTACK_2_SIZE, MOCK_ATTACK_2_POINT);
}

int32_t game_cheat_find(const char *text)
{
    char buffer[CHEAT_INPUT_LENGTH];
    strncpy(buffer, text, CHEAT_INPUT_LENGTH);
    buffer[CHEAT_INPUT_LENGTH - 1] = 0;
    for (char *c = buffer; *c; c++) {
        *c ^= CHEAT_NAME_XOR;
    }
    if (!cheat_input_enabled) {
        return -1;
    }
    int32_t index;
    for (index = 0; index < CHEAT_COUNT; index++) {
        const cheat_entry *e = &cheat_table[index];
        if (cheats_disabled && !e->allowed) {
            continue;
        }
        if (strcmp(buffer, e->name) == 0) {
            break;
        }
    }
    return index;
}

bool game_cheat_execute(int32_t index)
{
    if (index < 0 || index >= CHEAT_COUNT) {
        return 0;
    }
    city_player_current()->cheats_used++;
    cheat_table[index].fn();
    return 1;
}

bool game_cheat_apply_input(void)
{
    window_current_id = (uint8_t) cheat_input_previous_window;
    return game_cheat_execute(game_cheat_find(cheat_input_text));
}

bool game_cheat_input_close(void)
{
    if (cheat_input_enabled) {
        city_warning_show(WARNING_CHEATS_DISABLED);
    }
    cheat_input_enabled = 0;
    return game_cheat_apply_input();
}

int32_t game_cheat_handle_key(int32_t message, int32_t key)
{
    int32_t alt = platform_key_state(PLATFORM_KEY_ALT) & CHEAT_KEY_PRESSED_MASK;
    if (window_current_id != WINDOW_CITY) {
        return 0;
    }
    if (message != PLATFORM_MSG_KEY_DOWN) {
        if (message != PLATFORM_MSG_SYS_KEY_DOWN) {
            return 0;
        }
        if (key == '1') {
            graphics_save_screenshot(screenshot_file_name_alt1);
        } else if (key == '2') {
            graphics_save_screenshot(screenshot_file_name_alt2);
        }
    }
    switch (key) {
        case 'C':
        case 'c':
            if (key_ctrl && alt) {
                window_cheat_input_show();
                return 1;
            }
            break;
        case 'F':
        case 'f':
            if (key_ctrl && alt && settings_data.player_name[0] && window_current_id != WINDOW_NAME_INPUT) {
                campaign_mark_all_played(&campaign_object.campaign);
                player_data_write_file(&campaign_object, player_record_path());
                return 1;
            }
            break;
    }
    return 0;
}
