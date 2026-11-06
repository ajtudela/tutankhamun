// Drawing the empire editor's window: the bottom panel with its mode buttons, the hint line, the route panel and the
// info panel of the current mode (the image picker in add mode, the selected object in edit mode).
//
// The image preview is clipped with the screen height as its right edge.

#include "editor/empire_editor_draw.h"
#include "editor/editor_data.h"
#include "editor/editor_internal.h"
#include "editor/editor_text.h"
#include "editor/empire_editor_objects.h"
#include "empire/empire_data.h"
#include "empire/empire_route.h"
#include "empire/empire_types.h"
#include "graphics/color.h"
#include "graphics/graphics_data.h"
#include "graphics/graphics_primitives.h"
#include "graphics/image_collection.h"
#include "graphics/image_draw.h"
#include "graphics/text_draw.h"
#include "graphics/ui_layout.h"
#include "map/grid.h"
#include "text/lang.h"
#include "window/empire_window.h"
#include "window/window_image_group.h"
#include <stdint.h>

enum empire_editor_panel_layout {
    PANEL_HEIGHT = 116,
    PANEL_BACKGROUND = 0xF73B,
    PICKER_BACKGROUND = 0xC618,
    BUTTON_COLUMN_1_X = 15,
    BUTTON_COLUMN_2_X = 119,
    BUTTON_WIDTH = 100,
    BUTTON_HEIGHT = 18,
    BUTTON_ROW_HEIGHT = 20,
    BUTTON_FIRST_ROW_Y = 30,
    BUTTON_TEXT_Y = 2,
    FIELD_X = 393,
    FIELD_TEXT_X = 413,
    FIELD_WIDTH = 180,
    FIELD_HEIGHT = 20,
    FIELD_TEXT_Y = 4,
    SECOND_FIELD_X = 593,
    PREVIEW_X = 293,
    PREVIEW_SIZE = 90,
    PREVIEW_CLIP_TOP = 382,
    IMAGE_NUMBER_X = 225,
    IMAGE_NUMBER_WIDTH = 48,
    RESOURCE_ICON_SIZE = 24,
};

static int32_t highlight(int32_t on)
{
    return on ? COLOR_RED : COLOR_BLACK;
}

static int32_t city_name_group(void)
{
    return settings_data.classical_names ? LANG_GROUP_CITY_NAMES_CLASSICAL : LANG_GROUP_CITY_NAMES;
}

static void draw_mode_button(int32_t text, int32_t x, int32_t row, int32_t color)
{
    int32_t y = g_screen_height - PANEL_HEIGHT + BUTTON_FIRST_ROW_Y + (row * BUTTON_ROW_HEIGHT) + BUTTON_TEXT_Y;
    lang_text_draw_centered_ui(LANG_GROUP_EDITOR, text, x, y, BUTTON_WIDTH, image_group(IMAGE_GROUP_FONT_BLACK), color, ui_origin_x, ui_origin_y);
}

void empire_editor_draw_mode_buttons(void)
{
    int32_t mode = empire_ui_mode;
    draw_mode_button(EDITOR_TEXT_ADD_OBJECT, BUTTON_COLUMN_1_X, 0, highlight(mode == EMPIRE_EDITOR_MODE_ADD_OBJECT));
    draw_mode_button(EDITOR_TEXT_EDIT_OBJECTS, BUTTON_COLUMN_1_X, 1, highlight(mode == EMPIRE_EDITOR_MODE_EDIT_OBJECTS));
    draw_mode_button(EDITOR_TEXT_DELETE_OBJECT, BUTTON_COLUMN_1_X, 2, highlight(mode == EMPIRE_EDITOR_MODE_DELETE_OBJECT));
    draw_mode_button(EDITOR_TEXT_GENERAL, BUTTON_COLUMN_1_X, 3, highlight(mode == EMPIRE_EDITOR_MODE_GENERAL));
    draw_mode_button(EDITOR_TEXT_ADD_ROUTE, BUTTON_COLUMN_2_X, 0, highlight(mode == EMPIRE_EDITOR_MODE_ADD_ROUTE));
    draw_mode_button(EDITOR_TEXT_EDIT_ROUTE, BUTTON_COLUMN_2_X, 1, highlight(mode == EMPIRE_EDITOR_MODE_EDIT_ROUTE));
    draw_mode_button(EDITOR_TEXT_RESET, BUTTON_COLUMN_2_X, 2, COLOR_BLACK);
    draw_mode_button(EDITOR_TEXT_OK, BUTTON_COLUMN_2_X, 3, COLOR_BLACK);
}

void empire_editor_draw_background(void)
{
    window_empire_draw_border();
    graphics_fill_rect(10, g_screen_height - PANEL_HEIGHT, g_screen_width - 20, PANEL_HEIGHT - 1, PANEL_BACKGROUND);
    graphics_draw_raised_rect(10, g_screen_height - PANEL_HEIGHT, g_screen_width - 20, PANEL_HEIGHT - 1);
    int32_t y = g_screen_height - PANEL_HEIGHT;
    lang_text_draw_ui(LANG_GROUP_EDITOR, EDITOR_TEXT_KINGDOM_BUILDER, BUTTON_COLUMN_1_X, y, image_group(IMAGE_GROUP_FONT_LARGE), 0, ui_origin_x, ui_origin_y);
    for (int32_t row = 0; row < 4; row++) {
        graphics_draw_raised_rect(BUTTON_COLUMN_1_X, y + BUTTON_FIRST_ROW_Y + (row * BUTTON_ROW_HEIGHT), BUTTON_WIDTH, BUTTON_HEIGHT);
    }
    for (int32_t row = 0; row < 4; row++) {
        graphics_draw_raised_rect(BUTTON_COLUMN_2_X, y + BUTTON_FIRST_ROW_Y + (row * BUTTON_ROW_HEIGHT), BUTTON_WIDTH, BUTTON_HEIGHT);
    }
    empire_editor_draw_mode_buttons();
    graphics_fill_rect(223, g_screen_height - 114, g_screen_width - 238, 109, PANEL_BACKGROUND);
    graphics_draw_raised_rect(223, g_screen_height - 114, g_screen_width - 238, 109);
    empire_editor_draw_info();
}

void empire_editor_draw_hint(void)
{
    g_text_width = 0;
    graphics_draw_raised_rect(231, g_screen_height - 34, 150, FIELD_HEIGHT);
    lang_text_draw_centered_ui(LANG_GROUP_EDITOR, EDITOR_TEXT_SET_KINGDOM_PRICES, 231, g_screen_height - 30, 120, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
}

static int32_t route_kind_text(uint8_t kind)
{
    if (kind == EMPIRE_ROUTE_KIND_LAND) {
        return EDITOR_TEXT_LAND_TRADE_ROUTE;
    }
    return (kind == EMPIRE_ROUTE_KIND_SEA) ? EDITOR_TEXT_SEA_TRADE_ROUTE : EDITOR_TEXT_GENERAL_ROUTE;
}

static void draw_route_end(int32_t object, int32_t y)
{
    if (object != -1) {
        lang_text_draw_centered_ui(
            city_name_group(), empire_map_objects[object].name_id, SECOND_FIELD_X, y, FIELD_WIDTH, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    }
}

void empire_editor_draw_routes(void)
{
    int32_t y = g_screen_height - 104;
    graphics_draw_raised_rect(FIELD_X, y, FIELD_WIDTH, FIELD_HEIGHT);
    int32_t color = highlight(focus_button_id == 1);
    g_text_width = 0;
    lang_text_draw_multiline_ui(
        LANG_GROUP_EDITOR, EDITOR_TEXT_ROUTE, FIELD_TEXT_X, y + FIELD_TEXT_Y, BUTTON_WIDTH, image_group(IMAGE_GROUP_FONT_BLACK), color, ui_origin_x, ui_origin_y);
    text_draw_number_ui(
        route_current, NUMBER_PREFIX_NONE, empty_text, g_text_width + FIELD_TEXT_X, y + FIELD_TEXT_Y, image_group(IMAGE_GROUP_FONT_BLACK), color, ui_origin_x, ui_origin_y);
    graphics_draw_raised_rect(FIELD_X, y + 25, FIELD_WIDTH, FIELD_HEIGHT);
    const empire_route *route = &empire_map_routes.routes[route_current];
    lang_text_draw_multiline_ui(LANG_GROUP_EDITOR, route_kind_text(route->kind), FIELD_TEXT_X, y + 29, FIELD_WIDTH, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    graphics_draw_raised_rect(FIELD_X, y + 75, FIELD_WIDTH, FIELD_HEIGHT);
    g_text_width = 0;
    lang_text_draw_multiline_ui(LANG_GROUP_EDITOR, EDITOR_TEXT_DELETE_ROUTE, FIELD_TEXT_X, y + 79, FIELD_WIDTH, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    draw_route_end(route->first_object, g_screen_height - 108);
    draw_route_end(route->last_object, g_screen_height - 88);
    graphics_fill_rect(SECOND_FIELD_X, g_screen_height - 68, FIELD_WIDTH, FIELD_HEIGHT, PANEL_BACKGROUND);
    g_text_width = 0;
    lang_text_draw_multiline_ui(
        LANG_GROUP_EDITOR, EDITOR_TEXT_ROUTE_LENGTH, SECOND_FIELD_X, g_screen_height - 68, FIELD_WIDTH, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    int32_t length = route->path_length * route->step;
    text_draw_number_ui(length, NUMBER_PREFIX_NONE, empty_text, g_text_width + SECOND_FIELD_X, g_screen_height - 68, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
}

static image_header *image_header_of(int32_t image)
{
    return &image_collection.headers[image < image_collection.image_count ? image : 0];
}

static void draw_preview(int32_t image, int32_t advance)
{
    graphics_fill_rect(PREVIEW_X, g_screen_height - 104, PREVIEW_SIZE, PREVIEW_SIZE, COLOR_WHITE);
    image_draw(image, PREVIEW_X + 1, g_screen_height - 103);
    if (image_header_of(image)->animation_speed_id == 0) {
        return;
    }
    int32_t ox = image_header_of(image)->sprite_offset_x;
    int32_t oy = image_header_of(image)->sprite_offset_y;
    if (advance) {
        uint8_t frame = (uint8_t) empire_animation_next_frame(image, image_header_of(image)->animation_frame);
        image_header_of(image)->animation_frame = frame;
    }
    image_draw(image + image_header_of(image)->animation_frame, ox + PREVIEW_X + 1, g_screen_height + oy - 103);
}

static void set_preview_clip(void)
{
    g_clip_x0 = PREVIEW_X + 1;
    g_clip_y0 = PREVIEW_CLIP_TOP;
    g_clip_x1 = g_screen_height - 103;
    g_clip_y1 = g_screen_height - 15;
}

static void reset_clip(void)
{
    g_clip_y1 = g_screen_height;
    g_clip_x1 = g_screen_width;
    g_clip_x0 = 0;
    g_clip_y0 = 0;
}

static void draw_image_kind(int32_t text, int32_t y, int32_t selected)
{
    graphics_draw_raised_rect(FIELD_X, y, FIELD_WIDTH, FIELD_HEIGHT);
    lang_text_draw_centered_ui(LANG_GROUP_EDITOR, text, FIELD_X, y + FIELD_TEXT_Y, FIELD_WIDTH, image_group(IMAGE_GROUP_FONT_BLACK), highlight(selected), ui_origin_x, ui_origin_y);
}

static void draw_mode_add_object(void)
{
    set_preview_clip();
    if (empire_editor_image_kind == EMPIRE_EDITOR_IMAGE_SIMPLE_GRAPHIC) {
        draw_preview(empire_editor_image, 0);
    }
    reset_clip();
    graphics_fill_rect(387, g_screen_height - 110, g_screen_width - 408, 98, PICKER_BACKGROUND);
    text_draw_number_centered(
        empire_editor_image + 1, NUMBER_PREFIX_NONE, postfix_none, IMAGE_NUMBER_X, g_screen_height - 28, IMAGE_NUMBER_WIDTH, image_group(IMAGE_GROUP_FONT_BLACK), 0);
    draw_image_kind(EDITOR_TEXT_IMAGE_SIMPLE_GRAPHIC, g_screen_height - 106, empire_editor_image_kind == EMPIRE_EDITOR_IMAGE_SIMPLE_GRAPHIC);
    draw_image_kind(EDITOR_TEXT_IMAGE_CITY, g_screen_height - 81, empire_editor_image_kind == EMPIRE_EDITOR_IMAGE_CITY);
    draw_image_kind(EDITOR_TEXT_IMAGE_REGION, g_screen_height - 56, empire_editor_image_kind == EMPIRE_EDITOR_IMAGE_REGION);
}

static void draw_resource(int32_t resource, int32_t x, int32_t y)
{
    graphics_draw_inset_rect(x - 1, y - 1, RESOURCE_ICON_SIZE + 2, RESOURCE_ICON_SIZE + 2);
    int32_t base = image_group(IMAGE_GROUP_RESOURCE_ICONS);
    image_draw(base + resource, x, y);
}

static void draw_city_goods(const empire_object *city)
{
    if (city->city_type == EMPIRE_CITY_OURS) {
        for (int32_t i = 0, x = 399; x < 588; i++, x += 27) {
            draw_resource(city->sells[i], x, g_screen_height - 61);
            draw_resource(city->sells[7 + i], x, g_screen_height - 33);
        }
        return;
    }
    lang_text_draw_ui(LANG_GROUP_EDITOR, EDITOR_TEXT_CITY_SELLS, FIELD_X, g_screen_height - 55, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    for (int32_t i = 0, x = 524; x < 764; i++, x += 30) {
        draw_resource(city->sells[i], x, g_screen_height - 61);
    }
    lang_text_draw_ui(LANG_GROUP_EDITOR, EDITOR_TEXT_CITY_BUYS, FIELD_X, g_screen_height - 27, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    for (int32_t i = 0, x = 524; x < 764; i++, x += 30) {
        draw_resource(city->buys[i], x, g_screen_height - 33);
    }
}

static void draw_city(const empire_object *city)
{
    graphics_draw_raised_rect(FIELD_X, g_screen_height - 112, FIELD_WIDTH, FIELD_HEIGHT);
    lang_text_draw_centered_ui(
        LANG_GROUP_EMPIRE_CITY_TYPES, city->city_type, FIELD_X, g_screen_height - 108, FIELD_WIDTH, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    graphics_draw_raised_rect(SECOND_FIELD_X, g_screen_height - 112, FIELD_WIDTH, FIELD_HEIGHT);
    lang_text_draw_centered_ui(city_name_group(), city->name_id, SECOND_FIELD_X, g_screen_height - 108, FIELD_WIDTH, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    graphics_draw_raised_rect(PREVIEW_X, g_screen_height - 112, BUTTON_WIDTH, FIELD_HEIGHT);
    lang_text_draw_centered_ui(
        LANG_GROUP_TEXT_ALIGNMENTS, city->text_align, PREVIEW_X, g_screen_height - 108, BUTTON_WIDTH, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_ui(LANG_GROUP_EDITOR, EDITOR_TEXT_ROUTE, FIELD_X, g_screen_height - 83, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    graphics_draw_raised_rect(453, g_screen_height - 87, 50, FIELD_HEIGHT);
    text_draw_number_centered(city->route_id, NUMBER_PREFIX_NONE, postfix_none, 453, g_screen_height - 83, 50, image_group(IMAGE_GROUP_FONT_BLACK), 0);
    if (city->city_type != EMPIRE_CITY_OURS) {
        graphics_draw_raised_rect(523, g_screen_height - 87, BUTTON_WIDTH, FIELD_HEIGHT);
        lang_text_draw_centered_ui(LANG_GROUP_EDITOR, EDITOR_TEXT_DEMAND, 523, g_screen_height - 83, BUTTON_WIDTH, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    }
    lang_text_draw_ui(LANG_GROUP_EDITOR, EDITOR_TEXT_COST, 653, g_screen_height - 83, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    graphics_draw_raised_rect(703, g_screen_height - 87, 80, FIELD_HEIGHT);
    text_draw_number_centered(city->cost_to_open, NUMBER_PREFIX_NONE, postfix_none, 703, g_screen_height - 83, 60, image_group(IMAGE_GROUP_FONT_BLACK), 0);
    draw_city_goods(city);
}

static void draw_battle_marker(const empire_object *marker)
{
    lang_text_draw_ui(LANG_GROUP_EDITOR, EDITOR_TEXT_BATTLE_MARKER, FIELD_X, g_screen_height - 100, image_group(IMAGE_GROUP_FONT_LARGE), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_ui(LANG_GROUP_EDITOR, EDITOR_TEXT_PATH, 423, g_screen_height - 64, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    graphics_draw_raised_rect(523, g_screen_height - 68, 50, FIELD_HEIGHT);
    text_draw_number_centered(marker->invasion_path_id, NUMBER_PREFIX_NONE, postfix_none, 523, g_screen_height - 64, 50, image_group(IMAGE_GROUP_FONT_BLACK), 0);
    lang_text_draw_ui(LANG_GROUP_EDITOR, EDITOR_TEXT_ORDER, 423, g_screen_height - 34, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    graphics_draw_raised_rect(523, g_screen_height - 38, 50, FIELD_HEIGHT);
    text_draw_number_centered(marker->invasion_years, NUMBER_PREFIX_NONE, postfix_none, 523, g_screen_height - 34, 50, image_group(IMAGE_GROUP_FONT_BLACK), 0);
}

static void draw_mode_edit_objects(void)
{
    int32_t image = empire_editor_image;
    set_preview_clip();
    graphics_fill_rect(IMAGE_NUMBER_X, g_screen_height - 54, IMAGE_NUMBER_WIDTH, FIELD_HEIGHT, PANEL_BACKGROUND);
    text_draw_number_centered(
        empire_editor_image + 1, NUMBER_PREFIX_NONE, postfix_none, IMAGE_NUMBER_X, g_screen_height - 52, IMAGE_NUMBER_WIDTH, image_group(IMAGE_GROUP_FONT_BLACK), 0);
    if (empire_map_objects[g_empire_selected_object].type == EMPIRE_OBJECT_ORNAMENT) {
        draw_preview(image, 1);
    }
    reset_clip();
    graphics_fill_rect(387, g_screen_height - 110, g_screen_width - 408, 98, PANEL_BACKGROUND);
    text_draw_number_centered(
        empire_editor_image + 1, NUMBER_PREFIX_NONE, postfix_none, IMAGE_NUMBER_X, g_screen_height - 28, IMAGE_NUMBER_WIDTH, image_group(IMAGE_GROUP_FONT_BLACK), 0);
    g_text_width = 0;
    const empire_object *object = &empire_map_objects[g_empire_selected_object];
    if (object->type == EMPIRE_OBJECT_CITY) {
        draw_city(object);
    } else if (object->type == EMPIRE_OBJECT_BATTLE_ICON) {
        draw_battle_marker(object);
    } else if (object->type == EMPIRE_OBJECT_TEXT) {
        graphics_draw_raised_rect(FIELD_X, g_screen_height - 84, FIELD_WIDTH, FIELD_HEIGHT);
        lang_text_draw_centered_ui(
            LANG_GROUP_EMPIRE_REGION_NAMES, object->name_id, FIELD_X, g_screen_height - 84, FIELD_WIDTH, image_group(IMAGE_GROUP_FONT_SMALL), 0, ui_origin_x, ui_origin_y);
    }
}

void empire_editor_draw_info(void)
{
    switch (empire_ui_mode) {
        case EMPIRE_EDITOR_MODE_ADD_OBJECT:
            draw_mode_add_object();
            return;
        case EMPIRE_EDITOR_MODE_EDIT_OBJECTS:
            draw_mode_edit_objects();
            return;
        case EMPIRE_EDITOR_MODE_GENERAL:
        case EMPIRE_EDITOR_MODE_UNSET_9:
        case EMPIRE_EDITOR_MODE_UNSET_10:
        case EMPIRE_EDITOR_MODE_UNSET_11:
        case EMPIRE_EDITOR_MODE_UNSET_12:
        case EMPIRE_EDITOR_MODE_UNSET_13:
            empire_editor_draw_hint();
            return;
        case EMPIRE_EDITOR_MODE_ADD_ROUTE:
        case EMPIRE_EDITOR_MODE_EDIT_ROUTE:
            empire_editor_draw_routes();
            return;
        default:
            return;
    }
}
