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
#include "main.h"
#include "cmsis_os.h"
#include "logger.h"

#include "ao_app.h"
#include "process_led.h"

typedef enum {
	FSM_UI_RECEIVE_TIME,
	FSM_UI_SEND_QUEUE_RED,
	FSM_UI_SEND_QUEUE_GREEN,
	FSM_UI_SEND_QUEUE_BLUE
} _state_ui;

typedef enum {
	STATE_LED_RED,
	STATE_LED_GREEN,
	STATE_LED_BULE,

	NUM_LEDS
} _leds;

static _state_ui stateUi = FSM_UI_RECEIVE_TIME;

static void fsm_ui_receive_time(ao_t * ao);
static void fsm_ui_send_queue_red(ao_t * ao);
static void fsm_ui_send_queue_green(ao_t * ao);
static void fsm_ui_send_queue_blue(ao_t * ao);
static void send_queues(ao_t * ao, state_led_t red, state_led_t green, state_led_t blue);

static void fsm_ui_receive_time(ao_t * ao){

	if(ao->hqueue1 == NULL) return;

	uint32_t* timeB = NULL;

	if(ao_receive_queue(ao->hqueue1, &timeB)){
		LOGGER_INFO("[%s] Queue receive value: %lums.", ao->param_task.name_task, *timeB);
		if((*timeB > 200U) && (*timeB <= 1000U))
			stateUi = FSM_UI_SEND_QUEUE_RED;
		else if((*timeB > 1000U) && (*timeB <= 2000U))
			stateUi = FSM_UI_SEND_QUEUE_GREEN;
		else if(*timeB > 2000U)
			stateUi = FSM_UI_SEND_QUEUE_BLUE;
	}else{
		vPortFree(timeB);
	}
}

static void fsm_ui_send_queue_red(ao_t * ao){

	send_queues(ao, LED_ON, LED_OFF, LED_OFF);
	stateUi = FSM_UI_RECEIVE_TIME;
}

static void fsm_ui_send_queue_green(ao_t * ao){

	send_queues(ao, LED_OFF, LED_ON, LED_OFF);
	stateUi = FSM_UI_RECEIVE_TIME;
}

static void fsm_ui_send_queue_blue(ao_t * ao){

	send_queues(ao, LED_OFF, LED_OFF, LED_ON);
	stateUi = FSM_UI_RECEIVE_TIME;
}

static void send_queues(ao_t * ao, state_led_t red, state_led_t green, state_led_t blue){

	if(ao->hqueue2 == NULL) return;
	if(ao->hqueue3 == NULL) return;
	if(ao->hqueue4 == NULL) return;

	state_led_t *led_state_red = (state_led_t *) pvPortMalloc(sizeof(state_led_t));
	state_led_t *led_state_green = (state_led_t *) pvPortMalloc(sizeof(state_led_t));
	state_led_t *led_state_blue = (state_led_t *) pvPortMalloc(sizeof(state_led_t));

	if(led_state_red != NULL){
		*led_state_red = red;
		if(!ao_send_queue(ao->hqueue2, &led_state_red)){
			vPortFree(led_state_red);
			LOGGER_INFO("[%s] Queue RED not sent.", ao->param_task.name_task);
		}else{
			LOGGER_INFO("[%s] Queue RED sent.", ao->param_task.name_task)
		}
	}else{
		LOGGER_INFO("[%s] Unable to reserve dynamic memory to send RED LED status.", ao->param_task.name_task);
	}

	if(led_state_green != NULL){
		*led_state_green = green;
		if(!ao_send_queue(ao->hqueue3, &led_state_green)){
			vPortFree(led_state_green);
			LOGGER_INFO("[%s] Queue GREEN not sent.", ao->param_task.name_task);
		}else{
			LOGGER_INFO("[%s] Queue GREEN sent.", ao->param_task.name_task)
		}
	}else{
		LOGGER_INFO("[%s] Unable to reserve dynamic memory to send GREEN LED status.", ao->param_task.name_task);
	}

	if(led_state_blue != NULL){
		*led_state_blue = blue;
		if(!ao_send_queue(ao->hqueue4, &led_state_blue)){
			vPortFree(led_state_blue);
			LOGGER_INFO("[%s] Queue BLUE not sent.", ao->param_task.name_task);
		}else{
			LOGGER_INFO("[%s] Queue BLUE sent.", ao->param_task.name_task)
		}
	}else{
		LOGGER_INFO("[%s] Unable to reserve dynamic memory to send BLUE LED status.", ao->param_task.name_task);
	}
}

void fsm_ui_run(void * param){

	if(param == NULL) return;

	ao_t *ao = (ao_t*)(param);

	switch(stateUi){
		case FSM_UI_RECEIVE_TIME:		fsm_ui_receive_time(ao); 			break;
		case FSM_UI_SEND_QUEUE_RED:		fsm_ui_send_queue_red(ao);			break;
		case FSM_UI_SEND_QUEUE_GREEN:	fsm_ui_send_queue_green(ao);		break;
		case FSM_UI_SEND_QUEUE_BLUE:	fsm_ui_send_queue_blue(ao);			break;
		default: 							stateUi = FSM_UI_RECEIVE_TIME;	break;
	}
}


