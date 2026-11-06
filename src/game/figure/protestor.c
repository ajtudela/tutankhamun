// The protestor: it stands where it was created, animates and is gone after 200 ticks; while dying it shows its corpse
// images.

#include "figure/protestor.h"
#include "figure/figure.h"
#include "figure/figure_action.h"
#include "figure/figure_data.h"
#include "figure/image_groups_walkers.h"
#include "figure/military.h"
#include "graphics/image_collection.h"
#include "map/grid.h"
#include <stdint.h>

void figure_protestor_action(void)
{
    figure *f = figure_get(figure_current_id);
    f->terrain_usage = TERRAIN_USAGE_ROADS;
    if (++f->animation_frame >= PROTESTOR_ANIMATION_FRAMES) {
        f->animation_frame = 0;
    }
    f->cart_image_id = 0;
    if (f->action_state == FIGURE_ACTION_CORPSE) {
        figure_combat_handle_corpse(figure_current_id);
    }
    if (++f->wait_ticks > PROTESTOR_LIFETIME_TICKS) {
        f->state = FIGURE_STATE_DEAD;
        f->animation_frame = 0;
    }
    if (f->action_state == FIGURE_ACTION_CORPSE) {
        f->image_id = figure_corpse_offsets[f->wait_ticks / 2] + image_group(IMAGE_GROUP_FIGURE_PROTESTOR_DEATH);
        return;
    }
    f->image_id = image_group(IMAGE_GROUP_FIGURE_PROTESTOR) + (f->animation_frame * DIRECTION_COUNT);
}
