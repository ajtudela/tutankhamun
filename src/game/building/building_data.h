// The tables of building/building_data.c.

#ifndef BUILDING_BUILDING_DATA_H
#define BUILDING_BUILDING_DATA_H

#include "building/building_counter_types.h"
#include "building/building_industry.h"
#include "building/building_storage_types.h"
#include "building/building_types.h"
#include "building/clear_land_types.h"
#include "building/ferry_queue_types.h"
#include "building/footprint_types.h"
#include "building/gatehouse_types.h"
#include "building/house_model_types.h"
#include "building/monument_object_types.h"
#include "building/monument_tables.h"
#include "building/pyramid_level_types.h"
#include "building/pyramid_refresh_types.h"
#include "building/trade_resource_types.h"
#include "core/core_data.h"
#include "editor/editor_new_scenario.h"
#include "figure/figure_properties.h"
#include "graphics/button.h"
#include "map/map_grid_types.h"
#include "map/vegetation_types.h"
#include <stdint.h>

extern uint8_t abu_simbel_stage_table[240];

extern int32_t farm_harvest_months[40];

extern trade_price_walk trade_goods_list;

extern int32_t overlay_submenu_ids[OVERLAY_SUBMENU_IDS];

extern int32_t gatehouse_rubble_offsets[98];

extern footprint_offset_tables tile_grid_offsets_base;

extern int32_t house_image_variants[40];

extern int32_t house_image_group[20];

extern int32_t house_image_offset[20];

extern int32_t building_properties[1185];

extern int32_t venue_direction_offsets[4];

extern uint8_t venue_patterns2[4];

extern uint8_t venue_patterns3[12];

extern uint8_t venue_patterns4[16];

extern uint8_t venue_patterns5[28];

extern int32_t venue_spot_rank3[9];

extern int32_t venue_spot_rank4[35];

extern char fmt_continue_question[40];

extern int32_t house_expand_delta[8];

extern int32_t house_expand_direction[4];

extern int32_t house_tile_offsets[16];

extern global_function house_evolve_callbacks[20];

extern int32_t festival_group_rates[4];

extern int32_t festival_type_lists[48];

extern const char *building_type_debug_names[237];

extern const char str_build_grand_tomb[20];
extern const char str_build_large_tomb[20];
extern const char str_build_med_tomb[16];
extern const char str_build_paint_maker[20];
extern const char str_build_lamp_maker[20];
extern const char str_build_artisans_guild[24];
extern const char str_build_abu_simbel[20];
extern const char str_build_small_tomb[20];
extern const char str_build_lighthouse[20];
extern const char str_build_caesareum[16];
extern const char str_build_zoo[12];
extern const char str_build_alex_library[20];
extern const char str_build_henna_farm[20];
extern const char str_build_map_cliff[16];
extern const char str_build_mausoleum[16];
extern const char str_build_sandstone_quarry[24];
extern const char str_build_valley_temple[20];
extern const char str_build_causeway[16];
extern const char str_build_mortuary_temple[24];
extern const char str_build_copper_mine[20];
extern const char str_build_granite_quarry[24];
extern const char str_build_thingamabob[20];
extern const char str_build_map_disembark_point[28];
extern const char str_build_sun_temple[20];
extern const char str_build_temp_upgrade2[20];
extern const char str_build_temp_upgrade1[20];
extern const char str_build_sphinx[16];
extern const char str_build_festival[16];
extern const char str_build_obelisk5x5[20];
extern const char str_build_obelisk3x3[20];
extern const char str_build_physician[16];
extern const char str_build_chariot_builder[24];
extern const char str_build_brick_maker[20];
extern const char str_build_papyrus_workshop[24];
extern const char str_build_large_stone_gatehouse[28];
extern const char str_build_large_brick_gatehouse[28];
extern const char str_build_large_mud_gatehouse[28];
extern const char str_build_peasant_house[20];
extern const char str_build_map_sand_dune[20];
extern const char str_build_map_marshland[20];
extern const char str_build_figs_farm[16];
extern const char str_build_reed_farm[16];
extern const char str_build_meat_farm[16];
extern const char str_build_warehouse2[20];
extern const char str_build_dock2[12];
extern const char str_build_granary2[16];
extern const char str_build_market2[16];
extern const char str_build_palace3[16];
extern const char str_build_palace2[16];
extern const char str_build_palace1[16];
extern const char str_build_military_academy3[24];
extern const char str_build_military_academy2[24];
extern const char str_build_courthouse[20];
extern const char str_build_pyramid[16];
extern const char str_build_warship_wharf[20];
extern const char str_build_transport_wharf[24];
extern const char str_build_large_well[20];
extern const char str_build_stonemason_lodge[24];
extern const char str_build_bricklayer_lodge[24];
extern const char str_build_carpenter_lodge[24];
extern const char str_build_stone_structures[24];
extern const char str_build_brick_structures[24];
extern const char str_build_mud_structures[24];
extern const char str_build_stone_tower[20];
extern const char str_build_brick_tower[20];
extern const char str_build_stone_gatehouse[24];
extern const char str_build_brick_gatehouse[24];
extern const char str_build_stone_wall[20];
extern const char str_build_brick_wall[20];
extern const char str_build_firehouse[16];
extern const char str_build_map_finishing_rock[28];
extern const char str_build_map_gem_rock[20];
extern const char str_build_map_gold_rock[20];
extern const char str_build_map_ordinary_rock[24];
extern const char str_build_gem_mine[16];
extern const char str_build_gold_mine[16];
extern const char str_build_temp_large9[20];
extern const char str_build_temp_large8[20];
extern const char str_build_temp_large7[20];
extern const char str_build_temp_large6[20];
extern const char str_build_temp_large5[20];
extern const char str_build_temp_small9[20];
extern const char str_build_temp_small8[20];
extern const char str_build_temp_small7[20];
extern const char str_build_temp_small6[20];
extern const char str_build_temp_small5[20];
extern const char str_build_shrine[16];
extern const char str_build_shrine_god9[20];
extern const char str_build_shrine_god8[20];
extern const char str_build_shrine_god7[20];
extern const char str_build_shrine_god6[20];
extern const char str_build_shrine_god5[20];
extern const char str_build_shrine_god4[20];
extern const char str_build_shrine_god3[20];
extern const char str_build_shrine_god2[20];
extern const char str_build_shrine_god1[20];
extern const char str_build_shrine_god0[20];
extern const char str_build_map_prey_point[24];
extern const char str_build_no_access[16];
extern const char str_build_road_system[20];
extern const char str_build_ferry[12];
extern const char str_build_map_irrigation[24];
extern const char str_build_map_floodplain[24];
extern const char str_build_map_killer_point[24];
extern const char str_build_map_fishing_point[24];
extern const char str_build_map_river_out[20];
extern const char str_build_map_river_in[20];
extern const char str_build_map_ramp[16];
extern const char str_build_map_exit_point[24];
extern const char str_build_map_entry_point[24];
extern const char str_build_map_invasion_point[28];
extern const char str_build_map_road[16];
extern const char str_build_map_anti_plateau[24];
extern const char str_build_map_plateau[20];
extern const char str_build_map_meadow[20];
extern const char str_build_map_rocks[16];
extern const char str_build_map_scrub[16];
extern const char str_build_map_earthquake[24];
extern const char str_build_map_water[16];
extern const char str_build_map_trees[16];
extern const char str_build_map_grass[16];
extern const char str_build_hunting_lodge[20];
extern const char str_build_pottery_workshop[24];
extern const char str_build_lux_goods_workshop[28];
extern const char str_build_weapons_workshop[24];
extern const char str_build_linen_workshop[24];
extern const char str_build_brewery[16];
extern const char str_build_clay_pit[16];
extern const char str_build_lumber_mill[20];
extern const char str_build_special_quarry[24];
extern const char str_build_quarry[16];
extern const char str_build_chicpeas_farm[20];
extern const char str_build_pom_farm[16];
extern const char str_build_lettuce_farm[20];
extern const char str_build_wheat_farm[20];
extern const char str_build_flax_farm[16];
extern const char str_build_barley_farm[20];
extern const char str_build_ruin[12];
extern const char str_build_oracle[16];
extern const char str_build_large_temple[20];
extern const char str_build_small_temple[20];
extern const char str_build_barracks[16];
extern const char str_build_military_academy[24];
extern const char str_build_native_feild[20];
extern const char str_build_well[12];
extern const char str_build_beautification[24];
extern const char str_build_waterlift[16];
extern const char str_build_native_cheif[20];
extern const char str_build_native_hut[20];
extern const char str_build_forum2[16];
extern const char str_build_forum1[16];
extern const char str_build_senate2[16];
extern const char str_build_senate1[16];
extern const char str_build_road_bridge[20];
extern const char str_build_foot_bridge[20];
extern const char str_build_eng_post[16];
extern const char str_build_mission[16];
extern const char str_build_gov_mansion3[20];
extern const char str_build_gov_mansion2[20];
extern const char str_build_gov_mansion1[20];
extern const char str_build_fishing_wharf[20];
extern const char str_build_dock[12];
extern const char str_build_shipyard[16];
extern const char str_build_warehouse_space[24];
extern const char str_build_warehouse[16];
extern const char str_build_granary[16];
extern const char str_build_market[16];
extern const char str_build_temp_bast_large[24];
extern const char str_build_temp_seth_large[24];
extern const char str_build_temp_ptah_large[24];
extern const char str_build_temp_ra_large[20];
extern const char str_build_temp_osiris_large[24];
extern const char str_build_temp_bast_small[24];
extern const char str_build_temp_seth_small[24];
extern const char str_build_temp_ptah_small[24];
extern const char str_build_temp_ra_small[20];
extern const char str_build_temp_osiris_small[24];
extern const char str_build_mud_tower[16];
extern const char str_build_mud_gatehouse[20];
extern const char str_build_fort[12];
extern const char str_build_arch[12];
extern const char str_build_prefecture[20];
extern const char str_build_training_ground[24];
extern const char str_build_library[16];
extern const char str_build_water_crossing[24];
extern const char str_build_school[16];
extern const char str_build_dist_centre[20];
extern const char str_build_dentist[16];
extern const char str_build_monuments[16];
extern const char str_build_mortuary[16];
extern const char str_build_apothecary[20];
extern const char str_build_aux_lgt_fort[20];
extern const char str_build_aux_mis_fort[20];
extern const char str_build_statue3[16];
extern const char str_build_statue2[16];
extern const char str_build_statue1[16];
extern const char str_build_leg_fort[16];
extern const char str_build_small_park[20];
extern const char str_build_plaza[12];
extern const char str_build_bull_fighting_train[28];
extern const char str_build_juggling_train[24];
extern const char str_build_dancer_train[20];
extern const char str_build_musician_train[24];
extern const char str_build_dancer[16];
extern const char str_build_senet_house[20];
extern const char str_build_juggler[16];
extern const char str_build_musician[16];
extern const char str_build_house20[16];
extern const char str_build_house19[16];
extern const char str_build_house18[16];
extern const char str_build_house17[16];
extern const char str_build_house16[16];
extern const char str_build_house15[16];
extern const char str_build_house14[16];
extern const char str_build_house13[16];
extern const char str_build_house12[16];
extern const char str_build_house11[16];
extern const char str_build_house10[16];
extern const char str_build_house9[16];
extern const char str_build_house8[16];
extern const char str_build_house7[16];
extern const char str_build_house6[16];
extern const char str_build_house5[16];
extern const char str_build_house4[16];
extern const char str_build_house3[16];
extern const char str_build_house2[16];
extern const char str_build_house1[16];
extern const char str_build_dig[12];
extern const char str_build_irrigation[20];
extern const char str_build_aqua_system[20];
extern const char str_build_mud_wall[16];
extern const char str_build_road[12];
extern const char str_build_construction[20];
extern const char str_build_extractors[20];
extern const char str_build_farms[12];
extern const char str_build_undo[12];
extern const char str_build_nothing[16];

extern int32_t canal_lift_ends[45];

extern int32_t complex_part_groups[9];

extern int32_t gatehouse_orientation_parts[33];

extern gatehouse_part_tables gatehouse_parts;

extern int32_t temple_complex_tiles[455];

extern int32_t temple_complex_origin[48];

extern const char *temple_complex_god_script_names[5];

extern const char str_temple_bast[12];
extern const char str_temple_seth[12];
extern const char str_temple_ptah[12];
extern const char temple_nile_script_name[12];

extern const char str_temple_ra[12];

extern const char *mausoleum_block_scripts[36];

extern const char str_maus2_3d[12];
extern const char str_maus2_3c[12];
extern const char str_maus2_3b[12];
extern const char str_maus2_3a[12];
extern const char str_maus2_2d[12];
extern const char str_maus2_2c[12];
extern const char str_maus2_2b[12];
extern const char str_maus2_2a[12];
extern const char str_maus2_1d[12];
extern const char str_maus2_1c[12];
extern const char str_maus2_1b[12];
extern const char str_maus2_1a[12];
extern const char str_maus1_3d[12];
extern const char str_maus1_3c[12];
extern const char str_maus1_3b[12];
extern const char str_maus1_3a[12];
extern const char str_maus1_2d[12];
extern const char str_maus1_2c[12];
extern const char str_maus1_2b[12];
extern const char str_maus1_2a[12];
extern const char str_maus1_1d[12];
extern const char str_maus1_1c[12];
extern const char str_maus1_1b[12];
extern const char str_maus1_1a[12];
extern const char str_maus0_3d[12];
extern const char str_maus0_3c[12];
extern const char str_maus0_3b[12];
extern const char str_maus0_3a[12];
extern const char str_maus0_2d[12];
extern const char str_maus0_2c[12];
extern const char str_maus0_2b[12];
extern const char str_maus0_2a[12];
extern const char str_maus0_1d[12];
extern const char str_maus0_1c[12];
extern const char str_maus0_1b[12];
extern const char str_maus0_1a[12];

extern int32_t city_warning_positions[10];

extern int32_t footprint_index_table[36];

extern const char str_suntemple3[12];
extern const char str_suntemple2[12];
extern const char str_suntemple1[16];

extern char lighthouse_script_names[10][12];

extern pyramid_preset_part_table pyramid_preset_part_data;

extern pyramid_preset_table pyramid_preset_parts;

extern pyramid_preset_list mastaba_preset_parts[MASTABA_SIZE_COUNT];

extern int32_t pyramid_level_neighbour_offsets[PYRAMID_LEVEL_NEIGHBOURS];

extern generic_button ratings_advisor_buttons[RATINGS_ADVISOR_BUTTONS];

extern int32_t routed_building_direction_indices[32];

extern const char str_obelisk_extra[16];

extern const char str_sun_temple_extra[20];

extern const char str_mausoleum1_extra[20];

extern const char str_mausoleum2_extra[20];

extern const char str_mausoleum0_extra[20];

extern char glory_of_ptolemy_name[164];

extern const char *model_names[5];

extern const char *figure_names[5];

extern const char *tax_names[5];

extern const char str_tax_sentiment_model_impossible_txt[36];
extern const char str_tax_sentiment_model_hard_txt[32];
extern const char str_tax_sentiment_model_normal_txt[32];
extern const char str_tax_sentiment_model_easy_txt[32];
extern const char str_tax_sentiment_model_veryeasy_txt[36];
extern const char str_figure_model_impossible_txt[28];
extern const char str_figure_model_hard_txt[24];
extern const char str_figure_model_normal_txt[24];
extern const char str_figure_model_easy_txt[24];
extern const char str_figure_model_veryeasy_txt[28];
extern const char str_pharaoh_model_impossible_txt[32];
extern const char str_pharaoh_model_hard_txt[24];
extern const char str_pharaoh_model_normal_txt[28];
extern const char str_pharaoh_model_easy_txt[24];
extern const char str_pharaoh_model_veryeasy_txt[28];

extern const char str_building_model_loaded[20];

extern const char str_all_houses[12];

extern const char str_building_model_out_of_sync[36];

extern char fmt_building_model_bad_lines[64];

extern const char str_all_buildings[16];

extern const char str_no_building_model_file[32];

extern const char str_no_spare_screen_model[32];

extern const char str_figure_model_loaded[32];

extern const char str_all_enemies[12];

extern const char str_figure_model_out_of_sync[32];

extern char fmt_figure_model_bad_lines[64];

extern const char str_all_figures[12];

extern const char str_no_figure_model_file[32];

extern const char str_tax_sentiment_loaded[32];

extern const char str_tax_sentiment_out_of_sync[40];

extern char fmt_tax_sentiment_bad_lines[72];

extern const char *sun_temple_scripts[4];

extern int32_t sun_temple_block_pixel_offsets[8];

extern int32_t ghost_tomb_footprint_tiles[36];

extern lighthouse_object_t lighthouse_object;

extern abu_simbel_object_t abu_simbel_object;

extern large_royal_tomb_record large_royal_tomb_object;

extern grand_royal_tomb_record grand_royal_tomb_object;

extern medium_royal_tomb_record medium_royal_tomb_object;

extern small_royal_tomb_record small_royal_tomb_object;

extern caesareum_object_t caesareum_object;

extern library_object_t library_object;

extern int32_t library_spot_variant;

extern building_type_counts building_count_active;

extern int32_t building_count_total[237];

extern building_model_record building_model_table[237];

extern enemy_models_table enemy_model;

extern house_model_table house_models;

extern int32_t venue_road_grid[25];

extern int32_t venue_remove_leaves_rubble;

extern int32_t house_evolve_expanded;

extern int32_t building_corrupt_houses_b;

extern int32_t building_corrupt_houses_a;

extern building_counter_block building_counters;

extern int32_t building_count_industry_active[36];

extern int32_t building_count_industry_total[36];

extern int32_t building_storage_highest_id;

extern int32_t building_highest_id_ever;

extern int32_t building_highest_id;

extern int32_t building_extra_sequence;

extern int32_t house_evolve_status;

extern int32_t building_list_small_size;

extern int32_t building_list_small_index;

extern int32_t building_list_burning_index;

extern int32_t building_burning_size;

extern int32_t building_burning_total;

extern int32_t building_list_large_size;

extern int32_t building_list_large_index;

extern int16_t granary_ids[100];

extern int32_t granary_count;

extern int16_t building_list_small[500];

extern int16_t building_list_burning[500];

extern int16_t building_list_large[4000];

extern building_storage building_storages[200];

extern building_table city_buildings;

extern ferry_path ferry_path_records[50];

extern int32_t houses_frozen;

extern int32_t criminal_day_counter;

extern vegetation_growth_record vegetation_growth_state;

extern uint8_t map_edge_backup[51984];

extern uint8_t map_bitfields_backup[51984];

extern uint8_t map_canals_backup[51984];

extern uint32_t map_terrain_backup[51984];

extern map_grid_set map_grids;

extern int32_t undo_cost;

extern int32_t undo_mode;

extern clear_land_rectangle clear_land_rect;

extern clear_land_confirmations clear_land_confirmed;

extern int32_t construction_size;

extern int32_t water_orient_raw;

extern int32_t construction_bridge_length;

extern int32_t bridge_end_grid_offset;

extern int32_t construction_bridge_step;

extern int32_t construction_orientation;

extern int32_t construction_cost_refund;

extern int32_t routed_building_items_placed;

extern int32_t construction_cost;

extern int32_t construction_place_cost;

extern int32_t construction_ferry_first;

extern pharaohmap_info map_info;

extern int32_t construction_ferry_state;

extern uint8_t construction_ferry_site_ok;

extern int32_t construction_canals_changed;

extern uint8_t osiris_double_yield_used;

extern int32_t farm_max_progress;

extern uint8_t farm_harvest_flags[FARM_CROP_ROWS];

extern int32_t building_barracks_tower_sentry_request;

extern int32_t building_warship_wharf_count;

extern int32_t formation_num_legions;

extern int32_t formation_id_last_legion;

extern int32_t formation_id_last_in_use;

extern int32_t storage_staff_missing;

extern uint16_t *screen_rgb565_ptr;

extern int32_t scenario_mausoleum_size_cached;

extern int32_t message_collapse_last_time;

extern int32_t message_fire_last_time;

extern int32_t tax_model[286];

extern int32_t city_current_player;

extern pyramid_complex_refresh_slots pyramid_complex_refresh_queue;

extern int32_t pyramid_sort_origin_x;

extern int32_t pyramid_frontier_parts[144];

extern uint8_t pyramid_complex_found;

extern int32_t pyramid_complex_search_building;

extern int32_t pyramid_complex_build_type;

extern int32_t pyramid_complex_free_tile;

extern int32_t pyramid_complex_area_end;

extern int32_t pyramid_frontier_count;

extern ferry_queue_records ferry_queues;

#endif
