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
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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

/* USER CODE BEGIN PV */
typedef enum {
    MODE_OFF,
    MODE_BLINK,
    MODE_STEALTH
} LedMode;

volatile LedMode ledMode = MODE_OFF;

uint32_t blinkLastToggle = 0;
GPIO_PinState blinkPinState = GPIO_PIN_RESET;
const uint32_t BLINK_INTERVAL_MS = 500;

GPIO_PinState buttonLastState = GPIO_PIN_SET; // klidový stav = SET (pull-up)
uint32_t buttonPressStart = 0;
uint32_t buttonReleaseTime = 0;
uint8_t longPressFired = 0;
uint8_t waitingForSecondClick = 0;

const uint32_t DEBOUNCE_MS = 50;
const uint32_t LONG_PRESS_MS = 800;
const uint32_t DOUBLE_CLICK_WINDOW_MS = 400;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void ButtonTask(void);
void LedTask(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#include <stdio.h>

extern UART_HandleTypeDef huart2;

int __io_putchar(int ch) {
    HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}
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
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1)
	{
		/* HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
		// Přepne stav pinu PA5 (pokud svítí, zhasne, a naopak)
		// ZAPNUTI LED:
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
		//HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
		printf("LED ON\r\n");
		HAL_Delay(1000);

		// Počká 500 milisekund (půl sekundy)
		//HAL_Delay(500);
		// Vypnutí LED
		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
		  printf("LED OFF\r\n");
		  HAL_Delay(1000);*/
		  //novy aproach
		  ButtonTask();
		  LedTask();


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

/* USER CODE BEGIN 4 */
void ButtonTask(void)
{
    GPIO_PinState raw = HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin);
    uint32_t now = HAL_GetTick();

    // stisk (SET -> RESET)
    if (buttonLastState == GPIO_PIN_SET && raw == GPIO_PIN_RESET)
    {
        buttonPressStart = now;
        longPressFired = 0;
    }

    // dlouhý stisk, detekujeme i během držení
    if (raw == GPIO_PIN_RESET && !longPressFired &&
        (now - buttonPressStart) >= LONG_PRESS_MS)
    {
        longPressFired = 1;
        ledMode = MODE_OFF;
        printf("MODE: OFF (long press)\r\n");
    }

    // uvolnění (RESET -> SET)
    if (buttonLastState == GPIO_PIN_RESET && raw == GPIO_PIN_SET)
    {
        uint32_t pressDuration = now - buttonPressStart;

        if (pressDuration >= DEBOUNCE_MS && !longPressFired)
        {
            if (waitingForSecondClick &&
                (now - buttonReleaseTime) <= DOUBLE_CLICK_WINDOW_MS)
            {
                ledMode = MODE_STEALTH;
                printf("MODE: STEALTH (double click)\r\n");
                waitingForSecondClick = 0;
            }
            else
            {
                waitingForSecondClick = 1;
                buttonReleaseTime = now;
            }
        }
    }

    // vypršelo okno na druhý klik -> byl to jen jeden klik
    if (waitingForSecondClick && (now - buttonReleaseTime) > DOUBLE_CLICK_WINDOW_MS)
    {
        waitingForSecondClick = 0;
        ledMode = MODE_BLINK;
        blinkLastToggle = now;
        blinkPinState = GPIO_PIN_SET;
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
        printf("MODE: BLINK (single click)\r\n");
    }

    buttonLastState = raw;
}

void LedTask(void)
{
    uint32_t now = HAL_GetTick();

    switch (ledMode)
    {
        case MODE_OFF:
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
            break;

        case MODE_BLINK:
            if (now - blinkLastToggle >= BLINK_INTERVAL_MS)
            {
                blinkLastToggle = now;
                blinkPinState = (blinkPinState == GPIO_PIN_SET) ? GPIO_PIN_RESET : GPIO_PIN_SET;
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, blinkPinState);
                printf(blinkPinState == GPIO_PIN_SET ? "LED ON\r\n" : "LED OFF\r\n");
            }
            break;

        case MODE_STEALTH:
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
            // logika běží potichu dál, jen se nic nevypisuje ani nesvítí
            if (now - blinkLastToggle >= BLINK_INTERVAL_MS)
            {
                blinkLastToggle = now;
                blinkPinState = (blinkPinState == GPIO_PIN_SET) ? GPIO_PIN_RESET : GPIO_PIN_SET;
            }
            break;
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
