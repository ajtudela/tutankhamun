// The event phrase table of event_text.c (the token table and the text buffers the monument tokens print into) and
// the functions of event_text.c that other files call.

#ifndef TEXT_EVENT_TEXT_H
#define TEXT_EVENT_TEXT_H

#include "empire/message_token.h"

#include <stdint.h>

enum event_phrase_buffers {
    EVENT_PHRASE_BUFFERS = 10,
    EVENT_PHRASE_BUFFER_SIZE = 100,
};

typedef struct {
    const message_token_entry *tokens;
    char phrase_buffers[EVENT_PHRASE_BUFFERS][EVENT_PHRASE_BUFFER_SIZE];
    char courses_text[12];
    char percent_text[8];
} event_phrase_table;

extern event_phrase_table event_phrases;

void event_text_format(int32_t id, char *dst, int32_t max);
int32_t event_text_expand(const char *src, char *dst, int32_t max);
const char *message_token_time_allotted(void);
const char *message_token_travel_time(void);
const char *message_token_time_until_attack(void);
const char *message_token_reason_phrase(void);
const char *message_token_pharaoh_name(void);
const char *message_token_foreign_army(void);
const char *message_token_god(void);

#endif
