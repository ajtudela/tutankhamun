// The tables of text/text_data.c.

#ifndef TEXT_TEXT_DATA_H
#define TEXT_TEXT_DATA_H

#include "core/key_text_types.h"
#include "text/lang.h"
#include "text/lang_message_types.h"
#include "window/building_info_monuments.h"

#include <stdint.h>

extern key_text_table event_texts;

extern uint8_t message_use_alternate;

extern uint8_t dialog_has_phrases;

extern lang_message_file_index lang_message_index;

extern lang_text_group_entry lang_text_index[1000];

extern uint8_t *lang_text_base;

extern uint8_t *lang_message_data;

extern monument_text_resources bi_monument_resources;

#endif
