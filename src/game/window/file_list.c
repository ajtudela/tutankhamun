// The file list object (the names of a folder listing in one buffer, with their offsets) that holds the saved games,
// the player records and the maps: listing a pattern with buffers that grow as needed, sorting by name, and reading
// the player's record with the kept monument positions and the campaign's played links.
//
// A record from an older, shorter file leaves 24 bytes of the mission records from the previous read.

#include "window/file_list.h"
#include "core/crt_compat.h"
#include "core/string_compare.h"
#include "io/io_data.h"
#include "platform/platform_file.h"
#include "window/window_data.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

string_compare_fn file_list_sort_compare;

char *sort_names;

enum file_list_growth {
    FILE_LIST_NAMES_GROWTH = 1000,
    FILE_LIST_OFFSETS_GROWTH = 100,
};


bool file_list_scan(file_list *list, const char *pattern, int32_t *total_bytes, int32_t *total_count)
{
    platform_dir_listing listing;
    *total_bytes = 0;
    bool all = true;
    *total_count = 0;
    if (!platform_dir_first(&listing, pattern)) {
        return false;
    }
    for (;;) {
        int32_t size = (int32_t) strlen(listing.name) + 1;
        (*total_count)++;
        *total_bytes += size;
        if ((list->count < list->offsets_capacity) && ((list->names_used + size) < list->names_capacity)) {
            list->offsets[list->count] = list->names_used;
            memcpy(list->names + list->offsets[list->count], listing.name, (size_t) size);
            list->count++;
            list->names_used += size;
        } else {
            all = false;
        }
        if (!platform_dir_next(&listing)) {
            platform_dir_close(&listing);
            return all;
        }
    }
}

static int32_t file_list_grow_names(file_list *list, int32_t bytes, char keep)
{
    if (!keep) {
        if (list->names) {
            free(list->names);
            list->names = 0;
        }
        list->names_capacity = 0;
    }
    int32_t capacity = bytes + list->names_capacity + FILE_LIST_NAMES_GROWTH;
    if (capacity <= 0) {
        return 0;
    }
    char *names = malloc((size_t) capacity);
    if (!names) {
        return 0;
    }
    if (list->names) {
        memcpy(names, list->names, (size_t) list->names_used);
        free(list->names);
    }
    list->names = names;
    list->names_capacity = capacity;
    return 1;
}

static int32_t file_list_grow_offsets(file_list *list, int32_t count, char keep)
{
    if (!keep) {
        if (list->offsets) {
            free(list->offsets);
            list->offsets = 0;
        }
        list->offsets_capacity = 0;
    }
    int32_t capacity = count + list->offsets_capacity + FILE_LIST_OFFSETS_GROWTH;
    int32_t *offsets = malloc((size_t) capacity * sizeof(int32_t));
    if (!offsets) {
        return 0;
    }
    if (list->offsets) {
        for (int32_t i = 0; i < list->count; i++) {
            offsets[i] = list->offsets[i];
        }
        free(list->offsets);
    }
    list->offsets = offsets;
    list->offsets_capacity = capacity;
    return 1;
}

bool file_list_list_dir(file_list *list, const char *pattern, char keep)
{
    if (!keep) {
        list->count = 0;
        list->names_used = 0;
    }
    int32_t saved_count = list->count;
    int32_t saved_used = list->names_used;
    int32_t bytes, count;
    if (file_list_scan(list, pattern, &bytes, &count)) {
        return true;
    }
    for (;;) {
        list->count = saved_count;
        list->names_used = saved_used;
        if ((bytes <= list->names_capacity) && (count <= list->offsets_capacity)) {
            return false;
        }
        if (((bytes + saved_used) > list->names_capacity) && !file_list_grow_names(list, bytes, keep)) {
            return false;
        }
        if (((list->count + count) > list->offsets_capacity) && !file_list_grow_offsets(list, count, keep)) {
            return false;
        }
        if (file_list_scan(list, pattern, &bytes, &count)) {
            return true;
        }
    }
}

int file_list_compare(const void *a, const void *b)
{
    return file_list_sort_compare(sort_names + *(const int32_t *) a, sort_names + *(const int32_t *) b);
}

void file_list_sort(file_list *list, string_compare_fn compare)
{
    if ((list->offsets == 0) || (list->names == 0) || (list->count < 2)) {
        return;
    }
    file_list_sort_compare = compare;
    sort_names = list->names;
    crt_qsort(list->offsets, (uint32_t) list->count, sizeof(int32_t), file_list_compare);
}

const char *file_list_name(int32_t index)
{
    if ((index < 0) || (index >= file_list_saves.count)) {
        return 0;
    }
    return file_list_saves.names + file_list_saves.offsets[index];
}

void file_remove_extension(char *name)
{
    for (char *p = name;; p++) {
        if (*p == '.') {
            *p = 0;
            return;
        }
        if (*p == 0) {
            return;
        }
    }
}

void file_append_extension(char *name)
{
    char *p = name + strlen(name);
    p[0] = '.';
    p[1] = file_extension[0];
    p[2] = file_extension[1];
    p[3] = file_extension[2];
    p[4] = 0;
}

bool player_data_read_file(player_data *self, const char *file)
{
    static player_data file_record;
    campaign_construct(&file_record.campaign);
    memset(&self->monuments, 0, sizeof(self->monuments));
    FILE *f = fopen(file, file_mode_read_binary);
    if (!f) {
        return false;
    }
    size_t read = fread(&file_record, sizeof(player_data), 1, f);
    if (read == 0) {
        fseek(f, 0, SEEK_SET);
        read = fread(&file_record, PLAYER_DATA_OLD_SIZE, 1, f);
    } else {
        self->monuments = file_record.monuments;
    }
    fclose(f);
    if (read != 1) {
        return false;
    }
    campaign *own = &self->campaign;
    strcpy(own->saved_game_path, file_record.campaign.saved_game_path);
    memcpy(own->mission_records, file_record.campaign.mission_records, sizeof(own->mission_records));
    for (int32_t i = 0; i < own->mission_name_count; i++) {
        own->links[i].played = own->mission_records[i].used != 0;
    }
    return true;
}
