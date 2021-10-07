#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "esp_log.h"

/* FreeRTOS.org includes. */
#include "freertos/FreeRTOSConfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "my_helper_fct.h"


static const uint32_t MESS_MAX_MSG=5;
/* Default stack size for tasks */
static const uint32_t STACK_SIZE = 4000;
static const uint32_t T1_PRIO = 2;
static const uint32_t T2_PRIO = 2;
static const uint32_t T3_PRIO = 3;

QueueHandle_t xMsgQ;
static const char* TAG="MsgQ";


/**
 * @brief Starting point function
 * 
 */
void vTaskFunction3(void *pvParameters){
	for(;;){
		// Task blocked during 100 ms
		DISPLAY("Task is waiting");
		vTaskDelay(pdMS_TO_TICKS (100)) ;
		DISPLAY("Task is running");
		// Compute time : 2 ticks or 20 ms
		COMPUTE_IN_TICK(2);
	}
	
}
void vTaskFunction2(void *pvParameters){
	static const char *TAG =" MsgTimeOut ";
	uint32_t value=0;
	for(;;){
		// Wait for message,timeout=300ms ,30 ticks
		uint32_t result=xQueueReceive(xMsgQ,&value,30);
		// display task number and message
		if (result==pdPASS){
			DISPLAYI(TAG,"Task2 mess =  %d",value);
			// Compute time : 3 ticks or 30 ms
			COMPUTE_IN_TIME_MS(30) ;
		}
		else{
			DISPLAYE(TAG,"Task 2,Timeout!")
			COMPUTE_IN_TIME_MS(10);
		}
	}
		
}
void vTaskFunction1(void *pvParameters){
	uint32_t value=0;
	TickType_t xLasWaketime=xTaskGetTickCount();
	// Post
	for(;;){
		value++;
		// 0 means no timeout
		uint32_t result = xQueueSend(xMsgQ,&value,0) ;
			// Check result
		if (result==pdPASS){
			DISPLAY("Message of Task2 add %d to queue",value);
		}
		else
			DISPLAY("Message of Task2 failed to add %d to queue",value);
		// Compute time : 4 ticks or 40 ms
		COMPUTE_IN_TIME_MS(40) ;
		// block periodically : 500 ms
		vTaskDelayUntil (&xLasWaketime,pdMS_TO_TICKS(500)) ;
	}
	
}
void app_main(void) {

	printf("Hello ESP32 !\n");
	xMsgQ=xQueueCreate(MESS_MAX_MSG,sizeof(uint32_t));
	if(xMsgQ==NULL){
		ESP_LOGE(TAG,"MsgQ cannot be created");
	}

	vTaskSuspendAll();

    BaseType_t result;

	result=xTaskCreatePinnedToCore(vTaskFunction1, "Task 1", STACK_SIZE, (void*)"Task 1", T1_PRIO, NULL,CORE_1);
	if(result!=pdPASS){
		ESP_LOGE(TAG,"Task1 cannot be created");
	}
	result=xTaskCreatePinnedToCore(vTaskFunction2, "Task 2", STACK_SIZE, (void*)"Task 2", T2_PRIO, NULL,CORE_0);
	if(result!=pdPASS){
		ESP_LOGE(TAG,"Task2 cannot be created");
	}
	result=xTaskCreatePinnedToCore(vTaskFunction3, "Task 3", STACK_SIZE, (void*)"Task 3", T3_PRIO, NULL,CORE_0);
	if(result!=pdPASS){
		ESP_LOGE(TAG,"Task2 cannot be created");
	}

	xTaskResumeAll();
	vTaskDelete(NULL);
}

