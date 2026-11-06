// The sidebar: the minimap, the messages count with the buttons it enables, the city's or editor's sidebar
// background, and the slide between the expanded and collapsed sidebar. On screen sizes other than the
// standard ones the 1024 layout's pieces are repeated to fill the screen.

#include "widget/sidebar.h"
#include "city/city_data.h"
#include "editor/editor_sidebar.h"
#include "game/game_data.h"
#include "game/game_internal.h"
#include "game/startup_load.h"
#include "graphics/graphics_data.h"
#include "graphics/graphics_present.h"
#include "graphics/graphics_primitives.h"
#include "graphics/image_buttons.h"
#include "graphics/image_collection.h"
#include "graphics/image_draw.h"
#include "graphics/text_draw.h"
#include "graphics/ui_layout.h"
#include "map/grid.h"
#include "text/lang.h"
#include "widget/city_draw_view.h"
#include "widget/city_view.h"
#include "widget/city_viewport.h"
#include "widget/minimap.h"
#include "widget/screen_layout.h"
#include "widget/sidebar_buttons.h"
#include "widget/top_menu.h"
#include "widget/widget_data.h"
#include "widget/widget_image_group.h"
#include "window/window_data.h"
#include "window/window_draw.h"
#include "window/window_id.h"
#include "window/window_image_group.h"
#include <stdint.h>

int32_t g_drawn_message_total;

enum sidebar_screen {
    SCREEN_WIDTH_800 = 800,
    SCREEN_WIDTH_1024 = 1024,
    BOTTOM_BAR_HEIGHT_1024 = 9,
    BOTTOM_BAR_HEIGHT = 6,
    EDITOR_SIDEBAR_TOP = 24,
    EDITOR_SIDEBAR_LEFT = 6,
    SIDEBAR_BOTTOM_PANEL_Y = 480,
    SIDEBAR_TEXT_X = 11,
};

enum sidebar_editor_panel_image {
    EDITOR_PANEL_BOTTOM_800 = 1,
    EDITOR_PANEL_BOTTOM_1024 = 2,
};

enum sidebar_editor_minimap_frame {
    EDITOR_FRAME_LEFT = 7,
    EDITOR_FRAME_TOP = 35,
    EDITOR_FRAME_RIGHT = 153,
    EDITOR_FRAME_BOTTOM = 147,
    EDITOR_FRAME_DARK = 0x4208,
    EDITOR_FRAME_LIGHT = 0xC618,
    EDITOR_BUTTONS = 15,
    EDITOR_TITLE_Y = 152,
    EDITOR_TITLE_TEXT = 132,
    EDITOR_NAME_X = 94,
};

enum sidebar_messages {
    MESSAGES_X = 48,
    MESSAGES_Y = 452,
    MESSAGES_WIDTH = 32,
    MESSAGES_SHADOW_COLOR = 0,
    MESSAGES_COLOR = 0xFFFF,
};

enum sidebar_slide {
    SLIDE_SPEED_1 = 24,
    SLIDE_SPEED_2 = 60,
    SLIDE_SPEED_3 = 96,
    SLIDE_MINIMAP_CLEAR_LEFT = 8,
    SLIDE_MINIMAP_CLEAR_WIDTH = 145,
};

void widget_sidebar_draw_minimap(int32_t force)
{
    if (g_sidebar_collapsed != 0) {
        return;
    }
    int32_t dirty = 0;
    if (minimap_invalidate_flag != 0) {
        minimap_invalidate_flag = 0;
        dirty = 1;
    }
    if (editor_active == 1 && construction_in_progress != 0) {
        dirty = 1;
    }
    if (scroll_in_progress != 0) {
        dirty = 1;
    }
    if (force == 0 && dirty == 0) {
        return;
    }
    if (editor_active == 0) {
        widget_minimap_draw(city_view_width_expanded + MINIMAP_SIDEBAR_DRAW_X, MINIMAP_SIDEBAR_Y, MINIMAP_WIDTH, MINIMAP_HEIGHT);
        return;
    }
    int32_t left = city_view_width_expanded + EDITOR_FRAME_LEFT;
    int32_t right = city_view_width_expanded + EDITOR_FRAME_RIGHT;
    widget_minimap_draw(city_view_width_expanded + MINIMAP_EDITOR_X, MINIMAP_EDITOR_DRAW_Y, MINIMAP_WIDTH, MINIMAP_HEIGHT);
    graphics_draw_line(left, EDITOR_FRAME_TOP, right, EDITOR_FRAME_TOP, EDITOR_FRAME_DARK);
    graphics_draw_line(left, EDITOR_FRAME_TOP + 1, left, EDITOR_FRAME_BOTTOM, EDITOR_FRAME_DARK);
    graphics_draw_line(right, EDITOR_FRAME_TOP + 1, right, EDITOR_FRAME_BOTTOM, EDITOR_FRAME_LIGHT);
}

void widget_sidebar_draw_number_of_messages(int32_t force)
{
    if (editor_active == 1 || window_current_id != WINDOW_CITY || g_sidebar_collapsed != 0) {
        return;
    }
    if (city_message_total == g_drawn_message_total && force == 0) {
        return;
    }
    g_drawn_message_total = city_message_total;
    buttons_expanded_build[SIDEBAR_BUTTON_UNDO].enabled = (undo_ready != 0 && undo_available != 0);
    buttons_expanded_build[SIDEBAR_BUTTON_MESSAGES].enabled = (city_message_total > 0);
    buttons_expanded_build[SIDEBAR_BUTTON_PROBLEMS].enabled = (city_message_problem_count > 0);
    buttons_expanded_build[SIDEBAR_BUTTON_BRIEFING].enabled = (settings_data.is_custom_scenario == 0);
    if (city_message_total != 0) {
        int32_t x = city_view_width_expanded + MESSAGES_X;
        text_draw_number_centered(city_message_total, NUMBER_PREFIX_NONE, postfix_none, x, MESSAGES_Y, MESSAGES_WIDTH, image_group(IMAGE_GROUP_FONT_SMALL), MESSAGES_SHADOW_COLOR);
        text_draw_number_centered(city_message_total, NUMBER_PREFIX_NONE, postfix_none, x - 1, MESSAGES_Y + 1, MESSAGES_WIDTH, image_group(IMAGE_GROUP_FONT_SMALL), MESSAGES_COLOR);
    }
}

static void draw_bottom_panels_of_screen_layout(int32_t tall_image, int32_t short_image, int32_t x)
{
    if (g_screen_height - SIDEBAR_BOTTOM_PANEL_Y <= SCREEN_LAYOUT_SHORT_BOTTOM_PANEL_HEIGHT) {
        image_draw(short_image, x, SIDEBAR_BOTTOM_PANEL_Y);
        return;
    }
    for (int32_t y = SIDEBAR_BOTTOM_PANEL_Y; y < g_screen_height; y += SCREEN_LAYOUT_BOTTOM_PANEL_HEIGHT) {
        image_draw(tall_image, x, y);
    }
}

static void draw_city_bottom_panels_of_screen_layout(int32_t collapsed)
{
    if (collapsed) {
        draw_bottom_panels_of_screen_layout(image_group(IMAGE_GROUP_PANEL) + SIDEBAR_PANEL_BOTTOM_1024_COLLAPSED, image_group(IMAGE_GROUP_PANEL) + SIDEBAR_PANEL_BOTTOM_800_COLLAPSED,
            city_view_width_collapsed);
    } else {
        draw_bottom_panels_of_screen_layout(image_group(IMAGE_GROUP_PANEL) + SIDEBAR_PANEL_BOTTOM_1024, image_group(IMAGE_GROUP_PANEL) + SIDEBAR_PANEL_BOTTOM_800, city_view_width_expanded);
    }
}

static void draw_background_of_screen_layout(void)
{
    for (int32_t x = screen_layout_sidebar_right(); x < g_screen_width; x += SCREEN_LAYOUT_FILLER_WIDTH) {
        for (int32_t y = 0; y < g_screen_height; y += SCREEN_LAYOUT_FILLER_HEIGHT) {
            image_draw(image_group(IMAGE_GROUP_PANEL) + SIDEBAR_PANEL_FILLER, x, y);
        }
    }
    int32_t bar_height = screen_layout_bottom_bar_height();
    graphics_fill_rect(0, g_screen_height - bar_height, g_screen_width, bar_height, 0);
}

static void draw_city_sidebar_bottom(void)
{
    if (!screen_layout_is_standard_size()) {
        if (g_sidebar_collapsed == 0 || g_sidebar_collapsed == 1) {
            draw_city_bottom_panels_of_screen_layout(g_sidebar_collapsed);
        }
        return;
    }
    if (g_screen_width == SCREEN_WIDTH_1024) {
        if (g_sidebar_collapsed == 0) {
            image_draw(image_group(IMAGE_GROUP_PANEL) + SIDEBAR_PANEL_BOTTOM_1024, city_view_width_expanded, SIDEBAR_BOTTOM_PANEL_Y);
        } else if (g_sidebar_collapsed == 1) {
            image_draw(image_group(IMAGE_GROUP_PANEL) + SIDEBAR_PANEL_BOTTOM_1024_COLLAPSED, city_view_width_collapsed, SIDEBAR_BOTTOM_PANEL_Y);
        }
    } else if (g_screen_width == SCREEN_WIDTH_800) {
        if (g_sidebar_collapsed == 0) {
            image_draw(image_group(IMAGE_GROUP_PANEL) + SIDEBAR_PANEL_BOTTOM_800, city_view_width_expanded, SIDEBAR_BOTTOM_PANEL_Y);
        } else if (g_sidebar_collapsed == 1) {
            image_draw(image_group(IMAGE_GROUP_PANEL) + SIDEBAR_PANEL_BOTTOM_800_COLLAPSED, city_view_width_collapsed, SIDEBAR_BOTTOM_PANEL_Y);
        }
    }
}

static void draw_city_sidebar(void)
{
    int32_t panel_image = image_group(IMAGE_GROUP_PANEL) + SIDEBAR_PANEL_EXPANDED;
    int32_t panel_x = city_view_width_expanded;
    if (g_sidebar_collapsed != 0) {
        panel_image = image_group(IMAGE_GROUP_PANEL) + SIDEBAR_PANEL_COLLAPSED;
        panel_x = city_view_width_collapsed;
    }
    image_draw(panel_image, panel_x, SIDEBAR_TOP);
    widget_sidebar_draw_buttons();
    widget_sidebar_draw_overlay_text(1, city_view_width_expanded + SIDEBAR_TEXT_X);
    widget_sidebar_draw_build_image(0, city_view_width_expanded + SIDEBAR_TEXT_X);
    widget_sidebar_draw_minimap(1);
    draw_city_sidebar_bottom();
}

static void draw_editor_sidebar(void)
{
    graphics_and_rect(city_view_width_expanded + EDITOR_SIDEBAR_LEFT, EDITOR_SIDEBAR_TOP, (g_screen_width - city_view_width_expanded) - EDITOR_SIDEBAR_LEFT,
        screen_height - EDITOR_SIDEBAR_TOP, 0);
    widget_city_draw_current_view();
    image_draw(image_group(IMAGE_GROUP_EDITOR_SIDEBAR), city_view_width_expanded, EDITOR_SIDEBAR_TOP);
    if (!screen_layout_is_standard_size()) {
        draw_bottom_panels_of_screen_layout(
            image_group(IMAGE_GROUP_EDITOR_SIDEBAR) + EDITOR_PANEL_BOTTOM_1024, image_group(IMAGE_GROUP_EDITOR_SIDEBAR) + EDITOR_PANEL_BOTTOM_800, city_view_width_expanded);
    } else if (g_screen_width == SCREEN_WIDTH_1024) {
        image_draw(image_group(IMAGE_GROUP_EDITOR_SIDEBAR) + EDITOR_PANEL_BOTTOM_1024, city_view_width_expanded, SIDEBAR_BOTTOM_PANEL_Y);
    } else if (g_screen_width == SCREEN_WIDTH_800) {
        image_draw(image_group(IMAGE_GROUP_EDITOR_SIDEBAR) + EDITOR_PANEL_BOTTOM_800, city_view_width_expanded, SIDEBAR_BOTTOM_PANEL_Y);
    }
    editor_sidebar_draw_text();
    image_buttons_draw(city_view_width_expanded, EDITOR_SIDEBAR_TOP, buttons_editor.buttons, EDITOR_BUTTONS);
    widget_sidebar_draw_minimap(1);
    lang_text_draw_ui(LANG_GROUP_EDITOR, EDITOR_TITLE_TEXT, city_view_width_expanded + SIDEBAR_TEXT_X, EDITOR_TITLE_Y, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_ui(LANG_GROUP_MENU_OPTIONS, 0, city_view_width_expanded + EDITOR_NAME_X, EDITOR_TITLE_Y, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
}

void widget_sidebar_draw_background(void)
{
    if (!screen_layout_is_standard_size()) {
        draw_background_of_screen_layout();
    } else {
        if (g_screen_width == SCREEN_WIDTH_1024) {
            widget_sidebar_draw_filler();
        } else if (g_screen_width == SCREEN_WIDTH_800) {
            sidebar_draw_top_image();
        }
        if (g_screen_width == SCREEN_WIDTH_1024) {
            graphics_fill_rect(0, g_screen_height - BOTTOM_BAR_HEIGHT_1024, SCREEN_WIDTH_1024, BOTTOM_BAR_HEIGHT_1024, 0);
        } else {
            graphics_fill_rect(0, g_screen_height - BOTTOM_BAR_HEIGHT, g_screen_width, BOTTOM_BAR_HEIGHT, 0);
        }
    }
    window_ui_redraw = 1;
    if (editor_active == 0) {
        draw_city_sidebar();
    } else {
        draw_editor_sidebar();
    }
    widget_top_menu_draw(1);
    widget_sidebar_draw_number_of_messages(1);
}

static int32_t slide_step_increment(void)
{
    if (g_slide_speed_metric <= SLIDE_SPEED_1) {
        return 1;
    }
    if (g_slide_speed_metric <= SLIDE_SPEED_2) {
        return 2;
    }
    return (g_slide_speed_metric <= SLIDE_SPEED_3) ? 3 : 4;
}

static void finish_slide(void)
{
    if (g_sidebar_collapsed == 0) {
        city_view_set_viewport_without_sidebar();
        g_sidebar_collapsed = 1;
    } else {
        city_view_set_viewport_with_sidebar();
        g_sidebar_collapsed = 0;
    }
    window_current_id = WINDOW_CITY;
    city_view_check_camera_boundaries();
    window_draw_background_dispatch(1);
}

void widget_sidebar_slide(void)
{
    int32_t step = g_sidebar_slide_step + slide_step_increment();
    g_sidebar_slide_step = step;
    if (step >= SIDEBAR_SLIDE_STEPS) {
        finish_slide();
        return;
    }

    int32_t offset = (g_sidebar_collapsed == 1) ? sidebar_slide_offsets[SIDEBAR_SLIDE_STEPS - step] : sidebar_slide_offsets[step];
    int32_t expanded_x = city_view_width_expanded + offset;

    widget_city_draw_current_view();
    if (!screen_layout_is_standard_size()) {
        g_clip_x1 = screen_layout_sidebar_right();
    } else if (g_screen_width == SCREEN_WIDTH_1024) {
        g_clip_x1 = SIDEBAR_RIGHT_EDGE_1024;
    } else if (g_screen_width == SCREEN_WIDTH_800) {
        g_clip_x1 = SIDEBAR_RIGHT_EDGE_800;
    }
    window_ui_redraw = 1;

    image_draw(image_group(IMAGE_GROUP_PANEL) + SIDEBAR_PANEL_COLLAPSED, city_view_width_collapsed, SIDEBAR_TOP);
    image_buttons_draw(city_view_width_collapsed, SIDEBAR_TOP, buttons_collapsed_build, SIDEBAR_COLLAPSED_BUILD_BUTTONS);
    image_buttons_draw(city_view_width_collapsed, SIDEBAR_TOP, buttons_collapsed_expand.buttons, SIDEBAR_COLLAPSED_EXPAND_BUTTONS);
    image_draw(image_group(IMAGE_GROUP_PANEL) + SIDEBAR_PANEL_EXPANDED, expanded_x, SIDEBAR_TOP);
    image_buttons_draw(expanded_x, SIDEBAR_TOP, buttons_expanded_build, SIDEBAR_EXPANDED_BUILD_BUTTONS);
    image_buttons_draw(expanded_x, SIDEBAR_TOP, buttons_expanded_extra, SIDEBAR_EXPANDED_EXTRA_BUTTONS);
    image_buttons_draw(expanded_x, SIDEBAR_TOP, buttons_expanded_top, SIDEBAR_EXPANDED_TOP_BUTTONS);
    widget_sidebar_draw_overlay_text(1, expanded_x + SIDEBAR_TEXT_X);
    widget_sidebar_draw_build_image(1, expanded_x + SIDEBAR_TEXT_X);

    int32_t clip_width = (offset > SLIDE_MINIMAP_CLEAR_LEFT) ? SLIDE_MINIMAP_CLEAR_LEFT : offset;
    graphics_fill_rect(expanded_x + SLIDE_MINIMAP_CLEAR_LEFT, MINIMAP_SIDEBAR_Y, (clip_width - offset) + SLIDE_MINIMAP_CLEAR_WIDTH, MINIMAP_HEIGHT, 0);

    if (!screen_layout_is_standard_size()) {
        draw_city_bottom_panels_of_screen_layout(1);
        draw_bottom_panels_of_screen_layout(image_group(IMAGE_GROUP_PANEL) + SIDEBAR_PANEL_BOTTOM_1024, image_group(IMAGE_GROUP_PANEL) + SIDEBAR_PANEL_BOTTOM_800, expanded_x);
    } else if (g_screen_width == SCREEN_WIDTH_1024) {
        image_draw(image_group(IMAGE_GROUP_PANEL) + SIDEBAR_PANEL_BOTTOM_1024_COLLAPSED, city_view_width_collapsed, SIDEBAR_BOTTOM_PANEL_Y);
        image_draw(image_group(IMAGE_GROUP_PANEL) + SIDEBAR_PANEL_BOTTOM_1024, expanded_x, SIDEBAR_BOTTOM_PANEL_Y);
    } else if (g_screen_width == SCREEN_WIDTH_800) {
        image_draw(image_group(IMAGE_GROUP_PANEL) + SIDEBAR_PANEL_BOTTOM_800_COLLAPSED, city_view_width_collapsed, SIDEBAR_BOTTOM_PANEL_Y);
        image_draw(image_group(IMAGE_GROUP_PANEL) + SIDEBAR_PANEL_BOTTOM_800, expanded_x, SIDEBAR_BOTTOM_PANEL_Y);
    }
    g_clip_x1 = g_screen_width;
}
