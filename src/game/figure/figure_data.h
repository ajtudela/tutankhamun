// The tables of figure/figure_data.c.

#ifndef FIGURE_FIGURE_DATA_H
#define FIGURE_FIGURE_DATA_H

#include "building/route_candidate_types.h"
#include "core/calc.h"
#include "core/core_data.h"
#include "core/random_types.h"
#include "figure/figure_phrase_types.h"
#include "figure/figure_profile_types.h"
#include "figure/figure_properties.h"
#include "figure/figure_types.h"
#include "figure/formation_layout_types.h"
#include "figure/formation_types.h"
#include <stdint.h>

extern formation_layout_spot enemy_formation_layout_offsets[FORMATION_LAYOUTS][FORMATION_LAYOUT_SPOTS];

static inline const formation_layout_spot *herd_roaming_offsets(void)
{
    return enemy_formation_layout_offsets[FORMATION_LAYOUT_HERD];
}

static inline const formation_layout_spot *herd_spawn_offsets(void)
{
    return enemy_formation_layout_offsets[FORMATION_LAYOUT_HERD_SPAWN];
}

extern uint8_t figure_corpse_offsets[256];

extern uint8_t figure_missile_frames[336];

extern int32_t figure_type_coverage_tag[110];

extern int8_t figure_cart_offsets[16];

extern uint8_t figure_ship_direction_table[64];

extern global_function figure_action_table[110];

extern const char *figure_type_debug_names[110];

extern int32_t military_size_tile_offsets[EXPLOSION_SIZES];

extern const int32_t explosion_size_cloud_offsets[EXPLOSION_SIZES];

extern const int32_t explosion_cloud_drifts[EXPLOSION_CLOUDS][2];

extern const int32_t explosion_cloud_speeds[EXPLOSION_CLOUDS];

extern const formation_layout_spot enemy_formation_spots[ENEMY_FORMATION_LAYOUTS][ENEMY_FORMATION_SPOT_DIRECTIONS][ENEMY_FORMATION_SPOTS];

extern int16_t enemy_attack_priority[400];

extern int16_t enemy_attack_fallback[100];

extern const char *enemy_graphics_names[12];

extern const char str_enemy_persian[8];
extern const char str_enemy_phoenician[12];
extern const char str_enemy_roman[8];
extern const char str_enemy_assyrian[12];
extern const char str_enemy_canaanite[12];
extern const char str_enemy_nubian[8];
extern const char str_enemy_libian[8];
extern const char str_enemy_kushite[8];
extern const char str_enemy_mitani[8];
extern const char str_enemy_hittite[8];
extern const char str_enemy_seapeople[12];
extern const char str_enemy_hyksos[8];
extern const char str_fig_mummy[12];
extern const char str_fig_artisan[12];
extern const char str_fig_locust[12];
extern const char str_fig_frog[12];
extern const char str_fig_zookeeper[16];
extern const char str_fig_scorpion[16];
extern const char str_fig_lion[12];
extern const char str_fig_asp[8];
extern const char str_fig_egypt_transport[20];
extern const char str_fig_egypt_warship[20];
extern const char str_fig_bedouin[12];
extern const char str_fig_plague_dude[16];
extern const char str_fig_showman[12];
extern const char str_fig_stone_puller[20];
extern const char str_fig_jumping_fish[20];
extern const char str_fig_funeral_walker[20];
extern const char str_fig_enemy_warship[20];
extern const char str_fig_enemy_transport[20];
extern const char str_fig_festival_priest[20];
extern const char str_fig_reed_cutter[16];
extern const char str_fig_magistrate[16];
extern const char str_fig_policeman[16];
extern const char str_fig_water_carrier[20];
extern const char str_fig_pulled_stone[20];
extern const char str_fig_laborer[12];
extern const char str_fig_hippo[12];
extern const char str_fig_hyena[12];
extern const char str_fig_croc[12];
extern const char str_fig_stone_mason[16];
extern const char str_fig_brick_layer[16];
extern const char str_fig_carpenter[16];
extern const char str_fig_warship[12];
extern const char str_fig_transport[16];
extern const char str_fig_ferry_boat[16];
extern const char str_fig_lumberjack[16];
extern const char str_fig_hunter_spear[20];
extern const char str_fig_hunter[12];
extern const char str_fig_chariot_racers[20];
extern const char str_fig_enemy_missile[20];
extern const char str_fig_antelope[16];
extern const char str_fig_ostrich[12];
extern const char str_fig_birds[12];
extern const char str_fig_shipwreck[16];
extern const char str_fig_granary_boy[16];
extern const char str_fig_seagulls[16];
extern const char str_fig_missionary[16];
extern const char str_fig_creature[16];
extern const char str_fig_ballista[16];
extern const char str_fig_bolt[12];
extern const char str_fig_player_missile[20];
extern const char str_fig_arrow[12];
extern const char str_fig_barb_donkey[16];
extern const char str_fig_enemy_leg_heavy[20];
extern const char str_fig_egypt_heavy[16];
extern const char str_fig_egypt_miss[16];
extern const char str_fig_egypt_aux[16];
extern const char str_fig_barb_heavy2[16];
extern const char str_fig_barb_horse[16];
extern const char str_fig_barb_miss[16];
extern const char str_fig_barb_heavy[16];
extern const char str_fig_barb_light[16];
extern const char str_fig_barb_chariot[20];
extern const char str_fig_enemy_elephant[20];
extern const char str_fig_enemy_camel[16];
extern const char str_fig_enemy_heavy[16];
extern const char str_fig_enemy_aux[16];
extern const char str_fig_enemy_miss[16];
extern const char str_fig_wallguy[12];
extern const char str_fig_barbarian[16];
extern const char str_fig_patrician[16];
extern const char str_fig_buyer[12];
extern const char str_fig_docker[12];
extern const char str_fig_flotsam[12];
extern const char str_fig_map_flag[16];
extern const char str_fig_worker[12];
extern const char str_fig_embalmer[16];
extern const char str_fig_apothecary[16];
extern const char str_fig_physician[16];
extern const char str_fig_dentist[12];
extern const char str_fig_librarian[16];
extern const char str_fig_teacher[12];
extern const char str_fig_bubbles[12];
extern const char str_fig_priest[12];
extern const char str_fig_market_girl[16];
extern const char str_fig_fishing_boat[20];
extern const char str_fig_tomb_robber[16];
extern const char str_fig_thief[12];
extern const char str_fig_simple_crime[20];
extern const char str_fig_van_follower[20];
extern const char str_fig_junk[12];
extern const char str_fig_caravan[12];
extern const char str_fig_bullfighter[16];
extern const char str_fig_dancer[12];
extern const char str_fig_musician[16];
extern const char str_fig_juggler[12];
extern const char str_fig_standard[16];
extern const char str_fig_leg_heavy[16];
extern const char str_fig_leg_aux[12];
extern const char str_fig_leg_miss[16];
extern const char str_fig_fireman[12];
extern const char str_fig_warehouseman[20];
extern const char str_fig_engineer[16];
extern const char str_fig_taxman[12];
extern const char str_fig_explosion[16];
extern const char str_fig_ghost[12];
extern const char str_fig_cart[12];
extern const char str_fig_vagrant[12];
extern const char str_fig_emmigrant[16];
extern const char str_fig_immigrant[16];
extern const char str_fig_null[12];

extern const char str_horn1_wav[16];

extern const char str_horn2_wav[16];

extern const char str_drums_wav[132];

extern int32_t animal_threat_types[8];

extern int32_t animal_prey_types[7];

extern int32_t hippo_water_grid_offsets[9];

extern int32_t hippo_actions[4];

extern int32_t crocodile_water_grid_offsets[8];

extern int32_t crocodile_actions[2];

extern uint8_t cheat_side_show_enabled;

extern figure_properties_table figure_properties;

extern route_candidate_lists route_candidates;

extern figure_profile_clock figure_profile_frequency;

extern figure_profile_table figure_action_profile;

extern uint8_t formation_globals_block[16];

extern int32_t formation_move_result_y;

extern int32_t formation_move_result_x;

extern int32_t figure_roaming_enabled;

extern int32_t figure_search_best_distance;

extern int32_t figure_highest_id;

extern int32_t figure_unused_counter;

extern figure_table city_figures;

extern int16_t figure_route_figure_ids[1000];

extern enemy_army_totals_record enemy_army_totals;

extern int32_t figure_count_processed;

extern int32_t figure_current_id;

extern int32_t figure_created_sequence;

extern uint8_t figure_profile_qpc_available;

extern random_data random_generator_state;

extern int32_t map_result_y;

extern int32_t map_result_x;

extern int32_t figure_image_direction;

extern int32_t figure_phrase_id;

extern phrase_weight_record phrase_weight_block;

#endif
