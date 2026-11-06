// Config table arrow_repeat_pattern.json: for the repeats 8..47 of a held repeating arrow button, whether the
// button fires again (1) or waits (0); from the 48th repeat on it fires every time.

#include "core/config.h"
#include "graphics/graphics_data.h"

static const config_field arrow_repeat_pattern_fields[] = {
    { "fires", (int32_t *) arrow_repeat_pattern, 0, CONFIG_COUNT(arrow_repeat_pattern), 0, 1, 0, 0, 1 },
};

static const config_storage arrow_repeat_pattern_storage[] = {
    { arrow_repeat_pattern, sizeof(arrow_repeat_pattern) },
};

const config_table arrow_repeat_pattern_table = {
    "arrow_repeat_pattern.json",
    "pattern",
    0,
    1,
    CONFIG_TABLE_COLUMNS(arrow_repeat_pattern_fields, arrow_repeat_pattern_storage),
};
