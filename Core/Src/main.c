/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "main.h"
#include "cmsis_os.h"
#include <string.h>
#include <stdio.h>
//#include "semphr.h"

//osThreadId_t task1Handle;
//osThreadId_t task2Handle;
//osThreadId_t producerTaskHandle;
//osThreadId_t recieverTaskHandle;

osThreadId_t nav1TaskHandle;
osThreadId_t nav2TaskHandle;
osThreadId_t workerTaskHandle;
osSemaphoreId_t navSemaphoreHandle;
osTimerId_t heartbeatTimerHandle;

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
osMutexId_t uartMutexHandle;
const osMutexAttr_t uartMutex_attrs = {
  .name = "uartMutex"
};

osMessageQueueId_t MessageQueueHandle;
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart2;

/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* USER CODE BEGIN PV */
typedef struct {
    uint32_t roll;
    uint32_t pitch;
} messageQueue_t;
typedef struct {
    uint32_t latitude;
    uint32_t altitude;
    uint32_t direction;
} NavData_t;


NavData_t navData; // global so both tasks can see it
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
void StartDefaultTask(void *argument);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
/*
const osThreadAttr_t task1_attrs = {
		.name = "Task 1",
		.priority = osPriorityNormal,
		.stack_size = 256
};
const osThreadAttr_t task2_attrs = {
		.name = "Task 2",
		.priority = osPriorityHigh,
		.stack_size = 256
};

const osThreadAttr_t producer_attrs = {
    .name = "Producer Task",
    .priority = osPriorityNormal,
    .stack_size = 128 * 4
};
const osThreadAttr_t reciever_attrs = {
    .name = "Receiver Task",
    .priority = osPriorityNormal,
    .stack_size = 128 * 4
};
*/
const osThreadAttr_t nav1_attrs = {
		.name = "Task 1",
		.priority = osPriorityNormal,
		.stack_size = 128*4,
};
const osThreadAttr_t nav2_attrs = {
		.name = "Task 2",
		.priority = osPriorityNormal,
		.stack_size = 128*4,
};
const osThreadAttr_t worker_attrs = {
		.name = "Worker Task",
		.priority = osPriorityAboveNormal,
		.stack_size = 128*4,
};
/*
void Task1(void *arguement){
	const char *msg = "[TELEMETRY] System\r\n";
	for(;;){
	if (osMutexAcquire(uartMutexHandle, osWaitForever) == osOK) {
	HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
	 osMutexRelease(uartMutexHandle);
	}
	osDelay(500);
	}
}
void Task2(void *arguement){
	const char *msg = "[ALERT] Engine\r\n";
	for(;;){
	if (osMutexAcquire(uartMutexHandle, osWaitForever) == osOK) {
	HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
	 osMutexRelease(uartMutexHandle);
	}
	osDelay(3000);
	}
}
*/

/*
void ProducerTask(void *arguement){
	messageQueue_t msg;
	for(;;){
		msg.pitch = 10;
		msg.roll = 34;
		osMessageQueuePut(MessageQueueHandle, &msg, 0, 0);
		osDelay(1000);

	}
}

void RecieverTask(void *arguement){
	messageQueue_t msg;
	for(;;){
		if (osMessageQueueGet(MessageQueueHandle, &msg, 0, osWaitForever) == osOK)
			  {
			char buf[50];
			snprintf(buf, sizeof(buf), "Pitch: %lu, Roll: %lu\r\n", msg.pitch, msg.roll);
			HAL_UART_Transmit(&huart2, (uint8_t*)buf, strlen(buf), HAL_MAX_DELAY);
			  }

	}
}
*/
void nav1Task(void *arguement){
	for(;;){
		osSemaphoreAcquire(navSemaphoreHandle,osWaitForever);
		navData.latitude = 48;
		navData.altitude = 1200;
		navData.direction = 270;
		char buf[50];
		snprintf(buf, sizeof(buf), "[NAV1] Writing - Lat: %lu, Alt: %lu, Dir: %lu\r\n", navData.latitude, navData.altitude, navData.direction);
		HAL_UART_Transmit(&huart2, (uint8_t*)buf, strlen(buf), HAL_MAX_DELAY);
		osSemaphoreRelease(navSemaphoreHandle);
		osDelay(1000);


	}
}
void nav2Task(void *arguement){

	for(;;){
	osStatus_t status = osSemaphoreAcquire(navSemaphoreHandle, osWaitForever);

	if (status == osOK) {
		char buf[50];
		snprintf(buf, sizeof(buf), "[NAV2] Reading - Lat: %lu, Alt: %lu, Dir: %lu\r\n",
		         navData.latitude, navData.altitude, navData.direction);
		HAL_UART_Transmit(&huart2, (uint8_t*)buf, strlen(buf), HAL_MAX_DELAY);
		osSemaphoreRelease(navSemaphoreHandle);
		osDelay(1000);
	}
	}

}
void workerTask(void *argument){
    for(;;){
        HAL_UART_Transmit(&huart2, (uint8_t*)"[STATUS] Navigation: Online\r\n",
                          strlen("[STATUS] Navigation: Online\r\n"), HAL_MAX_DELAY);
        osDelay(2000);
        HAL_UART_Transmit(&huart2, (uint8_t*)"[STATUS] Sensors: Online\r\n",
                          strlen("[STATUS] Sensors: Online\r\n"), HAL_MAX_DELAY);
        osDelay(2000);
        HAL_UART_Transmit(&huart2, (uint8_t*)"[STATUS] Comms: Online\r\n",
                          strlen("[STATUS] Comms: Online\r\n"), HAL_MAX_DELAY);
        osDelay(2000);
    }
}
void heartbeatCallback(void *argument)
{
    HAL_UART_Transmit(&huart2, (uint8_t*)"[HEARTBEAT] System alive\r\n", 26, HAL_MAX_DELAY);
}


int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  char msg[] = "Hello World\r\n";
  HAL_UART_Transmit(&huart2, (uint8_t*)msg, sizeof(msg)-1, HAL_MAX_DELAY);
  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  //uartMutexHandle = osMutexNew(&uartMutex_attrs);
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  navSemaphoreHandle = osSemaphoreNew(1, 1, NULL);
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  heartbeatTimerHandle = osTimerNew(heartbeatCallback, osTimerPeriodic, NULL, NULL);
  osTimerStart(heartbeatTimerHandle, 1000);
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  //MessageQueueHandle = osMessageQueueNew(5, sizeof(messageQueue_t), NULL);
  //if(MessageQueueHandle == NULL)
  //{
      // queue creation failed
    //  while(1);
 // }
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  //task1Handle = osThreadNew(Task1, NULL, &task1_attrs);
  //task2Handle = osThreadNew(Task2, NULL, &task2_attrs);
  //1producerTaskHandle = osThreadNew(ProducerTask,NULL,&producer_attrs);
  //recieverTaskHandle = osThreadNew(RecieverTask, NULL, &reciever_attrs);
 // nav1TaskHandle = osThreadNew(nav1Task, NULL, &nav1_attrs);
  //nav2TaskHandle = osThreadNew(nav2Task, NULL, &nav2_attrs);
  workerTaskHandle = osThreadNew(workerTask, NULL, &worker_attrs);
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}




/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI48;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 38400;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END 5 */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM14 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM14)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
