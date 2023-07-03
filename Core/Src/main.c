/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ssd1306_images.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define X_TEXT_BEGIN           10
#define Y_TEXT_BEGIN           12
#define X_ICON_BEGIN           102
#define Y_ICON_BEGIN           8
#define DEFAULT_PRESS_COUNTER  10000
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
uint32_t pressCount;
uint8_t testEngineState;
TESTER_STATE testerState;
TESTER_DATA gTesterCurData;
TESTER_DATA gTesterPrevData;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
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
  uint8_t waitCounter;
  uint8_t needToRedrawIcon = 1;
  gTesterCurData.status.pressCount = DEFAULT_PRESS_COUNTER;
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  Buttons_Init();
  ssd1306_Init();
  testerState = STATE_CONFIG;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  switch(testerState){
	  	  case STATE_CONFIG:
	  		  if(needToRedrawIcon){
	  			  needToRedrawIcon = 0;
	  			  clear_icon();
	  			  ssd1306_DrawBitmap(X_ICON_BEGIN, Y_ICON_BEGIN, CONFIG_IMG, CONFIG_IMG_WIDTH, CONFIG_IMG_HEIGHT, White);
	  			  ssd1306_UpdateScreen();
	  		  }
	  		  if( gButtons.btnPlus.click ){
	  			  gTesterCurData.status.pressCount++;
	  		    }
	  		  else if(gButtons.btnMinus.click){
	  			  gTesterCurData.status.pressCount--;
	  		  }
	  		  break;
	  	  case STATE_PRESS:
	  		  if(needToRedrawIcon){
	  			  needToRedrawIcon = 0;
	  			  clear_icon();
	  			  ssd1306_UpdateScreen();
	  		  }
	  //		  HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_1);   //Activating press relay
	  		  gTesterCurData.status.pressCount--;
	  		  testerState = STATE_WAIT;

	  //		  HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
	  		  HAL_Delay(10);
	  //		  break;
	  	  case STATE_WAIT:
	  		  //FIXME
	  		  if(++waitCounter != 20){
	  			  HAL_Delay(1);
	  		  }
	  		  else{
	  			  waitCounter = 0;
	  			  testerState = STATE_PRESS;
	  		  }
	  //		  if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_2) == RESET){
	  //			  HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
	  //			  testerState = STATE_PRESS;
	  //		  }
	  //		  else{
	  //			  HAL_Delay(1);
	  //		  }
	  		  break;
	  	  case STATE_PAUSE:
	  		  if(needToRedrawIcon){
	  			  needToRedrawIcon = 0;
	  			  clear_icon();
	  			  ssd1306_DrawBitmap(X_ICON_BEGIN, Y_ICON_BEGIN, PAUSE_IMG, PAUSE_IMG_WIDTH, PAUSE_IMG_HEIGHT, White);
	  			  ssd1306_UpdateScreen();
	  		  }
	  		  HAL_Delay(1);
	  		  break;


	  	  case STATE_FINISH:
	  		  HAL_Delay(1);
	  		  break;



	  	  };
	  //	  HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_1);
	  	  if (testerState != STATE_CONFIG){
	  		  if(gButtons.btnOk.longPressEnd){
	  				  testerState = STATE_CONFIG;
	  				  needToRedrawIcon = 1;
	  				  gTesterCurData.status.pressCount = DEFAULT_PRESS_COUNTER;
	  		  }
	  		  else if(gButtons.btnOk.click){
	  			  testerState = (testerState == STATE_PAUSE ? STATE_PRESS : STATE_PAUSE);
	  			  needToRedrawIcon = 1;
	  		  }
	  	  }
	  	  else if(gButtons.btnOk.click){
	  //		  if(testerState == STATE_CONFIG){
	  		  testerState = STATE_PRESS;
	  		  needToRedrawIcon = 1;
	  //		  }

	  	  }
	  //	  HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
	  //	  HAL_Delay(500);


	  	  Buttons_Task();
	  	  if(gTesterCurData.val != gTesterPrevData.val){
	  //		  ssd1306_DrawPixel(0,40,White);
	  		  //ssd1306_DrawBitmap(102, 8, PAUSE_IMG, PAUSE_IMG_WIDTH, PAUSE_IMG_HEIGHT, White);
	  		  memcpy(&gTesterPrevData.status, &gTesterCurData.status, sizeof(gTesterPrevData.status));
	  		  int32_to_str(gTesterCurData.status.pressCount, gTesterCurData.status.str_pressCount);
	  		  clear_text();
	  		  ssd1306_SetCursor(X_TEXT_BEGIN, Y_TEXT_BEGIN);
	  		  ssd1306_WriteString(gTesterCurData.status.str_pressCount, Font_6x8, White);
	  		  ssd1306_UpdateScreen();
	  	  }
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
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
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
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
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

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
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PB12 PB13 PB14 */
  GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void int32_to_str(int32_t num, char* str) {
    snprintf(str, MAX_INT32_STR_LENGTH, "%d", num);
}

//void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim){
//
//}

void clear_text(){
	ssd1306_FillRectangle(X_TEXT_BEGIN, Y_TEXT_BEGIN,
					  X_TEXT_BEGIN + MAX_INT32_STR_LENGTH*Font_6x8.FontWidth,
					  Y_TEXT_BEGIN + Font_6x8.FontHeight, Black);
}

void clear_icon(){
	ssd1306_FillRectangle(X_ICON_BEGIN, Y_ICON_BEGIN,
					  X_ICON_BEGIN + 16, Y_ICON_BEGIN + 16, Black);
}
/* USER CODE END 4 */

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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
