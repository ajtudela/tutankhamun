// The token table of the message texts: each token (greeting, player name and the others), its number and the
// function that gives its text.

#ifndef EMPIRE_MESSAGE_TOKEN_H
#define EMPIRE_MESSAGE_TOKEN_H

#include <stdint.h>

typedef const char *(*message_token_function)(void);

typedef struct {
    int32_t id;
    const char *name;
    message_token_function text;
} message_token_entry;

#endif
