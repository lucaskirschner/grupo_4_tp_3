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
#define ON_TIME_MS_		 		 (5000)

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
const char* const button_action_name[] = {
		"MESSAGE_BUTTON_NONE",
		"MESSAGE_BUTTON_PULSE",
		"MESSAGE_BUTTON_SHORT",
		"MESSAGE_BUTTON_LONG",
};

/********************** internal functions definition ************************/

static void callback_task_ui(void *pmsg)
{
	ao_led_message_t *msg = (ao_led_message_t *)pmsg;
	LOGGER_INFO("Liberando memoria de %s", priority_name[msg->prio]);
	vPortFree(pmsg);
}

void task_ui(void* argument)
{
	while (true)
	{
		ao_ui_message_t *pmsg;
		if (pdPASS == xQueueReceive(hao_ui.hqueue, &pmsg, pdMS_TO_TICKS(UI_IDLE_TIMEOUT_MS_)))
		{
			ao_led_message_t *pmsg_led = (ao_led_message_t*)pvPortMalloc(sizeof(ao_led_message_t));
			if(NULL != pmsg_led)
			{
				pmsg_led->action   = AO_LED_MESSAGE_ON;
				pmsg_led->on_time  = pdMS_TO_TICKS(ON_TIME_MS_);
				pmsg_led->callback = callback_task_ui;

				switch(pmsg->action)
				{
				case MSG_EVENT_BUTTON_NONE:
					break;
				case MSG_EVENT_BUTTON_PULSE:
					/* Función que encola evento de Prioridad Alta */
					pmsg_led->prio  = AO_LED_MESSAGE_HIGH_PRIORITY;
					pmsg_led->color = AO_LED_COLOR_RED;
					LOGGER_INFO("Creando %s", priority_name[pmsg_led->prio]);
					break;
				case MSG_EVENT_BUTTON_SHORT:
					/* Función que encola evento de Prioridad Media */
					pmsg_led->prio  = AO_LED_MESSAGE_MEDIA_PRIORITY;
					pmsg_led->color = AO_LED_COLOR_GREEN;
					LOGGER_INFO("Creando %s", priority_name[pmsg_led->prio]);
					break;
				case MSG_EVENT_BUTTON_LONG:
					/* Función que encola evento de Prioridad Baja */
					pmsg_led->prio  = AO_LED_MESSAGE_LOW_PRIORITY;
					pmsg_led->color = AO_LED_COLOR_BLUE;
					LOGGER_INFO("Creando %s", priority_name[pmsg_led->prio]);
					break;
				default:
					break;
				}

				if(!ao_led_send_event(pmsg_led, 0))
				{
					LOGGER_INFO("No se pudo enviar %s", priority_name[pmsg_led->prio]);
					vPortFree(pmsg_led);
				}
			}

			if(pmsg->callback)
			{
				pmsg->callback(pmsg);
			}
		}

		else
		{
			queue_ui_delete();
			LOGGER_INFO("Tarea UI eliminada");
			vTaskDelete(NULL);
		}
	}
}

/********************** external functions definition ************************/

bool ao_ui_send_event(ao_ui_message_t *pmsg, TickType_t ticksToWait)
{
	if(NULL == hao_ui.hqueue)
	{
		LOGGER_INFO("Creando cola de UI");
		hao_ui.hqueue = xQueueCreate(QUEUE_LENGTH_, QUEUE_ITEM_SIZE_);
		if (NULL == hao_ui.hqueue)
		{
			// error
			LOGGER_INFO("Error creando cola de UI");
			return false;
		}

		LOGGER_INFO("Creando tarea de UI");
		BaseType_t status;
		status = xTaskCreate(task_ui, "task_ui", 128, NULL, tskIDLE_PRIORITY, NULL);
		if (pdPASS != status)
		{
			// error
			LOGGER_INFO("Error creando tarea de UI");
			LOGGER_INFO("Eliminando cola de UI");
			vQueueDelete(hao_ui.hqueue);
			LOGGER_INFO("Cola UI eliminada");
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
		LOGGER_INFO("Eliminando cola de UI");
		ao_ui_message_t *pmsg;
		taskENTER_CRITICAL();{
			while(pdPASS == xQueueReceive(hao_ui.hqueue, (void*)&pmsg, 0))
			{
				LOGGER_INFO("Liberando memoria de %s de la cola UI", button_action_name[pmsg->action]);
				vPortFree(pmsg);
			}
			vQueueDelete(hao_ui.hqueue);
			LOGGER_INFO("Cola UI eliminada");
			hao_ui.hqueue = NULL;
		}taskEXIT_CRITICAL();
	}
}

/********************** end of file ******************************************/
