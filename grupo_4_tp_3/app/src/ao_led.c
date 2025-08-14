/*
 * ao_led.c
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

#include "ao_led.h"

/********************** macros and definitions *******************************/

#define UI_IDLE_TIMEOUT_MS_		 (10000)
#define QUEUE_LENGTH_            (10)
#define QUEUE_ITEM_SIZE_         (sizeof(ao_led_message_t*))

/********************** external data definition ****************************/
const char* const led_color_name[] = {
		"LED_RED",
		"LED_GREEN",
		"LED_BLUE",
		"LED_NONE"
};

const char* const led_action_name[] = {
		"MESSAGE_LED_ON",
		"MESSAGE_LED_OFF",
		"MESSAGE_LED_BLINK",
		"MESSAGE_LED_NONE",
};

/********************** external data declaration ****************************/
typedef struct
{
	PriorityQueueHandle_t hqueue;
} ao_led_handle_t;

/********************** internal functions declaration ***********************/
static void queue_led_delete(void);

/********************** internal data definition *****************************/
static ao_led_handle_t hao_led = {.hqueue = NULL};

/********************** internal functions definition ************************/
static void turn_on_led(ao_led_color led)
{
	/* Enciende LED */
	LOGGER_INFO("%s encendido", led_color_name[led]);
}

static void turn_off_led(ao_led_color led)
{
	/* Apaga LED */
	LOGGER_INFO("%s apagado", led_color_name[led]);
}

/********************** external functions definition ************************/
void task_led(void* argument)
{
	while(true)
	{
		ao_led_message_t* pmsg;

		if(NULL != hao_led.hqueue)
		{
			if (pdPASS == xPriorityQueueReceive(hao_led.hqueue, (void**)&pmsg, pdMS_TO_TICKS(UI_IDLE_TIMEOUT_MS_)))
			{
				if (pmsg->action == AO_LED_MESSAGE_ON)
				{
					turn_on_led(pmsg->color);
					vTaskDelay(pmsg->on_time);
					turn_off_led(pmsg->color);
				}
				else if(pmsg->action == AO_LED_MESSAGE_OFF)
				{
					turn_off_led(pmsg->color);
				}

				if(pmsg->callback)
				{
					pmsg->callback(pmsg);
				}
			}
			else
			{
				queue_led_delete();
				LOGGER_INFO("Tarea LED eliminada");
				vTaskDelete(NULL);
			}
		}
	}
}

bool ao_led_send_event(ao_led_message_t* pmsg, TickType_t ticksToWait)
{
	if(NULL == hao_led.hqueue)
	{
		LOGGER_INFO("Creando cola prioritaria LED");
		hao_led.hqueue = xPriorityQueueCreate(QUEUE_LENGTH_, sizeof(void*));
		if (NULL == hao_led.hqueue)
		{
			// error
			LOGGER_INFO("Error creando cola de LED");
			return false;
		}

		LOGGER_INFO("Creando tarea de LED");
		BaseType_t status;
		status = xTaskCreate(task_led, "task_led", 128, NULL, tskIDLE_PRIORITY, NULL);
		if (pdPASS != status)
		{
			// error
			LOGGER_INFO("Error creando tarea de LED");
			LOGGER_INFO("Eliminando cola prioritaria LED");
			vPriorityQueueDelete(hao_led.hqueue);
			LOGGER_INFO("Cola prioritaria LED eliminada");
			hao_led.hqueue = NULL;
			return false;
		}
	}
	return (pdPASS == xPriorityQueueSend(hao_led.hqueue, (void*)&pmsg, ticksToWait));
}

/********************** internal functions definition ************************/
static void queue_led_delete(void)
{
	if(NULL != hao_led.hqueue)
	{
		LOGGER_INFO("Eliminando cola prioritaria LED");
		ao_led_message_t *pmsg;
		taskENTER_CRITICAL();{
			while(pdPASS == xPriorityQueueReceive(hao_led.hqueue, (void**)&pmsg, 0))
			{
				LOGGER_INFO("Liberando memoria de %s de la cola prioritaria LED", priority_name[pmsg->prio]);
				vPortFree(pmsg);
			}
			vPriorityQueueDelete(hao_led.hqueue);
			LOGGER_INFO("Cola prioritaria LED eliminada");
			hao_led.hqueue = NULL;
		}taskEXIT_CRITICAL();
	}
}

/********************** end of file ******************************************/
