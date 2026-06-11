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
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "MySrc/close_ir.h"
#include "MySrc/debug_vars.h"
#include "MySrc/light.h"
#include "MySrc/median_calculator.h"
#include "MySrc/move.h"
#include "MySrc/platform_adapter.h"
#include "MySrc/seek.h"
#include "MySrc/sharp_manager.h"
#include "MySrc/ws2812b.h"

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
volatile uint16_t adc_raw[5] = {0};

static SharpManager g_sharp_manager;
static CloseIR g_close_ir;
static MedianCalculator g_median_calculator;
static Move move;
static Seek g_seek;
static Light g_fr;
static Light g_fl;
static Light g_br;
static Light g_bl;

/* Fast seek tuning template: update values here for quick strategy iteration. */
static const SeekTuning g_seek_tuning = SEEK_TUNING_DEFAULT_INITIALIZER;
static volatile SeekMode g_seek_mode = SEEK_MODE_LOOK;

/* Robot run state driven by the PB8 remote line (EXTI): 1 = on, 0 = off. */
static volatile bool g_robot_on = false;

static Light *g_lights[4] =
{
  &g_fr,
  &g_fl,
  &g_br,
  &g_bl
};

static const uint8_t g_light_input_ids[4] =
{
  PLATFORM_LIGHT_INPUT_COL1,
  PLATFORM_LIGHT_INPUT_COL2,
  PLATFORM_LIGHT_INPUT_COL3,
  PLATFORM_LIGHT_INPUT_COL4
};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
static void MX_USART1_UART_Init(void);
static void App_InitParityGpio(void);

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
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_TIM4_Init();
  MX_TIM2_Init();
  MX_I2C2_Init();

  /* USER CODE BEGIN 2 */
  if (HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_raw, 5) != HAL_OK)
  {
    Error_Handler();
  }

  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_4);

  App_InitParityGpio();

  SharpManager_Init(&g_sharp_manager);
  CloseIR_Init(&g_close_ir);
  MedianCalculator_Init(&g_median_calculator);
  Move_Init(&move);
  Seek_Init(&g_seek);
  Seek_SetTuning(&g_seek, &g_seek_tuning);
  WS2812B_Init();

  /* Seed the run state from the current PA8 level; the EXTI keeps it updated. */
  g_robot_on = (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_8) == GPIO_PIN_SET);

  {
    uint32_t i;

    for (i = 0; i < 4U; ++i)
    {
      Light_Init(g_lights[i], g_light_input_ids[i], false);
    }
  }

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    int short_raw_adc;
    int long_raw_adc;
    int left_raw_adc;
    int right_raw_adc;
    double short_voltage;
    double long_voltage;
    double left_voltage;
    double right_voltage;
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

    if (!g_robot_on)
    {
      Move_Stop(&move);
      Move_Update(&move);
      g_seek_mode = SEEK_MODE_LOOK;
      HAL_Delay(5);
      continue;
    }

    /* WS2812B heartbeat blink */
    {
      static uint32_t s_led_last = 0U;
      static bool     s_led_on = false;
      uint32_t now = HAL_GetTick();

      if ((now - s_led_last) >= 500U)
      {
        s_led_last = now;
        s_led_on = !s_led_on;
        WS2812B_SetColor(0, s_led_on ? 255U : 0U, 0U, 0U);
        WS2812B_Send();
      }
    }

    SharpManager_Update(&g_sharp_manager);
    CloseIR_Update(&g_close_ir);

    short_raw_adc = SharpManager_GetShortRawAdc(&g_sharp_manager);
    long_raw_adc  = SharpManager_GetLongRawAdc(&g_sharp_manager);
    left_raw_adc  = SharpManager_GetLeftRawAdc(&g_sharp_manager);
    right_raw_adc = SharpManager_GetRightRawAdc(&g_sharp_manager);

    short_voltage = SharpManager_AdcToVoltage(short_raw_adc, 3.3, 4095.0);
    long_voltage  = SharpManager_AdcToVoltage(long_raw_adc,  3.3, 4095.0);
    left_voltage  = SharpManager_AdcToVoltage(left_raw_adc,  3.3, 4095.0);
    right_voltage = SharpManager_AdcToVoltage(right_raw_adc, 3.3, 4095.0);

    g_debug_short_raw_adc = short_raw_adc;
    g_debug_long_raw_adc  = long_raw_adc;
    g_debug_left_raw_adc  = left_raw_adc;
    g_debug_right_raw_adc = right_raw_adc;
    g_debug_short_voltage = (float)short_voltage;
    g_debug_long_voltage  = (float)long_voltage;
    g_debug_left_voltage  = (float)left_voltage;
    g_debug_right_voltage = (float)right_voltage;

    MedianCalculator_Update(
        &g_median_calculator,
        short_voltage,
        long_voltage);

    if ((g_seek_mode == SEEK_MODE_LOOK) && Seek_IsTargetVisible(&g_seek))
    {
      g_seek_mode = SEEK_MODE_CHASE;
    }
    else if ((g_seek_mode == SEEK_MODE_CHASE) && !Seek_IsTargetVisible(&g_seek))
    {
      g_seek_mode = SEEK_MODE_LOOK;
    }

    if (SharpManager_GetMiddleDistance(&g_sharp_manager) < 30 && !Seek_IsTargetVisible(&g_seek))
    {
      g_seek_mode = SEEK_MODE_LOOK;
    }

    Seek_Update(&g_seek, g_seek_mode, &move, &g_sharp_manager);

    Move_Update(&move);

    HAL_Delay(5);
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
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
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                              | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

static void MX_USART1_UART_Init(void)
{
  GPIO_InitTypeDef gpio = {0};
  uint32_t brr;

  __HAL_RCC_USART1_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  gpio.Pin = GPIO_PIN_9;
  gpio.Mode = GPIO_MODE_AF_PP;
  gpio.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &gpio);

  gpio.Pin = GPIO_PIN_10;
  gpio.Mode = GPIO_MODE_INPUT;
  gpio.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &gpio);

  USART1->CR1 = 0U;
  USART1->CR2 = 0U;
  USART1->CR3 = 0U;

  brr = (HAL_RCC_GetPCLK2Freq() + (115200U / 2U)) / 115200U;
  USART1->BRR = brr;

  USART1->CR1 = USART_CR1_TE | USART_CR1_RE;
  USART1->CR1 |= USART_CR1_UE;
}

static void App_InitParityGpio(void)
{
  GPIO_InitTypeDef gpio = {0};

  __HAL_RCC_GPIOC_CLK_ENABLE();

  gpio.Pin = GPIO_PIN_13;
  gpio.Mode = GPIO_MODE_OUTPUT_PP;
  gpio.Pull = GPIO_NOPULL;
  gpio.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &gpio);

  gpio.Pin = GPIO_PIN_15;
  gpio.Mode = GPIO_MODE_INPUT;
  gpio.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &gpio);

  gpio.Pin = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_13;
  gpio.Mode = GPIO_MODE_INPUT;
  gpio.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &gpio);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == GPIO_PIN_8)
  {
    g_robot_on = (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_8) == GPIO_PIN_SET);

    if (!g_robot_on)
    {
      g_seek_mode = SEEK_MODE_LOOK;
    }
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
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
