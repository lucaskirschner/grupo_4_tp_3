/*
 * priority_queue.h
 *
 *  Created on: Aug 12, 2025
 *      Author: Grupo 4
 */

#ifndef INC_PRIORITY_QUEUE_H_
#define INC_PRIORITY_QUEUE_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "cmsis_os.h"

typedef enum {
    PQ_PRIO_HIGH = 0,   // Pulso
    PQ_PRIO_MED  = 1,   // Corto
    PQ_PRIO_LOW  = 2,   // Largo
    PQ_PRIO__N
} pq_prio_t;

typedef struct {
    pq_prio_t  prio;
    uint32_t   seq;            // lo completa el queue en push
    void      *payload;        // ej: (ao_led_job_t*)
    void     (*free_cb)(void*);// cómo liberar payload si se descarta por overflow
} pq_item_t;

// Cola de prioridad multiproductor/multiconsumidor
typedef struct prio_queue_t prio_queue_t;

// Estructura opaca para el manejador de la cola de prioridad
typedef struct PriorityQueueOpaque* PriorityQueueHandle_t;

// Crea una cola de prioridad con capacidad, tamaño de item y número de prioridades
PriorityQueueHandle_t xPriorityQueueCreateEx(size_t capacity, size_t item_size, uint8_t num_priorities);


static inline PriorityQueueHandle_t xPriorityQueueCreate(size_t capacity, size_t item_size) {
    return xPriorityQueueCreateEx(capacity, item_size, (uint8_t)PQ_PRIO__N);
}

// Elimina la cola de prioridad y libera todos los recursos
void vPriorityQueueDelete(PriorityQueueHandle_t h);

/* Envia un elemento: *ppItem debe apuntar a un mensaje cuyo primer campo sea 'pq_prio_t prio' */
BaseType_t xPriorityQueueSend(PriorityQueueHandle_t h, void * const *ppItem, TickType_t ticksToWait);

/* Recibe el elemento más prioritario y, si empatan, el más antiguo.
   Devuelve en *ppItem el puntero al mismo mensaje que fue enviado. */
BaseType_t xPriorityQueueReceive(PriorityQueueHandle_t h, void **ppItem, TickType_t ticksToWait);


#endif /* INC_PRIORITY_QUEUE_H_ */
