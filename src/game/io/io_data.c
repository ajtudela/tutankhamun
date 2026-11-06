// The built-in data of the files: the savegame and scenario chunk tables, the empire file, the file modes, the
// message token table, the event text keys and the compression tables. io/io_data.h declares them.

#include "building/building.h"
#include "city/message.h"
#include "building/building_data.h"
#include "building/monument_data.h"
#include "city/city_data.h"
#include "editor/editor_data.h"
#include "game/game_data.h"
#include "io/io_data.h"
#include "map/map_data.h"
#include "widget/widget_data.h"
#include "window/window_data.h"
#include "core/random.h"
#include "figure/figure.h"
#include "io/game_file.h"
#include "io/game_file_upgrade.h"
#include "editor/editor_menu.h"
#include "empire/empire.h"
#include "scenario/requests.h"
#include "text/event_text.h"
#include "text/message_tokens_monument.h"

#include <assert.h>
#include <stddef.h>

uint8_t g_invasion_warnings[3232];

int32_t figure_name_counters_unused[4];

int32_t routing_stats_unused_a;

int32_t routing_stats_unused_b;

const char file_mode_read_binary[4] = "rb";

const char str_removed_manmades[24] = "OK :removed manmades=";

const char str_pharaoh_inf[12] = "Pharaoh.inf";

const char empire_file_name_cleopatra[] = "Pharaoh2.emp";

const char empire_file_name[] = "Pharaoh.emp";

const char *str_pharaoh_emp = empire_file_name;

const char *str_pharaoh2_emp = empire_file_name_cleopatra;

const char file_mode_read_update_binary[8] = "r+b";

struct event_text_key_name_pool {
    char pyramid_speedup_bast[28];
    char pyramid_speedup_seth[28];
    char pyramid_speedup_ptah[28];
    char pyramid_speedup_ra[28];
    char pyramid_speedup_osiris[32];
    char pyramid_minor_speedup_announcement[44];
    char pyramid_speedup_announcement[36];
    char pyramid_speedup_title[32];
    char troopcarryover_all_three[32];
    char troopcarryover_arch_char[32];
    char troopcarryover_inf_char[32];
    char troopcarryover_inf_arch[32];
    char troopcarryover_char_only[32];
    char troopcarryover_arch_only[32];
    char troopcarryover_inf_only[32];
    char troopcarryover_initial_announcement[44];
    char troopcarryover_title[28];
    char grandtomb_congratulations[36];
    char grandtomb_congratulations_title[40];
    char largetomb_congratulations[36];
    char largetomb_congratulations_title[40];
    char medtomb_congratulations[32];
    char medtomb_congratulations_title[40];
    char smalltomb_congratulations[36];
    char smalltomb_congratulations_title[40];
    char mausoleum_congratulations[36];
    char mausoleum_congratulations_title[40];
    char lighthouse_congratulations[36];
    char lighthouse_congratulations_title[40];
    char caesareum_congratulations[36];
    char caesareum_congratulations_title[40];
    char abu_simbel_congratulations[36];
    char abu_simbel_congratulations_title[40];
    char alex_library_congratulations[36];
    char alex_library_congratulations_title[44];
    char sun_temple_congratulations[36];
    char sun_temple_congratulations_title[40];
    char obelisk_congratulations[32];
    char obelisk_congratulations_title[40];
    char sphinx_congratulations[32];
    char sphinx_congratulations_title[36];
    char mastaba_congratulations[32];
    char mastaba_congratulations_title[40];
    char mudbrick_pyramid_congratulations[40];
    char mudbrick_pyramid_congratulations_title[48];
    char bent_pyramid_congratulations[36];
    char bent_pyramid_congratulations_title[44];
    char stepped_pyramid_congratulations[40];
    char stepped_pyramid_congratulations_title[48];
    char pyramid_congratulations[32];
    char pyramid_congratulations_title[40];
    char gift_no_reason_c_c[28];
    char gift_no_reason_c_b[28];
    char gift_no_reason_p_c[28];
    char gift_no_reason_p_b[28];
    char gift_refused_reason_c_c[32];
    char gift_refused_reason_c_b[32];
    char gift_refused_reason_p_c[32];
    char gift_refused_reason_p_b[32];
    char gift_forfeited_reason_c_c[36];
    char gift_forfeited_reason_c_b[36];
    char gift_forfeited_reason_p_c[36];
    char gift_forfeited_reason_p_b[36];
    char gift_accepted_reason_c_c[32];
    char gift_accepted_reason_c_b[32];
    char gift_accepted_reason_p_c[32];
    char gift_accepted_reason_p_b[32];
    char bedouin_attacks_you_no_reason_c[40];
    char bedouin_attacks_you_no_reason_b[40];
    char bedouin_attacks_you_reason_c[36];
    char bedouin_attacks_you_reason_b[36];
    char perfect_flood_no_reason_c[36];
    char perfect_flood_no_reason_b[36];
    char perfect_flood_reason_c[32];
    char perfect_flood_reason_b[32];
    char flood_fails_no_reason_c[32];
    char flood_fails_no_reason_b[32];
    char flood_fails_reason_c[28];
    char flood_fails_reason_b[28];
    char landslide_no_reason_c[32];
    char landslide_no_reason_b[32];
    char landslide_reason_c[28];
    char landslide_reason_b[28];
    char goldmine_cavein_no_reason_c[36];
    char goldmine_cavein_no_reason_b[36];
    char goldmine_cavein_reason_c[32];
    char goldmine_cavein_reason_b[32];
    char bad_water_no_reason_c[32];
    char bad_water_no_reason_b[32];
    char bad_water_reason_c[28];
    char bad_water_reason_b[28];
    char wage_change_no_reason_d_c[36];
    char wage_change_no_reason_d_b[36];
    char wage_change_reason_d_c[32];
    char wage_change_reason_d_b[32];
    char wage_change_no_reason_i_c[36];
    char wage_change_no_reason_i_b[36];
    char wage_change_reason_i_c[32];
    char wage_change_reason_i_b[32];
    char sandstorm_no_reason_c[32];
    char sandstorm_no_reason_b[32];
    char sandstorm_reason_c[28];
    char sandstorm_reason_b[28];
    char stormy_seas_no_reason_c[32];
    char stormy_seas_no_reason_b[32];
    char stormy_seas_reason_c[28];
    char stormy_seas_reason_b[28];
    char earthquake_no_reason_c[32];
    char earthquake_no_reason_b[32];
    char earthquake_reason_c[28];
    char earthquake_reason_b[28];
    char demand_change_no_reason_d_c[36];
    char demand_change_no_reason_d_b[36];
    char demand_change_reason_d_c[32];
    char demand_change_reason_d_b[32];
    char demand_change_no_reason_i_c[36];
    char demand_change_no_reason_i_b[36];
    char demand_change_reason_i_c[32];
    char demand_change_reason_i_b[32];
    char price_change_no_reason_d_c[36];
    char price_change_no_reason_d_b[36];
    char price_change_reason_d_c[32];
    char price_change_reason_d_b[32];
    char price_change_no_reason_i_c[36];
    char price_change_no_reason_i_b[36];
    char price_change_reason_i_c[32];
    char price_change_reason_i_b[32];
    char rating_change_no_reason_d_c[36];
    char rating_change_no_reason_d_b[36];
    char rating_change_reason_d_c[32];
    char rating_change_reason_d_b[32];
    char rating_change_no_reason_i_c[36];
    char rating_change_no_reason_i_b[36];
    char rating_change_reason_i_c[32];
    char rating_change_reason_i_b[32];
    char foreign_army_attacks_you_no_reason_c[44];
    char foreign_army_attacks_you_no_reason_b[44];
    char foreign_army_attacks_you_reason_c[44];
    char foreign_army_attacks_you_reason_b[44];
    char eg_city_attacks_you_no_reason_c[40];
    char eg_city_attacks_you_no_reason_b[40];
    char eg_city_attacks_you_reason_c[36];
    char eg_city_attacks_you_reason_b[36];
    char pharaoh_attacks_you_no_reason_c[40];
    char pharaoh_attacks_you_no_reason_b[40];
    char pharaoh_attacks_you_reason_c[36];
    char pharaoh_attacks_you_reason_b[36];
    char acknowledgement_no_reason_c[36];
    char acknowledgement_no_reason_b[36];
    char acknowledgement_reason_c[32];
    char acknowledgement_reason_b[32];
    char battle_lost_no_reason_c[32];
    char battle_lost_no_reason_b[32];
    char battle_lost_reason_c[28];
    char battle_lost_reason_b[28];
    char battle_won_no_reason_c[32];
    char battle_won_no_reason_b[32];
    char battle_won_reason_c[28];
    char battle_won_reason_b[28];
    char eg_city_saved_no_reason_c[36];
    char eg_city_saved_no_reason_b[36];
    char eg_city_saved_reason_c[32];
    char eg_city_saved_reason_b[32];
    char siege_no_reason_c[28];
    char siege_no_reason_b[28];
    char siege_reason_c[24];
    char siege_reason_b[24];
    char route_closed_no_reason_c[32];
    char route_closed_no_reason_b[32];
    char route_closed_reason_c[32];
    char route_closed_reason_b[32];
    char route_opened_no_reason_c[32];
    char route_opened_no_reason_b[32];
    char route_opened_reason_c[32];
    char route_opened_reason_b[32];
    char foreign_city_conquered_no_reason_c[44];
    char foreign_city_conquered_no_reason_b[44];
    char foreign_city_conquered_reason_c[40];
    char foreign_city_conquered_reason_b[40];
    char eg_city_falls_no_reason_c[36];
    char eg_city_falls_no_reason_b[36];
    char eg_city_falls_reason_c[32];
    char eg_city_falls_reason_b[32];
    char threat_no_reason_c_c[28];
    char threat_no_reason_c_b[28];
    char threat_no_reason_p_c[28];
    char threat_no_reason_p_b[28];
    char threat_refuse_reason_c_c[32];
    char threat_refuse_reason_c_b[32];
    char threat_refuse_reason_p_c[32];
    char threat_refuse_reason_p_b[32];
    char threat_too_late_reason_c_c[36];
    char threat_too_late_reason_c_b[36];
    char threat_too_late_reason_p_c[36];
    char threat_too_late_reason_p_b[36];
    char threat_comply_reason_c_c[32];
    char threat_comply_reason_c_b[32];
    char threat_comply_reason_p_c[32];
    char threat_comply_reason_p_b[32];
    char famine_no_reason_c_c[28];
    char famine_no_reason_c_b[28];
    char famine_no_reason_p_c[28];
    char famine_no_reason_p_b[28];
    char famine_refuse_reason_c_c[32];
    char famine_refuse_reason_c_b[32];
    char famine_refuse_reason_p_c[32];
    char famine_refuse_reason_p_b[32];
    char famine_too_late_reason_c_c[36];
    char famine_too_late_reason_c_b[36];
    char famine_too_late_reason_p_c[36];
    char famine_too_late_reason_p_b[36];
    char famine_comply_reason_c_c[32];
    char famine_comply_reason_c_b[32];
    char famine_comply_reason_p_c[32];
    char famine_comply_reason_p_b[32];
    char project_no_reason_c_c[32];
    char project_no_reason_c_b[32];
    char project_no_reason_p_c[32];
    char project_no_reason_p_b[32];
    char project_refuse_reason_c_c[36];
    char project_refuse_reason_c_b[36];
    char project_refuse_reason_p_c[36];
    char project_refuse_reason_p_b[36];
    char project_too_late_reason_c_c[36];
    char project_too_late_reason_c_b[36];
    char project_too_late_reason_p_c[36];
    char project_too_late_reason_p_b[36];
    char project_comply_reason_c_c[36];
    char project_comply_reason_c_b[36];
    char project_comply_reason_p_c[36];
    char project_comply_reason_p_b[36];
    char great_festival_no_reason_c_c[36];
    char great_festival_no_reason_c_b[36];
    char great_festival_no_reason_p_c[36];
    char great_festival_no_reason_p_b[36];
    char great_festival_refuse_reason_c_c[40];
    char great_festival_refuse_reason_c_b[40];
    char great_festival_refuse_reason_p_c[40];
    char great_festival_refuse_reason_p_b[40];
    char great_festival_too_late_reason_c_c[44];
    char great_festival_too_late_reason_c_b[44];
    char great_festival_too_late_reason_p_c[44];
    char great_festival_too_late_reason_p_b[44];
    char great_festival_comply_reason_c_c[40];
    char great_festival_comply_reason_c_b[40];
    char great_festival_comply_reason_p_c[40];
    char great_festival_comply_reason_p_b[40];
    char general_request_no_reason_c_c[40];
    char general_request_no_reason_c_b[40];
    char general_request_no_reason_p_c[40];
    char general_request_no_reason_p_b[40];
    char general_request_refuse_reason_c_c[44];
    char general_request_refuse_reason_c_b[44];
    char general_request_refuse_reason_p_c[44];
    char general_request_refuse_reason_p_b[44];
    char general_request_too_late_reason_c_c[44];
    char general_request_too_late_reason_c_b[44];
    char general_request_too_late_reason_p_c[44];
    char general_request_too_late_reason_p_b[44];
    char general_request_comply_reason_c_c[44];
    char general_request_comply_reason_c_b[44];
    char general_request_comply_reason_p_c[44];
    char general_request_comply_reason_p_b[44];
    char distant_battle_no_reason_c_c[36];
    char distant_battle_no_reason_c_b[36];
    char distant_battle_no_reason_p_c[36];
    char distant_battle_no_reason_p_b[36];
    char distant_battle_lost_battle_reason_c_c[48];
    char distant_battle_lost_battle_reason_c_b[48];
    char distant_battle_lost_battle_reason_p_c[48];
    char distant_battle_lost_battle_reason_p_b[48];
    char distant_battle_refuse_reason_c_c[40];
    char distant_battle_refuse_reason_c_b[40];
    char distant_battle_refuse_reason_p_c[40];
    char distant_battle_refuse_reason_p_b[40];
    char distant_battle_too_late_reason_c_c[44];
    char distant_battle_too_late_reason_c_b[44];
    char distant_battle_too_late_reason_p_c[44];
    char distant_battle_too_late_reason_p_b[44];
    char distant_battle_comply_reason_c_c[40];
    char distant_battle_comply_reason_c_b[40];
    char distant_battle_comply_reason_p_c[40];
    char distant_battle_comply_reason_p_b[40];
    char egyptian_city_attacked_no_reason_c_c[44];
    char egyptian_city_attacked_no_reason_c_b[44];
    char egyptian_city_attacked_no_reason_p_c[44];
    char egyptian_city_attacked_no_reason_p_b[44];
    char egyptian_city_attacked_lost_battle_reason_c_c[56];
    char egyptian_city_attacked_lost_battle_reason_c_b[56];
    char egyptian_city_attacked_lost_battle_reason_p_c[56];
    char egyptian_city_attacked_lost_battle_reason_p_b[56];
    char egyptian_city_attacked_refuse_reason_c_c[48];
    char egyptian_city_attacked_refuse_reason_c_b[48];
    char egyptian_city_attacked_refuse_reason_p_c[48];
    char egyptian_city_attacked_refuse_reason_p_b[48];
    char egyptian_city_attacked_too_late_reason_c_c[52];
    char egyptian_city_attacked_too_late_reason_c_b[52];
    char egyptian_city_attacked_too_late_reason_p_c[52];
    char egyptian_city_attacked_too_late_reason_p_b[52];
    char egyptian_city_attacked_comply_reason_c_c[48];
    char egyptian_city_attacked_comply_reason_c_b[48];
    char egyptian_city_attacked_comply_reason_p_c[48];
    char egyptian_city_attacked_comply_reason_p_b[48];
    char bedouin_attacks_you_disembarked_alert[48];
    char foreign_army_attacks_you_disembarked_alert[52];
    char eg_city_attacks_you_disembarked_alert[48];
    char pharaoh_attacks_you_disembarked_alert[48];
    char gift_no_reason_c_a[28];
    char gift_no_reason_p_a[28];
    char gift_refused_reason_c_a[32];
    char gift_refused_reason_p_a[32];
    char gift_forfeited_reason_c_a[36];
    char gift_forfeited_reason_p_a[36];
    char gift_accepted_reason_c_a[32];
    char gift_accepted_reason_p_a[32];
    char gift_refused_c[24];
    char gift_refused_p[24];
    char gift_postponed_c[24];
    char gift_postponed_p[24];
    char gift_cash_accepted_c[28];
    char gift_cash_accepted_p[28];
    char gift_accepted_c[24];
    char gift_accepted_p[24];
    char gift_forfeited_c[24];
    char gift_forfeited_p[24];
    char gift_last_chance_c[28];
    char gift_last_chance_p[28];
    char gift_insufficient_space_c[36];
    char gift_insufficient_space_p[36];
    char gift_partial_space_c[28];
    char gift_partial_space_p[28];
    char gift_cash_granted_c[28];
    char gift_cash_granted_p[28];
    char gift_granted_c[24];
    char gift_granted_p[24];
    char gift_title_c[20];
    char gift_title_p[20];
    char bedouin_attacks_you_no_reason_a[40];
    char bedouin_attacks_you_reason_a[36];
    char bedouin_attacks_you_city_attacked_alert[48];
    char bedouin_attacks_you_1month_warning[44];
    char bedouin_attacks_you_6month_warning[44];
    char bedouin_attacks_you_1year_reminder[44];
    char bedouin_attacks_you_2year_reminder[44];
    char bedouin_attacks_you_initial_announcement[48];
    char bedouin_attacks_you_title[36];
    char perfect_flood_no_reason_a[36];
    char perfect_flood_reason_a[32];
    char perfect_flood_initial_announcement[44];
    char perfect_flood_title[28];
    char flood_fails_no_reason_a[32];
    char flood_fails_reason_a[28];
    char flood_fails_initial_announcement[40];
    char flood_fails_title[28];
    char landslide_no_reason_a[32];
    char landslide_reason_a[28];
    char landslide_initial_announcement[40];
    char landslide_title[24];
    char goldmine_cavein_no_reason_a[36];
    char goldmine_cavein_reason_a[32];
    char goldmine_cavein_initial_announcement[44];
    char goldmine_cavein_title[32];
    char bad_water_no_reason_a[32];
    char bad_water_reason_a[28];
    char bad_water_initial_announcement[40];
    char bad_water_title[24];
    char wage_change_no_reason_d_a[36];
    char wage_change_reason_d_a[32];
    char wage_change_initial_announcement_d[44];
    char wage_change_title_d[28];
    char wage_change_no_reason_i_a[36];
    char wage_change_reason_i_a[32];
    char wage_change_initial_announcement_i[44];
    char wage_change_title_i[28];
    char sandstorm_no_reason_a[32];
    char sandstorm_reason_a[28];
    char sandstorm_initial_announcement[40];
    char sandstorm_title[24];
    char stormy_seas_no_reason_a[32];
    char stormy_seas_reason_a[28];
    char stormy_seas_initial_announcement[40];
    char stormy_seas_title[28];
    char earthquake_no_reason_a[32];
    char earthquake_reason_a[28];
    char earthquake_initial_announcement[40];
    char earthquake_title[24];
    char demand_change_no_reason_d_a[36];
    char demand_change_reason_d_a[32];
    char demand_change_initial_announcement_d[44];
    char demand_change_title_d[32];
    char demand_change_no_reason_i_a[36];
    char demand_change_reason_i_a[32];
    char demand_change_initial_announcement_i[44];
    char demand_change_title_i[32];
    char price_change_no_reason_d_a[36];
    char price_change_reason_d_a[32];
    char price_change_initial_announcement_d[44];
    char price_change_title_d[28];
    char price_change_no_reason_i_a[36];
    char price_change_reason_i_a[32];
    char price_change_initial_announcement_i[44];
    char price_change_title_i[28];
    char rating_change_no_reason_d_a[36];
    char rating_change_reason_d_a[32];
    char rating_change_initial_announcement_d[44];
    char rating_change_title_d[32];
    char rating_change_no_reason_i_a[36];
    char rating_change_reason_i_a[32];
    char rating_change_initial_announcement_i[44];
    char rating_change_title_i[32];
    char foreign_army_attacks_you_no_reason_a[44];
    char foreign_army_attacks_you_reason_a[44];
    char foreign_army_attacks_you_city_attacked_alert[52];
    char foreign_army_attacks_you_1month_warning[48];
    char foreign_army_attacks_you_6month_warning[48];
    char foreign_army_attacks_you_1year_reminder[48];
    char foreign_army_attacks_you_2year_reminder[48];
    char foreign_army_attacks_you_initial_announcement[56];
    char foreign_army_attacks_you_title[40];
    char eg_city_attacks_you_because_of_low_favor[48];
    char eg_city_attacks_you_no_reason_a[40];
    char eg_city_attacks_you_reason_a[36];
    char eg_city_attacks_you_city_attacked_alert[48];
    char eg_city_attacks_you_1month_warning[44];
    char eg_city_attacks_you_6month_warning[44];
    char eg_city_attacks_you_1year_reminder[44];
    char eg_city_attacks_you_2year_reminder[44];
    char eg_city_attacks_you_initial_announcement[48];
    char eg_city_attacks_you_title[36];
    char pharaoh_attacks_you_because_of_low_favor[48];
    char pharaoh_attacks_you_no_reason_a[40];
    char pharaoh_attacks_you_reason_a[36];
    char pharaoh_attacks_you_city_attacked_alert[48];
    char pharaoh_attacks_you_1month_warning[44];
    char pharaoh_attacks_you_6month_warning[44];
    char pharaoh_attacks_you_1year_reminder[44];
    char pharaoh_attacks_you_2year_reminder[44];
    char pharaoh_attacks_you_initial_announcement[48];
    char pharaoh_attacks_you_title[36];
    char acknowledgement_no_reason_a[36];
    char acknowledgement_reason_a[32];
    char acknowledgement_initial_announcement[44];
    char acknowledgement_title[32];
    char battle_lost_no_reason_a[32];
    char battle_lost_reason_a[28];
    char battle_lost_initial_announcement[40];
    char battle_lost_title[28];
    char battle_won_no_reason_a[32];
    char battle_won_reason_a[28];
    char battle_won_initial_announcement[40];
    char battle_won_title[24];
    char eg_city_saved_no_reason_a[36];
    char eg_city_saved_reason_a[32];
    char eg_city_saved_initial_announcement[44];
    char eg_city_saved_title[28];
    char siege_no_reason_a[28];
    char siege_reason_a[24];
    char siege_initial_announcement[36];
    char siege_title[20];
    char route_closed_no_reason_a[32];
    char route_closed_reason_a[32];
    char route_closed_initial_announcement[44];
    char route_closed_title[28];
    char route_opened_no_reason_a[32];
    char route_opened_reason_a[32];
    char route_opened_initial_announcement[44];
    char route_opened_title[28];
    char foreign_city_conquered_no_reason_a[44];
    char foreign_city_conquered_reason_a[40];
    char foreign_city_conquered_initial_announcement[52];
    char foreign_city_conquered_title[36];
    char eg_city_falls_no_reason_a[36];
    char eg_city_falls_reason_a[32];
    char eg_city_falls_initial_announcement[44];
    char eg_city_falls_title[28];
    char threat_no_reason_c_a[28];
    char threat_no_reason_p_a[28];
    char threat_refuse_reason_c_a[32];
    char threat_refuse_reason_p_a[32];
    char threat_too_late_reason_c_a[36];
    char threat_too_late_reason_p_a[36];
    char threat_comply_reason_c_a[32];
    char threat_comply_reason_p_a[32];
    char threat_warning_c[24];
    char threat_warning_p[24];
    char threat_overdue_c[24];
    char threat_overdue_p[24];
    char threat_reminder_c[28];
    char threat_reminder_p[28];
    char threat_initial_announcement_c[40];
    char threat_initial_announcement_p[40];
    char threat_title_c[24];
    char threat_title_p[24];
    char famine_no_reason_c_a[28];
    char famine_no_reason_p_a[28];
    char famine_refuse_reason_c_a[32];
    char famine_refuse_reason_p_a[32];
    char famine_too_late_reason_c_a[36];
    char famine_too_late_reason_p_a[36];
    char famine_comply_reason_c_a[32];
    char famine_comply_reason_p_a[32];
    char famine_warning_c[24];
    char famine_warning_p[24];
    char famine_overdue_c[24];
    char famine_overdue_p[24];
    char famine_reminder_c[28];
    char famine_reminder_p[28];
    char famine_initial_announcement_c[40];
    char famine_initial_announcement_p[40];
    char famine_title_c[24];
    char famine_title_p[24];
    char project_no_reason_c_a[32];
    char project_no_reason_p_a[32];
    char project_refuse_reason_c_a[36];
    char project_refuse_reason_p_a[36];
    char project_too_late_reason_c_a[36];
    char project_too_late_reason_p_a[36];
    char project_comply_reason_c_a[36];
    char project_comply_reason_p_a[36];
    char project_warning_c[28];
    char project_warning_p[28];
    char project_overdue_c[28];
    char project_overdue_p[28];
    char project_reminder_c[28];
    char project_reminder_p[28];
    char project_initial_announcement_c[40];
    char project_initial_announcement_p[40];
    char project_title_c[24];
    char project_title_p[24];
    char great_festival_no_reason_c_a[36];
    char great_festival_no_reason_p_a[36];
    char great_festival_refuse_reason_c_a[40];
    char great_festival_refuse_reason_p_a[40];
    char great_festival_too_late_reason_c_a[44];
    char great_festival_too_late_reason_p_a[44];
    char great_festival_comply_reason_c_a[40];
    char great_festival_comply_reason_p_a[40];
    char great_festival_warning_c[32];
    char great_festival_warning_p[32];
    char great_festival_overdue_c[32];
    char great_festival_overdue_p[32];
    char great_festival_reminder_c[36];
    char great_festival_reminder_p[36];
    char great_festival_initial_announcement_c[48];
    char great_festival_initial_announcement_p[48];
    char great_festival_title_c[32];
    char great_festival_title_p[32];
    char general_request_no_reason_c_a[40];
    char general_request_no_reason_p_a[40];
    char general_request_refuse_reason_c_a[44];
    char general_request_refuse_reason_p_a[44];
    char general_request_too_late_reason_c_a[44];
    char general_request_too_late_reason_p_a[44];
    char general_request_comply_reason_c_a[44];
    char general_request_comply_reason_p_a[44];
    char general_request_warning_c[36];
    char general_request_warning_p[36];
    char general_request_overdue_c[36];
    char general_request_overdue_p[36];
    char general_request_reminder_c[36];
    char general_request_reminder_p[36];
    char general_request_initial_announcement_c[48];
    char general_request_initial_announcement_p[48];
    char general_request_title_c[32];
    char general_request_title_p[32];
    char distant_battle_no_reason_c_a[36];
    char distant_battle_no_reason_p_a[36];
    char distant_battle_lost_battle_reason_c_a[48];
    char distant_battle_lost_battle_reason_p_a[48];
    char distant_battle_refuse_reason_c_a[40];
    char distant_battle_refuse_reason_p_a[40];
    char distant_battle_too_late_reason_c_a[44];
    char distant_battle_too_late_reason_p_a[44];
    char distant_battle_comply_reason_c_a[40];
    char distant_battle_comply_reason_p_a[40];
    char distant_battle_last_reminder_c[40];
    char distant_battle_last_reminder_p[40];
    char distant_battle_first_reminder_c[40];
    char distant_battle_first_reminder_p[40];
    char distant_battle_initial_announcement_c[48];
    char distant_battle_initial_announcement_p[48];
    char distant_battle_title_c[32];
    char distant_battle_title_p[32];
    char egyptian_city_attacked_no_reason_c_a[44];
    char egyptian_city_attacked_no_reason_p_a[44];
    char egyptian_city_attacked_lost_battle_reason_c_a[56];
    char egyptian_city_attacked_lost_battle_reason_p_a[56];
    char egyptian_city_attacked_refuse_reason_c_a[48];
    char egyptian_city_attacked_refuse_reason_p_a[48];
    char egyptian_city_attacked_too_late_reason_c_a[52];
    char egyptian_city_attacked_too_late_reason_p_a[52];
    char egyptian_city_attacked_comply_reason_c_a[48];
    char egyptian_city_attacked_comply_reason_p_a[48];
    char egyptian_city_attacked_last_reminder_c[48];
    char egyptian_city_attacked_last_reminder_p[48];
    char egyptian_city_attacked_first_reminder_c[48];
    char egyptian_city_attacked_first_reminder_p[48];
    char egyptian_city_attacked_initial_announcement_c[56];
    char egyptian_city_attacked_initial_announcement_p[56];
    char egyptian_city_attacked_title_c[40];
    char egyptian_city_attacked_title_p[40];
};

event_text_key_name_pool event_text_key_names = {
    .pyramid_speedup_bast = "PHRASE_pyramid_speedup_Bast",
    .pyramid_speedup_seth = "PHRASE_pyramid_speedup_Seth",
    .pyramid_speedup_ptah = "PHRASE_pyramid_speedup_Ptah",
    .pyramid_speedup_ra = "PHRASE_pyramid_speedup_Ra",
    .pyramid_speedup_osiris = "PHRASE_pyramid_speedup_Osiris",
    .pyramid_minor_speedup_announcement = "PHRASE_pyramid_minor_speedup_announcement",
    .pyramid_speedup_announcement = "PHRASE_pyramid_speedup_announcement",
    .pyramid_speedup_title = "PHRASE_pyramid_speedup_title",
    .troopcarryover_all_three = "PHRASE_troopcarryover_all_three",
    .troopcarryover_arch_char = "PHRASE_troopcarryover_arch_char",
    .troopcarryover_inf_char = "PHRASE_troopcarryover_inf_char",
    .troopcarryover_inf_arch = "PHRASE_troopcarryover_inf_arch",
    .troopcarryover_char_only = "PHRASE_troopcarryover_char_only",
    .troopcarryover_arch_only = "PHRASE_troopcarryover_arch_only",
    .troopcarryover_inf_only = "PHRASE_troopcarryover_inf_only",
    .troopcarryover_initial_announcement = "PHRASE_troopcarryover_initial_announcement",
    .troopcarryover_title = "PHRASE_troopcarryover_title",
    .grandtomb_congratulations = "PHRASE_grandtomb_congratulations",
    .grandtomb_congratulations_title = "PHRASE_grandtomb_congratulations_title",
    .largetomb_congratulations = "PHRASE_largetomb_congratulations",
    .largetomb_congratulations_title = "PHRASE_largetomb_congratulations_title",
    .medtomb_congratulations = "PHRASE_medtomb_congratulations",
    .medtomb_congratulations_title = "PHRASE_medtomb_congratulations_title",
    .smalltomb_congratulations = "PHRASE_smalltomb_congratulations",
    .smalltomb_congratulations_title = "PHRASE_smalltomb_congratulations_title",
    .mausoleum_congratulations = "PHRASE_mausoleum_congratulations",
    .mausoleum_congratulations_title = "PHRASE_mausoleum_congratulations_title",
    .lighthouse_congratulations = "PHRASE_lighthouse_congratulations",
    .lighthouse_congratulations_title = "PHRASE_lighthouse_congratulations_title",
    .caesareum_congratulations = "PHRASE_caesareum_congratulations",
    .caesareum_congratulations_title = "PHRASE_caesareum_congratulations_title",
    .abu_simbel_congratulations = "PHRASE_abu_simbel_congratulations",
    .abu_simbel_congratulations_title = "PHRASE_abu_simbel_congratulations_title",
    .alex_library_congratulations = "PHRASE_alex_library_congratulations",
    .alex_library_congratulations_title = "PHRASE_alex_library_congratulations_title",
    .sun_temple_congratulations = "PHRASE_sun_temple_congratulations",
    .sun_temple_congratulations_title = "PHRASE_sun_temple_congratulations_title",
    .obelisk_congratulations = "PHRASE_obelisk_congratulations",
    .obelisk_congratulations_title = "PHRASE_obelisk_congratulations_title",
    .sphinx_congratulations = "PHRASE_sphinx_congratulations",
    .sphinx_congratulations_title = "PHRASE_sphinx_congratulations_title",
    .mastaba_congratulations = "PHRASE_mastaba_congratulations",
    .mastaba_congratulations_title = "PHRASE_mastaba_congratulations_title",
    .mudbrick_pyramid_congratulations = "PHRASE_mudbrick_pyramid_congratulations",
    .mudbrick_pyramid_congratulations_title = "PHRASE_mudbrick_pyramid_congratulations_title",
    .bent_pyramid_congratulations = "PHRASE_bent_pyramid_congratulations",
    .bent_pyramid_congratulations_title = "PHRASE_bent_pyramid_congratulations_title",
    .stepped_pyramid_congratulations = "PHRASE_stepped_pyramid_congratulations",
    .stepped_pyramid_congratulations_title = "PHRASE_stepped_pyramid_congratulations_title",
    .pyramid_congratulations = "PHRASE_pyramid_congratulations",
    .pyramid_congratulations_title = "PHRASE_pyramid_congratulations_title",
    .gift_no_reason_c_c = "PHRASE_gift_no_reason_C_C",
    .gift_no_reason_c_b = "PHRASE_gift_no_reason_C_B",
    .gift_no_reason_p_c = "PHRASE_gift_no_reason_P_C",
    .gift_no_reason_p_b = "PHRASE_gift_no_reason_P_B",
    .gift_refused_reason_c_c = "PHRASE_gift_refused_reason_C_C",
    .gift_refused_reason_c_b = "PHRASE_gift_refused_reason_C_B",
    .gift_refused_reason_p_c = "PHRASE_gift_refused_reason_P_C",
    .gift_refused_reason_p_b = "PHRASE_gift_refused_reason_P_B",
    .gift_forfeited_reason_c_c = "PHRASE_gift_forfeited_reason_C_C",
    .gift_forfeited_reason_c_b = "PHRASE_gift_forfeited_reason_C_B",
    .gift_forfeited_reason_p_c = "PHRASE_gift_forfeited_reason_P_C",
    .gift_forfeited_reason_p_b = "PHRASE_gift_forfeited_reason_P_B",
    .gift_accepted_reason_c_c = "PHRASE_gift_accepted_reason_C_C",
    .gift_accepted_reason_c_b = "PHRASE_gift_accepted_reason_C_B",
    .gift_accepted_reason_p_c = "PHRASE_gift_accepted_reason_P_C",
    .gift_accepted_reason_p_b = "PHRASE_gift_accepted_reason_P_B",
    .bedouin_attacks_you_no_reason_c = "PHRASE_bedouin_attacks_you_no_reason_C",
    .bedouin_attacks_you_no_reason_b = "PHRASE_bedouin_attacks_you_no_reason_B",
    .bedouin_attacks_you_reason_c = "PHRASE_bedouin_attacks_you_reason_C",
    .bedouin_attacks_you_reason_b = "PHRASE_bedouin_attacks_you_reason_B",
    .perfect_flood_no_reason_c = "PHRASE_perfect_flood_no_reason_C",
    .perfect_flood_no_reason_b = "PHRASE_perfect_flood_no_reason_B",
    .perfect_flood_reason_c = "PHRASE_perfect_flood_reason_C",
    .perfect_flood_reason_b = "PHRASE_perfect_flood_reason_B",
    .flood_fails_no_reason_c = "PHRASE_flood_fails_no_reason_C",
    .flood_fails_no_reason_b = "PHRASE_flood_fails_no_reason_B",
    .flood_fails_reason_c = "PHRASE_flood_fails_reason_C",
    .flood_fails_reason_b = "PHRASE_flood_fails_reason_B",
    .landslide_no_reason_c = "PHRASE_landslide_no_reason_C",
    .landslide_no_reason_b = "PHRASE_landslide_no_reason_B",
    .landslide_reason_c = "PHRASE_landslide_reason_C",
    .landslide_reason_b = "PHRASE_landslide_reason_B",
    .goldmine_cavein_no_reason_c = "PHRASE_goldmine_cavein_no_reason_C",
    .goldmine_cavein_no_reason_b = "PHRASE_goldmine_cavein_no_reason_B",
    .goldmine_cavein_reason_c = "PHRASE_goldmine_cavein_reason_C",
    .goldmine_cavein_reason_b = "PHRASE_goldmine_cavein_reason_B",
    .bad_water_no_reason_c = "PHRASE_bad_water_no_reason_C",
    .bad_water_no_reason_b = "PHRASE_bad_water_no_reason_B",
    .bad_water_reason_c = "PHRASE_bad_water_reason_C",
    .bad_water_reason_b = "PHRASE_bad_water_reason_B",
    .wage_change_no_reason_d_c = "PHRASE_wage_change_no_reason_D_C",
    .wage_change_no_reason_d_b = "PHRASE_wage_change_no_reason_D_B",
    .wage_change_reason_d_c = "PHRASE_wage_change_reason_D_C",
    .wage_change_reason_d_b = "PHRASE_wage_change_reason_D_B",
    .wage_change_no_reason_i_c = "PHRASE_wage_change_no_reason_I_C",
    .wage_change_no_reason_i_b = "PHRASE_wage_change_no_reason_I_B",
    .wage_change_reason_i_c = "PHRASE_wage_change_reason_I_C",
    .wage_change_reason_i_b = "PHRASE_wage_change_reason_I_B",
    .sandstorm_no_reason_c = "PHRASE_sandstorm_no_reason_C",
    .sandstorm_no_reason_b = "PHRASE_sandstorm_no_reason_B",
    .sandstorm_reason_c = "PHRASE_sandstorm_reason_C",
    .sandstorm_reason_b = "PHRASE_sandstorm_reason_B",
    .stormy_seas_no_reason_c = "PHRASE_stormy_seas_no_reason_C",
    .stormy_seas_no_reason_b = "PHRASE_stormy_seas_no_reason_B",
    .stormy_seas_reason_c = "PHRASE_stormy_seas_reason_C",
    .stormy_seas_reason_b = "PHRASE_stormy_seas_reason_B",
    .earthquake_no_reason_c = "PHRASE_earthquake_no_reason_C",
    .earthquake_no_reason_b = "PHRASE_earthquake_no_reason_B",
    .earthquake_reason_c = "PHRASE_earthquake_reason_C",
    .earthquake_reason_b = "PHRASE_earthquake_reason_B",
    .demand_change_no_reason_d_c = "PHRASE_demand_change_no_reason_D_C",
    .demand_change_no_reason_d_b = "PHRASE_demand_change_no_reason_D_B",
    .demand_change_reason_d_c = "PHRASE_demand_change_reason_D_C",
    .demand_change_reason_d_b = "PHRASE_demand_change_reason_D_B",
    .demand_change_no_reason_i_c = "PHRASE_demand_change_no_reason_I_C",
    .demand_change_no_reason_i_b = "PHRASE_demand_change_no_reason_I_B",
    .demand_change_reason_i_c = "PHRASE_demand_change_reason_I_C",
    .demand_change_reason_i_b = "PHRASE_demand_change_reason_I_B",
    .price_change_no_reason_d_c = "PHRASE_price_change_no_reason_D_C",
    .price_change_no_reason_d_b = "PHRASE_price_change_no_reason_D_B",
    .price_change_reason_d_c = "PHRASE_price_change_reason_D_C",
    .price_change_reason_d_b = "PHRASE_price_change_reason_D_B",
    .price_change_no_reason_i_c = "PHRASE_price_change_no_reason_I_C",
    .price_change_no_reason_i_b = "PHRASE_price_change_no_reason_I_B",
    .price_change_reason_i_c = "PHRASE_price_change_reason_I_C",
    .price_change_reason_i_b = "PHRASE_price_change_reason_I_B",
    .rating_change_no_reason_d_c = "PHRASE_rating_change_no_reason_D_C",
    .rating_change_no_reason_d_b = "PHRASE_rating_change_no_reason_D_B",
    .rating_change_reason_d_c = "PHRASE_rating_change_reason_D_C",
    .rating_change_reason_d_b = "PHRASE_rating_change_reason_D_B",
    .rating_change_no_reason_i_c = "PHRASE_rating_change_no_reason_I_C",
    .rating_change_no_reason_i_b = "PHRASE_rating_change_no_reason_I_B",
    .rating_change_reason_i_c = "PHRASE_rating_change_reason_I_C",
    .rating_change_reason_i_b = "PHRASE_rating_change_reason_I_B",
    .foreign_army_attacks_you_no_reason_c = "PHRASE_foreign_army_attacks_you_no_reason_C",
    .foreign_army_attacks_you_no_reason_b = "PHRASE_foreign_army_attacks_you_no_reason_B",
    .foreign_army_attacks_you_reason_c = "PHRASE_foreign_army_attacks_you_reason_C",
    .foreign_army_attacks_you_reason_b = "PHRASE_foreign_army_attacks_you_reason_B",
    .eg_city_attacks_you_no_reason_c = "PHRASE_eg_city_attacks_you_no_reason_C",
    .eg_city_attacks_you_no_reason_b = "PHRASE_eg_city_attacks_you_no_reason_B",
    .eg_city_attacks_you_reason_c = "PHRASE_eg_city_attacks_you_reason_C",
    .eg_city_attacks_you_reason_b = "PHRASE_eg_city_attacks_you_reason_B",
    .pharaoh_attacks_you_no_reason_c = "PHRASE_pharaoh_attacks_you_no_reason_C",
    .pharaoh_attacks_you_no_reason_b = "PHRASE_pharaoh_attacks_you_no_reason_B",
    .pharaoh_attacks_you_reason_c = "PHRASE_pharaoh_attacks_you_reason_C",
    .pharaoh_attacks_you_reason_b = "PHRASE_pharaoh_attacks_you_reason_B",
    .acknowledgement_no_reason_c = "PHRASE_acknowledgement_no_reason_C",
    .acknowledgement_no_reason_b = "PHRASE_acknowledgement_no_reason_B",
    .acknowledgement_reason_c = "PHRASE_acknowledgement_reason_C",
    .acknowledgement_reason_b = "PHRASE_acknowledgement_reason_B",
    .battle_lost_no_reason_c = "PHRASE_battle_lost_no_reason_C",
    .battle_lost_no_reason_b = "PHRASE_battle_lost_no_reason_B",
    .battle_lost_reason_c = "PHRASE_battle_lost_reason_C",
    .battle_lost_reason_b = "PHRASE_battle_lost_reason_B",
    .battle_won_no_reason_c = "PHRASE_battle_won_no_reason_C",
    .battle_won_no_reason_b = "PHRASE_battle_won_no_reason_B",
    .battle_won_reason_c = "PHRASE_battle_won_reason_C",
    .battle_won_reason_b = "PHRASE_battle_won_reason_B",
    .eg_city_saved_no_reason_c = "PHRASE_eg_city_saved_no_reason_C",
    .eg_city_saved_no_reason_b = "PHRASE_eg_city_saved_no_reason_B",
    .eg_city_saved_reason_c = "PHRASE_eg_city_saved_reason_C",
    .eg_city_saved_reason_b = "PHRASE_eg_city_saved_reason_B",
    .siege_no_reason_c = "PHRASE_siege_no_reason_C",
    .siege_no_reason_b = "PHRASE_siege_no_reason_B",
    .siege_reason_c = "PHRASE_siege_reason_C",
    .siege_reason_b = "PHRASE_siege_reason_B",
    .route_closed_no_reason_c = "PHRASE_route_closed_no_reason_C",
    .route_closed_no_reason_b = "PHRASE_route_closed_no_reason_B",
    .route_closed_reason_c = "PHRASE_route_closed_reason_C",
    .route_closed_reason_b = "PHRASE_route_closed_reason_B",
    .route_opened_no_reason_c = "PHRASE_route_opened_no_reason_C",
    .route_opened_no_reason_b = "PHRASE_route_opened_no_reason_B",
    .route_opened_reason_c = "PHRASE_route_opened_reason_C",
    .route_opened_reason_b = "PHRASE_route_opened_reason_B",
    .foreign_city_conquered_no_reason_c = "PHRASE_foreign_city_conquered_no_reason_C",
    .foreign_city_conquered_no_reason_b = "PHRASE_foreign_city_conquered_no_reason_B",
    .foreign_city_conquered_reason_c = "PHRASE_foreign_city_conquered_reason_C",
    .foreign_city_conquered_reason_b = "PHRASE_foreign_city_conquered_reason_B",
    .eg_city_falls_no_reason_c = "PHRASE_eg_city_falls_no_reason_C",
    .eg_city_falls_no_reason_b = "PHRASE_eg_city_falls_no_reason_B",
    .eg_city_falls_reason_c = "PHRASE_eg_city_falls_reason_C",
    .eg_city_falls_reason_b = "PHRASE_eg_city_falls_reason_B",
    .threat_no_reason_c_c = "PHRASE_threat_no_reason_C_C",
    .threat_no_reason_c_b = "PHRASE_threat_no_reason_C_B",
    .threat_no_reason_p_c = "PHRASE_threat_no_reason_P_C",
    .threat_no_reason_p_b = "PHRASE_threat_no_reason_P_B",
    .threat_refuse_reason_c_c = "PHRASE_threat_refuse_reason_C_C",
    .threat_refuse_reason_c_b = "PHRASE_threat_refuse_reason_C_B",
    .threat_refuse_reason_p_c = "PHRASE_threat_refuse_reason_P_C",
    .threat_refuse_reason_p_b = "PHRASE_threat_refuse_reason_P_B",
    .threat_too_late_reason_c_c = "PHRASE_threat_too_late_reason_C_C",
    .threat_too_late_reason_c_b = "PHRASE_threat_too_late_reason_C_B",
    .threat_too_late_reason_p_c = "PHRASE_threat_too_late_reason_P_C",
    .threat_too_late_reason_p_b = "PHRASE_threat_too_late_reason_P_B",
    .threat_comply_reason_c_c = "PHRASE_threat_comply_reason_C_C",
    .threat_comply_reason_c_b = "PHRASE_threat_comply_reason_C_B",
    .threat_comply_reason_p_c = "PHRASE_threat_comply_reason_P_C",
    .threat_comply_reason_p_b = "PHRASE_threat_comply_reason_P_B",
    .famine_no_reason_c_c = "PHRASE_famine_no_reason_C_C",
    .famine_no_reason_c_b = "PHRASE_famine_no_reason_C_B",
    .famine_no_reason_p_c = "PHRASE_famine_no_reason_P_C",
    .famine_no_reason_p_b = "PHRASE_famine_no_reason_P_B",
    .famine_refuse_reason_c_c = "PHRASE_famine_refuse_reason_C_C",
    .famine_refuse_reason_c_b = "PHRASE_famine_refuse_reason_C_B",
    .famine_refuse_reason_p_c = "PHRASE_famine_refuse_reason_P_C",
    .famine_refuse_reason_p_b = "PHRASE_famine_refuse_reason_P_B",
    .famine_too_late_reason_c_c = "PHRASE_famine_too_late_reason_C_C",
    .famine_too_late_reason_c_b = "PHRASE_famine_too_late_reason_C_B",
    .famine_too_late_reason_p_c = "PHRASE_famine_too_late_reason_P_C",
    .famine_too_late_reason_p_b = "PHRASE_famine_too_late_reason_P_B",
    .famine_comply_reason_c_c = "PHRASE_famine_comply_reason_C_C",
    .famine_comply_reason_c_b = "PHRASE_famine_comply_reason_C_B",
    .famine_comply_reason_p_c = "PHRASE_famine_comply_reason_P_C",
    .famine_comply_reason_p_b = "PHRASE_famine_comply_reason_P_B",
    .project_no_reason_c_c = "PHRASE_project_no_reason_C_C",
    .project_no_reason_c_b = "PHRASE_project_no_reason_C_B",
    .project_no_reason_p_c = "PHRASE_project_no_reason_P_C",
    .project_no_reason_p_b = "PHRASE_project_no_reason_P_B",
    .project_refuse_reason_c_c = "PHRASE_project_refuse_reason_C_C",
    .project_refuse_reason_c_b = "PHRASE_project_refuse_reason_C_B",
    .project_refuse_reason_p_c = "PHRASE_project_refuse_reason_P_C",
    .project_refuse_reason_p_b = "PHRASE_project_refuse_reason_P_B",
    .project_too_late_reason_c_c = "PHRASE_project_too_late_reason_C_C",
    .project_too_late_reason_c_b = "PHRASE_project_too_late_reason_C_B",
    .project_too_late_reason_p_c = "PHRASE_project_too_late_reason_P_C",
    .project_too_late_reason_p_b = "PHRASE_project_too_late_reason_P_B",
    .project_comply_reason_c_c = "PHRASE_project_comply_reason_C_C",
    .project_comply_reason_c_b = "PHRASE_project_comply_reason_C_B",
    .project_comply_reason_p_c = "PHRASE_project_comply_reason_P_C",
    .project_comply_reason_p_b = "PHRASE_project_comply_reason_P_B",
    .great_festival_no_reason_c_c = "PHRASE_great_festival_no_reason_C_C",
    .great_festival_no_reason_c_b = "PHRASE_great_festival_no_reason_C_B",
    .great_festival_no_reason_p_c = "PHRASE_great_festival_no_reason_P_C",
    .great_festival_no_reason_p_b = "PHRASE_great_festival_no_reason_P_B",
    .great_festival_refuse_reason_c_c = "PHRASE_great_festival_refuse_reason_C_C",
    .great_festival_refuse_reason_c_b = "PHRASE_great_festival_refuse_reason_C_B",
    .great_festival_refuse_reason_p_c = "PHRASE_great_festival_refuse_reason_P_C",
    .great_festival_refuse_reason_p_b = "PHRASE_great_festival_refuse_reason_P_B",
    .great_festival_too_late_reason_c_c = "PHRASE_great_festival_too_late_reason_C_C",
    .great_festival_too_late_reason_c_b = "PHRASE_great_festival_too_late_reason_C_B",
    .great_festival_too_late_reason_p_c = "PHRASE_great_festival_too_late_reason_P_C",
    .great_festival_too_late_reason_p_b = "PHRASE_great_festival_too_late_reason_P_B",
    .great_festival_comply_reason_c_c = "PHRASE_great_festival_comply_reason_C_C",
    .great_festival_comply_reason_c_b = "PHRASE_great_festival_comply_reason_C_B",
    .great_festival_comply_reason_p_c = "PHRASE_great_festival_comply_reason_P_C",
    .great_festival_comply_reason_p_b = "PHRASE_great_festival_comply_reason_P_B",
    .general_request_no_reason_c_c = "PHRASE_general_request_no_reason_C_C",
    .general_request_no_reason_c_b = "PHRASE_general_request_no_reason_C_B",
    .general_request_no_reason_p_c = "PHRASE_general_request_no_reason_P_C",
    .general_request_no_reason_p_b = "PHRASE_general_request_no_reason_P_B",
    .general_request_refuse_reason_c_c = "PHRASE_general_request_refuse_reason_C_C",
    .general_request_refuse_reason_c_b = "PHRASE_general_request_refuse_reason_C_B",
    .general_request_refuse_reason_p_c = "PHRASE_general_request_refuse_reason_P_C",
    .general_request_refuse_reason_p_b = "PHRASE_general_request_refuse_reason_P_B",
    .general_request_too_late_reason_c_c = "PHRASE_general_request_too_late_reason_C_C",
    .general_request_too_late_reason_c_b = "PHRASE_general_request_too_late_reason_C_B",
    .general_request_too_late_reason_p_c = "PHRASE_general_request_too_late_reason_P_C",
    .general_request_too_late_reason_p_b = "PHRASE_general_request_too_late_reason_P_B",
    .general_request_comply_reason_c_c = "PHRASE_general_request_comply_reason_C_C",
    .general_request_comply_reason_c_b = "PHRASE_general_request_comply_reason_C_B",
    .general_request_comply_reason_p_c = "PHRASE_general_request_comply_reason_P_C",
    .general_request_comply_reason_p_b = "PHRASE_general_request_comply_reason_P_B",
    .distant_battle_no_reason_c_c = "PHRASE_distant_battle_no_reason_C_C",
    .distant_battle_no_reason_c_b = "PHRASE_distant_battle_no_reason_C_B",
    .distant_battle_no_reason_p_c = "PHRASE_distant_battle_no_reason_P_C",
    .distant_battle_no_reason_p_b = "PHRASE_distant_battle_no_reason_P_B",
    .distant_battle_lost_battle_reason_c_c = "PHRASE_distant_battle_lost_battle_reason_C_C",
    .distant_battle_lost_battle_reason_c_b = "PHRASE_distant_battle_lost_battle_reason_C_B",
    .distant_battle_lost_battle_reason_p_c = "PHRASE_distant_battle_lost_battle_reason_P_C",
    .distant_battle_lost_battle_reason_p_b = "PHRASE_distant_battle_lost_battle_reason_P_B",
    .distant_battle_refuse_reason_c_c = "PHRASE_distant_battle_refuse_reason_C_C",
    .distant_battle_refuse_reason_c_b = "PHRASE_distant_battle_refuse_reason_C_B",
    .distant_battle_refuse_reason_p_c = "PHRASE_distant_battle_refuse_reason_P_C",
    .distant_battle_refuse_reason_p_b = "PHRASE_distant_battle_refuse_reason_P_B",
    .distant_battle_too_late_reason_c_c = "PHRASE_distant_battle_too_late_reason_C_C",
    .distant_battle_too_late_reason_c_b = "PHRASE_distant_battle_too_late_reason_C_B",
    .distant_battle_too_late_reason_p_c = "PHRASE_distant_battle_too_late_reason_P_C",
    .distant_battle_too_late_reason_p_b = "PHRASE_distant_battle_too_late_reason_P_B",
    .distant_battle_comply_reason_c_c = "PHRASE_distant_battle_comply_reason_C_C",
    .distant_battle_comply_reason_c_b = "PHRASE_distant_battle_comply_reason_C_B",
    .distant_battle_comply_reason_p_c = "PHRASE_distant_battle_comply_reason_P_C",
    .distant_battle_comply_reason_p_b = "PHRASE_distant_battle_comply_reason_P_B",
    .egyptian_city_attacked_no_reason_c_c = "PHRASE_egyptian_city_attacked_no_reason_C_C",
    .egyptian_city_attacked_no_reason_c_b = "PHRASE_egyptian_city_attacked_no_reason_C_B",
    .egyptian_city_attacked_no_reason_p_c = "PHRASE_egyptian_city_attacked_no_reason_P_C",
    .egyptian_city_attacked_no_reason_p_b = "PHRASE_egyptian_city_attacked_no_reason_P_B",
    .egyptian_city_attacked_lost_battle_reason_c_c = "PHRASE_egyptian_city_attacked_lost_battle_reason_C_C",
    .egyptian_city_attacked_lost_battle_reason_c_b = "PHRASE_egyptian_city_attacked_lost_battle_reason_C_B",
    .egyptian_city_attacked_lost_battle_reason_p_c = "PHRASE_egyptian_city_attacked_lost_battle_reason_P_C",
    .egyptian_city_attacked_lost_battle_reason_p_b = "PHRASE_egyptian_city_attacked_lost_battle_reason_P_B",
    .egyptian_city_attacked_refuse_reason_c_c = "PHRASE_egyptian_city_attacked_refuse_reason_C_C",
    .egyptian_city_attacked_refuse_reason_c_b = "PHRASE_egyptian_city_attacked_refuse_reason_C_B",
    .egyptian_city_attacked_refuse_reason_p_c = "PHRASE_egyptian_city_attacked_refuse_reason_P_C",
    .egyptian_city_attacked_refuse_reason_p_b = "PHRASE_egyptian_city_attacked_refuse_reason_P_B",
    .egyptian_city_attacked_too_late_reason_c_c = "PHRASE_egyptian_city_attacked_too_late_reason_C_C",
    .egyptian_city_attacked_too_late_reason_c_b = "PHRASE_egyptian_city_attacked_too_late_reason_C_B",
    .egyptian_city_attacked_too_late_reason_p_c = "PHRASE_egyptian_city_attacked_too_late_reason_P_C",
    .egyptian_city_attacked_too_late_reason_p_b = "PHRASE_egyptian_city_attacked_too_late_reason_P_B",
    .egyptian_city_attacked_comply_reason_c_c = "PHRASE_egyptian_city_attacked_comply_reason_C_C",
    .egyptian_city_attacked_comply_reason_c_b = "PHRASE_egyptian_city_attacked_comply_reason_C_B",
    .egyptian_city_attacked_comply_reason_p_c = "PHRASE_egyptian_city_attacked_comply_reason_P_C",
    .egyptian_city_attacked_comply_reason_p_b = "PHRASE_egyptian_city_attacked_comply_reason_P_B",
    .bedouin_attacks_you_disembarked_alert = "PHRASE_bedouin_attacks_you_disembarked_alert",
    .foreign_army_attacks_you_disembarked_alert = "PHRASE_foreign_army_attacks_you_disembarked_alert",
    .eg_city_attacks_you_disembarked_alert = "PHRASE_eg_city_attacks_you_disembarked_alert",
    .pharaoh_attacks_you_disembarked_alert = "PHRASE_pharaoh_attacks_you_disembarked_alert",
    .gift_no_reason_c_a = "PHRASE_gift_no_reason_C_A",
    .gift_no_reason_p_a = "PHRASE_gift_no_reason_P_A",
    .gift_refused_reason_c_a = "PHRASE_gift_refused_reason_C_A",
    .gift_refused_reason_p_a = "PHRASE_gift_refused_reason_P_A",
    .gift_forfeited_reason_c_a = "PHRASE_gift_forfeited_reason_C_A",
    .gift_forfeited_reason_p_a = "PHRASE_gift_forfeited_reason_P_A",
    .gift_accepted_reason_c_a = "PHRASE_gift_accepted_reason_C_A",
    .gift_accepted_reason_p_a = "PHRASE_gift_accepted_reason_P_A",
    .gift_refused_c = "PHRASE_gift_refused_C",
    .gift_refused_p = "PHRASE_gift_refused_P",
    .gift_postponed_c = "PHRASE_gift_postponed_C",
    .gift_postponed_p = "PHRASE_gift_postponed_P",
    .gift_cash_accepted_c = "PHRASE_gift_cash_accepted_C",
    .gift_cash_accepted_p = "PHRASE_gift_cash_accepted_P",
    .gift_accepted_c = "PHRASE_gift_accepted_C",
    .gift_accepted_p = "PHRASE_gift_accepted_P",
    .gift_forfeited_c = "PHRASE_gift_forfeited_C",
    .gift_forfeited_p = "PHRASE_gift_forfeited_P",
    .gift_last_chance_c = "PHRASE_gift_last_chance_C",
    .gift_last_chance_p = "PHRASE_gift_last_chance_P",
    .gift_insufficient_space_c = "PHRASE_gift_insufficient_space_C",
    .gift_insufficient_space_p = "PHRASE_gift_insufficient_space_P",
    .gift_partial_space_c = "PHRASE_gift_partial_space_C",
    .gift_partial_space_p = "PHRASE_gift_partial_space_P",
    .gift_cash_granted_c = "PHRASE_gift_cash_granted_C",
    .gift_cash_granted_p = "PHRASE_gift_cash_granted_P",
    .gift_granted_c = "PHRASE_gift_granted_C",
    .gift_granted_p = "PHRASE_gift_granted_P",
    .gift_title_c = "PHRASE_gift_title_C",
    .gift_title_p = "PHRASE_gift_title_P",
    .bedouin_attacks_you_no_reason_a = "PHRASE_bedouin_attacks_you_no_reason_A",
    .bedouin_attacks_you_reason_a = "PHRASE_bedouin_attacks_you_reason_A",
    .bedouin_attacks_you_city_attacked_alert = "PHRASE_bedouin_attacks_you_city_attacked_alert",
    .bedouin_attacks_you_1month_warning = "PHRASE_bedouin_attacks_you_1month_warning",
    .bedouin_attacks_you_6month_warning = "PHRASE_bedouin_attacks_you_6month_warning",
    .bedouin_attacks_you_1year_reminder = "PHRASE_bedouin_attacks_you_1year_reminder",
    .bedouin_attacks_you_2year_reminder = "PHRASE_bedouin_attacks_you_2year_reminder",
    .bedouin_attacks_you_initial_announcement = "PHRASE_bedouin_attacks_you_initial_announcement",
    .bedouin_attacks_you_title = "PHRASE_bedouin_attacks_you_title",
    .perfect_flood_no_reason_a = "PHRASE_perfect_flood_no_reason_A",
    .perfect_flood_reason_a = "PHRASE_perfect_flood_reason_A",
    .perfect_flood_initial_announcement = "PHRASE_perfect_flood_initial_announcement",
    .perfect_flood_title = "PHRASE_perfect_flood_title",
    .flood_fails_no_reason_a = "PHRASE_flood_fails_no_reason_A",
    .flood_fails_reason_a = "PHRASE_flood_fails_reason_A",
    .flood_fails_initial_announcement = "PHRASE_flood_fails_initial_announcement",
    .flood_fails_title = "PHRASE_flood_fails_title",
    .landslide_no_reason_a = "PHRASE_landslide_no_reason_A",
    .landslide_reason_a = "PHRASE_landslide_reason_A",
    .landslide_initial_announcement = "PHRASE_landslide_initial_announcement",
    .landslide_title = "PHRASE_landslide_title",
    .goldmine_cavein_no_reason_a = "PHRASE_goldmine_cavein_no_reason_A",
    .goldmine_cavein_reason_a = "PHRASE_goldmine_cavein_reason_A",
    .goldmine_cavein_initial_announcement = "PHRASE_goldmine_cavein_initial_announcement",
    .goldmine_cavein_title = "PHRASE_goldmine_cavein_title",
    .bad_water_no_reason_a = "PHRASE_bad_water_no_reason_A",
    .bad_water_reason_a = "PHRASE_bad_water_reason_A",
    .bad_water_initial_announcement = "PHRASE_bad_water_initial_announcement",
    .bad_water_title = "PHRASE_bad_water_title",
    .wage_change_no_reason_d_a = "PHRASE_wage_change_no_reason_D_A",
    .wage_change_reason_d_a = "PHRASE_wage_change_reason_D_A",
    .wage_change_initial_announcement_d = "PHRASE_wage_change_initial_announcement_D",
    .wage_change_title_d = "PHRASE_wage_change_title_D",
    .wage_change_no_reason_i_a = "PHRASE_wage_change_no_reason_I_A",
    .wage_change_reason_i_a = "PHRASE_wage_change_reason_I_A",
    .wage_change_initial_announcement_i = "PHRASE_wage_change_initial_announcement_I",
    .wage_change_title_i = "PHRASE_wage_change_title_I",
    .sandstorm_no_reason_a = "PHRASE_sandstorm_no_reason_A",
    .sandstorm_reason_a = "PHRASE_sandstorm_reason_A",
    .sandstorm_initial_announcement = "PHRASE_sandstorm_initial_announcement",
    .sandstorm_title = "PHRASE_sandstorm_title",
    .stormy_seas_no_reason_a = "PHRASE_stormy_seas_no_reason_A",
    .stormy_seas_reason_a = "PHRASE_stormy_seas_reason_A",
    .stormy_seas_initial_announcement = "PHRASE_stormy_seas_initial_announcement",
    .stormy_seas_title = "PHRASE_stormy_seas_title",
    .earthquake_no_reason_a = "PHRASE_earthquake_no_reason_A",
    .earthquake_reason_a = "PHRASE_earthquake_reason_A",
    .earthquake_initial_announcement = "PHRASE_earthquake_initial_announcement",
    .earthquake_title = "PHRASE_earthquake_title",
    .demand_change_no_reason_d_a = "PHRASE_demand_change_no_reason_D_A",
    .demand_change_reason_d_a = "PHRASE_demand_change_reason_D_A",
    .demand_change_initial_announcement_d = "PHRASE_demand_change_initial_announcement_D",
    .demand_change_title_d = "PHRASE_demand_change_title_D",
    .demand_change_no_reason_i_a = "PHRASE_demand_change_no_reason_I_A",
    .demand_change_reason_i_a = "PHRASE_demand_change_reason_I_A",
    .demand_change_initial_announcement_i = "PHRASE_demand_change_initial_announcement_I",
    .demand_change_title_i = "PHRASE_demand_change_title_I",
    .price_change_no_reason_d_a = "PHRASE_price_change_no_reason_D_A",
    .price_change_reason_d_a = "PHRASE_price_change_reason_D_A",
    .price_change_initial_announcement_d = "PHRASE_price_change_initial_announcement_D",
    .price_change_title_d = "PHRASE_price_change_title_D",
    .price_change_no_reason_i_a = "PHRASE_price_change_no_reason_I_A",
    .price_change_reason_i_a = "PHRASE_price_change_reason_I_A",
    .price_change_initial_announcement_i = "PHRASE_price_change_initial_announcement_I",
    .price_change_title_i = "PHRASE_price_change_title_I",
    .rating_change_no_reason_d_a = "PHRASE_rating_change_no_reason_D_A",
    .rating_change_reason_d_a = "PHRASE_rating_change_reason_D_A",
    .rating_change_initial_announcement_d = "PHRASE_rating_change_initial_announcement_D",
    .rating_change_title_d = "PHRASE_rating_change_title_D",
    .rating_change_no_reason_i_a = "PHRASE_rating_change_no_reason_I_A",
    .rating_change_reason_i_a = "PHRASE_rating_change_reason_I_A",
    .rating_change_initial_announcement_i = "PHRASE_rating_change_initial_announcement_I",
    .rating_change_title_i = "PHRASE_rating_change_title_I",
    .foreign_army_attacks_you_no_reason_a = "PHRASE_foreign_army_attacks_you_no_reason_A",
    .foreign_army_attacks_you_reason_a = "PHRASE_foreign_army_attacks_you_reason_A",
    .foreign_army_attacks_you_city_attacked_alert = "PHRASE_foreign_army_attacks_you_city_attacked_alert",
    .foreign_army_attacks_you_1month_warning = "PHRASE_foreign_army_attacks_you_1month_warning",
    .foreign_army_attacks_you_6month_warning = "PHRASE_foreign_army_attacks_you_6month_warning",
    .foreign_army_attacks_you_1year_reminder = "PHRASE_foreign_army_attacks_you_1year_reminder",
    .foreign_army_attacks_you_2year_reminder = "PHRASE_foreign_army_attacks_you_2year_reminder",
    .foreign_army_attacks_you_initial_announcement = "PHRASE_foreign_army_attacks_you_initial_announcement",
    .foreign_army_attacks_you_title = "PHRASE_foreign_army_attacks_you_title",
    .eg_city_attacks_you_because_of_low_favor = "PHRASE_eg_city_attacks_you_because_of_low_favor",
    .eg_city_attacks_you_no_reason_a = "PHRASE_eg_city_attacks_you_no_reason_A",
    .eg_city_attacks_you_reason_a = "PHRASE_eg_city_attacks_you_reason_A",
    .eg_city_attacks_you_city_attacked_alert = "PHRASE_eg_city_attacks_you_city_attacked_alert",
    .eg_city_attacks_you_1month_warning = "PHRASE_eg_city_attacks_you_1month_warning",
    .eg_city_attacks_you_6month_warning = "PHRASE_eg_city_attacks_you_6month_warning",
    .eg_city_attacks_you_1year_reminder = "PHRASE_eg_city_attacks_you_1year_reminder",
    .eg_city_attacks_you_2year_reminder = "PHRASE_eg_city_attacks_you_2year_reminder",
    .eg_city_attacks_you_initial_announcement = "PHRASE_eg_city_attacks_you_initial_announcement",
    .eg_city_attacks_you_title = "PHRASE_eg_city_attacks_you_title",
    .pharaoh_attacks_you_because_of_low_favor = "PHRASE_pharaoh_attacks_you_because_of_low_favor",
    .pharaoh_attacks_you_no_reason_a = "PHRASE_pharaoh_attacks_you_no_reason_A",
    .pharaoh_attacks_you_reason_a = "PHRASE_pharaoh_attacks_you_reason_A",
    .pharaoh_attacks_you_city_attacked_alert = "PHRASE_pharaoh_attacks_you_city_attacked_alert",
    .pharaoh_attacks_you_1month_warning = "PHRASE_pharaoh_attacks_you_1month_warning",
    .pharaoh_attacks_you_6month_warning = "PHRASE_pharaoh_attacks_you_6month_warning",
    .pharaoh_attacks_you_1year_reminder = "PHRASE_pharaoh_attacks_you_1year_reminder",
    .pharaoh_attacks_you_2year_reminder = "PHRASE_pharaoh_attacks_you_2year_reminder",
    .pharaoh_attacks_you_initial_announcement = "PHRASE_pharaoh_attacks_you_initial_announcement",
    .pharaoh_attacks_you_title = "PHRASE_pharaoh_attacks_you_title",
    .acknowledgement_no_reason_a = "PHRASE_acknowledgement_no_reason_A",
    .acknowledgement_reason_a = "PHRASE_acknowledgement_reason_A",
    .acknowledgement_initial_announcement = "PHRASE_acknowledgement_initial_announcement",
    .acknowledgement_title = "PHRASE_acknowledgement_title",
    .battle_lost_no_reason_a = "PHRASE_battle_lost_no_reason_A",
    .battle_lost_reason_a = "PHRASE_battle_lost_reason_A",
    .battle_lost_initial_announcement = "PHRASE_battle_lost_initial_announcement",
    .battle_lost_title = "PHRASE_battle_lost_title",
    .battle_won_no_reason_a = "PHRASE_battle_won_no_reason_A",
    .battle_won_reason_a = "PHRASE_battle_won_reason_A",
    .battle_won_initial_announcement = "PHRASE_battle_won_initial_announcement",
    .battle_won_title = "PHRASE_battle_won_title",
    .eg_city_saved_no_reason_a = "PHRASE_eg_city_saved_no_reason_A",
    .eg_city_saved_reason_a = "PHRASE_eg_city_saved_reason_A",
    .eg_city_saved_initial_announcement = "PHRASE_eg_city_saved_initial_announcement",
    .eg_city_saved_title = "PHRASE_eg_city_saved_title",
    .siege_no_reason_a = "PHRASE_siege_no_reason_A",
    .siege_reason_a = "PHRASE_siege_reason_A",
    .siege_initial_announcement = "PHRASE_siege_initial_announcement",
    .siege_title = "PHRASE_siege_title",
    .route_closed_no_reason_a = "PHRASE_route_closed_no_reason_A",
    .route_closed_reason_a = "PHRASE_route_closed_reason_A",
    .route_closed_initial_announcement = "PHRASE_route_closed_initial_announcement",
    .route_closed_title = "PHRASE_route_closed_title",
    .route_opened_no_reason_a = "PHRASE_route_opened_no_reason_A",
    .route_opened_reason_a = "PHRASE_route_opened_reason_A",
    .route_opened_initial_announcement = "PHRASE_route_opened_initial_announcement",
    .route_opened_title = "PHRASE_route_opened_title",
    .foreign_city_conquered_no_reason_a = "PHRASE_foreign_city_conquered_no_reason_A",
    .foreign_city_conquered_reason_a = "PHRASE_foreign_city_conquered_reason_A",
    .foreign_city_conquered_initial_announcement = "PHRASE_foreign_city_conquered_initial_announcement",
    .foreign_city_conquered_title = "PHRASE_foreign_city_conquered_title",
    .eg_city_falls_no_reason_a = "PHRASE_eg_city_falls_no_reason_A",
    .eg_city_falls_reason_a = "PHRASE_eg_city_falls_reason_A",
    .eg_city_falls_initial_announcement = "PHRASE_eg_city_falls_initial_announcement",
    .eg_city_falls_title = "PHRASE_eg_city_falls_title",
    .threat_no_reason_c_a = "PHRASE_threat_no_reason_C_A",
    .threat_no_reason_p_a = "PHRASE_threat_no_reason_P_A",
    .threat_refuse_reason_c_a = "PHRASE_threat_refuse_reason_C_A",
    .threat_refuse_reason_p_a = "PHRASE_threat_refuse_reason_P_A",
    .threat_too_late_reason_c_a = "PHRASE_threat_too_late_reason_C_A",
    .threat_too_late_reason_p_a = "PHRASE_threat_too_late_reason_P_A",
    .threat_comply_reason_c_a = "PHRASE_threat_comply_reason_C_A",
    .threat_comply_reason_p_a = "PHRASE_threat_comply_reason_P_A",
    .threat_warning_c = "PHRASE_threat_warning_C",
    .threat_warning_p = "PHRASE_threat_warning_P",
    .threat_overdue_c = "PHRASE_threat_overdue_C",
    .threat_overdue_p = "PHRASE_threat_overdue_P",
    .threat_reminder_c = "PHRASE_threat_reminder_C",
    .threat_reminder_p = "PHRASE_threat_reminder_P",
    .threat_initial_announcement_c = "PHRASE_threat_initial_announcement_C",
    .threat_initial_announcement_p = "PHRASE_threat_initial_announcement_P",
    .threat_title_c = "PHRASE_threat_title_C",
    .threat_title_p = "PHRASE_threat_title_P",
    .famine_no_reason_c_a = "PHRASE_famine_no_reason_C_A",
    .famine_no_reason_p_a = "PHRASE_famine_no_reason_P_A",
    .famine_refuse_reason_c_a = "PHRASE_famine_refuse_reason_C_A",
    .famine_refuse_reason_p_a = "PHRASE_famine_refuse_reason_P_A",
    .famine_too_late_reason_c_a = "PHRASE_famine_too_late_reason_C_A",
    .famine_too_late_reason_p_a = "PHRASE_famine_too_late_reason_P_A",
    .famine_comply_reason_c_a = "PHRASE_famine_comply_reason_C_A",
    .famine_comply_reason_p_a = "PHRASE_famine_comply_reason_P_A",
    .famine_warning_c = "PHRASE_famine_warning_C",
    .famine_warning_p = "PHRASE_famine_warning_P",
    .famine_overdue_c = "PHRASE_famine_overdue_C",
    .famine_overdue_p = "PHRASE_famine_overdue_P",
    .famine_reminder_c = "PHRASE_famine_reminder_C",
    .famine_reminder_p = "PHRASE_famine_reminder_P",
    .famine_initial_announcement_c = "PHRASE_famine_initial_announcement_C",
    .famine_initial_announcement_p = "PHRASE_famine_initial_announcement_P",
    .famine_title_c = "PHRASE_famine_title_C",
    .famine_title_p = "PHRASE_famine_title_P",
    .project_no_reason_c_a = "PHRASE_project_no_reason_C_A",
    .project_no_reason_p_a = "PHRASE_project_no_reason_P_A",
    .project_refuse_reason_c_a = "PHRASE_project_refuse_reason_C_A",
    .project_refuse_reason_p_a = "PHRASE_project_refuse_reason_P_A",
    .project_too_late_reason_c_a = "PHRASE_project_too_late_reason_C_A",
    .project_too_late_reason_p_a = "PHRASE_project_too_late_reason_P_A",
    .project_comply_reason_c_a = "PHRASE_project_comply_reason_C_A",
    .project_comply_reason_p_a = "PHRASE_project_comply_reason_P_A",
    .project_warning_c = "PHRASE_project_warning_C",
    .project_warning_p = "PHRASE_project_warning_P",
    .project_overdue_c = "PHRASE_project_overdue_C",
    .project_overdue_p = "PHRASE_project_overdue_P",
    .project_reminder_c = "PHRASE_project_reminder_C",
    .project_reminder_p = "PHRASE_project_reminder_P",
    .project_initial_announcement_c = "PHRASE_project_initial_announcement_C",
    .project_initial_announcement_p = "PHRASE_project_initial_announcement_P",
    .project_title_c = "PHRASE_project_title_C",
    .project_title_p = "PHRASE_project_title_P",
    .great_festival_no_reason_c_a = "PHRASE_great_festival_no_reason_C_A",
    .great_festival_no_reason_p_a = "PHRASE_great_festival_no_reason_P_A",
    .great_festival_refuse_reason_c_a = "PHRASE_great_festival_refuse_reason_C_A",
    .great_festival_refuse_reason_p_a = "PHRASE_great_festival_refuse_reason_P_A",
    .great_festival_too_late_reason_c_a = "PHRASE_great_festival_too_late_reason_C_A",
    .great_festival_too_late_reason_p_a = "PHRASE_great_festival_too_late_reason_P_A",
    .great_festival_comply_reason_c_a = "PHRASE_great_festival_comply_reason_C_A",
    .great_festival_comply_reason_p_a = "PHRASE_great_festival_comply_reason_P_A",
    .great_festival_warning_c = "PHRASE_great_festival_warning_C",
    .great_festival_warning_p = "PHRASE_great_festival_warning_P",
    .great_festival_overdue_c = "PHRASE_great_festival_overdue_C",
    .great_festival_overdue_p = "PHRASE_great_festival_overdue_P",
    .great_festival_reminder_c = "PHRASE_great_festival_reminder_C",
    .great_festival_reminder_p = "PHRASE_great_festival_reminder_P",
    .great_festival_initial_announcement_c = "PHRASE_great_festival_initial_announcement_C",
    .great_festival_initial_announcement_p = "PHRASE_great_festival_initial_announcement_P",
    .great_festival_title_c = "PHRASE_great_festival_title_C",
    .great_festival_title_p = "PHRASE_great_festival_title_P",
    .general_request_no_reason_c_a = "PHRASE_general_request_no_reason_C_A",
    .general_request_no_reason_p_a = "PHRASE_general_request_no_reason_P_A",
    .general_request_refuse_reason_c_a = "PHRASE_general_request_refuse_reason_C_A",
    .general_request_refuse_reason_p_a = "PHRASE_general_request_refuse_reason_P_A",
    .general_request_too_late_reason_c_a = "PHRASE_general_request_too_late_reason_C_A",
    .general_request_too_late_reason_p_a = "PHRASE_general_request_too_late_reason_P_A",
    .general_request_comply_reason_c_a = "PHRASE_general_request_comply_reason_C_A",
    .general_request_comply_reason_p_a = "PHRASE_general_request_comply_reason_P_A",
    .general_request_warning_c = "PHRASE_general_request_warning_C",
    .general_request_warning_p = "PHRASE_general_request_warning_P",
    .general_request_overdue_c = "PHRASE_general_request_overdue_C",
    .general_request_overdue_p = "PHRASE_general_request_overdue_P",
    .general_request_reminder_c = "PHRASE_general_request_reminder_C",
    .general_request_reminder_p = "PHRASE_general_request_reminder_P",
    .general_request_initial_announcement_c = "PHRASE_general_request_initial_announcement_C",
    .general_request_initial_announcement_p = "PHRASE_general_request_initial_announcement_P",
    .general_request_title_c = "PHRASE_general_request_title_C",
    .general_request_title_p = "PHRASE_general_request_title_P",
    .distant_battle_no_reason_c_a = "PHRASE_distant_battle_no_reason_C_A",
    .distant_battle_no_reason_p_a = "PHRASE_distant_battle_no_reason_P_A",
    .distant_battle_lost_battle_reason_c_a = "PHRASE_distant_battle_lost_battle_reason_C_A",
    .distant_battle_lost_battle_reason_p_a = "PHRASE_distant_battle_lost_battle_reason_P_A",
    .distant_battle_refuse_reason_c_a = "PHRASE_distant_battle_refuse_reason_C_A",
    .distant_battle_refuse_reason_p_a = "PHRASE_distant_battle_refuse_reason_P_A",
    .distant_battle_too_late_reason_c_a = "PHRASE_distant_battle_too_late_reason_C_A",
    .distant_battle_too_late_reason_p_a = "PHRASE_distant_battle_too_late_reason_P_A",
    .distant_battle_comply_reason_c_a = "PHRASE_distant_battle_comply_reason_C_A",
    .distant_battle_comply_reason_p_a = "PHRASE_distant_battle_comply_reason_P_A",
    .distant_battle_last_reminder_c = "PHRASE_distant_battle_last_reminder_C",
    .distant_battle_last_reminder_p = "PHRASE_distant_battle_last_reminder_P",
    .distant_battle_first_reminder_c = "PHRASE_distant_battle_first_reminder_C",
    .distant_battle_first_reminder_p = "PHRASE_distant_battle_first_reminder_P",
    .distant_battle_initial_announcement_c = "PHRASE_distant_battle_initial_announcement_C",
    .distant_battle_initial_announcement_p = "PHRASE_distant_battle_initial_announcement_P",
    .distant_battle_title_c = "PHRASE_distant_battle_title_C",
    .distant_battle_title_p = "PHRASE_distant_battle_title_P",
    .egyptian_city_attacked_no_reason_c_a = "PHRASE_egyptian_city_attacked_no_reason_C_A",
    .egyptian_city_attacked_no_reason_p_a = "PHRASE_egyptian_city_attacked_no_reason_P_A",
    .egyptian_city_attacked_lost_battle_reason_c_a = "PHRASE_egyptian_city_attacked_lost_battle_reason_C_A",
    .egyptian_city_attacked_lost_battle_reason_p_a = "PHRASE_egyptian_city_attacked_lost_battle_reason_P_A",
    .egyptian_city_attacked_refuse_reason_c_a = "PHRASE_egyptian_city_attacked_refuse_reason_C_A",
    .egyptian_city_attacked_refuse_reason_p_a = "PHRASE_egyptian_city_attacked_refuse_reason_P_A",
    .egyptian_city_attacked_too_late_reason_c_a = "PHRASE_egyptian_city_attacked_too_late_reason_C_A",
    .egyptian_city_attacked_too_late_reason_p_a = "PHRASE_egyptian_city_attacked_too_late_reason_P_A",
    .egyptian_city_attacked_comply_reason_c_a = "PHRASE_egyptian_city_attacked_comply_reason_C_A",
    .egyptian_city_attacked_comply_reason_p_a = "PHRASE_egyptian_city_attacked_comply_reason_P_A",
    .egyptian_city_attacked_last_reminder_c = "PHRASE_egyptian_city_attacked_last_reminder_C",
    .egyptian_city_attacked_last_reminder_p = "PHRASE_egyptian_city_attacked_last_reminder_P",
    .egyptian_city_attacked_first_reminder_c = "PHRASE_egyptian_city_attacked_first_reminder_C",
    .egyptian_city_attacked_first_reminder_p = "PHRASE_egyptian_city_attacked_first_reminder_P",
    .egyptian_city_attacked_initial_announcement_c = "PHRASE_egyptian_city_attacked_initial_announcement_C",
    .egyptian_city_attacked_initial_announcement_p = "PHRASE_egyptian_city_attacked_initial_announcement_P",
    .egyptian_city_attacked_title_c = "PHRASE_egyptian_city_attacked_title_C",
    .egyptian_city_attacked_title_p = "PHRASE_egyptian_city_attacked_title_P"
};

struct message_token_name_pool {
    char quantity_total_secondary[28];
    char quantity_total_main[20];
    char number_loads_remainder_secondary[36];
    char number_loads_remainder_main[28];
    char number_loads_current_course_secondary[40];
    char number_loads_current_course_main[36];
    char quantity_needed_remainder_secondary[36];
    char quantity_needed_remainder_main[32];
    char quantity_needed_current_course_secondary[44];
    char quantity_needed_current_course_main[36];
    char number_courses_complete[24];
    char percent_complete[20];
    char monument_name[16];
    char god[4];
    char a_foreign_army[16];
    char amount_granted[16];
    char pharaoh_name[16];
    char reason_phrase[16];
    char time_until_attack[20];
    char travel_time[12];
    char time_allotted[16];
    char item[8];
    char amount[8];
    char city_name[12];
    char player_name[12];
    char greeting[12];
};

message_token_name_pool message_token_names = {
    .quantity_total_secondary = "quantity_total_secondary",
    .quantity_total_main = "quantity_total_main",
    .number_loads_remainder_secondary = "number_loads_remainder_secondary",
    .number_loads_remainder_main = "number_loads_remainder_main",
    .number_loads_current_course_secondary = "number_loads_current_course_secondary",
    .number_loads_current_course_main = "number_loads_current_course_main",
    .quantity_needed_remainder_secondary = "quantity_needed_remainder_secondary",
    .quantity_needed_remainder_main = "quantity_needed_remainder_main",
    .quantity_needed_current_course_secondary = "quantity_needed_current_course_secondary",
    .quantity_needed_current_course_main = "quantity_needed_current_course_main",
    .number_courses_complete = "number_courses_complete",
    .percent_complete = "percent_complete",
    .monument_name = "monument_name",
    .god = "god",
    .a_foreign_army = "a_foreign_army",
    .amount_granted = "amount_granted",
    .pharaoh_name = "pharaoh_name",
    .reason_phrase = "reason_phrase",
    .time_until_attack = "time_until_attack",
    .travel_time = "travel_time",
    .time_allotted = "time_allotted",
    .item = "item",
    .amount = "amount",
    .city_name = "city_name",
    .player_name = "player_name",
    .greeting = "greeting"
};

message_token_table message_tokens = { {
        { 0, message_token_names.greeting, (message_token_function) message_token_player_rank },
        { 1, message_token_names.player_name, (message_token_function) message_token_player_name },
        { 2, message_token_names.city_name, (message_token_function) message_token_city_name },
        { 3, message_token_names.amount, (message_token_function) message_token_amount },
        { 4, message_token_names.item, (message_token_function) message_token_resource_name },
        { 5, message_token_names.time_allotted, (message_token_function) message_token_time_allotted },
        { 6, message_token_names.travel_time, (message_token_function) message_token_travel_time },
        { 7, message_token_names.time_until_attack, (message_token_function) message_token_time_until_attack },
        { 8, message_token_names.reason_phrase, (message_token_function) message_token_reason_phrase },
        { 9, message_token_names.pharaoh_name, (message_token_function) message_token_pharaoh_name },
        { 10, message_token_names.amount_granted, (message_token_function) message_token_request_amount_text },
        { 11, message_token_names.a_foreign_army, (message_token_function) message_token_foreign_army },
        { 12, message_token_names.god, (message_token_function) message_token_god },
        { 13, message_token_names.monument_name, (message_token_function) message_token_monument_name },
        { 14, message_token_names.percent_complete, (message_token_function) message_token_percent_complete },
        { 15, message_token_names.number_courses_complete, (message_token_function) message_token_courses_complete },
        { 16, message_token_names.quantity_needed_current_course_main, (message_token_function) message_token_needed_current_main },
        { 17, message_token_names.quantity_needed_current_course_secondary, (message_token_function) message_token_needed_current_secondary },
        { 18, message_token_names.quantity_needed_remainder_main, (message_token_function) message_token_needed_remainder_main },
        { 19, message_token_names.quantity_needed_remainder_secondary, (message_token_function) message_token_needed_remainder_secondary },
        { 20, message_token_names.number_loads_current_course_main, (message_token_function) message_token_loads_current_main },
        { 21, message_token_names.number_loads_current_course_secondary, (message_token_function) message_token_loads_current_secondary },
        { 22, message_token_names.number_loads_remainder_main, (message_token_function) message_token_loads_remainder_main },
        { 23, message_token_names.number_loads_remainder_secondary, (message_token_function) message_token_loads_remainder_secondary },
        { 24, message_token_names.quantity_total_main, (message_token_function) message_token_total_main },
        { 25, message_token_names.quantity_total_secondary, (message_token_function) message_token_total_secondary },
        { -1, 0, 0 } },
    0 };

text_key event_text_keys[602] = {
    { 0, event_text_key_names.egyptian_city_attacked_title_p },
    { 1, event_text_key_names.egyptian_city_attacked_title_c },
    { 2, event_text_key_names.egyptian_city_attacked_initial_announcement_p },
    { 3, event_text_key_names.egyptian_city_attacked_initial_announcement_c },
    { 4, event_text_key_names.egyptian_city_attacked_first_reminder_p },
    { 5, event_text_key_names.egyptian_city_attacked_first_reminder_c },
    { 6, event_text_key_names.egyptian_city_attacked_last_reminder_p },
    { 7, event_text_key_names.egyptian_city_attacked_last_reminder_c },
    { 8, event_text_key_names.egyptian_city_attacked_comply_reason_p_a },
    { 9, event_text_key_names.egyptian_city_attacked_comply_reason_c_a },
    { 10, event_text_key_names.egyptian_city_attacked_too_late_reason_p_a },
    { 11, event_text_key_names.egyptian_city_attacked_too_late_reason_c_a },
    { 12, event_text_key_names.egyptian_city_attacked_refuse_reason_p_a },
    { 13, event_text_key_names.egyptian_city_attacked_refuse_reason_c_a },
    { 14, event_text_key_names.egyptian_city_attacked_lost_battle_reason_p_a },
    { 15, event_text_key_names.egyptian_city_attacked_lost_battle_reason_c_a },
    { 16, event_text_key_names.egyptian_city_attacked_no_reason_p_a },
    { 17, event_text_key_names.egyptian_city_attacked_no_reason_c_a },
    { 18, event_text_key_names.distant_battle_title_p },
    { 19, event_text_key_names.distant_battle_title_c },
    { 20, event_text_key_names.distant_battle_initial_announcement_p },
    { 21, event_text_key_names.distant_battle_initial_announcement_c },
    { 22, event_text_key_names.distant_battle_first_reminder_p },
    { 23, event_text_key_names.distant_battle_first_reminder_c },
    { 24, event_text_key_names.distant_battle_last_reminder_p },
    { 25, event_text_key_names.distant_battle_last_reminder_c },
    { 26, event_text_key_names.distant_battle_comply_reason_p_a },
    { 27, event_text_key_names.distant_battle_comply_reason_c_a },
    { 28, event_text_key_names.distant_battle_too_late_reason_p_a },
    { 29, event_text_key_names.distant_battle_too_late_reason_c_a },
    { 30, event_text_key_names.distant_battle_refuse_reason_p_a },
    { 31, event_text_key_names.distant_battle_refuse_reason_c_a },
    { 32, event_text_key_names.distant_battle_lost_battle_reason_p_a },
    { 33, event_text_key_names.distant_battle_lost_battle_reason_c_a },
    { 34, event_text_key_names.distant_battle_no_reason_p_a },
    { 35, event_text_key_names.distant_battle_no_reason_c_a },
    { 36, event_text_key_names.general_request_title_p },
    { 37, event_text_key_names.general_request_title_c },
    { 38, event_text_key_names.general_request_initial_announcement_p },
    { 39, event_text_key_names.general_request_initial_announcement_c },
    { 40, event_text_key_names.general_request_reminder_p },
    { 41, event_text_key_names.general_request_reminder_c },
    { 42, event_text_key_names.general_request_overdue_p },
    { 43, event_text_key_names.general_request_overdue_c },
    { 44, event_text_key_names.general_request_warning_p },
    { 45, event_text_key_names.general_request_warning_c },
    { 46, event_text_key_names.general_request_comply_reason_p_a },
    { 47, event_text_key_names.general_request_comply_reason_c_a },
    { 48, event_text_key_names.general_request_too_late_reason_p_a },
    { 49, event_text_key_names.general_request_too_late_reason_c_a },
    { 50, event_text_key_names.general_request_refuse_reason_p_a },
    { 51, event_text_key_names.general_request_refuse_reason_c_a },
    { 52, event_text_key_names.general_request_no_reason_p_a },
    { 53, event_text_key_names.general_request_no_reason_c_a },
    { 54, event_text_key_names.great_festival_title_p },
    { 55, event_text_key_names.great_festival_title_c },
    { 56, event_text_key_names.great_festival_initial_announcement_p },
    { 57, event_text_key_names.great_festival_initial_announcement_c },
    { 58, event_text_key_names.great_festival_reminder_p },
    { 59, event_text_key_names.great_festival_reminder_c },
    { 60, event_text_key_names.great_festival_overdue_p },
    { 61, event_text_key_names.great_festival_overdue_c },
    { 62, event_text_key_names.great_festival_warning_p },
    { 63, event_text_key_names.great_festival_warning_c },
    { 64, event_text_key_names.great_festival_comply_reason_p_a },
    { 65, event_text_key_names.great_festival_comply_reason_c_a },
    { 66, event_text_key_names.great_festival_too_late_reason_p_a },
    { 67, event_text_key_names.great_festival_too_late_reason_c_a },
    { 68, event_text_key_names.great_festival_refuse_reason_p_a },
    { 69, event_text_key_names.great_festival_refuse_reason_c_a },
    { 70, event_text_key_names.great_festival_no_reason_p_a },
    { 71, event_text_key_names.great_festival_no_reason_c_a },
    { 72, event_text_key_names.project_title_p },
    { 73, event_text_key_names.project_title_c },
    { 74, event_text_key_names.project_initial_announcement_p },
    { 75, event_text_key_names.project_initial_announcement_c },
    { 76, event_text_key_names.project_reminder_p },
    { 77, event_text_key_names.project_reminder_c },
    { 78, event_text_key_names.project_overdue_p },
    { 79, event_text_key_names.project_overdue_c },
    { 80, event_text_key_names.project_warning_p },
    { 81, event_text_key_names.project_warning_c },
    { 82, event_text_key_names.project_comply_reason_p_a },
    { 83, event_text_key_names.project_comply_reason_c_a },
    { 84, event_text_key_names.project_too_late_reason_p_a },
    { 85, event_text_key_names.project_too_late_reason_c_a },
    { 86, event_text_key_names.project_refuse_reason_p_a },
    { 87, event_text_key_names.project_refuse_reason_c_a },
    { 88, event_text_key_names.project_no_reason_p_a },
    { 89, event_text_key_names.project_no_reason_c_a },
    { 90, event_text_key_names.famine_title_p },
    { 91, event_text_key_names.famine_title_c },
    { 92, event_text_key_names.famine_initial_announcement_p },
    { 93, event_text_key_names.famine_initial_announcement_c },
    { 94, event_text_key_names.famine_reminder_p },
    { 95, event_text_key_names.famine_reminder_c },
    { 96, event_text_key_names.famine_overdue_p },
    { 97, event_text_key_names.famine_overdue_c },
    { 98, event_text_key_names.famine_warning_p },
    { 99, event_text_key_names.famine_warning_c },
    { 100, event_text_key_names.famine_comply_reason_p_a },
    { 101, event_text_key_names.famine_comply_reason_c_a },
    { 102, event_text_key_names.famine_too_late_reason_p_a },
    { 103, event_text_key_names.famine_too_late_reason_c_a },
    { 104, event_text_key_names.famine_refuse_reason_p_a },
    { 105, event_text_key_names.famine_refuse_reason_c_a },
    { 106, event_text_key_names.famine_no_reason_p_a },
    { 107, event_text_key_names.famine_no_reason_c_a },
    { 108, event_text_key_names.threat_title_p },
    { 109, event_text_key_names.threat_title_c },
    { 110, event_text_key_names.threat_initial_announcement_p },
    { 111, event_text_key_names.threat_initial_announcement_c },
    { 112, event_text_key_names.threat_reminder_p },
    { 113, event_text_key_names.threat_reminder_c },
    { 114, event_text_key_names.threat_overdue_p },
    { 115, event_text_key_names.threat_overdue_c },
    { 116, event_text_key_names.threat_warning_p },
    { 117, event_text_key_names.threat_warning_c },
    { 118, event_text_key_names.threat_comply_reason_p_a },
    { 119, event_text_key_names.threat_comply_reason_c_a },
    { 120, event_text_key_names.threat_too_late_reason_p_a },
    { 121, event_text_key_names.threat_too_late_reason_c_a },
    { 122, event_text_key_names.threat_refuse_reason_p_a },
    { 123, event_text_key_names.threat_refuse_reason_c_a },
    { 124, event_text_key_names.threat_no_reason_p_a },
    { 125, event_text_key_names.threat_no_reason_c_a },
    { 126, event_text_key_names.eg_city_falls_title },
    { 127, event_text_key_names.eg_city_falls_initial_announcement },
    { 128, event_text_key_names.eg_city_falls_reason_a },
    { 129, event_text_key_names.eg_city_falls_no_reason_a },
    { 130, event_text_key_names.foreign_city_conquered_title },
    { 131, event_text_key_names.foreign_city_conquered_initial_announcement },
    { 132, event_text_key_names.foreign_city_conquered_reason_a },
    { 133, event_text_key_names.foreign_city_conquered_no_reason_a },
    { 134, event_text_key_names.route_opened_title },
    { 135, event_text_key_names.route_opened_initial_announcement },
    { 136, event_text_key_names.route_opened_reason_a },
    { 137, event_text_key_names.route_opened_no_reason_a },
    { 138, event_text_key_names.route_closed_title },
    { 139, event_text_key_names.route_closed_initial_announcement },
    { 140, event_text_key_names.route_closed_reason_a },
    { 141, event_text_key_names.route_closed_no_reason_a },
    { 142, event_text_key_names.siege_title },
    { 143, event_text_key_names.siege_initial_announcement },
    { 144, event_text_key_names.siege_reason_a },
    { 145, event_text_key_names.siege_no_reason_a },
    { 146, event_text_key_names.eg_city_saved_title },
    { 147, event_text_key_names.eg_city_saved_initial_announcement },
    { 148, event_text_key_names.eg_city_saved_reason_a },
    { 149, event_text_key_names.eg_city_saved_no_reason_a },
    { 150, event_text_key_names.battle_won_title },
    { 151, event_text_key_names.battle_won_initial_announcement },
    { 152, event_text_key_names.battle_won_reason_a },
    { 153, event_text_key_names.battle_won_no_reason_a },
    { 154, event_text_key_names.battle_lost_title },
    { 155, event_text_key_names.battle_lost_initial_announcement },
    { 156, event_text_key_names.battle_lost_reason_a },
    { 157, event_text_key_names.battle_lost_no_reason_a },
    { 158, event_text_key_names.acknowledgement_title },
    { 159, event_text_key_names.acknowledgement_initial_announcement },
    { 160, event_text_key_names.acknowledgement_reason_a },
    { 161, event_text_key_names.acknowledgement_no_reason_a },
    { 162, event_text_key_names.pharaoh_attacks_you_title },
    { 163, event_text_key_names.pharaoh_attacks_you_initial_announcement },
    { 164, event_text_key_names.pharaoh_attacks_you_2year_reminder },
    { 165, event_text_key_names.pharaoh_attacks_you_1year_reminder },
    { 166, event_text_key_names.pharaoh_attacks_you_6month_warning },
    { 167, event_text_key_names.pharaoh_attacks_you_1month_warning },
    { 168, event_text_key_names.pharaoh_attacks_you_city_attacked_alert },
    { 169, event_text_key_names.pharaoh_attacks_you_reason_a },
    { 170, event_text_key_names.pharaoh_attacks_you_no_reason_a },
    { 171, event_text_key_names.pharaoh_attacks_you_because_of_low_favor },
    { 172, event_text_key_names.eg_city_attacks_you_title },
    { 173, event_text_key_names.eg_city_attacks_you_initial_announcement },
    { 174, event_text_key_names.eg_city_attacks_you_2year_reminder },
    { 175, event_text_key_names.eg_city_attacks_you_1year_reminder },
    { 176, event_text_key_names.eg_city_attacks_you_6month_warning },
    { 177, event_text_key_names.eg_city_attacks_you_1month_warning },
    { 178, event_text_key_names.eg_city_attacks_you_city_attacked_alert },
    { 179, event_text_key_names.eg_city_attacks_you_reason_a },
    { 180, event_text_key_names.eg_city_attacks_you_no_reason_a },
    { 181, event_text_key_names.eg_city_attacks_you_because_of_low_favor },
    { 182, event_text_key_names.foreign_army_attacks_you_title },
    { 183, event_text_key_names.foreign_army_attacks_you_initial_announcement },
    { 184, event_text_key_names.foreign_army_attacks_you_2year_reminder },
    { 185, event_text_key_names.foreign_army_attacks_you_1year_reminder },
    { 186, event_text_key_names.foreign_army_attacks_you_6month_warning },
    { 187, event_text_key_names.foreign_army_attacks_you_1month_warning },
    { 188, event_text_key_names.foreign_army_attacks_you_city_attacked_alert },
    { 189, event_text_key_names.foreign_army_attacks_you_reason_a },
    { 190, event_text_key_names.foreign_army_attacks_you_no_reason_a },
    { 191, event_text_key_names.rating_change_title_i },
    { 192, event_text_key_names.rating_change_initial_announcement_i },
    { 193, event_text_key_names.rating_change_reason_i_a },
    { 194, event_text_key_names.rating_change_no_reason_i_a },
    { 195, event_text_key_names.rating_change_title_d },
    { 196, event_text_key_names.rating_change_initial_announcement_d },
    { 197, event_text_key_names.rating_change_reason_d_a },
    { 198, event_text_key_names.rating_change_no_reason_d_a },
    { 199, event_text_key_names.price_change_title_i },
    { 200, event_text_key_names.price_change_initial_announcement_i },
    { 201, event_text_key_names.price_change_reason_i_a },
    { 202, event_text_key_names.price_change_no_reason_i_a },
    { 203, event_text_key_names.price_change_title_d },
    { 204, event_text_key_names.price_change_initial_announcement_d },
    { 205, event_text_key_names.price_change_reason_d_a },
    { 206, event_text_key_names.price_change_no_reason_d_a },
    { 207, event_text_key_names.demand_change_title_i },
    { 208, event_text_key_names.demand_change_initial_announcement_i },
    { 209, event_text_key_names.demand_change_reason_i_a },
    { 210, event_text_key_names.demand_change_no_reason_i_a },
    { 211, event_text_key_names.demand_change_title_d },
    { 212, event_text_key_names.demand_change_initial_announcement_d },
    { 213, event_text_key_names.demand_change_reason_d_a },
    { 214, event_text_key_names.demand_change_no_reason_d_a },
    { 215, event_text_key_names.earthquake_title },
    { 216, event_text_key_names.earthquake_initial_announcement },
    { 217, event_text_key_names.earthquake_reason_a },
    { 218, event_text_key_names.earthquake_no_reason_a },
    { 219, event_text_key_names.stormy_seas_title },
    { 220, event_text_key_names.stormy_seas_initial_announcement },
    { 221, event_text_key_names.stormy_seas_reason_a },
    { 222, event_text_key_names.stormy_seas_no_reason_a },
    { 223, event_text_key_names.sandstorm_title },
    { 224, event_text_key_names.sandstorm_initial_announcement },
    { 225, event_text_key_names.sandstorm_reason_a },
    { 226, event_text_key_names.sandstorm_no_reason_a },
    { 227, event_text_key_names.wage_change_title_i },
    { 228, event_text_key_names.wage_change_initial_announcement_i },
    { 229, event_text_key_names.wage_change_reason_i_a },
    { 230, event_text_key_names.wage_change_no_reason_i_a },
    { 231, event_text_key_names.wage_change_title_d },
    { 232, event_text_key_names.wage_change_initial_announcement_d },
    { 233, event_text_key_names.wage_change_reason_d_a },
    { 234, event_text_key_names.wage_change_no_reason_d_a },
    { 235, event_text_key_names.bad_water_title },
    { 236, event_text_key_names.bad_water_initial_announcement },
    { 237, event_text_key_names.bad_water_reason_a },
    { 238, event_text_key_names.bad_water_no_reason_a },
    { 239, event_text_key_names.goldmine_cavein_title },
    { 240, event_text_key_names.goldmine_cavein_initial_announcement },
    { 241, event_text_key_names.goldmine_cavein_reason_a },
    { 242, event_text_key_names.goldmine_cavein_no_reason_a },
    { 243, event_text_key_names.landslide_title },
    { 244, event_text_key_names.landslide_initial_announcement },
    { 245, event_text_key_names.landslide_reason_a },
    { 246, event_text_key_names.landslide_no_reason_a },
    { 247, event_text_key_names.flood_fails_title },
    { 248, event_text_key_names.flood_fails_initial_announcement },
    { 249, event_text_key_names.flood_fails_reason_a },
    { 250, event_text_key_names.flood_fails_no_reason_a },
    { 251, event_text_key_names.perfect_flood_title },
    { 252, event_text_key_names.perfect_flood_initial_announcement },
    { 253, event_text_key_names.perfect_flood_reason_a },
    { 254, event_text_key_names.perfect_flood_no_reason_a },
    { 255, event_text_key_names.bedouin_attacks_you_title },
    { 256, event_text_key_names.bedouin_attacks_you_initial_announcement },
    { 257, event_text_key_names.bedouin_attacks_you_2year_reminder },
    { 258, event_text_key_names.bedouin_attacks_you_1year_reminder },
    { 259, event_text_key_names.bedouin_attacks_you_6month_warning },
    { 260, event_text_key_names.bedouin_attacks_you_1month_warning },
    { 261, event_text_key_names.bedouin_attacks_you_city_attacked_alert },
    { 262, event_text_key_names.bedouin_attacks_you_reason_a },
    { 263, event_text_key_names.bedouin_attacks_you_no_reason_a },
    { 264, event_text_key_names.gift_title_p },
    { 265, event_text_key_names.gift_title_c },
    { 266, event_text_key_names.gift_granted_p },
    { 267, event_text_key_names.gift_granted_c },
    { 268, event_text_key_names.gift_cash_granted_p },
    { 269, event_text_key_names.gift_cash_granted_c },
    { 270, event_text_key_names.gift_partial_space_p },
    { 271, event_text_key_names.gift_partial_space_c },
    { 272, event_text_key_names.gift_insufficient_space_p },
    { 273, event_text_key_names.gift_insufficient_space_c },
    { 274, event_text_key_names.gift_last_chance_p },
    { 275, event_text_key_names.gift_last_chance_c },
    { 276, event_text_key_names.gift_forfeited_p },
    { 277, event_text_key_names.gift_forfeited_c },
    { 278, event_text_key_names.gift_accepted_p },
    { 279, event_text_key_names.gift_accepted_c },
    { 280, event_text_key_names.gift_cash_accepted_p },
    { 281, event_text_key_names.gift_cash_accepted_c },
    { 282, event_text_key_names.gift_postponed_p },
    { 283, event_text_key_names.gift_postponed_c },
    { 284, event_text_key_names.gift_refused_p },
    { 285, event_text_key_names.gift_refused_c },
    { 286, event_text_key_names.gift_accepted_reason_p_a },
    { 287, event_text_key_names.gift_accepted_reason_c_a },
    { 288, event_text_key_names.gift_forfeited_reason_p_a },
    { 289, event_text_key_names.gift_forfeited_reason_c_a },
    { 290, event_text_key_names.gift_refused_reason_p_a },
    { 291, event_text_key_names.gift_refused_reason_c_a },
    { 292, event_text_key_names.gift_no_reason_p_a },
    { 293, event_text_key_names.gift_no_reason_c_a },
    { 294, event_text_key_names.pharaoh_attacks_you_disembarked_alert },
    { 295, event_text_key_names.eg_city_attacks_you_disembarked_alert },
    { 296, event_text_key_names.foreign_army_attacks_you_disembarked_alert },
    { 297, event_text_key_names.bedouin_attacks_you_disembarked_alert },
    { 298, event_text_key_names.egyptian_city_attacked_comply_reason_p_b },
    { 299, event_text_key_names.egyptian_city_attacked_comply_reason_p_c },
    { 300, event_text_key_names.egyptian_city_attacked_comply_reason_c_b },
    { 301, event_text_key_names.egyptian_city_attacked_comply_reason_c_c },
    { 302, event_text_key_names.egyptian_city_attacked_too_late_reason_p_b },
    { 303, event_text_key_names.egyptian_city_attacked_too_late_reason_p_c },
    { 304, event_text_key_names.egyptian_city_attacked_too_late_reason_c_b },
    { 305, event_text_key_names.egyptian_city_attacked_too_late_reason_c_c },
    { 306, event_text_key_names.egyptian_city_attacked_refuse_reason_p_b },
    { 307, event_text_key_names.egyptian_city_attacked_refuse_reason_p_c },
    { 308, event_text_key_names.egyptian_city_attacked_refuse_reason_c_b },
    { 309, event_text_key_names.egyptian_city_attacked_refuse_reason_c_c },
    { 310, event_text_key_names.egyptian_city_attacked_lost_battle_reason_p_b },
    { 311, event_text_key_names.egyptian_city_attacked_lost_battle_reason_p_c },
    { 312, event_text_key_names.egyptian_city_attacked_lost_battle_reason_c_b },
    { 313, event_text_key_names.egyptian_city_attacked_lost_battle_reason_c_c },
    { 314, event_text_key_names.egyptian_city_attacked_no_reason_p_b },
    { 315, event_text_key_names.egyptian_city_attacked_no_reason_p_c },
    { 316, event_text_key_names.egyptian_city_attacked_no_reason_c_b },
    { 317, event_text_key_names.egyptian_city_attacked_no_reason_c_c },
    { 318, event_text_key_names.distant_battle_comply_reason_p_b },
    { 319, event_text_key_names.distant_battle_comply_reason_p_c },
    { 320, event_text_key_names.distant_battle_comply_reason_c_b },
    { 321, event_text_key_names.distant_battle_comply_reason_c_c },
    { 322, event_text_key_names.distant_battle_too_late_reason_p_b },
    { 323, event_text_key_names.distant_battle_too_late_reason_p_c },
    { 324, event_text_key_names.distant_battle_too_late_reason_c_b },
    { 325, event_text_key_names.distant_battle_too_late_reason_c_c },
    { 326, event_text_key_names.distant_battle_refuse_reason_p_b },
    { 327, event_text_key_names.distant_battle_refuse_reason_p_c },
    { 328, event_text_key_names.distant_battle_refuse_reason_c_b },
    { 329, event_text_key_names.distant_battle_refuse_reason_c_c },
    { 330, event_text_key_names.distant_battle_lost_battle_reason_p_b },
    { 331, event_text_key_names.distant_battle_lost_battle_reason_p_c },
    { 332, event_text_key_names.distant_battle_lost_battle_reason_c_b },
    { 333, event_text_key_names.distant_battle_lost_battle_reason_c_c },
    { 334, event_text_key_names.distant_battle_no_reason_p_b },
    { 335, event_text_key_names.distant_battle_no_reason_p_c },
    { 336, event_text_key_names.distant_battle_no_reason_c_b },
    { 337, event_text_key_names.distant_battle_no_reason_c_c },
    { 338, event_text_key_names.general_request_comply_reason_p_b },
    { 339, event_text_key_names.general_request_comply_reason_p_c },
    { 340, event_text_key_names.general_request_comply_reason_c_b },
    { 341, event_text_key_names.general_request_comply_reason_c_c },
    { 342, event_text_key_names.general_request_too_late_reason_p_b },
    { 343, event_text_key_names.general_request_too_late_reason_p_c },
    { 344, event_text_key_names.general_request_too_late_reason_c_b },
    { 345, event_text_key_names.general_request_too_late_reason_c_c },
    { 346, event_text_key_names.general_request_refuse_reason_p_b },
    { 347, event_text_key_names.general_request_refuse_reason_p_c },
    { 348, event_text_key_names.general_request_refuse_reason_c_b },
    { 349, event_text_key_names.general_request_refuse_reason_c_c },
    { 350, event_text_key_names.general_request_no_reason_p_b },
    { 351, event_text_key_names.general_request_no_reason_p_c },
    { 352, event_text_key_names.general_request_no_reason_c_b },
    { 353, event_text_key_names.general_request_no_reason_c_c },
    { 354, event_text_key_names.great_festival_comply_reason_p_b },
    { 355, event_text_key_names.great_festival_comply_reason_p_c },
    { 356, event_text_key_names.great_festival_comply_reason_c_b },
    { 357, event_text_key_names.great_festival_comply_reason_c_c },
    { 358, event_text_key_names.great_festival_too_late_reason_p_b },
    { 359, event_text_key_names.great_festival_too_late_reason_p_c },
    { 360, event_text_key_names.great_festival_too_late_reason_c_b },
    { 361, event_text_key_names.great_festival_too_late_reason_c_c },
    { 362, event_text_key_names.great_festival_refuse_reason_p_b },
    { 363, event_text_key_names.great_festival_refuse_reason_p_c },
    { 364, event_text_key_names.great_festival_refuse_reason_c_b },
    { 365, event_text_key_names.great_festival_refuse_reason_c_c },
    { 366, event_text_key_names.great_festival_no_reason_p_b },
    { 367, event_text_key_names.great_festival_no_reason_p_c },
    { 368, event_text_key_names.great_festival_no_reason_c_b },
    { 369, event_text_key_names.great_festival_no_reason_c_c },
    { 370, event_text_key_names.project_comply_reason_p_b },
    { 371, event_text_key_names.project_comply_reason_p_c },
    { 372, event_text_key_names.project_comply_reason_c_b },
    { 373, event_text_key_names.project_comply_reason_c_c },
    { 374, event_text_key_names.project_too_late_reason_p_b },
    { 375, event_text_key_names.project_too_late_reason_p_c },
    { 376, event_text_key_names.project_too_late_reason_c_b },
    { 377, event_text_key_names.project_too_late_reason_c_c },
    { 378, event_text_key_names.project_refuse_reason_p_b },
    { 379, event_text_key_names.project_refuse_reason_p_c },
    { 380, event_text_key_names.project_refuse_reason_c_b },
    { 381, event_text_key_names.project_refuse_reason_c_c },
    { 382, event_text_key_names.project_no_reason_p_b },
    { 383, event_text_key_names.project_no_reason_p_c },
    { 384, event_text_key_names.project_no_reason_c_b },
    { 385, event_text_key_names.project_no_reason_c_c },
    { 386, event_text_key_names.famine_comply_reason_p_b },
    { 387, event_text_key_names.famine_comply_reason_p_c },
    { 388, event_text_key_names.famine_comply_reason_c_b },
    { 389, event_text_key_names.famine_comply_reason_c_c },
    { 390, event_text_key_names.famine_too_late_reason_p_b },
    { 391, event_text_key_names.famine_too_late_reason_p_c },
    { 392, event_text_key_names.famine_too_late_reason_c_b },
    { 393, event_text_key_names.famine_too_late_reason_c_c },
    { 394, event_text_key_names.famine_refuse_reason_p_b },
    { 395, event_text_key_names.famine_refuse_reason_p_c },
    { 396, event_text_key_names.famine_refuse_reason_c_b },
    { 397, event_text_key_names.famine_refuse_reason_c_c },
    { 398, event_text_key_names.famine_no_reason_p_b },
    { 399, event_text_key_names.famine_no_reason_p_c },
    { 400, event_text_key_names.famine_no_reason_c_b },
    { 401, event_text_key_names.famine_no_reason_c_c },
    { 402, event_text_key_names.threat_comply_reason_p_b },
    { 403, event_text_key_names.threat_comply_reason_p_c },
    { 404, event_text_key_names.threat_comply_reason_c_b },
    { 405, event_text_key_names.threat_comply_reason_c_c },
    { 406, event_text_key_names.threat_too_late_reason_p_b },
    { 407, event_text_key_names.threat_too_late_reason_p_c },
    { 408, event_text_key_names.threat_too_late_reason_c_b },
    { 409, event_text_key_names.threat_too_late_reason_c_c },
    { 410, event_text_key_names.threat_refuse_reason_p_b },
    { 411, event_text_key_names.threat_refuse_reason_p_c },
    { 412, event_text_key_names.threat_refuse_reason_c_b },
    { 413, event_text_key_names.threat_refuse_reason_c_c },
    { 414, event_text_key_names.threat_no_reason_p_b },
    { 415, event_text_key_names.threat_no_reason_p_c },
    { 416, event_text_key_names.threat_no_reason_c_b },
    { 417, event_text_key_names.threat_no_reason_c_c },
    { 418, event_text_key_names.eg_city_falls_reason_b },
    { 419, event_text_key_names.eg_city_falls_reason_c },
    { 420, event_text_key_names.eg_city_falls_no_reason_b },
    { 421, event_text_key_names.eg_city_falls_no_reason_c },
    { 422, event_text_key_names.foreign_city_conquered_reason_b },
    { 423, event_text_key_names.foreign_city_conquered_reason_c },
    { 424, event_text_key_names.foreign_city_conquered_no_reason_b },
    { 425, event_text_key_names.foreign_city_conquered_no_reason_c },
    { 426, event_text_key_names.route_opened_reason_b },
    { 427, event_text_key_names.route_opened_reason_c },
    { 428, event_text_key_names.route_opened_no_reason_b },
    { 429, event_text_key_names.route_opened_no_reason_c },
    { 430, event_text_key_names.route_closed_reason_b },
    { 431, event_text_key_names.route_closed_reason_c },
    { 432, event_text_key_names.route_closed_no_reason_b },
    { 433, event_text_key_names.route_closed_no_reason_c },
    { 434, event_text_key_names.siege_reason_b },
    { 435, event_text_key_names.siege_reason_c },
    { 436, event_text_key_names.siege_no_reason_b },
    { 437, event_text_key_names.siege_no_reason_c },
    { 438, event_text_key_names.eg_city_saved_reason_b },
    { 439, event_text_key_names.eg_city_saved_reason_c },
    { 440, event_text_key_names.eg_city_saved_no_reason_b },
    { 441, event_text_key_names.eg_city_saved_no_reason_c },
    { 442, event_text_key_names.battle_won_reason_b },
    { 443, event_text_key_names.battle_won_reason_c },
    { 444, event_text_key_names.battle_won_no_reason_b },
    { 445, event_text_key_names.battle_won_no_reason_c },
    { 446, event_text_key_names.battle_lost_reason_b },
    { 447, event_text_key_names.battle_lost_reason_c },
    { 448, event_text_key_names.battle_lost_no_reason_b },
    { 449, event_text_key_names.battle_lost_no_reason_c },
    { 450, event_text_key_names.acknowledgement_reason_b },
    { 451, event_text_key_names.acknowledgement_reason_c },
    { 452, event_text_key_names.acknowledgement_no_reason_b },
    { 453, event_text_key_names.acknowledgement_no_reason_c },
    { 454, event_text_key_names.pharaoh_attacks_you_reason_b },
    { 455, event_text_key_names.pharaoh_attacks_you_reason_c },
    { 456, event_text_key_names.pharaoh_attacks_you_no_reason_b },
    { 457, event_text_key_names.pharaoh_attacks_you_no_reason_c },
    { 458, event_text_key_names.eg_city_attacks_you_reason_b },
    { 459, event_text_key_names.eg_city_attacks_you_reason_c },
    { 460, event_text_key_names.eg_city_attacks_you_no_reason_b },
    { 461, event_text_key_names.eg_city_attacks_you_no_reason_c },
    { 462, event_text_key_names.foreign_army_attacks_you_reason_b },
    { 463, event_text_key_names.foreign_army_attacks_you_reason_c },
    { 464, event_text_key_names.foreign_army_attacks_you_no_reason_b },
    { 465, event_text_key_names.foreign_army_attacks_you_no_reason_c },
    { 466, event_text_key_names.rating_change_reason_i_b },
    { 467, event_text_key_names.rating_change_reason_i_c },
    { 468, event_text_key_names.rating_change_no_reason_i_b },
    { 469, event_text_key_names.rating_change_no_reason_i_c },
    { 470, event_text_key_names.rating_change_reason_d_b },
    { 471, event_text_key_names.rating_change_reason_d_c },
    { 472, event_text_key_names.rating_change_no_reason_d_b },
    { 473, event_text_key_names.rating_change_no_reason_d_c },
    { 474, event_text_key_names.price_change_reason_i_b },
    { 475, event_text_key_names.price_change_reason_i_c },
    { 476, event_text_key_names.price_change_no_reason_i_b },
    { 477, event_text_key_names.price_change_no_reason_i_c },
    { 478, event_text_key_names.price_change_reason_d_b },
    { 479, event_text_key_names.price_change_reason_d_c },
    { 480, event_text_key_names.price_change_no_reason_d_b },
    { 481, event_text_key_names.price_change_no_reason_d_c },
    { 482, event_text_key_names.demand_change_reason_i_b },
    { 483, event_text_key_names.demand_change_reason_i_c },
    { 484, event_text_key_names.demand_change_no_reason_i_b },
    { 485, event_text_key_names.demand_change_no_reason_i_c },
    { 486, event_text_key_names.demand_change_reason_d_b },
    { 487, event_text_key_names.demand_change_reason_d_c },
    { 488, event_text_key_names.demand_change_no_reason_d_b },
    { 489, event_text_key_names.demand_change_no_reason_d_c },
    { 490, event_text_key_names.earthquake_reason_b },
    { 491, event_text_key_names.earthquake_reason_c },
    { 492, event_text_key_names.earthquake_no_reason_b },
    { 493, event_text_key_names.earthquake_no_reason_c },
    { 494, event_text_key_names.stormy_seas_reason_b },
    { 495, event_text_key_names.stormy_seas_reason_c },
    { 496, event_text_key_names.stormy_seas_no_reason_b },
    { 497, event_text_key_names.stormy_seas_no_reason_c },
    { 498, event_text_key_names.sandstorm_reason_b },
    { 499, event_text_key_names.sandstorm_reason_c },
    { 500, event_text_key_names.sandstorm_no_reason_b },
    { 501, event_text_key_names.sandstorm_no_reason_c },
    { 502, event_text_key_names.wage_change_reason_i_b },
    { 503, event_text_key_names.wage_change_reason_i_c },
    { 504, event_text_key_names.wage_change_no_reason_i_b },
    { 505, event_text_key_names.wage_change_no_reason_i_c },
    { 506, event_text_key_names.wage_change_reason_d_b },
    { 507, event_text_key_names.wage_change_reason_d_c },
    { 508, event_text_key_names.wage_change_no_reason_d_b },
    { 509, event_text_key_names.wage_change_no_reason_d_c },
    { 510, event_text_key_names.bad_water_reason_b },
    { 511, event_text_key_names.bad_water_reason_c },
    { 512, event_text_key_names.bad_water_no_reason_b },
    { 513, event_text_key_names.bad_water_no_reason_c },
    { 514, event_text_key_names.goldmine_cavein_reason_b },
    { 515, event_text_key_names.goldmine_cavein_reason_c },
    { 516, event_text_key_names.goldmine_cavein_no_reason_b },
    { 517, event_text_key_names.goldmine_cavein_no_reason_c },
    { 518, event_text_key_names.landslide_reason_b },
    { 519, event_text_key_names.landslide_reason_c },
    { 520, event_text_key_names.landslide_no_reason_b },
    { 521, event_text_key_names.landslide_no_reason_c },
    { 522, event_text_key_names.flood_fails_reason_b },
    { 523, event_text_key_names.flood_fails_reason_c },
    { 524, event_text_key_names.flood_fails_no_reason_b },
    { 525, event_text_key_names.flood_fails_no_reason_c },
    { 526, event_text_key_names.perfect_flood_reason_b },
    { 527, event_text_key_names.perfect_flood_reason_c },
    { 528, event_text_key_names.perfect_flood_no_reason_b },
    { 529, event_text_key_names.perfect_flood_no_reason_c },
    { 530, event_text_key_names.bedouin_attacks_you_reason_b },
    { 531, event_text_key_names.bedouin_attacks_you_reason_c },
    { 532, event_text_key_names.bedouin_attacks_you_no_reason_b },
    { 533, event_text_key_names.bedouin_attacks_you_no_reason_c },
    { 534, event_text_key_names.gift_accepted_reason_p_b },
    { 535, event_text_key_names.gift_accepted_reason_p_c },
    { 536, event_text_key_names.gift_accepted_reason_c_b },
    { 537, event_text_key_names.gift_accepted_reason_c_c },
    { 538, event_text_key_names.gift_forfeited_reason_p_b },
    { 539, event_text_key_names.gift_forfeited_reason_p_c },
    { 540, event_text_key_names.gift_forfeited_reason_c_b },
    { 541, event_text_key_names.gift_forfeited_reason_c_c },
    { 542, event_text_key_names.gift_refused_reason_p_b },
    { 543, event_text_key_names.gift_refused_reason_p_c },
    { 544, event_text_key_names.gift_refused_reason_c_b },
    { 545, event_text_key_names.gift_refused_reason_c_c },
    { 546, event_text_key_names.gift_no_reason_p_b },
    { 547, event_text_key_names.gift_no_reason_p_c },
    { 548, event_text_key_names.gift_no_reason_c_b },
    { 549, event_text_key_names.gift_no_reason_c_c },
    { 550, event_text_key_names.pyramid_congratulations_title },
    { 551, event_text_key_names.pyramid_congratulations },
    { 552, event_text_key_names.stepped_pyramid_congratulations_title },
    { 553, event_text_key_names.stepped_pyramid_congratulations },
    { 554, event_text_key_names.bent_pyramid_congratulations_title },
    { 555, event_text_key_names.bent_pyramid_congratulations },
    { 556, event_text_key_names.mudbrick_pyramid_congratulations_title },
    { 557, event_text_key_names.mudbrick_pyramid_congratulations },
    { 558, event_text_key_names.mastaba_congratulations_title },
    { 559, event_text_key_names.mastaba_congratulations },
    { 560, event_text_key_names.sphinx_congratulations_title },
    { 561, event_text_key_names.sphinx_congratulations },
    { 562, event_text_key_names.obelisk_congratulations_title },
    { 563, event_text_key_names.obelisk_congratulations },
    { 564, event_text_key_names.sun_temple_congratulations_title },
    { 565, event_text_key_names.sun_temple_congratulations },
    { 566, event_text_key_names.alex_library_congratulations_title },
    { 567, event_text_key_names.alex_library_congratulations },
    { 568, event_text_key_names.abu_simbel_congratulations_title },
    { 569, event_text_key_names.abu_simbel_congratulations },
    { 570, event_text_key_names.caesareum_congratulations_title },
    { 571, event_text_key_names.caesareum_congratulations },
    { 572, event_text_key_names.lighthouse_congratulations_title },
    { 573, event_text_key_names.lighthouse_congratulations },
    { 574, event_text_key_names.mausoleum_congratulations_title },
    { 575, event_text_key_names.mausoleum_congratulations },
    { 576, event_text_key_names.smalltomb_congratulations_title },
    { 577, event_text_key_names.smalltomb_congratulations },
    { 578, event_text_key_names.medtomb_congratulations_title },
    { 579, event_text_key_names.medtomb_congratulations },
    { 580, event_text_key_names.largetomb_congratulations_title },
    { 581, event_text_key_names.largetomb_congratulations },
    { 582, event_text_key_names.grandtomb_congratulations_title },
    { 583, event_text_key_names.grandtomb_congratulations },
    { 584, event_text_key_names.troopcarryover_title },
    { 585, event_text_key_names.troopcarryover_initial_announcement },
    { 586, event_text_key_names.troopcarryover_inf_only },
    { 587, event_text_key_names.troopcarryover_arch_only },
    { 588, event_text_key_names.troopcarryover_char_only },
    { 589, event_text_key_names.troopcarryover_inf_arch },
    { 590, event_text_key_names.troopcarryover_inf_char },
    { 591, event_text_key_names.troopcarryover_arch_char },
    { 592, event_text_key_names.troopcarryover_all_three },
    { 593, event_text_key_names.pyramid_speedup_title },
    { 594, event_text_key_names.pyramid_speedup_announcement },
    { 595, event_text_key_names.pyramid_minor_speedup_announcement },
    { 596, event_text_key_names.pyramid_speedup_osiris },
    { 597, event_text_key_names.pyramid_speedup_ra },
    { 598, event_text_key_names.pyramid_speedup_ptah },
    { 599, event_text_key_names.pyramid_speedup_seth },
    { 600, event_text_key_names.pyramid_speedup_bast },
    { -1, 0 }
};

const char file_extension_555[4] = "555";

const char image_555_directory[20] = "555\\\0\0\0\0"
                       "status.txt";

chunk_converter savegame_converter_scenario_events[2] = { { 15628, 1, SAVE_VERSION_EVENT_RELAYOUT, game_file_convert_scenario_events }, { 0, 0, 0, NULL } };

chunk_converter savegame_converter_messages[2] = { { 44, 1000, SAVE_VERSION_MESSAGE_RELAYOUT, game_file_convert_message }, { 0, 0, 0, NULL } };

chunk_entry savegame_chunk_table[SAVEGAME_CHUNKS] = {
    { 0, &settings_data.mission_id, 1, 1, NULL },
    { 0, &settings_data.campaign, 1, 1, NULL },
    { 0, &settings_data.previous_campaign_link, 1, 1, NULL },
    { 0, &settings_data.campaign_link, 1, 1, NULL },
    { 0, &savegame_file_version, 4, 1, NULL },
    { 0, &savegame_schema_chunk_count, 4, 1, NULL },
    { 0, savegame_chunk_schema, 20, 300, NULL },
    { 1, map_grids.images, 4, 51984, NULL },
    { 1, map_grids.edge, 1, 51984, NULL },
    { 1, map_grids.building_ids, 2, 51984, NULL },
    { 1, map_grids.terrain, 4, 51984, NULL },
    { 1, map_grids.canals, 1, 51984, NULL },
    { 1, map_grids.figure_ids, 2, 51984, NULL },
    { 1, map_grids.bitfields, 1, 51984, NULL },
    { 1, map_grids.sprite, 1, 51984, NULL },
    { 0, map_grids.random, 1, 51984, NULL },
    { 1, map_grids.desirability, 1, 51984, NULL },
    { 1, map_grids.elevation, 1, 51984, NULL },
    { 1, map_grids.building_damage, 2, 51984, NULL },
    { 1, map_canals_backup, 1, 51984, NULL },
    { 1, map_sprite_backup, 1, 51984, NULL },
    { 1, city_figures.records, 388, 2000, NULL },
    { 1, figure_route_figure_ids, 2, 1000, NULL },
    { 1, city_figures.route_paths, 1, 500000, NULL },
    { 1, &formation_array, 144, 50, NULL },
    { 0, &formation_id_last_in_use, 4, 1, NULL },
    { 0, &formation_id_last_legion, 4, 1, NULL },
    { 0, &formation_num_legions, 4, 1, NULL },
    { 1, city_players, 18904, 2, NULL },
    { 0, city_player_faction, 2, 2, NULL },
    { 0, city_player_names, 32, 2, NULL },
    { 0, &city_current_player, 4, 1, NULL },
    { 1, city_buildings.records, 264, 4000, NULL },
    { 0, &city_view_orientation, 4, 1, NULL },
    { 0, &game_time_tick, 4, 1, NULL },
    { 0, &game_time_day, 4, 1, NULL },
    { 0, &game_time_month, 4, 1, NULL },
    { 0, &game_time_year, 4, 1, NULL },
    { 0, &game_time_total_days, 4, 1, NULL },
    { 0, &building_highest_id_ever, 4, 1, NULL },
    { 0, &city_locusts_countdown, 4, 1, NULL },
    { 0, &random_generator_state.iv1, 4, 1, NULL },
    { 0, &random_generator_state.iv2, 4, 1, NULL },
    { 0, &map_info.camera_x, 4, 1, NULL },
    { 0, &map_info.camera_y, 4, 1, NULL },
    { 0, &city_population_graph_order, 4, 1, NULL },
    { 0, &city_hailstorm_countdown, 4, 1, NULL },
    { 0, &empire_scroll_x, 4, 1, NULL },
    { 0, &empire_scroll_y, 4, 1, NULL },
    { 0, &empire_selected_object, 4, 1, NULL },
    { 1, &empire_cities, 106, 61, NULL },
    { 0, building_count_industry_total, 4, 36, NULL },
    { 0, building_count_industry_active, 4, 36, NULL },
    { 0, trade_prices, 8, 36, NULL },
    { 0, &figure_name_counter_table.by_slot[40], 4, 1, NULL },
    { 0, &figure_name_counter_table.by_slot[39], 4, 1, NULL },
    { 0, &figure_name_counter_table.by_slot[38], 4, 1, NULL },
    { 0, &figure_name_counter_table.by_slot[37], 4, 1, NULL },
    { 0, &figure_name_counter_table.by_slot[36], 4, 1, NULL },
    { 0, &figure_name_counter_table.by_slot[35], 4, 1, NULL },
    { 0, &figure_name_counter_table.by_slot[34], 4, 1, NULL },
    { 0, &figure_name_counter_table.by_slot[33], 4, 1, NULL },
    { 0, &figure_name_counter_table.by_slot[32], 4, 1, NULL },
    { 0, &figure_name_counter_table.by_slot[31], 4, 1, NULL },
    { 0, &figure_name_counter_table.by_slot[30], 4, 1, NULL },
    { 0, &figure_name_counter_table.by_slot[29], 4, 1, NULL },
    { 0, &figure_name_counter_table.by_slot[28], 4, 1, NULL },
    { 0, &figure_name_counter_table.by_slot[27], 4, 1, NULL },
    { 0, &figure_name_counter_table.by_slot[26], 4, 1, NULL },
    { 0, &figure_name_counter_table.by_slot[25], 4, 1, NULL },
    { 0, &figure_name_counter_table.by_slot[24], 4, 1, NULL },
    { 0, &figure_name_counter_table.by_slot[23], 4, 1, NULL },
    { 0, &figure_name_counter_table.by_slot[22], 4, 1, NULL },
    { 0, &figure_name_counter_table.by_slot[21], 4, 1, NULL },
    { 0, &figure_name_counter_table.by_slot[20], 4, 1, NULL },
    { 0, &scenario_data, 1592, 1, NULL },
    { 0, &scenario_max_year, 4, 1, NULL },
    { 1, city_messages, 48, 1000, savegame_converter_messages },
    { 0, &city_message_next_sequence, 4, 1, NULL },
    { 0, &city_message_total, 4, 1, NULL },
    { 0, &city_message_current_id, 4, 1, NULL },
    { 0, city_message_population_shown, 1, 10, NULL },
    { 0, city_message_count, 4, 20, NULL },
    { 0, city_message_delay, 4, 20, NULL },
    { 0, &building_burning_total, 4, 1, NULL },
    { 0, &building_burning_size, 4, 1, NULL },
    { 0, &figure_created_sequence, 4, 1, NULL },
    { 0, &settings_data.starting_favor, 4, 1, NULL },
    { 0, &settings_data.personal_savings_last_mission, 4, 1, NULL },
    { 0, &settings_data.current_mission_id, 4, 1, NULL },
    { 1, g_invasion_warnings, 32, 101, NULL },
    { 0, &settings_data.is_custom_scenario, 4, 1, NULL },
    { 0, &city_sounds, 128, 70, NULL },
    { 0, &building_highest_id, 4, 1, NULL },
    { 0, empire_map_routes.traders, 88, 100, NULL },
    { 0, &empire_map_routes.trader_next_index, 4, 1, NULL },
    { 1, building_list_burning, 2, 500, NULL },
    { 1, building_list_small, 2, 500, NULL },
    { 1, building_list_large, 2, 4000, NULL },
    { 0, &saved_reserved_1, 4, 1, NULL },
    { 0, &scenario_start_flags[SCENARIO_START_FLAG_EMPIRE_LOCKED], 4, 1, NULL },
    { 0, &figure_name_counter_table.by_slot[3], 4, 1, NULL },
    { 0, &figure_name_counter_table.by_slot[2], 4, 1, NULL },
    { 0, &figure_name_counter_table.by_slot[1], 4, 1, NULL },
    { 0, &figure_name_counter_table.by_slot[0], 4, 1, NULL },
    { 0, &city_frogs_countdown, 4, 1, NULL },
    { 0, &pyramid_speedup_countdown, 4, 1, NULL },
    { 0, &map_sweep_total, 4, 1, NULL },
    { 0, &enemy_army_totals.enemy_formations, 4, 1, NULL },
    { 0, &enemy_army_totals.enemy_strength, 4, 1, NULL },
    { 0, &enemy_army_totals.legion_formations, 4, 1, NULL },
    { 0, &enemy_army_totals.legion_strength, 4, 1, NULL },
    { 0, &enemy_army_totals.update_counter, 4, 1, NULL },
    { 0, building_storages, 196, 200, NULL },
    { 1, &trade_route_limits, 144, 20, NULL },
    { 1, &trade_route_traded, 144, 20, NULL },
    { 0, &building_barracks_tower_sentry_request, 4, 1, NULL },
    { 0, &building_extra_sequence, 4, 1, NULL },
    { 0, &routing_stats_unused_b, 4, 1, NULL },
    { 0, &routing_stats_enemy_routes_calculated, 4, 1, NULL },
    { 0, &routing_stats_total_routes_calculated, 4, 1, NULL },
    { 0, &routing_stats_unused_a, 4, 1, NULL },
    { 0, &city_entry_flag_x, 4, 1, NULL },
    { 0, &city_entry_flag_y, 4, 1, NULL },
    { 0, &city_exit_flag_x, 4, 1, NULL },
    { 0, &city_exit_flag_y, 4, 1, NULL },
    { 0, &invasion_last_id, 2, 1, NULL },
    { 0, &building_corrupt_houses_a, 4, 1, NULL },
    { 0, &building_corrupt_houses_b, 4, 1, NULL },
    { 0, scenario_map_name, 65, 1, NULL },
    { 0, &map_bookmarks, 8, 4, NULL },
    { 0, &map_sweep_countdown, 4, 1, NULL },
    { 0, &city_entry_flag_offset, 4, 1, NULL },
    { 0, &city_exit_flag_offset, 4, 1, NULL },
    { 0, &settings_data.saved_with_game, 4, sizeof(settings_saved_with_game) / 4, NULL },
    { 0, map_grids.soil_fertility, 1, 51984, NULL },
    { 0, &scenario_events, 18628, 1, savegame_converter_scenario_events },
    { 0, &ferry_queues, 224, 50, NULL },
    { 0, ferry_path_records, 44, 50, NULL },
    { 0, figure_name_counters_unused, 4, 4, NULL },
    { 0, dock_queues.records, 820, 10, NULL },
    { 1, &empire_header, 32, 40, NULL },
    { 1, &empire_map_objects, 98, 200, NULL },
    { 1, empire_map_routes.routes, 324, 50, NULL },
    { 0, map_grids.vegetation_growth, 1, 51984, NULL },
    { 0, &vegetation_growth_state, 20, 1, NULL },
    { 0, sphinx_spot_objects, 24, 22, NULL },
    { 1, NULL, 0, 0, NULL } };

char scenario_file_magic[8] = "MAPS";

char scenario_file_magic_expected[12] = "MAPS";

chunk_entry scenario_chunk_table[SCENARIO_CHUNKS] = {
    { 0, scenario_file_magic, 4, 1, NULL },
    { 0, &savegame_file_version, 4, 1, NULL },
    { 0, &savegame_schema_chunk_count, 4, 1, NULL },
    { 0, savegame_chunk_schema, 20, 300, NULL },
    { 0, map_grids.images, 4, 51984, NULL },
    { 0, map_grids.edge, 1, 51984, NULL },
    { 0, map_grids.terrain, 4, 51984, NULL },
    { 0, map_grids.bitfields, 1, 51984, NULL },
    { 0, map_grids.random, 1, 51984, NULL },
    { 0, map_grids.elevation, 1, 51984, NULL },
    { 0, &random_generator_state.iv1, 4, 1, NULL },
    { 0, &random_generator_state.iv2, 4, 1, NULL },
    { 0, &map_info.camera_x, 4, 1, NULL },
    { 0, &map_info.camera_y, 4, 1, NULL },
    { 0, &scenario_data, 1592, 1, NULL },
    { 0, map_grids.soil_fertility, 1, 51984, NULL },
    { 0, &scenario_events, 18628, 1, savegame_converter_scenario_events },
    { 1, &empire_header, 32, 40, NULL },
    { 1, &empire_map_objects, 98, 200, NULL },
    { 1, empire_map_routes.routes, 324, 50, NULL },
    { 0, map_grids.vegetation_growth, 1, 51984, NULL },
    { 1, NULL, 0, 0, NULL } };

uint32_t zip_dictionary_size = 4096;

int32_t pk_tables_reference[835] = { 0, 1996959894, -301047508, -1727442502, 124634137, 1886057615, -379345611, -1637575261, 249268274, 2044508324,
    -522852066, -1747789432, 162941995, 2125561021, -407360249, -1866523247, 498536548, 1789927666, -205950648, -2067906082, 450548861, 1843258603, -187386543,
    -2083289657, 325883990, 1684777152, -43845254, -1973040660, 335633487, 1661365465, -99664541, -1928851979, 997073096, 1281953886, -715111964, -1570279054,
    1006888145, 1258607687, -770865667, -1526024853, 901097722, 1119000684, -608450090, -1396901568, 853044451, 1172266101, -589951537, -1412350631, 651767980,
    1373503546, -925412992, -1076862698, 565507253, 1454621731, -809855591, -1195530993, 671266974, 1594198024, -972236366, -1324619484, 795835527, 1483230225,
    -1050600021, -1234817731, 1994146192, 31158534, -1731059524, -271249366, 1907459465, 112637215, -1614814043, -390540237, 2013776290, 251722036, -1777751922,
    -519137256, 2137656763, 141376813, -1855689577, -429695999, 1802195444, 476864866, -2056965928, -228458418, 1812370925, 453092731, -2113342271, -183516073,
    1706088902, 314042704, -1950435094, -54949764, 1658658271, 366619977, -1932296973, -69972891, 1303535960, 984961486, -1547960204, -725929758, 1256170817,
    1037604311, -1529756563, -740887301, 1131014506, 879679996, -1385723834, -631195440, 1141124467, 855842277, -1442165665, -586318647, 1342533948, 654459306,
    -1106571248, -921952122, 1466479909, 544179635, -1184443383, -832445281, 1591671054, 702138776, -1328506846, -942167884, 1504918807, 783551873, -1212326853,
    -1061524307, -306674912, -1698712650, 62317068, 1957810842, -355121351, -1647151185, 81470997, 1943803523, -480048366, -1805370492, 225274430, 2053790376,
    -468791541, -1828061283, 167816743, 2097651377, -267414716, -2029476910, 503444072, 1762050814, -144550051, -2140837941, 426522225, 1852507879, -19653770,
    -1982649376, 282753626, 1742555852, -105259153, -1900089351, 397917763, 1622183637, -690576408, -1580100738, 953729732, 1340076626, -776247311, -1497606297,
    1068828381, 1219638859, -670225446, -1358292148, 906185462, 1090812512, -547295293, -1469587627, 829329135, 1181335161, -882789492, -1134132454, 628085408,
    1382605366, -871598187, -1156888829, 570562233, 1426400815, -977650754, -1296233688, 733239954, 1555261956, -1026031705, -1244606671, 752459403, 1541320221,
    -1687895376, -328994266, 1969922972, 40735498, -1677130071, -351390145, 1913087877, 83908371, -1782625662, -491226604, 2075208622, 213261112, -1831694693,
    -438977011, 2094854071, 198958881, -2032938284, -237706686, 1759359992, 534414190, -2118248755, -155638181, 1873836001, 414664567, -2012718362, -15766928,
    1711684554, 285281116, -1889165569, -127750551, 1634467795, 376229701, -1609899400, -686959890, 1308918612, 956543938, -1486412191, -799009033, 1231636301,
    1047427035, -1362007478, -640263460, 1088359270, 936918000, -1447252397, -558129467, 1202900863, 817233897, -1111625188, -893730166, 1404277552, 615818150,
    -1160759803, -841546093, 1423857449, 601450431, -1285129682, -1000256840, 1567103746, 711928724, -1274298825, -1022587231, 1510334235, 755167117, 84149250,
    100992261, 101058054, 101058054, 101058054, 117900806, 117901063, 117901063, 117901063, 117901063, 117901063, 117901063, 134744072, 134744072, 134744072,
    134744072, 419761411, 1040257289, 906898974, 973481494, 839526938, 37888530, 475806844, 206318700, 341062772, 71574628, 408434808, 138946664, 816869616,
    277893328, 547381472, 8405184, 0, 0, 67305985, 134678021, 50528771, 84149252, 100992261, 117900806, 100729605, 336331274, 805836804, 4202512, 202116107,
    202116108, 201787404, 202114828, 202116108, 202116108, 202181644, 202116108, 201853444, 134876170, 151521031, 134678023, 117900807, 117966599, 201852935,
    185141003, 101123596, 134677766, 151717384, 101058310, 101059335, 134809350, 134940937, 135006475, 101057804, 101057798, 118162694, 84215301, 84216326,
    117966085, 185206792, 202116107, 218959117, 218959117, 218959117, 218959117, 218959117, 218959117, 218959117, 218959117, 218959117, 218959117, 218959117,
    218959117, 202116108, 202116108, 202116108, 202116108, 202116108, 202116108, 202116108, 202116108, 202116108, 202116108, 202116108, 202116108, 218958861,
    218958861, 218958861, 218893581, 202116365, 218959116, 218959117, 218959117, 266339472, 199231456, 232784864, 165676512, 12059104, 249561186, 2229984,
    48237280, 81792224, 14682336, 123735904, 56626016, 90180960, 157291072, 241172832, 174065248, 38797327, 39845944, 207618128, 14156688, 131138, 28311640,
    2687100, 9961532, 589916, 7077916, 4980780, 786456, 15204468, 73400424, 3408016, 118489264, 3213408, 1114196, 1507361, 11010068, 65576, 19923728, 6553662,
    3014686, 84934692, 3538958, 4456470, 13107248, 13631952, 4718864, 22021648, 8912992, 462752, 393254, 1769530, 2752538, 720906, 262672, 3276819, 1900547,
    26214418, 1376269, 1638405, 7864328, 7340272, 68158096, 127926288, 60820384, 473956928, 339741760, 406848576, 272631872, 528482368, 394268544, 461375360,
    327158656, 494928768, 360713600, 427820416, 293603712, 511705472, 377491072, 444597888, 310381184, 478151296, 343936128, 411042944, 276826240, 520093824,
    385879808, 452986624, 318769920, 94375328, 27265440, 111152800, 44042912, 77597856, 10487968, 119541536, 52431648, 85986592, 18876704, 102764064, 35654176,
    69209120, 2099232, 130027456, 62917568, 96472512, 29362624, 113249984, 46140096, 79695040, 12585152, 121638720, 54528832, 222298880, 218111232, 88085760,
    419431680, 155191552, 16781568, 234888704, 369099072, 436209152, 104861248, 167774784, 33559040, 201333760, 67113984, 134223872, 4096, 1096239952,
    1142965586, 543257697, 1886220099, 1936942450, 544108393, 1919052108, 544830049, 544370534, 862873943, 1124732210, 1920561263, 1952999273, 943272224,
    959524153, 1344288057, 1380013899, 1850286149, 538979939, 543976513, 1751607634, 1377858420, 1919251301, 224683382, 1952534538, 544501349, 539914062,
    892349493, 876031025, 1342836021, 1380013899, 1631854661, 1126195572, 1919970671, 1769173861, 1277193839, 1634886249, 1377859954, 539912037, 777203285,
    1952534560, 1851858990, 1834229860, 1716461614, 168636006, 1936876886, 544108393, 825306673, 0, 0, 84149250, 100992261, 101058054, 101058054, 101058054,
    117900806, 117901063, 117901063, 117901063, 117901063, 117901063, 117901063, 134744072, 134744072, 134744072, 134744072, 419761411, 1040257289, 906898974,
    973481494, 839526938, 37888530, 475806844, 206318700, 341062772, 71574628, 408434808, 138946664, 816869616, 277893328, 547381472, 8405184, 0, 0, 67305985,
    134678021, 65536, 196610, 327684, 458758, 655368, 1441806, 4587558, 17170566, 50528771, 84149252, 100992261, 117900806, 100729605, 336331274, 805836804,
    4202512, 202116107, 202116108, 201787404, 202114828, 202116108, 202116108, 202181644, 202116108, 201853444, 134876170, 151521031, 134678023, 117900807,
    117966599, 201852935, 185141003, 101123596, 134677766, 151717384, 101058310, 101059335, 134809350, 134940937, 135006475, 101057804, 101057798, 118162694,
    84215301, 84216326, 117966085, 185206792, 202116107, 218959117, 218959117, 218959117, 218959117, 218959117, 218959117, 218959117, 218959117, 218959117,
    218959117, 218959117, 218959117, 202116108, 202116108, 202116108, 202116108, 202116108, 202116108, 202116108, 202116108, 202116108, 202116108, 202116108,
    202116108, 218958861, 218958861, 218958861, 218893581, 202116365, 218959116, 218959117, 218959117, 266339472, 199231456, 232784864, 165676512, 12059104,
    249561186, 2229984, 48237280, 81792224, 14682336, 123735904, 56626016, 90180960, 157291072, 241172832, 174065248, 38797327, 39845944, 207618128, 14156688,
    131138, 28311640, 2687100, 9961532, 589916, 7077916, 4980780, 786456, 15204468, 73400424, 3408016, 118489264, 3213408, 1114196, 1507361, 11010068, 65576,
    19923728, 6553662, 3014686, 84934692, 3538958, 4456470, 13107248, 13631952, 4718864, 22021648, 8912992, 462752, 393254, 1769530, 2752538, 720906, 262672,
    3276819, 1900547, 26214418, 1376269, 1638405, 7864328, 7340272, 68158096, 127926288, 60820384, 473956928, 339741760, 406848576, 272631872, 528482368,
    394268544, 461375360, 327158656, 494928768, 360713600, 427820416, 293603712, 511705472, 377491072, 444597888, 310381184, 478151296, 343936128, 411042944,
    276826240, 520093824, 385879808, 452986624, 318769920, 94375328, 27265440, 111152800, 44042912, 77597856, 10487968, 119541536, 52431648, 85986592, 18876704,
    102764064, 35654176, 69209120, 2099232, 130027456, 62917568, 96472512, 29362624, 113249984, 46140096, 79695040, 12585152, 121638720, 54528832, 222298880,
    218111232, 88085760, 419431680, 155191552, 16781568, 234888704, 369099072, 436209152, 104861248, 167774784, 33559040, 201333760, 67113984, 134223872, 4096,
    1096239952, 1142965586, 543257697, 1886220099, 1936942450, 544108393, 1919052108, 544830049, 544370534, 862873943, 1124732210, 1920561263, 1952999273,
    943272224, 959524153, 1344288057, 1380013899, 1850286149, 538979939, 543976513, 1751607634, 1377858420, 1919251301, 778331510, 1632635405, 1953391988,
    779046432, 808203552, 925643061, 168637748, 1096239952, 1142965586, 543257697, 1886220099, 1936942450, 544108393, 1919052108, 544830049, 778528082,
    1395545376, 1632641070, 1629498996, 1411409006, 1327509101, 221144678, 1919243786, 1852795251, 825110816, 49 };

static_assert(sizeof(city_sound_table) == 8960, "city_sounds is the savegame's city sounds of 8960 bytes");
city_sound_table city_sounds;

int32_t scenario_climate_current;

int32_t scenario_start_flags[8];

static_assert(sizeof(dock_queue_table) == 8480, "dock_queues is the savegame's dock queues of 8480 bytes");
static_assert(sizeof(dock_queue) * 10 == 280, "dock_queues ends with 10 dock queues of 28 bytes");
dock_queue_table dock_queues;

int32_t city_exit_flag_offset;

int32_t city_entry_flag_offset;

int32_t city_entry_flag_x;

int32_t city_entry_flag_y;

int32_t city_exit_flag_x;

int32_t city_exit_flag_y;

static_assert(sizeof(figure_name_counter_record) == 164, "figure_name_counter_table is the savegame's figure name counters of 164 bytes");
figure_name_counter_record figure_name_counter_table;


int32_t g_footprint_data_off;

static_assert(sizeof(chunk_schema_table) == 6000, "savegame_chunk_schema is the savegame's chunk schema of 6000 bytes");
chunk_schema_table savegame_chunk_schema;

int32_t savegame_file_version;

int32_t savegame_schema_chunk_count;

int32_t buildings_without_id_cleared;

uint32_t zip_compress_type;
