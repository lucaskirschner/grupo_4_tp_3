/*
 * priority_queue_core.h
 *
 *  Created on: Aug 18, 2025
 *      Author: Eze
 */

#ifndef INC_PRIORITY_QUEUE_CORE_H_
#define INC_PRIORITY_QUEUE_CORE_H_

#include "linked_list.h"
#include <stdint.h>

typedef enum {
    PQ_PRIO_HIGH = 0,	// Pulso
    PQ_PRIO_MED  = 1,	// Corto
    PQ_PRIO_LOW  = 2,	// Largo
    PQ_PRIO__N   = 3
} pq_priority_t;

typedef struct {
    pq_priority_t prio;
    uint32_t seq;
    void *payload;
    void (*free_cb)(void*);
} pq_item_t;

typedef struct {
    linked_list_t queues[PQ_PRIO__N];
    size_t total_size;
    size_t capacity;
    uint32_t seq_counter;
} priority_queue_core_t;

// API baremetal - sin dependencias del OS
bool pqc_init(priority_queue_core_t *pq, size_t capacity);
bool pqc_push(priority_queue_core_t *pq, pq_item_t *item);
bool pqc_pop(priority_queue_core_t *pq, pq_item_t *out_item);
bool pqc_is_empty(priority_queue_core_t *pq);
bool pqc_is_full(priority_queue_core_t *pq);
size_t pqc_size(priority_queue_core_t *pq);
void pqc_destroy(priority_queue_core_t *pq);

#endif /* INC_PRIORITY_QUEUE_CORE_H_ */
