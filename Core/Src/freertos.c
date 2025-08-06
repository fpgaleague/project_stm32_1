/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
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
#include "queue.h"

#include "aht20.h"
#include "BH1750.h"

#include "usart.h"
#include <string.h>
#include "i2c.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct {
	float Temp;
	float Humidity;
	float light;
} Sensor_data;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
  QueueHandle_t sensorDataQueue;  //添加的队列句

/* USER CODE END Variables */
/* Definitions for task_aht20 */
osThreadId_t task_aht20Handle;
const osThreadAttr_t task_aht20_attributes = {
  .name = "task_aht20",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for task_bh1750 */
osThreadId_t task_bh1750Handle;
const osThreadAttr_t task_bh1750_attributes = {
  .name = "task_bh1750",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for task_esp01s */
osThreadId_t task_esp01sHandle;
const osThreadAttr_t task_esp01s_attributes = {
  .name = "task_esp01s",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityBelowNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartTask02(void *argument);
void StartTask03(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
	sensorDataQueue = xQueueCreate(1, sizeof(Sensor_data));
	if (sensorDataQueue != NULL)   //这个判断相当于检查这个缓冲区是否申请成功，如果成功才能往里存数据�???
	{
		HAL_UART_Transmit(&huart1, (uint8_t *)"Queue created successfully!\r\n",
		strlen("Queue created successfully!\r\n"), HAL_MAX_DELAY);
	    // 队列创建失败处理
	    //Error_Handler();
	}
	else
	{
		HAL_UART_Transmit(&huart1, (uint8_t *)"Queue created Error!\r\n",
		strlen("Queue created Error!\r\n"), HAL_MAX_DELAY);
	}
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of task_aht20 */
  task_aht20Handle = osThreadNew(StartDefaultTask, NULL, &task_aht20_attributes);

  /* creation of task_bh1750 */
  task_bh1750Handle = osThreadNew(StartTask02, NULL, &task_bh1750_attributes);

  /* creation of task_esp01s */
  task_esp01sHandle = osThreadNew(StartTask03, NULL, &task_esp01s_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  AHT20_Init();

  float Temp,Humi;
  Sensor_data sensorData; // 定义结构体变

  TickType_t lastWakeTime = xTaskGetTickCount();
  for(;;)
  {

    AHT20_Read(&Temp,&Humi);

    sensorData.Temp = Temp;
    sensorData.Humidity = Humi;
    sensorData.light = 0; // 光照数据由另

    //sprintf(message,"Tempeturate:%.2f,Humidity:%.2f %%\r\n",Temp,Humi);
    xQueueOverwrite(sensorDataQueue, &sensorData);
    //key = xQueueSend(sensorDataQueue , &sensorData, portMAX_DELAY); //队列写入数据
    //if(key != pdPASS)

    osDelayUntil(lastWakeTime + 1000);  // 精确1秒周
    lastWakeTime = xTaskGetTickCount();
    //osDelay(1000);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartTask02 */
/**
* @brief Function implementing the task_bh1750 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask02 */
void StartTask02(void *argument)
{
  /* USER CODE BEGIN StartTask02 */
  /* Infinite loop */
  BH1750_Init();

  for(;;)
  {
	//char message1[50];
	uint8_t DataBuff[2];//存储数组
	float LightData;//光照强度

	Sensor_data sensorData;

	if(xQueuePeek(sensorDataQueue, &sensorData, pdMS_TO_TICKS(100)) == pdPASS) {
	      BH1750_ReadData(DataBuff, 2);
	      LightData=((DataBuff[0]<<8)+DataBuff[1])/1.2f;//数据转换成光强度，单位lx
	      sensorData.light = LightData;
	      xQueueOverwrite(sensorDataQueue, &sensorData);  // 原子化更新
	    }


	// 格式化字符串
	//sprintf(message1, "Light: %d lux\r\n", LightData_Hex);
	//HAL_UART_Transmit(&huart1, (uint8_t*)message1, strlen(message1), HAL_MAX_DELAY);

    osDelay(10);
  }
  /* USER CODE END StartTask02 */
}

/* USER CODE BEGIN Header_StartTask03 */
/**
* @brief Function implementing the task_esp01s thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask03 */
void StartTask03(void *argument)
{
  /* USER CODE BEGIN StartTask03 */
  /* Infinite loop */
	Sensor_data receivedData;
	char message[100];

  for(;;)
  {
	  // 从队列接收完整数据包
	  if(xQueueReceive(sensorDataQueue, &receivedData, pdMS_TO_TICKS(1000)) == pdPASS)
	  {
	  // 格式化所有传感器数据
	  sprintf(message, "Temp:%.2fC, Humi:%.2f%%, Light:%.2flux\r\n",
	          receivedData.Temp, receivedData.Humidity, receivedData.light);

	  // 通过串口发到ESP模块
	  HAL_UART_Transmit(&huart1, (uint8_t*)message, strlen(message), 100);
	  }
	  osDelay(100);
  }
  /* USER CODE END StartTask03 */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

