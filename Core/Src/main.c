/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
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
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ssd1306.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define HALF_WORD_TO_BINARY(byte)  \
  (byte & 0x8000 ? '1' : '0'), \
  (byte & 0x4000 ? '1' : '0'), \
  (byte & 0x2000 ? '1' : '0'), \
  (byte & 0x1000 ? '1' : '0'), \
  (byte & 0x0800 ? '1' : '0'), \
  (byte & 0x0400 ? '1' : '0'), \
  (byte & 0x0200 ? '1' : '0'), \
  (byte & 0x0100 ? '1' : '0'), \
  (byte & 0x0080 ? '1' : '0'), \
  (byte & 0x0040 ? '1' : '0'), \
  (byte & 0x0020 ? '1' : '0'), \
  (byte & 0x0010 ? '1' : '0'), \
  (byte & 0x0008 ? '1' : '0'), \
  (byte & 0x0004 ? '1' : '0'), \
  (byte & 0x0002 ? '1' : '0'), \
  (byte & 0x0001 ? '1' : '0')

//╔════════════════════════════════════════════════╗
#define NON_BLOCKING_MODE
//╚════════════════════════════════════════════════╝

#define ATM_REQUEST_PERIOD 100

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

uint16_t amt23_tx_16b;
volatile uint16_t amt23_rx_16b;
uint16_t amt32_position;

volatile uint8_t miso_line_state;

uint8_t UartMessage[256];
uint16_t MessageLength;

char lcd_line[64];

uint32_t AtmRequestSoftTimer;

volatile uint8_t amt_data_ready_flag = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
uint8_t IsAmtDataValid(uint16_t data);
void DelayOneTenthOfus(uint16_t one_tenth_of_us);
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

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_USART2_UART_Init();
	MX_I2C3_Init();
	MX_SPI1_Init();
	MX_TIM15_Init();
	/* USER CODE BEGIN 2 */

	HAL_TIM_Base_Start(&htim15);

	ssd1306_Init();

	ssd1306_Fill(Black);
	ssd1306_SetCursor(20, 0);
	ssd1306_WriteString("ufnalski.edu.pl", Font_6x8, White);
	ssd1306_SetCursor(13, 12);
	ssd1306_WriteString("Encoder [SSI, _IT]", Font_6x8, White);
	ssd1306_SetCursor(20, 24);
	ssd1306_WriteString("AMT232B-V demo", Font_6x8, White);

	ssd1306_UpdateScreen();

	AtmRequestSoftTimer = HAL_GetTick();

#ifdef NON_BLOCKING_MODE
	HAL_SPI_TransmitReceive_IT(&hspi1, (uint8_t*) (&amt23_tx_16b),
			(uint8_t*) (&amt23_rx_16b), 1);
#endif

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1)
	{

		if (HAL_GetTick() - AtmRequestSoftTimer > ATM_REQUEST_PERIOD)
		{
			AtmRequestSoftTimer = HAL_GetTick();

#ifdef NON_BLOCKING_MODE
			if (amt_data_ready_flag == 1)
			{
				amt_data_ready_flag = 0;

				HAL_UART_Transmit(&huart2, (uint8_t*) "\x1B[2J\x1B[H",
						sizeof("\x1B[2J\x1B[H"), 100);

				MessageLength =
						sprintf((char*) UartMessage,
								"\e[36m╔════════════════════════════════════════════════╗\e[37m\r\n");
				HAL_UART_Transmit(&huart2, UartMessage, MessageLength, 100);

				if (IsAmtDataValid(amt23_rx_16b) == 1)
				{

					MessageLength =
							sprintf((char*) UartMessage,
									"\e[36m║  AMT23 SSI raw reading:\e[37m 0b \e[32m%c%c \e[37m%c%c%c%c%c%c %c%c%c%c%c%c%c%c  \e[36m║\e[37m\r\n",
									HALF_WORD_TO_BINARY(amt23_rx_16b));
					HAL_UART_Transmit(&huart2, UartMessage, MessageLength, 100);

					MessageLength =
							sprintf((char*) UartMessage,
									"\e[36m║  \e[32mData OK.                                      \e[36m║\e[37m\r\n");
					HAL_UART_Transmit(&huart2, UartMessage, MessageLength, 100);

					amt32_position = amt23_rx_16b & 0x3FFF;
					sprintf(lcd_line, "Position (raw): %d   ", amt32_position);
					ssd1306_SetCursor(2, 40);
					ssd1306_WriteString(lcd_line, Font_6x8, White);
					sprintf(lcd_line, "Position (deg): %.1f   ",
							360.0f * ((float) amt32_position) / 16384.0f);
					ssd1306_SetCursor(2, 50);
					ssd1306_WriteString(lcd_line, Font_6x8, White);
					ssd1306_UpdateScreen();
				}
				else
				{
					MessageLength =
							sprintf((char*) UartMessage,
									"\e[36m║  AMT23 SSI raw reading:\e[37m 0b \e[31m%c%c \e[37m%c%c%c%c%c%c %c%c%c%c%c%c%c%c  \e[36m║\e[37m\r\n",
									HALF_WORD_TO_BINARY(amt23_rx_16b));
					HAL_UART_Transmit(&huart2, UartMessage, MessageLength, 100);

					MessageLength =
							sprintf((char*) UartMessage,
									"\e[36m║  \e[31mData corrupted!                               \e[36m║\e[37m\r\n");
					HAL_UART_Transmit(&huart2, UartMessage, MessageLength, 100);
				}

				MessageLength =
						sprintf((char*) UartMessage,
								"\e[36m╚════════════════════════════════════════════════╝\e[37m\r\n");
				HAL_UART_Transmit(&huart2, UartMessage, MessageLength, 100);

				HAL_GPIO_WritePin(AMT23_CS_GPIO_Port, AMT23_CS_Pin,
						GPIO_PIN_RESET);
				HAL_SPI_TransmitReceive_IT(&hspi1, (uint8_t*) (&amt23_tx_16b),
						(uint8_t*) (&amt23_rx_16b), 1);

			}
#else

			HAL_GPIO_WritePin(AMT23_CS_GPIO_Port, AMT23_CS_Pin, GPIO_PIN_RESET);
			DelayOneTenthOfus(5);

			HAL_SPI_TransmitReceive(&hspi1, (uint8_t*) (&amt23_tx_16b),
					(uint8_t*) (&amt23_rx_16b), 1, 10);
			miso_line_state = HAL_GPIO_ReadPin(SPI1_MISO_GPIO_Port,
					SPI1_MISO_Pin);

			amt23_rx_16b <<= 1;
			amt23_rx_16b |= (uint16_t) (miso_line_state == GPIO_PIN_SET);

			HAL_GPIO_WritePin(AMT23_CS_GPIO_Port, AMT23_CS_Pin, GPIO_PIN_SET);
			DelayOneTenthOfus(10);

			HAL_UART_Transmit(&huart2, (uint8_t*) "\x1B[2J\x1B[H",
					sizeof("\x1B[2J\x1B[H"), 100);

			MessageLength =
					sprintf((char*) UartMessage,
							"\e[36m╔════════════════════════════════════════════════╗\e[37m\r\n");
			HAL_UART_Transmit(&huart2, UartMessage, MessageLength, 100);

			if (IsAmtDataValid(amt23_rx_16b) == 1)
			{

				MessageLength =
						sprintf((char*) UartMessage,
								"\e[36m║  AMT23 SSI raw reading:\e[37m 0b \e[32m%c%c \e[37m%c%c%c%c%c%c %c%c%c%c%c%c%c%c  \e[36m║\e[37m\r\n",
								HALF_WORD_TO_BINARY(amt23_rx_16b));

				HAL_UART_Transmit(&huart2, UartMessage, MessageLength, 100);

				MessageLength =
						sprintf((char*) UartMessage,
								"\e[36m║  \e[32mData OK.                                      \e[36m║\e[37m\r\n");
				HAL_UART_Transmit(&huart2, UartMessage, MessageLength, 100);

				amt32_position = amt23_rx_16b & 0x3FFF;
				sprintf(lcd_line, "Position (raw): %d   ", amt32_position);
				ssd1306_SetCursor(2, 40);
				ssd1306_WriteString(lcd_line, Font_6x8, White);
				sprintf(lcd_line, "Position (deg): %.1f   ",
						360.0f * ((float) amt32_position) / 16384.0f);
				ssd1306_SetCursor(2, 50);
				ssd1306_WriteString(lcd_line, Font_6x8, White);
				ssd1306_UpdateScreen();
			}
			else
			{
				MessageLength =
						sprintf((char*) UartMessage,
								"\e[36m║  AMT23 SSI raw reading:\e[37m 0b \e[31m%c%c \e[37m%c%c%c%c%c%c %c%c%c%c%c%c%c%c  \e[36m║\e[37m\r\n",
								HALF_WORD_TO_BINARY(amt23_rx_16b));
				HAL_UART_Transmit(&huart2, UartMessage, MessageLength, 100);

				MessageLength =
						sprintf((char*) UartMessage,
								"\e[36m║  \e[31mData corrupted!                               \e[36m║\e[37m\r\n");
				HAL_UART_Transmit(&huart2, UartMessage, MessageLength, 100);
			}

			MessageLength =
					sprintf((char*) UartMessage,
							"\e[36m╚════════════════════════════════════════════════╝\e[37m\r\n");
			HAL_UART_Transmit(&huart2, UartMessage, MessageLength, 100);
#endif
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
	RCC_OscInitTypeDef RCC_OscInitStruct =
	{ 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct =
	{ 0 };

	/** Configure the main internal regulator output voltage
	 */
	HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
	RCC_OscInitStruct.PLL.PLLN = 10;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
	RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
	{
		Error_Handler();
	}
}

/* USER CODE BEGIN 4 */
uint8_t IsAmtDataValid(uint16_t data)
{
	uint8_t csum0 = 0;
	uint8_t csum1 = 0;
	for (uint8_t i = 0; i < 8; i++)
	{
		csum0 ^= (data >> i * 2) & 1;
		csum1 ^= (data >> (i * 2 + 1)) & 1;
	}
	return (csum0 && csum1);
}

void DelayOneTenthOfus(uint16_t one_tenth_of_us)
{
	__HAL_TIM_SET_COUNTER(&htim15, 0);
	while (__HAL_TIM_GET_COUNTER(&htim15) < one_tenth_of_us)
	{
		;  // wait
	}
}

#ifdef NON_BLOCKING_MODE
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
	if (hspi->Instance == SPI1)
	{
		miso_line_state = HAL_GPIO_ReadPin(SPI1_MISO_GPIO_Port,
		SPI1_MISO_Pin);
		HAL_GPIO_WritePin(AMT23_CS_GPIO_Port, AMT23_CS_Pin, GPIO_PIN_SET);
		amt23_rx_16b <<= 1;
		amt23_rx_16b |= (uint16_t) (miso_line_state == GPIO_PIN_SET);
		amt_data_ready_flag = 1;
	}
}
#endif
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
