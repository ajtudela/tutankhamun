// The checks of house evolution (building/house_evolve_checks.c): whether a house has the goods and services of its
// level, and the text of what it needs to evolve.

#ifndef BUILDING_HOUSE_EVOLVE_CHECKS_H
#define BUILDING_HOUSE_EVOLVE_CHECKS_H

#include <stdint.h>

int32_t house_has_required_goods_and_services(int32_t building_id, int32_t for_upgrade);
void house_update_evolve_text(int32_t building_id);

#endif
