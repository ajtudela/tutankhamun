// The scenario editor's allowed buildings window: drawing the list and toggling a row. Drawing first switches off
// the buildings the city cannot run (no linen for the mortuary, no recruiter for the forts, ...) with a popup that
// says why; a toggle runs the same checks for its row.
//
// The checks are not quite symmetric, and the list is longer than its table:
// - the draw checks the chariot maker against chariots, the toggle against timber (the popup says wood);
// - rows 46..48 read the entries after the list, and a flag index from 298 on is a word of event_phrases.

#include "editor/editor_allowed_buildings.h"
#include "city/resource_type.h"
#include "editor/editor_dialogs.h"
#include "editor/editor_text.h"
#include "empire/empire_helpers.h"
#include "graphics/graphics_primitives.h"
#include "graphics/image_collection.h"
#include "text/event_text.h"
#include "text/lang.h"
#include "window/building_menu.h"
#include "window/popup_dialog.h"
#include "window/window_id.h"
#include "window/window_image_group.h"
#include <stdint.h>

int32_t scenario_allowed_building_count(void)
{
    return SCENARIO_ALLOWED_BUILDING_ENTRIES;
}

static int32_t *allowed_list(void)
{
    return &build_menu_types[BUILD_MENU_TYPE_FIRST + BUILD_MENU_GROUPS * BUILD_MENU_ITEMS];
}

enum { ALLOWED_FLAG_FIRST_IN_EVENT_PHRASES = 298 };

static uint16_t *allowed_flag(int32_t entry)
{
    if (entry < ALLOWED_FLAG_FIRST_IN_EVENT_PHRASES) {
        return &scenario_data.allowed_buildings[entry];
    }
    return (uint16_t *) &event_phrases + (entry - ALLOWED_FLAG_FIRST_IN_EVENT_PHRASES);
}

static void disallow_without_resource(uint16_t *flag, int32_t resource, int32_t popup_message)
{
    if (*flag != 0 && window_current_id != WINDOW_POPUP_DIALOG && !empire_resource_available(resource)) {
        *flag = 0;
        window_popup_dialog_show(popup_message, button_none, 1);
    }
}

void window_editor_allowed_buildings_draw(void)
{
    int32_t x0 = ui_origin_x;
    int32_t y0 = ui_origin_y;
    int32_t px = ui_origin_x + 16;
    int32_t py = ui_origin_y + 32;
    disallow_without_resource(&scenario_data.allowed_buildings[ALLOWED_MORTUARY], RESOURCE_LINEN, POPUP_MORTUARIES_NO_LINEN);
    disallow_without_resource(&scenario_data.allowed_buildings[ALLOWED_SENET_HOUSE], RESOURCE_BEER, POPUP_SENET_HOUSES_NO_BEER);
    if (scenario_data.allowed_buildings[ALLOWED_ZOO] != 0 && window_current_id != WINDOW_POPUP_DIALOG &&
        (!empire_resource_available(RESOURCE_GAMEMEAT) || !empire_resource_available(RESOURCE_STRAW))) {
        scenario_data.allowed_buildings[ALLOWED_ZOO] = 0;
        window_popup_dialog_show(POPUP_ZOO_NO_GAME_MEAT_OR_STRAW, button_none, 1);
    }
    disallow_without_resource(&scenario_data.allowed_buildings[ALLOWED_SCRIBAL_SCHOOL], RESOURCE_PAPYRUS, POPUP_SCRIBAL_SCHOOLS_NO_PAPYRUS);
    disallow_without_resource(&scenario_data.allowed_buildings[ALLOWED_LIBRARY], RESOURCE_PAPYRUS, POPUP_LIBRARIES_NO_PAPYRUS);
    disallow_without_resource(&scenario_data.allowed_buildings[ALLOWED_WEAPONSMITH], RESOURCE_COPPER, POPUP_WEAPONSMITHS_NO_COPPER);
    disallow_without_resource(&scenario_data.allowed_buildings[ALLOWED_CHARIOT_MAKER], RESOURCE_CHARIOTS, POPUP_CHARIOT_MAKERS_NO_WOOD);
    if (scenario_data.allowed_buildings[ALLOWED_FORT_INFANTRY] != 0 && window_current_id != WINDOW_POPUP_DIALOG) {
        if (scenario_data.allowed_buildings[ALLOWED_RECRUITER] == 0) {
            scenario_data.allowed_buildings[ALLOWED_FORT_INFANTRY] = 0;
            window_popup_dialog_show(POPUP_FORT_INFANTRY_NO_RECRUITER, button_none, 1);
            return;
        }
        if (scenario_data.allowed_buildings[ALLOWED_WEAPONSMITH] == 0 && !empire_resource_available(RESOURCE_WEAPONS)) {
            scenario_data.allowed_buildings[ALLOWED_FORT_INFANTRY] = 0;
            window_popup_dialog_show(POPUP_FORT_INFANTRY_NO_WEAPONSMITH, button_none, 1);
            window_redraw_request = 1;
            return;
        }
    }
    if (scenario_data.allowed_buildings[ALLOWED_FORT_ARCHERS] != 0 && window_current_id != WINDOW_POPUP_DIALOG &&
        scenario_data.allowed_buildings[ALLOWED_RECRUITER] == 0) {
        scenario_data.allowed_buildings[ALLOWED_FORT_ARCHERS] = 0;
        window_popup_dialog_show(POPUP_FORT_ARCHERS_NO_RECRUITER, button_none, 1);
        return;
    }
    if (scenario_data.allowed_buildings[ALLOWED_FORT_CHARIOTEERS] != 0 && window_current_id != WINDOW_POPUP_DIALOG) {
        if (scenario_data.allowed_buildings[ALLOWED_RECRUITER] == 0) {
            scenario_data.allowed_buildings[ALLOWED_FORT_CHARIOTEERS] = 0;
            window_popup_dialog_show(POPUP_FORT_CHARIOTEERS_NO_RECRUITER, button_none, 1);
            return;
        }
        if (scenario_data.allowed_buildings[ALLOWED_CHARIOT_MAKER] == 0 && !empire_resource_available(RESOURCE_CHARIOTS)) {
            scenario_data.allowed_buildings[ALLOWED_FORT_CHARIOTEERS] = 0;
            window_popup_dialog_show(POPUP_FORT_CHARIOTEERS_NO_CHARIOT_MAKER, button_none, 1);
            window_redraw_request = 1;
            return;
        }
    }
    if (scenario_data.allowed_buildings[ALLOWED_ACADEMY] != 0 && window_current_id != WINDOW_POPUP_DIALOG && scenario_data.allowed_buildings[ALLOWED_RECRUITER] == 0) {
        scenario_data.allowed_buildings[ALLOWED_ACADEMY] = 0;
        window_popup_dialog_show(POPUP_ACADEMIES_NO_RECRUITER, button_none, 1);
    }
    outer_panel_draw(px, py, 38, 26);
    lang_text_draw_ui(LANG_GROUP_EDITOR, EDITOR_TEXT_ALLOWED_STRUCTURES, x0 + 26, y0 + 42, image_group(IMAGE_GROUP_FONT_LARGE), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_centered_ui(LANG_GROUP_WINDOW_HINTS, WINDOW_HINT_RIGHT_CLICK_TO_CLOSE, px, y0 + 424, 608, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    int32_t dy = -270;
    for (int32_t i = ALLOWED_LIST_FIRST_ROW; i < scenario_allowed_building_count(); i++, dy += ALLOWED_ROW_HEIGHT) {
        int32_t entry = allowed_list()[i];
        int32_t column;
        int32_t y;
        if (i <= ALLOWED_LIST_LAST_OF_COLUMN_1) {
            column = 9;
            y = dy + ALLOWED_COLUMN_HEIGHT;
        } else if (i <= ALLOWED_LIST_LAST_OF_COLUMN_2) {
            column = 208;
            y = dy;
        } else {
            column = 407;
            y = dy - ALLOWED_COLUMN_HEIGHT;
        }
        graphics_draw_rect(px + column, y + py, ALLOWED_ROW_WIDTH, 18, (focus_button_id == (i - 1)) ? EDITOR_COLOR_ORANGE : 0);
        int32_t color = (*allowed_flag(entry) != 0) ? image_group(IMAGE_GROUP_FONT_BLACK) : image_group(IMAGE_GROUP_FONT_RED);
        lang_text_draw_centered_ui(LANG_GROUP_ALLOWED_BUILDINGS, entry, px + column, y + 4 + py, ALLOWED_ROW_WIDTH, color, 0, ui_origin_x, ui_origin_y);
    }
}

void editor_allowed_building_toggle(void)
{
    int32_t entry = allowed_list()[button_parameter];
    if (entry == ALLOWED_MORTUARY) {
        if (!empire_resource_available(RESOURCE_LINEN)) {
            scenario_data.allowed_buildings[ALLOWED_MORTUARY] = 0;
            window_popup_dialog_show(POPUP_NO_LINEN, button_none, 1);
            window_redraw_request = 1;
            return;
        }
    } else if (entry == ALLOWED_SENET_HOUSE) {
        if (!empire_resource_available(RESOURCE_BEER)) {
            scenario_data.allowed_buildings[ALLOWED_SENET_HOUSE] = 0;
            window_popup_dialog_show(POPUP_NO_BEER, button_none, 1);
            window_redraw_request = 1;
            return;
        }
    } else {
        if ((entry == ALLOWED_SCRIBAL_SCHOOL || entry == ALLOWED_LIBRARY) && !empire_resource_available(RESOURCE_PAPYRUS)) {
            *allowed_flag(entry) = 0;
            window_popup_dialog_show(POPUP_NO_PAPYRUS, button_none, 1);
            window_redraw_request = 1;
            return;
        }
        if (entry == ALLOWED_WEAPONSMITH) {
            if (!empire_resource_available(RESOURCE_COPPER)) {
                scenario_data.allowed_buildings[ALLOWED_WEAPONSMITH] = 0;
                window_popup_dialog_show(POPUP_NO_COPPER, button_none, 1);
                window_redraw_request = 1;
                return;
            }
        } else if (entry == ALLOWED_CHARIOT_MAKER) {
            if (!empire_resource_available(RESOURCE_TIMBER)) {
                scenario_data.allowed_buildings[ALLOWED_CHARIOT_MAKER] = 0;
                window_popup_dialog_show(POPUP_NO_WOOD, button_none, 1);
                window_redraw_request = 1;
                return;
            }
        } else if (entry == ALLOWED_FORT_INFANTRY || entry == ALLOWED_FORT_ARCHERS || entry == ALLOWED_FORT_CHARIOTEERS || entry == ALLOWED_ACADEMY) {
            if (scenario_data.allowed_buildings[ALLOWED_RECRUITER] == 0) {
                *allowed_flag(entry) = 0;
                window_popup_dialog_show(POPUP_NO_RECRUITER, button_none, 1);
                window_redraw_request = 1;
                return;
            }
            if (entry == ALLOWED_FORT_INFANTRY) {
                if (scenario_data.allowed_buildings[ALLOWED_WEAPONSMITH] == 0 && !empire_resource_available(RESOURCE_WEAPONS)) {
                    scenario_data.allowed_buildings[ALLOWED_FORT_INFANTRY] = 0;
                    window_popup_dialog_show(POPUP_FORT_INFANTRY_NO_WEAPONSMITH, button_none, 1);
                    window_redraw_request = 1;
                    return;
                }
            } else if (entry == ALLOWED_FORT_CHARIOTEERS && scenario_data.allowed_buildings[ALLOWED_CHARIOT_MAKER] == 0 &&
                !empire_resource_available(RESOURCE_CHARIOTS)) {
                scenario_data.allowed_buildings[ALLOWED_FORT_CHARIOTEERS] = 0;
                window_popup_dialog_show(POPUP_FORT_CHARIOTEERS_NO_CHARIOT_MAKER, button_none, 1);
                window_redraw_request = 1;
                return;
            }
        }
    }
    window_redraw_request = 1;
    *allowed_flag(entry) = *allowed_flag(entry) == 0;
}
