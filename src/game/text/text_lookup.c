// Language strings: lang_get_string gives the index-th string of a group of the loaded text, the chosen language's
// (text/lang_pack.c) when it has one; lang_group_string_count counts a group's strings in the game's own text file,
// where a group ends at the next group's first string.

#include "text/text_lookup.h"

#include "text/lang.h"
#include "text/lang_pack.h"
#include "text/text_data.h"

#include <stdint.h>

static const char *file_string(int32_t group, int32_t index)
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

const char *lang_get_string(int32_t group, int32_t index)
{
    const char *translated = lang_pack_string(group, index);
    if (translated) {
        return translated;
    }
    return file_string(group, index);
}

int32_t lang_group_string_count(int32_t group)
{
    const char *end = file_string(group + 1, 0);
    int32_t count = 1;
    while (file_string(group, count) != end) {
        count++;
    }
    return count;
}
