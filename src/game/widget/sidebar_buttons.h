// The sidebar's buttons and layout (top, right edge per screen width, panel images, button counts) and the
// build menu pictures per build menu group.

#ifndef WIDGET_SIDEBAR_BUTTONS_H
#define WIDGET_SIDEBAR_BUTTONS_H

#include <stdint.h>

enum {
    SIDEBAR_BUILD_GROUPS = 13,
};

enum sidebar_layout {
    SIDEBAR_TOP = 30,
    SIDEBAR_RIGHT_EDGE_800 = 760,
    SIDEBAR_RIGHT_EDGE_1024 = 1000,
    SIDEBAR_EXPANDED_EXTRA_BUTTONS = 2,
    SIDEBAR_EXPANDED_BUILD_BUTTONS = 16,
    SIDEBAR_EXPANDED_TOP_BUTTONS = 2,
    SIDEBAR_COLLAPSED_EXPAND_BUTTONS = 1,
    SIDEBAR_COLLAPSED_BUILD_BUTTONS = 12,
};

enum sidebar_expanded_build_button {
    SIDEBAR_BUTTON_UNDO = 12,
    SIDEBAR_BUTTON_MESSAGES = 13,
    SIDEBAR_BUTTON_PROBLEMS = 14,
    SIDEBAR_BUTTON_BRIEFING = 15,
};

enum sidebar_panel_image {
    SIDEBAR_PANEL_EXPANDED = 0,
    SIDEBAR_PANEL_COLLAPSED = 1,
    SIDEBAR_PANEL_FILLER = 2,
    SIDEBAR_PANEL_TOP = 3,
    SIDEBAR_PANEL_BOTTOM_1024 = 4,
    SIDEBAR_PANEL_BOTTOM_1024_COLLAPSED = 5,
    SIDEBAR_PANEL_BOTTOM_800 = 6,
    SIDEBAR_PANEL_BOTTOM_800_COLLAPSED = 7,
};

typedef struct sidebar_build_image {
    int32_t image;
} sidebar_build_image;

extern sidebar_build_image sidebar_build_images[SIDEBAR_BUILD_GROUPS];

int32_t widget_sidebar_draw_buttons(void);
void widget_sidebar_draw_overlay_text(int32_t force, int32_t x);
int32_t widget_sidebar_draw_build_image(int32_t force, int32_t x);
int32_t widget_sidebar_draw_filler(void);
int32_t sidebar_draw_top_image(void);
void city_button_go_to_problem(void);
void city_button_undo(void);
void sidebar_build_button_click(void);
void sidebar_collapse(void);
void build_sidebar_hover_update(void);

#endif
