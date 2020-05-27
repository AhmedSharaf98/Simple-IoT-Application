/* USER CODE BEGIN Header */
/**
  **************************
  * @file           : main.c
  * @brief          : Main program body
  **************************
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
  **************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "semphr.h"
#include "stdlib.h"
#include "stdio.h"
#include "task.h"
#include "string.h"

UART_HandleTypeDef huart1;

/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 128 * 4
};
/* Definitions for Task1_Evaluate */
osThreadId_t Task1_EvaluateHandle;
const osThreadAttr_t Task1_Evaluate_attributes = {
  .name = "Task1_Evaluate",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 128 * 4
};
/* Definitions for Task2_UARTSend */
osThreadId_t Task2_UARTSendHandle;
const osThreadAttr_t Task2_UARTSend_attributes = {
  .name = "Task2_UARTSend",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 128 * 4
};
/* Definitions for Task3_UARTRecei */
osThreadId_t Task3_UARTReceiHandle;
const osThreadAttr_t Task3_UARTRecei_attributes = {
  .name = "Task3_UARTRecei",
  .priority = (osPriority_t) osPriorityHigh,
  .stack_size = 128 * 4
};
/* Definitions for RQ */
osMessageQueueId_t RQHandle;
const osMessageQueueAttr_t RQ_attributes = {
  .name = "RQ"
};
/* Definitions for SQ */
osMessageQueueId_t SQHandle;
const osMessageQueueAttr_t SQ_attributes = {
  .name = "SQ"
};
/* Definitions for UARTSem */
osSemaphoreId_t UARTSemHandle;
const osSemaphoreAttr_t UARTSem_attributes = {
  .name = "UARTSem"
};
/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
void StartDefaultTask(void *argument);
void StartTask01(void *argument);
void StartTask02(void *argument);
void StartTask03(void *argument);

char receiveBuffer [1];
int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  osKernelInitialize();
	vSemaphoreCreateBinary(UARTSemHandle);
	xSemaphoreTake(UARTSemHandle, 0);

  RQHandle = osMessageQueueNew (32, 1, &RQ_attributes);
  SQHandle = osMessageQueueNew (32, 8, &SQ_attributes);

  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);
  Task1_EvaluateHandle = osThreadNew(StartTask01, NULL, &Task1_Evaluate_attributes);
  Task2_UARTSendHandle = osThreadNew(StartTask02, NULL, &Task2_UARTSend_attributes);
  Task3_UARTReceiHandle = osThreadNew(StartTask03, NULL, &Task3_UARTRecei_attributes);
	
	__HAL_UART_ENABLE_IT(&huart1,UART_IT_RXNE);
	
  osKernelStart();
  while (1)
  {	
  }
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();

}

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
} 

int evaluate(int a, int b, char op){
	return (op == '+')? a+b :(op == '*')? a*b : a-b;
}

void StartTask01(void *argument)
{
	char rec, op;
	int first , second , count;
	first = second = count = 0 ;
	char buffer[32];
	char output[32];
  for(;;)
  {
		if( xQueueReceive(RQHandle, &rec, 1000) == pdPASS ) {
			memset(output, 0, 32);
			if (rec=='\r'){
				buffer[count]=0;
				second=atoi(buffer);
				count=0;
				int result = evaluate(first,second, op);
				sprintf(output,"= %d\n\n", result);
				xQueueSendToBack(SQHandle, output, 10000);
			} else if (rec=='+' || rec=='-' || rec == '*'){
				op=rec;
				buffer[count]=0;
				first=atoi(buffer);
				count=0;
				sprintf(output,"%c\n", rec);
				xQueueSendToBack(SQHandle, output, 10000);
			} else {
				buffer[count++]=rec;
				sprintf(output,"%c\n", rec);
				xQueueSendToBack(SQHandle, output, 10000);
			}
		}
    //osDelay(1);
  }
}

void StartTask02(void *argument)
{
	char buffer[8];
	memset(buffer, 0, 8);
  for(;;)
  {
		if( xQueueReceive(SQHandle, buffer, 10000) == pdPASS ){
			buffer[7]='\r';
			HAL_UART_Transmit(&huart1, (uint8_t *)buffer, 8, 3000);
			memset(buffer, 0, 8);
		}
  }
}

void StartTask03(void *argument)
{
  for(;;)
  {
		if(xSemaphoreTake(UARTSemHandle, 9999999)) {
			// Process the interrupt
			taskENTER_CRITICAL();
			xQueueSendToBack(RQHandle, receiveBuffer, 500);
			taskEXIT_CRITICAL();
		}
  }
}

void USART1_IRQHandler(void){
	HAL_UART_Receive(&huart1, (uint8_t *)receiveBuffer, sizeof receiveBuffer, 300);
	xSemaphoreGiveFromISR(UARTSemHandle, NULL);
	HAL_UART_IRQHandler(&huart1);
}

 /**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM1) {
    HAL_IncTick();
  }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{

}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 

}
#endif /* USE_FULL_ASSERT */

/******** (C) COPYRIGHT STMicroelectronics **END OF FILE*/
