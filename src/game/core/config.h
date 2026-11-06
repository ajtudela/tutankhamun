// The gameplay configs: JSON files beside the program that override the built-in C tables of the game rules
// (prices, difficulty, salaries, building properties, ...). Each table is described once by a config_table (its
// file, the key field that picks a record, a config_field per column with its range and enum names), and that
// description loads, writes and checks the file.
//
// A file is an array of records; fields left out keep the built-in value, enum values are names or numbers, a
// column may be a list of element objects. A table is loaded whole or not at all: on any error the message names
// the file, record and field, and the built-in table stays.

#ifndef CORE_CONFIG_H
#define CORE_CONFIG_H

#include <stddef.h>
#include <stdint.h>

#define CONFIG_COUNT(array) ((int32_t) (sizeof(array) / sizeof((array)[0])))
#define CONFIG_TABLE_COLUMNS(fields, storage) fields, CONFIG_COUNT(fields), storage, CONFIG_COUNT(storage)

enum config_width {
    CONFIG_WIDTH_TEXT = -1,
};

typedef struct config_name {
    const char *name;
    int32_t value;
} config_name;

typedef struct config_list config_list;

typedef struct config_field {
    const char *name;
    int32_t *values;
    int32_t record_stride;
    int32_t length;
    int32_t min;
    int32_t max;
    const config_name *names;
    int32_t element_offset;
    int32_t width;
    const config_list *list;
} config_field;

struct config_list {
    const char *element;
    int32_t element_size;
    const config_field *fields;
    int32_t field_count;
    void *(*block)(int32_t record, int32_t *count);
    int32_t fixed_count;
};

typedef struct config_storage {
    void *data;
    size_t size;
} config_storage;

typedef struct config_table {
    const char *file;
    const char *key;
    const config_name *key_names;
    int32_t record_count;
    const config_field *fields;
    int32_t field_count;
    const config_storage *storage;
    int32_t storage_count;
    int32_t (*records_valid)(char *problem, size_t problem_size);
} config_table;

typedef enum config_load_result { CONFIG_LOAD_FAILED = -1, CONFIG_LOAD_MISSING = 0, CONFIG_LOAD_DONE = 1 } config_load_result;

config_load_result config_table_load(const config_table *table, const char *dir);
int32_t config_table_export(const config_table *table, const char *dir);
int32_t config_table_check(const config_table *table, const char *dir);

const config_table *const *config_tables(int32_t *count);
void config_load_all(const char *dir);
int32_t config_export_all(const char *dir);
int32_t config_check_all(const char *dir);

extern const config_name config_building_type_names[];
extern const config_name config_resource_names[];
extern const config_name config_figure_type_names[];
extern const config_name config_difficulty_names[];
extern const config_name config_anim_script_names[];
extern const config_name config_anim_images_names[];
extern const config_name config_anim_image_group_names[];
extern const config_name config_flotsam_kind_names[];
extern const config_name config_direction_names[];
extern const config_name config_overlay_names[];
extern const config_name config_overlay_tile_names[];
extern const config_name config_monument_def_names[];
extern const config_name config_event_type_names[];

#endif
