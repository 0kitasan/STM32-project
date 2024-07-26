// CalcLib_FFT.c
#include "CalcLib_FFT.h"
#include <math.h> // 包含数学函数库
#include <stdbool.h>

int findMaxIndexInRange(uint16_t arr[], int start, int end) {
	int maxIndex = start;
	for (int i = start + 1; i <= end; i++) {
		if (arr[i] > arr[maxIndex]) {
			maxIndex = i;
		}
	}
	return maxIndex;
}

float calculateTHD (uint16_t fft_int[])
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
  float thd = 100 * (harmonics_amplitude_sum_squared) / (fundamental_amplitude);
  return thd;
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

uint16_t findMin(uint16_t arr[], int size) {
	uint16_t min = UINT16_MAX;
	for (int i = 0; i < size; i++) {
		if (arr[i] < min) {
			min = arr[i];
		}
	}
	return min;
}

uint32_t cal_ADC_SampleRate(uint16_t square_freq, uint16_t *adc_sample_res,
                            uint16_t adc_sample_size) {
  // 该函数用于测试，输入一个固定频率的方波以计算ADC采样率
  // 单位与输入的square_freq保持一致
  uint16_t start_index = 10; // 从第10个样本开始以保证稳定
  uint16_t prev_sample = adc_sample_res[start_index];
  uint16_t first_fall_index = 0;
  uint16_t second_fall_index = 0;
  bool first_fall_found = false;
  // 寻找第一个下降沿
  for (uint16_t i = start_index + 1; i < adc_sample_size; i++) {
    if (prev_sample >= 2048 && adc_sample_res[i] < 2048) {
      if (!first_fall_found) {
        first_fall_index = i;
        first_fall_found = true;
      } else {
        second_fall_index = i;
        break;
      }
    }
    prev_sample = adc_sample_res[i];
  }
  // 计算从第一个下降沿到第二个下降沿的样本数
  uint16_t period_samples = second_fall_index - first_fall_index;
  // 计算采样率：采样率 = 方波频率 * 周期样本数
  uint32_t sample_rate = period_samples * square_freq;

  return sample_rate;
}