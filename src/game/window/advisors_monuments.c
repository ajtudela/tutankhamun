// The monuments advisor: the burial provisions still needed (with what the storage yards hold), a row per monument of
// the scenario (what it needs before it is placed, its progress while built, the completion text when done), and its
// buttons: send a needed provision (the burial provisions window) and go to an unfinished monument.

#include "window/advisors_monuments.h"
#include "building/abu_simbel.h"
#include "building/building.h"
#include "building/building_type.h"
#include "building/caesareum.h"
#include "building/library.h"
#include "building/lighthouse.h"
#include "building/mausoleum.h"
#include "building/monument.h"
#include "building/monument_data.h"
#include "building/monument_tables.h"
#include "building/royal_tomb.h"
#include "building/sun_temple.h"
#include "city/resource_type.h"
#include "game/game_internal.h"
#include "graphics/button_border.h"
#include "graphics/graphics_data.h"
#include "graphics/graphics_primitives.h"
#include "graphics/image_collection.h"
#include "graphics/image_draw.h"
#include "graphics/label_animation_slots.h"
#include "graphics/panel_draw.h"
#include "graphics/text_draw.h"
#include "graphics/ui_layout.h"
#include "text/lang.h"
#include "widget/city_view.h"
#include "window/advisors.h"
#include "window/advisors_kingdom.h"
#include "window/popup_dialog.h"
#include "window/window_id.h"
#include "window/window_image_group.h"
#include <stdint.h>

enum monuments_layout {
    MONUMENTS_WIDTH_BLOCKS = 40,
    MONUMENTS_HEIGHT_BLOCKS = 27,
    MONUMENTS_BLOCK = 16,
    PROVISION_ROW_HEIGHT = 42,
    PROVISION_ROWS_Y = 66,
    MONUMENT_FIRST_ROW = 5,
    MONUMENT_TEXT_INSET = 8,
    MONUMENT_SECOND_LINE_Y = 24,
    MONUMENT_BORDER_UNFINISHED = 0xF800,
};

enum monument_text_step {
    MONUMENT_TEXT_NOT_BEGUN = 0,
    MONUMENT_TEXT_NEEDS = 1,
    MONUMENT_TEXT_PERCENT = 2,
    MONUMENT_TEXT_NOW_COMPLETE = 3,
    MONUMENT_TEXT_COMPLETE_FOREVER = 4,
};

enum monument_amounts {
    SPHINX_STAGES = 16,
    SMALL_OBELISK_BLOCKS_NEEDED = 10000,
    LARGE_OBELISK_BLOCKS_NEEDED = 20000,
    SUN_TEMPLE_BLOCKS_NEEDED = 22000,
    MAUSOLEUM_BLOCKS_NEEDED = 24000,
    STORAGE_UNITS_PER_BLOCK = 100,
    RECENT_COMPLETION_MONTHS = 12,
    RESOURCE_UNIT_NAMES = 54,
};

typedef struct {
    int32_t text;
    int32_t text2;
    int32_t percent;
    int32_t needed;
    int32_t have;
} monument_row;

static const int16_t *monument_rect(int32_t row)
{
    return &mansion_salary_button.monuments_advisor[row].x;
}

static int32_t resource_stored_in_hundredths(int32_t resource)
{
    switch (resource) {
        case RESOURCE_WEAPONS:
        case RESOURCE_STONE:
        case RESOURCE_LIMESTONE:
        case RESOURCE_GRANITE:
        case RESOURCE_CHARIOTS:
        case RESOURCE_SANDSTONE:
        case RESOURCE_MARBLE:
        case RESOURCE_DEBEN:
            return 1;
        default:
            return 0;
    }
}

static int32_t resource_is_burial_provision(int32_t resource)
{
    switch (resource) {
        case RESOURCE_GRAIN:
        case RESOURCE_GAMEMEAT:
        case RESOURCE_WEAPONS:
        case RESOURCE_POTTERY:
        case RESOURCE_BEER:
        case RESOURCE_LINEN:
        case RESOURCE_GEMS:
        case RESOURCE_LUXURY_GOODS:
        case RESOURCE_TIMBER:
        case RESOURCE_PAPYRUS:
        case RESOURCE_STONE:
        case RESOURCE_LIMESTONE:
        case RESOURCE_GRANITE:
        case RESOURCE_CHARIOTS:
        case RESOURCE_SANDSTONE:
            return 1;
        default:
            return 0;
    }
}

static int32_t resource_counted_in_blocks(int32_t resource)
{
    return resource == RESOURCE_STONE || resource == RESOURCE_LIMESTONE || resource == RESOURCE_GRANITE || resource == RESOURCE_SANDSTONE ||
        resource == RESOURCE_MARBLE;
}

static void draw_monuments_text(int32_t text, int32_t x, int32_t y)
{
    lang_text_draw_ui(LANG_GROUP_MONUMENTS_ADVISOR, text, x, y, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
}

static void draw_number(int32_t value, int32_t x, int32_t y)
{
    text_draw_number_ui(value, NUMBER_PREFIX_NONE, postfix_none, x, y, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
}

static void draw_provision_row(const city_player *player, int32_t resource, int32_t row, int32_t y)
{
    g_text_width = 0;
    image_draw(image_group(IMAGE_GROUP_RESOURCE_ICONS) + resource, advisor_x + 40, y);
    int32_t missing = scenario_data.provisions_needed[resource] - scenario_data.provisions_sent[resource];
    if (!resource_stored_in_hundredths(resource)) {
        missing *= STORAGE_UNITS_PER_BLOCK;
    }
    button_border_draw_rect(monument_rect(row), focus_button_id == (row + 1), 0, 0);
    g_text_width = 0;
    if (missing != 0) {
        draw_number(missing, advisor_x + 60, y);
    } else {
        draw_monuments_text(MONUMENTS_TEXT_FINISHED, advisor_x + 60, y);
    }
    int32_t plural;
    if (resource_counted_in_blocks(resource)) {
        plural = 1;
    } else if (resource == RESOURCE_WEAPONS || resource == RESOURCE_CHARIOTS) {
        plural = missing > 1;
    } else {
        plural = 0;
    }
    int32_t name = plural ? (resource + RESOURCE_UNIT_NAMES) : resource;
    lang_text_draw_ui(LANG_GROUP_RESOURCE_NAMES, name, g_text_width + advisor_x + 60, y, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
    g_text_width = 0;
    int32_t stored = player->storage_stored[resource];
    if (resource_stored_in_hundredths(resource)) {
        stored /= STORAGE_UNITS_PER_BLOCK;
    }
    draw_number(stored, advisor_x + 300, y);
    int32_t stored_text = MONUMENTS_TEXT_IN_STORAGE;
    if (resource_counted_in_blocks(resource)) {
        stored_text = (stored == 1) ? MONUMENTS_TEXT_BLOCK_STORED : MONUMENTS_TEXT_BLOCKS_STORED;
    }
    draw_monuments_text(stored_text, g_text_width + advisor_x + 300, y);
}

static void draw_provision_rows(const city_player *player)
{
    int32_t row = 0;
    for (int32_t resource = 0; resource < RESOURCE_MAX; resource++) {
        if (!resource_is_burial_provision(resource) || scenario_data.provisions_needed[resource] <= 0) {
            continue;
        }
        draw_provision_row(player, resource, row, advisor_y + PROVISION_ROWS_Y + (row * PROVISION_ROW_HEIGHT));
        row++;
    }
}

static int32_t pyramid_first_text(int32_t style)
{
    switch (style) {
        case PYRAMID_STYLE_BENT:
            return BENT_PYRAMID_TEXT_FIRST;
        case PYRAMID_STYLE_MUDBRICK:
            return BRICK_PYRAMID_TEXT_FIRST;
        case PYRAMID_STYLE_STEPPED:
            return STEPPED_PYRAMID_TEXT_FIRST;
        case PYRAMID_STYLE_PYRAMID:
            return PYRAMID_TEXT_FIRST;
        default:
            return MASTABA_TEXT_FIRST;
    }
}

static int32_t monument_first_text(int32_t subkind)
{
    switch (subkind) {
        case MONUMENT_DEF_SPHINX:
            return SPHINX_TEXT_FIRST;
        case MONUMENT_DEF_OBELISK:
            return OBELISK_TEXT_FIRST;
        case MONUMENT_DEF_SUN_TEMPLE:
            return SUN_TEMPLE_TEXT_FIRST;
        case MONUMENT_DEF_LIGHTHOUSE:
            return LIGHTHOUSE_TEXT_FIRST;
        case MONUMENT_DEF_LIBRARY:
            return LIBRARY_TEXT_FIRST;
        case MONUMENT_DEF_CAESAREUM:
            return CAESAREUM_TEXT_FIRST;
        case MONUMENT_DEF_SMALL_ROYAL_TOMB:
            return SMALL_TOMB_TEXT_FIRST;
        case MONUMENT_DEF_MEDIUM_ROYAL_TOMB:
            return MEDIUM_TOMB_TEXT_FIRST;
        case MONUMENT_DEF_LARGE_ROYAL_TOMB:
            return LARGE_TOMB_TEXT_FIRST;
        case MONUMENT_DEF_GRAND_ROYAL_TOMB:
            return GRAND_TOMB_TEXT_FIRST;
        case MONUMENT_DEF_ABU_SIMBEL:
            return ABU_SIMBEL_TEXT_FIRST;
        default:
            return MAUSOLEUM_TEXT_FIRST;
    }
}

static int32_t pyramid_progress_percent(int32_t building_id)
{
    pyramid_progress info;
    pyramid_get_progress_info(building_id, &info);
    return info.percent;
}

static int32_t monument_progress(const city_player *player, int32_t subkind, int32_t building_id)
{
    switch (subkind) {
        case MONUMENT_DEF_SPHINX:
            return (player->sphinx_stage * 100) / SPHINX_STAGES;
        case MONUMENT_DEF_OBELISK:
            return building_get(building_id)->data.obelisk.progress;
        case MONUMENT_DEF_SUN_TEMPLE:
            return sun_temple_get_progress(building_id, 0);
        case MONUMENT_DEF_LIGHTHOUSE:
            return lighthouse_get_progress(&lighthouse_object, building_id, 0);
        case MONUMENT_DEF_LIBRARY:
            return library_get_progress(&library_object, building_id, 0);
        case MONUMENT_DEF_CAESAREUM:
            return caesareum_get_progress(&caesareum_object, building_id, 0);
        case MONUMENT_DEF_SMALL_ROYAL_TOMB:
            return royal_tomb_progress(BUILDING_SMALL_ROYAL_TOMB, building_id, 0);
        case MONUMENT_DEF_MEDIUM_ROYAL_TOMB:
            return royal_tomb_progress(BUILDING_MEDIUM_ROYAL_TOMB, building_id, 0);
        case MONUMENT_DEF_LARGE_ROYAL_TOMB:
            return royal_tomb_progress(BUILDING_LARGE_ROYAL_TOMB, building_id, 0);
        case MONUMENT_DEF_GRAND_ROYAL_TOMB:
            return royal_tomb_progress(BUILDING_GRAND_ROYAL_TOMB, building_id, 0);
        case MONUMENT_DEF_ABU_SIMBEL:
            return abu_simbel_get_progress(&abu_simbel_object, building_id, 0);
        default:
            return mausoleum_get_progress(building_id, 0);
    }
}

static void monument_blocks_needed(const city_player *player, int32_t slot, int32_t subkind, monument_row *row)
{
    switch (subkind) {
        case MONUMENT_DEF_OBELISK:
            row->needed = (scenario_monument_slot_obelisk_type(slot) != BUILDING_SMALL_OBELISK) ? LARGE_OBELISK_BLOCKS_NEEDED : SMALL_OBELISK_BLOCKS_NEEDED;
            row->have = player->storage_stored[RESOURCE_GRANITE];
            break;
        case MONUMENT_DEF_SUN_TEMPLE:
            row->needed = SUN_TEMPLE_BLOCKS_NEEDED;
            row->have = player->storage_stored[RESOURCE_SANDSTONE];
            break;
        case MONUMENT_DEF_SPHINX:
        case MONUMENT_DEF_LIGHTHOUSE:
        case MONUMENT_DEF_LIBRARY:
        case MONUMENT_DEF_CAESAREUM:
        case MONUMENT_DEF_SMALL_ROYAL_TOMB:
        case MONUMENT_DEF_MEDIUM_ROYAL_TOMB:
        case MONUMENT_DEF_LARGE_ROYAL_TOMB:
        case MONUMENT_DEF_GRAND_ROYAL_TOMB:
        case MONUMENT_DEF_ABU_SIMBEL:
            break;
        default:
            row->needed = MAUSOLEUM_BLOCKS_NEEDED;
            row->have = player->storage_stored[RESOURCE_SANDSTONE];
            break;
    }
}

static int32_t completion_is_recent(const city_player *player)
{
    return player->monument_completion_flag == 0 && player->monument_completion_months >= RECENT_COMPLETION_MONTHS;
}

static void monument_row_state(const city_player *player, int32_t slot, int32_t row_index, monument_row *row)
{
    const monument_def *def = &monument_defs[scenario_data.monument_def_ids[slot]];
    int32_t pyramid = def->kind == MONUMENT_DEF_KIND_PYRAMID_FAMILY;
    int32_t subkind = def->subkind;
    int32_t first = pyramid ? pyramid_first_text(subkind) : monument_first_text(subkind);
    int32_t is_mausoleum = !pyramid && first == MAUSOLEUM_TEXT_FIRST;
    row->text2 = 0;
    row->needed = 0;
    row->have = 0;
    if (player->monument_def_id[slot] == 0) {
        row->percent = 100;
        row->text = first + MONUMENT_TEXT_NOT_BEGUN;
        row->text2 = first + MONUMENT_TEXT_NEEDS;
        if (!pyramid) {
            monument_blocks_needed(player, slot, subkind, row);
        }
        return;
    }
    int32_t building_id = player->monument_building_id[slot];
    row->percent = pyramid ? pyramid_progress_percent(building_id) : monument_progress(player, subkind, building_id);
    if (row->percent < 100) {
        row->text = first + MONUMENT_TEXT_PERCENT;
        row->text2 = MONUMENTS_TEXT_VISIT_SITE;
        if (!is_mausoleum && focus_button_id == (row_index + 1)) {
            button_border_draw_rect(monument_rect(row_index), MONUMENT_BORDER_UNFINISHED, 0, 0);
        }
        return;
    }
    row->percent = 100;
    if ((pyramid || is_mausoleum) && !completion_is_recent(player)) {
        row->text = first + MONUMENT_TEXT_COMPLETE_FOREVER;
    } else {
        row->text = first + MONUMENT_TEXT_NOW_COMPLETE;
    }
}

static void draw_number_at_rect(int32_t value, const char *postfix, const int16_t *rect, int32_t dx, int32_t dy)
{
    text_draw_number(value, NUMBER_PREFIX_NONE, postfix, rect[0] + ui_origin_x + dx, rect[1] + ui_origin_y + dy, image_group(IMAGE_GROUP_FONT_WHITE), 0);
}

static void draw_monument_row(const monument_row *row, int32_t row_index)
{
    const int16_t *rect = monument_rect(row_index);
    int32_t font = image_group(IMAGE_GROUP_FONT_WHITE);
    g_text_width = 0;
    g_draw_y_accum = 0;
    lang_text_draw_rect_ui(LANG_GROUP_MONUMENTS_ADVISOR, row->text, rect, font, 0, MONUMENT_TEXT_INSET, MONUMENT_TEXT_INSET, ui_origin_x, ui_origin_y);
    if (row->percent < 100) {
        draw_number_at_rect(row->percent, pct_postfix, rect, g_text_width + MONUMENT_TEXT_INSET, MONUMENT_TEXT_INSET);
        lang_text_draw_rect_ui(LANG_GROUP_MONUMENTS_ADVISOR, MONUMENTS_TEXT_FINISHED, rect, font, 0, g_text_width + MONUMENT_TEXT_INSET, MONUMENT_TEXT_INSET,
            ui_origin_x, ui_origin_y);
    }
    if (row->needed != 0) {
        draw_number_at_rect(row->needed / STORAGE_UNITS_PER_BLOCK, postfix_none, rect, g_text_width + MONUMENT_TEXT_INSET, MONUMENT_TEXT_INSET);
    }
    g_text_width = 0;
    if (row->text2 != 0) {
        lang_text_draw_rect_ui(LANG_GROUP_MONUMENTS_ADVISOR, row->text2, rect, font, 0, MONUMENT_TEXT_INSET, MONUMENT_SECOND_LINE_Y, ui_origin_x, ui_origin_y);
    }
    if (row->needed != 0) {
        draw_number_at_rect(row->have / STORAGE_UNITS_PER_BLOCK, postfix_none, rect, g_text_width + MONUMENT_TEXT_INSET, MONUMENT_SECOND_LINE_Y);
        int32_t stored_text = (row->have == STORAGE_UNITS_PER_BLOCK) ? MONUMENTS_TEXT_BLOCK_STORED : MONUMENTS_TEXT_BLOCKS_STORED;
        lang_text_draw_rect_ui(
            LANG_GROUP_MONUMENTS_ADVISOR, stored_text, rect, font, 0, g_text_width + MONUMENT_TEXT_INSET, MONUMENT_SECOND_LINE_Y, ui_origin_x, ui_origin_y);
    }
}

void window_advisor_monuments_draw_foreground(void)
{
    const city_player *player = city_player_current();
    if (monument_count_needed_resources(0) != 0) {
        draw_provision_rows(player);
    }
    inner_panel_draw_ui(advisor_x + 38, advisor_y + 282, 35, 8, ui_origin_x, ui_origin_y);
    int32_t row_index = MONUMENT_FIRST_ROW;
    for (int32_t slot = 0; slot < MONUMENT_SLOTS; slot++) {
        if (scenario_data.monument_def_ids[slot] <= 0) {
            continue;
        }
        monument_row row;
        monument_row_state(player, slot, row_index, &row);
        draw_monument_row(&row, row_index);
        row_index++;
    }
}

int32_t monument_needed_resource_nth(int32_t n)
{
    if (monument_count_needed_resources(0) == 0) {
        return 0;
    }
    int32_t found = 0;
    for (int32_t resource = RESOURCE_MIN; resource < RESOURCE_MAX; resource++) {
        if (scenario_data.provisions_needed[resource] > 0) {
            found++;
        }
        if (found == n) {
            return resource;
        }
    }
    return 0;
}

void button_monument_resource(void)
{
    int32_t resource = monument_needed_resource_nth(button_parameter + 1);
    if (resource == RESOURCE_NONE) {
        return;
    }
    int32_t sent = scenario_data.provisions_sent[resource];
    int32_t missing = scenario_data.provisions_needed[resource] - sent;
    int32_t minimum = resource_stored_in_hundredths(resource) ? 1 : STORAGE_UNITS_PER_BLOCK;
    if (missing <= 0) {
        window_popup_dialog_show(POPUP_PROVISIONS_COMPLETED, button_none, 0);
        return;
    }
    city_player *player = city_player_current();
    if (player->storage_stored[resource] < minimum) {
        window_popup_dialog_show(POPUP_NOT_ENOUGH_GOODS, button_none, 1);
        return;
    }
    player->request_resource = resource;
    player->request_available = sent;
    player->request_amount = 0;
    window_go_burial_provisions();
    window_redraw_request = 1;
}

static int32_t monument_building_unfinished(const city_player *player, int32_t building_id)
{
    switch (building_get(building_id)->type) {
        case BUILDING_PYRAMID:
            return pyramid_progress_percent(building_id) < 100;
        case BUILDING_SMALL_OBELISK:
        case BUILDING_LARGE_OBELISK:
            return monument_progress(player, MONUMENT_DEF_OBELISK, building_id) < 100;
        case BUILDING_SPHINX:
            return player->sphinx_stage < SPHINX_STAGES;
        case BUILDING_SUN_TEMPLE:
            return monument_progress(player, MONUMENT_DEF_SUN_TEMPLE, building_id) < 100;
        case BUILDING_MAUSOLEUM:
            return mausoleum_get_progress(building_id, 0) < 100;
        case BUILDING_ALEXANDRIA_LIBRARY:
            return monument_progress(player, MONUMENT_DEF_LIBRARY, building_id) < 100;
        case BUILDING_CAESAREUM:
            return monument_progress(player, MONUMENT_DEF_CAESAREUM, building_id) < 100;
        case BUILDING_LIGHTHOUSE:
            return monument_progress(player, MONUMENT_DEF_LIGHTHOUSE, building_id) < 100;
        case BUILDING_SMALL_ROYAL_TOMB:
            return monument_progress(player, MONUMENT_DEF_SMALL_ROYAL_TOMB, building_id) < 100;
        case BUILDING_ABU_SIMBEL:
            return monument_progress(player, MONUMENT_DEF_ABU_SIMBEL, building_id) < 100;
        case BUILDING_MEDIUM_ROYAL_TOMB:
            return monument_progress(player, MONUMENT_DEF_MEDIUM_ROYAL_TOMB, building_id) < 100;
        case BUILDING_LARGE_ROYAL_TOMB:
            return monument_progress(player, MONUMENT_DEF_LARGE_ROYAL_TOMB, building_id) < 100;
        case BUILDING_GRAND_ROYAL_TOMB:
            return monument_progress(player, MONUMENT_DEF_GRAND_ROYAL_TOMB, building_id) < 100;
        default:
            return 0;
    }
}

void button_monument_go_to(void)
{
    const city_player *player = city_player_current();
    int32_t building_id = player->monument_building_id[button_parameter];
    if (!monument_building_unfinished(player, building_id)) {
        return;
    }
    city_view_go_to_grid_offset(building_get(building_id)->grid_offset);
    window_current_id = WINDOW_CITY;
    window_redraw_request = 1;
}

void window_advisor_monuments_draw_background(void)
{
    advisor_x = ui_origin_x;
    advisor_y = ui_origin_y;
    advisor_height_blocks = MONUMENTS_HEIGHT_BLOCKS;
    int32_t frame = ADVISOR_OPEN_LAST_STEP - label_animation[LABEL_SLOT_ADVISOR_OPEN];
    if (frame >= ADVISOR_OPEN_LAST_STEP) {
        return;
    }
    if (frame > 0) {
        graphics_draw_rect((frame * MONUMENTS_BLOCK) + ui_origin_x, ui_origin_y + (frame * 8), (MONUMENTS_WIDTH_BLOCKS - (frame * 2)) * MONUMENTS_BLOCK,
            (MONUMENTS_HEIGHT_BLOCKS - (frame * 2)) * MONUMENTS_BLOCK, 0);
        return;
    }
    outer_panel_draw(ui_origin_x, ui_origin_y, MONUMENTS_WIDTH_BLOCKS, MONUMENTS_HEIGHT_BLOCKS);
    inner_panel_draw_ui(advisor_x + 32, advisor_y + 53, 36, 14, ui_origin_x, ui_origin_y);
    image_draw(image_group(IMAGE_GROUP_ADVISOR_ICONS) + 12, advisor_x + 10, advisor_y + 10);
    g_text_width = 0;
    lang_text_draw_ui(LANG_GROUP_MONUMENTS_ADVISOR, MONUMENTS_TEXT_OVERSEER, advisor_x + 60, advisor_y + 12, image_group(IMAGE_GROUP_FONT_LARGE), 0, ui_origin_x, ui_origin_y);
    if (monument_count_needed_resources(0) != 0) {
        lang_text_draw_ui(LANG_GROUP_MONUMENTS_ADVISOR, MONUMENTS_TEXT_CLICK_PROVISIONS_TO_DELIVER, g_text_width + advisor_x + 60, advisor_y + 30, image_group(IMAGE_GROUP_FONT_BLACK),
            0, ui_origin_x, ui_origin_y);
        return;
    }
    draw_monuments_text(MONUMENTS_TEXT_NO_PROVISIONS_NEEDED, advisor_x + 84, advisor_y + 140);
}

int32_t monument_count_needed_resources(int32_t skip)
{
    int32_t count = 0;
    for (int32_t resource = 0; resource < RESOURCE_MAX; resource++) {
        if (resource != skip && scenario_data.provisions_needed[resource] > 0) {
            count++;
        }
    }
    return count;
}
