/*
 * priority_queue_core.c
 *
 *  Created on: Aug 18, 2025
 *      Author: Eze
 */


#include "priority_queue_core.h"
#include <stdlib.h>
#include <string.h>

bool pqc_init(priority_queue_core_t *pq, size_t capacity) {

	if (!pq || capacity == 0) return false;

    for (int i = 0; i < PQ_PRIO__N; i++) {
        ll_init(&pq->queues[i]);
    }

    pq->total_size = 0;
    pq->capacity = capacity;
    pq->seq_counter = 0;

    return true;
}

static pq_item_t* create_item_copy(const pq_item_t *src) {
    pq_item_t *copy = malloc(sizeof(pq_item_t));
    if (!copy) return NULL;
    *copy = *src;
    return copy;
}

static void discard_oldest(priority_queue_core_t *pq) {

    int oldest_queue = -1;
    uint32_t min_seq = UINT32_MAX;

    // Buscar el elemento más antiguo en todas las colas
    for (int i = 0; i < PQ_PRIO__N; i++) {
    	// Se mira el primer elemento de cada cola
        pq_item_t *item = (pq_item_t*)ll_peek_front(&pq->queues[i]);

        // Si hay elemento y es mas viejo que mi candidato actual
        if (item && item->seq < min_seq) {
            min_seq = item->seq;
            oldest_queue = i;
        }
    }
    // Si encontré algo que descartar
    if (oldest_queue >= 0) {
    	// Sacar el elemento más antiguo de la cola correspondiente
        pq_item_t *discarded = (pq_item_t*)ll_pop_front(&pq->queues[oldest_queue]);

        if (discarded) {
        	// Si tiene callback de liberación, lo llamo
            if (discarded->free_cb && discarded->payload) {
                discarded->free_cb(discarded->payload);
            }
            // Liberar el espacio del elemento descartado
            free(discarded);
            pq->total_size--; // Reducir el tamaño total de la cola
        }
    }
}

bool pqc_push(priority_queue_core_t *pq, pq_item_t *item) {
    if (!pq || !item || item->prio >= PQ_PRIO__N) return false;

    // Asignar secuencia
    item->seq = pq->seq_counter++;

    // Verificar capacidad
    if (pq->total_size >= pq->capacity) {
        discard_oldest(pq);
    }

    // Crear copia y agregar a la cola correspondiente
    pq_item_t *copy = create_item_copy(item);
    if (!copy) return false;

    if (!ll_push_back(&pq->queues[item->prio], copy)) {
        free(copy);
        return false;
    }

    pq->total_size++;
    return true;
}

bool pqc_pop(priority_queue_core_t *pq, pq_item_t *out_item) {
    if (!pq || !out_item) return false;

    // Buscar en orden de prioridad: HIGH > MED > LOW
    for (int i = 0; i < PQ_PRIO__N; i++) {
        if (!ll_is_empty(&pq->queues[i])) {
            pq_item_t *item = (pq_item_t*)ll_pop_front(&pq->queues[i]);
            if (item) {
                *out_item = *item;
                free(item);
                pq->total_size--;
                return true;
            }
        }
    }

    return false;
}

bool pqc_is_empty(priority_queue_core_t *pq) {
    return (!pq || pq->total_size == 0);
}

bool pqc_is_full(priority_queue_core_t *pq) {
    return (pq && pq->total_size >= pq->capacity);
}

size_t pqc_size(priority_queue_core_t *pq) {
    return pq ? pq->total_size : 0;
}

void pqc_destroy(priority_queue_core_t *pq) {
    if (!pq) return;

    for (int i = 0; i < PQ_PRIO__N; i++) {
        ll_clear(&pq->queues[i], free);
    }

    pq->total_size = 0;
}
