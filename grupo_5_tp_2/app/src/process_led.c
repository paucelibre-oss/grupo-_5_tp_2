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
#include "process_led.h"
#include "main.h"
#include "cmsis_os.h"
#include "logger.h"

#include "ao_app.h"

static void print_and_set_pin(ao_t *ao, state_led_t * led_state);

void fsm_led_run(void * param){

	if(param == NULL) return;

	ao_t * ao = (ao_t*)(param);

	state_led_t *state;

	if(ao_receive_queue(ao->hqueue2, &state)){ /*The queue will always be referenced in hqueue2*/
		print_and_set_pin(ao, state);
		vPortFree(state);
	}else{
		vPortFree(state);
	}

}

static void print_and_set_pin(ao_t *ao, state_led_t * led_state){

	if(*led_state == LED_ON){
		LOGGER_INFO("[%s] Led on.", ao->param_task.name_task);
		HAL_GPIO_WritePin(ao->GPIO_Port, ao->GPIO_led, GPIO_PIN_SET);
	}else{
		LOGGER_INFO("[%s] Led off.", ao->param_task.name_task);
		HAL_GPIO_WritePin(ao->GPIO_Port, ao->GPIO_led, GPIO_PIN_RESET);
	}
}





