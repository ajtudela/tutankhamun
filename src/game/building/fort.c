// Forts and their legions: the soldiers a fort's legion brings when it is placed while a save loads (from the
// entry point, the exit point or the given tile, whichever can reach the legion), and the formations cleared at
// the start of a game.

#include "building/fort.h"
#include "building/building.h"
#include "city/city_player.h"
#include "core/calc.h"
#include "figure/figure.h"
#include "figure/figure_action.h"
#include "figure/formation.h"
#include "map/routing_land.h"
#include <stdint.h>
#include <string.h>

void formation_add_soldiers(int32_t count, int32_t formation_id, int32_t x, int32_t y)
{
    const formation *legion = formation_get(formation_id);
    const city_player *player = city_player_current();
    int32_t fort_player = building_get(legion->building_id)->player;
    for (int32_t i = count; i > 0; i--) {
        int32_t start_x = player->entry_point_x;
        int32_t start_y = player->entry_point_y;
        if (map_routing_citizen_can_travel_no_ferry(start_x, start_y, legion->x, legion->y) == 0) {
            start_x = player->exit_point_x;
            start_y = player->exit_point_y;
            if (map_routing_citizen_can_travel_no_ferry(start_x, start_y, legion->x, legion->y) == 0) {
                start_x = x;
                start_y = y;
            }
        }
        figure *soldier = figure_get(figure_create(fort_player, legion->figure_type, start_x, start_y, 0));
        soldier->formation_id = (int16_t) formation_id;
        soldier->formation_at_rest = 1;
        soldier->action_state = FIGURE_ACTION_SOLDIER_RETURNING_FROM_REQUEST;
    }
    formation_calculate_figures();
}

void formation_clear_all(void)
{
    for (int32_t id = 0; id < MAX_FORMATIONS; id++) {
        memset(formation_get(id), 0, sizeof(formation));
    }
    formation_id_last_in_use = 0;
    formation_id_last_legion = 0;
    formation_num_legions = 0;
}
