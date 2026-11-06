// The progress reports of the scripted monuments, the library, Abu Simbel and the mausoleum: what their
// *_get_progress fill in for the building info window.

#ifndef BUILDING_MONUMENT_PROGRESS_TYPES_H
#define BUILDING_MONUMENT_PROGRESS_TYPES_H

#include <stdint.h>

typedef struct {
    int32_t stage;
    int32_t stonemasons;
    int32_t stonemasons_waiting;
    int32_t laborers;
    int32_t carpenters;
    int32_t material;
    int32_t percent;
    int32_t material_total;
} scripted_monument_info_progress;

typedef struct {
    int32_t stage;
    int32_t spare_51[6];
    int32_t stonemasons;
    int32_t stonemasons_waiting;
    int32_t laborers;
    int32_t carpenters;
} library_info_progress;

typedef struct {
    int32_t material;
    int32_t percent;
    int32_t material_total;
} library_info_stock;

typedef struct {
    library_info_progress progress;
    library_info_stock stock;
} library_info_report;

typedef struct {
    int32_t stage;
    int32_t spare_51[2];
    int32_t percent;
} abu_simbel_info_progress;

typedef struct {
    int32_t status;
    int32_t parts_at_stage_0;
    int32_t missing_subtiles;
    int32_t parts_below_stage_2;
    int32_t stonemasons;
    int32_t stonemasons_waiting;
    int32_t laborers;
    int32_t material;
    int32_t percent;
    int32_t material_total;
    int32_t carpenters;
    int32_t blocks_left;
} mausoleum_info_progress;

#endif
