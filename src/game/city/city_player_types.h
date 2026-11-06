// The city record of a player in its saved layout, and city_players, the two records the game keeps (player 0 and the
// human player). Bytes no code reads are reserved_<n>. The figure counters (rioters, animals, enemies, soldiers and
// the rest) are recounted every tick. city_player_faction: per player a faction byte and the started flag.

#ifndef CITY_CITY_PLAYER_TYPES_H
#define CITY_CITY_PLAYER_TYPES_H

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    char name[32];
} city_player_name;

typedef struct {
    int32_t workers_needed;
    int32_t workers_allocated;
    int32_t total_houses_covered;
    int32_t buildings;
    int32_t priority;
} labor_category;

static_assert(sizeof(labor_category) == 0x14, "labor_category is a savegame record of 20 bytes");

typedef struct {
    int32_t id;
    int32_t size;
} road_network_entry;

enum {
    PLAYER_LARGEST_ROAD_NETWORKS = 10,
};

typedef struct {
    uint8_t reserved_1[2];
    uint8_t in_use;
    uint8_t enabled;
    uint8_t reserved_2;
    uint8_t building_reserved_id;
    uint8_t reserved_3[2];
    int32_t tax_percentage;
    int32_t treasury;
    int32_t sentiment;
    int32_t health_target;
    int32_t health;
    int32_t mortuary_workers;
    int32_t food_consumption_state;
    int32_t population;
    int32_t population_last_year;
    int32_t school_age;
    int32_t academy_age;
    int32_t total_capacity;
    int32_t room_in_houses;
    int32_t monthly_population[2400];
    int32_t monthly_population_next_index;
    int32_t monthly_population_count;
    int16_t population_at_age[100];
    int32_t population_per_level[20];
    int32_t yearly_births;
    int32_t yearly_deaths;
    int32_t lost_removal;
    int32_t immigration_amount_per_batch;
    int32_t emigration_amount_per_batch;
    int32_t emigration_queue_size;
    int32_t immigration_queue_size;
    int32_t lost_homeless;
    int32_t last_change;
    int32_t average_per_year;
    int32_t total_all_years;
    int32_t people_in_huts_and_shanties;
    int32_t people_in_manors_and_up;
    int32_t total_years;
    int32_t yearly_update_requested;
    int32_t last_used_house_add;
    int16_t last_used_house_remove;
    uint8_t reserved_4[2];
    int32_t immigrated_today;
    int32_t emigrated_today;
    int32_t refused_immigrants_today;
    int32_t migration_percentage;
    uint8_t reserved_5[4];
    int32_t emigration_duration;
    int32_t immigration_duration;
    int32_t migration_newcomers;
    int32_t migration_direction;
    uint8_t reserved_6[14];
    int16_t last_storage_yard_id;
    uint8_t reserved_7[36];
    int16_t entry_point_x;
    int16_t entry_point_y;
    int32_t entry_point_grid_offset;
    int16_t exit_point_x;
    int16_t exit_point_y;
    int32_t exit_point_grid_offset;
    int16_t palace_x;
    int16_t palace_y;
    int32_t palace_grid_offset;
    int32_t palace_building_id;
    uint8_t reserved_8[146];
    int16_t trade_status[36];
    int16_t trading_amount[36];
    int16_t mothballed[36];
    uint8_t reserved_9[110];
    int32_t food_stored_total;
    int32_t food_types_available;
    int32_t food_types_eaten;
    uint8_t reserved_10[4];
    int32_t request_resource;
    int32_t request_available;
    int32_t request_amount;
    int32_t sphinx_completed;
    uint8_t food_known[9];
    uint8_t resource_known[36];
    uint8_t reserved_11[207];
    int32_t stockpiled[36];
    int32_t food_supply_months;
    int32_t granaries_operating;
    int32_t percentage_plebs;
    int32_t working_age;
    int32_t workers_available;
    labor_category labor_categories[10];
    int32_t workers_employed;
    int32_t workers_unemployed;
    int32_t unemployment_percentage;
    int32_t unemployment_percentage_for_senate;
    int32_t workers_needed;
    int32_t wages;
    int32_t wages_kingdom;
    uint8_t reserved_12[4];
    int32_t wages_so_far;
    int32_t wages_this_year;
    int32_t wages_last_year;
    int32_t taxed_plebs;
    int32_t taxed_nobles;
    int32_t untaxed_plebs;
    int32_t untaxed_nobles;
    int32_t percentage_taxed_plebs;
    int32_t percentage_taxed_nobles;
    int32_t percentage_taxed_people;
    int32_t yearly_collected_plebs;
    int32_t yearly_collected_nobles;
    int32_t yearly_uncollected_plebs;
    int32_t yearly_uncollected_nobles;
    int32_t taxes_this_year;
    int32_t taxes_last_year;
    int32_t monthly_collected_plebs;
    int32_t monthly_uncollected_plebs;
    int32_t monthly_collected_nobles;
    int32_t monthly_uncollected_nobles;
    int32_t exports_this_year;
    int32_t exports_last_year;
    int32_t imports_this_year;
    int32_t imports_last_year;
    int32_t interest_so_far;
    int32_t interest_last_year;
    int32_t interest_this_year;
    int32_t stolen_last_year;
    int32_t stolen_this_year;
    int32_t construction_last_year;
    int32_t construction_this_year;
    int32_t salary_last_year;
    int32_t salary_this_year;
    int32_t salary_amount;
    int32_t salary_rank;
    int32_t salary_so_far;
    int32_t income_last_year;
    int32_t income_this_year;
    int32_t expenses_last_year;
    int32_t expenses_this_year;
    int32_t net_in_out_last_year;
    int32_t net_in_out_this_year;
    int32_t balance_last_year;
    int32_t balance_this_year;
    uint8_t reserved_13[4000];
    int32_t workshop_stored[40];
    int32_t workshop_space[40];
    uint8_t reserved_14[1280];
    int32_t houses_evolve_demand_1;
    uint8_t reserved_15[4];
    int32_t houses_evolve_demand_2;
    uint8_t reserved_16[4];
    int32_t houses_evolve_demand_3;
    uint8_t reserved_17[4];
    int32_t houses_evolve_demand_4;
    uint8_t reserved_18[4];
    int32_t caravan_import_resource;
    int32_t caravan_backup_import_resource;
    int32_t rating_culture;
    int32_t rating_prosperity;
    int32_t rating_monument;
    int32_t rating_kingdom;
    int32_t rating_peace;
    int32_t gold_delivered;
    int32_t prosperity_trend;
    uint8_t reserved_19[20];
    int32_t monument_num_criminals;
    int32_t monument_num_rioters;
    int32_t houses_missing_water_supply;
    int32_t houses_missing_well;
    int32_t houses_missing_more_entertainment;
    int32_t houses_missing_more_education;
    int32_t houses_missing_education;
    int32_t houses_requiring_school;
    int32_t houses_requiring_library;
    int32_t houses_requiring_academy;
    int32_t houses_missing_dentist;
    int32_t houses_missing_courthouse;
    int32_t houses_missing_food;
    uint8_t reserved_20[10];
    int16_t sphinx_stage;
    int32_t houses_missing_health;
    int32_t houses_missing_more_health;
    int32_t houses_requiring_dentist;
    int32_t houses_requiring_courthouse;
    int32_t houses_requiring_health;
    int32_t houses_missing_religion;
    int32_t houses_missing_second_religion;
    int32_t houses_missing_third_religion;
    int32_t houses_requiring_religion;
    int32_t juggler_shows;
    int32_t juggler_no_shows_weighted;
    int32_t musician_shows;
    int32_t musician_no_shows_weighted;
    int32_t dancer_shows;
    int32_t dancer_no_shows_weighted;
    int32_t senet_shows;
    int32_t senet_no_shows_weighted;
    int32_t venue_needing_shows;
    int32_t average_entertainment;
    int32_t houses_missing_entertainment;
    int32_t months_since_festival;
    int8_t god_target_mood[5];
    uint8_t reserved_21[5];
    int8_t god_mood[5];
    uint8_t reserved_22[5];
    int8_t god_wrath_bolts[5];
    uint8_t reserved_23[25];
    uint8_t god_extra_state_1[5];
    uint8_t reserved_24[5];
    uint8_t god_extra_state_2[5];
    uint8_t reserved_25[5];
    uint8_t god_extra_state_3[5];
    uint8_t reserved_26[5];
    int32_t god_months_since_festival[5];
    uint8_t reserved_27[20];
    int32_t least_happy_god;
    uint8_t reserved_28[4];
    int32_t no_immigration_cause;
    int32_t protesters;
    int32_t thefts;
    int32_t health_demand;
    int32_t religion_demand;
    int32_t education_demand;
    int32_t entertainment_demand;
    int32_t rioters;
    int32_t selected_rating;
    int32_t culture_explanation;
    int32_t prosperity_explanation;
    int32_t monument_explanation;
    int32_t kingdom_explanation;
    int32_t peace_explanation;
    uint8_t reserved_29[4];
    int32_t player_rank;
    int32_t personal_savings;
    int32_t donated_last_year;
    int32_t donated_this_year;
    int32_t donate_amount;
    int16_t working_dock_ids[10];
    uint8_t reserved_30[4];
    int16_t has_temple_complex;
    int16_t animals;
    int16_t num_sea_routes;
    int16_t num_land_routes;
    int16_t sea_trade_problem_duration;
    int16_t land_trade_problem_duration;
    uint8_t reserved_31[2];
    int16_t palace_placed;
    int16_t working_wharfs;
    int16_t temple_complex_upgrades;
    int16_t palace_robberies;
    int16_t palace_robberies_last_year;
    int32_t docker_import_resource;
    int32_t docker_export_resource;
    int32_t bailout_taken;
    int32_t bailout_refused_message_shown;
    int32_t cheats_used;
    int16_t recruiter_x;
    int16_t recruiter_y;
    int32_t recruiter_grid_offset;
    int32_t recruiter_building_id;
    int32_t recruiter_placed;
    uint8_t reserved_32[20];
    int32_t zoo_shows;
    int32_t zoo_no_shows_weighted;
    int32_t mission_has_won;
    int32_t mission_continue_months_left;
    int32_t mission_continue_months_chosen;
    int32_t wage_rate_paid_this_year;
    int32_t tribute_estimate;
    int32_t tribute_paid;
    int32_t tribute_not_paid_last_year;
    int32_t tribute_not_paid_total_years;
    int32_t festival_selected_god;
    int32_t festival_selected_size;
    int32_t festival_planned_size;
    int32_t festival_planned_months_to_go;
    int32_t festival_planned_god;
    int32_t festival_cost[3];
    int32_t festival_grand_beer;
    int32_t festival_not_enough_beer;
    int32_t average_religion;
    int32_t average_education;
    int32_t average_health;
    uint8_t reserved_33[4];
    int32_t first_festival_effect_months;
    int32_t second_festival_effect_months;
    uint8_t reserved_34[4];
    int32_t sentiment_unemployment;
    int32_t sentiment_previous_value;
    uint8_t reserved_35[4];
    int32_t sentiment_low_mood_cause;
    int32_t security_breach_duration;
    int32_t rating_yearly_counters[4];
    int32_t selected_gift_size;
    int32_t months_since_gift;
    int32_t gift_overdose_penalty;
    uint8_t reserved_36[4];
    int32_t gift_id[3];
    int32_t gift_cost[3];
    int32_t kingdom_salary_penalty;
    int32_t kingdom_milestone_penalty;
    int32_t kingdom_ignored_request_penalty;
    int32_t kingdom_last_year;
    int32_t kingdom_change;
    int32_t natives_hostile;
    uint8_t reserved_37[4];
    int32_t mission_post_operational;
    int32_t main_native_meeting_x;
    int32_t main_native_meeting_y;
    int32_t wage_rate_paid_last_year;
    int32_t food_needed_per_month;
    int32_t granaries_understaffed;
    int32_t granaries_not_operating;
    int32_t granaries_not_operating_with_food;
    uint8_t reserved_38[16];
    int32_t ra_exports_bonus_months;
    int32_t enemies_to_leave;
    uint8_t reserved_39[4];
    int32_t gods_angry_message_delay;
    int32_t food_consumed_last_month;
    int32_t food_produced_last_month;
    int32_t food_produced_this_month;
    uint8_t reserved_40[4];
    int32_t estimated_tax_income;
    uint8_t reserved_41[4];
    int16_t distribution_center_x;
    int16_t distribution_center_y;
    int32_t distribution_center_grid_offset;
    int32_t distribution_center_building_id;
    int32_t distribution_center_placed;
    uint8_t reserved_42[44];
    int32_t shipyard_boats_requested;
    int32_t enemies;
    int32_t sentiment_wages;
    int32_t people_in_huts;
    int32_t people_in_residences_and_up;
    int32_t kingdom_soldiers;
    uint8_t reserved_43[16];
    int32_t destroyed_buildings;
    uint8_t reserved_44[8];
    uint8_t total_transport_ships;
    uint8_t empire_service_legions_fighting;
    uint8_t empire_service_legions_not_fighting;
    uint8_t reserved_45;
    uint8_t total_warships;
    uint8_t empire_service_warships;
    uint8_t legions_fighting;
    uint8_t legions_not_fighting;
    uint8_t total_legions;
    uint8_t empire_service_legions;
    uint8_t reserved_46;
    uint8_t total_soldiers;
    uint8_t reserved_47[4];
    int32_t trade_center_building_id;
    int32_t soldiers;
    uint8_t reserved_48[3];
    int8_t sound_march_enemy;
    int8_t sound_march_horse;
    uint8_t reserved_49[10];
    uint8_t sentiment_food_toggle;
    uint8_t reserved_50[8];
    int32_t kingdom_soldiers_removed;
    int32_t chariot_legions;
    int32_t population_highest_ever;
    int32_t estimated_wages;
    int32_t luxury_goods_kinds;
    int32_t prosperity_max;
    road_network_entry largest_road_networks[PLAYER_LARGEST_ROAD_NETWORKS];
    int32_t houses_missing_second_luxury;
    int32_t flotsam_special;
    int32_t senet_house_has_plays;
    uint8_t reserved_51[12];
    int32_t fired_message_shown;
    int32_t victory_message_shown;
    int32_t mission_saved_game_state;
    int32_t tutorial_flag_seen_a;
    int32_t tutorial_flag_seen_g;
    int32_t attacking_natives;
    int32_t temple_complex_building_id;
    int32_t monument_def_id[3];
    int32_t monument_building_id[3];
    int32_t houses_requiring_more_courthouse;
    int32_t houses_missing_any_courthouse;
    int32_t plague_houses;
    int32_t requests_and_festivals_last_year;
    int32_t requests_and_festivals_this_year;
    int32_t migration_newcomers_last_month;
    int32_t crimes_this_month;
    int32_t gold_stolen_this_month;
    int32_t advisor_comment;
    int32_t hunting_lodges_working;
    int32_t game_meat_last_month;
    int32_t game_meat_this_month;
    int32_t fishing_wharfs_working;
    int32_t fish_last_month;
    int32_t fish_this_month;
    int32_t savings_lost;
    uint8_t reserved_52[24];
    int32_t festival_square_x;
    int32_t festival_square_y;
    int32_t difficulty;
    int32_t gold_last_year;
    int32_t gold_this_year;
    int32_t ra_no_traders_months;
    uint8_t reserved_53[92];
    int16_t festival_square_placed;
    int8_t god_happy_bolts[5];
    uint8_t reserved_54[5];
    int32_t monument_completion_flag;
    uint8_t reserved_55[4];
    int32_t monument_completion_months;
    int32_t months_in_debt;
    int32_t years_in_debt;
    int32_t resource_list_scroll;
    int32_t resource_settings_step;
    int32_t granary_food_stored[9];
    int32_t storage_space[36];
    int32_t storage_stored[36];
    int32_t plague_count;
    int16_t ra_trade_bonus_months;
    int16_t ra_trade_penalty_months;
    int16_t ra_trade_minor_penalty_months;
    uint8_t reserved_56[2];
    int32_t seth_minor_blessing_months;
    int32_t bast_culture_decay_skip;
    int32_t osiris_double_farm_yield;
    int32_t osiris_flood_destroys_farms;
    int32_t mansion_placed;
    int32_t enemy_warships;
    uint8_t resource_gift_received[36];
    uint8_t reserved_57[4];
    uint8_t trade_prices_mode;
    uint8_t reserved_58[3];
    int32_t military_academy_placed;
    int32_t sphinx_stage_before_delete;
} city_player;

static_assert(sizeof(city_player) == 0x49D8, "city_player is a savegame record of 18904 bytes");
static_assert(offsetof(city_player, in_use) == 0x2 && offsetof(city_player, enabled) == 0x3 && offsetof(city_player, building_reserved_id) == 0x5 &&
                  offsetof(city_player, tax_percentage) == 0x8 && offsetof(city_player, treasury) == 0xC && offsetof(city_player, sentiment) == 0x10 &&
                  offsetof(city_player, health_target) == 0x14 && offsetof(city_player, health) == 0x18 && offsetof(city_player, mortuary_workers) == 0x1C &&
                  offsetof(city_player, food_consumption_state) == 0x20 && offsetof(city_player, population) == 0x24 &&
                  offsetof(city_player, population_last_year) == 0x28 && offsetof(city_player, school_age) == 0x2C &&
                  offsetof(city_player, academy_age) == 0x30 && offsetof(city_player, total_capacity) == 0x34 &&
                  offsetof(city_player, room_in_houses) == 0x38 && offsetof(city_player, monthly_population) == 0x3C &&
                  offsetof(city_player, monthly_population_next_index) == 0x25BC && offsetof(city_player, monthly_population_count) == 0x25C0 &&
                  offsetof(city_player, population_at_age) == 0x25C4 && offsetof(city_player, population_per_level) == 0x268C &&
                  offsetof(city_player, yearly_births) == 0x26DC && offsetof(city_player, yearly_deaths) == 0x26E0 &&
                  offsetof(city_player, lost_removal) == 0x26E4 && offsetof(city_player, immigration_amount_per_batch) == 0x26E8 &&
                  offsetof(city_player, emigration_amount_per_batch) == 0x26EC && offsetof(city_player, emigration_queue_size) == 0x26F0 &&
                  offsetof(city_player, immigration_queue_size) == 0x26F4 && offsetof(city_player, lost_homeless) == 0x26F8 &&
                  offsetof(city_player, last_change) == 0x26FC && offsetof(city_player, average_per_year) == 0x2700 &&
                  offsetof(city_player, total_all_years) == 0x2704 && offsetof(city_player, people_in_huts_and_shanties) == 0x2708 &&
                  offsetof(city_player, people_in_manors_and_up) == 0x270C && offsetof(city_player, total_years) == 0x2710 &&
                  offsetof(city_player, yearly_update_requested) == 0x2714 && offsetof(city_player, last_used_house_add) == 0x2718 &&
                  offsetof(city_player, last_used_house_remove) == 0x271C && offsetof(city_player, immigrated_today) == 0x2720 &&
                  offsetof(city_player, emigrated_today) == 0x2724 && offsetof(city_player, refused_immigrants_today) == 0x2728 &&
                  offsetof(city_player, migration_percentage) == 0x272C && offsetof(city_player, emigration_duration) == 0x2734 &&
                  offsetof(city_player, immigration_duration) == 0x2738 && offsetof(city_player, migration_newcomers) == 0x273C &&
                  offsetof(city_player, migration_direction) == 0x2740 && offsetof(city_player, last_storage_yard_id) == 0x2752 &&
                  offsetof(city_player, entry_point_x) == 0x2778 && offsetof(city_player, entry_point_y) == 0x277A &&
                  offsetof(city_player, entry_point_grid_offset) == 0x277C && offsetof(city_player, exit_point_x) == 0x2780 &&
                  offsetof(city_player, exit_point_y) == 0x2782 && offsetof(city_player, exit_point_grid_offset) == 0x2784 &&
                  offsetof(city_player, palace_x) == 0x2788 && offsetof(city_player, palace_y) == 0x278A &&
                  offsetof(city_player, palace_grid_offset) == 0x278C && offsetof(city_player, palace_building_id) == 0x2790 &&
                  offsetof(city_player, reserved_8) == 0x2794 && offsetof(city_player, trade_status) == 0x2826 &&
                  offsetof(city_player, trading_amount) == 0x286E && offsetof(city_player, mothballed) == 0x28B6 &&
                  offsetof(city_player, food_stored_total) == 0x296C && offsetof(city_player, food_types_available) == 0x2970 &&
                  offsetof(city_player, food_types_eaten) == 0x2974 && offsetof(city_player, request_resource) == 0x297C &&
                  offsetof(city_player, request_available) == 0x2980 && offsetof(city_player, request_amount) == 0x2984 &&
                  offsetof(city_player, sphinx_completed) == 0x2988 && offsetof(city_player, food_known) == 0x298C &&
                  offsetof(city_player, resource_known) == 0x2995 && offsetof(city_player, stockpiled) == 0x2A88 &&
                  offsetof(city_player, food_supply_months) == 0x2B18 && offsetof(city_player, granaries_operating) == 0x2B1C &&
                  offsetof(city_player, percentage_plebs) == 0x2B20 && offsetof(city_player, working_age) == 0x2B24 &&
                  offsetof(city_player, workers_available) == 0x2B28 && offsetof(city_player, labor_categories) == 0x2B2C &&
                  offsetof(city_player, workers_employed) == 0x2BF4 && offsetof(city_player, workers_unemployed) == 0x2BF8 &&
                  offsetof(city_player, unemployment_percentage) == 0x2BFC && offsetof(city_player, unemployment_percentage_for_senate) == 0x2C00 &&
                  offsetof(city_player, workers_needed) == 0x2C04 && offsetof(city_player, wages) == 0x2C08 && offsetof(city_player, wages_kingdom) == 0x2C0C &&
                  offsetof(city_player, wages_so_far) == 0x2C14 && offsetof(city_player, wages_this_year) == 0x2C18 &&
                  offsetof(city_player, wages_last_year) == 0x2C1C && offsetof(city_player, taxed_plebs) == 0x2C20 &&
                  offsetof(city_player, taxed_nobles) == 0x2C24 && offsetof(city_player, untaxed_plebs) == 0x2C28 &&
                  offsetof(city_player, untaxed_nobles) == 0x2C2C && offsetof(city_player, percentage_taxed_plebs) == 0x2C30 &&
                  offsetof(city_player, percentage_taxed_nobles) == 0x2C34 && offsetof(city_player, percentage_taxed_people) == 0x2C38 &&
                  offsetof(city_player, yearly_collected_plebs) == 0x2C3C && offsetof(city_player, yearly_collected_nobles) == 0x2C40 &&
                  offsetof(city_player, yearly_uncollected_plebs) == 0x2C44 && offsetof(city_player, yearly_uncollected_nobles) == 0x2C48 &&
                  offsetof(city_player, taxes_this_year) == 0x2C4C && offsetof(city_player, taxes_last_year) == 0x2C50 &&
                  offsetof(city_player, monthly_collected_plebs) == 0x2C54 && offsetof(city_player, monthly_uncollected_plebs) == 0x2C58 &&
                  offsetof(city_player, monthly_collected_nobles) == 0x2C5C && offsetof(city_player, monthly_uncollected_nobles) == 0x2C60 &&
                  offsetof(city_player, exports_this_year) == 0x2C64 && offsetof(city_player, exports_last_year) == 0x2C68 &&
                  offsetof(city_player, imports_this_year) == 0x2C6C && offsetof(city_player, imports_last_year) == 0x2C70 &&
                  offsetof(city_player, interest_so_far) == 0x2C74 && offsetof(city_player, interest_last_year) == 0x2C78 &&
                  offsetof(city_player, interest_this_year) == 0x2C7C && offsetof(city_player, stolen_last_year) == 0x2C80 &&
                  offsetof(city_player, stolen_this_year) == 0x2C84 && offsetof(city_player, construction_last_year) == 0x2C88 &&
                  offsetof(city_player, construction_this_year) == 0x2C8C && offsetof(city_player, salary_last_year) == 0x2C90 &&
                  offsetof(city_player, salary_this_year) == 0x2C94 && offsetof(city_player, salary_amount) == 0x2C98 &&
                  offsetof(city_player, salary_rank) == 0x2C9C && offsetof(city_player, salary_so_far) == 0x2CA0 &&
                  offsetof(city_player, income_last_year) == 0x2CA4 && offsetof(city_player, income_this_year) == 0x2CA8 &&
                  offsetof(city_player, expenses_last_year) == 0x2CAC && offsetof(city_player, expenses_this_year) == 0x2CB0 &&
                  offsetof(city_player, net_in_out_last_year) == 0x2CB4 && offsetof(city_player, net_in_out_this_year) == 0x2CB8 &&
                  offsetof(city_player, balance_last_year) == 0x2CBC && offsetof(city_player, balance_this_year) == 0x2CC0 &&
                  offsetof(city_player, workshop_stored) == 0x3C64 && offsetof(city_player, workshop_space) == 0x3D04 &&
                  offsetof(city_player, houses_evolve_demand_1) == 0x42A4 && offsetof(city_player, houses_evolve_demand_2) == 0x42AC &&
                  offsetof(city_player, houses_evolve_demand_3) == 0x42B4 && offsetof(city_player, houses_evolve_demand_4) == 0x42BC &&
                  offsetof(city_player, caravan_import_resource) == 0x42C4 && offsetof(city_player, caravan_backup_import_resource) == 0x42C8 &&
                  offsetof(city_player, rating_culture) == 0x42CC && offsetof(city_player, rating_prosperity) == 0x42D0 &&
                  offsetof(city_player, rating_monument) == 0x42D4 && offsetof(city_player, rating_kingdom) == 0x42D8 &&
                  offsetof(city_player, rating_peace) == 0x42DC && offsetof(city_player, gold_delivered) == 0x42E0 &&
                  offsetof(city_player, prosperity_trend) == 0x42E4 && offsetof(city_player, monument_num_criminals) == 0x42FC &&
                  offsetof(city_player, monument_num_rioters) == 0x4300 && offsetof(city_player, houses_missing_water_supply) == 0x4304 &&
                  offsetof(city_player, houses_missing_well) == 0x4308 && offsetof(city_player, houses_missing_more_entertainment) == 0x430C &&
                  offsetof(city_player, houses_missing_more_education) == 0x4310 && offsetof(city_player, houses_missing_education) == 0x4314 &&
                  offsetof(city_player, houses_requiring_school) == 0x4318 && offsetof(city_player, houses_requiring_library) == 0x431C &&
                  offsetof(city_player, houses_requiring_academy) == 0x4320 && offsetof(city_player, houses_missing_dentist) == 0x4324 &&
                  offsetof(city_player, houses_missing_courthouse) == 0x4328 && offsetof(city_player, houses_missing_food) == 0x432C &&
                  offsetof(city_player, sphinx_stage) == 0x433A && offsetof(city_player, houses_missing_health) == 0x433C &&
                  offsetof(city_player, houses_missing_more_health) == 0x4340 && offsetof(city_player, houses_requiring_dentist) == 0x4344 &&
                  offsetof(city_player, houses_requiring_courthouse) == 0x4348 && offsetof(city_player, houses_requiring_health) == 0x434C &&
                  offsetof(city_player, houses_missing_religion) == 0x4350 && offsetof(city_player, houses_missing_second_religion) == 0x4354 &&
                  offsetof(city_player, houses_missing_third_religion) == 0x4358 && offsetof(city_player, houses_requiring_religion) == 0x435C &&
                  offsetof(city_player, juggler_shows) == 0x4360 && offsetof(city_player, juggler_no_shows_weighted) == 0x4364 &&
                  offsetof(city_player, musician_shows) == 0x4368 && offsetof(city_player, musician_no_shows_weighted) == 0x436C &&
                  offsetof(city_player, dancer_shows) == 0x4370 && offsetof(city_player, dancer_no_shows_weighted) == 0x4374 &&
                  offsetof(city_player, senet_shows) == 0x4378 && offsetof(city_player, senet_no_shows_weighted) == 0x437C &&
                  offsetof(city_player, venue_needing_shows) == 0x4380 && offsetof(city_player, average_entertainment) == 0x4384 &&
                  offsetof(city_player, houses_missing_entertainment) == 0x4388 && offsetof(city_player, months_since_festival) == 0x438C &&
                  offsetof(city_player, god_target_mood) == 0x4390 && offsetof(city_player, god_mood) == 0x439A &&
                  offsetof(city_player, god_wrath_bolts) == 0x43A4 && offsetof(city_player, god_extra_state_1) == 0x43C2 &&
                  offsetof(city_player, god_extra_state_2) == 0x43CC && offsetof(city_player, god_extra_state_3) == 0x43D6 &&
                  offsetof(city_player, god_months_since_festival) == 0x43E0 && offsetof(city_player, least_happy_god) == 0x4408 &&
                  offsetof(city_player, no_immigration_cause) == 0x4410 && offsetof(city_player, protesters) == 0x4414 &&
                  offsetof(city_player, thefts) == 0x4418 && offsetof(city_player, health_demand) == 0x441C &&
                  offsetof(city_player, religion_demand) == 0x4420 && offsetof(city_player, education_demand) == 0x4424 &&
                  offsetof(city_player, entertainment_demand) == 0x4428 && offsetof(city_player, rioters) == 0x442C &&
                  offsetof(city_player, selected_rating) == 0x4430 && offsetof(city_player, culture_explanation) == 0x4434 &&
                  offsetof(city_player, prosperity_explanation) == 0x4438 && offsetof(city_player, monument_explanation) == 0x443C &&
                  offsetof(city_player, kingdom_explanation) == 0x4440 && offsetof(city_player, peace_explanation) == 0x4444 &&
                  offsetof(city_player, player_rank) == 0x444C && offsetof(city_player, personal_savings) == 0x4450 &&
                  offsetof(city_player, donated_last_year) == 0x4454 && offsetof(city_player, donated_this_year) == 0x4458 &&
                  offsetof(city_player, donate_amount) == 0x445C && offsetof(city_player, working_dock_ids) == 0x4460 &&
                  offsetof(city_player, has_temple_complex) == 0x4478 && offsetof(city_player, animals) == 0x447A &&
                  offsetof(city_player, num_sea_routes) == 0x447C && offsetof(city_player, num_land_routes) == 0x447E &&
                  offsetof(city_player, sea_trade_problem_duration) == 0x4480 && offsetof(city_player, land_trade_problem_duration) == 0x4482 &&
                  offsetof(city_player, palace_placed) == 0x4486 && offsetof(city_player, working_wharfs) == 0x4488 &&
                  offsetof(city_player, temple_complex_upgrades) == 0x448A && offsetof(city_player, palace_robberies) == 0x448C &&
                  offsetof(city_player, palace_robberies_last_year) == 0x448E && offsetof(city_player, docker_import_resource) == 0x4490 &&
                  offsetof(city_player, docker_export_resource) == 0x4494 && offsetof(city_player, bailout_taken) == 0x4498 &&
                  offsetof(city_player, bailout_refused_message_shown) == 0x449C && offsetof(city_player, cheats_used) == 0x44A0 &&
                  offsetof(city_player, recruiter_x) == 0x44A4 && offsetof(city_player, recruiter_y) == 0x44A6 &&
                  offsetof(city_player, recruiter_grid_offset) == 0x44A8 && offsetof(city_player, recruiter_building_id) == 0x44AC &&
                  offsetof(city_player, recruiter_placed) == 0x44B0 && offsetof(city_player, zoo_shows) == 0x44C8 &&
                  offsetof(city_player, zoo_no_shows_weighted) == 0x44CC && offsetof(city_player, mission_has_won) == 0x44D0 &&
                  offsetof(city_player, mission_continue_months_left) == 0x44D4 && offsetof(city_player, mission_continue_months_chosen) == 0x44D8 &&
                  offsetof(city_player, wage_rate_paid_this_year) == 0x44DC && offsetof(city_player, tribute_estimate) == 0x44E0 &&
                  offsetof(city_player, tribute_paid) == 0x44E4 && offsetof(city_player, tribute_not_paid_last_year) == 0x44E8 &&
                  offsetof(city_player, tribute_not_paid_total_years) == 0x44EC && offsetof(city_player, festival_selected_god) == 0x44F0 &&
                  offsetof(city_player, festival_selected_size) == 0x44F4 && offsetof(city_player, festival_planned_size) == 0x44F8 &&
                  offsetof(city_player, festival_planned_months_to_go) == 0x44FC && offsetof(city_player, festival_planned_god) == 0x4500 &&
                  offsetof(city_player, festival_cost) == 0x4504 && offsetof(city_player, festival_grand_beer) == 0x4510 &&
                  offsetof(city_player, festival_not_enough_beer) == 0x4514 && offsetof(city_player, average_religion) == 0x4518 &&
                  offsetof(city_player, average_education) == 0x451C && offsetof(city_player, average_health) == 0x4520 &&
                  offsetof(city_player, first_festival_effect_months) == 0x4528 && offsetof(city_player, second_festival_effect_months) == 0x452C &&
                  offsetof(city_player, sentiment_unemployment) == 0x4534 && offsetof(city_player, sentiment_previous_value) == 0x4538 &&
                  offsetof(city_player, sentiment_low_mood_cause) == 0x4540 && offsetof(city_player, security_breach_duration) == 0x4544 &&
                  offsetof(city_player, rating_yearly_counters) == 0x4548 && offsetof(city_player, selected_gift_size) == 0x4558 &&
                  offsetof(city_player, months_since_gift) == 0x455C && offsetof(city_player, gift_overdose_penalty) == 0x4560 &&
                  offsetof(city_player, gift_id) == 0x4568 && offsetof(city_player, gift_cost) == 0x4574 &&
                  offsetof(city_player, kingdom_salary_penalty) == 0x4580 && offsetof(city_player, kingdom_milestone_penalty) == 0x4584 &&
                  offsetof(city_player, kingdom_ignored_request_penalty) == 0x4588 && offsetof(city_player, kingdom_last_year) == 0x458C &&
                  offsetof(city_player, kingdom_change) == 0x4590 && offsetof(city_player, natives_hostile) == 0x4594 &&
                  offsetof(city_player, mission_post_operational) == 0x459C && offsetof(city_player, main_native_meeting_x) == 0x45A0 &&
                  offsetof(city_player, main_native_meeting_y) == 0x45A4 && offsetof(city_player, wage_rate_paid_last_year) == 0x45A8 &&
                  offsetof(city_player, food_needed_per_month) == 0x45AC && offsetof(city_player, granaries_understaffed) == 0x45B0 &&
                  offsetof(city_player, granaries_not_operating) == 0x45B4 && offsetof(city_player, granaries_not_operating_with_food) == 0x45B8 &&
                  offsetof(city_player, ra_exports_bonus_months) == 0x45CC && offsetof(city_player, enemies_to_leave) == 0x45D0 &&
                  offsetof(city_player, gods_angry_message_delay) == 0x45D8 && offsetof(city_player, food_consumed_last_month) == 0x45DC &&
                  offsetof(city_player, food_produced_last_month) == 0x45E0 && offsetof(city_player, food_produced_this_month) == 0x45E4 &&
                  offsetof(city_player, estimated_tax_income) == 0x45EC && offsetof(city_player, distribution_center_x) == 0x45F4 &&
                  offsetof(city_player, distribution_center_y) == 0x45F6 && offsetof(city_player, distribution_center_grid_offset) == 0x45F8 &&
                  offsetof(city_player, distribution_center_building_id) == 0x45FC && offsetof(city_player, distribution_center_placed) == 0x4600 &&
                  offsetof(city_player, shipyard_boats_requested) == 0x4630 && offsetof(city_player, enemies) == 0x4634 &&
                  offsetof(city_player, sentiment_wages) == 0x4638 && offsetof(city_player, people_in_huts) == 0x463C &&
                  offsetof(city_player, people_in_residences_and_up) == 0x4640 && offsetof(city_player, kingdom_soldiers) == 0x4644 &&
                  offsetof(city_player, destroyed_buildings) == 0x4658 && offsetof(city_player, total_transport_ships) == 0x4664 &&
                  offsetof(city_player, empire_service_legions_fighting) == 0x4665 && offsetof(city_player, empire_service_legions_not_fighting) == 0x4666 &&
                  offsetof(city_player, total_warships) == 0x4668 && offsetof(city_player, empire_service_warships) == 0x4669 &&
                  offsetof(city_player, legions_fighting) == 0x466A && offsetof(city_player, legions_not_fighting) == 0x466B &&
                  offsetof(city_player, total_legions) == 0x466C && offsetof(city_player, empire_service_legions) == 0x466D &&
                  offsetof(city_player, total_soldiers) == 0x466F && offsetof(city_player, trade_center_building_id) == 0x4674 &&
                  offsetof(city_player, soldiers) == 0x4678 && offsetof(city_player, sound_march_enemy) == 0x467F &&
                  offsetof(city_player, sound_march_horse) == 0x4680 && offsetof(city_player, sentiment_food_toggle) == 0x468B &&
                  offsetof(city_player, kingdom_soldiers_removed) == 0x4694 && offsetof(city_player, chariot_legions) == 0x4698 &&
                  offsetof(city_player, population_highest_ever) == 0x469C && offsetof(city_player, estimated_wages) == 0x46A0 &&
                  offsetof(city_player, luxury_goods_kinds) == 0x46A4 && offsetof(city_player, prosperity_max) == 0x46A8 &&
                  offsetof(city_player, largest_road_networks) == 0x46AC && offsetof(city_player, houses_missing_second_luxury) == 0x46FC &&
                  offsetof(city_player, flotsam_special) == 0x4700 && offsetof(city_player, senet_house_has_plays) == 0x4704 &&
                  offsetof(city_player, fired_message_shown) == 0x4714 && offsetof(city_player, victory_message_shown) == 0x4718 &&
                  offsetof(city_player, mission_saved_game_state) == 0x471C && offsetof(city_player, tutorial_flag_seen_a) == 0x4720 &&
                  offsetof(city_player, tutorial_flag_seen_g) == 0x4724 && offsetof(city_player, attacking_natives) == 0x4728 &&
                  offsetof(city_player, temple_complex_building_id) == 0x472C && offsetof(city_player, monument_def_id) == 0x4730 &&
                  offsetof(city_player, monument_building_id) == 0x473C && offsetof(city_player, houses_requiring_more_courthouse) == 0x4748 &&
                  offsetof(city_player, houses_missing_any_courthouse) == 0x474C && offsetof(city_player, plague_houses) == 0x4750 &&
                  offsetof(city_player, requests_and_festivals_last_year) == 0x4754 && offsetof(city_player, requests_and_festivals_this_year) == 0x4758 &&
                  offsetof(city_player, migration_newcomers_last_month) == 0x475C && offsetof(city_player, crimes_this_month) == 0x4760 &&
                  offsetof(city_player, gold_stolen_this_month) == 0x4764 && offsetof(city_player, advisor_comment) == 0x4768 &&
                  offsetof(city_player, hunting_lodges_working) == 0x476C && offsetof(city_player, game_meat_last_month) == 0x4770 &&
                  offsetof(city_player, game_meat_this_month) == 0x4774 && offsetof(city_player, fishing_wharfs_working) == 0x4778 &&
                  offsetof(city_player, fish_last_month) == 0x477C && offsetof(city_player, fish_this_month) == 0x4780 &&
                  offsetof(city_player, savings_lost) == 0x4784 && offsetof(city_player, festival_square_x) == 0x47A0 &&
                  offsetof(city_player, festival_square_y) == 0x47A4 && offsetof(city_player, difficulty) == 0x47A8 &&
                  offsetof(city_player, gold_last_year) == 0x47AC && offsetof(city_player, gold_this_year) == 0x47B0 &&
                  offsetof(city_player, ra_no_traders_months) == 0x47B4 && offsetof(city_player, festival_square_placed) == 0x4814 &&
                  offsetof(city_player, god_happy_bolts) == 0x4816 && offsetof(city_player, monument_completion_flag) == 0x4820 &&
                  offsetof(city_player, monument_completion_months) == 0x4828 && offsetof(city_player, months_in_debt) == 0x482C &&
                  offsetof(city_player, years_in_debt) == 0x4830 && offsetof(city_player, resource_list_scroll) == 0x4834 &&
                  offsetof(city_player, resource_settings_step) == 0x4838 && offsetof(city_player, granary_food_stored) == 0x483C &&
                  offsetof(city_player, storage_space) == 0x4860 && offsetof(city_player, storage_stored) == 0x48F0 &&
                  offsetof(city_player, plague_count) == 0x4980 && offsetof(city_player, ra_trade_bonus_months) == 0x4984 &&
                  offsetof(city_player, ra_trade_penalty_months) == 0x4986 && offsetof(city_player, ra_trade_minor_penalty_months) == 0x4988 &&
                  offsetof(city_player, seth_minor_blessing_months) == 0x498C && offsetof(city_player, bast_culture_decay_skip) == 0x4990 &&
                  offsetof(city_player, osiris_double_farm_yield) == 0x4994 && offsetof(city_player, osiris_flood_destroys_farms) == 0x4998 &&
                  offsetof(city_player, mansion_placed) == 0x499C && offsetof(city_player, enemy_warships) == 0x49A0 &&
                  offsetof(city_player, resource_gift_received) == 0x49A4 && offsetof(city_player, trade_prices_mode) == 0x49CC &&
                  offsetof(city_player, military_academy_placed) == 0x49D0 && offsetof(city_player, sphinx_stage_before_delete) == 0x49D4,
              "city_player is the savegame's layout");

enum {
    CITY_PLAYER_RECORDS = 2,
};

typedef city_player city_player_records[CITY_PLAYER_RECORDS];

typedef struct {
    uint8_t faction;
    uint8_t started;
} city_player_faction_bytes;

typedef union {
    int16_t word;
    city_player_faction_bytes bytes;
} city_player_faction_entry;

static_assert(sizeof(city_player_faction_entry) == 2, "city_player_faction_entry is a savegame record of 2 bytes");

#endif
