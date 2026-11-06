// Config tables menu_bar_city.json and menu_bar_editor.json: per menu bar entry its text group and the range
// of its items in the menu item table; a file whose items lie outside the table is rejected.

#include "core/config.h"
#include "graphics/menu.h"
#include "window/window_data.h"
#include <stdio.h>

enum menu_bar_config_limit {
    MENU_BAR_LAST_LANG_GROUP = 998,
};

static int32_t menu_bar_items_inside(const menu_bar_entry *entries, char *problem, size_t problem_size)
{
    for (int32_t entry = 0; entry < MENU_BAR_ENTRIES; entry++) {
        if (entries[entry].first_item + entries[entry].item_count > MENU_ITEMS) {
            snprintf(problem, problem_size, "entry %d: first_item %d + item_count %d passes the %d menu items", entry, entries[entry].first_item,
                entries[entry].item_count, MENU_ITEMS);
            return 0;
        }
    }
    return 1;
}

static int32_t menu_bar_city_valid(char *problem, size_t problem_size)
{
    return menu_bar_items_inside(menu_bar_city, problem, problem_size);
}

static int32_t menu_bar_editor_valid(char *problem, size_t problem_size)
{
    return menu_bar_items_inside(menu_bar_editor, problem, problem_size);
}

static const config_field menu_bar_city_fields[] = {
    { "text_group", &menu_bar_city[0].text_group, (int32_t) (sizeof(menu_bar_entry) / sizeof(int32_t)), 1, 0, MENU_BAR_LAST_LANG_GROUP, 0 },
    { "first_item", &menu_bar_city[0].first_item, (int32_t) (sizeof(menu_bar_entry) / sizeof(int32_t)), 1, 0, MENU_ITEMS - 1, 0 },
    { "item_count", &menu_bar_city[0].item_count, (int32_t) (sizeof(menu_bar_entry) / sizeof(int32_t)), 1, 0, MENU_ITEMS, 0 },
};

static const config_storage menu_bar_city_storage[] = {
    { menu_bar_city, sizeof(menu_bar_city) },
};

const config_table menu_bar_city_table = {
    "menu_bar_city.json",
    "entry",
    0,
    MENU_BAR_ENTRIES,
    CONFIG_TABLE_COLUMNS(menu_bar_city_fields, menu_bar_city_storage),
    menu_bar_city_valid,
};

static const config_field menu_bar_editor_fields[] = {
    { "text_group", &menu_bar_editor[0].text_group, (int32_t) (sizeof(menu_bar_entry) / sizeof(int32_t)), 1, 0, MENU_BAR_LAST_LANG_GROUP, 0 },
    { "first_item", &menu_bar_editor[0].first_item, (int32_t) (sizeof(menu_bar_entry) / sizeof(int32_t)), 1, 0, MENU_ITEMS - 1, 0 },
    { "item_count", &menu_bar_editor[0].item_count, (int32_t) (sizeof(menu_bar_entry) / sizeof(int32_t)), 1, 0, MENU_ITEMS, 0 },
};

static const config_storage menu_bar_editor_storage[] = {
    { menu_bar_editor, sizeof(menu_bar_editor) },
};

const config_table menu_bar_editor_table = {
    "menu_bar_editor.json",
    "entry",
    0,
    MENU_BAR_ENTRIES,
    CONFIG_TABLE_COLUMNS(menu_bar_editor_fields, menu_bar_editor_storage),
    menu_bar_editor_valid,
};
