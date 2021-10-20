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
const uint32_t DEC_TABLE_TASK_PRIORITY = 3;
const uint32_t INSPECTOR_TASK_PRIORITY = 4; 



/* Communications */
SemaphoreHandle_t xSemIncTab;
SemaphoreHandle_t xSemDecTab;
SemaphoreHandle_t xMutex;


/* Tasks */
void vTaskTimer(void *pvParameters);
void vTaskIncTable(void *pvParameters);
void vTaskDecTable(void *pvParameters);
void vTaskInspector(void *pvParameters);

/* Datas */

int Table[TABLE_SIZE];

//Cons number
int constNumber=7;
	

/* Main function */
void app_main(void) {

	/* Init Table */
	memset(Table,0,TABLE_SIZE*sizeof(int));
	for(int i=0;i<TABLE_SIZE;i++){
		Table[i]=i;
	}

	/* Create semaphore */
	xSemIncTab=xSemaphoreCreateBinary();
	if(xSemIncTab==NULL){
		ESP_LOGE(TAG,"xSemClock cannot be created");
		return;
	}

	xSemDecTab=xSemaphoreCreateBinary();
	if(xSemDecTab==NULL){
		ESP_LOGE(TAG,"xSemClock cannot be created");
		return;
	}
	//Create Mutex
	xMutex=xSemaphoreCreateMutex();
	if(xMutex==NULL){
		ESP_LOGE(TAG,"xMutex cannot be created");
		return;
	}

	/* Stop scheduler */
	vTaskSuspendAll();

	/* Create Tasks */
	xTaskCreatePinnedToCore(vTaskTimer, "Task timer", STACK_SIZE, (void*)"Task timer", TIMER_TASK_PRIORITY, NULL,CORE_0);
	xTaskCreatePinnedToCore(vTaskIncTable, "vTaskIncTable", STACK_SIZE, (void*)"vTaskIncTable", INC_TABLE_TASK_PRIORITY, NULL,CORE_0);
	xTaskCreatePinnedToCore(vTaskDecTable, "vTaskDecTable", STACK_SIZE, (void*)"vTaskDecTable", DEC_TABLE_TASK_PRIORITY, NULL,CORE_0);
	xTaskCreatePinnedToCore(vTaskInspector,"vTaskInspector",STACK_SIZE, (void*)"vTaskInspector", INSPECTOR_TASK_PRIORITY, NULL,CORE_0);
	/* Continue scheduler */
	xTaskResumeAll();

	/* to ensure its exit is clean */
	vTaskDelete(NULL);
}
/*-----------------------------------------------------------*/

void vTaskTimer(void *pvParameters) {
	TickType_t xLastWaketime=xTaskGetTickCount();
	for(;;){
		//wait for 250ms
		vTaskDelayUntil (&xLastWaketime,pdMS_TO_TICKS(250));
		COMPUTE_IN_TIME_MS(20);
		DISPLAY("Task timer:give sem");
		// Release the semaphore so that the creating function can finish
		xSemaphoreGive(xSemIncTab);
		xSemaphoreGive(xSemDecTab);

	}
	
}

void vTaskIncTable(void *pvParameters) {
	int ActivationNumber=0;
	for(;;){
		xSemaphoreTake(xSemIncTab,portMAX_DELAY);
		xSemaphoreTake(xMutex, portMAX_DELAY);
		if(ActivationNumber==0){
			DISPLAY("Start vTaskIncTable");
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
		xSemaphoreGive(xMutex);

	}
	/* to ensure its exit is clean */
	//vTaskDelete(NULL);
}

void vTaskDecTable(void *pvParameters) {
	for(;;){
		xSemaphoreTake(xSemDecTab,portMAX_DELAY);
		xSemaphoreTake(xMutex, portMAX_DELAY);
		for(int i=0;i<TABLE_SIZE;i++){
			Table[i]=Table[i]-constNumber;
		}
		DISPLAY("Start vTaskDecTable");
		COMPUTE_IN_TIME_MS(50);
		DISPLAY("END vTaskDecTable");
		xSemaphoreGive(xMutex);

	}

	/* to ensure its exit is clean */
//	vTaskDelete(NULL);
}

void vTaskInspector(void *pvParameters){
	int ActivationNumber=0;
	
	for(;;){
		// Do nothing until mutex has been returned (maximum delay)
		xSemaphoreTake(xMutex, portMAX_DELAY);
		DISPLAY("Task Inspector is checking");
		int reference=Table[0];
		bool error=false;
		for(int i=0;i<TABLE_SIZE;i++){
			COMPUTE_IN_TIME_US(100);
			if(Table[i]!=reference+i){
				error=true;
			}
		}
		DISPLAY("Task Inspector ended its checking");
		if(error==true){
			ESP_LOGE(TAG,"Consistency error in he Table variable");
			exit(1);
		}
		
		xSemaphoreGive(xMutex);
		vTaskDelay(pdMS_TO_TICKS(20));
	}
	
}

