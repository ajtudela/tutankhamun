// What is left of a sunk ship: the shipwreck, which settles on open water nearby, and the bubbles of a sinking ship,
// gone after 22 ticks.

#include "figure/shipwreck.h"
#include "figure/figure.h"
#include "figure/figure_data.h"
#include "figure/image_groups_boats.h"
#include "graphics/image_collection.h"
#include "map/grid.h"
#include "map/water.h"
#include <stdint.h>

void figure_bubbles_action(void)
{
    figure *f = figure_get(figure_current_id);
    int16_t ticks = f->wait_ticks++;
    if (ticks > BUBBLES_LAST_TICK) {
        f->state = FIGURE_STATE_DEAD;
    }
    f->image_id = f->wait_ticks + image_group(IMAGE_GROUP_FIGURE_BUBBLES) + 1;
}

void figure_shipwreck_action(void)
{
    int32_t figure_id = figure_current_id;
    figure *f = figure_get(figure_id);
    f->is_ghost = 0;
    f->height_adjusted_ticks = 0;
    f->is_boat = FIGURE_BOAT_SHIP;
    if (++f->animation_frame >= SHIPWRECK_FRAMES) {
        f->state = FIGURE_STATE_DEAD;
    }
    if (f->wait_ticks < SHIPWRECK_WAIT_SETTLED) {
        map_figure_delete(figure_id);
        if (map_water_find_shipwreck_tile(figure_id)) {
            f->x = (int16_t) map_result_x;
            f->y = (int16_t) map_result_y;
            f->grid_offset = map_grid_offset(f->x, f->y);
            f->cross_country.x = (int16_t) ((f->x * 15) + 7);
            f->cross_country.y = (int16_t) ((f->y * 15) + 7);
        }
        map_figure_add(figure_id);
        f->wait_ticks = SHIPWRECK_WAIT_SETTLED;
    }
    f->image_id = f->animation_frame + image_group(IMAGE_GROUP_FIGURE_SHIPWRECK);
}
