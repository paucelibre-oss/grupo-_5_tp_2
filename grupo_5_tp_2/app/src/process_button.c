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
#include "main.h"
#include "cmsis_os.h"
#include "logger.h"

#include "ao_app.h"

#define BUTTON_DELAY_MS		10

typedef enum {
	FSM_BUTTON_DELAY,
	FSM_BUTTON_SAVE_TIME,
	FSM_BUTTON_WAIT_MS,
	FSM_BUTTON_CALC_AND_SEND,
	FSM_BUTTON_INIT
} _state_button;

static _state_button stateButton = FSM_BUTTON_INIT;
static const TickType_t period 	 = pdMS_TO_TICKS(BUTTON_DELAY_MS);
static TickType_t lastWakeTime;
static TickType_t startTime;
static TickType_t endTime;


static void fsm_button_init(void);
static void fsm_button_delay(void);
static void fsm_button_save_time(void);
static void fsm_button_wait_ms(void);
static void fsm_button_calc_and_send(ao_t * ao);

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

static void fsm_button_calc_and_send(ao_t * ao){

	uint32_t *time_elapsed = (uint32_t *)pvPortMalloc(sizeof(uint32_t));

	if (time_elapsed == NULL) {
		LOGGER_INFO("[%s] Memory allocation failed.", ao->param_task.name_task);
	    return;
	}

	*time_elapsed = (uint32_t)(endTime - startTime);

	if(!ao_send_queue(ao->hqueue1, &time_elapsed)){
		LOGGER_INFO("[%s] Queue not sent.", ao->param_task.name_task);
		vPortFree(time_elapsed);
	}else {LOGGER_INFO("[%s] Queue sent ok.", ao->param_task.name_task)};

	stateButton = FSM_BUTTON_DELAY;
}

void fsm_button_run(void * param){


	if(param == NULL) return;

	ao_t *ao = (ao_t*)(param);

	switch(stateButton){
		case FSM_BUTTON_INIT:			fsm_button_init();				break;
		case FSM_BUTTON_DELAY:			fsm_button_delay();				break;
		case FSM_BUTTON_SAVE_TIME:		fsm_button_save_time();			break;
		case FSM_BUTTON_WAIT_MS:		fsm_button_wait_ms();			break;
		case FSM_BUTTON_CALC_AND_SEND:	fsm_button_calc_and_send(ao);	break;
		default: 						stateButton = FSM_BUTTON_INIT;  break;
	}

}

