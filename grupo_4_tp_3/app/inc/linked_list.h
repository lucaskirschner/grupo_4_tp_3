/*
 * linked_list.h
 *
 *  Created on: Aug 18, 2025
 *      Author: Eze
 */

#ifndef INC_LINKED_LIST_H_
#define INC_LINKED_LIST_H_

#include <stddef.h>
#include <stdbool.h>

typedef struct ll_node {
    void *data;
    struct ll_node *next;
} ll_node_t;

typedef struct {
    ll_node_t *head;
    ll_node_t *tail;
    size_t size;
} linked_list_t;

// API de lista enlazada - sin dependencias del OS
void ll_init(linked_list_t *list);
bool ll_push_back(linked_list_t *list, void *data);
void* ll_pop_front(linked_list_t *list);
void* ll_peek_front(linked_list_t *list);
bool ll_is_empty(linked_list_t *list);
size_t ll_size(linked_list_t *list);
void ll_clear(linked_list_t *list, void (*free_fn)(void*));

#endif /* INC_LINKED_LIST_H_ */
