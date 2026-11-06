// Legions and warships sent to the kingdom's requests for troops (city/legion_requests.c).

#ifndef CITY_LEGION_REQUESTS_H
#define CITY_LEGION_REQUESTS_H

#include <stdint.h>

enum request_strength {
    LEGION_STRENGTH_BASE = 100,
    CHARIOTEER_STRENGTH_BASE = 200,
    WARSHIP_STRENGTH_DIVISOR = 10,
};

enum request_losses {
    REQUEST_LOSSES_ALL = 100,
};

int32_t figure_warship_strength(int32_t figure_id);
int32_t formation_request_strength(int32_t request_id, int32_t *warships, int32_t *legions);
int32_t figure_find_legion_transport_ship(int32_t formation_id);
void formation_legions_dispatch_to_request(int32_t event_id);
void formation_legions_return_from_request(int32_t event_id);
void formation_legions_request_losses(int32_t percentage, int32_t event_id);

#endif
