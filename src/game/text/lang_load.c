// Loading the language: the text file (Pharaoh_Text.eng) and the message file (Pharaoh_MM.eng), each a fixed header
// and index followed by the data. Returns 1 when both were read, 0 when one does not open.

#include "text/lang_load.h"

#include "platform/platform_file.h"
#include "text/lang.h"
#include "text/text_data.h"

#include <stdint.h>
#include <stdlib.h>

uint8_t lang_text_header[28];

enum lang_file_sizes {
    TEXT_HEADER_SIZE = 28,
    TEXT_INDEX_SIZE = LANG_TEXT_GROUPS * sizeof(lang_text_group_entry),
    MESSAGE_INDEX_SIZE = 80024,
};

int32_t lang_load(const char *text_file, const char *message_file)
{
    int32_t fd = platform_file_open(text_file, PLATFORM_OPEN_READ);
    if (fd == -1) {
        return 0;
    }
    int32_t size = (int32_t) platform_file_length(fd) - (TEXT_HEADER_SIZE + TEXT_INDEX_SIZE);
    if (lang_text_base) {
        free(lang_text_base);
        lang_text_base = 0;
    }
    lang_text_base = malloc((size_t) size);
    platform_file_read(fd, lang_text_header, TEXT_HEADER_SIZE);
    platform_file_read(fd, lang_text_index, TEXT_INDEX_SIZE);
    platform_file_read(fd, lang_text_base, (uint32_t) size);
    platform_file_close(fd);

    fd = platform_file_open(message_file, PLATFORM_OPEN_READ);
    if (fd == -1) {
        return 0;
    }
    size = (int32_t) platform_file_length(fd) - MESSAGE_INDEX_SIZE;
    if (lang_message_data) {
        free(lang_message_data);
        lang_message_data = 0;
    }
    lang_message_data = malloc((size_t) size);
    platform_file_read(fd, &lang_message_index, MESSAGE_INDEX_SIZE);
    platform_file_read(fd, lang_message_data, (uint32_t) size);
    platform_file_close(fd);
    return 1;
}
