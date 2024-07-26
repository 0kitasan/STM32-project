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
#include "arm_math.h"
#include "arm_const_structs.h"
#include <string.h>
#include "uart_reg.h"
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define samples 4096
#define FFT_LENGTH 4096
#define samples1 1024
#define FFT_LENGTH1 1024
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc2;
ADC_HandleTypeDef hadc3;
ADC_HandleTypeDef hadc4;
DMA_HandleTypeDef hdma_adc1;

CRC_HandleTypeDef hcrc;

DAC_HandleTypeDef hdac1;
DAC_HandleTypeDef hdac2;
DMA_HandleTypeDef hdma_dac1_ch1;

I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim6;

UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart2_rx;
DMA_HandleTypeDef hdma_usart2_tx;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_CRC_Init(void);
static void MX_ADC1_Init(void);
static void MX_I2C1_Init(void);
static void MX_DAC1_Init(void);
static void MX_TIM6_Init(void);
static void MX_ADC2_Init(void);
static void MX_ADC3_Init(void);
static void MX_ADC4_Init(void);
static void MX_TIM1_Init(void);
static void MX_DAC2_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

uint16_t adc_buffer[samples];
uint8_t adc_buffer1[samples1];
float thd = 0;
volatile int adc_ongoing = 0; //volatile affects compiler compilation results. volatile indicates that variables may change at any time. Do not optimize operations related to volatile variables to avoid errors
void floatToString(float num, char *str, int precision);
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
	if (hadc == &hadc1) {
		adc_ongoing = 0;
		HAL_TIM_Base_Stop(&htim6);
		HAL_ADC_Stop_DMA(hadc);
	}
}
void i2c_handler(uint8_t *data, size_t size) {
	if (data[0] == 'W') {
		uint8_t addr = data[1];
		uint8_t len = size - 2;
		uint8_t ret = 0;
		if (HAL_I2C_Master_Transmit(&hi2c1, addr, data + 2, len, len + 1)
				!= HAL_OK)
			ret = -1;
		uart_transmit(&ret, 1);
	} else if (data[0] == 'R') {
		uint8_t addr = data[1];
		uint8_t len = data[2];
		uint8_t ret[len + 1];
		ret[0] = 0;
		if (HAL_I2C_Master_Receive(&hi2c1, addr, ret + 1, len, len + 1)
				!= HAL_OK)
			ret[0] = -1;
		uart_transmit(ret, len + 1);
	}
}

void adc_handler(uint8_t *data, size_t size) {
//	const char lzz[6]="wsy\r\n";
//	uart_transmit(lzz, 6);
	uart_transmit(adc_buffer1, samples1);
}

void split_uint16_to_uint8(const uint16_t *uint16_array,
		size_t uint16_array_size, uint8_t *uint8_array) {
	for (size_t i = 0; i < uint16_array_size; i++) {
		uint8_array[i * 2 + 1] = (uint8_t) (uint16_array[i] >> 8);
		uint8_array[i * 2] = (uint8_t) (uint16_array[i] & 0xFF);
	}
}
int findMaxIndexInRange(uint16_t arr[], int start, int end) {
	int maxIndex = start;
	for (int i = start + 1; i <= end; i++) {
		if (arr[i] > arr[maxIndex]) {
			maxIndex = i;
		}
	}
	return maxIndex;
}
void calculateTHD (uint16_t fft_int[])
{
  int max = findMaxIndexInRange (fft_int, 0, 511);
  float fundamental_amplitude = fft_int[max];
  float harmonics_amplitude_sum_squared = 0.0f;
  int second = findMaxIndexInRange (fft_int, 150, 250);
  int third = findMaxIndexInRange (fft_int, 250, 350);
  int fourth = findMaxIndexInRange (fft_int, 350, 450);
  int fifth = findMaxIndexInRange (fft_int, 450, 511);
  harmonics_amplitude_sum_squared = fft_int[second] * fft_int[second]
      + fft_int[third] * fft_int[third] + fft_int[fourth] * fft_int[fourth]
      + fft_int[fifth] * fft_int[fifth];
  harmonics_amplitude_sum_squared = sqrtf (harmonics_amplitude_sum_squared);
  thd = 100 * (harmonics_amplitude_sum_squared) / (fundamental_amplitude);
  int a = 0;
  a = a + 1;
}
uint16_t findMax(uint16_t arr[], int size) {
	uint16_t max = 0;
	for (int i = 0; i < size; i++) {
		if (arr[i] > max) {
			max = arr[i];
		}
	}
	return max;
}

// 找出数组中的�???????小�??
uint16_t findMin(uint16_t arr[], int size) {
	uint16_t min = UINT16_MAX;
	for (int i = 0; i < size; i++) {
		if (arr[i] < min) {
			min = arr[i];
		}
	}
	return min;
}
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {

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
	MX_USART2_UART_Init();
	MX_CRC_Init();
	MX_ADC1_Init();
	MX_I2C1_Init();
	MX_DAC1_Init();
	MX_TIM6_Init();
	MX_ADC2_Init();
	MX_ADC3_Init();
	MX_ADC4_Init();
	MX_TIM1_Init();
	MX_DAC2_Init();
	/* USER CODE BEGIN 2 */
	for (int i = 0; i < 3; i++) {
		HAL_TIM_Base_Start(&htim6);
		HAL_ADC_Start_DMA(&hadc1, (uint32_t*) adc_buffer, samples);
		adc_ongoing = 1;
		while (adc_ongoing)
			;
		HAL_Delay(30);
	}
	float fft_inputbuf1[FFT_LENGTH * 2];	//FFT输入数组
	float fft_outputbuf1[FFT_LENGTH];	//FFT输出数组
	uint16_t fft_int1[FFT_LENGTH / 2];		//将输出变成整�???????
	for (int i = 0; i < FFT_LENGTH; i++)	//生成信号序列
			{
		fft_inputbuf1[2 * i] = adc_buffer[i] / 8;
		fft_inputbuf1[2 * i + 1] = 0;	//信号虚部，全部为0
	}
	arm_cfft_f32(&arm_cfft_sR_f32_len4096, fft_inputbuf1, 0, 1);
	arm_cmplx_mag_f32(fft_inputbuf1, fft_outputbuf1, FFT_LENGTH);
	for (int i = 0; i < FFT_LENGTH / 2; i++)	//生成信号序列
			{
		fft_int1[i] = (uint16_t) fft_outputbuf1[i];
	}
	fft_int1[0] = 0;
	int max1 = 15000 / findMaxIndexInRange(fft_int1, 0, 511);
	TIM6->ARR = max1 - 1;
	HAL_GPIO_WritePin(SW1_GPIO_Port, SW1_Pin, GPIO_PIN_RESET);
	HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 2048);
	HAL_DAC_Start(&hdac1, DAC_CHANNEL_1);
	HAL_DAC_SetValue(&hdac2, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 2048);
	HAL_DAC_Start(&hdac2, DAC_CHANNEL_1);
	int v = 1000;
	int vl = 200;
	int vr = 2000;
	int x = v * 4096 / 3300;
	int xl = vl * 4096 / 3300;
	int xr = vr * 4096 / 3300;
	uint16_t vpp = 0;
	HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_12B_R, x);
	HAL_DAC_Start(&hdac1, DAC_CHANNEL_1);
	HAL_Delay(20);
	for (int i = 0; i < 2; i++) {
		HAL_TIM_Base_Start(&htim6);
		HAL_ADC_Start_DMA(&hadc1, (uint32_t*) adc_buffer, samples);
		adc_ongoing = 1;
		while (adc_ongoing)
			;
		HAL_Delay(30);
	}
	vpp = findMax(adc_buffer, samples) - findMin(adc_buffer, samples);
	for (int i = 0; i < 10; i++) {
		if (vpp > 3000 * 4096 / 3300) {
			xl = x;
			x = (x + xr) / 2;
		} else {
			xr = x;
			x = (x + xl) / 2;
		}
		HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_12B_R, x);
		HAL_DAC_Start(&hdac1, DAC_CHANNEL_1);
		for (int i = 0; i < 2; i++) {
			HAL_TIM_Base_Start(&htim6);
			HAL_ADC_Start_DMA(&hadc1, (uint32_t*) adc_buffer, samples);
			adc_ongoing = 1;
			while (adc_ongoing)
				;
			HAL_Delay(30);
		}
		vpp = findMax(adc_buffer, samples) - findMin(adc_buffer, samples);
	}
	HAL_DAC_SetValue(&hdac2, DAC_CHANNEL_1, DAC_ALIGN_12B_R,
			(int) 2200 * 4096 / 3300);
	HAL_DAC_Start(&hdac2, DAC_CHANNEL_1);
	uart_init();
	uart_register_handler("I2C", i2c_handler);
	uart_register_handler("ADC", adc_handler);
//	TIM6->ARR = 299;
//	for (int i = 0; i < 3; i++) {
//		HAL_TIM_Base_Start(&htim6);
//		HAL_ADC_Start_DMA(&hadc1, (uint32_t*) adc_buffer, samples);
//		adc_ongoing = 1;
//		while (adc_ongoing)
//			;
//		HAL_Delay(300);
//	}
//	for (int i = 0; i < FFT_LENGTH; i++)	//生成信号序列
//			{
//		fft_inputbuf[2 * i] = adc_buffer[i] / 4;
//		fft_inputbuf[2 * i + 1] = 0;	//信号虚部，全部为0
//	}
//	arm_cfft_f32(&arm_cfft_sR_f32_len4096, fft_inputbuf, 0, 1);
//	arm_cmplx_mag_f32(fft_inputbuf, fft_outputbuf, FFT_LENGTH);
//	for (int i = 0; i < FFT_LENGTH; i++)	//生成信号序列
//			{
//		fft_int[i] = (uint16_t) fft_outputbuf[i];
//	}
//	fft_int[0] = 0;
//	max = 3000 / findMaxIndexInRange(fft_int, 0, 511);
//	TIM6->PSC = 9;
//	TIM6->ARR = (max - 1);
//	for (int i = 0; i < 3; i++) {
//		HAL_TIM_Base_Start(&htim6);
//		HAL_ADC_Start_DMA(&hadc1, (uint32_t*) adc_buffer, samples);
//		adc_ongoing = 1;
//		while (adc_ongoing)
//			;
//		HAL_Delay(30);
//	}
//	for (int i = 0; i < FFT_LENGTH; i++)	//生成信号序列
//			{
//		fft_inputbuf[2 * i] = adc_buffer[i] / 8;
//		fft_inputbuf[2 * i + 1] = 0;	//信号虚部，全部为0
//	}
//	arm_cfft_f32(&arm_cfft_sR_f32_len4096, fft_inputbuf, 0, 1);
//	arm_cmplx_mag_f32(fft_inputbuf, fft_outputbuf, FFT_LENGTH);
//	for (int i = 0; i < FFT_LENGTH / 2; i++)	//生成信号序列
//			{
//		fft_int[i] = (uint16_t) fft_outputbuf[i];
//	}
//	fft_int[0] = 0;
//	split_uint16_to_uint8(fft_int, samples /2, adc_buffer1);
//	calculateTHD(fft_int);

//	for (int i = 0; i < 3; i++) {
//		HAL_TIM_Base_Start(&htim6);
//		HAL_ADC_Start_DMA(&hadc1, (uint32_t*) adc_buffer, samples);
//		adc_ongoing = 1;
//		while (adc_ongoing)
//			;
//		HAL_Delay(1000);
//	}
//	for (int i = 0; i < FFT_LENGTH; i++)	//生成信号序列
//			{
//		fft_inputbuf[2 * i] = adc_buffer[i];
//		fft_inputbuf[2 * i + 1] = 0;	//信号虚部，全部为0
//	}
//	arm_cfft_f32(&arm_cfft_sR_f32_len4096, fft_inputbuf, 0, 1);
//	arm_cmplx_mag_f32(fft_inputbuf, fft_outputbuf, FFT_LENGTH);
//	for (int i = 0; i < FFT_LENGTH / 2; i++)	//生成信号序列
//			{
//		fft_int[i] = (uint16_t) fft_outputbuf[i]/2;
//	}
//	fft_int[0] = 0;
//	max = 20000/findMaxIndexInRange(fft_int, 0,511);
//	TIM6->ARR = max-1;
//	for (int i = 0; i < 3; i++) {
//		HAL_TIM_Base_Start(&htim6);
//		HAL_ADC_Start_DMA(&hadc1, (uint32_t*) adc_buffer, samples);
//		adc_ongoing = 1;
//		while (adc_ongoing)
//			;
//		HAL_Delay(1000);
//	}
//	for (int i = 0; i < FFT_LENGTH; i++)	//生成信号序列
//			{
//		fft_inputbuf[2 * i] = adc_buffer[i];
//		fft_inputbuf[2 * i + 1] = 0;	//信号虚部，全部为0
//	}
//	arm_cfft_f32(&arm_cfft_sR_f32_len4096, fft_inputbuf, 0, 1);
//	arm_cmplx_mag_f32(fft_inputbuf, fft_outputbuf, FFT_LENGTH);
//	for (int i = 0; i < FFT_LENGTH / 2; i++)	//生成信号序列
//			{
//		fft_int[i] = (uint16_t) fft_outputbuf[i]/2;
//	}
//	fft_int[0] = 0;
//	split_uint16_to_uint8(fft_int, samples / 2, adc_buffer1);
//	calculateTHD(fft_int);
	TIM6->ARR = 149;
	for (int i = 0; i < 3; i++) {
		HAL_TIM_Base_Start(&htim6);
		HAL_ADC_Start_DMA(&hadc1, (uint32_t*) adc_buffer, samples1);
		adc_ongoing = 1;
		while (adc_ongoing)
			;
		HAL_Delay(1000);
	}
	float fft_inputbuf[FFT_LENGTH1 * 2];	//FFT输入数组
	float fft_outputbuf[FFT_LENGTH1];	//FFT输出数组
	uint16_t fft_int[FFT_LENGTH1 / 2];		//将输出变成整�??
	for (int i = 0; i < FFT_LENGTH1; i++)	//生成信号序列
			{
		fft_inputbuf[2 * i] = adc_buffer[i] / 8;
		fft_inputbuf[2 * i + 1] = 0;	//信号虚部，全部为0
	}
	arm_cfft_f32(&arm_cfft_sR_f32_len1024, fft_inputbuf, 0, 1);
	arm_cmplx_mag_f32(fft_inputbuf, fft_outputbuf, FFT_LENGTH1);
	for (int i = 0; i < FFT_LENGTH1 / 2; i++)	//生成信号序列
			{
		fft_int[i] = (uint16_t) fft_outputbuf[i];
	}
	fft_int[0] = 0;
	if (findMaxIndexInRange(fft_int, 0, 511) < 2) {
		TIM6->PSC = 9;
		for (int i = 0; i < 3; i++) {
			HAL_TIM_Base_Start(&htim6);
			HAL_ADC_Start_DMA(&hadc1, (uint32_t*) adc_buffer, samples1);
			adc_ongoing = 1;
			while (adc_ongoing)
				;
			HAL_Delay(1000);
		}
		for (int i = 0; i < FFT_LENGTH1; i++)	//生成信号序列
				{
			fft_inputbuf[2 * i] = adc_buffer[i] / 8;
			fft_inputbuf[2 * i + 1] = 0;	//信号虚部，全部为0
		}
		arm_cfft_f32(&arm_cfft_sR_f32_len1024, fft_inputbuf, 0, 1);
		arm_cmplx_mag_f32(fft_inputbuf, fft_outputbuf, FFT_LENGTH1);
		for (int i = 0; i < FFT_LENGTH1 / 2; i++)	//生成信号序列
				{
			fft_int[i] = (uint16_t) fft_outputbuf[i];
		}
		fft_int[0] = 0;
	}
	int max = 14000 / findMaxIndexInRange(fft_int, 0, 511);
	TIM6->ARR = (max - 1);
	for (int i = 0; i < 3; i++) {
		HAL_TIM_Base_Start(&htim6);
		HAL_ADC_Start_DMA(&hadc1, (uint32_t*) adc_buffer, samples1);
		adc_ongoing = 1;
		while (adc_ongoing)
			;
		HAL_Delay(1000);
	}
	for (int i = 0; i < FFT_LENGTH1; i++)	//生成信号序列
			{
		fft_inputbuf[2 * i] = adc_buffer[i] / 8;
		fft_inputbuf[2 * i + 1] = 0;	//信号虚部，全部为0
	}
	arm_cfft_f32(&arm_cfft_sR_f32_len1024, fft_inputbuf, 0, 1);
	arm_cmplx_mag_f32(fft_inputbuf, fft_outputbuf, FFT_LENGTH1);
	for (int i = 0; i < FFT_LENGTH1 / 2; i++)	//生成信号序列
			{
		fft_int[i] = (uint16_t) fft_outputbuf[i];
	}
	fft_int[0] = 0;
	split_uint16_to_uint8(fft_int, samples1 / 2, adc_buffer1);
	calculateTHD(fft_int);
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */

	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Configure the main internal regulator output voltage
	 */
	HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI
			| RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV5;
	RCC_OscInitStruct.PLL.PLLN = 60;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV5;
	RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
	RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
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

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK) {
		Error_Handler();
	}
}

/**
 * @brief ADC1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_ADC1_Init(void) {

	/* USER CODE BEGIN ADC1_Init 0 */

	/* USER CODE END ADC1_Init 0 */

	ADC_MultiModeTypeDef multimode = { 0 };
	ADC_ChannelConfTypeDef sConfig = { 0 };

	/* USER CODE BEGIN ADC1_Init 1 */

	/* USER CODE END ADC1_Init 1 */

	/** Common config
	 */
	hadc1.Instance = ADC1;
	hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
	hadc1.Init.Resolution = ADC_RESOLUTION_12B;
	hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc1.Init.GainCompensation = 0;
	hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
	hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
	hadc1.Init.LowPowerAutoWait = DISABLE;
	hadc1.Init.ContinuousConvMode = DISABLE;
	hadc1.Init.NbrOfConversion = 1;
	hadc1.Init.DiscontinuousConvMode = DISABLE;
	hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIG_T6_TRGO;
	hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
	hadc1.Init.DMAContinuousRequests = DISABLE;
	hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
	hadc1.Init.OversamplingMode = DISABLE;
	if (HAL_ADC_Init(&hadc1) != HAL_OK) {
		Error_Handler();
	}

	/** Configure the ADC multi-mode
	 */
	multimode.Mode = ADC_MODE_INDEPENDENT;
	if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK) {
		Error_Handler();
	}

	/** Configure Regular Channel
	 */
	sConfig.Channel = ADC_CHANNEL_3;
	sConfig.Rank = ADC_REGULAR_RANK_1;
	sConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;
	sConfig.SingleDiff = ADC_SINGLE_ENDED;
	sConfig.OffsetNumber = ADC_OFFSET_NONE;
	sConfig.Offset = 0;
	if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN ADC1_Init 2 */

	/* USER CODE END ADC1_Init 2 */

}

/**
 * @brief ADC2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_ADC2_Init(void) {

	/* USER CODE BEGIN ADC2_Init 0 */

	/* USER CODE END ADC2_Init 0 */

	ADC_ChannelConfTypeDef sConfig = { 0 };

	/* USER CODE BEGIN ADC2_Init 1 */

	/* USER CODE END ADC2_Init 1 */

	/** Common config
	 */
	hadc2.Instance = ADC2;
	hadc2.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
	hadc2.Init.Resolution = ADC_RESOLUTION_12B;
	hadc2.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc2.Init.GainCompensation = 0;
	hadc2.Init.ScanConvMode = ADC_SCAN_DISABLE;
	hadc2.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
	hadc2.Init.LowPowerAutoWait = DISABLE;
	hadc2.Init.ContinuousConvMode = DISABLE;
	hadc2.Init.NbrOfConversion = 1;
	hadc2.Init.DiscontinuousConvMode = DISABLE;
	hadc2.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	hadc2.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
	hadc2.Init.DMAContinuousRequests = DISABLE;
	hadc2.Init.Overrun = ADC_OVR_DATA_PRESERVED;
	hadc2.Init.OversamplingMode = DISABLE;
	if (HAL_ADC_Init(&hadc2) != HAL_OK) {
		Error_Handler();
	}

	/** Configure Regular Channel
	 */
	sConfig.Channel = ADC_CHANNEL_2;
	sConfig.Rank = ADC_REGULAR_RANK_1;
	sConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;
	sConfig.SingleDiff = ADC_SINGLE_ENDED;
	sConfig.OffsetNumber = ADC_OFFSET_NONE;
	sConfig.Offset = 0;
	if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN ADC2_Init 2 */

	/* USER CODE END ADC2_Init 2 */

}

/**
 * @brief ADC3 Initialization Function
 * @param None
 * @retval None
 */
static void MX_ADC3_Init(void) {

	/* USER CODE BEGIN ADC3_Init 0 */

	/* USER CODE END ADC3_Init 0 */

	ADC_MultiModeTypeDef multimode = { 0 };
	ADC_ChannelConfTypeDef sConfig = { 0 };

	/* USER CODE BEGIN ADC3_Init 1 */

	/* USER CODE END ADC3_Init 1 */

	/** Common config
	 */
	hadc3.Instance = ADC3;
	hadc3.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
	hadc3.Init.Resolution = ADC_RESOLUTION_12B;
	hadc3.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc3.Init.GainCompensation = 0;
	hadc3.Init.ScanConvMode = ADC_SCAN_DISABLE;
	hadc3.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
	hadc3.Init.LowPowerAutoWait = DISABLE;
	hadc3.Init.ContinuousConvMode = DISABLE;
	hadc3.Init.NbrOfConversion = 1;
	hadc3.Init.DiscontinuousConvMode = DISABLE;
	hadc3.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	hadc3.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
	hadc3.Init.DMAContinuousRequests = DISABLE;
	hadc3.Init.Overrun = ADC_OVR_DATA_PRESERVED;
	hadc3.Init.OversamplingMode = DISABLE;
	if (HAL_ADC_Init(&hadc3) != HAL_OK) {
		Error_Handler();
	}

	/** Configure the ADC multi-mode
	 */
	multimode.Mode = ADC_MODE_INDEPENDENT;
	if (HAL_ADCEx_MultiModeConfigChannel(&hadc3, &multimode) != HAL_OK) {
		Error_Handler();
	}

	/** Configure Regular Channel
	 */
	sConfig.Channel = ADC_CHANNEL_5;
	sConfig.Rank = ADC_REGULAR_RANK_1;
	sConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;
	sConfig.SingleDiff = ADC_SINGLE_ENDED;
	sConfig.OffsetNumber = ADC_OFFSET_NONE;
	sConfig.Offset = 0;
	if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN ADC3_Init 2 */

	/* USER CODE END ADC3_Init 2 */

}

/**
 * @brief ADC4 Initialization Function
 * @param None
 * @retval None
 */
static void MX_ADC4_Init(void) {

	/* USER CODE BEGIN ADC4_Init 0 */

	/* USER CODE END ADC4_Init 0 */

	ADC_ChannelConfTypeDef sConfig = { 0 };

	/* USER CODE BEGIN ADC4_Init 1 */

	/* USER CODE END ADC4_Init 1 */

	/** Common config
	 */
	hadc4.Instance = ADC4;
	hadc4.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
	hadc4.Init.Resolution = ADC_RESOLUTION_12B;
	hadc4.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc4.Init.GainCompensation = 0;
	hadc4.Init.ScanConvMode = ADC_SCAN_DISABLE;
	hadc4.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
	hadc4.Init.LowPowerAutoWait = DISABLE;
	hadc4.Init.ContinuousConvMode = DISABLE;
	hadc4.Init.NbrOfConversion = 1;
	hadc4.Init.DiscontinuousConvMode = DISABLE;
	hadc4.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	hadc4.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
	hadc4.Init.DMAContinuousRequests = DISABLE;
	hadc4.Init.Overrun = ADC_OVR_DATA_PRESERVED;
	hadc4.Init.OversamplingMode = DISABLE;
	if (HAL_ADC_Init(&hadc4) != HAL_OK) {
		Error_Handler();
	}

	/** Configure Regular Channel
	 */
	sConfig.Channel = ADC_CHANNEL_4;
	sConfig.Rank = ADC_REGULAR_RANK_1;
	sConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;
	sConfig.SingleDiff = ADC_SINGLE_ENDED;
	sConfig.OffsetNumber = ADC_OFFSET_NONE;
	sConfig.Offset = 0;
	if (HAL_ADC_ConfigChannel(&hadc4, &sConfig) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN ADC4_Init 2 */

	/* USER CODE END ADC4_Init 2 */

}

/**
 * @brief CRC Initialization Function
 * @param None
 * @retval None
 */
static void MX_CRC_Init(void) {

	/* USER CODE BEGIN CRC_Init 0 */

	/* USER CODE END CRC_Init 0 */

	/* USER CODE BEGIN CRC_Init 1 */

	/* USER CODE END CRC_Init 1 */
	hcrc.Instance = CRC;
	hcrc.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_DISABLE;
	hcrc.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_DISABLE;
	hcrc.Init.GeneratingPolynomial = 4129;
	hcrc.Init.CRCLength = CRC_POLYLENGTH_16B;
	hcrc.Init.InitValue = 0;
	hcrc.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_NONE;
	hcrc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE;
	hcrc.InputDataFormat = CRC_INPUTDATA_FORMAT_BYTES;
	if (HAL_CRC_Init(&hcrc) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN CRC_Init 2 */

	/* USER CODE END CRC_Init 2 */

}

/**
 * @brief DAC1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_DAC1_Init(void) {

	/* USER CODE BEGIN DAC1_Init 0 */

	/* USER CODE END DAC1_Init 0 */

	DAC_ChannelConfTypeDef sConfig = { 0 };

	/* USER CODE BEGIN DAC1_Init 1 */

	/* USER CODE END DAC1_Init 1 */

	/** DAC Initialization
	 */
	hdac1.Instance = DAC1;
	if (HAL_DAC_Init(&hdac1) != HAL_OK) {
		Error_Handler();
	}

	/** DAC channel OUT1 config
	 */
	sConfig.DAC_HighFrequency = DAC_HIGH_FREQUENCY_INTERFACE_MODE_AUTOMATIC;
	sConfig.DAC_DMADoubleDataMode = DISABLE;
	sConfig.DAC_SignedFormat = DISABLE;
	sConfig.DAC_SampleAndHold = DAC_SAMPLEANDHOLD_DISABLE;
	sConfig.DAC_Trigger = DAC_TRIGGER_SOFTWARE;
	sConfig.DAC_Trigger2 = DAC_TRIGGER_NONE;
	sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
	sConfig.DAC_ConnectOnChipPeripheral = DAC_CHIPCONNECT_EXTERNAL;
	sConfig.DAC_UserTrimming = DAC_TRIMMING_FACTORY;
	if (HAL_DAC_ConfigChannel(&hdac1, &sConfig, DAC_CHANNEL_1) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN DAC1_Init 2 */

	/* USER CODE END DAC1_Init 2 */

}

/**
 * @brief DAC2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_DAC2_Init(void) {

	/* USER CODE BEGIN DAC2_Init 0 */

	/* USER CODE END DAC2_Init 0 */

	DAC_ChannelConfTypeDef sConfig = { 0 };

	/* USER CODE BEGIN DAC2_Init 1 */

	/* USER CODE END DAC2_Init 1 */

	/** DAC Initialization
	 */
	hdac2.Instance = DAC2;
	if (HAL_DAC_Init(&hdac2) != HAL_OK) {
		Error_Handler();
	}

	/** DAC channel OUT1 config
	 */
	sConfig.DAC_HighFrequency = DAC_HIGH_FREQUENCY_INTERFACE_MODE_AUTOMATIC;
	sConfig.DAC_DMADoubleDataMode = DISABLE;
	sConfig.DAC_SignedFormat = DISABLE;
	sConfig.DAC_SampleAndHold = DAC_SAMPLEANDHOLD_DISABLE;
	sConfig.DAC_Trigger = DAC_TRIGGER_SOFTWARE;
	sConfig.DAC_Trigger2 = DAC_TRIGGER_NONE;
	sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
	sConfig.DAC_ConnectOnChipPeripheral = DAC_CHIPCONNECT_EXTERNAL;
	sConfig.DAC_UserTrimming = DAC_TRIMMING_FACTORY;
	if (HAL_DAC_ConfigChannel(&hdac2, &sConfig, DAC_CHANNEL_1) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN DAC2_Init 2 */

	/* USER CODE END DAC2_Init 2 */

}

/**
 * @brief I2C1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_I2C1_Init(void) {

	/* USER CODE BEGIN I2C1_Init 0 */

	/* USER CODE END I2C1_Init 0 */

	/* USER CODE BEGIN I2C1_Init 1 */

	/* USER CODE END I2C1_Init 1 */
	hi2c1.Instance = I2C1;
	hi2c1.Init.Timing = 0x00E063FF;
	hi2c1.Init.OwnAddress1 = 0;
	hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c1.Init.OwnAddress2 = 0;
	hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
	hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	if (HAL_I2C_Init(&hi2c1) != HAL_OK) {
		Error_Handler();
	}

	/** Configure Analogue filter
	 */
	if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE)
			!= HAL_OK) {
		Error_Handler();
	}

	/** Configure Digital filter
	 */
	if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN I2C1_Init 2 */

	/* USER CODE END I2C1_Init 2 */

}

/**
 * @brief TIM1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM1_Init(void) {

	/* USER CODE BEGIN TIM1_Init 0 */

	/* USER CODE END TIM1_Init 0 */

	TIM_MasterConfigTypeDef sMasterConfig = { 0 };
	TIM_OC_InitTypeDef sConfigOC = { 0 };
	TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = { 0 };

	/* USER CODE BEGIN TIM1_Init 1 */

	/* USER CODE END TIM1_Init 1 */
	htim1.Instance = TIM1;
	htim1.Init.Prescaler = 0;
	htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim1.Init.Period = 149;
	htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim1.Init.RepetitionCounter = 0;
	htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_PWM_Init(&htim1) != HAL_OK) {
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig)
			!= HAL_OK) {
		Error_Handler();
	}
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = 75;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
	sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
	if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1)
			!= HAL_OK) {
		Error_Handler();
	}
	sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
	sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
	sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
	sBreakDeadTimeConfig.DeadTime = 0;
	sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
	sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
	sBreakDeadTimeConfig.BreakFilter = 0;
	sBreakDeadTimeConfig.BreakAFMode = TIM_BREAK_AFMODE_INPUT;
	sBreakDeadTimeConfig.Break2State = TIM_BREAK2_DISABLE;
	sBreakDeadTimeConfig.Break2Polarity = TIM_BREAK2POLARITY_HIGH;
	sBreakDeadTimeConfig.Break2Filter = 0;
	sBreakDeadTimeConfig.Break2AFMode = TIM_BREAK_AFMODE_INPUT;
	sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
	if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig)
			!= HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN TIM1_Init 2 */

	/* USER CODE END TIM1_Init 2 */
	HAL_TIM_MspPostInit(&htim1);

}

/**
 * @brief TIM6 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM6_Init(void) {

	/* USER CODE BEGIN TIM6_Init 0 */

	/* USER CODE END TIM6_Init 0 */

	TIM_MasterConfigTypeDef sMasterConfig = { 0 };

	/* USER CODE BEGIN TIM6_Init 1 */

	/* USER CODE END TIM6_Init 1 */
	htim6.Instance = TIM6;
	htim6.Init.Prescaler = 0;
	htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim6.Init.Period = 149;
	htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
	if (HAL_TIM_Base_Init(&htim6) != HAL_OK) {
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig)
			!= HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN TIM6_Init 2 */

	/* USER CODE END TIM6_Init 2 */

}

/**
 * @brief USART2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART2_UART_Init(void) {

	/* USER CODE BEGIN USART2_Init 0 */

	/* USER CODE END USART2_Init 0 */

	/* USER CODE BEGIN USART2_Init 1 */

	/* USER CODE END USART2_Init 1 */
	huart2.Instance = USART2;
	huart2.Init.BaudRate = 115200;
	huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_NONE;
	huart2.Init.Mode = UART_MODE_TX_RX;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.OverSampling = UART_OVERSAMPLING_16;
	huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	huart2.Init.ClockPrescaler = UART_PRESCALER_DIV1;
	huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	if (HAL_UART_Init(&huart2) != HAL_OK) {
		Error_Handler();
	}
	if (HAL_UARTEx_SetTxFifoThreshold(&huart2, UART_TXFIFO_THRESHOLD_1_8)
			!= HAL_OK) {
		Error_Handler();
	}
	if (HAL_UARTEx_SetRxFifoThreshold(&huart2, UART_RXFIFO_THRESHOLD_1_8)
			!= HAL_OK) {
		Error_Handler();
	}
	if (HAL_UARTEx_DisableFifoMode(&huart2) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN USART2_Init 2 */

	/* USER CODE END USART2_Init 2 */

}

/**
 * Enable DMA controller clock
 */
static void MX_DMA_Init(void) {

	/* DMA controller clock enable */
	__HAL_RCC_DMAMUX1_CLK_ENABLE();
	__HAL_RCC_DMA1_CLK_ENABLE();

	/* DMA interrupt init */
	/* DMA1_Channel1_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 1, 0);
	HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
	/* DMA1_Channel2_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(DMA1_Channel2_IRQn, 1, 0);
	HAL_NVIC_EnableIRQ(DMA1_Channel2_IRQn);
	/* DMA1_Channel3_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(DMA1_Channel3_IRQn, 1, 0);
	HAL_NVIC_EnableIRQ(DMA1_Channel3_IRQn);
	/* DMA1_Channel4_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(DMA1_Channel4_IRQn, 1, 0);
	HAL_NVIC_EnableIRQ(DMA1_Channel4_IRQn);

}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	/* USER CODE BEGIN MX_GPIO_Init_1 */
	/* USER CODE END MX_GPIO_Init_1 */

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOF_CLK_ENABLE();
	__HAL_RCC_GPIOG_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(SW1_GPIO_Port, SW1_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_10, GPIO_PIN_RESET);

	/*Configure GPIO pin : BTN_Pin */
	GPIO_InitStruct.Pin = BTN_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(BTN_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : SW1_Pin */
	GPIO_InitStruct.Pin = SW1_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(SW1_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : PG10 */
	GPIO_InitStruct.Pin = GPIO_PIN_10;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

	/* USER CODE BEGIN MX_GPIO_Init_2 */
	/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void floatToString(float num, char *str, int precision) {
// 提取整数部分
	int integerPart = (int) num;
	num -= integerPart;

// 确定符号
	int isNegative = 0;
	if (integerPart < 0) {
		isNegative = 1;
		integerPart = -integerPart;
		num = -num;
	}

// 提取小数部分
	for (int i = 0; i < precision; ++i) {
		num *= 10;
	}
	int decimalPart = (int) (num + 0.5); // 四舍五入

// 将整数部分转换为字符�???????
	char buffer[20];
	int index = 0;
	do {
		buffer[index++] = (integerPart % 10) + '0';
		integerPart /= 10;
	} while (integerPart > 0);

	if (isNegative) {
		buffer[index++] = '-';
	}
	buffer[index] = '\0';

// 反转整数部分字符�???????
	for (int i = 0; i < index / 2; ++i) {
		char temp = buffer[i];
		buffer[i] = buffer[index - i - 1];
		buffer[index - i - 1] = temp;
	}

// 将整数部分复制到结果字符�???????
	int strIndex = 0;
	for (int i = 0; buffer[i] != '\0'; ++i) {
		str[strIndex++] = buffer[i];
	}

// 添加小数�???????
	str[strIndex++] = '.';

// 将小数部分转换为字符�???????
	index = 0;
	do {
		buffer[index++] = (decimalPart % 10) + '0';
		decimalPart /= 10;
	} while (decimalPart > 0);

// 补齐小数部分不足的位�???????
	while (index < precision) {
		buffer[index++] = '0';
	}
	buffer[index] = '\0';

// 反转小数部分字符�???????
	for (int i = 0; i < index / 2; ++i) {
		char temp = buffer[i];
		buffer[i] = buffer[index - i - 1];
		buffer[index - i - 1] = temp;
	}

// 将小数部分复制到结果字符�???????
	for (int i = 0; buffer[i] != '\0'; ++i) {
		str[strIndex++] = buffer[i];
	}
	str[strIndex] = '\0';
}
/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
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
