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
 * @author : Paulo Cesar Libreros <paucelibre@gmail.com>.
 */

/********************** inclusions *******************************************/
#include "main.h"
#include "cmsis_os.h"
#include "logger.h"
#include "ao_app.h"
#include "process_button.h"
#include "process_led.h"
#include "process_ui.h"
#include "task_app.h"

/********************** macros and definitions *******************************/
#define QUEUE_LENGTH_       (5)
#define STACK_SIZE_MINIM	(configMINIMAL_STACK_SIZE * 2)

static QueueHandle_t hqueue_time  = NULL;
static QueueHandle_t hqueue_led_r = NULL;
static QueueHandle_t hqueue_led_g = NULL;
static QueueHandle_t hqueue_led_b = NULL;

static TaskHandle_t htask_button = NULL;
static TaskHandle_t htask_ui	 = NULL;
static TaskHandle_t htask_ledR;
static TaskHandle_t htask_ledG;
static TaskHandle_t htask_ledB;

static ao_t ao_button;
static ao_t ao_ui;
static ao_t ao_ledR;
static ao_t ao_ledG;
static ao_t ao_ledB;

static void create_queue(void);
static void init_aoq_button(void);
static void init_aoq_ui(void);
static void init_aos_ledr(void);
static void init_aos_ledg(void);
static void init_aos_ledb(void);

void app_init(void){

	create_queue();

	init_aoq_button();
	init_aoq_ui();
	init_aos_ledr();
	init_aos_ledg();
	init_aos_ledb();

	ao_init(&ao_button, fsm_button_run);
	ao_init(&ao_ui, fsm_ui_run);
	ao_init(&ao_ledR, fsm_led_run);
	ao_init(&ao_ledG, fsm_led_run);
	ao_init(&ao_ledB, fsm_led_run);

	vTaskStartScheduler();

}

static void create_queue(void){

	hqueue_time  = xQueueCreate(QUEUE_LENGTH_, (sizeof(uint32_t*)));
	hqueue_led_r = xQueueCreate(QUEUE_LENGTH_, (sizeof(state_led_t*)));
	hqueue_led_g = xQueueCreate(QUEUE_LENGTH_, (sizeof(state_led_t*)));
	hqueue_led_b = xQueueCreate(QUEUE_LENGTH_, (sizeof(state_led_t*)));

	configASSERT(hqueue_time != NULL);
	configASSERT(hqueue_led_r != NULL);
	configASSERT(hqueue_led_g != NULL);
	configASSERT(hqueue_led_b != NULL);
}

static void init_aoq_button(void){

	ao_button.hqueue1			 			= hqueue_time;
	ao_button.param_task.task				= generic_task;
	ao_button.param_task.name_task			= (const char*)("task_button");
	ao_button.param_task.stack_size			= STACK_SIZE_MINIM;
	ao_button.param_task.parameters			= &ao_button;
	ao_button.param_task.priority			= 3U;
	ao_button.param_task.id_task			= htask_button;
}

static void init_aoq_ui(void){

	ao_ui.hqueue1 							= hqueue_time;
	ao_ui.hqueue2							= hqueue_led_r;
	ao_ui.hqueue3							= hqueue_led_g;
	ao_ui.hqueue4							= hqueue_led_b;
	ao_ui.param_task.task					= generic_task;
	ao_ui.param_task.name_task				= (const char*)("task_ui");
	ao_ui.param_task.stack_size				= STACK_SIZE_MINIM;
	ao_ui.param_task.parameters				= &ao_ui;
	ao_ui.param_task.priority				= 4U;
	ao_ui.param_task.id_task				= htask_ui;
}

static void init_aos_ledr(void){

	ao_ledR.hqueue2							= hqueue_led_r;
	ao_ledR.GPIO_led						= LED_RED_GPIO;
	ao_ledR.param_task.task					= generic_task;
	ao_ledR.param_task.name_task			= (const char*)("task_led_red");
	ao_ledR.param_task.stack_size			= STACK_SIZE_MINIM;
	ao_ledR.param_task.parameters			= &ao_ledR;
	ao_ledR.param_task.priority				= 2U;
	ao_ledR.param_task.id_task				= htask_ledR;
}

static void init_aos_ledg(void){

	ao_ledG.hqueue2							= hqueue_led_g;
	ao_ledG.GPIO_led						= LED_GREEN_GPIO;
	ao_ledG.param_task.task					= generic_task;
	ao_ledG.param_task.name_task			= (const char*)("task_led_green");
	ao_ledG.param_task.stack_size			= STACK_SIZE_MINIM;
	ao_ledG.param_task.parameters			= &ao_ledG;
	ao_ledG.param_task.priority				= 2U;
	ao_ledG.param_task.id_task				= htask_ledG;
}

static void init_aos_ledb(void){

	ao_ledB.hqueue2							= hqueue_led_b;
	ao_ledB.GPIO_led						= LED_BLUE_GPIO;
	ao_ledB.param_task.task					= generic_task;
	ao_ledB.param_task.name_task			= (const char*)("task_led_blue");
	ao_ledB.param_task.stack_size			= STACK_SIZE_MINIM;
	ao_ledB.param_task.parameters			= &ao_ledB;
	ao_ledB.param_task.priority				= 2U;
	ao_ledB.param_task.id_task				= htask_ledB;
}

/********************** end of file ******************************************/



