// The build menu window: its background, the click on an entry, the next shown entry of a group and the tests of an
// entry's flags. An entry is shown when its enable flag is set; the monuments, temple complexes, guilds and temple
// complex upgrades also need a monument or an upgrade the city still lacks.
//
// The religion menu's temple complexes test reads the group open on screen, not the group asked for.

#include "window/build_menu.h"
#include "building/building_figure_helpers.h"
#include "building/building_type.h"
#include "building/construction_ui.h"
#include "building/monument.h"
#include "building/monument_city.h"
#include "game/game_internal.h"
#include "graphics/button.h"
#include "graphics/generic_buttons.h"
#include "graphics/label_animation_slots.h"
#include "graphics/ui_layout.h"
#include "input/input_data.h"
#include "widget/city_draw_view.h"
#include "widget/sidebar.h"
#include "widget/widget_data.h"
#include "window/building_menu.h"
#include "window/overlay_menu.h"
#include <stdbool.h>
#include <stdint.h>

enum build_menu_layout {
    BUILD_MENU_FROM_VIEW_RIGHT = 258,
    BUILD_MENU_ITEMS_Y = 110,
    BUILD_MENU_ITEMS_X = 161,
};

enum build_flags {
    BUILD_FLAG_BIT1 = 0x2,
    BUILD_FLAGS_KIND_MASK = 0x20300002,
    BUILD_FLAGS_KIND_A = 0x100002,
    BUILD_FLAGS_KIND_B = 0x2,
};

static int32_t temple_complex_lacks_upgrade(const city_player *player, int32_t upgrade)
{
    return building_temple_complex_has_upgrade(player->temple_complex_building_id, upgrade) == 0;
}

static int32_t religion_item_shown(const city_player *player, int32_t index, int32_t item, int32_t type)
{
    if (type == BUILDING_MENU_MONUMENTS) {
        return building_menu_tutorial_ok() != 0 && city_count_monuments() < scenario_count_monuments();
    }
    int32_t shown_index = (build_menu_group * BUILD_MENU_ITEMS) + item;
    if (build_menu_types[BUILD_MENU_TYPE_FIRST + shown_index] != BUILDING_MENU_TEMPLE_COMPLEXES) {
        return menu_enabled[index] != 0;
    }
    if (player->has_temple_complex == 0) {
        return menu_enabled[shown_index] != 0;
    }
    return temple_complex_lacks_upgrade(player, 1) || temple_complex_lacks_upgrade(player, 2);
}

static int32_t build_menu_item_shown(int32_t group, int32_t item, int32_t type)
{
    const city_player *player = city_player_current();
    int32_t index = (group * BUILD_MENU_ITEMS) + item;
    switch (group) {
        case BUILD_MENU_RELIGION:
            return religion_item_shown(player, index, item, type);
        case BUILD_MENU_INDUSTRY:
            if (type != BUILDING_MENU_CONSTRUCTION_GUILDS) {
                return menu_enabled[index] != 0;
            }
            return building_menu_tutorial_ok() != 0 &&
                (guild_has_monument_demand(BUILDING_STONEMASONS_GUILD) != 0 || guild_has_monument_demand(BUILDING_BRICKLAYERS_GUILD) != 0 ||
                    guild_has_monument_demand(BUILDING_CARPENTERS_GUILD) != 0);
        case BUILD_MENU_GUILDS:
            return building_menu_tutorial_ok() != 0 && guild_has_monument_demand(type) != 0;
        case BUILD_MENU_MONUMENTS:
            return building_menu_tutorial_ok() != 0 && player->monument_def_id[item] == 0;
        case BUILD_MENU_TEMPLE_COMPLEX_UPGRADES:
            return temple_complex_lacks_upgrade(player, item + 1);
        default:
            return menu_enabled[index] != 0;
    }
}

int32_t build_menu_next_item(int32_t group, int32_t prev)
{
    for (int32_t item = prev + 1; item < BUILD_MENU_ITEMS; item++) {
        int32_t type = build_menu_types[BUILD_MENU_TYPE_FIRST + (group * BUILD_MENU_ITEMS) + item];
        if (type <= 0) {
            return 0;
        }
        if (build_menu_item_shown(group, item, type)) {
            return item;
        }
    }
    return 0;
}

int32_t build_menu_handle_input(void)
{
    int32_t x = city_view_width_expanded - BUILD_MENU_FROM_VIEW_RIGHT;
    if (g_sidebar_collapsed == 1) {
        x = city_view_width_collapsed - BUILD_MENU_FROM_VIEW_RIGHT;
    }
    if (editor_active == 1) {
        x = city_view_width_expanded - BUILD_MENU_FROM_VIEW_RIGHT;
    }
    int32_t y = build_menu_y_offset + BUILD_MENU_ITEMS_Y;
    x += BUILD_MENU_ITEMS_X - ui_layout_label_width_get();
    const generic_button *buttons = build_menu_buttons.build_menu;
    focus_button_id = generic_buttons_get_focus(x, y, buttons, build_menu_item_count);
    int32_t item = -1;
    for (int32_t row = 0; row < build_menu_item_count; row++) {
        item = build_menu_next_item(build_menu_group, item);
        const generic_button *button = &buttons[row];
        int32_t inside = (button->x + x <= mouse_x) && (mouse_x < button->x_end + x) && (button->y + y <= mouse_y) && (mouse_y < button->y_end + y);
        if (inside && mouse_left_went_down != 0) {
            button_parameter = item + 1;
            build_menu_set_type();
            return item + 1;
        }
    }
    return 0;
}

void window_build_menu_draw_background(void)
{
    widget_sidebar_draw_background();
    widget_city_draw_current_view();
    if (window_redraw_request >= 2) {
        label_animation[LABEL_SLOT_MENU_DRAW] = 0;
        window_build_menu_draw_foreground();
    }
}

bool build_flags_is_kind_a(int32_t flags)
{
    return ((uint32_t) flags & BUILD_FLAGS_KIND_MASK) == BUILD_FLAGS_KIND_A;
}

bool build_flags_is_kind_b(int32_t flags)
{
    return ((uint32_t) flags & BUILD_FLAGS_KIND_MASK) == BUILD_FLAGS_KIND_B;
}

bool build_flags_has_bit1(int32_t flags)
{
    return (flags & BUILD_FLAG_BIT1) == BUILD_FLAG_BIT1;
}
