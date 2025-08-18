/*
 * priority_queue.c
 *
 *  Created on: Aug 12, 2025
 *      Author: Grupo 4
 */

#include "priority_queue.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "logger.h"

// Nodo interno
typedef struct pq_node {
    pq_item_t        item;
    struct pq_node  *next;
} pq_node_t;

// FIFO por prioridad
typedef struct {
    pq_node_t *head;
    pq_node_t *tail;
    size_t     size;
} pq_fifo_t;

// Estructura principal de la cola de prioridad
struct prio_queue_t {
    pq_fifo_t        fifos[PQ_PRIO__N];
    size_t           size;       	// elementos totales
    size_t           capacity;  	 // límite global
    uint32_t         seq_counter;	// Contador para asignar "antigüedad"
    SemaphoreHandle_t mutex;     	// Mutex para proteger acceso concurrente
    SemaphoreHandle_t items_sem; 	// Semaforo contador de elementos disponibles
};

// Estructura opaca para el manejador de la cola de prioridad
struct PriorityQueueOpaque {
    prio_queue_t q;
    size_t       item_size;
    uint8_t      num_prios;
};

// Prototipos de funciones internas
bool pq_init   (prio_queue_t *q, size_t capacity);
bool pq_push   (prio_queue_t *q, pq_item_t *in_item);   // copia el metadato; NO roba payload
bool pq_pop    (prio_queue_t *q, pq_item_t *out_item, TickType_t timeout);
void pq_destroy(prio_queue_t *q);


// Inserta un nodo a una FIFO
static inline void fifo_push(pq_fifo_t *f, pq_node_t *n){
    n->next = NULL;
    if(!f->head){ f->head = f->tail = n; }
    else { f->tail->next = n; f->tail = n; }
    f->size++;
}

// Saca un nodo de una FIFO
static inline pq_node_t* fifo_pop(pq_fifo_t *f){
    if(!f->head) return NULL;
    pq_node_t *n = f->head;
    f->head = n->next;
    if(!f->head) f->tail = NULL;
    f->size--;
    n->next = NULL;
    return n;
}

// Solo consulta el frente de una FIFPO sin sacarlo
static inline pq_node_t* fifo_peek(pq_fifo_t *f){ return f->head; }


// Inicializa la cola de prioridad
bool pq_init(prio_queue_t *q, size_t capacity){
    if(!q || capacity == 0) return false;
    // Inicializar todas las FIFOs
    for(int i=0;i<PQ_PRIO__N;i++){ q->fifos[i].head=q->fifos[i].tail=NULL; q->fifos[i].size=0; }
    q->size = 0;
    q->capacity = capacity;
    q->seq_counter = 0;
    q->mutex = xSemaphoreCreateMutex(); // Crear mutex para acceso exclusivo
    if(!q->mutex) return false;
    q->items_sem = xSemaphoreCreateCounting((UBaseType_t)capacity, 0); // Contador de elementos disponibles
    if(!q->items_sem){ vSemaphoreDelete(q->mutex); return false; }
    return true;
}

// Reserva un nodo y copia dentro el item que llega
static pq_node_t* alloc_node_copy(const pq_item_t *src){
    pq_node_t *n = (pq_node_t*)pvPortMalloc(sizeof(pq_node_t));
    if(!n) return NULL;
    n->item = *src; // copia metadatos (payload ptr incluido)
    n->next = NULL;
    return n;
}

// Descarta el nodo más antiguo de la cola, sin considerar las prioridades
static void discard_oldest_locked(prio_queue_t *q){

    pq_node_t *cand[PQ_PRIO__N] = { fifo_peek(&q->fifos[PQ_PRIO_HIGH]),
                                    fifo_peek(&q->fifos[PQ_PRIO_MED ]),
                                    fifo_peek(&q->fifos[PQ_PRIO_LOW ]) };
    int idx = -1;
    uint32_t minseq = 0;
    for(int i=0;i<PQ_PRIO__N;i++){
        if(cand[i]){
            if(idx < 0 || cand[i]->item.seq < minseq){ idx = i; minseq = cand[i]->item.seq; }
        }
    }
    if(idx >= 0){
    	// Sacar el nodo más antiguo de la FIFO correspondiente
        pq_node_t *old = fifo_pop(&q->fifos[idx]);
        q->size--;

 	   // Liberar el payload si tiene callback
        if(old->item.free_cb && old->item.payload){ old->item.free_cb(old->item.payload); }
        vPortFree(old);
    }
}

// Inserta un nuevo item en la cola de prioridad
bool pq_push(prio_queue_t *q, pq_item_t *in_item){
    if(!q || !in_item) return false;

    //Asignar un número de secuencia (antiguedad) y crear un nodo
    pq_item_t temp = *in_item;
    temp.seq = q->seq_counter++;
    pq_node_t *node = alloc_node_copy(&temp);
    if(!node) return false;


    xSemaphoreTake(q->mutex, portMAX_DELAY);
    // Verificar si la cola está llena
    if(q->size >= q->capacity){
        // descartar el más antiguo global
        discard_oldest_locked(q);
    }

    // Insertar el nodo en la FIFO correspondiente
    fifo_push(&q->fifos[temp.prio], node);
    q->size++;
    xSemaphoreGive(q->mutex);

    xSemaphoreGive(q->items_sem); // nuevo elemento disponible
    return true;
}

// Saca el item más prioritario de la cola, respetando las prioridades
bool pq_pop(prio_queue_t *q, pq_item_t *out_item, TickType_t timeout){
    if(!q || !out_item) return false;

    // Esperar hasta que haya al menos un elemento disponible
    if(xSemaphoreTake(q->items_sem, timeout) != pdPASS){
        return false; // timeout
    }
    xSemaphoreTake(q->mutex, portMAX_DELAY);
    pq_node_t *n = NULL;

    // prioridad: HIGH > MED > LOW
    if(!n) n = fifo_pop(&q->fifos[PQ_PRIO_HIGH]);
    if(!n) n = fifo_pop(&q->fifos[PQ_PRIO_MED ]);
    if(!n) n = fifo_pop(&q->fifos[PQ_PRIO_LOW ]);
    if(n){ q->size--; *out_item = n->item; vPortFree(n); }
    xSemaphoreGive(q->mutex);
    return (n != NULL);
}

// Destruye la cola de prioridad y libera todos los recursos
void pq_destroy(prio_queue_t *q){
    if(!q) return;
    xSemaphoreTake(q->mutex, portMAX_DELAY);
    for(int i=0;i<PQ_PRIO__N;i++){
        pq_node_t *n;
        while((n = fifo_pop(&q->fifos[i])) != NULL){
            if(n->item.free_cb && n->item.payload){ n->item.free_cb(n->item.payload); }
            vPortFree(n);
        }
    }
    q->size = 0;
    xSemaphoreGive(q->mutex);
    vSemaphoreDelete(q->items_sem);
    vSemaphoreDelete(q->mutex);
}


// ----------------------------------------
// (wrapper para FreeRTOS)
// ----------------------------------------

// Crea una cola de prioridad
PriorityQueueHandle_t xPriorityQueueCreateEx(size_t capacity, size_t item_size, uint8_t num_priorities)
{
	// Validaciones iniciales
    if (item_size != sizeof(void*)) return NULL;
    if (num_priorities != (uint8_t)PQ_PRIO__N) return NULL; // hoy soportamos 3 prioridades

    // Reservar memoria para la estructura de la cola
    struct PriorityQueueOpaque *h = pvPortMalloc(sizeof(*h));
    if (!h) return NULL;

    // Inicializar la cola de prioridad
    if (!pq_init(&h->q, capacity)) {
        vPortFree(h);
        return NULL;
    }

    h->item_size = item_size;
    h->num_prios = num_priorities;
    return h;
}

// Elimina la cola de prioridad y libera todos los recursos
void vPriorityQueueDelete(PriorityQueueHandle_t h)
{
    if (!h) return;
    pq_destroy(&h->q);
    vPortFree(h);
}

// Estructura del mensaje que se envía a la cola de prioridad (debe tener prio como primer campo)
typedef struct { pq_prio_t prio; } _pq_msg_header_t;

// Envia un item a la cola de prioridad
BaseType_t xPriorityQueueSend(PriorityQueueHandle_t h, void * const *ppItem, TickType_t ticksToWait)
{
    (void)ticksToWait;
    if (!h || !ppItem || !*ppItem) return errQUEUE_FULL;

    // Validar que el item tenga el formato correcto
    _pq_msg_header_t *hdr = (_pq_msg_header_t*)(*ppItem);
    pq_item_t it = {
        .prio    = hdr->prio,
        .seq     = 0,			 // se asigna automáticamente al hacer push
        .payload = *ppItem,
        .free_cb = vPortFree	 // Callback para liberar el payload
    };
    return pq_push(&h->q, &it) ? pdPASS : errQUEUE_FULL;
}

// Recibe el item más prioritario de la cola de prioridad
BaseType_t xPriorityQueueReceive(PriorityQueueHandle_t h, void **ppItem, TickType_t ticksToWait)
{

    if (!h || !ppItem) return errQUEUE_EMPTY;

    pq_item_t it;
    if (!pq_pop(&h->q, &it, ticksToWait)) return errQUEUE_EMPTY;
    *ppItem = it.payload;
    return pdPASS;
}
