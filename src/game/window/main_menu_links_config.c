// The config table of the main menu's link buttons: main_menu_links.json (per link its web address and the icon's
// distance from the screen's right and bottom edges), described over its C table.

#include "core/config.h"
#include "window/main_menu_links.h"

#define MAIN_MENU_LINK_STRIDE ((int32_t) (sizeof(main_menu_link_place) / sizeof(int32_t)))

static const config_name main_menu_link_names[] = {
    { "discord", MAIN_MENU_LINK_DISCORD },
    { "github", MAIN_MENU_LINK_GITHUB },
    { "patreon", MAIN_MENU_LINK_PATREON },
    { "wiki", MAIN_MENU_LINK_WIKI },
    { 0, 0 },
};

static const config_field main_menu_link_fields[] = {
    { "url", (int32_t *) main_menu_links[0].url, MAIN_MENU_LINK_STRIDE, MAIN_MENU_LINK_URL_SIZE, 0, 0, 0, 0, CONFIG_WIDTH_TEXT },
    { "right", &main_menu_links[0].right, MAIN_MENU_LINK_STRIDE, 1, MAIN_MENU_LINK_ICON_SIZE, MAIN_MENU_LINK_MAX_OFFSET, 0 },
    { "bottom", &main_menu_links[0].bottom, MAIN_MENU_LINK_STRIDE, 1, MAIN_MENU_LINK_ICON_SIZE, MAIN_MENU_LINK_MAX_OFFSET, 0 },
};

static const config_storage main_menu_link_storage[] = {
    { main_menu_links, sizeof(main_menu_links) },
};

const config_table main_menu_link_table = {
    "main_menu_links.json",
    "link",
    main_menu_link_names,
    MAIN_MENU_LINKS,
    CONFIG_TABLE_COLUMNS(main_menu_link_fields, main_menu_link_storage),
};
