// Language packs: the texts of the game's translated editions, made from their own text, message and event files by
// tools/lang_to_json.py and kept as config/lang/<code>.json ("texts": the text file's strings by group and index,
// "messages": the message file's titles, subtitles and contents by message, "events": EVENTMSG.TXT by key). The English
// texts of the game's files stay loaded; a pack lays its texts over them:
// - the text file's strings by group and index (lang_get_string asks lang_pack_string first);
// - the message file's titles, subtitles and contents: the pack's texts are appended to a copy of the message data
//   and the message index points at them (the English index and data are kept to go back);
// - the event messages by their keys.
// A text the pack does not have, or has empty, stays English. The packs are UTF-8; the game's fonts have Latin-1, so
// the texts are turned into it: Hungarian o and u with a double acute become the umlauted letters, the Polish letters
// the plain ones, typographic quotes, dashes and the ellipsis plain ones, anything else '?'.

#include "text/lang_pack.h"

#include "graphics/ui_layout.h"
#include "platform/language_store.h"
#include "host/status_log.h"
#include "text/lang.h"
#include "text/lang_message_types.h"
#include "text/text_data.h"
#include "third_party/parson/parson.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    const char *code;
    const char *name;
    const char *menu_text;
} lang_pack_info;

// the languages' own names and the Options menu's "Language" in the game's characters
static const lang_pack_info pack_infos[] = {
    { "en", "English", "Language" },
    { "de", "Deutsch", "Sprache" },
    { "it", "Italiano", "Lingua" },
    { "es", "Espa\xF1ol", "Idioma" },
    { "pl", "Polski", "Jezyk" },
};

enum lang_pack_limits {
    PACK_INFOS = (int32_t) (sizeof(pack_infos) / sizeof(pack_infos[0])),
    PACK_PATH_SIZE = 1200,
    MESSAGE_FIELDS = 3,
    MESSAGE_FIELD_TITLE = 0,
    MESSAGE_FIELD_SUBTITLE = 1,
    MESSAGE_FIELD_CONTENT = 2,
};

static const char *const message_field_names[MESSAGE_FIELDS] = { "title", "subtitle", "content" };

typedef struct {
    int32_t a;
    int32_t b;
    size_t offset;
} pack_entry;

typedef struct {
    pack_entry *items;
    int32_t count;
    int32_t capacity;
} pack_entries;

typedef struct {
    char *data;
    size_t size;
    size_t capacity;
} pack_pool;

typedef struct {
    pack_pool pool;
    pack_entries texts;
    pack_entries messages;
    pack_entries events;
    const char **group_strings[LANG_TEXT_GROUPS];
    int32_t group_counts[LANG_TEXT_GROUPS];
} lang_pack;

static lang_pack active;
static int32_t current_info;
static int32_t available[PACK_INFOS];
static int32_t available_count;
static int32_t requested_info = -1;

static uint8_t *english_message_data;
static int32_t english_message_offsets[LANG_MESSAGE_COUNT][MESSAGE_FIELDS];
static uint8_t *pack_message_data;
static char **english_event_values;
static int32_t english_event_count;

// --- storage ---

static int32_t pool_reserve(pack_pool *pool, size_t more)
{
    if (pool->size + more <= pool->capacity) {
        return 1;
    }
    size_t capacity = pool->capacity ? pool->capacity : 65536;
    while (capacity < pool->size + more) {
        capacity *= 2;
    }
    char *data = realloc(pool->data, capacity);
    if (!data) {
        return 0;
    }
    pool->data = data;
    pool->capacity = capacity;
    return 1;
}

static int32_t pool_put(pack_pool *pool, char c)
{
    if (!pool_reserve(pool, 1)) {
        return 0;
    }
    pool->data[pool->size++] = c;
    return 1;
}

static int32_t entries_add(pack_entries *entries, int32_t a, int32_t b, size_t offset)
{
    if (entries->count == entries->capacity) {
        int32_t capacity = entries->capacity ? entries->capacity * 2 : 1024;
        pack_entry *items = realloc(entries->items, (size_t) capacity * sizeof(*items));
        if (!items) {
            return 0;
        }
        entries->items = items;
        entries->capacity = capacity;
    }
    pack_entry *entry = &entries->items[entries->count++];
    entry->a = a;
    entry->b = b;
    entry->offset = offset;
    return 1;
}

static void pack_free(lang_pack *pack)
{
    free(pack->pool.data);
    free(pack->texts.items);
    free(pack->messages.items);
    free(pack->events.items);
    for (int32_t group = 0; group < LANG_TEXT_GROUPS; group++) {
        free((void *) pack->group_strings[group]);
    }
    memset(pack, 0, sizeof(*pack));
}

// --- characters ---

static uint32_t utf8_next(const uint8_t **p)
{
    const uint8_t *s = *p;
    uint32_t c = *s++;
    int32_t more = c >= 0xF0 ? 3 : c >= 0xE0 ? 2 : c >= 0xC0 ? 1 : 0;
    if (c >= 0x80 && more == 0) {
        *p = s;
        return '?';
    }
    if (more) {
        c &= 0x3F >> more;
    }
    while (more-- > 0) {
        if ((*s & 0xC0) != 0x80) {
            *p = s;
            return '?';
        }
        c = (c << 6) | (*s++ & 0x3F);
    }
    *p = s;
    return c;
}

// the game's characters for a code point the fonts do not have
static const char *latin_replacement(uint32_t c)
{
    switch (c) {
        case 0x0150:
            return "\xD6";
        case 0x0151:
            return "\xF6";
        case 0x0170:
            return "\xDC";
        case 0x0171:
            return "\xFC";
        case 0x0104:
            return "A";
        case 0x0105:
            return "a";
        case 0x0106:
            return "C";
        case 0x0107:
            return "c";
        case 0x0118:
            return "E";
        case 0x0119:
            return "e";
        case 0x0141:
            return "L";
        case 0x0142:
            return "l";
        case 0x0143:
            return "N";
        case 0x0144:
            return "n";
        case 0x015A:
            return "S";
        case 0x015B:
            return "s";
        case 0x0179:
        case 0x017B:
            return "Z";
        case 0x017A:
        case 0x017C:
            return "z";
        case 0x0152:
            return "OE";
        case 0x0153:
            return "oe";
        case 0x2018:
        case 0x2019:
        case 0x201A:
            return "'";
        case 0x201C:
        case 0x201D:
        case 0x201E:
            return "\"";
        case 0x2013:
        case 0x2014:
            return "-";
        case 0x2026:
            return "...";
        default:
            return "?";
    }
}

// the UTF-8 text in the game's characters at the end of the pool with its terminator
static int32_t pool_add_string(pack_pool *pool, const char *text, size_t *offset)
{
    *offset = pool->size;
    const uint8_t *p = (const uint8_t *) text;
    while (*p) {
        uint32_t c = utf8_next(&p);
        if (c < 0x80 || (c >= 0xA0 && c <= 0xFF)) {
            if (!pool_put(pool, (char) c)) {
                return 0;
            }
            continue;
        }
        for (const char *r = latin_replacement(c); *r; r++) {
            if (!pool_put(pool, *r)) {
                return 0;
            }
        }
    }
    return pool_put(pool, 0);
}

// --- the pack file ---

static int32_t pack_path(char *path, size_t size, const char *code)
{
    char dir[1024];
    if (!language_store_config_dir(dir, sizeof(dir))) {
        return 0;
    }
    snprintf(path, size, "%s/lang/%s.json", dir, code);
    return 1;
}

static int32_t read_number(const char *name, int32_t *value)
{
    char *end;
    long number = strtol(name, &end, 10);
    if (end == name || *end != 0 || number < 0) {
        return 0;
    }
    *value = (int32_t) number;
    return 1;
}

static int32_t read_texts(lang_pack *pack, const JSON_Object *texts)
{
    for (size_t i = 0; texts && i < json_object_get_count(texts); i++) {
        int32_t group;
        const JSON_Array *strings = json_value_get_array(json_object_get_value_at(texts, i));
        if (!read_number(json_object_get_name(texts, i), &group) || group >= LANG_TEXT_GROUPS || !strings) {
            continue;
        }
        for (size_t index = 0; index < json_array_get_count(strings); index++) {
            const char *text = json_array_get_string(strings, index);
            size_t offset;
            if (!text || !text[0]) {
                continue;
            }
            if (!pool_add_string(&pack->pool, text, &offset) || !entries_add(&pack->texts, group, (int32_t) index, offset)) {
                return 0;
            }
        }
    }
    return 1;
}

static int32_t read_messages(lang_pack *pack, const JSON_Object *messages)
{
    for (size_t i = 0; messages && i < json_object_get_count(messages); i++) {
        int32_t message;
        const JSON_Object *fields = json_value_get_object(json_object_get_value_at(messages, i));
        if (!read_number(json_object_get_name(messages, i), &message) || message >= LANG_MESSAGE_COUNT || !fields) {
            continue;
        }
        for (int32_t f = 0; f < MESSAGE_FIELDS; f++) {
            const char *text = json_object_get_string(fields, message_field_names[f]);
            size_t offset;
            if (!text || !text[0]) {
                continue;
            }
            if (!pool_add_string(&pack->pool, text, &offset) || !entries_add(&pack->messages, message, f, offset)) {
                return 0;
            }
        }
    }
    return 1;
}

static int32_t names_equal_nocase(const char *a, const char *b)
{
    for (; *a && *b; a++, b++) {
        char x = (*a >= 'A' && *a <= 'Z') ? (char) (*a - 'A' + 'a') : *a;
        char y = (*b >= 'A' && *b <= 'Z') ? (char) (*b - 'A' + 'a') : *b;
        if (x != y) {
            return 0;
        }
    }
    return *a == *b;
}

static int32_t event_key_index(const char *key)
{
    for (int32_t i = 0; event_texts.keys && i < event_texts.count; i++) {
        if (names_equal_nocase(event_texts.keys[i].name, key)) {
            return i;
        }
    }
    return -1;
}

static int32_t read_events(lang_pack *pack, const JSON_Object *events)
{
    for (size_t i = 0; events && i < json_object_get_count(events); i++) {
        int32_t key = event_key_index(json_object_get_name(events, i));
        const char *text = json_value_get_string(json_object_get_value_at(events, i));
        size_t offset;
        if (key < 0 || !text || !text[0]) {
            continue;
        }
        if (!pool_add_string(&pack->pool, text, &offset) || !entries_add(&pack->events, key, 0, offset)) {
            return 0;
        }
    }
    return 1;
}

static int32_t index_texts(lang_pack *pack)
{
    for (int32_t i = 0; i < pack->texts.count; i++) {
        const pack_entry *e = &pack->texts.items[i];
        if (e->b + 1 > pack->group_counts[e->a]) {
            pack->group_counts[e->a] = e->b + 1;
        }
    }
    for (int32_t group = 0; group < LANG_TEXT_GROUPS; group++) {
        if (pack->group_counts[group] > 0) {
            pack->group_strings[group] = calloc((size_t) pack->group_counts[group], sizeof(const char *));
            if (!pack->group_strings[group]) {
                return 0;
            }
        }
    }
    for (int32_t i = 0; i < pack->texts.count; i++) {
        const pack_entry *e = &pack->texts.items[i];
        pack->group_strings[e->a][e->b] = pack->pool.data + e->offset;
    }
    return 1;
}

static int32_t pack_load(lang_pack *pack, const char *code)
{
    memset(pack, 0, sizeof(*pack));
    char path[PACK_PATH_SIZE];
    JSON_Value *root = pack_path(path, sizeof(path), code) ? json_parse_file(path) : 0;
    const JSON_Object *object = json_value_get_object(root);
    int32_t ok = object && read_texts(pack, json_object_get_object(object, "texts")) && read_messages(pack, json_object_get_object(object, "messages")) &&
        read_events(pack, json_object_get_object(object, "events")) && index_texts(pack);
    json_value_free(root);
    if (!ok) {
        log_info("language: the pack does not load", path, 0);
        pack_free(pack);
    }
    return ok;
}

// --- the game's tables ---

static int32_t *message_field(lang_message *message, int32_t field)
{
    return field == MESSAGE_FIELD_TITLE ? &message->title_text : field == MESSAGE_FIELD_SUBTITLE ? &message->subtitle_text : &message->content_text;
}

// the English message data and index as lang_load read them (again after another lang_load)
static void keep_english_messages(void)
{
    if (!lang_message_data || lang_message_data == english_message_data || lang_message_data == pack_message_data) {
        return;
    }
    english_message_data = lang_message_data;
    pack_message_data = 0;
    for (int32_t m = 0; m < LANG_MESSAGE_COUNT; m++) {
        for (int32_t f = 0; f < MESSAGE_FIELDS; f++) {
            english_message_offsets[m][f] = *message_field(&lang_message_index.messages[m], f);
        }
    }
}

static void apply_messages(const lang_pack *pack)
{
    keep_english_messages();
    if (!english_message_data) {
        return;
    }
    for (int32_t m = 0; m < LANG_MESSAGE_COUNT; m++) {
        for (int32_t f = 0; f < MESSAGE_FIELDS; f++) {
            *message_field(&lang_message_index.messages[m], f) = english_message_offsets[m][f];
        }
    }
    lang_message_data = english_message_data;
    free(pack_message_data);
    pack_message_data = 0;
    if (!pack || pack->messages.count == 0) {
        return;
    }
    size_t size = (size_t) lang_message_data_size;
    for (int32_t i = 0; i < pack->messages.count; i++) {
        size += strlen(pack->pool.data + pack->messages.items[i].offset) + 1;
    }
    uint8_t *data = malloc(size);
    if (!data) {
        return;
    }
    memcpy(data, english_message_data, (size_t) lang_message_data_size);
    size_t at = (size_t) lang_message_data_size;
    for (int32_t i = 0; i < pack->messages.count; i++) {
        const pack_entry *e = &pack->messages.items[i];
        // a subtitle only where the English message has one: the dialogs draw what the index points at
        if (e->b == MESSAGE_FIELD_SUBTITLE && english_message_offsets[e->a][e->b] == 0) {
            continue;
        }
        const char *text = pack->pool.data + e->offset;
        size_t length = strlen(text) + 1;
        memcpy(data + at, text, length);
        *message_field(&lang_message_index.messages[e->a], e->b) = (int32_t) at;
        at += length;
    }
    pack_message_data = data;
    lang_message_data = data;
}

static void apply_events(const lang_pack *pack)
{
    if (!event_texts.values) {
        return;
    }
    if (!english_event_values || english_event_count != event_texts.count) {
        free(english_event_values);
        english_event_count = event_texts.count;
        english_event_values = malloc((size_t) english_event_count * sizeof(char *));
        if (!english_event_values) {
            english_event_count = 0;
            return;
        }
        memcpy(english_event_values, event_texts.values, (size_t) english_event_count * sizeof(char *));
    }
    memcpy(event_texts.values, english_event_values, (size_t) english_event_count * sizeof(char *));
    for (int32_t i = 0; pack && i < pack->events.count; i++) {
        const pack_entry *e = &pack->events.items[i];
        if (e->a < event_texts.count) {
            event_texts.values[e->a] = pack->pool.data + e->offset;
        }
    }
}

// --- the languages ---

static int32_t pack_present(const char *code)
{
    char path[PACK_PATH_SIZE];
    if (!pack_path(path, sizeof(path), code)) {
        return 0;
    }
    FILE *file = fopen(path, "rb");
    if (file) {
        fclose(file);
    }
    return file != 0;
}

static void find_available(void)
{
    if (available_count) {
        return;
    }
    available[available_count++] = 0;
    for (int32_t i = 1; i < PACK_INFOS; i++) {
        if (pack_present(pack_infos[i].code)) {
            available[available_count++] = i;
        }
    }
}

int32_t lang_pack_count(void)
{
    find_available();
    return available_count;
}

const char *lang_pack_name(int32_t index)
{
    find_available();
    return index >= 0 && index < available_count ? pack_infos[available[index]].name : "";
}

int32_t lang_pack_current(void)
{
    find_available();
    for (int32_t i = 0; i < available_count; i++) {
        if (available[i] == current_info) {
            return i;
        }
    }
    return 0;
}

static int32_t switch_to(int32_t info)
{
    lang_pack loaded;
    if (info != 0 && !pack_load(&loaded, pack_infos[info].code)) {
        return 0;
    }
    pack_free(&active);
    if (info != 0) {
        active = loaded;
    }
    current_info = info;
    apply_messages(info != 0 ? &active : 0);
    apply_events(info != 0 ? &active : 0);
    ui_layout_texts_changed();
    log_info("language:", pack_infos[info].code, active.texts.count);
    return 1;
}

int32_t lang_pack_select(int32_t index)
{
    find_available();
    if (index < 0 || index >= available_count) {
        return 0;
    }
    if (!switch_to(available[index])) {
        return 0;
    }
    language_store_write(pack_infos[current_info].code);
    return 1;
}

static int32_t info_of_code(const char *code)
{
    for (int32_t i = 0; i < PACK_INFOS; i++) {
        if (strcmp(code, pack_infos[i].code) == 0) {
            return i;
        }
    }
    return -1;
}

int32_t lang_pack_request(const char *code)
{
    requested_info = info_of_code(code);
    return requested_info >= 0;
}

const char *lang_pack_codes(void)
{
    return "en, de, it, es, pl";
}

void lang_pack_startup(void)
{
    int32_t info = requested_info;
    if (info < 0) {
        char code[16];
        language_store_read(code, sizeof(code));
        info = info_of_code(code);
    }
    if (info > 0) {
        switch_to(info);
    }
}

const char *lang_pack_string(int32_t group, int32_t index)
{
    if (group == LANG_GROUP_MENU_OPTIONS && index == LANG_PACK_MENU_TEXT) {
        return pack_infos[current_info].menu_text;
    }
    if (group < 0 || group >= LANG_TEXT_GROUPS || index < 0 || index >= active.group_counts[group]) {
        return 0;
    }
    return active.group_strings[group][index];
}
