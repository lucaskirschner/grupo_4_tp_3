/*
 * ao_ui.h
 *
 *  Created on: Aug 12, 2025
 *      Author: Grupo 4
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
#include "cmsis_os.h"

/********************** macros ***********************************************/

/********************** typedef **********************************************/

typedef enum
{
	MSG_EVENT_BUTTON_NONE,
	MSG_EVENT_BUTTON_PULSE,
	MSG_EVENT_BUTTON_SHORT,
	MSG_EVENT_BUTTON_LONG,
} ao_ui_action_t;

typedef void (*ao_ui_cb_t)(void*);

typedef struct
{
    ao_ui_cb_t callback;
    ao_ui_action_t action;
} ao_ui_message_t;

/********************** external data declaration ****************************/

/********************** external functions declaration ***********************/

void task_ui(void* argument);
bool ao_ui_send_event(ao_ui_message_t *pmsg, TickType_t ticksToWait);

/********************** End of CPP guard *************************************/
#ifdef __cplusplus
}
#endif

#endif /* TASK_UI_H_ */
/********************** end of file ******************************************/

