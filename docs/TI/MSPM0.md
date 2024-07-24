# MSPM0

嵌入式任务：代码移植

## THD.v2 源码

### 函数与全局变量定义

```cpp
/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#define samples 1024
#define FFT_LENGTH 1024

uint16_t adc_buffer[samples];
uint8_t adc_buffer1[2 * samples];
uint16_t dac_buffer[100];
float32_t FFT_Input_Q15_f[50];
float32_t aFFT_Input_Q15[50];
volatile int adc_ongoing = 0; 

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

void dac_handler(uint8_t *data, size_t size) {
	HAL_DAC_Stop_DMA(&hdac1, DAC_CHANNEL_1);
	memcpy(dac_buffer, data, size);
	HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, (uint32_t*) dac_buffer, size / 2,
	DAC_ALIGN_12B_R);
	HAL_TIM_Base_Start(&htim6);
}

void adc_handler(uint8_t *data, size_t size) {
	uart_transmit(adc_buffer1, 2 * samples);
}

void split_uint16_to_uint8(const uint16_t *uint16_array,
		size_t uint16_array_size, uint8_t *uint8_array) {
	for (size_t i = 0; i < uint16_array_size; i++) {
		uint8_array[i * 2 + 1] = (uint8_t) (uint16_array[i] >> 8);
		uint8_array[i * 2] = (uint8_t) (uint16_array[i] & 0xFF);
	}
}

float calculateTHD(float fft_outputbuf[]) {
    float fundamental_amplitude = fft_outputbuf[1];
    float harmonics_amplitude_sum_squared = 0.0f;

    for (int i = 2; i < 6; i++) {
        harmonics_amplitude_sum_squared += fft_outputbuf[i] * fft_outputbuf[i];
    }

    float thd = sqrtf(harmonics_amplitude_sum_squared) / fundamental_amplitude * 100.0f;
    return thd;
}
/* USER CODE END 0 */
```

### main函数逻辑

v2版本只是调试程序，DAC 尚未配置

```cpp
int main(void) {
	/* ...INIT... */

	/* USER CODE BEGIN 2 */
	uart_init();
	uart_register_handler("I2C", i2c_handler);
	uart_register_handler("DAC", dac_handler);
	uart_register_handler("ADC", adc_handler);

	HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 2048);
	HAL_DAC_Start(&hdac1, DAC_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);

	int i1 = 3;
	while (i1--) {
		HAL_TIM_Base_Start(&htim6);
		HAL_ADC_Start_DMA(&hadc1, (uint32_t*) adc_buffer, samples);
		split_uint16_to_uint8(adc_buffer, samples, adc_buffer1);
		adc_ongoing = 1;
		while (adc_ongoing)
			;
		HAL_Delay(1000);
	}

	float fft_inputbuf[FFT_LENGTH * 2];	//FFT输入数组
	float fft_outputbuf[FFT_LENGTH];	//FFT输出数组
	uint16_t fft_int[FFT_LENGTH];		//将输出变成整数
	for (int i = 0; i < FFT_LENGTH; i++)	//生成信号序列
			{
		fft_inputbuf[2 * i] = adc_buffer[i];
		//信号实部，直流分量100,1HZ信号幅值为10，50HZ信号幅值为20，300HZ信号幅值为30。
		fft_inputbuf[2 * i + 1] = 0;	//信号虚部，全部为0
	}
	arm_cfft_f32(&arm_cfft_sR_f32_len1024, fft_inputbuf, 0, 1);
	arm_cmplx_mag_f32(fft_inputbuf, fft_outputbuf, FFT_LENGTH);
	for (int i = 0; i < FFT_LENGTH; i++)	//生成信号序列
			{
		fft_int[i]=(uint16_t) fft_outputbuf[i];
	}

	split_uint16_to_uint8(fft_int, samples, adc_buffer1);
	float thd = calculateTHD(fft_outputbuf);
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */

	}
	/* USER CODE END 3 */
}
```


## 迁移工作

需要回去查看 CubeMX ,此外,可以测试一下 TIM1638 的底层驱动移植.

### 外设、引脚配置

#### ADC-DMA

ADC 测量部分

```cpp
	int i1 = 3;
	while (i1--) {
		HAL_TIM_Base_Start(&htim6);
		HAL_ADC_Start_DMA(&hadc1, (uint32_t*) adc_buffer, samples);
		split_uint16_to_uint8(adc_buffer, samples, adc_buffer1);
		adc_ongoing = 1;
		while (adc_ongoing)
			;
		HAL_Delay(1000);
	}
```

#### TIM


#### UART，可以不用，但方便调试

#### DAC(v2 暂时不用)



### CMSIS-DSP:FFT

经简单测试, 在 MSPM0 上直接使用 float 的 fft 似乎是 ok 的.

```cpp
	float fft_inputbuf[FFT_LENGTH * 2];	//FFT输入数组
	float fft_outputbuf[FFT_LENGTH];	//FFT输出数组
	uint16_t fft_int[FFT_LENGTH];		//将输出变成整数
	for (int i = 0; i < FFT_LENGTH; i++)	//生成信号序列
			{
		fft_inputbuf[2 * i] = adc_buffer[i];
		//信号实部，直流分量100,1HZ信号幅值为10，50HZ信号幅值为20，300HZ信号幅值为30。
		fft_inputbuf[2 * i + 1] = 0;	//信号虚部，全部为0
	}
	arm_cfft_f32(&arm_cfft_sR_f32_len1024, fft_inputbuf, 0, 1);
	arm_cmplx_mag_f32(fft_inputbuf, fft_outputbuf, FFT_LENGTH);
	for (int i = 0; i < FFT_LENGTH; i++)	//生成信号序列
			{
		fft_int[i]=(uint16_t) fft_outputbuf[i];
	}
```


#### ENV

直接 import 即可.

#### Change to Fixed-Point Number

后续优化方案
