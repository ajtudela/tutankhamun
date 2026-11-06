// Language strings: lang_get_string gives the index-th string of a group of the loaded text.

#include "text/text_lookup.h"

#include "text/lang.h"
#include "text/text_data.h"

#include <stdint.h>

const char *lang_get_string(int32_t group, int32_t index)
{
    const lang_text_group_entry *groups = lang_text_index;
    const uint8_t *p = lang_text_base + groups[group].offset;
    while (index > 0) {
        if (*p == 0 && (p[-1] >= 0x20 || p[-1] == 0)) {
            index--;
        }
        p++;
    }
    while (*p < 0x20) {
        p++;
    }
    return (const char *) p;
}
