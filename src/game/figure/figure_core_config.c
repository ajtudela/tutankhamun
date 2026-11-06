// The gameplay config tables of the figures' names (core/config.h): figure_name_groups.json and
// enemy_name_groups.json, per figure type and per enemy type the text group of its names and their number.

#include "core/config.h"
#include "figure/enemy.h"
#include "figure/figure_core.h"
#include "figure/figure_type.h"

static const config_field figure_name_group_fields[] = {
    { "group", &figure_name_groups[0].group, 2, 1, 0, FIGURE_NAME_GROUP_ENEMY_EXTRA_LAST, 0 },
    { "count", &figure_name_groups[0].count, 2, 1, 0, 10000, 0 },
};

static const config_storage figure_name_group_storage[] = {
    { figure_name_groups, sizeof(figure_name_group) * FIGURE_TYPE_COUNT },
};

const config_table figure_name_group_table = {
    "figure_name_groups.json",
    "type",
    config_figure_type_names,
    FIGURE_TYPE_COUNT,
    CONFIG_TABLE_COLUMNS(figure_name_group_fields, figure_name_group_storage),
};

static const config_name enemy_name_group_names[] = {
    { "hyksos", ENEMY_HYKSOS },
    { "sea_people", ENEMY_SEA_PEOPLE },
    { "hittite", ENEMY_HITTITE },
    { "mitani", ENEMY_MITANI },
    { "kushite", ENEMY_KUSHITE },
    { "libyan", ENEMY_LIBYAN },
    { "nubian", ENEMY_NUBIAN },
    { "canaanite", ENEMY_CANAANITE },
    { "assyrian", ENEMY_ASSYRIAN },
    { "roman", ENEMY_ROMAN },
    { "phoenician", ENEMY_PHOENICIAN },
    { "persian", ENEMY_PERSIAN },
    { 0, 0 },
};

static const config_field enemy_name_group_fields[] = {
    { "group", &enemy_name_groups[0].group, 2, 1, 0, FIGURE_NAME_GROUP_ENEMY_EXTRA_LAST, 0 },
    { "count", &enemy_name_groups[0].count, 2, 1, 0, 10000, 0 },
};

static const config_storage enemy_name_group_storage[] = {
    { enemy_name_groups, sizeof(figure_name_group) * ENEMY_EGYPTIAN },
};

const config_table enemy_name_group_table = {
    "enemy_name_groups.json",
    "enemy",
    enemy_name_group_names,
    ENEMY_EGYPTIAN,
    CONFIG_TABLE_COLUMNS(enemy_name_group_fields, enemy_name_group_storage),
};
