// The sidebar's buttons: their draw, the overlay name above the minimap, the top image, the build menu
// picture and the filler, the hovered minimap area, and the handlers of the city window's buttons (build menus,
// messages, undo, go to problem, collapse).

#include "widget/sidebar_buttons.h"
#include "building/building_data.h"
#include "building/construction_ui.h"
#include "city/city_data.h"
#include "game/undo_build.h"
#include "graphics/graphics_data.h"
#include "graphics/image_buttons.h"
#include "graphics/image_collection.h"
#include "graphics/image_draw.h"
#include "graphics/ui_layout.h"
#include "input/input_data.h"
#include "map/grid.h"
#include "platform/platform_time.h"
#include "sound/sound_play.h"
#include "text/lang.h"
#include "widget/city_view.h"
#include "widget/city_viewport.h"
#include "widget/city_with_overlay.h"
#include "widget/widget_data.h"
#include "widget/widget_image_group.h"
#include "window/building_menu.h"
#include "window/message_list.h"
#include "window/window_id.h"
#include "window/window_image_group.h"
#include <stdint.h>

enum sidebar_text_layout {
    OVERLAY_NAME_Y = 34,
    OVERLAY_NAME_WIDTH = 117,
    BUILD_IMAGE_Y = 211,
    TOP_MENU_TEXT_NO_OVERLAY = 4,
    TEXT_COLOR_WHITE = 0xFFFF,
};

enum sidebar_build_group {
    BUILD_GROUP_HOUSING = 1,
    BUILD_GROUP_SINGLE_LAST = 3,
    BUILD_GROUP_HOUSING_FROZEN = 13,
};

enum sidebar_timing {
    PROBLEM_CYCLE_RESTART_MS = 3000,
    SIDEBAR_SLIDE_SOUND = 6,
};

int32_t widget_sidebar_draw_buttons(void)
{
    buttons_expanded_build[SIDEBAR_BUTTON_UNDO].enabled = (undo_ready != 0 && undo_available != 0);
    if (g_sidebar_collapsed == 0) {
        image_buttons_draw(city_view_width_expanded, SIDEBAR_TOP, buttons_expanded_extra, SIDEBAR_EXPANDED_EXTRA_BUTTONS);
        image_buttons_draw(city_view_width_expanded, SIDEBAR_TOP, buttons_expanded_build, SIDEBAR_EXPANDED_BUILD_BUTTONS);
        return image_buttons_draw(city_view_width_expanded, SIDEBAR_TOP, buttons_expanded_top, SIDEBAR_EXPANDED_TOP_BUTTONS);
    }
    if (g_sidebar_collapsed == 1) {
        image_buttons_draw(city_view_width_collapsed, SIDEBAR_TOP, buttons_collapsed_expand.buttons, SIDEBAR_COLLAPSED_EXPAND_BUTTONS);
        return image_buttons_draw(city_view_width_collapsed, SIDEBAR_TOP, buttons_collapsed_build, SIDEBAR_COLLAPSED_BUILD_BUTTONS);
    }
    return g_sidebar_collapsed;
}

void widget_sidebar_draw_overlay_text(int32_t force, int32_t x)
{
    int32_t draw = 1;
    if (city_overlay == g_drawn_overlay) {
        draw = force;
    }
    if (window_ui_redraw != 0) {
        draw = 1;
    }
    if (g_sidebar_collapsed != 0 || draw == 0) {
        return;
    }
    g_drawn_overlay = city_overlay;
    if (city_overlay == OVERLAY_NONE) {
        lang_text_draw_centered_ui(
            LANG_GROUP_TOP_MENU, TOP_MENU_TEXT_NO_OVERLAY, x, OVERLAY_NAME_Y, OVERLAY_NAME_WIDTH, image_group(IMAGE_GROUP_FONT_BLACK), TEXT_COLOR_WHITE, ui_origin_x, ui_origin_y);
        return;
    }
    lang_text_draw_centered_ui(
        LANG_GROUP_OVERLAY_NAMES, city_overlay, x, OVERLAY_NAME_Y, OVERLAY_NAME_WIDTH, image_group(IMAGE_GROUP_FONT_BLACK), TEXT_COLOR_WHITE, ui_origin_x, ui_origin_y);
}

int32_t widget_sidebar_draw_build_image(int32_t force, int32_t x)
{
    if (force == 0 && g_sidebar_collapsed != 0) {
        return g_sidebar_collapsed;
    }
    int32_t offset = 0;
    if (build_menu_group >= 0 && build_menu_group < SIDEBAR_BUILD_GROUPS) {
        offset = sidebar_build_images[build_menu_group].image;
    }
    return image_draw(image_group(IMAGE_GROUP_SIDEBAR_BUILD_BUTTONS) + offset, x, BUILD_IMAGE_Y);
}

int32_t widget_sidebar_draw_filler(void)
{
    return image_draw(image_group(IMAGE_GROUP_PANEL) + SIDEBAR_PANEL_FILLER, SIDEBAR_RIGHT_EDGE_1024, 0);
}

void city_button_undo(void)
{
    game_undo_perform();
    window_redraw_request = WINDOW_REDRAW_ALL;
}

void city_button_go_to_problem(void)
{
    if (button_parameter == 0) {
        return;
    }
    int32_t now = platform_time_ms();
    if (now - city_message_problem_area_time > PROBLEM_CYCLE_RESTART_MS) {
        city_message_problem_area_b = 0;
    }
    city_message_problem_area_time = now;
    city_message_go_to_problem();
    window_redraw_request = WINDOW_REDRAW_ALL;
}

void sidebar_collapse(void)
{
    g_sidebar_slide_step = 0;
    window_current_id = WINDOW_SIDEBAR_SLIDE;
    city_view_set_viewport_without_sidebar();
    city_view_check_camera_boundaries();
    sound_effect_play(SIDEBAR_SLIDE_SOUND);
}

void sidebar_build_button_click(void)
{
    if (window_current_id == WINDOW_BUILD_MENU && editor_active == 1) {
        return;
    }
    window_current_id = WINDOW_BUILD_MENU;
    build_menu_group = button_parameter;
    if (button_parameter == BUILD_GROUP_HOUSING && houses_frozen == 1) {
        build_menu_group = BUILD_GROUP_HOUSING_FROZEN;
    }
    int32_t items = building_menu_count_items();
    build_menu_y_offset = build_menu_types[BUILD_MENU_SUB_POSITIONS_FIRST + items * 2];
    build_menu_item_count = items;
    if (build_menu_group >= BUILD_GROUP_HOUSING && build_menu_group <= BUILD_GROUP_SINGLE_LAST) {
        button_parameter = 1;
        build_menu_set_type();
    }
    window_redraw_request = WINDOW_REDRAW_ALL;
}

void build_sidebar_hover_update(void)
{
    minimap_hovered = 0;
    if (city_view.minimap_x0 <= mouse_x && mouse_x < city_view.minimap_x1 && city_view.minimap_y0 <= mouse_y && mouse_y < city_view.minimap_y1) {
        minimap_hovered = 1;
    }
}

int32_t sidebar_draw_top_image(void)
{
    return image_draw(image_group(IMAGE_GROUP_PANEL) + SIDEBAR_PANEL_TOP, SIDEBAR_RIGHT_EDGE_800, 0);
}
