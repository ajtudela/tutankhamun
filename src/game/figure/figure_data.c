// The built-in data of the figures: formation layouts, corpse and missile frames, the action table, the explosion
// cloud, the enemies' attack priorities and graphics names, and the debug names of the figure types.
// figure/figure_data.h declares them.

#include "figure/figure_data.h"
#include "figure/animals_internal.h"
#include "figure/carriers.h"
#include "figure/natives.h"
#include "figure/walkers.h"
#include "figure/military.h"
#include "figure/trade.h"
#include "figure/protestor.h"
#include "figure/boats.h"
#include "figure/shipwreck.h"
#include "editor/editor_new_map_tiles.h"
#include "figure/enemy.h"
#include "figure/figure_type.h"
#include "figure/missiles.h"
#include "figure/animals_wild.h"
#include "figure/animals.h"
#include "figure/construction_worker.h"
#include "figure/laborer.h"
#include "figure/enemy_ships.h"
#include "figure/plagues.h"
#include "figure/action.h"

#include <assert.h>

formation_layout_spot enemy_formation_layout_offsets[FORMATION_LAYOUTS][FORMATION_LAYOUT_SPOTS] = {
    {
        { 0, 0 }, { 1, 0 }, { 0, 1 }, { 1, 1 }, { -1, 0 }, { -1, 1 }, { 0, -1 }, { 1, -1 },
        { -1, -1 }, { 2, -1 }, { 2, 0 }, { 2, 1 }, { 0, 2 }, { 1, 2 }, { -1, 2 }, { 2, 2 }
    },
    {
        { 0, 0 }, { 0, 1 }, { -1, 0 }, { 1, 0 }, { -1, 1 }, { 1, 1 }, { -2, 0 }, { -2, 1 },
        { 2, 0 }, { 2, 1 }, { -3, 0 }, { -3, 1 }, { 3, 0 }, { 3, 1 }, { -4, 0 }, { -4, 1 }
    },
    {
        { 0, 0 }, { 0, -1 }, { 0, 1 }, { 1, 0 }, { 1, -1 }, { 1, 1 }, { 0, -2 }, { 1, -2 },
        { 0, 2 }, { 1, 2 }, { 0, -3 }, { 1, -3 }, { 0, 3 }, { 1, 3 }, { 0, -4 }, { 1, -4 }
    },
    {
        { 0, 0 }, { 2, 0 }, { -2, 0 }, { 1, 1 }, { -1, 1 }, { 3, 1 }, { -3, 1 }, { 4, 0 },
        { -4, 0 }, { 5, 1 }, { 6, 0 }, { -5, 1 }, { -6, 0 }, { 7, 1 }, { 8, 0 }, { -7, 1 }
    },
    {
        { 0, 0 }, { 0, -2 }, { 0, 2 }, { 1, -1 }, { 1, 1 }, { 1, -3 }, { 1, 3 }, { 0, -4 },
        { 0, 4 }, { 1, -5 }, { 0, -6 }, { 1, 5 }, { 0, 6 }, { 1, -7 }, { 0, -8 }, { 1, 7 }
    },
    {
        { 0, 0 }, { 0, -1 }, { 1, 0 }, { 0, 1 }, { -1, 0 }, { 1, -1 }, { 1, 1 }, { -1, 1 },
        { -1, -1 }, { 1, -1 }, { 1, 1 }, { -1, 1 }, { -1, -1 }, { 1, 0 }, { -1, 0 }, { 0, 0 }
    },
    {
        { 0, 0 }, { 1, 0 }, { 0, 1 }, { 1, 1 }, { 2, 0 }, { 2, 1 }, { 1, 2 }, { 0, 2 },
        { 2, 2 }, { 3, 0 }, { 3, 1 }, { 3, 2 }, { 1, 3 }, { 2, 3 }, { 0, 3 }, { 3, 3 }
    },
    {
        { 0, 0 }, { 1, 0 }, { 0, 1 }, { 1, 1 }, { 2, 0 }, { 2, 1 }, { 1, 2 }, { 0, 2 },
        { 2, 2 }, { 3, 0 }, { 3, 1 }, { 3, 2 }, { 1, 3 }, { 2, 3 }, { 0, 3 }, { 3, 3 }
    },
    {
        { 0, 0 }, { 1, -1 }, { 0, 1 }, { 0, 0 }, { 1, 0 }, { -1, 1 }, { 2, 1 }, { -1, -1 },
        { 1, -1 }, { 0, 1 }, { 1, 0 }, { 0, 2 }, { 1, 1 }, { -1, 1 }, { 1, -2 }, { -1, 1 }
    },
    {
        { 0, 0 }, { 2, 1 }, { -1, -1 }, { 1, 1 }, { 1, 0 }, { -1, 1 }, { 3, 1 }, { -2, -1 },
        { 0, 2 }, { -4, 0 }, { -1, 3 }, { 0, 3 }, { 1, 4 }, { 4, 0 }, { 2, 3 }, { -3, 2 }
    },
    {
        { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
        { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
    },
    {
        { 0, 0 }, { 2, 0 }, { 0, 2 }, { 2, 2 }, { -2, 0 }, { -2, 2 }, { 0, -2 }, { 2, -2 },
        { -2, -2 }, { 4, -2 }, { 4, 0 }, { 4, 2 }, { 0, 4 }, { 2, 4 }, { -2, 4 }, { 4, 4 }
    },
    {
        { 0, 0 }, { 1, 0 }, { 0, 1 }, { 1, 1 }, { 2, 0 }, { 2, 1 }, { 1, 2 }, { 0, 2 },
        { 2, 2 }, { 3, 0 }, { 3, 1 }, { 3, 2 }, { 1, 3 }, { 2, 3 }, { 0, 3 }, { 3, 3 }
    },
    {
        { 0, 0 }, { 0, 2 }, { -1, 0 }, { 1, 0 }, { -1, 2 }, { 1, 2 }, { -2, 0 }, { -2, 2 },
        { 2, 0 }, { 2, 2 }, { -3, 0 }, { -3, 2 }, { 3, 0 }, { 3, 2 }, { -4, 0 }, { -4, 2 }
    },
    {
        { 0, 0 }, { 0, -1 }, { 0, 1 }, { 2, 0 }, { 2, -1 }, { 2, 1 }, { 0, -2 }, { 2, -2 },
        { 0, 2 }, { 2, 2 }, { 0, -3 }, { 2, -3 }, { 0, 3 }, { 2, 3 }, { 0, -4 }, { 2, -4 }
    },
    {
        { 0, 0 }, { 1, 0 }, { 0, 1 }, { 1, 1 }, { 2, 0 }, { 2, 1 }, { 1, 2 }, { 0, 2 },
        { 2, 2 }, { 3, 0 }, { 3, 1 }, { 3, 2 }, { 1, 3 }, { 2, 3 }, { 0, 3 }, { 3, 3 }
    },
    {
        { 0, 0 }, { 1, 0 }, { 0, 1 }, { 1, 1 }, { 2, 0 }, { 2, 1 }, { 1, 2 }, { 0, 2 },
        { 2, 2 }, { 3, 0 }, { 3, 1 }, { 3, 2 }, { 1, 3 }, { 2, 3 }, { 0, 3 }, { 3, 3 }
    },
};

uint8_t figure_corpse_offsets[256] = { 0, 1, 2, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 0, 1, 2, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4 };

uint8_t figure_missile_frames[336] = { 0, 1, 2, 3, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 7, 0, 0, 0, 10, 0, 0, 0, 8, 0, 0, 0, 8, 0, 0, 0, 9, 0, 0, 0, 5, 0, 0, 0, 6, 0, 0, 0, 3, 0, 0, 0, 3, 0, 0, 0,
    3, 0, 0, 0, 2, 0, 0, 0, 2, 0, 0, 0, 4, 0, 0, 0, 4, 0, 0, 0, 4, 0, 0, 0, 6, 0, 0, 0, 1, 0, 0, 0, 6 };

int32_t figure_type_coverage_tag[110] = { -1, -1, -1, -1, -1, 7, -1, 7, 2, -1, 1, -1, -1, -1, -1, 4, 4, 4, 4, -1, -1, -1, -1, -1, -1, -1, 8, 6, -1, 5, 5, 9, 9,
    9, 9, -1, -1, -1, -1, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 6, -1, 8, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 3, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 4, -1, -1, -1, -1 };

int8_t figure_cart_offsets[16] = { 13, -7, 18, -1, 12, 7, 0, 11, -13, 6, -18, -1, -13, -7, 0, -12 };

uint8_t figure_ship_direction_table[64] = { 5, 3, 1, 2, 4, 2, 1, 3, 3, 5, 3, 1, 2, 4, 2, 1, 1, 3, 5, 3, 1, 2, 4, 2, 2, 1, 3, 5, 3, 1, 2, 4, 4, 2, 1, 3, 5, 3, 1,
    2, 2, 4, 2, 1, 3, 5, 3, 1, 1, 2, 4, 2, 1, 3, 4, 3, 3, 1, 2, 4, 3, 1, 3, 5 };

global_function figure_action_table[110] = {
    [FIGURE_NONE] = figure_action_none,
    [FIGURE_IMMIGRANT] = figure_immigrant_action,
    [FIGURE_EMIGRANT] = figure_emigrant_action,
    [FIGURE_HOMELESS] = figure_homeless_action,
    [FIGURE_CART_PUSHER] = figure_cartpusher_action,
    [FIGURE_LABOR_SEEKER] = figure_labor_seeker_action,
    [FIGURE_EXPLOSION] = figure_explosion_cloud_action,
    [FIGURE_TAX_COLLECTOR] = figure_tax_collector_action,
    [FIGURE_ARCHITECT] = figure_engineer_action,
    [FIGURE_STORAGE_YARD_CART] = figure_warehouseman_action,
    [FIGURE_FIREMAN] = figure_fireman_action,
    [FIGURE_ARCHER] = figure_soldier_action,
    [FIGURE_INFANTRY] = figure_soldier_action,
    [FIGURE_CHARIOTEER] = figure_soldier_action,
    [FIGURE_STANDARD_BEARER] = figure_military_standard_action,
    [FIGURE_JUGGLER] = figure_entertainer_action,
    [FIGURE_MUSICIAN] = figure_entertainer_action,
    [FIGURE_DANCER] = figure_entertainer_action,
    [FIGURE_SENET_PLAYER] = figure_entertainer_action,
    [FIGURE_TRADE_CARAVAN] = figure_trade_caravan_action,
    [FIGURE_TRADE_SHIP] = figure_trade_ship_action,
    [FIGURE_TRADE_CARAVAN_DONKEY] = figure_trade_caravan_donkey_action,
    [FIGURE_PROTESTER] = figure_protestor_action,
    [FIGURE_CRIMINAL] = figure_criminal_action,
    [FIGURE_TOMB_ROBBER] = figure_tomb_robber_action,
    [FIGURE_FISHING_BOAT] = figure_fishing_boat_action,
    [FIGURE_MARKET_TRADER] = figure_market_trader_action,
    [FIGURE_PRIEST] = figure_priest_action,
    [FIGURE_BUBBLES] = figure_bubbles_action,
    [FIGURE_TEACHER] = figure_teacher_action,
    [FIGURE_LIBRARIAN] = figure_librarian_action,
    [FIGURE_DENTIST] = figure_dentist_action,
    [FIGURE_PHYSICIAN] = figure_physician_action,
    [FIGURE_HERBALIST] = figure_apothecary_action,
    [FIGURE_EMBALMER] = figure_embalmer_action,
    [FIGURE_WORKER] = figure_worker_action,
    [FIGURE_MAP_FLAG] = figure_editor_flag_action,
    [FIGURE_FLOTSAM] = figure_flotsam_action,
    [FIGURE_DOCKER] = figure_docker_action,
    [FIGURE_MARKET_BUYER] = figure_market_buyer_action,
    [FIGURE_PATRICIAN] = figure_patrician_action,
    [FIGURE_NATIVE] = figure_indigenous_native_action,
    [FIGURE_TOWER_SENTRY] = figure_tower_sentry_action,
    [FIGURE_ENEMY_ARCHER] = figure_enemy_archer_action,
    [FIGURE_ENEMY_INFANTRY] = figure_enemy_infantry_action,
    [FIGURE_ENEMY_CHARIOT] = figure_enemy_chariot_action,
    [FIGURE_ENEMY_CAMEL] = figure_enemy_camel_action,
    [FIGURE_ENEMY_ELEPHANT] = figure_enemy_elephant_action,
    [FIGURE_ENEMY_BARBARIAN_CHARIOT] = figure_enemy_barbarian_chariot_action,
    [FIGURE_ENEMY_BARBARIAN_LIGHT] = figure_enemy_barbarian_light_action,
    [FIGURE_ENEMY_BARBARIAN_HEAVY] = figure_enemy_barbarian_heavy_action,
    [FIGURE_ENEMY_BARBARIAN_ARCHER] = figure_enemy_barbarian_archer_action,
    [FIGURE_ENEMY_BARBARIAN_MOUNTED] = figure_enemy_barbarian_mounted_action,
    [FIGURE_ENEMY_BARBARIAN_HEAVY_2] = figure_enemy_barbarian_heavy_2_action,
    [FIGURE_ENEMY_EGYPTIAN_INFANTRY] = figure_enemy_egyptian_infantry_action,
    [FIGURE_ENEMY_EGYPTIAN_ARCHER] = figure_enemy_egyptian_archer_action,
    [FIGURE_ENEMY_EGYPTIAN_CHARIOT] = figure_enemy_egyptian_chariot_action,
    [FIGURE_KINGDOM_SOLDIER] = figure_kingdom_soldier_action,
    [FIGURE_NATIVE_TRADER] = figure_native_trader_action,
    [FIGURE_ARROW] = figure_arrow_action,
    [FIGURE_JAVELIN] = figure_javelin_action,
    [FIGURE_BOLT] = figure_bolt_action,
    [FIGURE_BALLISTA] = figure_ballista_action,
    [FIGURE_CREATURE] = figure_creature_action,
    [FIGURE_MISSIONARY] = figure_missionary_action,
    [FIGURE_SEAGULLS] = figure_seagulls_action,
    [FIGURE_DELIVERY_BOY] = figure_delivery_boy_action,
    [FIGURE_SHIPWRECK] = figure_shipwreck_action,
    [FIGURE_BIRDS] = figure_birds_action,
    [FIGURE_OSTRICH] = figure_ostrich_action,
    [FIGURE_ANTELOPE] = figure_antelope_action,
    [FIGURE_SPEAR] = figure_spear_action,
    [FIGURE_CHARIOT_RACER] = figure_action_none,
    [FIGURE_HUNTER] = figure_hunter_action,
    [FIGURE_HUNTER_SPEAR] = figure_hunter_spear_action,
    [FIGURE_LUMBERJACK] = figure_lumberjack_action,
    [FIGURE_FERRY_BOAT] = figure_ferry_boat_action,
    [FIGURE_TRANSPORT_SHIP] = figure_transport_ship_action,
    [FIGURE_WARSHIP] = figure_warship_action,
    [FIGURE_CARPENTER] = figure_carpenter_action,
    [FIGURE_BRICKLAYER] = figure_bricklayer_action,
    [FIGURE_STONEMASON] = figure_stonemason_action,
    [FIGURE_CROCODILE] = figure_crocodile_action,
    [FIGURE_HYENA] = figure_hyena_action,
    [FIGURE_HIPPO] = figure_hippo_action,
    [FIGURE_LABORER] = figure_laborer_action,
    [FIGURE_SLED] = figure_sled_action,
    [FIGURE_WATER_CARRIER] = figure_water_carrier_action,
    [FIGURE_CONSTABLE] = figure_constable_action,
    [FIGURE_MAGISTRATE] = figure_magistrate_action,
    [FIGURE_REED_GATHERER] = figure_reed_gatherer_action,
    [FIGURE_FESTIVAL_PRIEST] = figure_festival_priest_action,
    [FIGURE_ENEMY_TRANSPORT_SHIP] = figure_enemy_transport_action,
    [FIGURE_ENEMY_WARSHIP] = figure_enemy_warship_action,
    [FIGURE_FUNERAL_WALKER] = figure_action_none,
    [FIGURE_FISHING_SPOT] = figure_fishing_spot_action,
    [FIGURE_SLED_PULLER] = figure_sled_puller_action,
    [FIGURE_SHOWMAN] = figure_showman_action,
    [FIGURE_PLAGUED_CITIZEN] = figure_plague_citizen_action,
    [FIGURE_BEDOUIN] = figure_bedouin_action,
    [FIGURE_ENEMY_EGYPTIAN_WARSHIP] = figure_egyptian_warship_action,
    [FIGURE_ENEMY_EGYPTIAN_TRANSPORT_SHIP] = figure_egyptian_transport_action,
    [FIGURE_ASP] = figure_asp_action,
    [FIGURE_LION] = figure_lion_action,
    [FIGURE_SCORPION] = figure_scorpion_action,
    [FIGURE_ZOOKEEPER] = figure_entertainer_action,
    [FIGURE_FROG] = figure_frog_action,
    [FIGURE_LOCUST] = figure_locust_action,
    [FIGURE_TOMB_ARTISAN] = figure_tomb_artisan_action,
    [FIGURE_MUMMY] = figure_mummy_action,
};

const char *figure_type_debug_names[110] = { str_fig_null, str_fig_immigrant, str_fig_emmigrant, str_fig_vagrant, str_fig_cart, str_fig_ghost, str_fig_explosion,
    str_fig_taxman, str_fig_engineer, str_fig_warehouseman, str_fig_fireman, str_fig_leg_miss, str_fig_leg_aux, str_fig_leg_heavy, str_fig_standard,
    str_fig_juggler, str_fig_musician, str_fig_dancer, str_fig_bullfighter, str_fig_caravan, str_fig_junk, str_fig_van_follower, str_fig_simple_crime,
    str_fig_thief, str_fig_tomb_robber, str_fig_fishing_boat, str_fig_market_girl, str_fig_priest, str_fig_bubbles, str_fig_teacher, str_fig_librarian,
    str_fig_dentist, str_fig_physician, str_fig_apothecary, str_fig_embalmer, str_fig_worker, str_fig_map_flag, str_fig_flotsam, str_fig_docker, str_fig_buyer,
    str_fig_patrician, str_fig_barbarian, str_fig_wallguy, str_fig_enemy_miss, str_fig_enemy_aux, str_fig_enemy_heavy, str_fig_enemy_camel,
    str_fig_enemy_elephant, str_fig_barb_chariot, str_fig_barb_light, str_fig_barb_heavy, str_fig_barb_miss, str_fig_barb_horse, str_fig_barb_heavy2,
    str_fig_egypt_aux, str_fig_egypt_miss, str_fig_egypt_heavy, str_fig_enemy_leg_heavy, str_fig_barb_donkey, str_fig_arrow, str_fig_player_missile,
    str_fig_bolt, str_fig_ballista, str_fig_creature, str_fig_missionary, str_fig_seagulls, str_fig_granary_boy, str_fig_shipwreck, str_fig_birds,
    str_fig_ostrich, str_fig_antelope, str_fig_enemy_missile, str_fig_chariot_racers, str_fig_hunter, str_fig_hunter_spear, str_fig_lumberjack,
    str_fig_ferry_boat, str_fig_transport, str_fig_warship, str_fig_carpenter, str_fig_brick_layer, str_fig_stone_mason, str_fig_croc, str_fig_hyena,
    str_fig_hippo, str_fig_laborer, str_fig_pulled_stone, str_fig_water_carrier, str_fig_policeman, str_fig_magistrate, str_fig_reed_cutter,
    str_fig_festival_priest, str_fig_enemy_transport, str_fig_enemy_warship, str_fig_funeral_walker, str_fig_jumping_fish, str_fig_stone_puller,
    str_fig_showman, str_fig_plague_dude, str_fig_bedouin, str_fig_egypt_warship, str_fig_egypt_transport, str_fig_asp, str_fig_lion, str_fig_scorpion,
    str_fig_zookeeper, str_fig_frog, str_fig_locust, str_fig_artisan, str_fig_mummy };

int32_t military_size_tile_offsets[EXPLOSION_SIZES] = { 0, 0, 0, 1, 1, 2 };

const int32_t explosion_size_cloud_offsets[EXPLOSION_SIZES] = { 0, 7, 14, 7, 14, 7 };

const int32_t explosion_cloud_drifts[EXPLOSION_CLOUDS][2] = {
    { 0, -6 }, { -2, -5 }, { -4, -4 }, { -5, -2 }, { -6, 0 }, { -5, -2 }, { -4, -4 }, { -2, -5 },
    { 0, -6 }, { -2, -5 }, { -4, -4 }, { -5, -2 }, { -6, 0 }, { -5, -2 }, { -4, -4 }, { -2, -5 }
};

const int32_t explosion_cloud_speeds[EXPLOSION_CLOUDS] = { 1, 2, 1, 3, 2, 4, 3, 2, 1, 4, 2, 3, 4, 1, 3, 1 };

const formation_layout_spot enemy_formation_spots[ENEMY_FORMATION_LAYOUTS][ENEMY_FORMATION_SPOT_DIRECTIONS][ENEMY_FORMATION_SPOTS] = {
    {
        {
            { 0, 0 }, { -3, 0 }, { 3, 0 }, { -8, 0 }, { 8, 0 }, { -3, 8 }, { 3, 8 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        },
        {
            { 0, 0 }, { 0, -3 }, { 0, 3 }, { 0, -8 }, { 0, 8 }, { 8, -3 }, { 8, 3 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        },
        {
            { 0, 0 }, { -3, 0 }, { 3, 0 }, { -8, 0 }, { 8, 0 }, { -3, -8 }, { 3, -8 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        },
        {
            { 0, 0 }, { 0, -3 }, { 0, 3 }, { 0, -8 }, { 0, 8 }, { -8, -3 }, { -8, 3 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        },
    },
    {
        {
            { 0, 0 }, { -6, 0 }, { 6, 0 }, { -6, 2 }, { 6, 2 }, { -2, 4 }, { 4, 6 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        },
        {
            { 0, 0 }, { 0, -6 }, { 0, 6 }, { 2, -6 }, { 2, 6 }, { 4, -2 }, { 6, 4 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        },
        {
            { 0, 0 }, { -6, 0 }, { 6, 0 }, { -6, -2 }, { 6, -2 }, { -4, -6 }, { 4, -6 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        },
        {
            { 0, 0 }, { 0, -6 }, { 0, 6 }, { -2, -6 }, { -2, 6 }, { -6, -4 }, { -6, 4 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        },
    },
    {
        {
            { 0, 0 }, { -6, 0 }, { 6, 0 }, { -6, 2 }, { 6, 2 }, { -2, 4 }, { 4, 6 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        },
        {
            { 0, 0 }, { 0, -6 }, { 0, 6 }, { 2, -6 }, { 2, 6 }, { 4, -2 }, { 6, 4 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        },
        {
            { 0, 0 }, { -6, 0 }, { 6, 0 }, { -6, -2 }, { 6, -2 }, { -4, -6 }, { 4, -6 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        },
        {
            { 0, 0 }, { 0, -6 }, { 0, 6 }, { -2, -6 }, { -2, 6 }, { -6, -4 }, { -6, 4 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        },
    },
    {
        {
            { 0, 0 }, { -6, 0 }, { 6, 0 }, { -6, 2 }, { 6, 2 }, { -2, 4 }, { 4, 6 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        },
        {
            { 0, 0 }, { 0, -6 }, { 0, 6 }, { 2, -6 }, { 2, 6 }, { 4, -2 }, { 6, 4 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        },
        {
            { 0, 0 }, { -6, 0 }, { 6, 0 }, { -6, -2 }, { 6, -2 }, { -4, -6 }, { 4, -6 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        },
        {
            { 0, 0 }, { 0, -6 }, { 0, 6 }, { -2, -6 }, { -2, 6 }, { -6, -4 }, { -6, 4 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        },
    },
    {
        {
            { 0, 0 }, { -6, 0 }, { 6, 0 }, { -6, 2 }, { 6, 2 }, { -2, 4 }, { 4, 6 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        },
        {
            { 0, 0 }, { 0, -6 }, { 0, 6 }, { 2, -6 }, { 2, 6 }, { 4, -2 }, { 6, 4 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        },
        {
            { 0, 0 }, { -6, 0 }, { 6, 0 }, { -6, -2 }, { 6, -2 }, { -4, -6 }, { 4, -6 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        },
        {
            { 0, 0 }, { 0, -6 }, { 0, 6 }, { -2, -6 }, { -2, 6 }, { -6, -4 }, { -6, 4 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        },
    },
    {
        {
            { 0, 0 }, { -3, 0 }, { 3, 0 }, { -8, 0 }, { 8, 0 }, { -3, 8 }, { 3, 8 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        },
        {
            { 0, 0 }, { 0, -3 }, { 0, 3 }, { 0, -8 }, { 0, 8 }, { 8, -3 }, { 8, 3 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        },
        {
            { 0, 0 }, { -3, 0 }, { 3, 0 }, { -8, 0 }, { 8, 0 }, { -3, -8 }, { 3, -8 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        },
        {
            { 0, 0 }, { 0, -3 }, { 0, 3 }, { 0, -8 }, { 0, 8 }, { -8, -3 }, { -8, 3 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        },
    },
    {
        {
            { 0, 0 }, { -3, 0 }, { 3, 0 }, { -8, 0 }, { 8, 0 }, { -3, 8 }, { 3, 8 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        },
        {
            { 0, 0 }, { 0, -3 }, { 0, 3 }, { 0, -8 }, { 0, 8 }, { 8, -3 }, { 8, 3 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        },
        {
            { 0, 0 }, { -3, 0 }, { 3, 0 }, { -8, 0 }, { 8, 0 }, { -3, -8 }, { 3, -8 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        },
        {
            { 0, 0 }, { 0, -3 }, { 0, 3 }, { 0, -8 }, { 0, 8 }, { -8, -3 }, { -8, 3 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        },
    },
    {
        {
            { 0, 0 }, { -3, 0 }, { 3, 0 }, { -8, 0 }, { 8, 0 }, { -3, 8 }, { 3, 8 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        },
        {
            { 0, 0 }, { 0, -3 }, { 0, 3 }, { 0, -8 }, { 0, 8 }, { 8, -3 }, { 8, 3 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        },
        {
            { 0, 0 }, { -3, 0 }, { 3, 0 }, { -8, 0 }, { 8, 0 }, { -3, -8 }, { 3, -8 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        },
        {
            { 0, 0 }, { 0, -3 }, { 0, 3 }, { 0, -8 }, { 0, 8 }, { -8, -3 }, { -8, 3 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        },
    },
    {
        {
            { 0, 0 }, { -3, 0 }, { 3, 0 }, { -8, 0 }, { 8, 0 }, { -3, 8 }, { 3, 8 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        },
        {
            { 0, 0 }, { 0, -3 }, { 0, 3 }, { 0, -8 }, { 0, 8 }, { 8, -3 }, { 8, 3 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        },
        {
            { 0, 0 }, { -3, 0 }, { 3, 0 }, { -8, 0 }, { 8, 0 }, { -3, -8 }, { 3, -8 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        },
        {
            { 0, 0 }, { 0, -3 }, { 0, 3 }, { 0, -8 }, { 0, 8 }, { -8, -3 }, { -8, 3 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        },
    },
    {
        {
            { 0, 0 }, { -3, 0 }, { 3, 0 }, { -8, 0 }, { 8, 0 }, { -3, 8 }, { 3, 8 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        },
        {
            { 0, 0 }, { 0, -3 }, { 0, 3 }, { 0, -8 }, { 0, 8 }, { 8, -3 }, { 8, 3 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        },
        {
            { 0, 0 }, { -3, 0 }, { 3, 0 }, { -8, 0 }, { 8, 0 }, { -3, -8 }, { 3, -8 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        },
        {
            { 0, 0 }, { 0, -3 }, { 0, 3 }, { 0, -8 }, { 0, 8 }, { -8, -3 }, { -8, 3 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        },
    },
    {
        {
            { 0, 0 }, { -3, 0 }, { 3, 0 }, { -8, 0 }, { 8, 0 }, { -3, 8 }, { 3, 8 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        },
        {
            { 0, 0 }, { 0, -3 }, { 0, 3 }, { 0, -8 }, { 0, 8 }, { 8, -3 }, { 8, 3 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        },
        {
            { 0, 0 }, { -3, 0 }, { 3, 0 }, { -8, 0 }, { 8, 0 }, { -3, -8 }, { 3, -8 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        },
        {
            { 0, 0 }, { 0, -3 }, { 0, 3 }, { 0, -8 }, { 0, 8 }, { -8, -3 }, { -8, 3 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        },
    },
    {
        {
            { 0, 0 }, { -4, 0 }, { 4, 0 }, { -12, 0 }, { 12, 0 }, { -4, 12 }, { 4, 12 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        },
        {
            { 0, 0 }, { 0, -4 }, { 0, 4 }, { 0, -12 }, { 0, 12 }, { 12, -4 }, { 12, 4 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        },
        {
            { 0, 0 }, { -4, 0 }, { 4, 0 }, { -12, 0 }, { 12, 0 }, { -4, -12 }, { 4, -12 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        },
        {
            { 0, 0 }, { 0, -4 }, { 0, 4 }, { 0, -12 }, { 0, 12 }, { -12, -4 }, { -12, 4 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        },
    },
    {
        {
            { 0, 0 }, { -3, 0 }, { 3, 0 }, { -8, 0 }, { 8, 0 }, { -3, 8 }, { 3, 8 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        },
        {
            { 0, 0 }, { 0, -3 }, { 0, 3 }, { 0, -8 }, { 0, 8 }, { 8, -3 }, { 8, 3 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        },
        {
            { 0, 0 }, { -3, 0 }, { 3, 0 }, { -8, 0 }, { 8, 0 }, { -3, -8 }, { 3, -8 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        },
        {
            { 0, 0 }, { 0, -3 }, { 0, 3 }, { 0, -8 }, { 0, 8 }, { -8, -3 }, { -8, 3 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        },
    },
};

int16_t enemy_attack_priority[400] = { 71, 72, 70, 191, 193, 190, 102, 103, 100, 101, 105, 194, 224, 104, 196, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 85, 84, 87, 86, 184, 187, 188, 189, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 187, 188, 189, 79, 78, 77, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11,
    10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 94, 185, 186, 55 };

int16_t enemy_attack_fallback[100] = { 187, 188, 189, 79, 78, 77, 29, 28, 27, 26, 25, 85, 84, 32, 33, 98, 65, 66, 67, 68, 69, 87, 86, 184, 30, 31, 47, 46, 53,
    51, 24, 23, 22, 21, 20, 206, 114, 177, 178, 179, 231, 113, 112, 111, 110, 71, 72, 70, 74, 75, 76, 60, 61, 62, 63, 64, 34, 36, 37, 35, 94, 185, 186, 19, 18,
    17, 16, 15, 49, 106, 107, 216, 161, 217, 162, 109, 108, 90, 102, 103, 104, 101, 100, 194, 105, 224, 55, 81, 180, 92, 14, 13, 12, 11, 10 };

const char *enemy_graphics_names[12] = { str_enemy_hyksos, str_enemy_seapeople, str_enemy_hittite, str_enemy_mitani, str_enemy_kushite, str_enemy_libian,
    str_enemy_nubian, str_enemy_canaanite, str_enemy_assyrian, str_enemy_roman, str_enemy_phoenician, str_enemy_persian };

const char str_enemy_persian[8] = "PERSIAN";
const char str_enemy_phoenician[12] = "PHOENICIAN";
const char str_enemy_roman[8] = "ROMAN";
const char str_enemy_assyrian[12] = "ASSYRIAN";
const char str_enemy_canaanite[12] = "CANAANITE";
const char str_enemy_nubian[8] = "NUBIAN";
const char str_enemy_libian[8] = "LIBIAN";
const char str_enemy_kushite[8] = "KUSHITE";
const char str_enemy_mitani[8] = "MITANI";
const char str_enemy_hittite[8] = "HITTITE";
const char str_enemy_seapeople[12] = "SEAPEOPLE";
const char str_enemy_hyksos[8] = "HYKSOS";
const char str_fig_mummy[12] = "FIG_MUMMY";
const char str_fig_artisan[12] = "FIG_ARTISAN";
const char str_fig_locust[12] = "FIG_LOCUST";
const char str_fig_frog[12] = "FIG_FROG";
const char str_fig_zookeeper[16] = "FIG_ZOOKEEPER";
const char str_fig_scorpion[16] = "FIG_SCORPION";
const char str_fig_lion[12] = "FIG_LION";
const char str_fig_asp[8] = "FIG_ASP";
const char str_fig_egypt_transport[20] = "FIG_EGYPT_TRANSPORT";
const char str_fig_egypt_warship[20] = "FIG_EGYPT_WARSHIP";
const char str_fig_bedouin[12] = "FIG_BEDOUIN";
const char str_fig_plague_dude[16] = "FIG_PLAGUE_DUDE";
const char str_fig_showman[12] = "FIG_SHOWMAN";
const char str_fig_stone_puller[20] = "FIG_STONE_PULLER";
const char str_fig_jumping_fish[20] = "FIG_JUMPING_FISH";
const char str_fig_funeral_walker[20] = "FIG_FUNERAL_WALKER";
const char str_fig_enemy_warship[20] = "FIG_ENEMY_WARSHIP";
const char str_fig_enemy_transport[20] = "FIG_ENEMY_TRANSPORT";
const char str_fig_festival_priest[20] = "FIG_FESTIVAL_PRIEST";
const char str_fig_reed_cutter[16] = "FIG_REED_CUTTER";
const char str_fig_magistrate[16] = "FIG_MAGISTRATE";
const char str_fig_policeman[16] = "FIG_POLICEMAN";
const char str_fig_water_carrier[20] = "FIG_WATER_CARRIER";
const char str_fig_pulled_stone[20] = "FIG_PULLED_STONE";
const char str_fig_laborer[12] = "FIG_LABORER";
const char str_fig_hippo[12] = "FIG_HIPPO";
const char str_fig_hyena[12] = "FIG_HYENA";
const char str_fig_croc[12] = "FIG_CROC";
const char str_fig_stone_mason[16] = "FIG_STONE_MASON";
const char str_fig_brick_layer[16] = "FIG_BRICK_LAYER";
const char str_fig_carpenter[16] = "FIG_CARPENTER";
const char str_fig_warship[12] = "FIG_WARSHIP";
const char str_fig_transport[16] = "FIG_TRANSPORT";
const char str_fig_ferry_boat[16] = "FIG_FERRY_BOAT";
const char str_fig_lumberjack[16] = "FIG_LUMBERJACK";
const char str_fig_hunter_spear[20] = "FIG_HUNTER_SPEAR";
const char str_fig_hunter[12] = "FIG_HUNTER";
const char str_fig_chariot_racers[20] = "FIG_CHARIOT_RACERS";
const char str_fig_enemy_missile[20] = "FIG_ENEMY_MISSILE";
const char str_fig_antelope[16] = "FIG_ANTELOPE";
const char str_fig_ostrich[12] = "FIG_OSTRICH";
const char str_fig_birds[12] = "FIG_BIRDS";
const char str_fig_shipwreck[16] = "FIG_SHIPWRECK";
const char str_fig_granary_boy[16] = "FIG_GRANARY_BOY";
const char str_fig_seagulls[16] = "FIG_SEAGULLS";
const char str_fig_missionary[16] = "FIG_MISSIONARY";
const char str_fig_creature[16] = "FIG_CREATURE";
const char str_fig_ballista[16] = "FIG_BALLISTA";
const char str_fig_bolt[12] = "FIG_BOLT";
const char str_fig_player_missile[20] = "FIG_PLAYER_MISSILE";
const char str_fig_arrow[12] = "FIG_ARROW";
const char str_fig_barb_donkey[16] = "FIG_BARB_DONKEY";
const char str_fig_enemy_leg_heavy[20] = "FIG_ENEMY_LEG_HEAVY";
const char str_fig_egypt_heavy[16] = "FIG_EGYPT_HEAVY";
const char str_fig_egypt_miss[16] = "FIG_EGYPT_MISS";
const char str_fig_egypt_aux[16] = "FIG_EGYPT_AUX";
const char str_fig_barb_heavy2[16] = "FIG_BARB_HEAVY2";
const char str_fig_barb_horse[16] = "FIG_BARB_HORSE";
const char str_fig_barb_miss[16] = "FIG_BARB_MISS";
const char str_fig_barb_heavy[16] = "FIG_BARB_HEAVY";
const char str_fig_barb_light[16] = "FIG_BARB_LIGHT";
const char str_fig_barb_chariot[20] = "FIG_BARB_CHARIOT";
const char str_fig_enemy_elephant[20] = "FIG_ENEMY_ELEPHANT";
const char str_fig_enemy_camel[16] = "FIG_ENEMY_CAMEL";
const char str_fig_enemy_heavy[16] = "FIG_ENEMY_HEAVY";
const char str_fig_enemy_aux[16] = "FIG_ENEMY_AUX";
const char str_fig_enemy_miss[16] = "FIG_ENEMY_MISS";
const char str_fig_wallguy[12] = "FIG_WALLGUY";
const char str_fig_barbarian[16] = "FIG_BARBARIAN";
const char str_fig_patrician[16] = "FIG_PATRICIAN";
const char str_fig_buyer[12] = "FIG_BUYER";
const char str_fig_docker[12] = "FIG_DOCKER";
const char str_fig_flotsam[12] = "FIG_FLOTSAM";
const char str_fig_map_flag[16] = "FIG_MAP_FLAG";
const char str_fig_worker[12] = "FIG_WORKER";
const char str_fig_embalmer[16] = "FIG_EMBALMER";
const char str_fig_apothecary[16] = "FIG_APOTHECARY";
const char str_fig_physician[16] = "FIG_PHYSICIAN";
const char str_fig_dentist[12] = "FIG_DENTIST";
const char str_fig_librarian[16] = "FIG_LIBRARIAN";
const char str_fig_teacher[12] = "FIG_TEACHER";
const char str_fig_bubbles[12] = "FIG_BUBBLES";
const char str_fig_priest[12] = "FIG_PRIEST";
const char str_fig_market_girl[16] = "FIG_MARKET_GIRL";
const char str_fig_fishing_boat[20] = "FIG_FISHING_BOAT";
const char str_fig_tomb_robber[16] = "FIG_TOMB_ROBBER";
const char str_fig_thief[12] = "FIG_THIEF";
const char str_fig_simple_crime[20] = "FIG_SIMPLE_CRIME";
const char str_fig_van_follower[20] = "FIG_VAN_FOLLOWER";
const char str_fig_junk[12] = "FIG_JUNK";
const char str_fig_caravan[12] = "FIG_CARAVAN";
const char str_fig_bullfighter[16] = "FIG_BULLFIGHTER";
const char str_fig_dancer[12] = "FIG_DANCER";
const char str_fig_musician[16] = "FIG_MUSICIAN";
const char str_fig_juggler[12] = "FIG_JUGGLER";
const char str_fig_standard[16] = "FIG_STANDARD";
const char str_fig_leg_heavy[16] = "FIG_LEG_HEAVY";
const char str_fig_leg_aux[12] = "FIG_LEG_AUX";
const char str_fig_leg_miss[16] = "FIG_LEG_MISS";
const char str_fig_fireman[12] = "FIG_FIREMAN";
const char str_fig_warehouseman[20] = "FIG_WAREHOUSEMAN";
const char str_fig_engineer[16] = "FIG_ENGINEER";
const char str_fig_taxman[12] = "FIG_TAXMAN";
const char str_fig_explosion[16] = "FIG_EXPLOSION";
const char str_fig_ghost[12] = "FIG_GHOST";
const char str_fig_cart[12] = "FIG_CART";
const char str_fig_vagrant[12] = "FIG_VAGRANT";
const char str_fig_emmigrant[16] = "FIG_EMMIGRANT";
const char str_fig_immigrant[16] = "FIG_IMMIGRANT";
const char str_fig_null[12] = "FIG_NULL";

const char str_horn1_wav[16] = "wavs\\horn1.wav";

const char str_horn2_wav[16] = "wavs\\horn2.wav";

const char str_drums_wav[132] = "wavs\\drums.wav\0\0"
                          "Laborer at x,y: %d, %d\n\0"
                          "Frog %d reached house\n\0\0"
                          "Trapped frog %d\n\0\0\0\0"
                          "follow_figure_closely following off pyramid\r\n";

int32_t animal_threat_types[8] = { 73, 82, 83, 84, 102, 103, 104 };

int32_t animal_prey_types[7] = { 70, 68, 69, -228, -1, 228, 1 };

int32_t hippo_water_grid_offsets[9] = { -228, -227, 1, 229, 228, 227, -1, -229 };

animal_offset hippo_water_steps[8] = {
    { 0, -1 },
    { 1, -1 },
    { 1, 0 },
    { 1, 1 },
    { 0, 1 },
    { -1, 1 },
    { -1, 0 },
    { -1, -1 },
};

int32_t hippo_actions[4] = { 19, 18, 26, 25 };

int32_t crocodile_water_grid_offsets[8] = { -228, -227, 1, 229, 228, 227, -1, -229 };

animal_offset crocodile_water_steps[8] = {
    { 0, -1 },
    { 1, -1 },
    { 1, 0 },
    { 1, 1 },
    { 0, 1 },
    { -1, 1 },
    { -1, 0 },
    { -1, -1 },
};

int32_t crocodile_actions[2] = { 19, 18 };

uint8_t cheat_side_show_enabled;

static_assert(sizeof(figure_properties_table) == 6160, "figure_properties is the figure model file's table of 6160 bytes");
figure_properties_table figure_properties;

route_candidate_lists route_candidates;

figure_profile_clock figure_profile_frequency;

figure_profile_table figure_action_profile;

uint8_t formation_globals_block[16];

int32_t formation_move_result_y;

int32_t formation_move_result_x;

int32_t figure_roaming_enabled;

int32_t figure_search_best_distance;

int32_t figure_highest_id;

int32_t figure_unused_counter;

static_assert(sizeof(figure_table) == 1276000, "city_figures is the savegame's figure table of 1276000 bytes");
figure_table city_figures;

int16_t figure_route_figure_ids[1000];

static_assert(sizeof(enemy_army_totals_record) == 20, "enemy_army_totals is the savegame's enemy army totals of 20 bytes");
enemy_army_totals_record enemy_army_totals;

int32_t figure_count_processed;

int32_t figure_current_id;

int32_t figure_created_sequence;

uint8_t figure_profile_qpc_available;

static_assert(sizeof(random_data) == 36, "random_generator_state is the savegame and scenario random state of 36 bytes");
random_data random_generator_state;

int32_t map_result_y;

int32_t map_result_x;

int32_t distance_dy;

int32_t distance_dx;

int32_t figure_image_direction;

int32_t figure_phrase_id;

phrase_weight_record phrase_weight_block;
