// The troop requests the military, political and fleet advisors show: the waiting requests for troops on a valid
// empire route (0..49), counted apart by sea routes and by the other routes with the first request id of each, and
// whether there are requests of each kind.

#include "window/advisor/troop_requests.h"
#include "city/resource_type.h"
#include "empire/empire_data.h"
#include "empire/empire_route.h"
#include "empire/empire_types.h"
#include "scenario/event_helpers.h"
#include "scenario/event_manager.h"
#include "window/window_data.h"
#include <stdbool.h>

static void count_request(const scenario_event *e, int32_t *first, int32_t *count)
{
    if (*first < 0) {
        *first = e->id;
    }
    (*count)++;
}

void advisor_count_troop_requests(int32_t *first_by_land, int32_t *first_by_sea, int32_t *count_by_land, int32_t *count_by_sea)
{
    *first_by_land = -1;
    *first_by_sea = -1;
    *count_by_land = 0;
    *count_by_sea = 0;
    for (scenario_event *e = scenario_request_next_active(&scenario_events, 0); e; e = scenario_request_next_active(&scenario_events, e)) {
        if (!scenario_request_is_waiting(e) || e->item.value != RESOURCE_TROOPS) {
            continue;
        }
        int32_t route = e->route.value;
        if (route < 0 || route >= EMPIRE_ROUTES) {
            continue;
        }
        if (empire_map_routes.routes[route].kind == EMPIRE_ROUTE_KIND_SEA) {
            count_request(e, first_by_sea, count_by_sea);
        } else {
            count_request(e, first_by_land, count_by_land);
        }
    }
}

void advisor_has_troop_requests(bool *has_by_land, bool *has_by_sea)
{
    int32_t first_by_land;
    int32_t first_by_sea;
    int32_t count_by_land;
    int32_t count_by_sea;
    advisor_count_troop_requests(&first_by_land, &first_by_sea, &count_by_land, &count_by_sea);
    *has_by_land = count_by_land > 0;
    *has_by_sea = count_by_sea > 0;
}
