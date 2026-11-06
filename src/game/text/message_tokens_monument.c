// The monument tokens of the event phrase table: the requirement texts of the monuments' building info window, from
// the window's progress reports and the info building's type, printed into the phrase object's token buffers. Amounts
// are counted in loads; the percent is clamped to 0..100.

#include "text/message_tokens_monument.h"

#include "building/building.h"
#include "building/building_type.h"
#include "building/monument.h"
#include "building/royal_tomb.h"
#include "building/sun_temple.h"
#include "city/city_player.h"
#include "city/resource_type.h"
#include "game/game_internal.h"
#include "text/event_text.h"
#include "text/lang.h"
#include "text/text_data.h"
#include "text/text_lookup.h"
#include "window/building_info.h"
#include "window/building_info_monuments.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

enum monument_phrase_buffer {
    PHRASE_BUFFER_TOTAL_SECONDARY,
    PHRASE_BUFFER_TOTAL_MAIN,
    PHRASE_BUFFER_LOADS_REMAINDER_SECONDARY,
    PHRASE_BUFFER_LOADS_REMAINDER_MAIN,
    PHRASE_BUFFER_LOADS_CURRENT_SECONDARY,
    PHRASE_BUFFER_LOADS_CURRENT_MAIN,
    PHRASE_BUFFER_NEEDED_REMAINDER_SECONDARY,
    PHRASE_BUFFER_NEEDED_REMAINDER_MAIN,
    PHRASE_BUFFER_NEEDED_CURRENT_SECONDARY,
    PHRASE_BUFFER_NEEDED_CURRENT_MAIN,
};

enum monument_token_amounts {
    STORED_UNITS_PER_LOAD_COUNTED = 400,
    STORED_UNITS_COUNTED = 4,
    PROGRESS_PER_UNIT = 100,
    LOADS_PER_UNIT = 400,
    SCRIPTED_STOCK_LIMIT = 800,
    LIBRARY_STOCK_LIMIT = 4000,
    LIBRARY_STOCK_FIRST_PART = 3200,
    CAESAREUM_GRANITE_REMAINDER = 8,
    CAESAREUM_GRANITE_REMAINDER_LOADS = 2,
    LIBRARY_COPPER_REMAINDER_LOADS = 8,
    LIBRARY_REMAINDER_SMALL = 8,
    LIBRARY_REMAINDER_SMALL_LOADS = 2,
    SPHINX_STAGES = 16,
};

static char *phrase_buffer(int32_t buffer)
{
    return event_phrases.phrase_buffers[buffer];
}

static const monument_text_resources *text_resources(void)
{
    return &bi_monument_resources;
}

static const pyramid_progress *pyramid_report(void)
{
    return &bi_progress_pyramid;
}

static const int32_t *sun_temple_report(void)
{
    return bi_progress_sun_temple;
}

static const library_info_stock *library_stock(void)
{
    return &bi_library_report.stock;
}

static const scripted_monument_info_progress *caesareum_report(void)
{
    return &bi_progress_caesareum;
}

static const scripted_monument_info_progress *lighthouse_report(void)
{
    return &bi_progress_lighthouse;
}

static const int32_t *tomb_report(void)
{
    return bi_progress_tomb;
}

static const abu_simbel_info_progress *abu_simbel_report(void)
{
    return &bi_progress_abu_simbel;
}

static const mausoleum_info_progress *mausoleum_report(void)
{
    return &bi_progress_mausoleum;
}

static int32_t counted_in_loads(int32_t resource)
{
    return resource == RESOURCE_BRICKS || resource == RESOURCE_COPPER;
}

static int32_t first_amount(int32_t value)
{
    return counted_in_loads(text_resources()->first) ? (value * STORED_UNITS_PER_LOAD_COUNTED) : (value * STORED_UNITS_COUNTED);
}

static int32_t second_amount_bricks_only(int32_t value)
{
    return text_resources()->second == RESOURCE_BRICKS ? (value * STORED_UNITS_PER_LOAD_COUNTED) : (value * STORED_UNITS_COUNTED);
}

static const char *amount_text(char *buffer, int32_t resource, int32_t amount)
{
    const char *name = window_building_monument_resource_name(resource);
    if (!name) {
        return 0;
    }
    sprintf(buffer, "%d %s", amount, name);
    return buffer;
}

static const char *loads_text(int32_t buffer_index, int32_t count)
{
    char *buffer = phrase_buffer(buffer_index);
    sprintf(buffer, "%d %s", count, message_token_loads_word(count));
    return buffer;
}

const char *message_token_monument_name(void)
{
    return window_building_monument_name(building_info_building_id);
}

const char *message_token_percent_complete(void)
{
    int32_t value;
    switch (building_info_building()->type) {
        case BUILDING_PYRAMID:
            value = pyramid_report()->percent;
            break;
        case BUILDING_SMALL_OBELISK:
        case BUILDING_LARGE_OBELISK:
            value = building_info_building()->data.obelisk.progress;
            break;
        case BUILDING_SPHINX:
            value = ((int32_t) city_player_current()->sphinx_stage * MONUMENT_PROGRESS_COMPLETE) / SPHINX_STAGES;
            break;
        case BUILDING_SUN_TEMPLE:
            value = sun_temple_report()[SUN_TEMPLE_PROGRESS_PERCENT];
            break;
        case BUILDING_MAUSOLEUM:
            value = mausoleum_report()->percent;
            break;
        case BUILDING_ALEXANDRIA_LIBRARY:
            value = library_stock()->percent;
            break;
        case BUILDING_CAESAREUM:
            value = caesareum_report()->percent;
            break;
        case BUILDING_LIGHTHOUSE:
            value = lighthouse_report()->percent;
            break;
        case BUILDING_SMALL_ROYAL_TOMB:
        case BUILDING_MEDIUM_ROYAL_TOMB:
        case BUILDING_LARGE_ROYAL_TOMB:
        case BUILDING_GRAND_ROYAL_TOMB:
            value = tomb_report()[TOMB_PROGRESS_PERCENT];
            break;
        case BUILDING_ABU_SIMBEL:
            value = abu_simbel_report()->percent;
            break;
        default:
            return 0;
    }
    if (value < 0) {
        value = 0;
    } else if (value > MONUMENT_PROGRESS_COMPLETE) {
        value = MONUMENT_PROGRESS_COMPLETE;
    }
    char *buffer = event_phrases.percent_text;
    sprintf(buffer, "%d%%", value);
    return buffer;
}

const char *message_token_courses_complete(void)
{
    char *buffer = event_phrases.courses_text;
    sprintf(buffer, "%d", pyramid_report()->height);
    return buffer;
}

const char *message_token_needed_current_main(void)
{
    return amount_text(phrase_buffer(PHRASE_BUFFER_NEEDED_CURRENT_MAIN), text_resources()->first, first_amount(pyramid_report()->frontier_main));
}

const char *message_token_needed_current_secondary(void)
{
    int32_t amount = second_amount_bricks_only(pyramid_report()->frontier_casing);
    return amount_text(phrase_buffer(PHRASE_BUFFER_NEEDED_CURRENT_SECONDARY), text_resources()->second, amount);
}

static bool remaining_main_amount(int16_t type, int32_t resource, int32_t *amount)
{
    int32_t value;
    if (type == BUILDING_PYRAMID) {
        *amount = first_amount(pyramid_report()->main_remaining);
        return true;
    }
    if (type == BUILDING_SUN_TEMPLE) {
        value = sun_temple_report()[SUN_TEMPLE_PROGRESS_MATERIAL];
    } else if (type == BUILDING_CAESAREUM) {
        value = caesareum_report()->material;
        if (resource == RESOURCE_GRANITE && value > SCRIPTED_STOCK_LIMIT) {
            *amount = CAESAREUM_GRANITE_REMAINDER;
            return true;
        }
    } else if (type == BUILDING_LIGHTHOUSE) {
        value = lighthouse_report()->material;
    } else if (type == BUILDING_ALEXANDRIA_LIBRARY) {
        int32_t stock = library_stock()->material;
        if (resource == RESOURCE_COPPER) {
            *amount = stock > LIBRARY_STOCK_LIMIT ? LIBRARY_STOCK_FIRST_PART : (stock - SCRIPTED_STOCK_LIMIT);
            return true;
        }
        if (stock <= LIBRARY_STOCK_LIMIT) {
            *amount = stock < SCRIPTED_STOCK_LIMIT ? 0 : LIBRARY_REMAINDER_SMALL;
            return true;
        }
        value = stock - LIBRARY_STOCK_FIRST_PART;
    } else if (type == BUILDING_MAUSOLEUM) {
        value = mausoleum_report()->material;
    } else {
        return false;
    }
    *amount = value / PROGRESS_PER_UNIT;
    return true;
}

const char *message_token_needed_remainder_main(void)
{
    int16_t type = building_info_building()->type;
    int32_t resource = text_resources()->first;
    int32_t amount;
    if (!remaining_main_amount(type, resource, &amount)) {
        return 0;
    }
    return amount_text(phrase_buffer(PHRASE_BUFFER_NEEDED_REMAINDER_MAIN), resource, amount);
}

const char *message_token_needed_remainder_secondary(void)
{
    int32_t value = pyramid_report()->casing_remaining;
    int32_t amount = counted_in_loads(text_resources()->second) ? (value * STORED_UNITS_PER_LOAD_COUNTED) : (value * STORED_UNITS_COUNTED);
    return amount_text(phrase_buffer(PHRASE_BUFFER_NEEDED_REMAINDER_SECONDARY), text_resources()->second, amount);
}

const char *message_token_loads_word(int32_t count)
{
    return lang_get_string(LANG_GROUP_AMOUNTS, (count != 1) + AMOUNT_TEXT_LOAD);
}

const char *message_token_loads_current_main(void)
{
    int32_t count = pyramid_report()->frontier_main;
    const char *word = message_token_loads_word(count);
    if (!word) {
        return 0;
    }
    char *buffer = phrase_buffer(PHRASE_BUFFER_LOADS_CURRENT_MAIN);
    sprintf(buffer, "%d %s", count, word);
    return buffer;
}

const char *message_token_loads_current_secondary(void)
{
    return loads_text(PHRASE_BUFFER_LOADS_CURRENT_SECONDARY, pyramid_report()->frontier_casing);
}

static bool remaining_main_loads(int16_t type, int32_t *count)
{
    int32_t value;
    if (type == BUILDING_PYRAMID) {
        *count = pyramid_report()->main_remaining;
        return true;
    }
    if (type == BUILDING_SUN_TEMPLE) {
        value = sun_temple_report()[SUN_TEMPLE_PROGRESS_MATERIAL];
    } else if (type == BUILDING_LIGHTHOUSE) {
        value = lighthouse_report()->material;
    } else if (type == BUILDING_CAESAREUM) {
        value = caesareum_report()->material;
        if (text_resources()->first == RESOURCE_GRANITE && value > SCRIPTED_STOCK_LIMIT) {
            *count = CAESAREUM_GRANITE_REMAINDER_LOADS;
            return true;
        }
    } else if (type == BUILDING_ALEXANDRIA_LIBRARY) {
        int32_t stock = library_stock()->material;
        if (text_resources()->first == RESOURCE_COPPER) {
            if (stock > LIBRARY_STOCK_LIMIT) {
                *count = LIBRARY_COPPER_REMAINDER_LOADS;
                return true;
            }
            value = stock - SCRIPTED_STOCK_LIMIT;
        } else if (stock > LIBRARY_STOCK_LIMIT) {
            value = stock - LIBRARY_STOCK_FIRST_PART;
        } else {
            *count = stock < SCRIPTED_STOCK_LIMIT ? 0 : LIBRARY_REMAINDER_SMALL_LOADS;
            return true;
        }
    } else if (type == BUILDING_MAUSOLEUM) {
        value = mausoleum_report()->material;
    } else {
        return false;
    }
    *count = value / LOADS_PER_UNIT;
    return true;
}

const char *message_token_loads_remainder_main(void)
{
    int32_t count;
    if (!remaining_main_loads(building_info_building()->type, &count)) {
        return 0;
    }
    return loads_text(PHRASE_BUFFER_LOADS_REMAINDER_MAIN, count);
}

const char *message_token_loads_remainder_secondary(void)
{
    return loads_text(PHRASE_BUFFER_LOADS_REMAINDER_SECONDARY, pyramid_report()->casing_remaining);
}

const char *message_token_total_main(void)
{
    int16_t type = building_info_building()->type;
    int32_t resource = text_resources()->first;
    int32_t amount;
    if (type == BUILDING_PYRAMID) {
        amount = first_amount(pyramid_report()->main_total);
    } else if (type == BUILDING_SUN_TEMPLE) {
        amount = sun_temple_report()[SUN_TEMPLE_PROGRESS_MATERIAL_TOTAL] / PROGRESS_PER_UNIT;
    } else if (type == BUILDING_MAUSOLEUM) {
        amount = mausoleum_report()->material_total / PROGRESS_PER_UNIT;
    } else {
        return 0;
    }
    return amount_text(phrase_buffer(PHRASE_BUFFER_TOTAL_MAIN), resource, amount);
}

const char *message_token_total_secondary(void)
{
    int32_t amount = second_amount_bricks_only(pyramid_report()->casing_total);
    return amount_text(phrase_buffer(PHRASE_BUFFER_TOTAL_SECONDARY), text_resources()->second, amount);
}
