// The key of a "key "text"" table such as EVENTMSG.TXT's: the text's id and the name its line starts with. A key
// table ends with a key whose name is null.

#ifndef CORE_TEXT_KEY_H
#define CORE_TEXT_KEY_H

#include <stdint.h>

typedef struct {
    int32_t id;
    const char *name;
} text_key;

#endif
