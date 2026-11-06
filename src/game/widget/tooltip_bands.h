// The value bands of the overlay tooltips: per band the edges that split a value (gods, risks,
// entertainment, desirability, labor coverage, fertility) into the tooltip's texts.

#ifndef WIDGET_TOOLTIP_BANDS_H
#define WIDGET_TOOLTIP_BANDS_H

#include <stdint.h>

enum tooltip_band_id {
    TOOLTIP_BAND_GODS = 0,
    TOOLTIP_BAND_FIRE = 1,
    TOOLTIP_BAND_DAMAGE = 2,
    TOOLTIP_BAND_CRIME = 3,
    TOOLTIP_BAND_ENTERTAINMENT = 4,
    TOOLTIP_BAND_DESIRABILITY = 5,
    TOOLTIP_BAND_LABOR = 6,
    TOOLTIP_BAND_FERTILITY = 7,
    TOOLTIP_BANDS = 8,
};

enum { TOOLTIP_BAND_EDGES_MAX = 10 };

typedef struct tooltip_band {
    int32_t edge_count;
    int32_t edges[TOOLTIP_BAND_EDGES_MAX];
} tooltip_band;

extern tooltip_band tooltip_bands[TOOLTIP_BANDS];

int32_t tooltip_band_index(int32_t band_id, int32_t value);

#endif
