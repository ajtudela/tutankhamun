// The shows of the entertainment venues: a juggler, musician or dancer that reaches its venue starts the show of its
// kind there for VENUE_SHOW_DAYS days.

#include "building/entertainment.h"
#include "building/building.h"
#include "figure/figure.h"
#include "figure/figure_type.h"
#include <stdint.h>

void figure_entertainer_update_shows(int32_t figure_id)
{
    const figure *f = figure_get(figure_id);
    building_entertainment *venue = &building_get(f->destination_building_id)->data.entertainment;
    if (f->type == FIGURE_JUGGLER) {
        venue->show_days_left[SHOW_JUGGLERS] = VENUE_SHOW_DAYS;
    } else if (f->type == FIGURE_MUSICIAN) {
        venue->show_days_left[SHOW_MUSICIANS] = VENUE_SHOW_DAYS;
    } else if (f->type == FIGURE_DANCER) {
        venue->show_days_left[SHOW_DANCERS] = VENUE_SHOW_DAYS;
    }
}
