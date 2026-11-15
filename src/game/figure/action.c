// The per-tick figure dispatcher: resets the city's counts of enemies, rioters, natives, animals and soldiers, runs
// the action of every figure in use from the figure action table and deletes the figures that died, keeping a timing
// profile per figure type. Also the empty action.

#include "figure/action.h"
#include "city/city_player.h"
#include "editor/editor_new_map_tiles.h"
#include "figure/animals.h"
#include "figure/animals_internal.h"
#include "figure/animals_wild.h"
#include "figure/boats.h"
#include "figure/carriers.h"
#include "figure/construction_worker.h"
#include "figure/enemy.h"
#include "figure/enemy_ships.h"
#include "figure/figure.h"
#include "figure/figure_core.h"
#include "figure/figure_data.h"
#include "figure/figure_type.h"
#include "figure/laborer.h"
#include "figure/military.h"
#include "figure/missiles.h"
#include "figure/natives.h"
#include "figure/plagues.h"
#include "figure/protestor.h"
#include "figure/shipwreck.h"
#include "figure/trade.h"
#include "figure/walkers.h"
#include "game/game_internal.h"
#include "platform/platform_time.h"

static global_function figure_action_table[110] = {
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

int32_t figure_profile_current_type;

int32_t figure_profile_start_ticks;

int32_t figure_profile_end_ticks;

figure_profile_clock figure_profile_start_counter;

figure_profile_end_clock figure_profile_end_counter;

static void read_counter(figure_profile_clock *clock)
{
    uint64_t count;
    if (platform_time_counter(&count)) {
        clock->low_part = (uint32_t) count;
        clock->high_part = (int32_t) (count >> 32);
    }
}

void figure_action_profile_begin(int32_t type)
{
    figure_profile_current_type = type;
    figure_profile_start_ticks = (int32_t) platform_tick_count();
    if (figure_profile_qpc_available) {
        read_counter(&figure_profile_start_counter);
    }
}

void figure_action_profile_end(void)
{
    if (figure_profile_qpc_available) {
        read_counter(&figure_profile_end_counter.clock);
    }
    int32_t now = (int32_t) platform_tick_count();
    figure_profile_end_ticks = now;
    int32_t elapsed = now - figure_profile_start_ticks;
    figure_profile_entry *p = &figure_action_profile[figure_profile_current_type];
    p->calls++;
    p->ticks_total += elapsed;
    if (elapsed > p->ticks_max) {
        p->ticks_max = elapsed;
    }
    if (figure_profile_qpc_available) {
        int32_t counts = (int32_t) (figure_profile_end_counter.clock.low_part - figure_profile_start_counter.low_part);
        double seconds = (double) counts / (double) figure_profile_frequency.low_part;
        p->seconds_total = seconds + p->seconds_total;
        if (seconds > p->seconds_max) {
            p->seconds_max = seconds;
        }
    }
}

void figure_action_handle(void)
{
    city_player *player = city_player_current();
    player->enemies = 0;
    player->rioters = 0;
    player->attacking_natives = 0;
    player->animals = 0;
    player->kingdom_soldiers = 0;
    player->soldiers = 0;
    player->senet_house_has_plays = 0;
    figure_count_processed = 0;
    if (player->security_breach_duration > 0) {
        player->security_breach_duration--;
    }
    for (figure_current_id_set(1); figure_current_id_get() < MAX_FIGURES; figure_current_id_set(figure_current_id_get() + 1)) {
        int32_t id = figure_current_id_get();
        figure *f = figure_get(id);
        if (f->state == FIGURE_STATE_NONE) {
            continue;
        }
        int16_t target_id = f->targeted_by_figure_id;
        if (target_id) {
            figure *target = figure_get(target_id);
            if (target->state != FIGURE_STATE_ALIVE) {
                f->targeted_by_figure_id = 0;
            }
            if (target->target_figure_id != id) {
                f->targeted_by_figure_id = 0;
            }
        }
        figure_count_processed++;
        figure_action_profile_begin(f->type);
        figure_action_table[f->type]();
        figure_action_profile_end();
        if (figure_get(figure_current_id_get())->state == FIGURE_STATE_DEAD) {
            figure_delete(figure_current_id_get());
        }
    }
}

void figure_action_none(void)
{
}
