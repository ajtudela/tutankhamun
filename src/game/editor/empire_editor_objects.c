// The empire editor's objects and mode buttons: counting and deleting objects, the checks after a city was edited
// (sorted trade lists, monuments and allowed buildings the goods no longer support are cleared with a warning), the
// objects' state at the start of an edited scenario, previous / next object, the empire file and the trade prices
// window.
//
// Counting and checking follow these rules:
// - an object counts as in use only when in_use is above 1;
// - a warning stops the later checks, except the second checks of the zoo and the forts.

#include "editor/empire_editor_objects.h"
#include "building/building_types.h"
#include "city/resource_type.h"
#include "core/calc.h"
#include "editor/editor_allowed_buildings.h"
#include "editor/editor_data.h"
#include "editor/editor_internal.h"
#include "editor/editor_menu.h"
#include "editor/editor_monuments.h"
#include "editor/editor_warning.h"
#include "editor/empire_editor_routes.h"
#include "empire/empire.h"
#include "empire/empire_helpers.h"
#include "empire/empire_types.h"
#include "empire/trade_status.h"
#include "graphics/image_collection.h"
#include "io/empire_file.h"
#include "io/io_data.h"
#include "window/empire_editor_input.h"
#include "window/popup_dialog.h"
#include "window/window_id.h"
#include "window/window_image_group.h"
#include <stdint.h>
#include <string.h>

enum {
    EMPIRE_EDITOR_MONUMENTS = 3,
    CITY_ROUTE_ID_MAX = EMPIRE_TRADE_ROUTES - 1,
};

enum object_step {
    OBJECT_STEP_NEXT = 0,
    OBJECT_STEP_PREVIOUS = 1,
};

void empire_editor_count_objects(void)
{
    int32_t count = 0;
    empire_editor_object_count = 0;
    while (count < EMPIRE_OBJECT_COUNT && empire_map_objects[count].in_use > 1) {
        count++;
    }
    empire_editor_object_count = count;
    if (count >= EMPIRE_OBJECT_COUNT) {
        empire_editor_object_count = EMPIRE_OBJECT_COUNT - 1;
    }
}

void empire_editor_delete_object(int32_t index)
{
    empire_object *object = &empire_map_objects[index];
    if (object->in_use == 0) {
        return;
    }
    memset(object, 0, sizeof(empire_object));
    if (index >= EMPIRE_OBJECT_COUNT - 1) {
        return;
    }
    for (int32_t i = index; i < EMPIRE_OBJECT_COUNT - 1; i++) {
        empire_object *next = &empire_map_objects[i + 1];
        if (next->in_use == 0) {
            return;
        }
        next[-1] = *next;
        memset(next, 0, sizeof(empire_object));
    }
}

static void sort_resources(uint8_t *list, int32_t count)
{
    for (int32_t pass = 0; pass < count; pass++) {
        for (int32_t i = count - 1; i > 0; i--) {
            uint8_t b = list[i];
            if (b == 0) {
                continue;
            }
            uint8_t a = list[i - 1];
            if (a == b) {
                list[i] = 0;
            } else if (a == 0 || a > b) {
                list[i - 1] = b;
                list[i] = a;
            }
        }
    }
}

static void warn(int32_t warning)
{
    window_current_id = WINDOW_EDITOR_RESOURCE_WARNING;
    editor_warning_draw(warning);
}

static int32_t no_warning_shown(void)
{
    return window_current_id != WINDOW_EDITOR_RESOURCE_WARNING;
}

static void remove_allowed(uint16_t *allowed, int32_t warning)
{
    *allowed = 0;
    warn(warning);
    window_redraw_request = 1;
}

static void check_allowed_needs(enum allowed_building building, int32_t resource, int32_t warning)
{
    uint16_t *allowed = &scenario_data.allowed_buildings[building];
    if (*allowed != 0 && no_warning_shown() && !empire_resource_available(resource)) {
        remove_allowed(allowed, warning);
    }
}

static void remove_meat_without_straw(empire_object *city, int32_t index)
{
    if (city->city_type != EMPIRE_CITY_OURS || empire_object_city_sells_resource(index, RESOURCE_MEAT) == 0 || empire_resource_available(RESOURCE_STRAW)) {
        return;
    }
    for (int32_t i = 0; i < EMPIRE_OBJECT_SOLD_RESOURCES; i++) {
        if (city->sells[i] == RESOURCE_MEAT) {
            city->sells[i] = 0;
            warn(EDITOR_WARNING_MEAT_REMOVED);
            break;
        }
    }
}

static void remove_unsupported_monuments(void)
{
    uint8_t cleared = 0;
    for (int32_t i = 0; i < EMPIRE_EDITOR_MONUMENTS; i++) {
        int16_t *monument = &scenario_data.monument_def_ids[i];
        if (!monument_def_goods_available(*monument)) {
            *monument = 0;
            cleared = 1;
            warn(EDITOR_WARNING_MONUMENTS_REMOVED);
        } else if (cleared) {
            warn(EDITOR_WARNING_MONUMENTS_REMOVED);
        }
    }
}

static void check_zoo(void)
{
    uint16_t *zoo = &scenario_data.allowed_buildings[ALLOWED_ZOO];
    if (*zoo == 0 || !no_warning_shown()) {
        return;
    }
    if (!empire_resource_available(RESOURCE_GAMEMEAT)) {
        remove_allowed(zoo, EDITOR_WARNING_ZOO_NO_STRAW_OR_GAME_MEAT);
    }
    if (!empire_resource_available(RESOURCE_STRAW)) {
        remove_allowed(zoo, EDITOR_WARNING_ZOO_NO_STRAW_OR_GAME_MEAT);
    }
}

static void check_military(void)
{
    uint16_t *allowed = scenario_data.allowed_buildings;
    if (allowed[ALLOWED_FORT_INFANTRY] != 0 && no_warning_shown()) {
        if (allowed[ALLOWED_RECRUITER] == 0) {
            remove_allowed(&allowed[ALLOWED_FORT_INFANTRY], EDITOR_WARNING_FORT_INFANTRY_NO_RECRUITER);
        }
        if (allowed[ALLOWED_WEAPONSMITH] == 0 && !empire_resource_available(RESOURCE_WEAPONS)) {
            remove_allowed(&allowed[ALLOWED_FORT_INFANTRY], EDITOR_WARNING_FORT_INFANTRY_NO_WEAPONS);
        }
    }
    if (allowed[ALLOWED_FORT_ARCHERS] != 0 && no_warning_shown() && allowed[ALLOWED_RECRUITER] == 0) {
        remove_allowed(&allowed[ALLOWED_FORT_ARCHERS], EDITOR_WARNING_FORT_ARCHERS_NO_RECRUITER);
    }
    if (allowed[ALLOWED_FORT_CHARIOTEERS] != 0 && no_warning_shown()) {
        if (allowed[ALLOWED_RECRUITER] == 0) {
            remove_allowed(&allowed[ALLOWED_FORT_CHARIOTEERS], EDITOR_WARNING_FORT_CHARIOTEERS_NO_RECRUITER);
        }
        if (allowed[ALLOWED_CHARIOT_MAKER] == 0 && !empire_resource_available(RESOURCE_CHARIOTS)) {
            allowed[ALLOWED_FORT_CHARIOTEERS] = 0;
            warn(EDITOR_WARNING_FORT_CHARIOTEERS_NO_CHARIOTS);
        }
    }
    if (allowed[ALLOWED_ACADEMY] != 0 && no_warning_shown() && allowed[ALLOWED_RECRUITER] == 0) {
        remove_allowed(&allowed[ALLOWED_ACADEMY], EDITOR_WARNING_ACADEMIES_NO_WOOD);
    }
}

void empire_editor_check_city(int32_t index)
{
    empire_object *city = &empire_map_objects[index];
    remove_meat_without_straw(city, index);
    sort_resources(city->sells, EMPIRE_OBJECT_SOLD_RESOURCES);
    sort_resources(city->buys, EMPIRE_OBJECT_BOUGHT_RESOURCES);
    remove_unsupported_monuments();
    check_allowed_needs(ALLOWED_MORTUARY, RESOURCE_LINEN, EDITOR_WARNING_MORTUARIES_NO_LINEN);
    check_allowed_needs(ALLOWED_SENET_HOUSE, RESOURCE_BEER, EDITOR_WARNING_SENET_HOUSE_NO_BEER);
    check_zoo();
    check_allowed_needs(ALLOWED_SCRIBAL_SCHOOL, RESOURCE_PAPYRUS, EDITOR_WARNING_SCRIBAL_SCHOOLS_NO_PAPYRUS);
    check_allowed_needs(ALLOWED_LIBRARY, RESOURCE_PAPYRUS, EDITOR_WARNING_LIBRARIES_NO_PAPYRUS);
    check_allowed_needs(ALLOWED_WEAPONSMITH, RESOURCE_COPPER, EDITOR_WARNING_WEAPONSMITHS_NO_COPPER);
    check_allowed_needs(ALLOWED_CHARIOT_MAKER, RESOURCE_TIMBER, EDITOR_WARNING_CHARIOT_MAKER_NO_WOOD);
    check_military();
}

static int32_t is_player_city(const empire_object *object)
{
    return object->type == EMPIRE_OBJECT_CITY && object->city_type == EMPIRE_CITY_OURS;
}

void empire_editor_objects_start(void)
{
    int32_t player_cities = 0;
    for (int32_t i = 0; i < EMPIRE_OBJECT_COUNT; i++) {
        empire_object *object = &empire_map_objects[i];
        if (object->in_use == 0) {
            continue;
        }
        if (object->city_type_countdown > 0) {
            object->city_type_countdown--;
        }
        if (is_player_city(object)) {
            player_cities++;
        }
    }
    empire_header[scenario_climate_current].editor_objects_started = (int16_t) ((player_cities >= 0) ? 1 : 0);
    if (player_cities == 1) {
        return;
    }
    for (int32_t i = 0; i < EMPIRE_OBJECT_COUNT; i++) {
        empire_object *object = &empire_map_objects[i];
        if (object->in_use != 0 && is_player_city(object) && object->city_type_countdown == 0) {
            object->city_type = EMPIRE_CITY_EGYPTIAN;
        }
    }
}

static void show_edited_object_image(void)
{
    int32_t selected = g_empire_selected_object;
    empire_object *object = &empire_map_objects[selected];
    if (object->in_use == 0) {
        return;
    }
    if (object->type == EMPIRE_OBJECT_CITY) {
        uint16_t image = (uint16_t) empire_object_city_image(selected);
        object->image_id = image;
        empire_editor_image = image;
    } else if (object->type == EMPIRE_OBJECT_ORNAMENT) {
        int32_t image = image_group(IMAGE_GROUP_EMPIRE_ORNAMENTS) + (uint16_t) object->image_offset;
        empire_editor_image = (image < image_group(IMAGE_GROUP_EMPIRE_ORNAMENTS_MAX)) ? image : image_group(IMAGE_GROUP_EMPIRE_ORNAMENTS_MAX);
    } else {
        empire_editor_image = object->image_id;
    }
}

void empire_editor_button(void)
{
    int32_t mode = button_parameter;
    if (mode == EMPIRE_EDITOR_MODE_OK) {
        if (!editor_food_kinds_valid()) {
            window_popup_dialog_show(POPUP_CANT_LEAVE_WORLD_MAP, button_none, POPUP_BUTTONS_OK);
            return;
        }
        editor_button_empire();
        return;
    }
    if (mode == EMPIRE_EDITOR_MODE_EDIT_OBJECTS) {
        if (empire_editor_object_count <= 0) {
            return;
        }
    } else if (mode == EMPIRE_EDITOR_MODE_DELETE_OBJECT) {
        empire_editor_delete_object(g_empire_selected_object);
        empire_editor_count_objects();
        empire_object_init_cities();
        button_parameter = OBJECT_STEP_NEXT;
        empire_editor_object_step();
        return;
    }
    empire_ui_mode = mode;
    if (mode == EMPIRE_EDITOR_MODE_ADD_ROUTE) {
        int32_t route = empire_route_first_free();
        route_current = route;
        empire_route_clear(&empire_map_routes.routes[route]);
    } else if (mode == EMPIRE_EDITOR_MODE_EDIT_ROUTE) {
        route_current = 1;
        window_redraw_request = 1;
        return;
    }
    if (empire_ui_mode == EMPIRE_EDITOR_MODE_EDIT_OBJECTS) {
        show_edited_object_image();
    }
    window_redraw_request = 1;
}

void empire_editor_object_step(void)
{
    if (window_current_id != WINDOW_EDITOR_EMPIRE) {
        return;
    }
    int32_t step = button_parameter;
    int32_t selected = g_empire_selected_object;
    if (step == OBJECT_STEP_PREVIOUS) {
        g_empire_selected_object = --selected;
    } else if (step == OBJECT_STEP_NEXT) {
        g_empire_selected_object = ++selected;
    }
    if (selected < 0) {
        selected = 0;
        g_empire_selected_object = 0;
    }
    if (selected >= empire_editor_object_count) {
        selected = empire_editor_object_count - 1;
        g_empire_selected_object = selected;
    }
    const empire_object *object = &empire_map_objects[selected];
    if (object->in_use != 0) {
        empire_editor_image = object->image_id;
    }
    window_redraw_request = 1;
}

void empire_editor_file_button(void)
{
    empire_file_select(0);
    window_redraw_request = 1;
}

void empire_editor_city_route_changed(void)
{
    empire_object *city = &empire_map_objects[g_empire_selected_object];
    if (city->route_id > CITY_ROUTE_ID_MAX) {
        city->route_id = CITY_ROUTE_ID_MAX;
    }
}

void empire_editor_open_trade_prices(void)
{
    window_current_id = WINDOW_EDITOR_TRADE_PRICES;
    window_redraw_request = 1;
}
