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

#define QUEUE_LENGTH_            (10)
#define QUEUE_ITEM_SIZE_         (sizeof(ao_led_message_t*))

/********************** external data declaration ****************************/
typedef struct
{
	PriorityQueueHandle_t hqueue;
} ao_led_handle_t;

/********************** internal functions declaration ***********************/

/********************** internal data definition *****************************/
static ao_led_handle_t hao_led = {.hqueue = NULL};

/********************** internal functions definition ************************/
static void turn_on_led(ao_led_handle_t* hao)
{
	/* Enciende LED */
}

static void turn_off_led(ao_led_handle_t* hao)
{
	/* Apaga LED */
}

/********************** external functions definition ************************/
void task_led(void* argument)
{
	ao_led_message_t* pmsg;

	if(NULL != hao_led->hqueue)
	{
		if (pdPASS == xQueueReceive(hao->hqueue, (void*)&pmsg, 0))
		{
			/* Decola los eventos de la priority queue y los procesa */

			if(pmsg->callback)
			{
				pmsg->callback(pmsg);
			}
		}
	}
}

bool ao_led_send_event(ao_led_message_t* pmsg, TickType_t ticksToWait)
{
	if(NULL == hao_led->hqueue)
	{
		hao_led->hqueue = xPriorityQueueCreate(QUEUE_LENGTH_, QUEUE_ITEM_SIZE_);
		if (NULL == hao_led->hqueue)
		{
			// error
			return false;
		}

		BaseType_t status;
		status = xTaskCreate(task_led, "task_led", 128, NULL, tskIDLE_PRIORITY, NULL);
		if (pdPASS != status)
		{
			// error
			vPriorityQueueDelete(hao_led->hqueue);
			hao_led->hqueue = NULL;
			return false;
		}
	}
	return (pdPASS == xPriorityQueueSend(hao_led->hqueue, (void*)&pmsg, ticksToWait));
}

/********************** end of file ******************************************/
