// The built-in data of the texts. text/text_data.h declares them.

#include "text/text_data.h"

#include <assert.h>

key_text_table event_texts;

uint8_t message_use_alternate;

uint8_t dialog_has_phrases;

static_assert(sizeof(lang_message_file_index) == 80024, "lang_message_index is the language message file's index of 80024 bytes");
lang_message_file_index lang_message_index;

static_assert(sizeof(lang_text_group_entry) * 1000 == 8000, "lang_text_index is the language text file's index of 8000 bytes");
lang_text_group_entry lang_text_index[1000];

uint8_t *lang_text_base;

uint8_t *lang_message_data;

monument_text_resources bi_monument_resources;
