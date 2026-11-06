// A doubly linked list kept in order of an integer key (core/sorted_list.c), each link pointing at its item: the flood
// tiles of a flood depth and the groups of an SG file while it loads.

#ifndef CORE_SORTED_LIST_H
#define CORE_SORTED_LIST_H

#include <stdint.h>

typedef struct sorted_link {
    struct sorted_link *next;
    struct sorted_link *prev;
    int32_t key;
    void *item;
} sorted_link;

typedef struct {
    sorted_link *head;
    sorted_link *tail;
} sorted_list;

void sorted_list_insert(sorted_list *list, sorted_link *link);
void sorted_list_unlink(sorted_list *list, sorted_link *link);

#endif
