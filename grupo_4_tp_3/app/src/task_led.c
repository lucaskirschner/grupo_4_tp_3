/*
 * Copyright (c) 2023 Sebastian Bedin <sebabedin@gmail.com>.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * @author : Sebastian Bedin <sebabedin@gmail.com>
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

#include "task_led.h"
#include "task_ui.h"
#include "freertos_priority_queue.h"

/********************** macros and definitions *******************************/

#define PQ_WAITING_PERIOD_MS    50

/********************** internal data declaration ****************************/

/********************** internal functions declaration ***********************/

/********************** internal data definition *****************************/

/********************** external data definition *****************************/

/********************** internal functions definition ************************/

/********************** external functions definition ************************/

void task_led(void *argument)
{

    PriorityQueueHandle_t hq = (PriorityQueueHandle_t)argument;

    //Simula el apagado de los LEDs al inicio
    LOGGER_INFO("Led RED off");
    LOGGER_INFO("Led GREEN off");
    LOGGER_INFO("Led BLUE off");

	while (true)
	{
	    ui_led_msg_t *job = NULL;

	    if (pdPASS == xPriorityQueueReceive(hq, (void**)&job, PQ_WAITING_PERIOD_MS))
	    {
			switch (job->color) {
				case UI_LED_RED:
					LOGGER_INFO("[%d]Led RED on",job->id);
					vTaskDelay(pdMS_TO_TICKS(job->on_time_ms));
					LOGGER_INFO("[%d]Led RED off",job->id);
					break;

				case UI_LED_GREEN:
					LOGGER_INFO("[%d]Led GREEN on",job->id);
					vTaskDelay(pdMS_TO_TICKS(job->on_time_ms));
					LOGGER_INFO("[%d]Led GREEN off",job->id);
					break;

				case UI_LED_BLUE:
					LOGGER_INFO("[%d]Led BlUE on",job->id);
					vTaskDelay(pdMS_TO_TICKS(job->on_time_ms));
					LOGGER_INFO("[%d]Led BLUE off",job->id);
					break;

				default:
					break;
					vPortFree(job);
			}

		}
	}
}

void ao_led_init(PriorityQueueHandle_t hq) {
  BaseType_t st = xTaskCreate(task_led, "task_ao_led", 256, (void*)hq, tskIDLE_PRIORITY + 1, NULL);
  while (pdPASS != st) {
	  /* error */
	  }
}


/********************** end of file ******************************************/
