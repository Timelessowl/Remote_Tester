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
#define VERTICAL_LINE_X             110
#define HORIZONTAL_LINE_Y           19
#define X_ICON_BEGIN           		112
#define Y_ICON_BEGIN          		8
#define DEFAULT_PRESS_COUNTER  		10000
#define DEFAULT_MAX_ERROR_COUNT     5
#define LONG_PRESS_PRESC			50

#define LED_write(state)            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, !state);
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

RTC_HandleTypeDef hrtc;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
uint32_t pressCount;
uint8_t testEngineState;
TESTER_STATE testerState;
TESTER_DATA gTesterCurData;
TESTER_DATA gTesterPrevData;

RTC_TimeTypeDef sTime = {0};
RTC_DateTypeDef DateToUpdate = {0};
ERROR_LOG errorLog[DEFAULT_MAX_ERROR_COUNT];

char trans_str[64] = {0,};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_RTC_Init(void);
/* USER CODE BEGIN PFP */
void clear_text(uint8_t x, uint8_t y, uint8_t lenght, FontDef font);
void clear_icon(void);
void setup_label(LABEL *Label, uint8_t x, uint8_t y, char str[]);
void ssd1306_WriteUint(LABEL *Label, uint32_t val, FontDef font);
void DrawResetScreen(void);
void DrawLinesAndLabels(LABEL *LabelClick, LABEL *LabelError);
void CreateErrorLog(ERROR_LOG *er);
void UartSendErrorLog(ERROR_LOG *er);
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
	uint8_t test;   //FIXME
	uint8_t fingerWaitCounter;
	uint8_t buttonWaitCounter;
	uint8_t needToRedrawIcon = 1;
	gTesterCurData.settings.maxErrorCount = DEFAULT_MAX_ERROR_COUNT;
	gTesterCurData.settings.startPressCount = DEFAULT_PRESS_COUNTER;

	gTesterCurData.status.pressCount = gTesterCurData.settings.startPressCount;
	gTesterCurData.status.errorCount = 0;
	uint8_t LongPressCount = 0;
	uint8_t _isreset = 0 ;

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_USART1_UART_Init();
  MX_RTC_Init();
  /* USER CODE BEGIN 2 */
	Buttons_Init();
	ssd1306_Init();
	testerState = STATE_CONFIG;

	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, RESET);

	LABEL label_clicks;
	LABEL label_clickCount;
	LABEL label_errors;
	LABEL label_errorCount;


	setup_label(&label_clicks, 0, 6, "clicks");
	setup_label(&label_errors, 0, 23, "errors:");
	setup_label(&label_clickCount, 42, 0, "not set");
	setup_label(&label_errorCount, 42, 22, "not set");

	LED_write(0);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  switch(testerState){
	  	  case STATE_CONFIG:
	  		  if(needToRedrawIcon){
	  			  needToRedrawIcon = 0;

	  			  DrawLinesAndLabels(&label_clicks, &label_errors);
	  			  ssd1306_WriteUint(&label_clickCount, gTesterCurData.status.pressCount, Font_11x18);
	  			  ssd1306_WriteUint(&label_errorCount, gTesterCurData.status.errorCount, Font_7x10);
	  			  ssd1306_DrawBitmap(X_ICON_BEGIN, Y_ICON_BEGIN, CONFIG_IMG, CONFIG_IMG_WIDTH, CONFIG_IMG_HEIGHT, White);
				  ssd1306_UpdateScreen();
	  		  }
	  		  break;
	  	  case STATE_PRESS:
	  		  if(needToRedrawIcon){
	  			  needToRedrawIcon = 0;
	  			  clear_icon();
	  			  ssd1306_UpdateScreen();
	  		  }
	  		if (gTesterCurData.status.pressCount != 0){
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, SET);   //Activating press finger
				testerState = STATE_WAIT;
				fingerWaitCounter = 0;
				buttonWaitCounter = 0 ;
	  		}
	  		else
	  			testerState = STATE_FINISH;


	  		  HAL_Delay(10);
	  //		  break;
	  	  case STATE_WAIT:

	  		  if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_14) != SET){
			  	  if (fingerWaitCounter++ == 20){
			  		  //Finger aren't working correctly
			  		  //Hardware error => stops cycle
			  		  snprintf(trans_str, 63, "Finger out of order");
			  		  HAL_UART_Transmit(&huart1, (uint8_t*)trans_str, strlen(trans_str), 1000);
			  		  testerState = STATE_CONFIG;
			  		  needToRedrawIcon = 1;
			  	  }
	  		  }
			  else{
				  if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13) != SET){  //FIXME: В итогой версии != RESET, т.к. нажатие будет прижимать линию к земле
					  LED_write(1);
					  if (buttonWaitCounter++ == 20){
						  //Finger working correctly
						  //No response from button
						  //Register error and move on
						  LED_write(0);
						  snprintf(trans_str, 63, "No response from button\n\r");
						  HAL_UART_Transmit(&huart1, (uint8_t*)trans_str, strlen(trans_str), 1000);
						  CreateErrorLog(&errorLog[gTesterCurData.status.errorCount]);
						  UartSendErrorLog(&errorLog[0]);
						  gTesterCurData.status.errorCount++;

						  if (gTesterCurData.status.errorCount >= gTesterCurData.settings.maxErrorCount){
							  LED_write(0);
							  testerState = STATE_FAILURE;
							  needToRedrawIcon = 1;
						  }
						  else
							  testerState = STATE_PRESS;
					  }
				  }
				  else{
					  LED_write(0);
					  gTesterCurData.status.pressCount--;
					  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, RESET);
					  testerState = STATE_PRESS;
					  HAL_Delay(10);
				  }
			  }


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

	  	  case STATE_FAILURE:
	  		  if (needToRedrawIcon)
	  		  {
	  			ssd1306_Fill(Black);
				ssd1306_DrawBitmap(X_ICON_BEGIN, Y_ICON_BEGIN, ALERT_IMG, ALERT_IMG_WIDTH, RESET_IMG_HEIGHT, White);
				ssd1306_SetCursor(10, 8);
				ssd1306_WriteString("Error", Font_11x18, White);
				ssd1306_UpdateScreen();
				needToRedrawIcon = 0;
	  		  }
	  		  else
	  			  HAL_Delay(1);
	  		  break;

	  	  };

	  //	  HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_1);
	  	  if (testerState != STATE_CONFIG){
	  		  if(gButtons.btnOk.click && testerState != STATE_FAILURE){
	  			  testerState = (testerState == STATE_PAUSE ? STATE_PRESS : STATE_PAUSE);
	  			  needToRedrawIcon = 1;
	  		  }
	  	  }
	  	  else if(gButtons.btnOk.click){
	  		  testerState = STATE_PRESS;
	  		  needToRedrawIcon = 1;
	  	  }

		if (gButtons.btnOk.longPress && !_isreset){

			if(LongPressCount == 0)
			{
				testerState = STATE_PAUSE;
				DrawResetScreen();
			}

			if((LongPressCount++) % LONG_PRESS_PRESC == 0){
				ssd1306_FillRectangle(10, 8, 10 + (int)(LongPressCount/50) * 15, 16, White);
				ssd1306_UpdateScreen();
			}
			if(LongPressCount == (LONG_PRESS_PRESC * 5) + 5){
				testerState = STATE_CONFIG;
				needToRedrawIcon = 1;
				gTesterCurData.status.pressCount = gTesterCurData.settings.startPressCount;
				gTesterCurData.status.errorCount = 0;
				gTesterPrevData.val = 0;

				_isreset = 1;
			}
		}
		else if(gButtons.btnOk.longPressEnd){
			LongPressCount = 0;
			if(_isreset)
				_isreset = 0;
			else{
				if (gTesterCurData.status.errorCount >= gTesterCurData.settings.maxErrorCount){
					testerState = STATE_FAILURE;
					needToRedrawIcon = 1;
				}
				else{
					testerState = STATE_PRESS;
					DrawLinesAndLabels(&label_clicks, &label_errors);
				}
			}
		}
		else if(gTesterCurData.val != gTesterPrevData.val){
			HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN); // RTC_FORMAT_BIN , RTC_FORMAT_BCD
//			snprintf(trans_str, 63, "Time %d:%d:%d\n", sTime.Hours, sTime.Minutes, sTime.Seconds);
//			HAL_UART_Transmit(&huart1, (uint8_t*)trans_str, strlen(trans_str), 1000);
			memcpy(&gTesterPrevData.status, &gTesterCurData.status, sizeof(gTesterPrevData.status));

			ssd1306_WriteUint(&label_clickCount, gTesterCurData.status.pressCount, Font_11x18);
			ssd1306_WriteUint(&label_errorCount, gTesterCurData.status.errorCount, Font_7x10);

//			ssd1306_WriteString(trans_str, Font_6x8, White);
//			snprintf(trans_str, 63, "clicks %d  errors %d\n\r", gTesterCurData.status.pressCount, gTesterCurData.status.errorCount);
//			HAL_UART_Transmit(&huart1, (uint8_t*)trans_str, strlen(trans_str), 1000);
//			HAL_Delay(100);
			ssd1306_UpdateScreen();
	  	  }


		Buttons_Task();

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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
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
  hi2c1.Init.ClockSpeed = 400000;
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
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef DateToUpdate = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
  hrtc.Init.OutPut = RTC_OUTPUTSOURCE_NONE;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0;
  sTime.Minutes = 0;
  sTime.Seconds = 0;

  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
  DateToUpdate.WeekDay = RTC_WEEKDAY_MONDAY;
  DateToUpdate.Month = RTC_MONTH_JANUARY;
  DateToUpdate.Date = 1;
  DateToUpdate.Year = 0;

  if (HAL_RTC_SetDate(&hrtc, &DateToUpdate, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

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

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET);

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

  /*Configure GPIO pin : PB15 */
  GPIO_InitStruct.Pin = GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */


//void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim){
//
//}

void clear_text(uint8_t x, uint8_t y, uint8_t length, FontDef font){
	ssd1306_FillRectangle(x, y, x + length*font.FontWidth,
					  y + font.FontHeight, Black);
}

void clear_icon(){
	ssd1306_FillRectangle(X_ICON_BEGIN, Y_ICON_BEGIN,
					  X_ICON_BEGIN + 16, Y_ICON_BEGIN + 16, Black);
}

void setup_label(LABEL *Label, uint8_t x, uint8_t y, char str[]){
	Label->start_x = x;
	Label->start_y = y;
	snprintf(Label->label, LABEL_MAX_LENGTH, str);

}

void ssd1306_WriteUint(LABEL *Label, uint32_t val, FontDef font){
	snprintf(Label->label, LABEL_MAX_LENGTH, "%d", val);
	clear_text(Label->start_x, Label->start_y, 6, font);
	ssd1306_SetCursor(Label->start_x, Label->start_y);
	ssd1306_WriteString(Label->label, font, White);

}

void DrawResetScreen(void){
	ssd1306_Fill(Black);
	ssd1306_DrawBitmap(X_ICON_BEGIN, Y_ICON_BEGIN, RESET_IMG, RESET_IMG_WIDTH, RESET_IMG_HEIGHT, White);
	ssd1306_DrawRectangle(10, 8, 85, 16, White);
	ssd1306_UpdateScreen();
}


void DrawLinesAndLabels(LABEL *LabelClick, LABEL *LabelError){
	ssd1306_Fill(Black);

	ssd1306_SetCursor(LabelClick->start_x, LabelClick->start_y);
	ssd1306_WriteString(LabelClick->label, Font_6x8, White);

	ssd1306_SetCursor(LabelError->start_x, LabelError->start_y);
	ssd1306_WriteString(LabelError->label, Font_6x8, White);

	ssd1306_Line(0, HORIZONTAL_LINE_Y, VERTICAL_LINE_X, HORIZONTAL_LINE_Y, White);
	ssd1306_Line(VERTICAL_LINE_X, 0, VERTICAL_LINE_X, 32, White);

	ssd1306_UpdateScreen();
}

void CreateErrorLog(ERROR_LOG *er){
	  er->failedPressNumber =  gTesterCurData.status.pressCount;
	  HAL_RTC_GetTime(&hrtc, &er->errorTime, RTC_FORMAT_BIN);
}

void UartSendErrorLog(ERROR_LOG *er){
	uint8_t i;
	for (i = 0 ; i < gTesterCurData.status.errorCount + 1; i++){
		snprintf(trans_str, 63, "Error on press %d at %d:%d:%d\n\r", er->failedPressNumber,
				er->errorTime.Hours, er->errorTime.Minutes, er->errorTime.Seconds);
		HAL_UART_Transmit(&huart1, (uint8_t*)trans_str, strlen(trans_str), 1000);
		er++;
	}
//	snprintf(trans_str, 63, "Time %d:%d:%d\n", sTime.Hours, sTime.Minutes, sTime.Seconds);
//	HAL_UART_Transmit(&huart1, (uint8_t*)trans_str, strlen(trans_str), 1000);
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
