// The scenario editor's empire object handlers: the ornament image arrows, placing, moving and nudging an object
// with the mouse, the object under a right click, the map image arrows, reloading the map's terrain from
// Pharaohmaps.bmp, and the editors' warning strip. A new city is refused with a popup when the trade cities 1..60
// are all in use.

#include "window/empire_editor_input.h"
#include "core/calc.h"
#include "editor/editor_data.h"
#include "editor/editor_new_scenario.h"
#include "editor/empire_editor_objects.h"
#include "empire/empire.h"
#include "empire/empire_helpers.h"
#include "empire/empire_types.h"
#include "game/game_internal.h"
#include "graphics/graphics_data.h"
#include "graphics/image_collection.h"
#include "graphics/panel_draw.h"
#include "graphics/ui_layout.h"
#include "input/input_data.h"
#include "map/grid.h"
#include "window/advisors_internal.h"
#include "window/popup_dialog.h"
#include "window/window_id.h"
#include "window/window_image_group.h"
#include <string.h>

enum {
    EMPIRE_TEXT_OBJECT_WIDTH = 90,
    EMPIRE_TEXT_OBJECT_HEIGHT = 20,
    EMPIRE_OBJECT_PICK_MARGIN = 8,
    EMPIRE_OBJECT_PICK_FAR = 10000,
    EMPIRE_OBJECT_DRAGGED = 1,
    EMPIRE_OBJECT_DROPPED = 2,
    POPUP_TOO_MANY_CITIES = 34,
    SCENARIO_IMAGE_LAST = 26,
};

enum image_arrow {
    IMAGE_ARROW_NEXT = 0,
    IMAGE_ARROW_PREVIOUS = 1,
    IMAGE_ARROW_FIRST = 2,
    IMAGE_ARROW_MIDDLE = 3,
    IMAGE_ARROW_LAST = 4,
};

void empire_editor_image_arrow(void)
{
    int32_t first = image_group(IMAGE_GROUP_EMPIRE_ORNAMENTS);
    int32_t last = image_group(IMAGE_GROUP_EMPIRE_ORNAMENTS_MAX);
    int32_t image;
    switch (button_parameter) {
        case IMAGE_ARROW_PREVIOUS:
            image = empire_editor_image - 1;
            break;
        case IMAGE_ARROW_NEXT:
            image = empire_editor_image + 1;
            break;
        case IMAGE_ARROW_FIRST:
            image = first;
            break;
        case IMAGE_ARROW_MIDDLE:
            image = ((last - first) / 2) + first;
            break;
        case IMAGE_ARROW_LAST:
            image = last;
            break;
        default:
            image = empire_editor_image;
            break;
    }
    empire_editor_image = image;
    if (image > last) {
        image = first;
        empire_editor_image = image;
    }
    if (image < first) {
        image = last;
        empire_editor_image = image;
    }
    if (empire_ui_mode == EMPIRE_EDITOR_MODE_EDIT_OBJECTS) {
        empire_object *o = &empire_map_objects[g_empire_selected_object];
        if (o->in_use != 0) {
            o->image_id = (uint16_t) image;
            o->image_offset = (int16_t) (image - first);
        }
    }
    window_redraw_request = 1;
}

static int32_t mouse_on_empire_map(void)
{
    return mouse_x >= empire_border_x && mouse_x < display_width - (empire_border_x * 2) && mouse_y >= empire_border_y &&
        mouse_y < screen_height - empire_border_bottom - empire_border_y;
}

static int32_t trade_cities_in_use(void)
{
    int32_t count = 1;
    for (int32_t i = 1; i < EMPIRE_CITY_COUNT; i++) {
        if (empire_cities[i].in_use != 0) {
            count++;
        }
    }
    return count;
}

void empire_editor_place_object(int32_t object, int32_t keep)
{
    uint8_t released = mouse_left_went_up;
    if (mouse_left_down == 0 && released == 0 && mouse_left_went_down == 0) {
        return;
    }
    if (!mouse_on_empire_map()) {
        return;
    }
    int32_t mx = mouse_x;
    int32_t my = mouse_y;
    int32_t new_city = 0;
    empire_object *o = &empire_map_objects[object];
    if (o->in_use == 0) {
        int32_t kind = empire_editor_image_kind;
        int32_t image = empire_editor_image;
        o->type = (uint8_t) kind;
        o->image_id = (uint16_t) image;
        if (kind == EMPIRE_OBJECT_CITY) {
            if (trade_cities_in_use() >= EMPIRE_CITY_COUNT) {
                empire_ui_mode = EMPIRE_EDITOR_MODE_GENERAL;
                window_popup_dialog_show(POPUP_TOO_MANY_CITIES, button_none, 1);
                return;
            }
            new_city = 1;
        } else if (kind == EMPIRE_OBJECT_ORNAMENT) {
            int32_t index = image - image_group(IMAGE_GROUP_EMPIRE_ORNAMENTS);
            if (index >= image_group(IMAGE_GROUP_EMPIRE_ORNAMENTS_MAX)) {
                index = image_group(IMAGE_GROUP_EMPIRE_ORNAMENTS_MAX);
            }
            o->image_offset = (int16_t) index;
        }
    }
    g_empire_selected_object = object;
    o->in_use = EMPIRE_OBJECT_DRAGGED;
    if (released != 0) {
        o->in_use = EMPIRE_OBJECT_DROPPED;
    }
    int32_t image = o->image_id;
    const image_header *header = &image_collection.headers[image < image_collection.image_count ? image : 0];
    int32_t w = header->width;
    int32_t h = header->height;
    if (o->type == EMPIRE_OBJECT_TEXT) {
        w = EMPIRE_TEXT_OBJECT_WIDTH;
        h = EMPIRE_TEXT_OBJECT_HEIGHT;
    }
    int32_t x = mx - (w / 2) - empire_border_x + empire_scroll_x;
    int32_t y = my - (h / 2) - empire_border_y + empire_scroll_y;
    if (scenario_data.empire_expanded != 0) {
        o->expanded_x = (int16_t) x;
        o->expanded_y = (int16_t) y;
        if (keep == 0) {
            o->x = (int16_t) x;
            o->y = (int16_t) y;
        }
    } else {
        o->x = (int16_t) x;
        o->y = (int16_t) y;
    }
    o->width = (int16_t) w;
    o->height = (int16_t) h;
    if (new_city) {
        o->city_type = EMPIRE_CITY_EGYPTIAN_TRADING;
        empire_object_set_city_images();
        empire_object_init_cities();
    }
    empire_editor_route_dragging = 0;
    window_redraw_request = 1;
}

int32_t empire_editor_object_at_mouse(void)
{
    if (mouse_right_went_down == 0 || !mouse_on_empire_map()) {
        return 0;
    }
    int32_t px = empire_scroll_x - empire_border_x + mouse_x;
    int32_t py = empire_scroll_y - empire_border_y + mouse_y;
    int32_t best = EMPIRE_OBJECT_PICK_FAR;
    int32_t found = 0;
    for (int32_t i = 0; i < EMPIRE_OBJECT_COUNT; i++) {
        const empire_object *o = &empire_map_objects[i];
        if (o->in_use == 0) {
            break;
        }
        int32_t ox, oy;
        if (scenario_data.empire_expanded != 0) {
            ox = o->expanded_x;
            oy = o->expanded_y;
        } else {
            ox = o->x;
            oy = o->y;
        }
        int32_t w = o->width;
        int32_t h = o->height;
        if (ox - EMPIRE_OBJECT_PICK_MARGIN > px || ox + w + EMPIRE_OBJECT_PICK_MARGIN <= px || oy - EMPIRE_OBJECT_PICK_MARGIN > py ||
            oy + h + EMPIRE_OBJECT_PICK_MARGIN <= py) {
            continue;
        }
        int32_t d = calc_maximum_distance(px, py, (w / 2) + ox, (h / 2) + oy);
        if (d < best) {
            best = d;
            found = i + 1;
        }
    }
    window_redraw_request = 1;
    return found;
}

void editor_map_terrain_reload(void)
{
    editor_terrain_to_pharaohmaps(map_info.map_id);
    window_redraw_request = 1;
}

void editor_scenario_image_arrow(void)
{
    if (button_parameter == IMAGE_ARROW_PREVIOUS) {
        scenario_data.image--;
    } else if (button_parameter == IMAGE_ARROW_NEXT) {
        scenario_data.image++;
    }
    if (scenario_data.image < 0) {
        scenario_data.image = SCENARIO_IMAGE_LAST;
    } else if (scenario_data.image > SCENARIO_IMAGE_LAST) {
        scenario_data.image = 0;
    }
    window_redraw_request = 1;
}

void editor_warning_draw(int32_t text)
{
    int32_t x = ui_origin_x;
    int32_t y = ui_origin_y;
    outer_panel_draw(x + 16, y + 32, 35, 4);
    lang_text_draw_centered_ui(19, text - 2, x + 19, y + 48, 560, image_group(IMAGE_GROUP_FONT_SMALL), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_centered_ui(13, 1, x + 16, y + 68, 560, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
}

void empire_editor_nudge_object(int32_t direction, int32_t amount)
{
    if (window_current_id != WINDOW_EDITOR_EMPIRE || empire_ui_mode != EMPIRE_EDITOR_MODE_EDIT_OBJECTS) {
        return;
    }
    empire_object *o = &empire_map_objects[g_empire_selected_object];
    if (o->in_use == 0) {
        return;
    }
    int16_t step = (int16_t) amount;
    int16_t *x = (scenario_data.empire_expanded != 0) ? &o->expanded_x : &o->x;
    int16_t *y = (scenario_data.empire_expanded != 0) ? &o->expanded_y : &o->y;
    switch (direction) {
        case DIRECTION_TOP:
            *y -= step;
            break;
        case DIRECTION_RIGHT:
            *x += step;
            break;
        case DIRECTION_BOTTOM:
            *y += step;
            break;
        case DIRECTION_LEFT:
            *x -= step;
            break;
        default:
            break;
    }
    empire_editor_route_dragging = 0;
}
