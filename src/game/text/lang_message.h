// A message of the language's message index (the MM file) by its type.

#ifndef TEXT_LANG_MESSAGE_H
#define TEXT_LANG_MESSAGE_H

#include "text/lang_message_types.h"
#include "text/text_data.h"

#include <stdint.h>

static inline lang_message *lang_message_get(int32_t type)
{
    return &lang_message_index.messages[type];
}

#endif
