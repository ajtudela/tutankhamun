// The functions of figure/carriers.c that other files call.

#ifndef FIGURE_CARRIERS_H
#define FIGURE_CARRIERS_H

#include <stdbool.h>
#include <stdint.h>

enum carrier_limit {
    CARRIER_ANIMATION_FRAMES = 12,
    DELIVERY_BOY_DEATH_IMAGES = 96,
    MIGRANT_SWIM_FRAME_TICKS = 3,
    MIGRANT_DROWNING_TICKS = 33,
    MIGRANT_LAST_BUBBLE_FRAME = 22,
    GRANARY_CART_MAX_TAKE = 800,
    BAZAAR_NO_SLOT = 0xFF,
};

void cartpusher_delivered(int32_t amount, int32_t figure_id, int32_t road_network);
void warehouseman_delivered(int32_t amount, int32_t figure_id, int32_t road_network, bool done_dies);
void cart_image(int32_t figure_id);
void buyer_next_goods_storage(int32_t figure_id, int32_t home_id, int32_t storage_id);
void buyer_next_food_storage(int32_t figure_id, int32_t home_id, int32_t storage_id);
void buyer_return_or_die(int32_t figure_id);
void buyer_return_home(int32_t figure_id);
void buyer_at_storage(int32_t figure_id, int32_t home_id, int32_t storage_id);
void figure_count_by_destination(void);
int32_t delivery_boy_find_buyer(int32_t figure_id);
int32_t criminal_find_rich_building(int32_t figure_id);
int32_t cartpusher_determine_destination(int32_t figure_id, int32_t road_network);
int32_t cart_image_group(int32_t resource);
int32_t building_granary_take_wanted(int32_t building_id, int32_t src_building_id);
void figure_cartpusher_action(void);
void figure_warehouseman_action(void);
void figure_market_buyer_action(void);
void figure_delivery_boy_action(void);
void figure_immigrant_action(void);
void figure_homeless_action(void);
int32_t buyer_take_food(int32_t figure_id, int32_t home_id, int32_t storage_id, int32_t index);
int32_t buyer_take_goods(int32_t figure_id, int32_t home_id, int32_t storage_id, int32_t index);
int32_t buyer_go_to_storage(int32_t figure_id, int32_t storage_id);
int32_t buyer_best_slot(int32_t *wants);
bool buyer_has_bought(int32_t figure_id);
int32_t buyer_buy_food(int32_t figure_id, int32_t home_id, int32_t storage_id, int32_t slot, int32_t bought, int32_t index);
int32_t buyer_buy_goods(int32_t figure_id, int32_t home_id, int32_t storage_id, int32_t slot, int32_t bought, int32_t index);
int32_t buyer_food_loads_yard(int32_t figure_id, int32_t resource, int32_t home_id, int32_t storage_id, int32_t bought, int32_t slot);
int32_t buyer_food_loads_granary(int32_t figure_id, int32_t resource, int32_t home_id, int32_t storage_id, int32_t bought, int32_t slot);
int32_t buyer_goods_loads(int32_t resource, int32_t figure_id, int32_t home_id, int32_t storage_id, int32_t bought, int32_t slot);
int32_t buyer_check_no_leader(int32_t figure_id);
int32_t buyer_send_food_boys(int32_t resource, int32_t loads, int32_t figure_id, int32_t home_id, int32_t storage_id, int32_t leader_id);
int32_t buyer_send_goods_boys(int32_t resource, int32_t loads, int32_t figure_id, int32_t home_id, int32_t storage_id, int32_t leader_id);

#endif
