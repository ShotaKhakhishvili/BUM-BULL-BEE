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
#include "MySrc/forward_range.h"
#include "MySrc/light.h"
#include "MySrc/magnet.h"
#include "MySrc/median_calculator.h"
#include "MySrc/move.h"
#include "MySrc/platform_adapter.h"
#include "MySrc/seek.h"
#include "MySrc/sharp_manager.h"
#include "MySrc/vl53l0x.h"
#include "MySrc/ws2812b.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* ===========================================================================
 *  SENSOR BENCH TESTS
 *  ---------------------------------------------------------------------------
 *  Only ONE test runs at a time. Set SENSOR_TEST_SELECT below, rebuild, flash.
 *  When a test is selected it runs forever and the normal robot logic (motors,
 *  ADC, etc.) never starts -- so the bot stays still on the bench.
 *  Set it back to SENSOR_TEST_NONE to build the real robot firmware again.
 * ===========================================================================*/
#define SENSOR_TEST_NONE        0
#define SENSOR_TEST_START_STOP  1   /* start_stop module on PA8 */
#define SENSOR_TEST_SHARP_IR    2   /* Sharp long IR x3 via SharpManager + ADC/DMA */
#define SENSOR_TEST_TOF         3   /* VL53L0X time-of-flight via Vl53l0x lib (I2C2) */

#ifndef SENSOR_TEST_SELECT
#define SENSOR_TEST_SELECT      SENSOR_TEST_NONE
#endif

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
volatile uint16_t adc_raw[4] = {0};

static SharpManager g_sharp_manager;
static Vl53l0x g_tof;
static ForwardRange g_forward;
static CloseIR g_close_ir;
static MedianCalculator g_median_calculator;
static Move move;
static Seek g_seek;
static Magnet g_magnet;
static Light g_fr;
static Light g_fl;
static Light g_br;
static Light g_bl;

/* Fast seek tuning template: update values here for quick strategy iteration. */
static const SeekTuning g_seek_tuning = SEEK_TUNING_DEFAULT_INITIALIZER;
static volatile SeekMode g_seek_mode = SEEK_MODE_LOOK;

/* Match control signals (all active-high level inputs, stable from power-up):
 *   PB5 - strategy select, latched once at boot (0 -> strategy 0, 1 -> strategy 1)
 *   PA8 - start: the match begins when this reads 1
 *   PA9 - finish: the match ends (stop + magnets off) when this reads 1 */
#define APP_STRATEGY_PORT   GPIOB
#define APP_STRATEGY_PIN    GPIO_PIN_5
#define APP_START_PORT      GPIOA
#define APP_START_PIN       GPIO_PIN_8
#define APP_FINISH_PORT     GPIOA
#define APP_FINISH_PIN      GPIO_PIN_9

/* Temporarily disable the PA8 start gate and PA9 finish handling: when 0 the bot
 * runs the selected strategy immediately and never auto-halts. Set back to 1 to
 * restore the start/finish signal logic. */
#define APP_USE_START_FINISH_SIGNALS 0

/* Temporary bench behaviour: ignore the strategies and just drive straight
 * forward with the electromagnet held at 100. Set to 0 to restore the normal
 * strategy dispatch. */
#define APP_FORWARD_MAGNET_TEST 1
#define APP_FORWARD_TEST_SPEED  150

typedef enum
{
    APP_STRATEGY_0 = 0,   /* PB5 == 0: seek / chase / catch (implemented) */
    APP_STRATEGY_1 = 1    /* PB5 == 1: alternate strategy (stub)          */
} AppStrategy;

static volatile AppStrategy g_strategy = APP_STRATEGY_0;

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
static void App_Strategy0_Tick(void);   /* seek / chase / catch */
static void App_Strategy1_Tick(void);   /* alternate strategy (stub) */
#if SENSOR_TEST_SELECT != SENSOR_TEST_NONE
static void SensorTest_Run(void);
#endif

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
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */
#if SENSOR_TEST_SELECT != SENSOR_TEST_NONE
  /* Bench test mode: run the selected sensor test forever. Each test brings up
   * only the peripherals it needs. The motor startup below is intentionally
   * skipped so the bot stays still on the bench. */
  SensorTest_Run();   /* never returns */
#endif

  if (HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_raw, 4) != HAL_OK)
  {
    Error_Handler();
  }

  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_4);

  App_InitParityGpio();

  SharpManager_Init(&g_sharp_manager);
  Vl53l0x_Init(&g_tof);
  ForwardRange_Init(&g_forward, &g_tof, &g_sharp_manager);
  CloseIR_Init(&g_close_ir);
  MedianCalculator_Init(&g_median_calculator);
  Move_Init(&move);
  Seek_Init(&g_seek);
  Seek_SetTuning(&g_seek, &g_seek_tuning);
  Magnet_Init(&g_magnet);   /* PWM up + default holding force from power-up */
  WS2812B_Init();

  {
    uint32_t i;

    for (i = 0; i < 4U; ++i)
    {
      Light_Init(g_lights[i], g_light_input_ids[i], false);
    }
  }

  /* USER CODE END 2 */

  /* Latch the strategy from PB5 once at boot (the line is stable from power-up). */
  g_strategy = (HAL_GPIO_ReadPin(APP_STRATEGY_PORT, APP_STRATEGY_PIN) == GPIO_PIN_SET)
                 ? APP_STRATEGY_1
                 : APP_STRATEGY_0;

#if APP_USE_START_FINISH_SIGNALS
  /* Hold position until the start module asserts PA8. Magnets already grip. */
  while (HAL_GPIO_ReadPin(APP_START_PORT, APP_START_PIN) != GPIO_PIN_SET)
  {
    /* idle: motors stopped, magnets holding at default */
  }
#endif

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

#if APP_USE_START_FINISH_SIGNALS
    /* Finish: PA9 high ends the match. Stop driving, release the magnets, and
     * halt - no further sensing or movement. */
    if (HAL_GPIO_ReadPin(APP_FINISH_PORT, APP_FINISH_PIN) == GPIO_PIN_SET)
    {
      Move_Stop(&move);
      Magnet_Off(&g_magnet);
      break;
    }
#endif

#if APP_FORWARD_MAGNET_TEST
    /* Straight-forward bench test: magnet at 100, drive forward. */
    Magnet_SetStrength(&g_magnet, MAGNET_STRENGTH_DEFAULT);
    Move_Walk(&move, MOVE_FORWARD, APP_FORWARD_TEST_SPEED);
    Move_Update(&move);
#else
    if (g_strategy == APP_STRATEGY_1)
    {
      App_Strategy1_Tick();
    }
    else
    {
      App_Strategy0_Tick();
    }
#endif

    HAL_Delay(5);
  }
  /* USER CODE END 3 */

#if APP_USE_START_FINISH_SIGNALS
  /* Match over: sit idle with motors stopped and magnets de-energized. */
  Move_Stop(&move);
  Magnet_Off(&g_magnet);
  while (1)
  {
  }
#endif
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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

#if SENSOR_TEST_SELECT == SENSOR_TEST_START_STOP
/*
 * start_stop module test (PA8).
 *   PA8 == 1  -> WS2812B LED red
 *   PA8 == 0  -> WS2812B LED blue
 * The signal is driven by another MCU (push-pull), so PA8 stays NOPULL.
 * PA8 is already configured as an input by MX_GPIO_Init(), so reading its level
 * works as-is. We only refresh the LED when the level changes to avoid hammering
 * the DMA every loop. Adjust LED_BRIGHTNESS (0..255) to taste.
 */
#define LED_BRIGHTNESS  120U

static void SensorTest_Run(void)
{
  GPIO_PinState last = (GPIO_PinState)2;  /* force a refresh on first pass */

  WS2812B_Init();

  for (;;)
  {
    GPIO_PinState now = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_8);

    if (now != last)
    {
      if (now == GPIO_PIN_SET)
      {
        WS2812B_SetColor(0U, LED_BRIGHTNESS, 0U, 0U);   /* PA8 = 1 -> red  */
      }
      else
      {
        WS2812B_SetColor(0U, 0U, 0U, LED_BRIGHTNESS);   /* PA8 = 0 -> blue */
      }
      WS2812B_Send();
      last = now;
    }

    HAL_Delay(10);   /* ~100 Hz poll, plenty for a start/stop signal */
  }
}
#endif /* SENSOR_TEST_START_STOP */

#if SENSOR_TEST_SELECT == SENSOR_TEST_SHARP_IR
/*
 * Sharp long-range IR test (3 sensors) using the existing SharpManager library
 * UNCHANGED. The Sharps are read through the ADC -> DMA buffer (adc_raw), so we
 * start the ADC scan here, then just call SharpManager_Update() in a loop.
 *
 * Watch these in the CubeIDE "Live Expressions" window (index 0=LEFT, 1=MIDDLE,
 * 2=RIGHT, matching the SharpSensor enum):
 *
 *     sharp_raw_adc      raw 12-bit ADC counts (0..4095)
 *     sharp_voltage      that ADC converted to volts (0..3.3)
 *     sharp_raw_cm       distance from the library, UNFILTERED
 *     sharp_dist_cm      distance from the library, EMA-FILTERED (what the robot uses)
 *     sharp_loop_count   increments every loop, so you can see it's alive
 *
 * Not static / not optimized away, so the debugger can always see them.
 */
volatile int    sharp_raw_adc[SHARP_SENSOR_COUNT];
volatile double sharp_voltage[SHARP_SENSOR_COUNT];
volatile double sharp_raw_cm[SHARP_SENSOR_COUNT];
volatile double sharp_dist_cm[SHARP_SENSOR_COUNT];
volatile uint32_t sharp_loop_count;

static SharpManager s_sharp_test;

static void SensorTest_Run(void)
{
  /* Start the ADC scan into adc_raw[] (DMA, circular). SharpManager reads from it
   * via the platform adapter. NOTE: length here matches the current adc_raw[]
   * size; see the warning I gave you about the ADC channel-count mismatch. */
  if (HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_raw, 4) != HAL_OK)
  {
    Error_Handler();
  }

  SharpManager_Init(&s_sharp_test);

  for (;;)
  {
    SharpManager_Update(&s_sharp_test);

    for (int i = 0; i < SHARP_SENSOR_COUNT; ++i)
    {
      sharp_raw_adc[i] = SharpManager_GetRawAdc(&s_sharp_test, (SharpSensor)i);
      sharp_voltage[i] = SharpManager_AdcToVoltage(sharp_raw_adc[i], 3.3, 4095.0);
      sharp_raw_cm[i]  = SharpManager_GetRawDistance(&s_sharp_test, (SharpSensor)i);
      sharp_dist_cm[i] = SharpManager_GetDistance(&s_sharp_test, (SharpSensor)i);
    }

    sharp_loop_count++;
    HAL_Delay(5);
  }
}
#endif /* SENSOR_TEST_SHARP_IR */

#if SENSOR_TEST_SELECT == SENSOR_TEST_TOF
/*
 * VL53L0X time-of-flight test using the existing Vl53l0x library UNCHANGED.
 * Runs on I2C2 (already brought up by MX_I2C2_Init() before this is called).
 * Vl53l0x_Init() boots the sensor and starts continuous ranging; we then poll
 * Vl53l0x_Update() in the loop.
 *
 * Watch these in Live Expressions:
 *
 *     tof_init_ok      1 = sensor answered and init succeeded, 0 = no/wrong device
 *     tof_valid        1 = current reading is in range, 0 = out of range / no target
 *     tof_distance_mm  latest range in millimetres
 *     tof_distance_cm  latest range in centimetres
 *     tof_loop_count   increments every loop, so you can see it's alive
 *
 * If tof_init_ok stays 0 -> I2C wiring / address / power problem (the loop will
 * still run, but distances stay 0).
 */
volatile uint8_t  tof_init_ok;
volatile uint8_t  tof_valid;
volatile uint16_t tof_distance_mm;
volatile double   tof_distance_cm;
volatile uint32_t tof_loop_count;

static Vl53l0x s_tof_test;

static void SensorTest_Run(void)
{
  tof_init_ok = (uint8_t)Vl53l0x_Init(&s_tof_test);

  for (;;)
  {
    Vl53l0x_Update(&s_tof_test);

    tof_valid       = (uint8_t)Vl53l0x_IsValid(&s_tof_test);
    tof_distance_mm = Vl53l0x_GetDistanceMm(&s_tof_test);
    tof_distance_cm = Vl53l0x_GetDistanceCm(&s_tof_test);

    tof_loop_count++;
    HAL_Delay(5);
  }
}
#endif /* SENSOR_TEST_TOF */

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

  gpio.Pin = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_13;
  gpio.Mode = GPIO_MODE_INPUT;
  gpio.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &gpio);

  /* PB5 = strategy select: pull-down so a disconnected line defaults to strategy 0. */
  gpio.Pin = GPIO_PIN_5;
  gpio.Mode = GPIO_MODE_INPUT;
  gpio.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOB, &gpio);
}

/*
 * Strategy 0 (PB5 == 0): seek / chase / catch.
 * One control tick - refresh sensors, pick the seek mode, drive, and set the
 * magnet grip from opponent proximity (default holding, stronger when close).
 */
static void App_Strategy0_Tick(void)
{
  SharpManager_Update(&g_sharp_manager);
  Vl53l0x_Update(&g_tof);
  CloseIR_Update(&g_close_ir);

  ForwardRange_Update(&g_forward);

  if (Seek_IsTargetVisible(&g_seek))
  {
    g_seek_mode = CloseIR_SeesObject(&g_close_ir) ? SEEK_MODE_CATCH : SEEK_MODE_CHASE;
  }
  else
  {
    g_seek_mode = SEEK_MODE_LOOK;
  }

  /* Clamp down harder when an opponent is right in front, otherwise just hold. */
  if (CloseIR_SeesObject(&g_close_ir))
  {
    Magnet_Close(&g_magnet);
  }
  else
  {
    Magnet_Default(&g_magnet);
  }

  Seek_Update(&g_seek, g_seek_mode, &move, &g_forward, &g_sharp_manager);
  Move_Update(&move);
}

/*
 * Strategy 1 (PB5 == 1): alternate strategy.
 * TODO: implement the second behaviour here. Runs every ~5 ms while the match
 * is active. The shared modules (g_sharp_manager, g_tof, g_forward, g_close_ir,
 * g_seek, move, g_magnet) are available exactly as in App_Strategy0_Tick.
 */
static void App_Strategy1_Tick(void)
{
  /* Placeholder: hold still until implemented. */
  Move_Stop(&move);
  Magnet_Default(&g_magnet);
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
