// The file list (file_list.c): listing and sorting a folder, the file name helpers and the player's record read.

#ifndef WINDOW_FILE_LIST_H
#define WINDOW_FILE_LIST_H

#include "core/static_init.h"
#include "core/string_compare.h"
#include "game/player_record.h"
#include "window/window_state_types.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

bool file_list_scan(file_list *list, const char *pattern, int32_t *total_bytes, int32_t *total_count);
int file_list_compare(const void *a, const void *b);
bool file_list_list_dir(file_list *list, const char *pattern, char keep);
void file_list_sort(file_list *list, string_compare_fn compare);
bool player_data_read_file(player_data *self, const char *file);
const char *file_list_name(int32_t index);
void file_remove_extension(char *name);
void file_append_extension(char *name);

#endif
