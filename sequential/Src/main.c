
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx_hal.h"

/* USER CODE BEGIN Includes */
/* vim: set ai et ts=4 sw=4: */
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <ctype.h>

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

void UART_Printf(const char* fmt, ...) {
    char buff[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buff, sizeof(buff), fmt, args);
    HAL_UART_Transmit(&huart1, (uint8_t*)buff, strlen(buff),
                      HAL_MAX_DELAY);
    va_end(args);
}

void keyboard_prepare() {
    HAL_GPIO_WritePin(Col1_GPIO_Port, Col1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(Col2_GPIO_Port, Col2_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(Col3_GPIO_Port, Col3_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(Col4_GPIO_Port, Col4_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(Col5_GPIO_Port, Col5_Pin, GPIO_PIN_RESET);
}

void change_column(uint8_t column) {
    switch(column) {
    case 1:
        HAL_GPIO_WritePin(Col5_GPIO_Port, Col5_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(Col1_GPIO_Port, Col1_Pin, GPIO_PIN_SET);
        break;
    case 2:
        HAL_GPIO_WritePin(Col1_GPIO_Port, Col1_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(Col2_GPIO_Port, Col2_Pin, GPIO_PIN_SET);
        break;
    case 3:
        HAL_GPIO_WritePin(Col2_GPIO_Port, Col2_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(Col3_GPIO_Port, Col3_Pin, GPIO_PIN_SET);
        break;
    case 4:
        HAL_GPIO_WritePin(Col3_GPIO_Port, Col3_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(Col4_GPIO_Port, Col4_Pin, GPIO_PIN_SET);
        break;
    default:
        HAL_GPIO_WritePin(Col4_GPIO_Port, Col4_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(Col5_GPIO_Port, Col5_Pin, GPIO_PIN_SET);
        break;
    }
}

bool keyboard_read(uint8_t* out_row, uint8_t* out_col) {
    keyboard_prepare();
    for(uint8_t col = 1; col <= 5; col++) {
        change_column(col);
        if(HAL_GPIO_ReadPin(Row1_GPIO_Port, Row1_Pin) == GPIO_PIN_SET) {
            *out_col = col;
            *out_row = 1;
            return true;
        } else if(HAL_GPIO_ReadPin(Row2_GPIO_Port, Row2_Pin) == GPIO_PIN_SET) {
            *out_col = col;
            *out_row = 2;
            return true;
        } else if(HAL_GPIO_ReadPin(Row3_GPIO_Port, Row3_Pin) == GPIO_PIN_SET) {
            *out_col = col;
            *out_row = 3;
            return true;
        } else if(HAL_GPIO_ReadPin(Row4_GPIO_Port, Row4_Pin) == GPIO_PIN_SET) {
            *out_col = col;
            *out_row = 4;
            return true;
        } else if(HAL_GPIO_ReadPin(Row5_GPIO_Port, Row5_Pin) == GPIO_PIN_SET) {
            *out_col = col;
            *out_row = 5;
            return true;
        } else if(HAL_GPIO_ReadPin(Row6_GPIO_Port, Row6_Pin) == GPIO_PIN_SET) {
            *out_col = col;
            *out_row = 6;
            return true;
        } else if(HAL_GPIO_ReadPin(Row7_GPIO_Port, Row7_Pin) == GPIO_PIN_SET) {
            *out_col = col;
            *out_row = 7;
            return true;
        }

   }
   return false;
}

void init() {
    UART_Printf("Ready!\r\n");
    HAL_Delay(1);
}

char* buttons[][5] = {
    /* ROW 1 */ {"q", "e", "r", "u", "o"},
    /* ROW 2 */ {"w", "s", "g", "h", "l"},
    /* ROW 3 */ {"SYM", "d", "t", "y", "i"},
    /* ROW 4 */ {"a", "p", "RSHIFT", "ENTER", "DEL"},
    /* ROW 5 */ {"ALT", "x", "v", "b", "$"},
    /* ROW 6 */ {" ", "z", "c", "n", "m"},
    /* ROW 7 */ {"MIC", "LSHIFT", "f", "j", "k"},
};

char* buttons_alt[][5] = {
    /* ROW 1 */ {"#", "2", "3", "_", "+"},
    /* ROW 2 */ {"1", "4", "/", ":", "\""},
    /* ROW 3 */ {NULL, "5", "(", ")", "-"},
    /* ROW 4 */ {"*", "@", NULL, NULL, NULL},
    /* ROW 5 */ {NULL, "8", "?", "!", NULL},
    /* ROW 6 */ {NULL, "7", "9", ",", "."},
    /* ROW 7 */ {"0", NULL, "6", ";", "'"},
};

// button should be released before
// pressing next button
bool btn_released = true;
bool shift_pressed = false;
bool alt_pressed = false;
uint32_t line_length = 0;

void loop() {
    uint8_t row, col;

    HAL_GPIO_WritePin(ULED_GPIO_Port, ULED_Pin,
        shift_pressed ? GPIO_PIN_SET : GPIO_PIN_RESET);

    if(keyboard_read(&row, &col)) {
        if(btn_released) {
            btn_released = false;

            char buff[16];
            if(alt_pressed && (buttons_alt[row-1][col-1] != NULL)) {
                strncpy(buff, buttons_alt[row-1][col-1], sizeof(buff));
            } else {
                strncpy(buff, buttons[row-1][col-1], sizeof(buff));
            }

            if((strcmp(buff, "LSHIFT") == 0) || (strcmp(buff, "RSHIFT") == 0)) {
                shift_pressed = !shift_pressed;
                alt_pressed = false;
            } else if(strcmp(buff, "DEL") == 0) {
                if(line_length > 0) {
                    UART_Printf("\x08 \x08");
                    line_length--;
                    shift_pressed = false;
                    alt_pressed = false;
                }
            } else if(strcmp(buff, "ALT") == 0) {
                alt_pressed = true;
                shift_pressed = false;
                // ULED will blink once. Since shift_pressed is false it will be turned off shortly.
                HAL_GPIO_WritePin(ULED_GPIO_Port, ULED_Pin, GPIO_PIN_SET);
            } else if((strcmp(buff, "MIC") == 0) || (strcmp(buff, "SYM") == 0)) {
                // ignore
            } else if(strcmp(buff, "ENTER") == 0) {
                UART_Printf("\r\n");
                line_length = 0;
                shift_pressed = false;
                alt_pressed = false;
            } else { // regular button
                if(shift_pressed && isalpha(buff[0])) {
                    buff[0] = (char)toupper(buff[0]);
                }

                UART_Printf("%s", buff);
                line_length++;
                shift_pressed = false;
                alt_pressed = false;
            }
        }
    } else {
        btn_released = true;
    }

    HAL_Delay(10);
}

/* USER CODE END 0 */

int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

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
  MX_USART1_UART_Init();

  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  init();
  while (1)
  {
    loop();
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */

  }
  /* USER CODE END 3 */

}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

    /**Configure the main internal regulator output voltage 
    */
  __HAL_RCC_PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* USART1 init function */
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
    _Error_Handler(__FILE__, __LINE__);
  }

}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
     PA5   ------> SPI1_SCK
*/
static void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, Col1_Pin|Col2_Pin|Col3_Pin|Col4_Pin 
                          |Col5_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(ULED_GPIO_Port, ULED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : Row1_Pin Row2_Pin Row3_Pin Row4_Pin 
                           Row5_Pin Row6_Pin Row7_Pin */
  GPIO_InitStruct.Pin = Row1_Pin|Row2_Pin|Row3_Pin|Row4_Pin 
                          |Row5_Pin|Row6_Pin|Row7_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PA5 */
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : Col1_Pin Col2_Pin Col3_Pin Col4_Pin 
                           Col5_Pin */
  GPIO_InitStruct.Pin = Col1_Pin|Col2_Pin|Col3_Pin|Col4_Pin 
                          |Col5_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : ULED_Pin */
  GPIO_InitStruct.Pin = ULED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(ULED_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void _Error_Handler(char * file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1) 
  {
  }
  /* USER CODE END Error_Handler_Debug */ 
}

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
