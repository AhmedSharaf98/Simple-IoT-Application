
#include "main.h"
I2C_HandleTypeDef hi2c1;
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_I2C1_Init(void);

uint8_t msg[13] =  "Hello World\n\r";
char buffer [16];
short bufferLocation = -1;
uint8_t rec[1];
int process = 0;

const char TOGGLE_LED[16] = "ToggleLed";
const char GET_TIME[16] = "GetTime";
const char SET_ALARM[16] = "SetAlarm";

/* USER CODE BEGIN 0 */
uint8_t secbuffer [2], minbuffer [2], hourbuffer [2];
uint8_t a1secbuffer [2], a1minbuffer [2], a1hourbuffer [2], contbuffer [2], statusbuffer [2];
uint8_t out[] = {0,0,':',0,0,':',0,0,'\r','\n'};

uint8_t hexToAscii(uint8_t n)//4-bit hex value converted to an ascii character
{
 if (n>=0 && n<=9) n = n + '0';
 else n = n - 10 + 'A';
 return n;
}

void setClock(){ 
	 // seconds
	 secbuffer[0] = 0x00; //register address
	 secbuffer[1] = 0x00; //data to put in register --> 0 sec
	 HAL_I2C_Master_Transmit(&hi2c1, 0xD0, secbuffer, 2, 10);
	 // minutes
	 minbuffer[0] = 0x01; //register address
	 minbuffer[1] = 0x30; //data to put in register --> 30 min
	 HAL_I2C_Master_Transmit(&hi2c1, 0xD0, minbuffer, 2, 10);
	 // hours
	 hourbuffer[0] = 0x02; //register address
	 hourbuffer[1] = 0x11; //data to put in register 00010001 --> 11 am
	 HAL_I2C_Master_Transmit(&hi2c1, 0xD0, hourbuffer, 2, 10);
}

void getTime(){
	HAL_I2C_Master_Transmit(&hi2c1, 0xD0, secbuffer, 1, 10);
	 //read data of register 00h to secbuffer[1]
	 HAL_I2C_Master_Receive(&hi2c1, 0xD1, secbuffer+1, 1, 10);
	 //prepare UART output
	 out[6] = hexToAscii(secbuffer[1] >> 4 );
	 out[7] = hexToAscii(secbuffer[1] & 0x0F );
	 HAL_I2C_Master_Transmit(&hi2c1, 0xD0, minbuffer, 1, 10);
	 HAL_I2C_Master_Receive(&hi2c1, 0xD1, minbuffer+1, 1, 10);
	 out[3] = hexToAscii(minbuffer[1] >> 4 );
	 out[4] = hexToAscii(minbuffer[1] & 0x0F );
	 HAL_I2C_Master_Transmit(&hi2c1, 0xD0, hourbuffer, 1, 10);
	 HAL_I2C_Master_Receive(&hi2c1, 0xD1, hourbuffer+1, 1, 10);
	 out[0] = hexToAscii(hourbuffer[1] >> 4);
	 out[1] = hexToAscii(hourbuffer[1] & 0x0F);
}

void handleRequest(){
	HAL_UART_Transmit(&huart2, "Handling\n\r", 10,500);
		if(strcmp(buffer, TOGGLE_LED) == 0){
			HAL_UART_Transmit(&huart2, "Togglled\n\r", 10,500);
			HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_3);
		}
		if(strcmp(buffer, GET_TIME) == 0){
			HAL_UART_Transmit(&huart2, "Getting Time\n\r", 14,500);
			getTime();
			HAL_UART_Transmit(&huart2,out, sizeof(out), 10);
		} else {
			HAL_UART_Transmit(&huart2, buffer, 16,500);
		}
		bufferLocation = 0;
		memset(buffer, 0, 16);
		process = 0;
}


int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_USART1_UART_Init();
  MX_I2C1_Init();
	
	if (HAL_I2C_IsDeviceReady(&hi2c1, 0xD0, 10, HAL_MAX_DELAY) == HAL_OK)
	 {
			for (int i = 1; i<=10;i++){
				 HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_3);
				 HAL_Delay(100);
			}
	 }
	setClock();
  __HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);
	__HAL_UART_ENABLE_IT(&huart1, UART_IT_TC);
	__HAL_UART_ENABLE_IT(&huart2, UART_IT_RXNE);
	__HAL_UART_ENABLE_IT(&huart2, UART_IT_TC);
	
  while (1)
  {
		if(process){
			__HAL_UART_DISABLE_IT(&huart1, UART_IT_RXNE);
			__HAL_UART_DISABLE_IT(&huart2, UART_IT_RXNE);
				handleRequest();
			__HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);
			__HAL_UART_ENABLE_IT(&huart2, UART_IT_RXNE);
		}	
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_USART2
                              |RCC_PERIPHCLK_I2C1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure the main internal regulator output voltage 
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x00000E14;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Analogue filter 
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Digital filter 
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
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
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
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
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 9600;
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
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);

  /*Configure GPIO pin : PB3 */
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/**
  * @brief This function handles USART1 global interrupt.
  */
void USART1_IRQHandler(void)
{
  HAL_UART_IRQHandler(&huart1);
	HAL_UART_Receive_IT(&huart1, rec, 1);
	if(rec[0] == '*'){
		process = 1;
	} else {
		buffer[bufferLocation++] = rec[0];
	}
}

/**
  * @brief This function handles USART2 global interrupt.
  */

uint8_t temp[1] =  "F";
void USART2_IRQHandler(void)
{
  HAL_UART_IRQHandler(&huart2);
	HAL_UART_Receive_IT(&huart2, temp, 1);
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
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
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
