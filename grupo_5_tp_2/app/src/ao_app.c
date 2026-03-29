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
#include "ao_app.h"

/*--------------- defines ---------------*/
#define MAX_WAIT_QUEUE_SEND_MS		2U

void ao_init(ao_t * ao, process_handler_t process){

	if(ao == NULL || process == NULL){
		LOGGER_INFO("\r\n[aoQ_init] The parameters are null.");
		return;
	}

	ao->process_handler = process;

	BaseType_t res = xTaskCreate(
						ao->param_task.task,
						ao->param_task.name_task,
						ao->param_task.stack_size,
						ao->param_task.parameters,
						ao->param_task.priority,
						&ao->param_task.id_task
					 );

	configASSERT(pdPASS == res);
}

bool ao_send_queue(QueueHandle_t queue, void * msg){
	return (xQueueSend(queue, msg, MAX_WAIT_QUEUE_SEND_MS) == pdPASS);
}

bool ao_receive_queue(QueueHandle_t queue, void * msg){
	return (xQueueReceive(queue, msg, portMAX_DELAY) == pdPASS);
}


