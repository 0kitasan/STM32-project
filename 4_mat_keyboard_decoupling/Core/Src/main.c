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

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void set_segment_code (uint8_t code)
{
	HAL_GPIO_WritePin(SEG0_GPIO_Port, SEG0_Pin, code&1<<0 ? GPIO_PIN_SET:GPIO_PIN_RESET);
	HAL_GPIO_WritePin(SEG1_GPIO_Port, SEG1_Pin, code&1<<1 ? GPIO_PIN_SET:GPIO_PIN_RESET);
	HAL_GPIO_WritePin(SEG2_GPIO_Port, SEG2_Pin, code&1<<2 ? GPIO_PIN_SET:GPIO_PIN_RESET);
	HAL_GPIO_WritePin(SEG3_GPIO_Port, SEG3_Pin, code&1<<3 ? GPIO_PIN_SET:GPIO_PIN_RESET);
	HAL_GPIO_WritePin(SEG4_GPIO_Port, SEG4_Pin, code&1<<4 ? GPIO_PIN_SET:GPIO_PIN_RESET);
	HAL_GPIO_WritePin(SEG5_GPIO_Port, SEG5_Pin, code&1<<5 ? GPIO_PIN_SET:GPIO_PIN_RESET);
	HAL_GPIO_WritePin(SEG6_GPIO_Port, SEG6_Pin, code&1<<6 ? GPIO_PIN_SET:GPIO_PIN_RESET);
}
int scan_keyboard(void)
{
	//SCAN ROW0
	HAL_GPIO_WritePin(SEG0_GPIO_Port, SEG0_Pin, GPIO_PIN_SET);
	if (HAL_GPIO_ReadPin(KC0_GPIO_Port, KC0_Pin) == GPIO_PIN_SET) return 1;
	if (HAL_GPIO_ReadPin(KC1_GPIO_Port, KC1_Pin) == GPIO_PIN_SET) return 2;
	if (HAL_GPIO_ReadPin(KC2_GPIO_Port, KC2_Pin) == GPIO_PIN_SET) return 3;
	HAL_GPIO_WritePin(SEG0_GPIO_Port, SEG0_Pin, GPIO_PIN_RESET);
	//SCAN ROW1
	HAL_GPIO_WritePin(SEG1_GPIO_Port, SEG1_Pin, GPIO_PIN_SET);
	if (HAL_GPIO_ReadPin(KC0_GPIO_Port, KC0_Pin) == GPIO_PIN_SET) return 4;
	if (HAL_GPIO_ReadPin(KC1_GPIO_Port, KC1_Pin) == GPIO_PIN_SET) return 5;
	if (HAL_GPIO_ReadPin(KC2_GPIO_Port, KC2_Pin) == GPIO_PIN_SET) return 6;
	HAL_GPIO_WritePin(SEG1_GPIO_Port, SEG1_Pin, GPIO_PIN_RESET);
	//SCAN ROW2
	HAL_GPIO_WritePin(SEG2_GPIO_Port, SEG2_Pin, GPIO_PIN_SET);
	if (HAL_GPIO_ReadPin(KC0_GPIO_Port, KC0_Pin) == GPIO_PIN_SET) return 7;
	if (HAL_GPIO_ReadPin(KC1_GPIO_Port, KC1_Pin) == GPIO_PIN_SET) return 8;
	if (HAL_GPIO_ReadPin(KC2_GPIO_Port, KC2_Pin) == GPIO_PIN_SET) return 9;
	HAL_GPIO_WritePin(SEG2_GPIO_Port, SEG2_Pin, GPIO_PIN_RESET);
	//SCAN ROW3
	HAL_GPIO_WritePin(SEG3_GPIO_Port, SEG3_Pin, GPIO_PIN_SET);
	if (HAL_GPIO_ReadPin(KC0_GPIO_Port, KC0_Pin) == GPIO_PIN_SET) return 12;
	if (HAL_GPIO_ReadPin(KC1_GPIO_Port, KC1_Pin) == GPIO_PIN_SET) return 0;
	if (HAL_GPIO_ReadPin(KC2_GPIO_Port, KC2_Pin) == GPIO_PIN_SET) return 13;
	HAL_GPIO_WritePin(SEG3_GPIO_Port, SEG3_Pin, GPIO_PIN_RESET);
	if (HAL_GPIO_ReadPin(SW0_GPIO_Port, SW0_Pin) == GPIO_PIN_RESET) return 10;
	if (HAL_GPIO_ReadPin(SW1_GPIO_Port, SW1_Pin) == GPIO_PIN_RESET) return 14;
	return -1;
}

uint8_t codes[]={0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f};
// 显示0-9
void set_segment_number (uint8_t number)
{
	set_segment_code(codes[number]);
}
void reset_segment(void)
{
	HAL_GPIO_WritePin(SEG0_GPIO_Port, SEG0_Pin,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(SEG1_GPIO_Port, SEG1_Pin,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(SEG2_GPIO_Port, SEG2_Pin,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(SEG3_GPIO_Port, SEG3_Pin,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(SEG4_GPIO_Port, SEG4_Pin,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(SEG5_GPIO_Port, SEG5_Pin,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(SEG6_GPIO_Port, SEG6_Pin,GPIO_PIN_RESET);
}
int operate(int a,int b, int operator)
{
	int ans;
	switch(operator)
	{
		case 0:
			ans=a+b;
			break;
		case 1:
			ans=a-b;
			break;
		case 2:
			ans=a*b;
			break;
		case 3:
			ans=a/b;
			break;
		default:
			ans=a;
	}
	if (ans<0 || ans >=10000) return 0;
	return ans;
}

void print_num(int display,uint8_t delayTime, uint8_t shouldDelay){
	set_segment_number(display%10);
	HAL_GPIO_WritePin(DIG0_GPIO_Port, DIG0_Pin, GPIO_PIN_SET);
	if(shouldDelay) HAL_Delay(delayTime);
	HAL_GPIO_WritePin(DIG0_GPIO_Port, DIG0_Pin, GPIO_PIN_RESET);
	set_segment_number((display/10)%10);
	HAL_GPIO_WritePin(DIG1_GPIO_Port, DIG1_Pin, GPIO_PIN_SET);
	if(shouldDelay) HAL_Delay(delayTime);
	HAL_GPIO_WritePin(DIG1_GPIO_Port, DIG1_Pin, GPIO_PIN_RESET);
	set_segment_number((display/100)%10);
	HAL_GPIO_WritePin(DIG2_GPIO_Port, DIG2_Pin, GPIO_PIN_SET);
	if(shouldDelay) HAL_Delay(delayTime);
	HAL_GPIO_WritePin(DIG2_GPIO_Port, DIG2_Pin, GPIO_PIN_RESET);
	set_segment_number((display/1000)%10);
	HAL_GPIO_WritePin(DIG3_GPIO_Port, DIG3_Pin, GPIO_PIN_SET);
	if(shouldDelay) HAL_Delay(delayTime);
	HAL_GPIO_WritePin(DIG3_GPIO_Port, DIG3_Pin, GPIO_PIN_RESET);
	reset_segment();
}

void print_digit(int position, int value) {
    reset_segment();
    // 根据位置重置其他数码管
    HAL_GPIO_WritePin(DIG0_GPIO_Port, DIG0_Pin, (position == 0) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(DIG1_GPIO_Port, DIG1_Pin, (position == 1) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(DIG2_GPIO_Port, DIG2_Pin, (position == 2) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(DIG3_GPIO_Port, DIG3_Pin, (position == 3) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    // 设置要显示的数字
    set_segment_number(value);
}

int digit[4]={0};
void get_digit(int value) {
    // 确保 value 是一个四位数
    if (value < 0 || value > 9999) {
        return;
    }
    // 从低位到高位提取每一位数字
    for (int i = 0; i < 4; i++) {
        digit[i] = value % 10;  // 提取最低位数字
        value /= 10;            // 除以 10，去掉最低位
    }
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
  // HAL_SYSTICK_Config(SystemCoreClock / 1000); // 配置 SysTick 产生 1 毫秒中断
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  uint32_t desired_frequency = 1000; // 1 kHz
  uint32_t reload_value = SystemCoreClock / desired_frequency - 1;
  HAL_SYSTICK_Config(reload_value);
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  /* USER CODE BEGIN 2 */
  int answer=0;
  int operator =0;
  int display=0;
  int i;
  int shouldDelay=1;
  int delayTime=200;
  /* USER CODE END 2 */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	i=-1;
	if(flag>=5){i=scan_keyboard();}

  	if (i>=0 && i<10)
  	{
  		// 键盘输入数字
  		if(shouldDelay) HAL_Delay(delayTime);
  		display=display*10+i;
  		if (display >=10000) display=0;
  	}
  	else if (i>=10 && i<14)
  	{
  		// 键盘输入运算符
  		if(shouldDelay) HAL_Delay(delayTime);
  		// answer=operate(answer,display,operator);
  		answer=display;
  		display=0;// 输入运算符时，屏幕上不应有显示
  		operator=i-10;
  	}
  	else if (i==14)
  	{
  		// 键盘输入等于号
  		if(shouldDelay) HAL_Delay(delayTime);
  		answer=operate(answer,display,operator);
  		display=answer;
  		// 清空operator，防止出现空按=时，answer被自己改变
  		operator=-1;
  	}
  	get_digit(display);
  	//HAL_Delay(2);
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
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LED0_Pin|LED1_Pin|SEG0_Pin|SEG1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, SEG2_Pin|DIG0_Pin|DIG1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, SEG3_Pin|SEG4_Pin|SEG5_Pin|SEG6_Pin
                          |SEG7_Pin|DIG2_Pin|DIG3_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : LED0_Pin LED1_Pin SEG0_Pin SEG1_Pin */
  GPIO_InitStruct.Pin = LED0_Pin|LED1_Pin|SEG0_Pin|SEG1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : SEG2_Pin DIG0_Pin DIG1_Pin */
  GPIO_InitStruct.Pin = SEG2_Pin|DIG0_Pin|DIG1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : SEG3_Pin SEG4_Pin SEG5_Pin SEG6_Pin
                           SEG7_Pin DIG2_Pin DIG3_Pin */
  GPIO_InitStruct.Pin = SEG3_Pin|SEG4_Pin|SEG5_Pin|SEG6_Pin
                          |SEG7_Pin|DIG2_Pin|DIG3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : SW1_Pin */
  GPIO_InitStruct.Pin = SW1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(SW1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SW0_Pin */
  GPIO_InitStruct.Pin = SW0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(SW0_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : KC0_Pin KC1_Pin KC2_Pin */
  GPIO_InitStruct.Pin = KC0_Pin|KC1_Pin|KC2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

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
