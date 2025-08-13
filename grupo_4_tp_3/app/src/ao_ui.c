/*
 * ao_ui.c
 *
 *  Created on: Aug 12, 2025
 *      Author: Grupo 4
 */

/********************** inclusions *******************************************/

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "main.h"
#include "cmsis_os.h"
#include "board.h"
#include "logger.h"
#include "dwt.h"

#include "ao_ui.h"
#include "ao_led.h"

/********************** macros and definitions *******************************/
#define UI_IDLE_TIMEOUT_MS_		 (10000)

#define QUEUE_LENGTH_            (10)
#define QUEUE_ITEM_SIZE_         (sizeof(ao_ui_message_t*))

/********************** internal data declaration ****************************/
typedef struct
{
	QueueHandle_t hqueue;
} ao_ui_handle_t;


/********************** internal functions declaration ***********************/
static void queue_ui_delete(void);

/********************** internal data definition *****************************/
static ao_ui_handle_t hao_ui = {.hqueue = NULL};

/********************** external data definition *****************************/

/********************** internal functions definition ************************/

static void callback_task_ui(void *pmsg)
{
	vPortFree(pmsg);
}

void task_ui(void* argument)
{
	while (true)
	{
		ao_ui_message_t *pmsg;
		if (pdPASS == xQueueReceive(hao_ui.hqueue, &pmsg, pdMS_TO_TICKS(UI_IDLE_TIMEOUT_MS_)))
		{
			switch(pmsg->action)
			{
			case MSG_EVENT_BUTTON_NONE:
				break;
			case MSG_EVENT_BUTTON_PULSE:
				/* Función que encola evento de Prioridad Alta */
				break;
			case MSG_EVENT_BUTTON_SHORT:
				/* Función que encola evento de Prioridad Media */
				break;
			case MSG_EVENT_BUTTON_LONG:
				/* Función que encola evento de Prioridad Baja */
				break;
			default:
				break;
			}

			if(pmsg->callback)
			{
				pmsg->callback(pmsg);
			}
		}

		else
		{
			queue_ui_delete();
			vTaskDelete(NULL);
		}
	}
}

/********************** external functions definition ************************/

bool ao_ui_send_event(ao_ui_message_t *pmsg, TickType_t ticksToWait)
{
	if(NULL == hao_ui.hqueue)
	{
		hao_ui.hqueue = xQueueCreate(QUEUE_LENGTH_, QUEUE_ITEM_SIZE_);
		if (NULL == hao_ui.hqueue)
		{
			// error
			return false;
		}

		BaseType_t status;
		status = xTaskCreate(task_ui, "task_ui", 128, NULL, tskIDLE_PRIORITY, NULL);
		if (pdPASS != status)
		{
			// error
			vQueueDelete(hao_ui.hqueue);
			hao_ui.hqueue = NULL;
			return false;
		}
	}
	return (pdPASS == xQueueSend(hao_ui.hqueue, (void*)&pmsg, ticksToWait));
}

/********************** internal functions definition ************************/
static void queue_ui_delete(void)
{
	if(NULL != hao_ui.hqueue)
	{
		ao_ui_message_t *pmsg;
		taskENTER_CRITICAL();{
			while(pdPASS == xQueueReceive(hao_ui.hqueue, (void*)&pmsg, 0))
			{
				vPortFree(pmsg);
			}
			vQueueDelete(hao_ui.hqueue);
			hao_ui.hqueue = NULL;
		}taskEXIT_CRITICAL();
	}
}

/********************** end of file ******************************************/
