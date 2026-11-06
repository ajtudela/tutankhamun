// Loading auto_reason_phrases.txt, the reason phrases of the event follow-ups: SCENARIO_REASON_GROUPS rows of byte
// values. A file with another row count asks whether to go on.

#include "scenario/reason_phrases.h"

#include "building/building_data.h"
#include "core/calc.h"
#include "core/text.h"
#include "graphics/graphics_data.h"
#include "io/file.h"
#include "platform/platform_dialog.h"
#include "platform/status_log.h"
#include "scenario/event_helpers.h"
#include "scenario/scenario_data.h"
#include "window/window_data.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

enum reason_phrases_file {
    REASON_PHRASES_BUFFER_BYTES = 100000,
    REASON_PHRASES_LINE_BYTES = 100,
};

static int32_t reason_rows_valid(const char *text, int32_t length)
{
    int32_t step = string_index_of(text, '{', length);
    if (step == 0) {
        return 0;
    }
    for (int32_t rows = 1; rows <= SCENARIO_REASON_GROUPS; rows++) {
        text += step;
        step = string_index_of(text, '{', length);
        if (step == 0) {
            return rows == SCENARIO_REASON_GROUPS;
        }
    }
    return 0;
}

static int32_t reason_rows_accepted(void)
{
    char detail[REASON_PHRASES_LINE_BYTES];
    char question[REASON_PHRASES_LINE_BYTES];
    sprintf(detail, fmt_reason_too_many_lines, SCENARIO_REASON_GROUPS);
    sprintf(question, fmt_continue_question, detail);
    if (platform_message_box(platform_main_window, question, str_reason_data_invalid, PLATFORM_BOX_YES_NO) == PLATFORM_ANSWER_NO) {
        log_info_text(detail, 0);
        return 0;
    }
    return 1;
}

bool scenario_events_load_reason_phrases(void)
{
    if (!spare_screen) {
        log_info_text("ERR:no spare screen for reason table", 0);
        return 0;
    }
    memset(spare_screen, 0, REASON_PHRASES_BUFFER_BYTES);
    if (!io_read_file_into_buffer(reason_phrases_file_name, spare_screen, 0)) {
        log_info_text(str_no_reason_phrases_file, 0);
        return 0;
    }
    int32_t length = file_get_size(reason_phrases_file_name);
    char *text = spare_screen;
    if (!reason_rows_valid(text, length) && !reason_rows_accepted()) {
        return 0;
    }
    for (uint8_t *row = reason_phrases; row < reason_phrases + (SCENARIO_REASON_GROUPS * SCENARIO_REASON_COLUMNS); row += SCENARIO_REASON_COLUMNS) {
        text += string_index_of(text, '{', length);
        for (int32_t column = 0; column < SCENARIO_REASON_COLUMNS; column++) {
            text += string_index_of_number(text);
            row[column] = (uint8_t) string_to_int(text);
            text += string_index_of(text, ',', length);
        }
    }
    log_info_text(str_reason_table_loaded, 0);
    return 1;
}
