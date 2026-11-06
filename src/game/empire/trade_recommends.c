// The trade recommendations: the food a city keeps before it exports, four rows of six numbers read from
// trade_recommends.txt. A file with another number of rows asks whether to go on.

#include "empire/trade_recommends.h"

#include "building/building_data.h"
#include "core/calc.h"
#include "core/text.h"
#include "empire/empire_data.h"
#include "graphics/graphics_data.h"
#include "io/file.h"
#include "platform/platform_dialog.h"
#include "platform/status_log.h"
#include "window/window_data.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

enum {
    SCREEN_BYTES = 100000,
    TRADE_RECOMMEND_ROWS = 4,
    TRADE_RECOMMEND_FIELDS = 6,
    ROWS_MAX = 299,
};

int32_t trade_recommends_load(void)
{
    char *text = spare_screen;
    if (!text) {
        log_info_text(str_no_spare_screen_model, 0);
        return 0;
    }
    memset(text, 0, SCREEN_BYTES);
    if (!io_read_file_into_buffer(str_trade_recommends_file, spare_screen, 0)) {
        log_info_text(str_no_trade_recommends_file, 0);
        return 0;
    }
    int32_t length = file_get_size(str_trade_recommends_file);
    text = spare_screen;
    int32_t rows = 0;
    for (int32_t left = ROWS_MAX; left > 0; left--) {
        int32_t step = string_index_of(text, '{', length);
        if (step == 0) {
            break;
        }
        rows++;
        text += step;
    }
    if (rows != TRADE_RECOMMEND_ROWS) {
        char detail[100];
        char question[100];
        sprintf(detail, fmt_trade_recommends_bad_lines, rows + 1, TRADE_RECOMMEND_ROWS);
        sprintf(question, fmt_continue_question, detail);
        if (platform_message_box(platform_main_window, question, str_trade_recommends_out_of_sync, PLATFORM_BOX_YES_NO) == PLATFORM_ANSWER_NO) {
            log_info_text(detail, 0);
            return 0;
        }
    }
    text = spare_screen;
    int32_t *dst = trade_recommends;
    for (int32_t left = ROWS_MAX; left > 0 && dst < trade_recommends + (TRADE_RECOMMEND_ROWS * TRADE_RECOMMEND_FIELDS); left--) {
        text += string_index_of(text, '{', length);
        for (int32_t field = 0; field < TRADE_RECOMMEND_FIELDS; field++) {
            text += string_index_of_number(text);
            *dst++ = string_to_int(text);
            text += string_index_of(text, ',', length);
        }
    }
    log_info_text(str_trade_recommends_loaded, 0);
    return 1;
}
