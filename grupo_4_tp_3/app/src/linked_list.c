/*
 * linked_list.c
 *
 *  Created on: Aug 18, 2025
 *      Author: Eze
 */

#include "linked_list.h"
#include <stdlib.h>

void ll_init(linked_list_t *list) {
    if (!list) return;
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
}

bool ll_push_back(linked_list_t *list, void *data) {
    if (!list || !data) return false;

    ll_node_t *node = malloc(sizeof(ll_node_t));
    if (!node) return false;

    node->data = data;
    node->next = NULL;

    if (!list->head) {
        list->head = list->tail = node;
    } else {
        list->tail->next = node;
        list->tail = node;
    }

    list->size++;
    return true;
}

void* ll_pop_front(linked_list_t *list) {
    if (!list || !list->head) return NULL;

    ll_node_t *node = list->head;
    void *data = node->data;

    list->head = node->next;
    if (!list->head) list->tail = NULL;

    free(node);
    list->size--;
    return data;
}

void* ll_peek_front(linked_list_t *list) {
    if (!list || !list->head) return NULL;
    return list->head->data;
}

bool ll_is_empty(linked_list_t *list) {
    return (!list || list->size == 0);
}

size_t ll_size(linked_list_t *list) {
    return list ? list->size : 0;
}

void ll_clear(linked_list_t *list, void (*free_fn)(void*)) {
    if (!list) return;

    while (!ll_is_empty(list)) {
        void *data = ll_pop_front(list);
        if (free_fn && data) free_fn(data);
    }
}
