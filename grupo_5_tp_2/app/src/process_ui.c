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
#include "process_ui.h"

#include "process_led_r.h"
#include "process_led_g.h"
#include "process_led_b.h"
#include "led_state.h"

#define STACK_SIZE_MINIM	(configMINIMAL_STACK_SIZE * 2)
#define PROCESS_UI_PRIORITY (4U)

static TaskHandle_t htask_ui				= NULL;
static QueueHandle_t hqueue_ao_ui 			= NULL;
static const UBaseType_t queue_length_ui 	= 10;
static const UBaseType_t queue_item_size_ui = sizeof(uint32_t);
static const char* name_task_ui 			= (const char*)("task_ui");

static void task_ui(void * pvParameters);
static void fsm_run(uint32_t *time_value);
static void fsm_ui_send_queues(state_led_t red, state_led_t green, state_led_t blue);

/*------------------------------------------------ PUBLIC METHODS ------------------------------------------------*/
void process_ui_init(void){

	hqueue_ao_ui = xQueueCreate(queue_length_ui, queue_item_size_ui);
	configASSERT(hqueue_ao_ui != NULL);

	BaseType_t res = xTaskCreate(
						task_ui,
						name_task_ui,
						STACK_SIZE_MINIM,
						NULL,
						PROCESS_UI_PRIORITY,
						&htask_ui
					 );
	configASSERT(pdPASS == res);
}

bool ao_ui_send_queue(void *msg){

	if(hqueue_ao_ui == NULL || msg == NULL) return false;

	return xQueueSend(hqueue_ao_ui, msg, 0) == pdPASS;
}

/*------------------------------------------------ PRIVATE METHODS ------------------------------------------------*/

static void task_ui(void * pvParameters){

	LOGGER_INFO("[%s] Init.", name_task_ui);

	for(;;){

		uint32_t *time_elapsed = (uint32_t *)pvPortMalloc(sizeof(uint32_t));

		if (time_elapsed == NULL) {
			LOGGER_INFO("[%s] Memory allocation failed.", name_task_ui);
		}else if(xQueueReceive(hqueue_ao_ui, &time_elapsed, portMAX_DELAY) == pdPASS){
			fsm_run(time_elapsed);
		}

		vPortFree(time_elapsed);
	}
}

static void fsm_run(uint32_t *time_value){

	if((*time_value > 200U) && (*time_value <= 1000U))
		fsm_ui_send_queues(LED_ON, LED_OFF, LED_OFF);
	else if((*time_value > 1000U) && (*time_value <= 2000U))
		fsm_ui_send_queues(LED_OFF, LED_ON, LED_OFF);
	else if(*time_value > 2000U)
		fsm_ui_send_queues(LED_OFF, LED_OFF, LED_ON);
}

static void fsm_ui_send_queues(state_led_t red, state_led_t green, state_led_t blue){

	state_led_t *led_red 	= (state_led_t*)pvPortMalloc(sizeof(state_led_t));
	state_led_t *led_green 	= (state_led_t*)pvPortMalloc(sizeof(state_led_t));
	state_led_t *led_blue 	= (state_led_t*)pvPortMalloc(sizeof(state_led_t));

	if(led_red == NULL || led_green == NULL || led_blue == NULL){
		LOGGER_INFO("[%s] Unable to reserve dynamic memory to send LEDs status.", name_task_ui);
		vPortFree(led_red);
		vPortFree(led_green);
		vPortFree(led_blue);
		return;
	}

	*led_red 	= red;
	*led_green 	= green;
	*led_blue 	= blue;

	if(!ao_ledr_send_queue(&led_red)){
		vPortFree(led_red);
		LOGGER_INFO("[%s] Queue RED not sent.", name_task_ui);
	}else{
		LOGGER_INFO("[%s] Queue RED sent.", name_task_ui);
	}

	if(!ao_ledg_send_queue(&led_green)){
		vPortFree(led_green);
		LOGGER_INFO("[%s] Queue GREEN not sent.", name_task_ui);
	}else{
		LOGGER_INFO("[%s] Queue GREEN sent.", name_task_ui);
	}

	if(!ao_ledb_send_queue(&led_blue)){
		vPortFree(led_blue);
		LOGGER_INFO("[%s] Queue BLUE not sent.", name_task_ui);
	}else{
		LOGGER_INFO("[%s] Queue BLUE sent.", name_task_ui);
	}
}
