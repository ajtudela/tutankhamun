// Config table ui_layout_columns.json: per layout column the lang group whose widest string gives the
// column's width.

#include "core/config.h"
#include "graphics/ui_layout.h"

enum ui_layout_config_limit {
    UI_LAYOUT_LAST_LANG_GROUP = 998,
};

static const config_field ui_layout_column_fields[] = {
    { "lang_group", ui_layout_column_groups, 1, 1, 0, UI_LAYOUT_LAST_LANG_GROUP, 0 },
};

static const config_storage ui_layout_column_storage[] = {
    { ui_layout_column_groups, sizeof(ui_layout_column_groups) },
};

const config_table ui_layout_column_table = {
    "ui_layout_columns.json",
    "column",
    0,
    UI_LAYOUT_COLUMNS,
    CONFIG_TABLE_COLUMNS(ui_layout_column_fields, ui_layout_column_storage),
};
