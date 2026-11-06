// Built-in value bands of the overlay tooltips; config/tooltip_bands.json can override them.

#include "widget/tooltip_bands.h"

tooltip_band tooltip_bands[TOOLTIP_BANDS] = {
    [TOOLTIP_BAND_GODS] = { 6, { 1, 2, 3, 4, 5, 6 } },
    [TOOLTIP_BAND_FIRE] = { 5, { 1, 201, 401, 601, 801 } },
    [TOOLTIP_BAND_DAMAGE] = { 5, { 1, 201, 401, 601, 801 } },
    [TOOLTIP_BAND_CRIME] = { 5, { 1, 101, 201, 301, 500 } },
    [TOOLTIP_BAND_ENTERTAINMENT] = { 10, { 1, 10, 20, 30, 40, 50, 60, 70, 80, 90 } },
    [TOOLTIP_BAND_DESIRABILITY] = { 5, { -10, 0, 16, 41, 71 } },
    [TOOLTIP_BAND_LABOR] = { 4, { 21, 41, 61, 81 } },
    [TOOLTIP_BAND_FERTILITY] = { 4, { 1, 25, 50, 75 } },
};

int32_t tooltip_band_index(int32_t band_id, int32_t value)
{
    const tooltip_band *band = &tooltip_bands[band_id];
    for (int32_t i = 0; i < band->edge_count; i++) {
        if (value < band->edges[i]) {
            return i;
        }
    }
    return band->edge_count;
}
