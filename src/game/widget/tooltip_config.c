// Config table tooltip_bands.json: per band the number of edges and the ascending edges that split its value
// into tooltip texts.

#include "core/config.h"
#include "widget/tooltip_bands.h"

enum { TOOLTIP_BAND_STRIDE = sizeof(tooltip_band) / sizeof(int32_t), TOOLTIP_BAND_VALUE_LIMIT = 100000 };

static const config_name tooltip_band_names[] = {
    { "gods", TOOLTIP_BAND_GODS },
    { "fire", TOOLTIP_BAND_FIRE },
    { "damage", TOOLTIP_BAND_DAMAGE },
    { "crime", TOOLTIP_BAND_CRIME },
    { "entertainment", TOOLTIP_BAND_ENTERTAINMENT },
    { "desirability", TOOLTIP_BAND_DESIRABILITY },
    { "labor", TOOLTIP_BAND_LABOR },
    { "fertility", TOOLTIP_BAND_FERTILITY },
    { 0, 0 },
};

static const config_field tooltip_band_fields[] = {
    { "edge_count", &tooltip_bands[0].edge_count, TOOLTIP_BAND_STRIDE, 1, 0, TOOLTIP_BAND_EDGES_MAX, 0 },
    { "edges", &tooltip_bands[0].edges[0], TOOLTIP_BAND_STRIDE, TOOLTIP_BAND_EDGES_MAX, -TOOLTIP_BAND_VALUE_LIMIT, TOOLTIP_BAND_VALUE_LIMIT, 0 },
};

static const config_storage tooltip_band_storage[] = {
    { tooltip_bands, sizeof(tooltip_bands) },
};

const config_table tooltip_band_table = {
    "tooltip_bands.json",
    "band",
    tooltip_band_names,
    TOOLTIP_BANDS,
    CONFIG_TABLE_COLUMNS(tooltip_band_fields, tooltip_band_storage),
};
