/****************************************************************************
 * Copyright (C) 2020 by Fabrice Muller                                     *
 *                                                                          *
 * This file is useful for ESP32 Design course.                             *
 *                                                                          *
 ****************************************************************************/

/**
 * @file lab3-1_main.c
 * @author Fabrice Muller
 * @date 20 Oct. 2020
 * @brief File containing the lab3-1 of Part 3.
 *
 * @see https://github.com/fmuller-pns/esp32-vscode-project-template
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include "esp_log.h"

/* FreeRTOS.org includes. */
#include "freertos/FreeRTOSConfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "my_helper_fct.h"

static const char* TAG = "SEM";

/* Application constants */
#define STACK_SIZE     4096
#define TABLE_SIZE     400

/* Task Priority */

const uint32_t TIMER_TASK_PRIORITY = 5;
const uint32_t INC_TABLE_TASK_PRIORITY = 3;
const uint32_t DEC_TABLE_TASK_PRIORITY = 4;



/* Communications */
SemaphoreHandle_t xSemIncTab;
SemaphoreHandle_t xSemDecTab;

/* Tasks */
void vTaskTimer(void *pvParameters);
void vTaskIncTable(void *pvParameters);
void vTaskDecTable(void *pvParameters);

/* Datas */
int Table[TABLE_SIZE];


//ActivationNumber


//Cons number
int constNumber=7;
	
	
	

/* Main function */
void app_main(void) {

	/* Init Table */
	memset(Table, 0, TABLE_SIZE*sizeof(int));

	/* Create semaphore */
	xSemIncTab=xSemaphoreCreateBinary();
	if(xSemIncTab==NULL){
		ESP_LOGE(TAG,"xSemClock cano be created");
		return;
	}

	xSemDecTab=xSemaphoreCreateBinary();
	if(xSemDecTab==NULL){
		ESP_LOGE(TAG,"xSemClock cano be created");
		return;
	}


	/* Stop scheduler */
	vTaskSuspendAll();

	/* Create Tasks */
	xTaskCreatePinnedToCore(vTaskTimer, "Task timer", STACK_SIZE, (void*)"Task timer", TIMER_TASK_PRIORITY, NULL,CORE_0);
	xTaskCreatePinnedToCore(vTaskIncTable, "vTaskIncTable", STACK_SIZE, (void*)"vTaskIncTable", INC_TABLE_TASK_PRIORITY, NULL,CORE_0);
	xTaskCreatePinnedToCore(vTaskDecTable, "vTaskDecTable", STACK_SIZE, (void*)"vTaskDecTable", DEC_TABLE_TASK_PRIORITY, NULL,CORE_1);

	/* Continue scheduler */
	xTaskResumeAll();

	/* to ensure its exit is clean */
	vTaskDelete(NULL);
}
/*-----------------------------------------------------------*/

void vTaskTimer(void *pvParameters) {
	TickType_t xLastWaketime=xTaskGetTickCount();
	for(;;){
		//wait fo 250ms
		vTaskDelayUntil (&xLastWaketime,pdMS_TO_TICKS(250));
		COMPUTE_IN_TIME_MS(20);
		DISPLAY("Task timer:give sem");
		//noify(clk); 	
		xSemaphoreGive(xSemIncTab);
		xSemaphoreGive(xSemDecTab);
	}
	
}

void vTaskIncTable(void *pvParameters) {
	int ActivationNumber=0;
	
	for(;;){
		xSemaphoreTake(xSemIncTab,portMAX_DELAY);
		if(ActivationNumber==0){
			DISPLAY("Sart vTaskIncTable");
			for(int i=0;i<TABLE_SIZE;i++){
				Table[i]=Table[i]+constNumber;
			//	DISPLAY("INCREMENTE");
			
			}
			DISPLAY("Run vTaskIncTable");
			COMPUTE_IN_TIME_MS(50);
			DISPLAY("END vTaskIncTable");
			ActivationNumber=4;
		}
		else ActivationNumber-=1;
	}
	/* to ensure its exit is clean */
	//vTaskDelete(NULL);
}

void vTaskDecTable(void *pvParameters) {
	for(;;){
		xSemaphoreTake(xSemDecTab,portMAX_DELAY);
		for(int i=0;i<TABLE_SIZE;i++){
			Table[i]=Table[i]-constNumber;
		
		}
		DISPLAY("Start vTaskDecTable");
		COMPUTE_IN_TIME_MS(50);
		DISPLAY("END vTaskDecTable");
	}

	/* to ensure its exit is clean */
//	vTaskDelete(NULL);
}

