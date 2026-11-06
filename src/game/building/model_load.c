// The model files: the building and house models, the figure and enemy models and the tax sentiment model, read
// from the text files of the difficulty. A section with the wrong number of rows asks whether to go on; "no"
// fails the load.

#include "building/model_load.h"
#include "building/house_evolve.h"
#include "building/building_type.h"
#include "core/calc.h"
#include "figure/figure_data.h"
#include "graphics/graphics_data.h"
#include "window/window_data.h"
#include "core/text.h"
#include "io/file.h"
#include "platform/platform_dialog.h"
#include "platform/status_log.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

enum model_file_limit {
    MODEL_FILE_BYTES = 100000,
    MODEL_FILE_MAX_ROWS = 299,
};

enum model_file_layout {
    BUILDING_MODEL_BRACES = 257,
    BUILDING_MODEL_FIELDS = 10,
    HOUSE_MODEL_FIELDS = 22,
    FIGURE_MODEL_BRACES = 170,
    FIGURE_MODEL_FIELDS = 14,
    ENEMY_MODEL_FIELDS = 14,
    ENEMY_MODEL_INTS = 840,
    TAX_MODEL_BRACES = 26,
    TAX_MODEL_FIELDS = 11,
    TAX_MODEL_INTS = 286,
};

static int32_t count_braces(char *text, int32_t limit)
{
    int32_t count = 0;
    for (int32_t left = MODEL_FILE_MAX_ROWS; left > 0; left--) {
        int32_t step = string_index_of(text, '{', limit);
        if (step == 0) {
            break;
        }
        count++;
        text += step;
    }
    return count;
}

static void read_rows(char *text, int32_t limit, int32_t *dst, const int32_t *end, int32_t fields)
{
    for (int32_t rows = MODEL_FILE_MAX_ROWS; rows > 0 && dst < end; rows--) {
        text += string_index_of(text, '{', limit);
        for (int32_t field = 0; field < fields; field++) {
            text += string_index_of_number(text);
            *dst++ = string_to_int(text);
            text += string_index_of(text, ',', limit);
        }
    }
}

static int32_t accept_count(int32_t count, int32_t expected, const char *err_format, const char *caption)
{
    if (count == expected) {
        return 1;
    }
    char detail[100];
    char question[100];
    sprintf(detail, err_format, count + 1, expected);
    sprintf(question, fmt_continue_question, detail);
    if (platform_message_box(platform_main_window, question, caption, PLATFORM_BOX_YES_NO) == PLATFORM_ANSWER_NO) {
        log_info_text(detail, NULL);
        return 0;
    }
    return 1;
}

static int32_t load_model_file(const char *path, const char *missing)
{
    char *screen = spare_screen;
    if (screen == 0) {
        log_info_text(str_no_spare_screen_model, NULL);
        return 0;
    }
    memset(screen, 0, MODEL_FILE_BYTES);
    if (io_read_file_into_buffer(path, screen, 0) == 0) {
        log_info_text(missing, NULL);
        return 0;
    }
    return 1;
}

int32_t building_model_load(void)
{
    const char *path = model_names[settings_data.difficulty];
    if (!load_model_file(path, str_no_building_model_file)) {
        return 0;
    }
    int32_t limit = file_get_size(path);
    char *screen = spare_screen;
    char *section = screen + string_find(screen, str_all_buildings, limit);
    if (!accept_count(count_braces(section, limit), BUILDING_MODEL_BRACES, fmt_building_model_bad_lines, str_building_model_out_of_sync)) {
        return 0;
    }
    section = screen + string_find(screen, str_all_buildings, limit);
    read_rows(section, limit, (int32_t *) building_model_table, (int32_t *) (building_model_table + BUILDING_TYPE_COUNT), BUILDING_MODEL_FIELDS);
    section = screen + string_find(screen, str_all_houses, limit);
    read_rows(section, limit, (int32_t *) house_models, (int32_t *) (house_models + HOUSE_MODEL_RECORDS), HOUSE_MODEL_FIELDS);
    log_info_text(str_building_model_loaded, NULL);
    return 1;
}

int32_t figure_model_load(void)
{
    const char *path = figure_names[settings_data.difficulty];
    if (!load_model_file(path, str_no_figure_model_file)) {
        return 0;
    }
    int32_t limit = file_get_size(path);
    char *screen = spare_screen;
    char *section = screen + string_find(screen, str_all_figures, limit);
    if (!accept_count(count_braces(section, limit), FIGURE_MODEL_BRACES, fmt_figure_model_bad_lines, str_figure_model_out_of_sync)) {
        return 0;
    }
    section = screen + string_find(screen, str_all_figures, limit);
    read_rows(section, limit, (int32_t *) figure_properties, (int32_t *) (figure_properties + FIGURE_PROPERTIES_TYPES), FIGURE_MODEL_FIELDS);
    section = screen + string_find(screen, str_all_enemies, limit);
    read_rows(section, limit, (int32_t *) enemy_model, (int32_t *) enemy_model + ENEMY_MODEL_INTS, ENEMY_MODEL_FIELDS);
    log_info_text(str_figure_model_loaded, NULL);
    return 1;
}

int32_t tax_sentiment_model_load(void)
{
    const char *path = tax_names[settings_data.difficulty];
    if (!load_model_file(path, str_no_figure_model_file)) {
        return 0;
    }
    int32_t limit = file_get_size(path);
    char *screen = spare_screen;
    if (!accept_count(count_braces(screen, limit), TAX_MODEL_BRACES, fmt_tax_sentiment_bad_lines, str_tax_sentiment_out_of_sync)) {
        return 0;
    }
    read_rows(screen, limit, tax_model, tax_model + TAX_MODEL_INTS, TAX_MODEL_FIELDS);
    log_info_text(str_tax_sentiment_loaded, NULL);
    return 1;
}
