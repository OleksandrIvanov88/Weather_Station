/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "tim.h"
#include "sensor.h"
#include "display.h"
#include "blootuoth.h"
#include "internet.h"
#include "usart.h"
#include "debug.h"
#include <string.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MINUTES 1
#define HTTP_DELAY MINUTES * 60 * 1000

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for thread_sensors */
osThreadId_t thread_sensorsHandle;
const osThreadAttr_t thread_sensors_attributes = {
  .name = "thread_sensors",
  .priority = (osPriority_t) osPriorityHigh,
  .stack_size = 512 * 4
};
/* Definitions for thread_lcd */
osThreadId_t thread_lcdHandle;
const osThreadAttr_t thread_lcd_attributes = {
  .name = "thread_lcd",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 512 * 4
};
/* Definitions for thread_btooth */
osThreadId_t thread_btoothHandle;
const osThreadAttr_t thread_btooth_attributes = {
  .name = "thread_btooth",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 512 * 4
};
/* Definitions for thread_reports */
osThreadId_t thread_reportsHandle;
const osThreadAttr_t thread_reports_attributes = {
  .name = "thread_reports",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 4096 * 4
};
/* Definitions for queue_sensors */
osMessageQueueId_t queue_sensorsHandle;
const osMessageQueueAttr_t queue_sensors_attributes = {
  .name = "queue_sensors"
};
/* Definitions for queue_reportMetar */
osMessageQueueId_t queue_reportMetarHandle;
const osMessageQueueAttr_t queue_reportMetar_attributes = {
  .name = "queue_reportMetar"
};
/* Definitions for queue_reportTAF */
osMessageQueueId_t queue_reportTAFHandle;
const osMessageQueueAttr_t queue_reportTAF_attributes = {
  .name = "queue_reportTAF"
};
/* Definitions for mutex_sensors */
osMutexId_t mutex_sensorsHandle;
const osMutexAttr_t mutex_sensors_attributes = {
  .name = "mutex_sensors"
};
/* Definitions for mutex_metar */
osMutexId_t mutex_metarHandle;
const osMutexAttr_t mutex_metar_attributes = {
  .name = "mutex_metar"
};
/* Definitions for mutex_taf */
osMutexId_t mutex_tafHandle;
const osMutexAttr_t mutex_taf_attributes = {
  .name = "mutex_taf"
};
/* Definitions for mutex_console */
osMutexId_t mutex_consoleHandle;
const osMutexAttr_t mutex_console_attributes = {
  .name = "mutex_console"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void thread_sensors_func(void *argument);
void thread_lcd_func(void *argument);
void thread_btooth_func(void *argument);
void thread_reports_func(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Hook prototypes */
void configureTimerForRunTimeStats(void);
unsigned long getRunTimeCounterValue(void);
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName);
void vApplicationMallocFailedHook(void);

/* USER CODE BEGIN 1 */
/* Functions needed when configGENERATE_RUN_TIME_STATS is on */
__weak void configureTimerForRunTimeStats(void)
{
	__HAL_DBGMCU_FREEZE_TIM2();
	HAL_TIM_Base_Start(&htim2);
}
__weak unsigned long getRunTimeCounterValue(void)
{
	return __HAL_TIM_GET_COUNTER(&htim2);
}
/* USER CODE END 1 */

/* USER CODE BEGIN 4 */
__weak void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName)
{
   /* Run time stack overflow checking is performed if
   configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2. This hook function is
   called if a stack overflow is detected. */
#ifdef CONSOLE_DEBUG_ERROR
	char *error = "*****Error!!!vApplicationStackOverflowHook().*****\r\n";
	console_display(&huart2, error, strlen(error));
#endif
	while(1);
}
/* USER CODE END 4 */

/* USER CODE BEGIN 5 */
__weak void vApplicationMallocFailedHook(void)
{
   /* vApplicationMallocFailedHook() will only be called if
   configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h. It is a hook
   function that will get called if a call to pvPortMalloc() fails.
   pvPortMalloc() is called internally by the kernel whenever a task, queue,
   timer or semaphore is created. It is also called by various parts of the
   demo application. If heap_1.c or heap_2.c are used, then the size of the
   heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
   FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
   to query the size of free heap space that remains (although it does not
   provide information on how the remaining heap might be fragmented). */
#ifdef CONSOLE_DEBUG_ERROR
	char *error = "*****Error!!!vApplicationMallocFailedHook().*****\r\n";
	console_display(&huart2, error, strlen(error));
#endif
	while(1);
}
/* USER CODE END 5 */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */
  /* Create the mutex(es) */
  /* creation of mutex_sensors */
  mutex_sensorsHandle = osMutexNew(&mutex_sensors_attributes);

  /* creation of mutex_metar */
  mutex_metarHandle = osMutexNew(&mutex_metar_attributes);

  /* creation of mutex_taf */
  mutex_tafHandle = osMutexNew(&mutex_taf_attributes);

  /* creation of mutex_console */
  mutex_consoleHandle = osMutexNew(&mutex_console_attributes);

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of queue_sensors */
  queue_sensorsHandle = osMessageQueueNew (16, sizeof(sensors_t), &queue_sensors_attributes);

  /* creation of queue_reportMetar */
  queue_reportMetarHandle = osMessageQueueNew (16, sizeof(report_metar_t), &queue_reportMetar_attributes);

  /* creation of queue_reportTAF */
  queue_reportTAFHandle = osMessageQueueNew (16, sizeof(report_taf_t), &queue_reportTAF_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of thread_sensors */
  thread_sensorsHandle = osThreadNew(thread_sensors_func, NULL, &thread_sensors_attributes);

  /* creation of thread_lcd */
  thread_lcdHandle = osThreadNew(thread_lcd_func, NULL, &thread_lcd_attributes);

  /* creation of thread_btooth */
  thread_btoothHandle = osThreadNew(thread_btooth_func, NULL, &thread_btooth_attributes);

  /* creation of thread_reports */
  thread_reportsHandle = osThreadNew(thread_reports_func, NULL, &thread_reports_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_thread_sensors_func */
/**
  * @brief  Function implementing the thread_sensors thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_thread_sensors_func */
void thread_sensors_func(void *argument)
{
  /* USER CODE BEGIN thread_sensors_func */
	if(!sensors_init()) {
		osThreadTerminate(thread_sensorsHandle);
	}
	sensors_t sensor_data = {0};
  /* Infinite loop */
	for (;;) {
		sensor_get_data(&sensor_data);
		if(osMessageQueuePut(queue_sensorsHandle, &sensor_data, 0, 9) != osOK) {
			/*Send unsuccessufuly*/
#ifdef CONSOLE_DEBUG_ERROR
			char *error = "*****Error!!!osMessageQueuePut() - sensor.*****\r\n";
			console_display(&huart2, error, strlen(error));
#endif
		}
		osDelay(100);
	}
  /* USER CODE END thread_sensors_func */
}

/* USER CODE BEGIN Header_thread_lcd_func */
/**
* @brief Function implementing the thread_lcd thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_thread_lcd_func */
void thread_lcd_func(void *argument)
{
  /* USER CODE BEGIN thread_lcd_func */
	lcd_int();
	sensors_t sensor_data = {0};
	report_metar_t metar_data = {0};
	report_taf_t taf_data;
  /* Infinite loop */
	for (;;) {
		if (osMessageQueueGet(queue_sensorsHandle, &sensor_data, NULL, 0) == osOK) {
			display_sensor_data(&sensor_data);
			if (osMutexAcquire(mutex_sensorsHandle, osWaitForever) == osOK) {
				write_sensor_data_bt(&sensor_data);
				osMutexRelease(mutex_sensorsHandle);
			}
		}
		if (osMessageQueueGet(queue_reportMetarHandle, &metar_data, NULL, 0) == osOK) {
			display_metar(&metar_data);
			if (osMutexAcquire( mutex_metarHandle, osWaitForever) == osOK) {
				write_metar_data_bt(&metar_data);
				osMutexRelease( mutex_metarHandle);
			}
		}
		if (osMessageQueueGet(queue_reportTAFHandle, &taf_data, NULL, 0) == osOK) {
			display_taf(&taf_data);
			if (osMutexAcquire(mutex_tafHandle, osWaitForever) == osOK) {
				write_taf_data_bt(&taf_data);
				osMutexRelease(mutex_tafHandle);
			}
		}
		osDelay(1);
	}
  /* USER CODE END thread_lcd_func */
}

/* USER CODE BEGIN Header_thread_btooth_func */
/**
* @brief Function implementing the thread_btooth thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_thread_btooth_func */
void thread_btooth_func(void *argument)
{
  /* USER CODE BEGIN thread_btooth_func */
	blootuoth_int();
  /* Infinite loop */
	for (;;) {
		parcin_bt_command();
		osDelay(50);
	}
  /* USER CODE END thread_btooth_func */
}

/* USER CODE BEGIN Header_thread_reports_func */
/**
* @brief Function implementing the thread_reports thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_thread_reports_func */
void thread_reports_func(void *argument)
{
  /* USER CODE BEGIN thread_reports_func */
	wifi_init();
  /* Infinite loop */
	for (;;) {
		htpp_request_metar();
		htpp_request_taf();
		osDelay(HTTP_DELAY);
	}
  /* USER CODE END thread_reports_func */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
