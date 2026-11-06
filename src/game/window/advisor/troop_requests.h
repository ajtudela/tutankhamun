// The troop requests of troop_requests.c: the waiting requests for troops by land and by sea (the first request id,
// -1 for none, and the count of each), and whether there are any of each.

#ifndef WINDOW_ADVISOR_TROOP_REQUESTS_H
#define WINDOW_ADVISOR_TROOP_REQUESTS_H

#include <stdbool.h>
#include <stdint.h>

void advisor_count_troop_requests(int32_t *first_by_land, int32_t *first_by_sea, int32_t *count_by_land, int32_t *count_by_sea);
void advisor_has_troop_requests(bool *has_by_land, bool *has_by_sea);

#endif
