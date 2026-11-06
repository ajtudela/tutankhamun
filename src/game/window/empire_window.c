// The empire map window: the border, the info panel (a city's trade, an event's tooltip, the hints), the city name and
// the map with its objects, routes and invasion and request markers (the map and the city name are shared with the
// scenario editor), its input and buttons, and opening a trade route (the popup and its cost).
//
// Routes on the map:
// - routes of at most one way point are marked free while drawn;
// - the open route button does nothing for a city whose open value is 0x32 or more.

#include "window/empire_window.h"
#include "city/finance_estimates.h"
#include "city/gods.h"
#include "city/message.h"
#include "city/resource_type.h"
#include "core/calc.h"
#include "editor/editor_data.h"
#include "game/game_data.h"
#include "graphics/graphics_data.h"
#include "input/input_data.h"
#include "io/io_data.h"
#include "editor/empire_editor_objects.h"
#include "editor/scenario_event_methods.h"
#include "empire/empire.h"
#include "empire/empire_helpers.h"
#include "empire/empire_route.h"
#include "empire/empire_types.h"
#include "empire/trade_status.h"
#include "game/game_internal.h"
#include "game/tutorial.h"
#include "graphics/graphics_present.h"
#include "graphics/graphics_primitives.h"
#include "graphics/image_collection.h"
#include "graphics/text_draw.h"
#include "scenario/event_helpers.h"
#include "scenario/event_manager.h"
#include "scenario/scenario_internal.h"
#include "text/text_lookup.h"
#include "widget/screen_layout.h"
#include "window/advisors_internal.h"
#include "window/building_menu.h"
#include "window/message_dialog.h"
#include "window/empire_scroll.h"
#include "window/main_menu.h"
#include "window/message_box.h"
#include "window/popup_dialog.h"
#include "window/window_id.h"
#include "window/window_image_group.h"

int32_t ew_anim_counter;

int32_t empire_marker_count;

int32_t ew_route_anim_counter;

enum {
    TRADE_ARROW_LARGE_LIMIT = 25,
    TRADE_ARROW_MEDIUM_LIMIT = 15,
    ANIMATION_FRAME_REVERSE = 128,
    ANIMATION_FRAME_NUMBER = 127,
    ANIMATION_COUNTER_MAX = 100,
    MAP_OBJECT_PICK_DISTANCE = 30,
    MAP_OBJECT_PICK_MARGIN = 8,
    TEXT_OBJECT_WIDTH = 90,
    TEXT_OBJECT_HEIGHT = 20,
    SELECTION_FRAME_MARGIN = 4,
    ROUTE_OPEN_LIMIT = 50,
    TREASURY_CREDIT_LIMIT = -5000,
    POPUP_NO_CREDIT = 119,
    POPUP_OPEN_LAND_ROUTE = 2,
    POPUP_OPEN_SEA_ROUTE = 4,
};

static const image_header *image_header_of(int32_t image)
{
    return &image_collection.headers[image < image_collection.image_count ? image : 0];
}

static int32_t city_name_group(void)
{
    return settings_data.classical_names ? 21 : 195;
}

static scenario_event *event_at(int32_t index)
{
    return (index >= 0 && index < scenario_events.count) ? &scenario_events.events[index] : 0;
}

void window_empire_draw_background(void)
{
    window_empire_draw_border();
    window_empire_draw_info_panel();
}

void window_empire_draw_border(void)
{
    int32_t width = g_screen_width;
    int32_t count = width / 70;
    int32_t x = 0;
    for (int32_t i = 0; i < count; i++, x += 70) {
        image_draw(image_group(IMAGE_GROUP_EMPIRE_BORDER) + 3, x, g_screen_height - 120);
        image_draw(image_group(IMAGE_GROUP_EMPIRE_BORDER) + 3, x, g_screen_height - 80);
        image_draw(image_group(IMAGE_GROUP_EMPIRE_BORDER) + 3, x, g_screen_height - 40);
    }
    image_draw(image_group(IMAGE_GROUP_EMPIRE_BORDER) + 3, width - 70, g_screen_height - 120);
    image_draw(image_group(IMAGE_GROUP_EMPIRE_BORDER) + 3, width - 70, g_screen_height - 80);
    image_draw(image_group(IMAGE_GROUP_EMPIRE_BORDER) + 3, width - 70, g_screen_height - 40);

    width = g_screen_width;
    count = width / 86;
    x = 0;
    for (int32_t i = 0; i < count; i++, x += 86) {
        image_draw(image_group(IMAGE_GROUP_EMPIRE_BORDER) + 1, x, 0);
        image_draw(image_group(IMAGE_GROUP_EMPIRE_BORDER) + 1, x, g_screen_height - 130);
        image_draw(image_group(IMAGE_GROUP_EMPIRE_BORDER) + 1, x, g_screen_height - 16);
    }
    image_draw(image_group(IMAGE_GROUP_EMPIRE_BORDER) + 1, width - 86, 0);
    image_draw(image_group(IMAGE_GROUP_EMPIRE_BORDER) + 1, width - 86, g_screen_height - 130);
    image_draw(image_group(IMAGE_GROUP_EMPIRE_BORDER) + 1, width - 86, g_screen_height - 16);

    int32_t height = g_screen_height;
    count = height / 86;
    int32_t y = 16;
    for (int32_t i = 0; i < count; i++, y += 86) {
        image_draw(image_group(IMAGE_GROUP_EMPIRE_BORDER), 0, y);
        image_draw(image_group(IMAGE_GROUP_EMPIRE_BORDER), g_screen_width - 16, y);
    }
    image_draw(image_group(IMAGE_GROUP_EMPIRE_BORDER), 0, height - 86);
    image_draw(image_group(IMAGE_GROUP_EMPIRE_BORDER), g_screen_width - 16, height - 86);

    image_draw(image_group(IMAGE_GROUP_EMPIRE_BORDER) + 2, 0, 0);
    image_draw(image_group(IMAGE_GROUP_EMPIRE_BORDER) + 2, 0, g_screen_height - 130);
    image_draw(image_group(IMAGE_GROUP_EMPIRE_BORDER) + 2, 0, g_screen_height - 16);
    image_draw(image_group(IMAGE_GROUP_EMPIRE_BORDER) + 2, g_screen_width - 16, 0);
    image_draw(image_group(IMAGE_GROUP_EMPIRE_BORDER) + 2, g_screen_width - 16, g_screen_height - 130);
    image_draw(image_group(IMAGE_GROUP_EMPIRE_BORDER) + 2, g_screen_width - 16, g_screen_height - 16);
}

void window_empire_draw_info_panel(void)
{
    if (empire_selected_object > 0) {
        const empire_object *o = &empire_map_objects[empire_selected_object - 1];
        if (o->type == EMPIRE_OBJECT_CITY) {
            window_empire_draw_city_trade();
        }
        return;
    }
    if (empire_hovered_event_marker >= 0) {
        window_empire_draw_event_tooltip();
        return;
    }
    lang_text_draw_centered_ui(47, 9, ui_origin_x, g_screen_height - 76, 640, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
}

void window_empire_draw_event_tooltip(void)
{
    if (window_current_id != WINDOW_EMPIRE) {
        return;
    }
    int32_t x = (g_screen_width - 240) / 2;
    const scenario_event *event = event_at(empire_markers.markers[empire_hovered_event_marker].event_id);
    if (event->type == EVENT_TYPE_REQUEST) {
        if (event->state == REQUEST_STATE_RECEIVED) {
            lang_text_draw_multiline_ui(47, 16, x, g_screen_height - 68, 240, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
        } else {
            lang_text_draw_multiline_ui(47, 17, x, g_screen_height - 68, 240, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
        }
        return;
    }
    lang_text_draw_multiline_ui(47, 15, x, g_screen_height - 68, 240, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
}

static void draw_trade_arrow(int32_t limit, int32_t x, int32_t y)
{
    if (limit > TRADE_ARROW_LARGE_LIMIT) {
        image_draw(image_group(IMAGE_GROUP_EMPIRE_TRADE_ARROWS) + 2, x, y);
    } else if (limit > TRADE_ARROW_MEDIUM_LIMIT) {
        image_draw(image_group(IMAGE_GROUP_EMPIRE_TRADE_ARROWS) + 1, x, y);
    } else if (limit > 0) {
        image_draw(image_group(IMAGE_GROUP_EMPIRE_TRADE_ARROWS), x, y);
    }
}

static int32_t trade_amount_scaled(int32_t resource)
{
    switch (resource) {
        case RESOURCE_STONE:
        case RESOURCE_LIMESTONE:
        case RESOURCE_GRANITE:
        case RESOURCE_SANDSTONE:
        case RESOURCE_MARBLE:
            return 0;
        default:
            return resource != RESOURCE_CHARIOTS && resource != RESOURCE_WEAPONS && resource != RESOURCE_DEBEN;
    }
}

void window_empire_draw_city_trade(void)
{
    int32_t object = empire_selected_object - 1;
    const empire_city *city = &empire_cities[empire_selected_city];
    const int32_t *route_limits = trade_route_limits[city->route_id];
    const int32_t *route_traded = trade_route_traded[city->route_id];
    uint8_t type = city->type;
    if (type == EMPIRE_CITY_OURS) {
        lang_text_draw_centered_ui(47, 1, (g_screen_width - 240) / 2, g_screen_height - 46, 240, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
        return;
    }
    if (type == EMPIRE_CITY_PHARAOH) {
        lang_text_draw_centered_ui(47, 19, (g_screen_width - 240) / 2, g_screen_height - 46, 240, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
        return;
    }
    if (type == EMPIRE_CITY_EGYPTIAN) {
        lang_text_draw_centered_ui(47, 13, (g_screen_width - 240) / 2, g_screen_height - 46, 240, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
        return;
    }
    if (!empire_city_type_is_trading(type)) {
        lang_text_draw_centered_ui(47, 0, (g_screen_width - 240) / 2, g_screen_height - 46, 240, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
        return;
    }
    int32_t x = (g_screen_width - 500) / 2;
    int32_t y = g_screen_height - 108;
    if (city->is_open == 0) {
        g_text_width = 0;
        lang_text_draw_ui(47, 5, x + 50, y + 33, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
        g_text_width = 0;
        for (int32_t r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
            if (empire_object_city_sells_resource(object, r) == 0 || empire_city_has_trade_limit(empire_selected_city, r) == 0) {
                continue;
            }
            int32_t limit = city_gods_ra_adjust_percentage(route_limits[r]);
            graphics_draw_inset_rect(g_text_width + x + 100, y + 28, 19, 17);
            image_draw(image_group(IMAGE_GROUP_RESOURCE_ICONS) + r, g_text_width + x + 101, y + 29);
            draw_trade_arrow(limit, g_text_width + x + 113, y + 29);
            g_text_width += 32;
        }
        lang_text_draw_ui(47, 4, x + 50, y + 53, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
        g_text_width = 0;
        for (int32_t r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
            if (empire_object_city_buys_resource(object, r) == 0 || empire_city_has_trade_limit(empire_selected_city, r) == 0) {
                continue;
            }
            int32_t limit = city_gods_ra_adjust_percentage(route_limits[r]);
            graphics_draw_inset_rect(g_text_width + x + 100, y + 50, 19, 17);
            image_draw(image_group(IMAGE_GROUP_RESOURCE_ICONS) + r, g_text_width + x + 101, y + 50);
            draw_trade_arrow(limit, g_text_width + x + 113, y + 50);
            g_text_width += 32;
        }
        button_border_draw_ui(x + 50, y + 68, 400, 20, focus_button_id_secondary != 0, ui_origin_x, ui_origin_y);
        lang_text_draw_amount(8, 0, city->cost_to_open, x + 60, y + 73, image_group(IMAGE_GROUP_FONT_BLACK), 0);
        uint32_t route = city->route_id;
        if (route < EMPIRE_ROUTES) {
            if (empire_map_routes.routes[route].kind == EMPIRE_ROUTE_KIND_LAND) {
                lang_text_draw_ui(47, 6, g_text_width + x + 60, y + 73, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
            } else {
                lang_text_draw_ui(47, 7, g_text_width + x + 60, y + 73, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
            }
        }
        return;
    }

    int32_t column = 0;
    int32_t count = 0;
    lang_text_draw_ui(47, 11, x + 30, y, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    int32_t dy = 15;
    int32_t row_y = y + 15;
    for (int32_t r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
        if (empire_object_city_sells_resource(object, r) == 0 || empire_city_has_trade_limit(empire_selected_city, r) == 0) {
            continue;
        }
        int32_t cx = column * 135;
        int32_t limit = city_gods_ra_adjust_percentage(route_limits[r]);
        graphics_draw_inset_rect(cx + x, dy + y + 1, 19, 17);
        image_draw(image_group(IMAGE_GROUP_RESOURCE_ICONS) + r, cx + x + 1, dy + y + 2);
        draw_trade_arrow(limit, cx + x + 13, row_y);
        int32_t traded = route_traded[r];
        if (traded > limit) {
            limit = traded;
        }
        if (trade_amount_scaled(r)) {
            traded *= 100;
            limit *= 100;
        }
        if (empire_resource_sold_and_bought(object, r) == 0) {
            g_text_width = 0;
            text_draw_number_ui(traded, NUMBER_PREFIX_NONE, empty_text, cx + 30 + x - 12, row_y + 8, image_group(IMAGE_GROUP_FONT_SMALL), 0, ui_origin_x, ui_origin_y);
            lang_text_draw_ui(47, 12, g_text_width + cx + 30 + x - 14, row_y + 8, image_group(IMAGE_GROUP_FONT_SMALL), 0, ui_origin_x, ui_origin_y);
            text_draw_number_ui(limit, NUMBER_PREFIX_NONE, empty_text, g_text_width + cx + 30 + x - 20, row_y + 8, image_group(IMAGE_GROUP_FONT_SMALL), 0, ui_origin_x, ui_origin_y);
        }
        column++;
        count++;
        if (count % 2 == 0) {
            column = 0;
            dy += 20;
            row_y += 20;
        }
    }
    graphics_draw_line(x + 240, y + 15, x + 240, y + 90, 0);
    column = 0;
    count = 0;
    lang_text_draw_ui(47, 10, x + 380, y, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    dy = 15;
    row_y = y + 15;
    for (int32_t r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
        if (empire_object_city_buys_resource(object, r) == 0 || empire_city_has_trade_limit(empire_selected_city, r) == 0) {
            continue;
        }
        int32_t cx = column * 135 + 280;
        int32_t limit = city_gods_ra_adjust_percentage(route_limits[r]);
        graphics_draw_inset_rect(cx + x, row_y, 19, 17);
        image_draw(image_group(IMAGE_GROUP_RESOURCE_ICONS) + r, cx + x + 1, dy + y + 1);
        draw_trade_arrow(limit, cx + x + 13, row_y);
        int32_t traded = route_traded[r];
        if (traded > limit) {
            limit = traded;
        }
        if (trade_amount_scaled(r)) {
            traded *= 100;
            limit *= 100;
        }
        g_text_width = 0;
        text_draw_number_ui(traded, NUMBER_PREFIX_NONE, empty_text, cx + 30 + x - 12, row_y + 8, image_group(IMAGE_GROUP_FONT_SMALL), 0, ui_origin_x, ui_origin_y);
        lang_text_draw_ui(47, 12, g_text_width + cx + 30 + x - 14, row_y + 8, image_group(IMAGE_GROUP_FONT_SMALL), 0, ui_origin_x, ui_origin_y);
        text_draw_number_ui(limit, NUMBER_PREFIX_NONE, empty_text, g_text_width + cx + 30 + x - 20, row_y + 8, image_group(IMAGE_GROUP_FONT_SMALL), 0, ui_origin_x, ui_origin_y);
        column++;
        count++;
        if (count % 2 == 0) {
            column = 0;
            dy += 20;
            row_y += 20;
        }
    }
}

int32_t empire_resource_sold_and_bought(int32_t object, int32_t resource)
{
    if (empire_object_city_sells_resource(object, resource) != 0 && empire_object_city_buys_resource(object, resource) != 0 &&
        empire_city_has_trade_limit(empire_selected_city, resource) != 0) {
        return 1;
    }
    return 0;
}

int32_t empire_animation_next_frame(int32_t image_id, int32_t frame)
{
    int32_t reverse = 0;
    if (frame <= 0) {
        frame = 1;
    }
    if (animation_should_advance[image_header_of(image_id)->animation_speed_id] == 0) {
        return frame;
    }
    if (image_header_of(image_id)->animation_can_reverse == 0) {
        frame++;
        if (frame > image_header_of(image_id)->animation_frames) {
            return 1;
        }
        return frame;
    }
    if (frame & ANIMATION_FRAME_REVERSE) {
        reverse = 1;
    }
    frame &= ANIMATION_FRAME_NUMBER;
    if (reverse == 0) {
        frame++;
        if (frame > image_header_of(image_id)->animation_frames) {
            return image_header_of(image_id)->animation_frames | ANIMATION_FRAME_REVERSE;
        }
        return frame;
    }
    frame--;
    if (frame < 1) {
        return 1;
    }
    return frame | ANIMATION_FRAME_REVERSE;
}

void window_empire_draw_city_name(void)
{
    if (editor_active == 1 || empire_selected_object <= 0) {
        return;
    }
    if (empire_map_objects[empire_selected_object - 1].type != EMPIRE_OBJECT_CITY) {
        return;
    }
    lang_text_draw_centered_ui(city_name_group(), empire_cities[empire_selected_city].name_id, (g_screen_width - 332) / 2, g_screen_height - 118, 332,
        image_group(IMAGE_GROUP_FONT_LARGE), 0, ui_origin_x, ui_origin_y);
}

static void route_path_point(const empire_route *route, int32_t percent, int32_t offset, int32_t *x, int32_t *y)
{
    int32_t length = route->path_length;
    if (length < 1) {
        *x = 0;
        *y = 0;
        return;
    }
    if (percent < 0) {
        percent = 0;
    } else if (percent > 100) {
        percent = 100;
    }
    int32_t index = ((length * percent) / 100) + offset;
    if (index >= length) {
        index = length - 1;
    }
    const empire_route_point *point = &route->path[index];
    *x = point->x - empire_scroll_x + empire_border_x;
    *y = point->y - empire_scroll_y + empire_border_y;
}

static void route_draw_points(const empire_route_point *points, int32_t count, int32_t image)
{
    for (int32_t j = 0; j < count; j++) {
        image_draw(image, points[j].x - empire_scroll_x + empire_border_x, points[j].y - empire_scroll_y + empire_border_y);
    }
}

static int32_t route_has_path(const empire_route *route)
{
    return route->path_length > 0 && route->path != 0;
}

static int32_t route_is_shown(empire_route *route)
{
    if (route->num_points <= 1) {
        route->in_use = 0;
    }
    return route->in_use != 0;
}

static void draw_marker(int32_t event_id, int32_t image, int32_t x, int32_t y)
{
    image_draw(image, x, y);
    const image_header *header = image_header_of(image);
    int32_t n = empire_marker_count;
    if (n < EMPIRE_EVENT_MARKERS) {
        empire_event_marker *marker = &empire_markers.markers[n];
        marker->event_id = event_id;
        marker->x = x;
        marker->y = y;
        marker->width = header->width;
        marker->height = header->height;
        empire_marker_count = n + 1;
    }
}

void window_empire_draw_map(void)
{
    ew_anim_counter++;
    if (ew_anim_counter > ANIMATION_COUNTER_MAX) {
        ew_anim_counter = 0;
    }
    graphics_dash_blink();
    image_view_width = display_width - (empire_border_x * 2);
    image_view_height = screen_height - empire_border_bottom - empire_border_y;
    if (empire_scroll_x >= empire_map_width - image_view_width) {
        empire_scroll_x = empire_map_width - image_view_width - 1;
    }
    if (empire_scroll_y >= empire_map_height - image_view_height) {
        empire_scroll_y = empire_map_height - image_view_height - 1;
    }
    if (!screen_layout_is_standard_size()) {
        empire_check_scroll_boundaries();
    }
    image_view_x = empire_scroll_x;
    image_view_y = empire_scroll_y;
    image_draw(image_group(IMAGE_GROUP_FULLSCREEN), empire_border_x, empire_border_y);
    g_clip_x1 = display_width - empire_border_x;
    g_clip_x0 = empire_border_x;
    g_clip_y0 = empire_border_y;
    g_clip_y1 = screen_height - empire_border_bottom;
    int32_t name_color;
    int32_t shadow_color;
    if (display_mode == 2) {
        name_color = 0x81C4;
        shadow_color = 0xAB0A;
    } else {
        name_color = 0x40E4;
        shadow_color = 0x558A;
    }

    for (int32_t i = 0; i < EMPIRE_OBJECT_COUNT; i++) {
        empire_object *o = &empire_map_objects[i];
        if (o->in_use == 0) {
            break;
        }
        uint8_t type = o->type;
        if (type > EMPIRE_OBJECT_BATTLE_ICON) {
            continue;
        }
        int32_t ox = o->x;
        int32_t oy = o->y;
        int32_t image = o->image_id;
        if (type == EMPIRE_OBJECT_TEXT) {
            lang_text_draw_centered_ui(196, o->name_id, empire_border_x - empire_scroll_x + ox, empire_border_y - empire_scroll_y + oy, 90, image_group(IMAGE_GROUP_FONT_SMALL),
                0, ui_origin_x, ui_origin_y);
            lang_text_draw_centered_ui(196, o->name_id, empire_border_x - empire_scroll_x + ox - 1, empire_border_y - empire_scroll_y + oy - 1, 90,
                image_group(IMAGE_GROUP_FONT_SMALL), shadow_color, ui_origin_x, ui_origin_y);
            continue;
        }
        if (type == EMPIRE_OBJECT_BATTLE_ICON) {
            if (window_current_id == WINDOW_EMPIRE) {
                continue;
            }
            text_draw_number_ui(o->invasion_path_id, NUMBER_PREFIX_NONE, postfix_none, empire_border_x - empire_scroll_x + ox - 8, empire_border_y - empire_scroll_y + oy - 10,
                image_group(IMAGE_GROUP_FONT_SMALL), 0, ui_origin_x, ui_origin_y);
            text_draw_number_ui(o->invasion_path_id, NUMBER_PREFIX_NONE, postfix_none, empire_border_x - empire_scroll_x + ox - 9, empire_border_y - empire_scroll_y + oy - 9,
                image_group(IMAGE_GROUP_FONT_SMALL), 0xFFFF, ui_origin_x, ui_origin_y);
            text_draw_number_ui(o->invasion_years, NUMBER_PREFIX_NONE, postfix_none, empire_border_x - empire_scroll_x + ox + 16,
                empire_border_y - empire_scroll_y + oy - 10, image_group(IMAGE_GROUP_FONT_SMALL), 0, ui_origin_x, ui_origin_y);
            text_draw_number_ui(o->invasion_years, NUMBER_PREFIX_NONE, postfix_none, empire_border_x - empire_scroll_x + ox + 15, empire_border_y - empire_scroll_y + oy - 9,
                image_group(IMAGE_GROUP_FONT_SMALL), 0xF800, ui_origin_x, ui_origin_y);
        }
        image_draw(image, empire_border_x - empire_scroll_x + ox, empire_border_y - empire_scroll_y + oy);
        if (o->type != EMPIRE_OBJECT_CITY || image == 0) {
            continue;
        }
        empire_city_get_for_object(i);
        const image_header *header = image_header_of(image);
        int32_t text_w = lang_text_get_width(city_name_group(), o->name_id, image_group(IMAGE_GROUP_FONT_SMALL));
        int32_t tx;
        int32_t ty;
        switch (o->text_align) {
            case 0:
                tx = empire_border_x - empire_scroll_x - text_w + ox;
                ty = header->height / 2 - empire_scroll_y + empire_border_y + oy;
                break;
            case 1:
                tx = (header->width - text_w) / 2 - empire_scroll_x + empire_border_x + ox;
                ty = empire_border_y - empire_scroll_y + oy - 10;
                break;
            case 2:
                tx = header->width - empire_scroll_x + empire_border_x + ox;
                ty = header->height / 2 - empire_scroll_y + empire_border_y + oy;
                break;
            default:
                tx = (header->width - text_w) / 2 - empire_scroll_x + empire_border_x + ox;
                ty = header->height - empire_scroll_y + empire_border_y + oy + 5;
                break;
        }
        lang_text_draw_ui(city_name_group(), o->name_id, tx, ty, image_group(IMAGE_GROUP_FONT_SMALL), name_color, ui_origin_x, ui_origin_y);
        if (empire_city_type_is_trading(o->city_type)) {
            int32_t fx = empire_border_x - empire_scroll_x + ox + (header->width / 2);
            int32_t fy = empire_border_y - empire_scroll_y + oy + header->height;
            int32_t base = image_group(IMAGE_GROUP_EMPIRE_CITY_FLAG);
            int32_t frame = empire_animation_next_frame(base, o->animation_frame);
            o->animation_frame = (uint8_t) frame;
            int32_t flag = base + (frame & 0xFF);
            image_draw(flag, fx, fy - image_header_of(flag)->height);
        }
    }

    int32_t editor_routes = 0;
    if (window_current_id == WINDOW_EDITOR_EMPIRE) {
        editor_routes = 1;
        int32_t selected = g_empire_selected_object;
        const empire_object *so = &empire_map_objects[selected];
        int32_t sx;
        int32_t sy;
        if (scenario_data.empire_expanded != 0) {
            sx = so->expanded_x;
            sy = so->expanded_y;
        } else {
            sx = so->x;
            sy = so->y;
        }
        if (so->in_use != 0) {
            int32_t w;
            int32_t h;
            if (so->type == EMPIRE_OBJECT_TEXT) {
                w = TEXT_OBJECT_WIDTH;
                h = TEXT_OBJECT_HEIGHT;
            } else {
                const image_header *header = image_header_of(so->image_id);
                h = header->height + (SELECTION_FRAME_MARGIN * 2);
                w = header->width + (SELECTION_FRAME_MARGIN * 2);
            }
            graphics_draw_dashed_rect(empire_border_x - empire_scroll_x + sx - SELECTION_FRAME_MARGIN, empire_border_y - empire_scroll_y + sy - SELECTION_FRAME_MARGIN,
                w, h);
            if (window_current_id != WINDOW_EDITOR_EMPIRE) {
                editor_routes = 0;
            }
        }
    }
    if (editor_routes) {
        for (int32_t i = 0; i < EMPIRE_ROUTES; i++) {
            empire_route *route = &empire_map_routes.routes[i];
            if (!route_is_shown(route)) {
                continue;
            }
            int32_t editing = empire_ui_mode == EMPIRE_EDITOR_MODE_ADD_ROUTE || empire_ui_mode == EMPIRE_EDITOR_MODE_EDIT_ROUTE;
            if (i == route_current && editing) {
                if (route_has_path(route)) {
                    route_draw_points(route->path, route->path_length, image_group(IMAGE_GROUP_COLUMN) + 1);
                    route_draw_points(route->points, route->num_points, image_group(IMAGE_GROUP_COLUMN) + 2);
                }
                ew_route_anim_counter++;
                if (ew_route_anim_counter > ANIMATION_COUNTER_MAX) {
                    ew_route_anim_counter = 0;
                }
                int32_t mx;
                int32_t my;
                route_path_point(route, ew_route_anim_counter, 0, &mx, &my);
                image_draw(route->kind == EMPIRE_ROUTE_KIND_SEA ? image_group(IMAGE_GROUP_EMPIRE_MARKER_SEA) : image_group(IMAGE_GROUP_EMPIRE_MARKER_LAND), mx, my);
            } else if (route_has_path(route)) {
                route_draw_points(route->path, route->path_length, image_group(IMAGE_GROUP_COLUMN) + 1);
            }
        }
    } else {
        for (int32_t i = 0; i < EMPIRE_ROUTES; i++) {
            empire_route *route = &empire_map_routes.routes[i];
            if (route_is_shown(route) && empire_city_is_trade_route_open(i) != 0 && route_has_path(route)) {
                route_draw_points(route->path, route->path_length, image_group(IMAGE_GROUP_COLUMN) + 1);
            }
        }
    }

    empire_marker_count = 0;
    for (scenario_event *event = scenario_invasion_next(&scenario_events, 0, 0); event != 0; event = scenario_invasion_next(&scenario_events, event, 0)) {
        if (scenario_event_is_running(event) == 0) {
            continue;
        }
        int32_t months = scenario_event_months_left(event);
        int32_t route_id = event->invasion.value;
        if (route_id < 0 || route_id >= EMPIRE_ROUTES) {
            continue;
        }
        const empire_route *route = &empire_map_routes.routes[route_id];
        int32_t total = empire_route_travel_months(route);
        if (months > total || total <= 0) {
            continue;
        }
        int32_t percent = 100 - ((months * 100) / total);
        if (percent > 100) {
            percent = 0;
        }
        int32_t offset = (ew_anim_counter - 50) / 4;
        if (offset < 0) {
            offset = 0;
        }
        int32_t mx;
        int32_t my;
        route_path_point(route, percent, offset, &mx, &my);
        draw_marker(event->id, route->kind == EMPIRE_ROUTE_KIND_SEA ? image_group(IMAGE_GROUP_EMPIRE_MARKER_SEA) : image_group(IMAGE_GROUP_EMPIRE_MARKER_LAND), mx, my);
    }
    for (scenario_event *event = scenario_request_next_active(&scenario_events, 0); event != 0; event = scenario_request_next_active(&scenario_events, event)) {
        if (event->item.value != RESOURCE_TROOPS) {
            continue;
        }
        int32_t route_id;
        int32_t percent;
        uint8_t by_land;
        int32_t zero;
        if (troops_request_state(event, &route_id, &percent, &by_land, &zero) == 0) {
            continue;
        }
        if (route_id < 0 || route_id >= EMPIRE_ROUTES) {
            continue;
        }
        int32_t mx;
        int32_t my;
        route_path_point(&empire_map_routes.routes[route_id], percent, 0, &mx, &my);
        draw_marker(event->id, by_land != 0 ? image_group(IMAGE_GROUP_EMPIRE_MARKER_LAND) : image_group(IMAGE_GROUP_EMPIRE_MARKER_SEA), mx, my);
    }
    g_clip_x0 = 0;
    g_clip_x1 = g_screen_width;
    g_clip_y0 = 0;
    g_clip_y1 = g_screen_height;
}

void empire_trade_route_open_answer(void)
{
    if (popup_answer != 1) {
        return;
    }
    empire_city *city = &empire_cities[empire_selected_city];
    city_finance_process_construction(city_current_player, city->cost_to_open);
    city->is_open = 1;
    building_menu_update();
    sidebar_build_buttons_update();
    popup.return_window = WINDOW_CITY_INTRO;
    window_redraw_request = 1;
}

void empire_button_open_trade_route(void)
{
    const empire_city *city = &empire_cities[empire_selected_city];
    if (city->is_open >= ROUTE_OPEN_LIMIT) {
        return;
    }
    if (city_player_current()->treasury <= TREASURY_CREDIT_LIMIT) {
        window_popup_dialog_show(POPUP_NO_CREDIT, button_none, 0);
        return;
    }
    popup_callback answer = empire_trade_route_open_answer;
    if (empire_map_routes.routes[city->route_id].kind == EMPIRE_ROUTE_KIND_LAND) {
        window_popup_dialog_show(POPUP_OPEN_LAND_ROUTE, answer, 2);
    } else {
        window_popup_dialog_show(POPUP_OPEN_SEA_ROUTE, answer, 2);
    }
}

void window_go_empire(void)
{
    window_current_id = WINDOW_EMPIRE;
    window_redraw_request = 1;
}

void empire_map_object_at_mouse(int32_t *object, int32_t *moving)
{
    int32_t x = mouse_x - empire_border_x + empire_scroll_x;
    int32_t y = mouse_y - empire_border_y + empire_scroll_y;
    int32_t best = MAP_OBJECT_PICK_DISTANCE;
    *object = 0;
    for (int32_t index = 0; index < EMPIRE_OBJECT_COUNT; index++) {
        const empire_object *o = &empire_map_objects[index];
        if (o->in_use == 0) {
            break;
        }
        int32_t ox;
        int32_t oy;
        if (scenario_data.empire_expanded != 0) {
            ox = o->expanded_x;
            oy = o->expanded_y;
        } else {
            ox = o->x;
            oy = o->y;
        }
        int32_t w = o->width;
        int32_t h = o->height;
        if (ox - MAP_OBJECT_PICK_MARGIN > x || w + ox + MAP_OBJECT_PICK_MARGIN <= x || oy - MAP_OBJECT_PICK_MARGIN > y || h + oy + MAP_OBJECT_PICK_MARGIN <= y) {
            continue;
        }
        int32_t d = calc_maximum_distance(x, y, (w / 2) + ox, (h / 2) + oy);
        if (d < best) {
            best = d;
            *object = index + 1;
        }
    }
    *moving = -1;
    for (int32_t i = 0; i < empire_marker_count; i++) {
        const empire_event_marker *marker = &empire_markers.markers[i];
        int32_t d = calc_maximum_distance(mouse_x, mouse_y, (marker->width / 2) + marker->x, (marker->height / 2) + marker->y);
        if (d < best) {
            best = d;
            *object = 0;
            *moving = i;
        }
    }
}

void button_empire_help(void)
{
    window_message_dialog_show(MESSAGE_DIALOG_EMPIRE_HELP, 1, 0);
}

void button_empire_exit(void)
{
    if (editor_active == 1) {
        int32_t climate = scenario_climate_current;
        int16_t x = (int16_t) empire_scroll_x;
        int16_t y = (int16_t) empire_scroll_y;
        scenario_data.climate = (int16_t) climate;
        empire_header[climate].scroll_x = x;
        empire_header[climate].scroll_y = y;
        empire_check_scroll_boundaries();
    }
    window_current_id = WINDOW_CITY;
    window_redraw_request = 1;
}

void button_empire_open(void)
{
    empire_hovered_event_marker = -1;
    empire_marker_count = 0;
    if (editor_active == 1) {
        if (settings_data.display_mode == 1) {
            const char *text = lang_get_string(5, 123);
            const char *caption = lang_get_string(5, 124);
            window_message_box_show(text, caption, 0);
            window_redraw_request = 1;
            return;
        }
        window_current_id = WINDOW_EDITOR_EMPIRE;
        empire_ui_mode = EMPIRE_EDITOR_MODE_GENERAL;
        empire_editor_image_kind = EMPIRE_OBJECT_CITY;
        empire_check_scroll_boundaries();
        window_redraw_request = 1;
        return;
    }
    int32_t custom = settings_data.is_custom_scenario;
    if (custom == 0 && scenario_start_flags[SCENARIO_START_FLAG_EMPIRE_LOCKED] != 0) {
        city_warning_show(13);
        return;
    }
    window_current_id = WINDOW_EMPIRE;
    if (custom == 0 && tutorial_flags_a[TUTORIAL_FLAG_EMPIRE_OPENED] == 0 && settings_data.mission_id == tutorial_briefing_tables.tutorial.entries[TUTORIAL_FLAG_EMPIRE_OPENED].mission) {
        window_current_id = WINDOW_CITY;
        tutorial_set_flag(TUTORIAL_FLAG_EMPIRE_OPENED);
        dialog_prev_window = 20;
    }
    window_redraw_request = 1;
}
