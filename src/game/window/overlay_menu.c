// The overlay menu (window 8): opening it from the sidebar, its draws, the hovered entry opening its submenu (closed
// after 500 ms with nothing hovered), the choice of an overlay; and the build menu's foreground (the editor's brushes
// and animals, monuments, temple complex upgrades, the buildings with their cost).

#include "window/overlay_menu.h"
#include "building/building.h"
#include "building/building_type.h"
#include "building/construction_place.h"
#include "building/royal_tomb.h"
#include "city/city_player.h"
#include "figure/figure_type.h"
#include "graphics/color.h"
#include "graphics/image_collection.h"
#include "graphics/image_draw.h"
#include "graphics/label_animation_slots.h"
#include "graphics/text_draw.h"
#include "graphics/ui_layout.h"
#include "scenario/scenario_climate.h"
#include "text/lang.h"
#include "widget/city_draw_view.h"
#include "widget/city_with_overlay.h"
#include "widget/sidebar.h"
#include "widget/widget_data.h"
#include "window/build_menu.h"
#include "window/building_menu.h"
#include "window/window_id.h"
#include "window/window_image_group.h"
#include <stdint.h>

enum overlay_menu_layout {
    OVERLAY_MENU_ENTRIES = 10,
    OVERLAY_SUBMENU_MAX_ENTRIES = 30,
    OVERLAY_MENU_ROW_HEIGHT = 24,
    OVERLAY_MENU_LABEL_BLOCKS = 10,
    OVERLAY_MENU_TEXT_WIDTH = 160,
    OVERLAY_MENU_FIRST_ROW_Y = 40,
    OVERLAY_MENU_ITEM_COUNT = 20,
    OVERLAY_SUBMENU_INDEX_CLOSED = 40,
    OVERLAY_SUBMENU_CLOSE_MILLIS = 500,
    LABEL_ANIMATION_MENU = 6,
    LABEL_TYPE_FOCUSED = 1,
    LABEL_TYPE_NORMAL = 2,
};

enum build_menu_text_group {
    BUILD_MENU_TEXT_OVERLAYS = 14,
    BUILD_MENU_TEXT_BUILDINGS = 28,
    BUILD_MENU_TEXT_TEMPLE_COMPLEX_UPGRADES = 189,
    BUILD_MENU_TEXT_MONUMENTS = 198,
    BUILD_MENU_NO_BRUSH_TEXT = 0x7FFFFFFF,
};

enum build_menu_editor_group {
    BUILD_MENU_EDITOR_TWO_RUN_BRUSHES = 17,
    BUILD_MENU_EDITOR_FIRST_RUN_LAST = 8,
};

enum build_menu_colors {
    BUILD_MENU_COLOR_FOCUSED = 0xFAC1,
    BUILD_MENU_COLOR_NORMAL = 0x3EFA,
    BUILD_MENU_ROW_HEIGHT = 24,
};

static int32_t view_width(void)
{
    return g_sidebar_collapsed == 1 ? city_view_width_collapsed : city_view_width_expanded;
}

static int32_t editor_brush_text_offset(int32_t group)
{
    switch (group) {
        case 14:
            return -1;
        case 15:
            return 4;
        case 16:
            return 6;
        case 18:
            return 17;
        case 19:
            return 19;
        case 26:
            return 22;
        case 27:
            return 34;
        default:
            return BUILD_MENU_NO_BRUSH_TEXT;
    }
}

static int32_t editor_animal_text(int32_t row)
{
    if (row != 0) {
        return scenario_data.climate_type + FIGURE_ASP;
    }
    if (scenario_data.climate_type == CLIMATE_DESERT) {
        return FIGURE_HYENA;
    }
    if (scenario_data.climate_type == CLIMATE_NORTHERN) {
        return FIGURE_CROCODILE;
    }
    return FIGURE_HIPPO;
}

static int32_t monument_entry_hidden(int32_t item, int32_t monument)
{
    city_player *player = city_player_current();
    if ((item == 1 || item == 2) && monument == scenario_data.monument_def_ids[0] && player->monument_def_id[0] == 0) {
        return 1;
    }
    if (item == 2 && monument == scenario_data.monument_def_ids[1] && player->monument_def_id[1] == 0) {
        return 1;
    }
    return monument == 0;
}

static void build_menu_draw_short_label(int32_t width, int32_t y, int32_t type)
{
    label_draw_animated_ui(LABEL_ANIMATION_MENU, width - 170, y + 110, 10, 1, type, ui_origin_x, ui_origin_y);
}

static void build_menu_draw_short_text(int32_t group, int32_t text, int32_t width, int32_t y, int32_t font, int32_t color)
{
    lang_text_draw_centered_ui(group, text, width - 170, y + 113, 160, font, color, ui_origin_x, ui_origin_y);
}

static void build_menu_draw_building(int32_t item, int32_t width, int32_t y, int32_t type, int32_t font, int32_t color)
{
    int32_t width_blocks = ui_layout_label_columns_get() + 7;
    int32_t label_width = ui_layout_label_width_get();
    label_draw_animated_ui(LABEL_ANIMATION_MENU, width - label_width - 97, y + 110, width_blocks, 1, type, ui_origin_x, ui_origin_y);
    int32_t building_type = build_menu_types[BUILD_MENU_TYPE_FIRST + (build_menu_group * BUILD_MENU_ITEMS) + item];
    label_width = ui_layout_label_width_get();
    if (build_menu_group == BUILD_MENU_TEMPLE_COMPLEX_UPGRADES) {
        int32_t complex_type = building_get(city_player_current()->temple_complex_building_id)->type;
        int32_t text = item + ((complex_type - BUILDING_TEMPLE_COMPLEX_OSIRIS) * 2);
        lang_text_draw_centered_ui(
            BUILD_MENU_TEXT_TEMPLE_COMPLEX_UPGRADES, text, width - label_width - 97, y + 113, 176, font, color, ui_origin_x, ui_origin_y);
    } else {
        lang_text_draw_centered_ui(BUILD_MENU_TEXT_BUILDINGS, building_type, width - label_width - 89, y + 113, 176, font, color, ui_origin_x, ui_origin_y);
    }
    if (building_type == BUILDING_MENU_WATER_LIFT) {
        building_type = BUILDING_WATER_LIFT;
    }
    if (building_type != BUILDING_FORT && building_model_get(building_type)->cost != 0) {
        text_draw_number_ui(building_model_get(building_type)->cost, NUMBER_PREFIX_NONE, money_postfix, width - 82, y + 114, font, COLOR_BLACK, ui_origin_x, ui_origin_y);
    }
}

void window_build_menu_draw_foreground(void)
{
    int32_t width = editor_active == 1 ? city_view_width_expanded : view_width();
    int32_t item = -1;
    for (int32_t row = 0; row < build_menu_item_count; row++) {
        item = build_menu_next_item(build_menu_group, item);
        int32_t focused = focus_button_id == row + 1;
        int32_t type = focused ? LABEL_TYPE_FOCUSED : LABEL_TYPE_NORMAL;
        int32_t color = focused ? BUILD_MENU_COLOR_FOCUSED : BUILD_MENU_COLOR_NORMAL;
        int32_t font = focused ? image_group(IMAGE_GROUP_FONT_GREEN) : image_group(IMAGE_GROUP_FONT_BLACK);
        int32_t group = build_menu_group;
        int32_t y = build_menu_y_offset + (row * BUILD_MENU_ROW_HEIGHT);
        int32_t building_type = build_menu_types[BUILD_MENU_TYPE_FIRST + (group * BUILD_MENU_ITEMS) + item];
        int32_t brush_offset = editor_brush_text_offset(group);
        if (brush_offset != BUILD_MENU_NO_BRUSH_TEXT) {
            build_menu_draw_short_label(width, y, type);
            build_menu_draw_short_text(LANG_GROUP_EDITOR_BRUSHES, building_type + brush_offset, width, y, font, color);
        } else if (group == BUILD_MENU_EDITOR_TWO_RUN_BRUSHES) {
            build_menu_draw_short_label(width, y, type);
            int32_t text = building_type <= BUILD_MENU_EDITOR_FIRST_RUN_LAST ? building_type + 9 : building_type + 39;
            build_menu_draw_short_text(LANG_GROUP_EDITOR_BRUSHES, text, width, y, font, color);
        } else if (group == BUILD_MENU_EDITOR_ANIMALS) {
            build_menu_draw_short_label(width, y, type);
            build_menu_draw_short_text(LANG_GROUP_FIGURE_TYPES, editor_animal_text(row), width, y, font, color);
        } else if (group == BUILD_MENU_MONUMENTS) {
            int32_t monument = scenario_data.monument_def_ids[building_type - 1];
            if (!monument_entry_hidden(item, monument)) {
                label_draw_animated_ui(LABEL_ANIMATION_MENU, width - 298, y + 110, 18, 1, type, ui_origin_x, ui_origin_y);
                lang_text_draw_centered_ui(BUILD_MENU_TEXT_MONUMENTS, monument, width - 266, y + 113, 176, font, color, ui_origin_x, ui_origin_y);
            }
        } else {
            build_menu_draw_building(item, width, y, type, font, color);
        }
    }
}

static void overlay_menu_draw_entry(int32_t text, int32_t x, int32_t y, int32_t type)
{
    label_draw_animated_ui(LABEL_ANIMATION_MENU, x, y, OVERLAY_MENU_LABEL_BLOCKS, 1, type, ui_origin_x, ui_origin_y);
    lang_text_draw_centered_ui(BUILD_MENU_TEXT_OVERLAYS, text, x, y + 3, OVERLAY_MENU_TEXT_WIDTH, image_group(IMAGE_GROUP_FONT_BLACK), COLOR_BLACK, ui_origin_x, ui_origin_y);
}

void window_overlay_menu_draw_foreground(void)
{
    int32_t width = view_width();
    for (int32_t i = 0; i < OVERLAY_MENU_ENTRIES; i++) {
        int32_t type = (focus_button_id == i + 1 && overlay_submenu_open <= 0) ? LABEL_TYPE_FOCUSED : LABEL_TYPE_NORMAL;
        overlay_menu_draw_entry(build_menu_types[OVERLAY_MENU_OVERLAYS_FIRST + i], width - 170, build_menu_y_offset + (i * OVERLAY_MENU_ROW_HEIGHT) + 34, type);
    }
    if (overlay_submenu_open <= 0) {
        return;
    }
    image_draw(image_group(IMAGE_GROUP_BULLET), width - 185, build_menu_y_offset + (overlay_submenu_index * OVERLAY_MENU_ROW_HEIGHT) + 40);
    const int32_t *submenu = &build_menu_types[OVERLAY_SUBMENU_OVERLAYS_FIRST + overlay_submenu_open * OVERLAY_SUBMENU_MAX_ENTRIES];
    for (int32_t k = 0; k < overlay_submenu_count; k++) {
        int32_t type = focus_button_id_secondary == k + 1 ? LABEL_TYPE_FOCUSED : LABEL_TYPE_NORMAL;
        overlay_menu_draw_entry(submenu[k], width - 348, build_menu_y_offset + ((overlay_submenu_index + k) * OVERLAY_MENU_ROW_HEIGHT) + 34, type);
    }
}

static void overlay_menu_close(void)
{
    window_current_id = WINDOW_CITY;
    window_redraw_request = 1;
    royal_tombs_update_all_images();
}

void overlay_submenu_select(void)
{
    int32_t overlay = build_menu_types[OVERLAY_SUBMENU_OVERLAYS_FIRST + (overlay_submenu_open * OVERLAY_SUBMENU_MAX_ENTRIES) + button_parameter];
    if (overlay != OVERLAY_NONE) {
        city_overlay_previous = OVERLAY_NONE;
        city_overlay = overlay;
    }
    overlay_menu_close();
}

void window_overlay_menu_show(void)
{
    window_current_id = WINDOW_OVERLAY_MENU;
    build_menu_item_count = OVERLAY_MENU_ITEM_COUNT;
    build_menu_y_offset = OVERLAY_MENU_FIRST_ROW_Y;
    overlay_submenu_open = 0;
    overlay_submenu_count = 0;
    overlay_submenu_index = OVERLAY_SUBMENU_INDEX_CLOSED;
    window_redraw_request = 1;
}

int32_t overlay_menu_submenu_count(int32_t id)
{
    const int32_t *submenu = &build_menu_types[OVERLAY_SUBMENU_OVERLAYS_FIRST + id * OVERLAY_SUBMENU_MAX_ENTRIES];
    int32_t count = 0;
    while (count < OVERLAY_SUBMENU_MAX_ENTRIES && submenu[count] > 0) {
        count++;
    }
    return count;
}

int32_t overlay_menu_handle_submenu(void)
{
    int32_t focus = focus_button_id;
    if (focus == 0 && focus_button_id_secondary == 0) {
        if ((time_current_millis - overlay_menu_hover_time) > OVERLAY_SUBMENU_CLOSE_MILLIS) {
            overlay_submenu_open = 0;
            overlay_submenu_count = 0;
        }
        return 0;
    }
    overlay_menu_hover_time = time_current_millis;
    if (focus != 0) {
        int32_t id = overlay_submenu_ids[focus];
        overlay_submenu_index = focus - 1;
        overlay_submenu_open = id;
        overlay_submenu_count = overlay_menu_submenu_count(id);
    }
    return 0;
}

static int32_t overlay_shown_directly(int32_t overlay)
{
    return overlay == OVERLAY_RELIGION || overlay == OVERLAY_WATER || overlay == OVERLAY_PROBLEMS || overlay == OVERLAY_HIDE_CLIFFS;
}

void overlay_menu_select(void)
{
    int32_t overlay = build_menu_types[OVERLAY_MENU_OVERLAYS_FIRST + button_parameter];
    if (overlay == OVERLAY_NONE) {
        int32_t previous = city_overlay;
        city_overlay = OVERLAY_NONE;
        city_overlay_previous = previous;
    } else if (overlay_shown_directly(overlay)) {
        city_overlay_previous = OVERLAY_NONE;
        city_overlay = overlay;
    }
    overlay_menu_close();
}

void overlay_menu_draw_background(void)
{
    widget_sidebar_draw_background();
    widget_city_draw_current_view();
    if (window_redraw_request >= 2) {
        label_animation[LABEL_SLOT_MENU_DRAW] = 0;
        window_overlay_menu_draw_foreground();
    }
}
