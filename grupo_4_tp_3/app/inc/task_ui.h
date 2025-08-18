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

#ifndef TASK_UI_H_
#define TASK_UI_H_

/********************** CPP guard ********************************************/
#ifdef __cplusplus
extern "C" {
#endif

/********************** inclusions *******************************************/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "main.h"
#include "priority_queue.h"
/********************** macros ***********************************************/

/********************** typedef **********************************************/
typedef enum
{
  MSG_EVENT_BUTTON_PULSE,
  MSG_EVENT_BUTTON_SHORT,
  MSG_EVENT_BUTTON_LONG,
  MSG_EVENT__N,
} msg_event_t;

typedef enum {
  UI_LED_RED,
  UI_LED_GREEN,
  UI_LED_BLUE,
} ui_led_color_t;

typedef struct {
  pq_prio_t prio;      // PQ_PRIO_HIGH / PQ_PRIO_MED / PQ_PRIO_LOW
  ui_led_color_t color;
  uint32_t on_time_ms;
  uint8_t id;
} ui_led_msg_t;

/********************** external data declaration ****************************/

/********************** external functions declaration ***********************/

void task_ui(void* argument);

bool ao_ui_send_event(msg_event_t msg);

//void ao_ui_init();

PriorityQueueHandle_t ao_ui_init(void);
/********************** End of CPP guard *************************************/
#ifdef __cplusplus
}
#endif

#endif /* TASK_UI_H_ */
/********************** end of file ******************************************/

