/*
 * Copyright (c) 2026 Paulo Cesar Libreros <paucelibre@gmail.com>.
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
 * @author : Paulo Cesar Libreros <paucelibre@gmail.com>
 */
#include "process_button.h"
#include "process_ui.h"

#define STACK_SIZE_MINIM		(configMINIMAL_STACK_SIZE * 2)
#define PROCESS_BUTTON_PRIORITY (3U)
#define BUTTON_DELAY_MS			10

typedef enum {
	FSM_BUTTON_DELAY,
	FSM_BUTTON_SAVE_TIME,
	FSM_BUTTON_WAIT_MS,
	FSM_BUTTON_CALC_AND_SEND,
	FSM_BUTTON_INIT
} _state_button;

static TaskHandle_t htask_button				= NULL;
static const char* name_task_button	 			= (const char*)("task_button");

static _state_button stateButton;
static const TickType_t period 	 				= pdMS_TO_TICKS(BUTTON_DELAY_MS);
static TickType_t lastWakeTime;
static TickType_t startTime;
static TickType_t endTime;

static void task_button(void * pvParameters);

static void fsm_button_init(void);
static void fsm_button_delay(void);
static void fsm_button_save_time(void);
static void fsm_button_wait_ms(void);
static void fsm_button_calc_and_send(void);

static void fsm_button_run(void);

/*------------------------------------------------ PUBLIC METHODS ------------------------------------------------*/

void process_button_init(void){

	stateButton 	= FSM_BUTTON_INIT;
	lastWakeTime	= 0;
	startTime		= 0;
	endTime			= 0;

	BaseType_t res = xTaskCreate(
						task_button,
						name_task_button,
						STACK_SIZE_MINIM,
						NULL,
						PROCESS_BUTTON_PRIORITY,
						&htask_button
					 );

	configASSERT(pdPASS == res);
}

/*------------------------------------------------ PRIVATE METHODS ------------------------------------------------*/
static void task_button(void * pvParameters){

	LOGGER_INFO("[%s] Init.", name_task_button);

	for(;;){
		fsm_button_run();
	}
}

static void fsm_button_init(void){

	startTime 		= 0U;
	endTime			= 0U;
	lastWakeTime	= xTaskGetTickCount();
	stateButton		= FSM_BUTTON_DELAY;
}

static void fsm_button_delay(void){

	vTaskDelayUntil(&lastWakeTime, period);
	bool press =  HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_RESET;

	if(press){
		stateButton = FSM_BUTTON_SAVE_TIME;
	}
}

static void fsm_button_save_time(void){

	startTime = xTaskGetTickCount();
	stateButton = FSM_BUTTON_WAIT_MS;
}

static void fsm_button_wait_ms(void){

	vTaskDelayUntil(&lastWakeTime, period);

	bool unpress = HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_SET;

	if(unpress){
		endTime = xTaskGetTickCount();
		stateButton = FSM_BUTTON_CALC_AND_SEND;
	}
}

static void fsm_button_calc_and_send(void){

	uint32_t *time_elapsed = (uint32_t *)pvPortMalloc(sizeof(uint32_t));

	if (time_elapsed == NULL) {
		LOGGER_INFO("[%s] Memory allocation failed.", name_task_button);
	    return;
	}

	*time_elapsed = (uint32_t)(endTime - startTime);

	if(!ao_ui_send_queue(&time_elapsed)){
		LOGGER_INFO("[%s] Queue not sent.", name_task_button);
	}else {LOGGER_INFO("[%s] Queue sent ok.", name_task_button)};

	vPortFree(time_elapsed);

	stateButton = FSM_BUTTON_DELAY;
}

void fsm_button_run(void){

	switch(stateButton){
		case FSM_BUTTON_INIT:			fsm_button_init();				break;
		case FSM_BUTTON_DELAY:			fsm_button_delay();				break;
		case FSM_BUTTON_SAVE_TIME:		fsm_button_save_time();			break;
		case FSM_BUTTON_WAIT_MS:		fsm_button_wait_ms();			break;
		case FSM_BUTTON_CALC_AND_SEND:	fsm_button_calc_and_send();		break;
		default: 						stateButton = FSM_BUTTON_INIT;  break;
	}

}

