// A "key "text"" table loaded from a text file such as EVENTMSG.TXT: its keys, the text of each and the file's bytes.

#ifndef CORE_KEY_TEXT_TYPES_H
#define CORE_KEY_TEXT_TYPES_H

#include "core/text_key.h"

#include <stdint.h>

typedef struct {
    const text_key *keys;
    char **values;
    char *data;
    int32_t count;
    int32_t spare_3;
} key_text_table;

#endif
