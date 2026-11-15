// The built-in data of the figures: formation layouts, corpse and missile frames, the action table, the explosion
// cloud, the enemies' attack priorities and graphics names, and the debug names of the figure types.
// figure/figure_data.h declares them.

#include "figure/figure_data.h"
#include "figure/animals_internal.h"
#include "figure/carriers.h"
#include "figure/figure.h"
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

static const uint8_t figure_corpse_offsets[256] = { 0, 1, 2, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6,
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

static const int8_t figure_cart_offsets[16] = { 13, -7, 18, -1, 12, 7, 0, 11, -13, 6, -18, -1, -13, -7, 0, -12 };

uint8_t figure_ship_direction_table[64] = { 5, 3, 1, 2, 4, 2, 1, 3, 3, 5, 3, 1, 2, 4, 2, 1, 1, 3, 5, 3, 1, 2, 4, 2, 2, 1, 3, 5, 3, 1, 2, 4, 4, 2, 1, 3, 5, 3, 1,
    2, 2, 4, 2, 1, 3, 5, 3, 1, 1, 2, 4, 2, 1, 3, 4, 3, 3, 1, 2, 4, 3, 1, 3, 5 };

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

static int32_t figure_search_best_distance;

static int32_t figure_highest_id;

int32_t figure_unused_counter;

static_assert(sizeof(figure_table) == 1276000, "city_figures is the savegame's figure table of 1276000 bytes");
static figure_table city_figures;

figure_table *figure_table_get(void)
{
    return &city_figures;
}

figure *figure_get(int32_t id)
{
    return &city_figures.records[id];
}

uint8_t *figure_route_paths(void)
{
    return city_figures.route_paths;
}

int16_t figure_route_figure_ids[1000];

static_assert(sizeof(enemy_army_totals_record) == 20, "enemy_army_totals is the savegame's enemy army totals of 20 bytes");
enemy_army_totals_record enemy_army_totals;

int32_t figure_count_processed;

static int32_t figure_current_id;

static int32_t figure_created_sequence;

uint8_t figure_profile_qpc_available;

static_assert(sizeof(random_data) == 36, "random_generator_state is the savegame and scenario random state of 36 bytes");
random_data random_generator_state;

int32_t map_result_y;

int32_t map_result_x;

int32_t distance_dy;

int32_t distance_dx;

static int32_t figure_image_direction;

int32_t figure_phrase_id;

phrase_weight_record phrase_weight_block;

int32_t figure_current_id_get(void)
{
    return figure_current_id;
}

void figure_current_id_set(int32_t value)
{
    figure_current_id = value;
}

int32_t figure_highest_id_get(void)
{
    return figure_highest_id;
}

void figure_highest_id_set(int32_t value)
{
    figure_highest_id = value;
}

int32_t figure_image_direction_get(void)
{
    return figure_image_direction;
}

void figure_image_direction_set(int32_t value)
{
    figure_image_direction = value;
}

int32_t figure_created_sequence_get(void)
{
    return figure_created_sequence;
}

void figure_created_sequence_set(int32_t value)
{
    figure_created_sequence = value;
}

int32_t figure_search_best_distance_get(void)
{
    return figure_search_best_distance;
}

void figure_search_best_distance_set(int32_t value)
{
    figure_search_best_distance = value;
}

int32_t *figure_created_sequence_ptr(void)
{
    return &figure_created_sequence;
}

uint8_t figure_corpse_offset(int32_t index)
{
    return figure_corpse_offsets[index];
}

int8_t figure_cart_offset(int32_t index)
{
    return figure_cart_offsets[index];
}

uint8_t *figure_bytes_past_formations(int32_t offset)
{
    int32_t *const counters[] = { &figure_roaming_enabled, &figure_search_best_distance, &figure_highest_id, &figure_unused_counter };
    int32_t counter_bytes = (int32_t) sizeof(counters) / (int32_t) sizeof(counters[0]) * (int32_t) sizeof(int32_t);
    if (offset < counter_bytes) {
        return (uint8_t *) counters[offset / (int32_t) sizeof(int32_t)] + (offset % (int32_t) sizeof(int32_t));
    }
    offset -= counter_bytes;
    if (offset < (int32_t) sizeof(figure_table)) {
        return (uint8_t *) &city_figures + offset;
    }
    return 0;
}
