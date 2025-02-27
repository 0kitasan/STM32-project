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
SPI_HandleTypeDef hspi1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
/* USER CODE BEGIN PFP */

uint8_t DIGITS[] = { 0x3f, 0x06, 0x5b, 0x4f,
		0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f };

uint8_t INIT_ADDRESS=0xC0;

void send_cmd(uint8_t cmd)
{
  HAL_GPIO_WritePin(STB_GPIO_Port, STB_Pin, GPIO_PIN_RESET);
  HAL_SPI_Transmit(&hspi1, &cmd, 1, 10);
  HAL_GPIO_WritePin(STB_GPIO_Port, STB_Pin, GPIO_PIN_SET);
}

void set_digit(uint8_t pos, uint8_t digit)
{
  uint8_t address = INIT_ADDRESS + pos;
  HAL_GPIO_WritePin(STB_GPIO_Port, STB_Pin, GPIO_PIN_RESET);
  HAL_SPI_Transmit(&hspi1, &address, 1, 10);
  HAL_SPI_Transmit(&hspi1, &digit, 1, 10);
  HAL_GPIO_WritePin(STB_GPIO_Port, STB_Pin, GPIO_PIN_SET);
}

// 传入8位的数组，直接显示，该函数仅用于调试
void set_allnums(uint8_t *nums)
{
  uint8_t data_set[16];
  // 16个显示寄存器，要求LED全不亮
  for(int i=0;i<16;i++){
	  if(i%2==0){
		  data_set[i]=nums[i/2];
	  }else{
		  data_set[i]=0;
	  }
  }
  send_cmd(0x40);
  HAL_GPIO_WritePin(STB_GPIO_Port, STB_Pin, GPIO_PIN_RESET);
  HAL_SPI_Transmit(&hspi1, &INIT_ADDRESS, 1, 10);
  HAL_SPI_Transmit(&hspi1, data_set, 16, 10);
  HAL_GPIO_WritePin(STB_GPIO_Port, STB_Pin, GPIO_PIN_SET);
  send_cmd(0x8f);
}

// 将十进制数转换为各个位上的数字并显示
void display_number(uint32_t number)
{
    uint8_t digits[8] = {0};  // TM1638 支持最多 8 位数字显示
    static uint8_t data_set[16] = {0}; // 16 个显示寄存器
    // 提取每一位数字，从高位到低位存储
    for (int i = 0; i < 8; ++i)
    {
        digits[7-i] = number % 10;
        number /= 10;
    }
    // 将数字转换为 TM1638 的显示数据格式
    for (int i = 0; i < 8; ++i)
    {
        data_set[i * 2] = DIGITS[digits[i]];
    }
    // 发送数据到 TM1638
    send_cmd(0x40); // 设置自动地址增加模式
    HAL_GPIO_WritePin(STB_GPIO_Port, STB_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi1, &INIT_ADDRESS, 1, 10);
    HAL_SPI_Transmit(&hspi1, data_set, 16, 10);
    HAL_GPIO_WritePin(STB_GPIO_Port, STB_Pin, GPIO_PIN_SET);
    send_cmd(0x8F); // 打开显示并设置亮度
}

// 只在左半边屏幕上显示数据，该函数用于调试
void display_number_half(uint32_t number)
{
    uint8_t digits[4] = {0};
    static uint8_t data_set[8] = {0};
    // 提取每一位数字，从高位到低位存储
    for (int i = 0; i < 8; ++i)
    {
        digits[7-i] = number % 10;
        number /= 10;
    }
    // 将数字转换为 TM1638 的显示数据格式
    for (int i = 0; i < 8; ++i)
    {
        data_set[i * 2] = DIGITS[digits[i]];
    }
    // 发送数据到 TM1638
    send_cmd(0x40); // 设置自动地址增加模式
    HAL_GPIO_WritePin(STB_GPIO_Port, STB_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi1, &INIT_ADDRESS, 1, 10);
    HAL_SPI_Transmit(&hspi1, data_set, 16, 10);
    HAL_GPIO_WritePin(STB_GPIO_Port, STB_Pin, GPIO_PIN_SET);
    send_cmd(0x8F); // 打开显示并设置亮度
}



void reset(void){
	send_cmd(0x40);
	uint8_t data0set[16]={0};
	HAL_GPIO_WritePin(STB_GPIO_Port, STB_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi1, &INIT_ADDRESS, 1, 10);
	HAL_SPI_Transmit(&hspi1, data0set, 16, 10);
	HAL_GPIO_WritePin(STB_GPIO_Port, STB_Pin, GPIO_PIN_SET);
	send_cmd(0x8f);
}

uint8_t read_button(void)
{
    uint8_t keys = 0;
    uint8_t command = 0x42; // Read keys command
    uint8_t received_data[4] = {0}; // 存储接收到的4个字节数据
    HAL_GPIO_WritePin(STB_GPIO_Port, STB_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi1, &command, 1, 10);
    // 一次性接收 4 个字节的按键状态
    HAL_SPI_Receive(&hspi1, received_data, 4, 40);
    // 处理接收到的 4 个字节数据，将其合并为一个 8 位的按键状态
    for (uint8_t i = 0; i < 4; i++) {
        keys |= (received_data[i] << i);
    }
    HAL_GPIO_WritePin(STB_GPIO_Port, STB_Pin, GPIO_PIN_SET);
    return keys; // 返回按键状态
}

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
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */
  HAL_GPIO_WritePin(STB_GPIO_Port, STB_Pin, GPIO_PIN_SET);
  // 很恶心：SPI要选低位先发，时钟极性是LOW
  // 经过实测，LED灯只有最低位有效，0灭1亮
  // 尽管手册上好像是低两位有效？
  reset();
  HAL_Delay(200);
  //set_allnums(DIGITS);
  display_number(114514);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	reset();
	uint8_t res=read_button();
	HAL_Delay(200);
	display_number(res);
	HAL_Delay(100);
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
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSE;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_1LINE;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_LSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

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
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(STB_GPIO_Port, STB_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : STB_Pin */
  GPIO_InitStruct.Pin = STB_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(STB_GPIO_Port, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
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
