// Loading, writing and checking the gameplay configs (core/config.h). A table's JSON file is read straight into
// its C table after a snapshot of everything the table can change; any invalid record, field or element puts the
// snapshot back. The writer prints the C table as the same JSON, and the check loads a file over a poisoned table
// and compares the result with the built-in bytes.

#include "core/config.h"

#include "platform/log.h"
#include "third_party/parson/parson.h"

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum config_limits {
    CONFIG_PATH_SIZE = 520,
    CONFIG_MESSAGE_SIZE = 400,
    CONFIG_KEY_TEXT_SIZE = 64,
    CONFIG_POISON = 0x5A5A5A5A,
    CONFIG_TEXT_POISON = 0x5A,
};

typedef struct config_record_context {
    const char *path;
    int32_t index;
} config_record_context;

typedef struct config_span {
    uint8_t *data;
    size_t size;
    uint8_t *copy;
    int32_t record;
    const char *field;
} config_span;

typedef struct config_snapshot {
    config_span *spans;
    int32_t count;
} config_snapshot;

static void config_report(const char *format, ...)
{
    char message[CONFIG_MESSAGE_SIZE];
    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);
    platform_log("%s", message);
    fprintf(stderr, "%s\n", message);
}

static void config_path(char *path, const char *dir, const char *file)
{
    snprintf(path, CONFIG_PATH_SIZE, "%s/%s", dir, file);
}

static int32_t config_file_exists(const char *path)
{
    FILE *file = fopen(path, "rb");
    if (!file) {
        return 0;
    }
    fclose(file);
    return 1;
}

static const char *config_name_of(const config_name *names, int32_t value)
{
    for (const config_name *entry = names; entry && entry->name; entry++) {
        if (entry->value == value) {
            return entry->name;
        }
    }
    return 0;
}

static int32_t config_value_of_name(const config_name *names, const char *name, int32_t *value)
{
    for (const config_name *entry = names; entry && entry->name; entry++) {
        if (strcmp(entry->name, name) == 0) {
            *value = entry->value;
            return 1;
        }
    }
    return 0;
}

static void config_key_text(char *text, const config_name *names, int32_t key)
{
    const char *name = config_name_of(names, key);
    if (name) {
        snprintf(text, CONFIG_KEY_TEXT_SIZE, "%s", name);
    } else {
        snprintf(text, CONFIG_KEY_TEXT_SIZE, "%d", key);
    }
}

static uint8_t *config_value_address(const config_field *field, uint8_t *element, int32_t record)
{
    if (element) {
        return element + field->element_offset;
    }
    return (uint8_t *) (field->values + record * field->record_stride);
}

static int32_t config_get(const config_field *field, const uint8_t *address, int32_t index)
{
    if (field->width == 1) {
        return ((const int8_t *) address)[index];
    }
    return ((const int32_t *) address)[index];
}

static void config_set(const config_field *field, uint8_t *address, int32_t index, int32_t value)
{
    if (field->width == 1) {
        ((int8_t *) address)[index] = (int8_t) value;
    } else {
        ((int32_t *) address)[index] = value;
    }
}

static uint8_t *config_list_block(const config_field *field, uint8_t *element, int32_t record, int32_t *count)
{
    if (element) {
        *count = field->list->fixed_count;
        return element + field->element_offset;
    }
    return field->list->block(record, count);
}

static int32_t config_list_owner(const config_field *field, int32_t record)
{
    int32_t count;
    void *block = field->list->block(record, &count);
    for (int32_t earlier = 0; earlier < record; earlier++) {
        if (field->list->block(earlier, &count) == block) {
            return earlier;
        }
    }
    return -1;
}

static int32_t config_read_int(const JSON_Value *json, const config_name *names, int32_t min, int32_t max, int32_t *value, char *problem)
{
    if (json_value_get_type(json) == JSONString) {
        const char *name = json_value_get_string(json);
        if (!names) {
            snprintf(problem, CONFIG_MESSAGE_SIZE, "\"%s\": a number is expected", name);
            return 0;
        }
        if (!config_value_of_name(names, name, value)) {
            snprintf(problem, CONFIG_MESSAGE_SIZE, "\"%s\" is not a known name", name);
            return 0;
        }
    } else if (json_value_get_type(json) == JSONNumber) {
        double number = json_value_get_number(json);
        if (number != floor(number) || number < -2147483648.0 || number > 2147483647.0) {
            snprintf(problem, CONFIG_MESSAGE_SIZE, "%g is not a whole number", number);
            return 0;
        }
        *value = (int32_t) number;
    } else {
        snprintf(problem, CONFIG_MESSAGE_SIZE, "a number%s is expected", names ? " or a name" : "");
        return 0;
    }
    if (*value < min || *value > max) {
        snprintf(problem, CONFIG_MESSAGE_SIZE, "%d is outside %d..%d", *value, min, max);
        return 0;
    }
    return 1;
}

static int32_t config_read_text(const config_field *field, uint8_t *address, const JSON_Value *json, char *problem)
{
    const char *text = json_value_get_string(json);
    if (!text) {
        snprintf(problem, CONFIG_MESSAGE_SIZE, "a string is expected");
        return 0;
    }
    size_t length = strlen(text);
    if (length >= (size_t) field->length) {
        snprintf(problem, CONFIG_MESSAGE_SIZE, "the string is %u characters long, the table holds %d", (uint32_t) length, field->length - 1);
        return 0;
    }
    memset(address, 0, (size_t) field->length);
    memcpy(address, text, length);
    return 1;
}

static int32_t config_read_element(const config_list *list, uint8_t *element, const JSON_Object *object, char *problem);

static int32_t config_read_list(const config_list *list, uint8_t *block, int32_t count, const JSON_Value *json, char *problem)
{
    const JSON_Array *array = json_value_get_array(json);
    if (!array) {
        snprintf(problem, CONFIG_MESSAGE_SIZE, "an array of %s objects is expected", list->element);
        return 0;
    }
    int32_t given = (int32_t) json_array_get_count(array);
    if (given > count) {
        snprintf(problem, CONFIG_MESSAGE_SIZE, "%d %ss given, the table holds %d", given, list->element, count);
        return 0;
    }
    for (int32_t i = 0; i < given; i++) {
        char element_problem[CONFIG_MESSAGE_SIZE];
        const JSON_Object *object = json_array_get_object(array, (size_t) i);
        if (!object) {
            snprintf(problem, CONFIG_MESSAGE_SIZE, "%s %d: an object is expected", list->element, i);
            return 0;
        }
        if (!config_read_element(list, block + (i * list->element_size), object, element_problem)) {
            snprintf(problem, CONFIG_MESSAGE_SIZE, "%s %d: %s", list->element, i, element_problem);
            return 0;
        }
    }
    return 1;
}

static int32_t config_read_field(const config_field *field, uint8_t *element, int32_t record, const JSON_Value *json, char *problem)
{
    if (field->list) {
        int32_t count;
        uint8_t *block = config_list_block(field, element, record, &count);
        return config_read_list(field->list, block, count, json, problem);
    }
    uint8_t *address = config_value_address(field, element, record);
    if (field->width == CONFIG_WIDTH_TEXT) {
        return config_read_text(field, address, json, problem);
    }
    int32_t value;
    if (field->length == 1) {
        if (!config_read_int(json, field->names, field->min, field->max, &value, problem)) {
            return 0;
        }
        config_set(field, address, 0, value);
        return 1;
    }
    const JSON_Array *array = json_value_get_array(json);
    if (!array || (int32_t) json_array_get_count(array) != field->length) {
        snprintf(problem, CONFIG_MESSAGE_SIZE, "an array of %d values is expected", field->length);
        return 0;
    }
    for (int32_t i = 0; i < field->length; i++) {
        char element_problem[CONFIG_MESSAGE_SIZE];
        if (!config_read_int(json_array_get_value(array, (size_t) i), field->names, field->min, field->max, &value, element_problem)) {
            snprintf(problem, CONFIG_MESSAGE_SIZE, "value %d: %s", i, element_problem);
            return 0;
        }
        config_set(field, address, i, value);
    }
    return 1;
}

static const config_field *config_field_named(const config_field *fields, int32_t field_count, const char *name)
{
    for (int32_t f = 0; f < field_count; f++) {
        if (strcmp(fields[f].name, name) == 0) {
            return &fields[f];
        }
    }
    return 0;
}

static int32_t config_read_element(const config_list *list, uint8_t *element, const JSON_Object *object, char *problem)
{
    size_t count = json_object_get_count(object);
    for (size_t i = 0; i < count; i++) {
        char field_problem[CONFIG_MESSAGE_SIZE];
        const char *name = json_object_get_name(object, i);
        const config_field *field = config_field_named(list->fields, list->field_count, name);
        if (!field) {
            snprintf(problem, CONFIG_MESSAGE_SIZE, "unknown field \"%s\"", name);
            return 0;
        }
        if (!config_read_field(field, element, 0, json_object_get_value_at(object, i), field_problem)) {
            snprintf(problem, CONFIG_MESSAGE_SIZE, "field \"%s\": %s", name, field_problem);
            return 0;
        }
    }
    return 1;
}

static int32_t config_read_record(const config_table *table, const config_record_context *context, const JSON_Object *record, uint8_t *seen)
{
    char problem[CONFIG_MESSAGE_SIZE];
    const JSON_Value *key_json = json_object_get_value(record, table->key);
    if (!key_json) {
        config_report("config: %s: record %d: the key \"%s\" is missing", context->path, context->index, table->key);
        return 0;
    }
    int32_t key;
    if (!config_read_int(key_json, table->key_names, 0, table->record_count - 1, &key, problem)) {
        config_report("config: %s: record %d: field \"%s\": %s", context->path, context->index, table->key, problem);
        return 0;
    }
    if (seen[key]) {
        config_report("config: %s: record %d: %s %d is given twice", context->path, context->index, table->key, key);
        return 0;
    }
    seen[key] = 1;
    char key_text[CONFIG_KEY_TEXT_SIZE];
    config_key_text(key_text, table->key_names, key);
    size_t count = json_object_get_count(record);
    for (size_t i = 0; i < count; i++) {
        const char *name = json_object_get_name(record, i);
        if (strcmp(name, table->key) == 0) {
            continue;
        }
        const config_field *field = config_field_named(table->fields, table->field_count, name);
        if (!field) {
            config_report("config: %s: record %d (%s %s): unknown field \"%s\"", context->path, context->index, table->key, key_text, name);
            return 0;
        }
        int32_t owner = field->list ? config_list_owner(field, key) : -1;
        if (owner >= 0) {
            char owner_text[CONFIG_KEY_TEXT_SIZE];
            config_key_text(owner_text, table->key_names, owner);
            config_report("config: %s: record %d (%s %s): field \"%s\": these are the %s of %s %s, give them there", context->path, context->index, table->key,
                key_text, name, name, table->key, owner_text);
            return 0;
        }
        if (!config_read_field(field, 0, key, json_object_get_value_at(record, i), problem)) {
            config_report("config: %s: record %d (%s %s): field \"%s\": %s", context->path, context->index, table->key, key_text, name, problem);
            return 0;
        }
    }
    return 1;
}

static int32_t config_read_records(const config_table *table, const char *path, const JSON_Value *root)
{
    const JSON_Array *records = json_value_get_array(root);
    if (!records) {
        config_report("config: %s: an array of records is expected", path);
        return 0;
    }
    uint8_t *seen = calloc((size_t) table->record_count, 1);
    int32_t ok = 1;
    size_t count = json_array_get_count(records);
    for (size_t i = 0; ok && i < count; i++) {
        config_record_context context = { path, (int32_t) i };
        const JSON_Object *record = json_array_get_object(records, i);
        if (!record) {
            config_report("config: %s: record %d: an object is expected", path, (int32_t) i);
            ok = 0;
        } else {
            ok = config_read_record(table, &context, record, seen);
        }
    }
    free(seen);
    return ok;
}

static int32_t config_list_field_count(const config_table *table)
{
    int32_t count = 0;
    for (int32_t f = 0; f < table->field_count; f++) {
        count += table->fields[f].list != 0;
    }
    return count;
}

static config_snapshot config_snapshot_take(const config_table *table)
{
    config_snapshot snapshot;
    int32_t capacity = table->storage_count + table->record_count * config_list_field_count(table);
    snapshot.spans = calloc((size_t) (capacity > 0 ? capacity : 1), sizeof(config_span));
    snapshot.count = 0;
    for (int32_t s = 0; s < table->storage_count; s++) {
        config_span *span = &snapshot.spans[snapshot.count++];
        span->data = table->storage[s].data;
        span->size = table->storage[s].size;
        span->record = -1;
        span->field = 0;
    }
    for (int32_t r = 0; r < table->record_count; r++) {
        for (int32_t f = 0; f < table->field_count; f++) {
            const config_field *field = &table->fields[f];
            if (!field->list) {
                continue;
            }
            int32_t count;
            config_span *span = &snapshot.spans[snapshot.count++];
            span->data = field->list->block(r, &count);
            span->size = (size_t) count * (size_t) field->list->element_size;
            span->record = r;
            span->field = field->name;
        }
    }
    for (int32_t s = 0; s < snapshot.count; s++) {
        config_span *span = &snapshot.spans[s];
        span->copy = malloc(span->size > 0 ? span->size : 1);
        memcpy(span->copy, span->data, span->size);
    }
    return snapshot;
}

static void config_snapshot_restore(const config_snapshot *snapshot)
{
    for (int32_t s = 0; s < snapshot->count; s++) {
        memcpy(snapshot->spans[s].data, snapshot->spans[s].copy, snapshot->spans[s].size);
    }
}

static void config_snapshot_free(config_snapshot *snapshot)
{
    for (int32_t s = 0; s < snapshot->count; s++) {
        free(snapshot->spans[s].copy);
    }
    free(snapshot->spans);
    snapshot->spans = 0;
    snapshot->count = 0;
}

config_load_result config_table_load(const config_table *table, const char *dir)
{
    char path[CONFIG_PATH_SIZE];
    config_path(path, dir, table->file);
    if (!config_file_exists(path)) {
        return CONFIG_LOAD_MISSING;
    }
    JSON_Value *root = json_parse_file_with_comments(path);
    if (!root) {
        config_report("config: %s: not valid JSON; the built-in table is kept", path);
        return CONFIG_LOAD_FAILED;
    }
    config_snapshot snapshot = config_snapshot_take(table);
    int32_t ok = config_read_records(table, path, root);
    if (ok && table->records_valid) {
        char problem[CONFIG_MESSAGE_SIZE];
        if (!table->records_valid(problem, sizeof(problem))) {
            config_report("config: %s: %s", path, problem);
            ok = 0;
        }
    }
    if (ok) {
        platform_log("config: %s loaded", path);
    } else {
        config_snapshot_restore(&snapshot);
        config_report("config: %s: the built-in table is kept", path);
    }
    config_snapshot_free(&snapshot);
    json_value_free(root);
    return ok ? CONFIG_LOAD_DONE : CONFIG_LOAD_FAILED;
}

static void config_write_int(FILE *file, const config_name *names, int32_t value)
{
    const char *name = config_name_of(names, value);
    if (name) {
        fprintf(file, "\"%s\"", name);
    } else {
        fprintf(file, "%d", value);
    }
}

static void config_write_text(FILE *file, const char *text)
{
    fputc('"', file);
    for (const char *c = text; *c; c++) {
        if (*c == '"' || *c == '\\') {
            fputc('\\', file);
        }
        fputc(*c, file);
    }
    fputc('"', file);
}

static void config_write_element(FILE *file, const config_list *list, uint8_t *element);

static void config_write_value(FILE *file, const config_field *field, uint8_t *element, int32_t record)
{
    if (field->list) {
        int32_t count;
        uint8_t *block = config_list_block(field, element, record, &count);
        fputc('[', file);
        for (int32_t i = 0; i < count; i++) {
            fputs(i > 0 ? ", " : "", file);
            config_write_element(file, field->list, block + (i * field->list->element_size));
        }
        fputc(']', file);
        return;
    }
    const uint8_t *address = config_value_address(field, element, record);
    if (field->width == CONFIG_WIDTH_TEXT) {
        config_write_text(file, (const char *) address);
        return;
    }
    if (field->length == 1) {
        config_write_int(file, field->names, config_get(field, address, 0));
        return;
    }
    fputc('[', file);
    for (int32_t i = 0; i < field->length; i++) {
        fputs(i > 0 ? ", " : "", file);
        config_write_int(file, field->names, config_get(field, address, i));
    }
    fputc(']', file);
}

static void config_write_element(FILE *file, const config_list *list, uint8_t *element)
{
    fputs("{ ", file);
    for (int32_t f = 0; f < list->field_count; f++) {
        fprintf(file, "%s\"%s\": ", f > 0 ? ", " : "", list->fields[f].name);
        config_write_value(file, &list->fields[f], element, 0);
    }
    fputs(" }", file);
}

static void config_write_record_list(FILE *file, const config_field *field, int32_t record)
{
    int32_t count;
    uint8_t *block = field->list->block(record, &count);
    fprintf(file, ",\n        \"%s\": [\n", field->name);
    for (int32_t i = 0; i < count; i++) {
        fputs("            ", file);
        config_write_element(file, field->list, block + (i * field->list->element_size));
        fputs(i + 1 < count ? ",\n" : "\n", file);
    }
    fputs("        ]", file);
}

int32_t config_table_export(const config_table *table, const char *dir)
{
    char path[CONFIG_PATH_SIZE];
    config_path(path, dir, table->file);
    FILE *file = fopen(path, "w");
    if (!file) {
        config_report("config: cannot write %s", path);
        return 0;
    }
    fprintf(file, "// Written by tutankhamun --export-configs from the built-in table; edit to change the game.\n");
    fprintf(file, "// A record or a field left out keeps the built-in value.\n[\n");
    for (int32_t r = 0; r < table->record_count; r++) {
        fprintf(file, "    { \"%s\": ", table->key);
        config_write_int(file, table->key_names, r);
        for (int32_t f = 0; f < table->field_count; f++) {
            const config_field *field = &table->fields[f];
            if (!field->list) {
                fprintf(file, ", \"%s\": ", field->name);
                config_write_value(file, field, 0, r);
            } else if (config_list_owner(field, r) < 0) {
                config_write_record_list(file, field, r);
            }
        }
        fprintf(file, " }%s\n", r + 1 < table->record_count ? "," : "");
    }
    fprintf(file, "]\n");
    int32_t ok = fclose(file) == 0;
    if (!ok) {
        config_report("config: cannot write %s", path);
    }
    return ok;
}

static void config_poison_fields(const config_field *fields, int32_t field_count, uint8_t *element, int32_t record)
{
    for (int32_t f = 0; f < field_count; f++) {
        const config_field *field = &fields[f];
        if (field->list) {
            int32_t count;
            uint8_t *block = config_list_block(field, element, record, &count);
            for (int32_t i = 0; i < count; i++) {
                config_poison_fields(field->list->fields, field->list->field_count, block + i * field->list->element_size, 0);
            }
            continue;
        }
        uint8_t *address = config_value_address(field, element, record);
        if (field->width == CONFIG_WIDTH_TEXT) {
            memset(address, CONFIG_TEXT_POISON, (size_t) field->length);
            continue;
        }
        for (int32_t i = 0; i < field->length; i++) {
            config_set(field, address, i, CONFIG_POISON);
        }
    }
}

static int32_t config_snapshot_equal(const config_table *table, const config_snapshot *built_in)
{
    int32_t equal = 1;
    for (int32_t s = 0; s < built_in->count; s++) {
        const config_span *span = &built_in->spans[s];
        for (size_t offset = 0; offset < span->size; offset++) {
            if (span->data[offset] == span->copy[offset]) {
                continue;
            }
            if (span->field) {
                char key_text[CONFIG_KEY_TEXT_SIZE];
                config_key_text(key_text, table->key_names, span->record);
                config_report("config check: %s: %s %s: \"%s\" differs from the built-in table at byte %u", table->file, table->key, key_text, span->field,
                    (uint32_t) offset);
            } else {
                config_report("config check: %s: storage %d differs from the built-in table at byte %u", table->file, s, (uint32_t) offset);
            }
            equal = 0;
            break;
        }
    }
    return equal;
}

int32_t config_table_check(const config_table *table, const char *dir)
{
    config_snapshot built_in = config_snapshot_take(table);
    for (int32_t r = 0; r < table->record_count; r++) {
        config_poison_fields(table->fields, table->field_count, 0, r);
    }
    config_load_result loaded = config_table_load(table, dir);
    int32_t ok = loaded == CONFIG_LOAD_DONE && config_snapshot_equal(table, &built_in);
    if (loaded == CONFIG_LOAD_MISSING) {
        config_report("config check: %s/%s is missing", dir, table->file);
    }
    config_snapshot_restore(&built_in);
    config_snapshot_free(&built_in);
    return ok;
}

void config_load_all(const char *dir)
{
    int32_t count;
    const config_table *const *tables = config_tables(&count);
    for (int32_t t = 0; t < count; t++) {
        config_table_load(tables[t], dir);
    }
}

int32_t config_export_all(const char *dir)
{
    int32_t count;
    int32_t ok = 1;
    const config_table *const *tables = config_tables(&count);
    for (int32_t t = 0; t < count; t++) {
        ok &= config_table_export(tables[t], dir);
    }
    return ok;
}

int32_t config_check_all(const char *dir)
{
    int32_t count;
    int32_t failed = 0;
    const config_table *const *tables = config_tables(&count);
    for (int32_t t = 0; t < count; t++) {
        int32_t ok = config_table_check(tables[t], dir);
        printf("%s: %s\n", tables[t]->file, ok ? "identical to the built-in table" : "FAILED");
        failed += !ok;
    }
    printf("%d of %d configs identical to the built-in tables\n", count - failed, count);
    return failed == 0;
}
