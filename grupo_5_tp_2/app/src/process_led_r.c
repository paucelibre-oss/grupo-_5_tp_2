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
#include "process_led_r.h"
#include "led_state.h"

#define STACK_SIZE_MINIM	(configMINIMAL_STACK_SIZE * 2)
#define PROCESS_LEDR_PRIORITY (2U)

static TaskHandle_t htask_led_red				= NULL;
static QueueHandle_t hqueue_ao_led_red			= NULL;
static const UBaseType_t queue_length_ledr 		= 10;
static const UBaseType_t queue_item_size_ledr 	= sizeof(uint32_t);
static const char* name_task_ledr	 			= (const char*)("task_led_red");

static void task_ledr(void * pvParameters);
static void fsm_run(state_led_t *st_led);

/*------------------------------------------------ PUBLIC METHODS ------------------------------------------------*/

void process_led_r_init(void){

	hqueue_ao_led_red = xQueueCreate(queue_length_ledr, queue_item_size_ledr);
	configASSERT(hqueue_ao_led_red != NULL);

	BaseType_t res = xTaskCreate(
						task_ledr,
						name_task_ledr,
						STACK_SIZE_MINIM,
						NULL,
						PROCESS_LEDR_PRIORITY,
						&htask_led_red
					 );
	configASSERT(pdPASS == res);
}

bool ao_ledr_send_queue(void *msg){

	if(hqueue_ao_led_red == NULL || msg == NULL) return false;

	return xQueueSend(hqueue_ao_led_red, msg, 0) == pdPASS;
}

/*------------------------------------------------ PRIVATE METHODS ------------------------------------------------*/
static void task_ledr(void * pvParameters){

	/*----- ----- -----  OLD  CODE  ----- ----- -----*/
	/*LOGGER_INFO("[%s] Init.", name_task_ledr);

	for(;;){
		state_led_t *state_led = (state_led_t *)pvPortMalloc(sizeof(state_led_t));
		if(state_led == NULL){
			LOGGER_INFO("[%s] Memory allocation failed.", name_task_ledr);
		}else if(xQueueReceive(hqueue_ao_led_red, &state_led, portMAX_DELAY) == pdPASS){
			fsm_run(state_led);
		}

		vPortFree(state_led);
	}*/
	/*----- ----- ----- ----- ----- ----- ----- -----*/
	state_led_t *state_led = NULL;
	LOGGER_INFO("[%s] Init.", name_task_ledr);

	for(;;){

		if(xQueueReceive(hqueue_ao_led_red, &state_led, portMAX_DELAY) == pdPASS){
			fsm_run(state_led);
			vPortFree(state_led);
			state_led = NULL;
		}
	}
}

static void fsm_run(state_led_t *st_led){

	if(*st_led == LED_ON){
		LOGGER_INFO("[%s] Led on.", name_task_ledr);
		HAL_GPIO_WritePin(LED_GPIO_Port, LED_RED_GPIO, GPIO_PIN_SET);
	}else{
		LOGGER_INFO("[%s] Led off.", name_task_ledr);
		HAL_GPIO_WritePin(LED_GPIO_Port, LED_RED_GPIO, GPIO_PIN_RESET);
	}
}






