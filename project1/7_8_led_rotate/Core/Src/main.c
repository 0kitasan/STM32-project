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
#include "dotfont_lib.h"
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
TIM_HandleTypeDef htim16;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM16_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void hc595_send(uint16_t data)
{
	for(int i = 0;i < 16;i++)
	{
		HAL_GPIO_WritePin(DS_GPIO_Port, DS_Pin, data & 1<<15 ? GPIO_PIN_SET : GPIO_PIN_RESET);
		HAL_GPIO_WritePin(SHCP_GPIO_Port,SHCP_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(SHCP_GPIO_Port,SHCP_Pin, GPIO_PIN_RESET);
		data<<=1;
	}
	HAL_GPIO_WritePin(STCP_GPIO_Port,STCP_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(STCP_GPIO_Port,STCP_Pin, GPIO_PIN_SET);
}

void print_dotmat(uint16_t **dotmat,uint8_t nums,uint8_t size,int delay){
	for(int j = 0;j < nums;++j)
	  {
		  for(int i = 0;i < size;++i)
		  {
			  hc595_send(dotmat[j][i]);
			  HAL_Delay(delay);
		  }
	  }
	  hc595_send(0x0000);
}

void print_digit(uint16_t digit,int delay){
	for(int i=0;i<8;i++){
		hc595_send(NUMBER[digit][i]);
		HAL_Delay(delay);
	}
	hc595_send(0x0000);
}

void print_num(uint16_t num, int delay) {
    uint8_t digits[5]; // 最大值 65535，有 5 位
    uint8_t num_digits = 0;
    // 分解数字并存入数组
    do {
        digits[num_digits++] = num % 10;
        num /= 10;
    } while (num > 0);

    // 反向输出数组中的数字
    for (int i = 0; i < num_digits; i++) {
        print_digit(digits[num_digits-i-1], delay);
    }

    hc595_send(0x0000); // 清除显示
}

uint8_t h = 0, m = 0, s = 0;

void renew_clock(void){
    // 清除之前的时、分、秒指针位置
	// CLOCK[h]=CLOCK[m]=CLOCK[s]=0;
	// 如果后续加入箭头之类的，就不能用上面这个方法
	for (int i = 0; i < 60; i++) {
		CLOCK[i] = 0;
	}
	// 更新时间
	s++;
	if (s >= 60) {
		s = 0;
		m++;
		if (m >= 60) {
			m = 0;
			h++;
			if (h >= 24) {
				h = 0;
			}
		}
	}
	// 设置新的时、分、秒指针位置
	CLOCK[h] = hour_hand;
	CLOCK[m] = min_hand;
	CLOCK[s] = sec_hand;
}

void print_clock(int delay) {
    // 静态变量保存时钟的时、分、秒
    for(int i=0;i<60;i++){
    	hc595_send(CLOCK[i]);
    	HAL_Delay(delay);
    }
}

//给电机1v，转速大约30s有600r，也就是20r/s
//人眼视觉暂留时间0.2s，也就是说5r/s是底线
//现在CPU主频16MHz，如果定时器分频16，频率为1MHz
//运行周期1us，我们希望他能一秒运行20次
//经测试，转速为10-20r/s不等
uint32_t difference=50000;
volatile uint32_t time_interval=100;
//最好附上默认值以防止未定义行为
//这里假设默认就是10r/s，那么就是0.1s/r,也就是100ms
volatile uint32_t overflow_cnt = 0;
uint8_t captured_once = 0;
uint32_t ic_val1 = 0;
uint32_t ic_val2 = 0;
volatile uint32_t rotate_round = 0;
//该阈值用于等待旋转稳定
uint8_t rotate_begin = 0;
const uint8_t round_threshod=5;
uint8_t round_finish_flag = 0;
const uint8_t CLOCK_MODE_ON=1;
uint8_t renew_clock_flag=0;
uint8_t freq=1;//MHz
uint8_t overflow_1s=0;
volatile uint32_t elapsed_secs=0;//经过的秒数

// 初始化函数，overflow_1s用于计算 1 秒内的溢出次数，配置没变的话应该是15
void init_overflow_1s(void) {
    overflow_1s = (freq * 1000000) / (htim16.Init.Period + 1);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	static uint32_t real_overflow_cnt=0;
	// overflow_cnt会被重置，因此需要新建一个变量
    if (htim == &htim16)
    {
        overflow_cnt++; // 记录溢出次数
        real_overflow_cnt++;
    }
    if(CLOCK_MODE_ON==1 && rotate_begin==1){
    	if(real_overflow_cnt%overflow_1s==0){
			//renew_clock();
    		renew_clock_flag=1;
    		elapsed_secs++;
		}
    }
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if (htim == &htim16)
    {
    	if(rotate_begin==0){
    		rotate_round++;
			if(rotate_round>round_threshod){
				rotate_begin=1;// 超过阈值，开始正常计数
				rotate_round=0;// 清零
			}
    	}else{
        	round_finish_flag=1;
        	rotate_round++;
            if (captured_once == 0)
            {
                ic_val1 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
                captured_once = 1;
                overflow_cnt = 0; // 重置溢出计数器
            }
            else
            {
                ic_val2 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
                difference = ic_val2 - ic_val1 + overflow_cnt * (htim16.Init.Period + 1);
                time_interval=difference/(freq * 1000);
                // 计算时间间隔（单位为微秒）,1MHz计数频率
                captured_once = 0; // 重置捕获状态
            }
    	}
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

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM16_Init();
  /* USER CODE BEGIN 2 */
  init_overflow_1s();
  // 启动输入捕获中断
  HAL_TIM_IC_Start_IT(&htim16, TIM_CHANNEL_1);
  // 启动更新中断
  __HAL_TIM_ENABLE_IT(&htim16, TIM_IT_UPDATE);
  // 仅用于测试
  hc595_send(0xff00);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	if(renew_clock_flag==1){
		renew_clock();
		renew_clock_flag=0;
	}
	if(round_finish_flag==1){
//		print_num(time_interval,time_interval/1000);
//		HAL_Delay(10);
//		print_num(elapsed_secs,time_interval/1000);
//		HAL_Delay(10);
//		print_num(rotate_round,time_interval/1000);
//		HAL_Delay(10);
		//renew_clock();
		print_clock(time_interval/60);
		round_finish_flag=0;
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
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM16 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM16_Init(void)
{

  /* USER CODE BEGIN TIM16_Init 0 */

  /* USER CODE END TIM16_Init 0 */

  TIM_IC_InitTypeDef sConfigIC = {0};

  /* USER CODE BEGIN TIM16_Init 1 */

  /* USER CODE END TIM16_Init 1 */
  htim16.Instance = TIM16;
  htim16.Init.Prescaler = 16-1;
  htim16.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim16.Init.Period = 65535;
  htim16.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim16.Init.RepetitionCounter = 0;
  htim16.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim16) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_IC_Init(&htim16) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_FALLING;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 0;
  if (HAL_TIM_IC_ConfigChannel(&htim16, &sConfigIC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM16_Init 2 */

  /* USER CODE END TIM16_Init 2 */

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
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, DS_Pin|SHCP_Pin|STCP_Pin|OE_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : DS_Pin SHCP_Pin STCP_Pin OE_Pin */
  GPIO_InitStruct.Pin = DS_Pin|SHCP_Pin|STCP_Pin|OE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
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
