// The scenario's resource lists, filled at a scenario's start: a count, then the resources in id order. city_foods
// holds the foods the scenario has, city_resources every good it has.

#ifndef CITY_RESOURCE_LIST_TYPES_H
#define CITY_RESOURCE_LIST_TYPES_H

#include "city/resource_type.h"

#include <stdint.h>

typedef struct {
    int32_t count;
    int32_t list[RESOURCE_MAX];
} city_resource_list;

#endif
