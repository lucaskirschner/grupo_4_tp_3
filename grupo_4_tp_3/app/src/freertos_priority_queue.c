/*
 * freertos_priority_queue.c
 *
 *  Created on: Aug 18, 2025
 *      Author: Eze
 */


#include "freertos_priority_queue.h"
#include "task.h"

struct freertos_pq_opaque {
    priority_queue_core_t core;
    SemaphoreHandle_t mutex;
    SemaphoreHandle_t items_sem;
    size_t item_size;
};

PriorityQueueHandle_t xPriorityQueueCreate(size_t capacity, size_t item_size) {
    if (item_size != sizeof(void*)) return NULL;

    struct freertos_pq_opaque *handle = pvPortMalloc(sizeof(*handle));
    if (!handle) return NULL;

    if (!pqc_init(&handle->core, capacity)) {
        vPortFree(handle);
        return NULL;
    }

    // Crear sincronización de FreeRTOS
    handle->mutex = xSemaphoreCreateMutex();
    if (!handle->mutex) {
        vPortFree(handle);
        return NULL;
    }

    handle->items_sem = xSemaphoreCreateCounting((UBaseType_t)capacity, 0); // Contador de elementos disponibles
    if (!handle->items_sem) {
        vSemaphoreDelete(handle->mutex);
        vPortFree(handle);
        return NULL;
    }

    handle->item_size = item_size;
    return handle;
}

void vPriorityQueueDelete(PriorityQueueHandle_t handle) {
    if (!handle) return;

    xSemaphoreTake(handle->mutex, portMAX_DELAY);
    pqc_destroy(&handle->core);
    xSemaphoreGive(handle->mutex);

    vSemaphoreDelete(handle->items_sem);
    vSemaphoreDelete(handle->mutex);
    vPortFree(handle);
}

BaseType_t xPriorityQueueSend(PriorityQueueHandle_t handle, void * const *ppItem, TickType_t ticksToWait) {
    if (!handle || !ppItem || !*ppItem) return errQUEUE_FULL;


    typedef struct { pq_priority_t prio; } msg_header_t;
    msg_header_t *hdr = (msg_header_t*)(*ppItem);


    pq_item_t item = {
        .prio = hdr->prio,
        .seq = 0,	// se asigna automáticamente al hacer push
        .payload = *ppItem,
        .free_cb = vPortFree
    };

    if (xSemaphoreTake(handle->mutex, ticksToWait) != pdPASS) {
        return errQUEUE_FULL;
    }

    bool success = pqc_push(&handle->core, &item);
    xSemaphoreGive(handle->mutex);

    if (success) {
        xSemaphoreGive(handle->items_sem);
        return pdPASS;
    }

    return errQUEUE_FULL;
}

BaseType_t xPriorityQueueReceive(PriorityQueueHandle_t handle, void **ppItem, TickType_t ticksToWait) {
    if (!handle || !ppItem) return errQUEUE_EMPTY;

    // Esperar elemento disponible
    if (xSemaphoreTake(handle->items_sem, ticksToWait) != pdPASS) {
        return errQUEUE_EMPTY;
    }

    // Tomar mutex con el mismo timeout para consistencia
    if (xSemaphoreTake(handle->mutex, ticksToWait) != pdPASS) {

        xSemaphoreGive(handle->items_sem);
        return errQUEUE_EMPTY;
    }

    pq_item_t item;
    bool success = pqc_pop(&handle->core, &item);
    xSemaphoreGive(handle->mutex);

    if (success) {
        *ppItem = item.payload;
        return pdPASS;
    }

    return errQUEUE_EMPTY;
}
