// City messages and warnings. Posting a message (its dialog at once or queued, a warning instead when the player
// filtered it out), the queue and its delays, the monthly sort of the message list, and the short warnings at the top
// of the city view with the tutorial banner of the first career missions.

#include "city/message.h"

#include "building/building_data.h"
#include "city/city_data.h"
#include "city/city_internal.h"
#include "core/calc.h"
#include "core/text.h"
#include "game/game_data.h"
#include "game/tutorial.h"
#include "graphics/button_border.h"
#include "graphics/graphics_data.h"
#include "graphics/image_collection.h"
#include "graphics/image_draw.h"
#include "graphics/label_animation_slots.h"
#include "graphics/text_draw.h"
#include "io/file.h"
#include "platform/platform_time.h"
#include "scenario/requests.h"
#include "sound/sound_play.h"
#include "text/lang.h"
#include "text/lang_message.h"
#include "text/text_data.h"
#include "text/text_lookup.h"
#include "video/smacker.h"
#include "video/video_data.h"
#include "widget/sidebar_buttons.h"
#include "widget/tooltip.h"
#include "widget/widget_data.h"
#include "window/message_dialog.h"
#include "window/window_id.h"
#include "window/window_image_group.h"

#include <string.h>

city_message city_message_swap_buffer;

city_warning city_warnings[MAX_WARNINGS];

static int32_t message_has_video(const lang_message *text)
{
    return file_exists((const char *) (lang_message_data + text->video_text));
}

int32_t city_message_new_id(void)
{
    for (int32_t i = 0; i < MAX_MESSAGES; i++) {
        if (city_messages[i].type == 0) {
            return i;
        }
    }
    return -1;
}

void city_message_set_category(int32_t type)
{
    switch (type) {
        case MESSAGE_UNUSED_MILITARY_1:
        case MESSAGE_WRATH_OF_THE_EMPEROR:
        case MESSAGE_UNUSED_MILITARY_2:
        case MESSAGE_BARBARIANS_ATTACK:
        case MESSAGE_LEGION_ATTACKS:
        case MESSAGE_DISTANT_BATTLE:
        case MESSAGE_ENEMIES_CLOSING:
        case MESSAGE_ENEMIES_AT_THE_DOOR:
        case MESSAGE_UNUSED_MILITARY_3:
        case MESSAGE_UNUSED_MILITARY_4:
        case MESSAGE_UNUSED_MILITARY_5:
            city_message_category = MESSAGE_ADVISOR_MILITARY;
            break;
        case MESSAGE_TEMPLATE_REQUEST:
        case MESSAGE_COMPLIANCE_NOW_POSSIBLE:
        case MESSAGE_STORAGE_YARDS_READY_TO_FULFILL_REQUEST:
            city_message_category = MESSAGE_ADVISOR_IMPERIAL;
            break;
        case MESSAGE_TEMPLATE_GIFT:
            city_message_category = MESSAGE_ADVISOR_PHRASE_BUTTONS;
            break;
        case MESSAGE_CITY_UNEMPLOYMENT:
        case MESSAGE_WORKERS_NEEDED:
            city_message_category = MESSAGE_ADVISOR_LABOR;
            break;
        case MESSAGE_ILLNESS:
        case MESSAGE_DISEASE:
        case MESSAGE_PESTILENCE:
            city_message_category = MESSAGE_ADVISOR_HEALTH;
            break;
        default:
            city_message_category = MESSAGE_ADVISOR_NONE;
            break;
    }
}

static int32_t filtered(int32_t warning)
{
    city_warning_show(warning);
    return 0;
}

static int32_t message_is_minor_blessing(int32_t type)
{
    return (type >= MESSAGE_SMALL_BLESSING_FROM_OSIRIS && type <= MESSAGE_SMALL_BLESSING_FROM_BAST) || type == MESSAGE_MINOR_BLESSING_TRADING_FROM_RA;
}

int32_t city_message_check_filter(int32_t type, int16_t *phrases)
{
    uint32_t filter = settings_data.message_filter;
    switch (type) {
        case MESSAGE_TEMPLATE_GIFT:
            return 1;
        case MESSAGE_PERFECT_INUNDATION:
            return (filter & MESSAGE_FILTER_FLOOD) ? filtered(WARNING_FLOOD_PERFECT) : 1;
        case MESSAGE_EXCELLENT_INUNDATION:
            return (filter & MESSAGE_FILTER_FLOOD) ? filtered(WARNING_FLOOD_EXCELLENT) : 1;
        case MESSAGE_GOOD_INUNDATION:
            return (filter & MESSAGE_FILTER_FLOOD) ? filtered(WARNING_FLOOD_GOOD) : 1;
        case MESSAGE_MEDIOCRE_INUNDATION_SEERS:
            return filtered(WARNING_FLOOD_MEDIOCRE);
        case MESSAGE_MEDIOCRE_INUNDATION:
            return (filter & MESSAGE_FILTER_FLOOD) ? filtered(WARNING_FLOOD_MEDIOCRE) : 1;
        case MESSAGE_POOR_INUNDATION_SEERS:
            return filtered(WARNING_FLOOD_POOR);
        case MESSAGE_POOR_INUNDATION:
            return (filter & MESSAGE_FILTER_FLOOD) ? filtered(WARNING_FLOOD_POOR) : 1;
        case MESSAGE_NO_INUNDATION:
            return (filter & MESSAGE_FILTER_FLOOD) ? filtered(WARNING_FLOOD_FAILED) : 1;
        case MESSAGE_PRICE_INCREASE:
            return (filter & MESSAGE_FILTER_PRICE) ? filtered(WARNING_PRICE_INCREASE) : 1;
        case MESSAGE_PRICE_DECREASE:
            return (filter & MESSAGE_FILTER_PRICE) ? filtered(WARNING_PRICE_DECREASE) : 1;
        case MESSAGE_WAGES_LOWERED:
            return (filter & MESSAGE_FILTER_WAGES) ? filtered(WARNING_WAGES_LOWERED) : 1;
        case MESSAGE_WAGES_RAISED:
            return (filter & MESSAGE_FILTER_WAGES) ? filtered(WARNING_WAGES_RAISED) : 1;
        case MESSAGE_TRADE_DECREASES:
            return (filter & MESSAGE_FILTER_TRADE) ? filtered(WARNING_TRADE_DECREASES) : 1;
        case MESSAGE_TRADE_INCREASES:
            return (filter & MESSAGE_FILTER_TRADE) ? filtered(WARNING_TRADE_INCREASES) : 1;
        case MESSAGE_TEMPLATE_GENERAL:
            return (phrases[0] == MESSAGE_PHRASE_KINGDOM && (filter & MESSAGE_FILTER_KINGDOM)) ? filtered(WARNING_KINGDOM_STANDING_RISES) : 1;
    }
    if (type >= MESSAGE_POPULATION_100 && type <= MESSAGE_POPULATION_25000 && (filter & MESSAGE_FILTER_POPULATION)) {
        return filtered(WARNING_POPULATION_MILESTONE);
    }
    if (message_is_minor_blessing(type) && (filter & MESSAGE_FILTER_MINOR_BLESSINGS)) {
        return filtered(WARNING_MINOR_GOD_BLESSING);
    }
    if (type >= MESSAGE_COMMON_FESTIVAL && type <= MESSAGE_GRAND_FESTIVAL && (filter & MESSAGE_FILTER_FESTIVALS)) {
        return filtered(WARNING_FESTIVAL_STARTING);
    }
    if (type == MESSAGE_COMPLIANCE_NOW_POSSIBLE || (type == MESSAGE_STORAGE_YARDS_READY_TO_FULFILL_REQUEST && (filter & MESSAGE_FILTER_COMPLIANCE))) {
        city_warning_show(WARNING_GOODS_AUTO_DISPATCHED);
        scenario_request_dispatch(scenario_current_request);
        return 0;
    }
    if (type == MESSAGE_STORAGE_YARDS_READY_TO_FULFILL_REQUEST) {
        return 1;
    }
    if (type == MESSAGE_DISEASE_STRIKES) {
        return (filter & MESSAGE_FILTER_DISEASE) ? filtered(WARNING_DISEASE_STRIKES) : 1;
    }
    if (type == MESSAGE_MALARIA) {
        return (filter & MESSAGE_FILTER_MALARIA) ? filtered(WARNING_MALARIA_STRIKES) : 1;
    }
    if (type == MESSAGE_WORKERS_NEEDED && (filter & MESSAGE_FILTER_EMPLOYEES)) {
        return filtered(WARNING_EMPLOYEES_NEEDED);
    }
    return 1;
}

static void show_dialog(int32_t index, int32_t type)
{
    city_message *m = &city_messages[index];
    city_message_queue_delay = MESSAGE_QUEUE_DELAY;
    city_message_shown_index = index;
    m->is_read = 1;
    dialog_month = m->month;
    dialog_year = m->year;
    dialog_param1 = m->param1;
    dialog_param2 = m->param2;
    game_fast_forward_year = 0;
    city_message_shown_type = type;
    tooltip_invalidate();
    if (message_has_video(lang_message_get(type))) {
        video_system_mode = VIDEO_SYSTEM_MODE_MESSAGE;
    }
}

void city_message_post(int32_t type, int32_t param1, int32_t param2, int16_t *phrases, bool no_check)
{
    if (!no_check) {
        city_message_use_popup = city_message_check_filter(type, phrases);
    } else {
        city_message_use_popup = 1;
    }
    if (window_current_id == WINDOW_CITY_LEGION_SELECTED || window_current_id == WINDOW_CITY_LEGION_ORDER ||
        window_current_id == WINDOW_CITY_TRANSPORT_SELECTED || window_current_id == WINDOW_CITY_WARSHIP_SELECTED) {
        window_current_id = WINDOW_CITY;
    }
    int32_t id = city_message_new_id();
    if (id == -1) {
        return;
    }
    sound_effect_play(SOUND_EFFECT_MESSAGE);
    if (city_message_use_popup == 0) {
        buttons_expanded_build[SIDEBAR_BUTTON_MESSAGES].animation = 1;
    }
    city_message *m = &city_messages[id];
    m->type = (int16_t) type;
    m->is_read = 0;
    m->year = (int16_t) game_time_year;
    m->month = (uint8_t) game_time_month;
    m->param1 = param1;
    m->param2 = param2;
    city_message_total++;
    m->sequence = (int16_t) city_message_next_sequence;
    city_message_current_id = id;
    city_message_next_sequence++;
    bool has_phrases = false;
    if (phrases == NULL) {
        m->phrases[0] = -1;
        m->phrases[1] = -1;
        m->phrases[2] = -1;
    } else {
        memcpy(m->phrases, phrases, sizeof(m->phrases));
        has_phrases = phrases[0] >= 0;
    }
    if (window_current_id == WINDOW_CITY) {
        city_message_set_category(type);
        id = city_message_current_id;
    }
    int32_t kind = lang_message_get(type)->message_type;
    if (kind == LANG_MESSAGE_TYPE_DISASTER || kind == LANG_MESSAGE_TYPE_INVASION) {
        city_message_problem_count = 1;
        window_redraw_request = 1;
    }
    if (city_message_use_popup != 0) {
        if (window_current_id == WINDOW_CITY) {
            show_dialog(id, type);
            window_message_dialog_show(city_message_shown_type, 0, has_phrases);
        } else {
            for (int32_t i = 0; i < MESSAGE_QUEUE_SIZE; i++) {
                if (city_message_queue[i] == 0) {
                    city_message_queue[i] = city_messages[id].sequence;
                    break;
                }
            }
        }
    }
    city_message_posted_once = 1;
    city_message_use_popup = 0;
    window_ui_redraw = 1;
}

void city_message_process_queue(void)
{
    if (window_current_id != WINDOW_CITY) {
        return;
    }
    if (city_message_queue_delay > 0) {
        city_message_queue_delay--;
        return;
    }
    int32_t slot = 0;
    while (city_message_queue[slot] == 0) {
        if (++slot >= MESSAGE_QUEUE_SIZE) {
            return;
        }
    }
    int32_t sequence = city_message_queue[slot];
    city_message_queue[slot] = 0;
    if (sequence <= 0) {
        return;
    }
    for (int32_t i = 0; i < MAX_MESSAGES && city_messages[i].type != 0; i++) {
        city_message *m = &city_messages[i];
        if (m->sequence != sequence) {
            continue;
        }
        int32_t type = m->type;
        city_message_queue_delay = MESSAGE_QUEUE_DELAY;
        city_message_set_category(type);
        m->is_read = 1;
        dialog_param1 = m->param1;
        dialog_year = m->year;
        dialog_month = m->month;
        dialog_param2 = m->param2;
        game_fast_forward_year = 0;
        city_message_shown_type = type;
        city_message_shown_index = i;
        tooltip_invalidate();
        if (message_has_video(lang_message_get(type))) {
            video_system_mode = VIDEO_SYSTEM_MODE_MESSAGE;
        }
        city_message_use_popup = 1;
        window_message_dialog_show(city_message_shown_type, 0, m->phrases[0] >= 0);
        window_ui_redraw = 1;
        city_message_use_popup = 0;
        return;
    }
}

void city_message_sort_and_compact(void)
{
    int16_t shown_sequence = -1;
    if (city_message_shown_index >= 0) {
        shown_sequence = city_messages[city_message_shown_index].sequence;
    }
    for (int32_t pass = 0; pass < MAX_MESSAGES; pass++) {
        for (int32_t i = 1; i < MAX_MESSAGES; i++) {
            city_message *prev = &city_messages[i - 1];
            city_message *cur = &city_messages[i];
            int32_t swap;
            if (prev->type == 0) {
                swap = cur->type != 0;
            } else {
                swap = prev->sequence < cur->sequence && cur->type != 0;
            }
            if (swap) {
                memcpy(&city_message_swap_buffer, prev, sizeof(city_message));
                memcpy(prev, cur, sizeof(city_message));
                memcpy(cur, &city_message_swap_buffer, sizeof(city_message));
            }
        }
    }
    city_message_total = 0;
    city_message_scroll_position = 0;
    for (int32_t i = 0; i < MAX_MESSAGES; i++) {
        if (city_messages[i].type != 0) {
            city_message_total++;
        }
    }
    if (city_message_total <= MESSAGE_LIST_VISIBLE_ROWS) {
        city_message_list_scroll = 0;
    } else {
        city_message_scroll_position = city_message_total - MESSAGE_LIST_VISIBLE_ROWS;
    }
    int32_t index = city_message_shown_index;
    if (shown_sequence >= 0) {
        for (index = 0; city_messages[index].sequence != shown_sequence; index++) {
            if (index + 1 >= MAX_MESSAGES) {
                return;
            }
        }
    }
    city_message_shown_index = index;
}

void city_message_init_problem_areas(void)
{
    city_message_problem_count = 0;
    city_message_problem_area_b = 0;
    city_message_problem_area_time = platform_time_ms() - 10000;
}

void city_message_decrease_delays(void)
{
    for (int32_t i = 0; i < 20; i++) {
        if (city_message_delay[i] > 0) {
            city_message_delay[i]--;
        }
    }
}

void city_warning_show(int32_t warning)
{
    if (!settings_data.warnings || (warning == 2 && editor_active == 1) || window_current_id == WINDOW_FRONT_MENU) {
        return;
    }
    int32_t i;
    for (i = 0; i < MAX_WARNINGS; i++) {
        if (city_warnings[i].in_use == 0) {
            break;
        }
    }
    if (i == MAX_WARNINGS) {
        return;
    }
    city_warning *w = &city_warnings[i];
    label_animation[LABEL_SLOT_WARNING_HOVER + i] = 0;
    w->in_use = 1;
    const char *text;
    if (warning == WARNING_ORIENTATION) {
        text = lang_get_string(LANG_GROUP_DIRECTIONS, map_info.orientation);
    } else {
        text = lang_get_string(LANG_GROUP_WARNINGS, warning - WARNING_MUST_BUILD_ON_CLEARED_LAND);
    }
    city_warning_shown = 1;
    w->warning_id = (int16_t) warning;
    w->box_width = (int16_t) text_get_width((const uint8_t *) text, image_group(IMAGE_GROUP_FONT_BLACK));
    w->text_length = (int16_t) string_length(text);
    int16_t width = w->box_width;
    if (width <= WARNING_TEXT_WIDTH_SHORT) {
        w->box_width = WARNING_BOX_WIDTH_SHORT;
    } else if (width <= WARNING_TEXT_WIDTH_MEDIUM) {
        w->box_width = WARNING_BOX_WIDTH_MEDIUM;
    } else {
        w->box_width = (width > WARNING_TEXT_WIDTH_LONG) ? WARNING_BOX_WIDTH_WIDEST : WARNING_BOX_WIDTH_LONG;
    }
    w->time = time_current_millis;
    memset(w->text, 0, sizeof(w->text));
    memcpy(w->text, text, w->text_length);
}

void city_warning_clear_all(void)
{
    int32_t x = (g_screen_width - WARNING_SCREEN_MARGIN) / 2;
    for (int32_t i = 0; i < MAX_WARNINGS; i++) {
        label_animation[LABEL_SLOT_WARNING_HOVER + i] = 0;
    }
    for (int32_t i = 0; i < MAX_WARNINGS; i++) {
        city_warning *w = &city_warnings[i];
        city_warning_positions[i * 2] = x;
        w->x = (int16_t) x;
        w->y = (int16_t) city_warning_positions[(i * 2) + 1];
        w->in_use = 0;
    }
}

const char *city_warning_tutorial_hint_text(void)
{
    if (settings_data.is_custom_scenario != 0) {
        return NULL;
    }
    int8_t mission = settings_data.mission_id;
    if (mission == 0) {
        if (tutorial_flags_a[TUTORIAL_FLAG_FIRE] != 0 && building_count_active[BUILDING_FIREHOUSE] <= 0) {
            return lang_get_string(LANG_GROUP_MISSION_GOALS, TUTORIAL_HINT_BUILD_FIREHOUSES);
        }
        if (tutorial_flags_a[TUTORIAL_FLAG_COLLAPSE] != 0 && building_count_active[BUILDING_ARCHITECT_POST] <= 0) {
            return lang_get_string(LANG_GROUP_MISSION_GOALS, TUTORIAL_HINT_BUILD_ARCHITECTS_POSTS);
        }
        if (tutorial_flags_a[TUTORIAL_FLAG_GRANARY_FOOD] != 0) {
            return lang_get_string(LANG_GROUP_MISSION_GOALS, TUTORIAL_HINT_FOOD_FROM_BAZAAR);
        }
        if (tutorial_flags_a[TUTORIAL_FLAG_POPULATION_150] != 0) {
            return lang_get_string(LANG_GROUP_MISSION_GOALS, TUTORIAL_HINT_BUILD_GRANARY_FOR_GAME);
        }
        return lang_get_string(LANG_GROUP_MISSION_GOALS, TUTORIAL_HINT_CREATE_HOUSING);
    }
    if (mission == 1) {
        if (tutorial_flags_a[TUTORIAL_FLAG_ENTERTAINMENT] != 0) {
            return lang_get_string(LANG_GROUP_MISSION_GOALS, TUTORIAL_HINT_BUILD_BOOTHS);
        }
        if (tutorial_flags_a[TUTORIAL_FLAG_GOLD_DELIVERED] != 0) {
            return lang_get_string(LANG_GROUP_MISSION_GOALS, TUTORIAL_HINT_BUILD_TEMPLES_TO_BAST);
        }
        return lang_get_string(LANG_GROUP_MISSION_GOALS, TUTORIAL_HINT_MINE_GOLD);
    }
    if (mission == 2) {
        if (tutorial_flags_a[TUTORIAL_FLAG_DISEASE] != 0 && building_count_active[BUILDING_PHYSICIAN] <= 0 && building_count_active[BUILDING_APOTHECARY] <= 0) {
            return lang_get_string(LANG_GROUP_MISSION_GOALS, TUTORIAL_HINT_BUILD_APOTHECARIES);
        }
        if (tutorial_flags_a[TUTORIAL_FLAG_POTTERY_STORED] != 0) {
            return lang_get_string(LANG_GROUP_MISSION_GOALS, TUTORIAL_HINT_BEAUTIFY_CITY);
        }
        if (tutorial_flags_a[TUTORIAL_FLAG_FOOD_STOCK] != 0) {
            return lang_get_string(LANG_GROUP_MISSION_GOALS, TUTORIAL_HINT_STORE_POTTERY);
        }
        return lang_get_string(LANG_GROUP_MISSION_GOALS, TUTORIAL_HINT_STORE_FIGS);
    }
    if (mission == 3) {
        if (tutorial_flags_a[TUTORIAL_FLAG_BEER_STORED] == 0) {
            return lang_get_string(LANG_GROUP_MISSION_GOALS, TUTORIAL_HINT_BREW_BEER);
        }
        return NULL;
    }
    if (mission == 4) {
        if (city_player_current()->rating_monument == scenario_data.win_monument) {
            return NULL;
        }
        if (tutorial_flags_a[TUTORIAL_FLAG_BRICKS_STORED] != 0) {
            return lang_get_string(LANG_GROUP_MISSION_GOALS, TUTORIAL_HINT_BUILD_MASTABA);
        }
        if (tutorial_flags_a[TUTORIAL_FLAG_SCRIBE] != 0) {
            return lang_get_string(LANG_GROUP_MISSION_GOALS, TUTORIAL_HINT_IMPORT_BRICKS);
        }
        if (tutorial_flags_a[TUTORIAL_FLAG_HOUSES_DONE] != 0) {
            return lang_get_string(LANG_GROUP_MISSION_GOALS, TUTORIAL_HINT_MAKE_PAPYRUS);
        }
        return lang_get_string(LANG_GROUP_MISSION_GOALS, TUTORIAL_HINT_SPACIOUS_APARTMENT);
    }
    return NULL;
}

void city_warning_draw(void)
{
    if (window_current_id != WINDOW_CITY && window_current_id != WINDOW_TOOLTIP_HOVER) {
        city_warning_clear_all();
    }
    int32_t y_offset = 0;
    if (settings_data.paused) {
        y_offset = WARNING_PAUSED_OFFSET;
    }
    if (window_current_id == WINDOW_CITY || window_current_id == WINDOW_TOOLTIP_HOVER || window_current_id == WINDOW_BUILD_MENU) {
        const char *hint = city_warning_tutorial_hint_text();
        if (hint) {
            const city_warning *first = &city_warnings[0];
            int32_t y = first->y + y_offset;
            int32_t x = first->x;
            label_draw_animated(WARNING_TUTORIAL_LABEL, x - TUTORIAL_BANNER_HALF_WIDTH, y, TUTORIAL_BANNER_BLOCKS, 1, 1);
            image_draw(image_group(IMAGE_GROUP_CONTEXT_ICONS) + ICON_TUTORIAL_BANNER_END, x - TUTORIAL_BANNER_LEFT_ICON, y + 2);
            image_draw(image_group(IMAGE_GROUP_CONTEXT_ICONS) + ICON_TUTORIAL_BANNER_END, x + TUTORIAL_BANNER_RIGHT_ICON, y + 2);
            text_draw_centered((const uint8_t *) hint, x - TUTORIAL_BANNER_HALF_WIDTH, y + 4, WARNING_BOX_WIDTH_WIDEST, image_group(IMAGE_GROUP_FONT_WHITE), 0);
            y_offset += TUTORIAL_BANNER_HEIGHT;
        }
    }
    city_warning_active_count = 0;
    for (int32_t i = 0; i < MAX_WARNINGS; i++) {
        city_warning *w = &city_warnings[i];
        int32_t button_id = WARNING_TUTORIAL_LABEL + i;
        if (w->in_use != 0) {
            city_warning_active_count++;
            int32_t y = w->y + y_offset;
            int32_t box_width = w->box_width;
            int32_t x = w->x;
            int32_t height_blocks = (box_width / 16) + 1;
            int32_t half_width = box_width / 2;
            int32_t left = x - half_width + 1;
            label_draw_animated(button_id, left, y, height_blocks, 0, 1);
            if (label_animation[LABEL_SLOT_WARNING_HOVER + i] > WARNING_HOVER_TEXT_SHOWN) {
                if (box_width < WARNING_BOX_WIDTH_WIDEST) {
                    image_draw(image_group(IMAGE_GROUP_CONTEXT_ICONS) + ICON_WARNING_BOX_END, x - half_width + 2, y + 2);
                    image_draw(image_group(IMAGE_GROUP_CONTEXT_ICONS) + ICON_WARNING_BOX_END, x + half_width - WARNING_RIGHT_ICON_INSET, y + 2);
                }
                text_draw_centered((const uint8_t *) w->text, left, y + 4, box_width, image_group(IMAGE_GROUP_FONT_WHITE), 0);
            }
            if (label_animation_click() != 0) {
                g_force_refresh = 1;
                label_animation[LABEL_SLOT_WARNING_HOVER + i] += 4;
            }
            if (w->time + WARNING_DURATION_MS < time_current_millis) {
                w->in_use = 0;
            }
        }
    }
}
