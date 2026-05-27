/* USER CODE BEGIN Header */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

// LED pins on PA0, PA1, PA4, PA5
uint16_t LED_PINS[4] = {GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_4, GPIO_PIN_5};
GPIO_TypeDef* LED_PORT = GPIOA;

// Button pins on PB0, PB1, PB4, PB5 (pull-up, active low)
uint16_t BTN_PINS[4] = {GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_4, GPIO_PIN_5};
GPIO_TypeDef* BTN_PORT = GPIOB;

#define START_BTN_PIN   GPIO_PIN_13
#define START_BTN_PORT  GPIOC

int turn = 0;
int randomArray[100];
int inputArray[100];
int highScore = 0;

/* USER CODE END PV */

/* Function prototypes -------------------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);

/* USER CODE BEGIN PFP */
void play_turn_flash(void);
void show_sequence(void);
void read_player_input(void);
void fail_sequence(void);
void print_round_summary(int len);
void print_failure_details(int wrongPos);
void print_game_over(void);
void print_success_message(int roundNum);
/* USER CODE END PFP */

int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_USART2_UART_Init();

  /* USER CODE BEGIN 2 */

  char msg[64];

  // Start message
  sprintf(msg, "\r\nPress USER button (PC13) to start Simon Says...\r\n");
  HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

  // Wait for press (active low)
  while (HAL_GPIO_ReadPin(START_BTN_PORT, START_BTN_PIN) == GPIO_PIN_SET);

  HAL_Delay(300);

  // Random seed
  srand(HAL_GetTick());

  /* USER CODE END 2 */

  while (1)
  {
    /* USER CODE BEGIN WHILE */

    play_turn_flash();

    sprintf(msg, "\r\nRound %d\r", (turn+1)); //
    HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    sprintf(msg, "\r\nRound Length: %d\r\n", (turn+1));
    HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

    randomArray[turn] = (rand() % 4) + 1;   // 1–4

    show_sequence();
    read_player_input();

    HAL_Delay(500);
    turn++;

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }

  /* USER CODE END 3 */
}

/* ================================
      GAME SUPPORT FUNCTIONS
   ================================ */

void play_turn_flash(void)
{
    for (int i = 0; i < 4; i++)
        HAL_GPIO_WritePin(LED_PORT, LED_PINS[i], GPIO_PIN_SET);
    HAL_Delay(600);

    for (int i = 0; i < 4; i++)
        HAL_GPIO_WritePin(LED_PORT, LED_PINS[i], GPIO_PIN_RESET);
    HAL_Delay(600);
}


void show_sequence(void)
{
    char msg[16];

    for (int i = 0; i <= turn; i++)
    {
    	//Debug code to see display what color was displayed
        //sprintf(msg, "%d ", randomArray[i]);
        //HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

        int color = randomArray[i] - 1;

        HAL_GPIO_WritePin(LED_PORT, LED_PINS[color], GPIO_PIN_SET);
        HAL_Delay(400);
        HAL_GPIO_WritePin(LED_PORT, LED_PINS[color], GPIO_PIN_RESET);
        HAL_Delay(100);
    }
}


char color_letter(int n)
{
    switch(n)
    {
        case 1: return 'G';  // Green
        case 2: return 'R';  // Red
        case 3: return 'B';  // Blue
        case 4: return 'Y';  // Yellow
    }
    return '?';
}


void read_player_input(void)
{
    char msg[8];

    for (int i = 0; i <= turn; )
    {
        for (int b = 0; b < 4; b++)
        {
            if (HAL_GPIO_ReadPin(BTN_PORT, BTN_PINS[b]) == GPIO_PIN_RESET)
            {
                HAL_GPIO_WritePin(LED_PORT, LED_PINS[b], GPIO_PIN_SET);
                HAL_Delay(200);
                HAL_GPIO_WritePin(LED_PORT, LED_PINS[b], GPIO_PIN_RESET);

                inputArray[i] = b + 1;

                char c = color_letter(inputArray[i]);
                HAL_UART_Transmit(&huart2, (uint8_t*)&c, 1, HAL_MAX_DELAY);
                HAL_UART_Transmit(&huart2, (uint8_t*)" ", 1, HAL_MAX_DELAY);

                if (inputArray[i] != randomArray[i])
                {
                    print_failure_details(i);
                    fail_sequence();
                    print_game_over();
                    memset(randomArray, 0, sizeof(randomArray));
                    memset(inputArray, 0, sizeof(inputArray));
                    return;
                }
                i++;
                HAL_Delay(250);
            }
        }
    }
    print_success_message(turn + 1);
    print_round_summary(turn + 1);
}


void fail_sequence(void)
{
    for (int r = 0; r < 3; r++)
    {
        for (int i = 0; i < 4; i++)
            HAL_GPIO_WritePin(LED_PORT, LED_PINS[i], GPIO_PIN_SET);

        HAL_Delay(250);

        for (int i = 0; i < 4; i++)
            HAL_GPIO_WritePin(LED_PORT, LED_PINS[i], GPIO_PIN_RESET);

        HAL_Delay(250);
    }

    HAL_Delay(600);
}


void print_round_summary(int len)
{
    char msg[128];

    // Expected sequence
    sprintf(msg, "\r\nExpected (len=%d): ", len);
    HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

    for(int i=0; i<len; i++)
    {
        char c = color_letter(randomArray[i]);
        HAL_UART_Transmit(&huart2, (uint8_t*)&c, 1, HAL_MAX_DELAY);
        HAL_UART_Transmit(&huart2, (uint8_t*)" ", 1, HAL_MAX_DELAY);
    }

    // Player input sequence
    sprintf(msg, "\r\nYour input (len=%d): ", len);
    HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

    for(int i=0; i<len; i++)
    {
        char c = color_letter(inputArray[i]);
        HAL_UART_Transmit(&huart2, (uint8_t*)&c, 1, HAL_MAX_DELAY);
        HAL_UART_Transmit(&huart2, (uint8_t*)" ", 1, HAL_MAX_DELAY);
    }

    HAL_UART_Transmit(&huart2, (uint8_t*)"\r\n", 2, HAL_MAX_DELAY);
}

void print_failure_details(int wrongPos)
{
    char msg[128];

    int expected = randomArray[wrongPos];
    int got = inputArray[wrongPos];

    sprintf(msg,
            "\r\nFailure at pos %d (expected %c, got %c)\r\n",
            wrongPos + 1,
            color_letter(expected),
            color_letter(got));
    HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

    // Expected sequence
    sprintf(msg, "Expected: ");
    HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    for (int i = 0; i <= turn; i++)
    {
        char c = color_letter(randomArray[i]);
        HAL_UART_Transmit(&huart2, (uint8_t*)&c, 1, HAL_MAX_DELAY);
        HAL_UART_Transmit(&huart2, (uint8_t*)" ", 1, HAL_MAX_DELAY);
    }

    // Player input
    sprintf(msg, "\r\nYour input: ");
    HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    for (int i = 0; i <= wrongPos; i++)
    {
        char c = color_letter(inputArray[i]);
        HAL_UART_Transmit(&huart2, (uint8_t*)&c, 1, HAL_MAX_DELAY);
        HAL_UART_Transmit(&huart2, (uint8_t*)" ", 1, HAL_MAX_DELAY);
    }

    sprintf(msg, "\r\nHigh score: %d\r\n", highScore);
    HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
}

void print_success_message(int roundNum)
{
    char msg[64];
    sprintf(msg, "\r\nRound %d OK. Score: %d\r", roundNum, roundNum);
    HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
}

void print_game_over(void)
{
    char msg[64];

    // Update high score
    if (turn > highScore)
        highScore = turn;

    sprintf(msg,
        "\r\nGAME OVER. Final score: %d | High score: %d\r\n",
        turn,
        highScore);
    HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

    sprintf(msg, "Press USER button to restart...\r\n");
    HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

    // Wait for PC13 press
    while (HAL_GPIO_ReadPin(START_BTN_PORT, START_BTN_PIN) == GPIO_PIN_SET);
    HAL_Delay(300);

    // Reset state
    turn = -1;
}


/* ================================
        SYSTEM INITIALIZATION
   ================================ */

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL12;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    Error_Handler();

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
    Error_Handler();

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);
}

static void MX_USART2_UART_Init(void)
{
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
    Error_Handler();
}

static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  // LED pins PA0 PA1 PA4 PA5
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  // Button pins PB0 PB1 PB4 PB5 (pull-up)
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  // USER button PC13
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

void Error_Handler(void)
{
  __disable_irq();
  while (1) {}
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
