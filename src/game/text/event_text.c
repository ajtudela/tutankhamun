// Event message texts: the text of an EVENTMSG.TXT key, the expansion of its [token] placeholders through the token
// functions of the phrase table, and the token functions of this module (months to comply and left, the reason
// phrase, the pharaoh's name, the foreign army and the god of a message).
//
// A token matches the first table entry whose name starts with the bracket's text, and the foreign army's enemy type is
// read as a 32-bit value.

#include "text/event_text.h"

#include "city/city_data.h"
#include "city/message.h"
#include "core/static_init.h"
#include "platform/platform_file.h"
#include "scenario/event_helpers.h"
#include "text/lang.h"
#include "text/text_data.h"
#include "window/window_data.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

char message_reason_phrase_buffer[200];

char message_time_until_attack_buffer[8];

char message_travel_time_buffer[8];

char message_time_allotted_buffer[8];

const message_token_entry *current_token;

enum event_text_limits {
    REASON_PHRASE_BUFFER_SIZE = 200,
    FOREIGN_ARMY_TEXT_FIRST = 28,
    MESSAGE_GODS = 5,
};

event_phrase_table event_phrases;

const char *key_text_get(const key_text_table *self, int32_t index)
{
    if (index >= 0 && index < self->count) {
        return self->values[index];
    }
    return 0;
}

int32_t text_expand_tokens(const event_phrase_table *self, const char *src, char *dst, int32_t max)
{
    int32_t i = 0;
    int32_t o = 0;
    for (;;) {
        while (src[i] != '[') {
            if (o >= max) {
                return 0;
            }
            dst[o++] = src[i];
            if (src[i] == 0) {
                return 1;
            }
            i++;
        }
        int32_t start = i + 1;
        while (src[i] != ']') {
            if (src[i] == 0) {
                return 0;
            }
            i++;
        }
        const message_token_entry *entry = self->tokens;
        for (;; entry++) {
            if (entry->name == 0) {
                return 0;
            }
            if (platform_text_compare_nocase_n(entry->name, src + start, (uint32_t) (i - start)) == 0) {
                break;
            }
        }
        const message_token_entry *saved = current_token;
        current_token = entry;
        const char *text = entry->text();
        current_token = saved;
        while (text != 0 && *text != 0) {
            if (o >= max) {
                return 0;
            }
            dst[o++] = *text++;
        }
        i++;
    }
}

void event_text_format(int32_t id, char *dst, int32_t max)
{
    if (max < 1) {
        return;
    }
    dst[0] = 0;
    const char *text = key_text_get(&event_texts, id);
    if (text != 0) {
        text_expand_tokens(&event_phrases, text, dst, max);
    }
}

int32_t event_text_expand(const char *src, char *dst, int32_t max)
{
    dst[0] = 0;
    return text_expand_tokens(&event_phrases, src, dst, max);
}

static const city_message *shown_message(void)
{
    return &city_messages[city_message_shown_index];
}

const char *message_token_time_allotted(void)
{
    sprintf(message_time_allotted_buffer, "%d", (int32_t) shown_message()->phrases[EVENT_PHRASE_MONTHS_TO_COMPLY]);
    return message_time_allotted_buffer;
}
const char *message_token_travel_time(void)
{
    sprintf(message_travel_time_buffer, "%d", (int32_t) shown_message()->phrases[EVENT_PHRASE_MONTHS_TO_COMPLY]);
    return message_travel_time_buffer;
}

const char *message_token_time_until_attack(void)
{
    sprintf(message_time_until_attack_buffer, "%d", (int32_t) shown_message()->phrases[EVENT_PHRASE_MONTHS_LEFT]);
    return message_time_until_attack_buffer;
}

const char *message_token_reason_phrase(void)
{
    message_use_alternate = 1;
    event_text_format(shown_message()->phrases[EVENT_PHRASE_TEXT], message_reason_phrase_buffer, REASON_PHRASE_BUFFER_SIZE);
    message_use_alternate = 0;
    return message_reason_phrase_buffer;
}

const char *message_token_pharaoh_name(void)
{
    return lang_get_string(LANG_GROUP_PHARAOH_NAMES, scenario_data.pharaoh_name_id);
}
const char *message_token_foreign_army(void)
{
    return lang_get_string(LANG_GROUP_ENEMY_NAMES, (int32_t) ((uint16_t) scenario_data.enemy_type | ((uint32_t) (uint16_t) scenario_data.enemy_type_high << 16)) + FOREIGN_ARMY_TEXT_FIRST);
}

const char *message_token_god(void)
{
    const uint8_t *variants = (const uint8_t *) &shown_message()->phrases[EVENT_PHRASE_VARIANTS];
    uint8_t god = variants[message_use_alternate ? 1 : 0];
    if (god >= MESSAGE_GODS) {
        god = 0;
    }
    return lang_get_string(LANG_GROUP_GOD_NAMES, god);
}
