// The records of the language's message index (the MM file): per message its dialog layout, its kind, its fanfare
// and the offsets of its texts, after the file's header.

#ifndef TEXT_LANG_MESSAGE_TYPES_H
#define TEXT_LANG_MESSAGE_TYPES_H

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

enum lang_message_dialog {
    LANG_MESSAGE_DIALOG_MANUAL = 0,
    LANG_MESSAGE_DIALOG_CITY_MESSAGE = 2,
};

enum lang_message_type {
    LANG_MESSAGE_TYPE_GENERAL = 0,
    LANG_MESSAGE_TYPE_DISASTER = 1,
    LANG_MESSAGE_TYPE_IMPERIAL = 2,
    LANG_MESSAGE_TYPE_EMIGRATION = 3,
    LANG_MESSAGE_TYPE_TUTORIAL = 4,
    LANG_MESSAGE_TYPE_TRADE_CHANGE = 5,
    LANG_MESSAGE_TYPE_PRICE_CHANGE = 6,
    LANG_MESSAGE_TYPE_INVASION = 7,
};

enum lang_message_table_size {
    LANG_MESSAGE_COUNT = 1000,
    LANG_MESSAGE_HEADER_SIZE = 24,
};

typedef struct {
    int16_t type;
    int16_t message_type;
    int16_t reserved_1;
    int16_t x;
    int16_t y;
    int16_t width_blocks;
    int16_t height_blocks;
    int16_t image_id;
    int16_t image_x;
    int16_t image_y;
    int16_t second_image_id;
    int16_t second_image_x;
    int16_t second_image_y;
    int16_t title_x;
    int16_t title_y;
    int16_t subtitle_x;
    int16_t subtitle_y;
    int16_t reserved_2[2];
    int16_t video_x;
    int16_t video_y;
    int16_t reserved_3[7];
    int32_t urgent;
    int32_t video_text;
    int32_t reserved_4;
    int32_t title_text;
    int32_t subtitle_text;
    int32_t content_text;
} lang_message;

typedef struct {
    uint8_t header[LANG_MESSAGE_HEADER_SIZE];
    lang_message messages[LANG_MESSAGE_COUNT];
} lang_message_file_index;

static_assert(sizeof(lang_message) == 0x50, "lang_message is a language message file record of 80 bytes");
static_assert(offsetof(lang_message, message_type) == 0x02 && offsetof(lang_message, title_x) == 0x1A && offsetof(lang_message, video_x) == 0x26 &&
                  offsetof(lang_message, urgent) == 0x38 && offsetof(lang_message, video_text) == 0x3C && offsetof(lang_message, title_text) == 0x44,
              "lang_message is the language message file's layout");

#endif
