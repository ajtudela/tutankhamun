// The tables of io/io_data.c.

#ifndef IO_IO_DATA_H
#define IO_IO_DATA_H

#include "core/text_key.h"
#include "empire/message_token.h"
#include "figure/dock_queue_types.h"
#include "figure/figure_core.h"
#include "graphics/button.h"
#include "io/chunk_io.h"
#include "sound/city_sound_types.h"

#include <stdint.h>

extern const char file_mode_read_binary[4];

extern const char str_removed_manmades[24];

extern const char str_pharaoh_inf[12];

extern const char empire_file_name_cleopatra[];

extern const char empire_file_name[];

extern const char *str_pharaoh_emp;

extern const char *str_pharaoh2_emp;

extern const char file_mode_read_update_binary[8];

typedef struct {
    message_token_entry entries[27];
    uint32_t reserved;
} message_token_table;
extern message_token_table message_tokens;

extern text_key event_text_keys[602];

typedef struct event_text_key_name_pool event_text_key_name_pool;
extern event_text_key_name_pool event_text_key_names;

typedef struct message_token_name_pool message_token_name_pool;
extern message_token_name_pool message_token_names;

extern const char file_extension_555[4];

extern const char image_555_directory[20];

extern chunk_converter savegame_converter_scenario_events[2];

extern chunk_converter savegame_converter_messages[2];

extern chunk_entry savegame_chunk_table[181];

extern char scenario_file_magic[8];

extern char scenario_file_magic_expected[12];

extern chunk_entry scenario_chunk_table[24];

extern uint32_t zip_dictionary_size;

extern int32_t pk_tables_reference[835];

extern city_sound_table city_sounds;

extern int32_t scenario_climate_current;

extern int32_t scenario_start_flags[8];

extern dock_queue_table dock_queues;

extern int32_t city_exit_flag_offset;

extern int32_t city_entry_flag_offset;

extern int32_t city_entry_flag_x;

extern int32_t city_entry_flag_y;

extern int32_t city_exit_flag_x;

extern int32_t city_exit_flag_y;

extern figure_name_counter_record figure_name_counter_table;


extern int32_t g_footprint_data_off;

extern chunk_schema_table savegame_chunk_schema;

extern int32_t savegame_file_version;

extern int32_t savegame_schema_chunk_count;

extern int32_t buildings_without_id_cleared;

extern uint32_t zip_compress_type;

#endif
