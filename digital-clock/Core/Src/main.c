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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "esp8266ex_driver.h"
#include "lcd.h"
#include "dht20.h"
#include "config_private.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define BUSY 1
#define FREE 0

//#define MODE_TIME		0
//#define MODE_WEATHER	1

typedef enum
{
	MODE_TIME,
	MODE_WEATHER,
	MODE_WEATHER_LOCAL
} MODES;

#define User_Button_Pin B1_Pin
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim5;

UART_HandleTypeDef huart6;

/* USER CODE BEGIN PV */
RTC_Time_t current_time;
RTC_Date_t current_date;

Weather weather;

uint8_t is_fetching_data_from_server = FREE;
uint8_t is_update_digital_clock = FREE;
uint8_t is_update_weather_local = FREE;
uint8_t is_update_weather = FREE;

MODES modes[3] = {MODE_TIME, MODE_WEATHER_LOCAL, MODE_WEATHER,};
uint8_t current_mode_index = 0;
uint8_t mode_changed = 0; // Used to clear LCD screen one time upon mode changes

DHT20_Handle_t DHT20;

GPIO_PinState user_button_state;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART6_UART_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM5_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
char* get_day_of_week(uint8_t i)
{
	char* days[] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
	return days[i-1];
}


void number_to_string(uint8_t num, char* buf)
{
	if(num < 10)
	{
		buf[0] = '0';
		buf[1] = num + 48;
	}
	else if (num >= 10 && num < 99)
	{
		buf[0] = (num/10) + 48;
		buf[1] = (num%10) + 48;
	}
}


char* time_to_string(RTC_Time_t *rtc_time)
{
	static char buf[9]; // Static because of dangling pointer; once function pops out of stack the value is lost
	buf[2] = ':';
	buf[5] = ':';

	number_to_string(rtc_time->hours, buf);
	number_to_string(rtc_time->minutes, &buf[3]);
	number_to_string(rtc_time->seconds, &buf[6]);

	buf[8] = '\0';

	return buf;

}


char* date_to_string(RTC_Date_t *rtc_date)
{
	static char buf[9]; // Static because of dangling pointer; once function pops out of stack the value is lost
	buf[2] = '/';
	buf[5] = '/';

	number_to_string(rtc_date->month, buf);
	number_to_string(rtc_date->date, &buf[3]);
	number_to_string(rtc_date->year, &buf[6]);

	return buf;
}


void delay_us(uint32_t us)
{
	__HAL_TIM_SET_COUNTER(&htim3, 0);
	HAL_TIM_Base_Start(&htim3);
	while (htim3.Instance->CNT < us);
	HAL_TIM_Base_Stop(&htim3);
}


void initialize_wifi()
{
	//	esp8266ex_firmware_version(&huart6);
	lcd_display_clear();
	lcd_set_cursor(1,1);
	lcd_print_string("ESP8266 to mode:");
	lcd_set_cursor(2,1);
	lcd_print_string("Station");
	esp8266ex_wifi_mode(&huart6, ESP8266EX_MODE_STATION);

	lcd_display_clear();
	lcd_set_cursor(1,1);
	lcd_print_string("Connecting to");
	lcd_set_cursor(2,1);
	lcd_print_string(WIFI_SSID);
	esp8266ex_connect_ap(&huart6, WIFI_SSID, WIFI_PASSWORD);
}

void get_time_wifi()
{
	if(ds1307_init(&hi2c1))
	{
		while(1); // TODO: Handle Failure
	}
	Clock clock;

	lcd_display_clear();
	lcd_set_cursor(1,1);
	lcd_print_string("Connecting to");
	lcd_set_cursor(2,1);
	lcd_print_string(SERVER_IP);
	esp8266ex_cipstart(&huart6, ESP8266EX_TRANSPORT_TCP, SERVER_IP, SERVER_PORT);

	lcd_display_clear();
	lcd_set_cursor(1,1);
	lcd_print_string("Getting time...");

	clock = esp8266ex_get_time(&huart6);
	current_time = clock.time;
	current_date = clock.date;
	ds1307_set_current_date(&hi2c1, &current_date);
	ds1307_set_current_time(&hi2c1, &current_time);
}


void get_weather_wifi()
{
	lcd_display_clear();
	lcd_set_cursor(1,1);
	lcd_print_string("Connecting to");
	lcd_set_cursor(2,1);
	lcd_print_string(SERVER_IP);
	esp8266ex_cipstart(&huart6, ESP8266EX_TRANSPORT_TCP, SERVER_IP, SERVER_PORT);

	lcd_display_clear();
	lcd_set_cursor(1,1);
	lcd_print_string("Getting weather");
	lcd_set_cursor(2,1);
	lcd_print_string("from ");
	lcd_print_string(ZIP_CODE);

	weather = esp8266ex_get_weather(&huart6);
}


void update_digital_clock()
{
	RTC_Time_t current_time;
	RTC_Date_t current_date;

	ds1307_get_current_time(&hi2c1, &current_time);
	ds1307_get_current_date(&hi2c1, &current_date);

	lcd_set_cursor(1,1);
	char *am_pm;
	if(current_time.time_format != TIME_FORMAT_24HRS)
	{
		am_pm = (current_time.time_format) ? "PM" : "AM";
		lcd_print_string(time_to_string(&current_time));
		lcd_print_string(am_pm);
	}
	else
	{
		lcd_print_string(time_to_string(&current_time));
	}

	lcd_set_cursor(2, 1);
	lcd_print_string(date_to_string(&current_date));
	lcd_print_char('<');
	lcd_print_string(get_day_of_week(current_date.day));
	lcd_print_char('>');
}


void update_weather()
{
	lcd_set_cursor(1,1);
	lcd_print_string("Temp: ");
	lcd_print_string(weather.temperature);
	lcd_print_string("F");

	lcd_set_cursor(2,1);
	lcd_print_string("Humidity: ");
	lcd_print_string(weather.humidity);
}

void update_weather_local()
{
	DHT20_Read(&hi2c1);

	char tmp_buf[8];
	uint8_t temp_int = (uint8_t)(DHT20.temperature * 100); // two decimal digits

	snprintf(tmp_buf, sizeof(tmp_buf),
	         "%d.%02d",
			 (uint8_t)(DHT20.temperature),
			 temp_int % 100);

	lcd_set_cursor(1,1);
	lcd_print_string("Temp: ");
	lcd_print_string(tmp_buf);
	lcd_print_string("F");

	temp_int = (uint8_t)(DHT20.humidity * 100); // two decimal digits
	snprintf(tmp_buf, sizeof(tmp_buf),
	         "%d.%02d",
			 (uint8_t)(DHT20.humidity),
			 temp_int % 100);

	lcd_set_cursor(2,1);
	lcd_print_string("Humidity: ");
	lcd_print_string(tmp_buf);
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  user_button_state = HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin);;
  DHT20.state = IDLE;

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
  MX_I2C1_Init();
  MX_USART6_UART_Init();
  MX_TIM3_Init();
  MX_TIM5_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */

  lcd_init();
  DHT20_Init(&hi2c1);
  HAL_Delay(2000);
  lcd_display_clear();
  lcd_set_cursor(1,1);

//  lcd_print_string("test");
//  initialize_wifi();

//  while(1)
//  {
//	  if (DHT20_Read(&hi2c1))
//	  {
//
//	  }
//  }


  lcd_display_clear();
  if (HAL_TIM_Base_Start_IT(&htim2) != HAL_OK) // Start global interrupt every 1 second
  {
	  /* Starting Error */
	  Error_Handler();
  }

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	if(mode_changed)
	{
		lcd_display_clear();
		mode_changed = 0;
	}

	if(is_fetching_data_from_server == BUSY)
	{
		get_time_wifi();
		get_weather_wifi();

		is_fetching_data_from_server = FREE;
		lcd_display_clear();
		lcd_set_cursor(1,1);
	}
	else if (is_fetching_data_from_server == FREE)
	{
		if(modes[current_mode_index] == MODE_TIME)
		{
			if(is_update_digital_clock == BUSY)
			{
				update_digital_clock();
				is_update_digital_clock = FREE;
			}
		}
		else if(modes[current_mode_index] == MODE_WEATHER)
		{
			update_weather();
		}
		else if(modes[current_mode_index] == MODE_WEATHER_LOCAL)
		{
			if(is_update_weather_local == BUSY)
			{
				update_weather_local();
				is_update_weather_local = FREE;
			}
		}
	}
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 100;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
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
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 99999999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 100 - 1;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 65535;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief TIM5 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM5_Init(void)
{

  /* USER CODE BEGIN TIM5_Init 0 */

  /* USER CODE END TIM5_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM5_Init 1 */

  /* USER CODE END TIM5_Init 1 */
  htim5.Instance = TIM5;
  htim5.Init.Prescaler = 10000;
  htim5.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim5.Init.Period = 100000000-1;
  htim5.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim5.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim5) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim5, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim5, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM5_Init 2 */

  /* USER CODE END TIM5_Init 2 */

}

/**
  * @brief USART6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART6_UART_Init(void)
{

  /* USER CODE BEGIN USART6_Init 0 */

  /* USER CODE END USART6_Init 0 */

  /* USER CODE BEGIN USART6_Init 1 */

  /* USER CODE END USART6_Init 1 */
  huart6.Instance = USART6;
  huart6.Init.BaudRate = 115200;
  huart6.Init.WordLength = UART_WORDLENGTH_8B;
  huart6.Init.StopBits = UART_STOPBITS_1;
  huart6.Init.Parity = UART_PARITY_NONE;
  huart6.Init.Mode = UART_MODE_TX_RX;
  huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart6.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart6) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART6_Init 2 */

  /* USER CODE END USART6_Init 2 */

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
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : USART_TX_Pin USART_RX_Pin */
  GPIO_InitStruct.Pin = USART_TX_Pin|USART_RX_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void TIM_IRQHandler() // Called in stm32f4xx_it.c
{
	if(is_update_digital_clock == FREE && (modes[current_mode_index] == MODE_TIME))
		is_update_digital_clock = BUSY;
	if(is_update_weather_local == FREE && (modes[current_mode_index] == MODE_WEATHER_LOCAL))
		is_update_weather_local = BUSY;

//	if(is_update_weather == FREE) is_update_weather = BUSY;
}

void EXTI_Falling_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == User_Button_Pin)
	{
		// Start TIM17
		__HAL_TIM_SET_COUNTER(&htim5, 0);
		HAL_TIM_Base_Start(&htim5);

	}
}


void EXTI_Rising_Callback(uint16_t GPIO_Pin)
{
	uint32_t elapsed_time = htim5.Instance->CNT;
	if(GPIO_Pin == User_Button_Pin)
	{
		if (elapsed_time < 30000) // < 3 Seconds
		{
			(++current_mode_index);
			current_mode_index %= sizeof(modes);
			mode_changed = 1;
		}
		else if (elapsed_time < 100000) // 3-10 seconds
		{
			is_fetching_data_from_server = BUSY;
		}
		HAL_TIM_Base_Stop(&htim5);
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	GPIO_PinState current_b1_state = HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin);

	if (user_button_state == GPIO_PIN_SET && current_b1_state == GPIO_PIN_RESET) // High to low
	{
		user_button_state = GPIO_PIN_RESET;
		EXTI_Falling_Callback(GPIO_Pin);
	}
	else if (user_button_state == GPIO_PIN_RESET && current_b1_state == GPIO_PIN_SET) // Low to High
	{
		user_button_state = GPIO_PIN_SET;
		EXTI_Rising_Callback(GPIO_Pin);
	}
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
