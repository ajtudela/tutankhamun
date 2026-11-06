// The sorted list: a link goes in front of the first link whose key is not smaller than its own, so it goes before
// links of an equal key; unlinking clears the link's neighbours.

#include "core/sorted_list.h"

#include <stddef.h>

void sorted_list_insert(sorted_list *list, sorted_link *link)
{
    if (!list->head || list->head->key >= link->key) {
        link->next = list->head;
        if (list->head) {
            list->head->prev = link;
        }
        link->prev = NULL;
        list->head = link;
        if (!list->tail) {
            list->tail = link;
        }
    } else if (list->tail->key <= link->key) {
        link->prev = list->tail;
        list->tail->next = link;
        link->next = NULL;
        list->tail = link;
    } else {
        sorted_link *at = list->head;
        while (at->key < link->key) {
            at = at->next;
        }
        link->next = at;
        link->prev = at->prev;
        at->prev->next = link;
        at->prev = link;
    }
}

void sorted_list_unlink(sorted_list *list, sorted_link *link)
{
    if (link->prev) {
        link->prev->next = link->next;
    } else {
        list->head = link->next;
    }
    if (link->next) {
        link->next->prev = link->prev;
    } else {
        list->tail = link->prev;
    }
    link->next = NULL;
    link->prev = NULL;
}
