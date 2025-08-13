/*
 * ao_led.h
 *
 *  Created on: Aug 12, 2025
 *      Author: Grupo 4
 */

#ifndef TASK_LED_H_
#define TASK_LED_H_

/********************** CPP guard ********************************************/
#ifdef __cplusplus
extern "C" {
#endif

/********************** inclusions *******************************************/

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "main.h"
#include "cmsis_os.h"

/********************** macros ***********************************************/

/********************** typedef **********************************************/

typedef enum
{
  AO_LED_MESSAGE_ON,
  AO_LED_MESSAGE_OFF,
  AO_LED_MESSAGE_BLINK,
  AO_LED_MESSAGE__N,
} ao_led_action_t;

typedef enum
{
  AO_LED_COLOR_RED,
  AO_LED_COLOR_GREEN,
  AO_LED_COLOR_BLUE,
  AO_LED_COLOR__N,
} ao_led_color;

typedef void (*ao_led_cb_t)(void*);

typedef struct
{
    ao_led_cb_t callback;
    ao_led_action_t action;
} ao_led_message_t;

typedef struct {
  ao_led_color color;
  QueueHandle_t hqueue;
} ao_led_handle_t;

/********************** external data declaration ****************************/

/********************** external functions declaration ***********************/
bool ao_led_send_event(ao_led_message_t* pmsg, TickType_t ticksToWait);

/********************** End of CPP guard *************************************/
#ifdef __cplusplus
}
#endif

#endif /* TASK_LED_H_ */
/********************** end of file ******************************************/

