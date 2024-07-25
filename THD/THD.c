/*
 * Copyright (c) 2021, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "arm_const_structs.h"
#include "arm_math.h"
#include "ti_msp_dl_config.h"

#define FFT_LENGTH 1024
#define ADC_SAMPLE_SIZE 1024
// unsigned short ADCResult[ADCSize]; // 定义ADC接收缓冲区
uint16_t adc_result[ADC_SAMPLE_SIZE]; // 定义ADC接收缓冲区
volatile bool adc_flag;
uint32_t adc_sample_rate;

void setupDMA(DMA_Regs *dma, uint8_t channelNum, unsigned int srcAddr,
              unsigned int destAddr, unsigned int transferSize) {
  DL_DMA_setSrcAddr(dma, channelNum, (unsigned int)srcAddr);
  DL_DMA_setDestAddr(dma, channelNum, (unsigned int)destAddr);
  DL_DMA_setTransferSize(dma, channelNum, transferSize);
  DL_DMA_enableChannel(dma, channelNum);
}
uint32_t cal_ADC_SampleRate(uint16_t square_freq, uint16_t *adc_sample_res,
                            uint16_t adc_sample_size);

float calculateTHD(float fft_outputbuf[]) {
  float fundamental_amplitude = fft_outputbuf[1];
  float harmonics_amplitude_sum_squared = 0.0f;

  for (int i = 2; i < 6; i++) {
    harmonics_amplitude_sum_squared += fft_outputbuf[i] * fft_outputbuf[i];
  }

  float thd =
      sqrtf(harmonics_amplitude_sum_squared) / fundamental_amplitude * 100.0f;
  return thd;
}

float thd;

int main(void) {
  SYSCFG_DL_init();

  NVIC_EnableIRQ(ADC12_0_INST_INT_IRQN);

  __BKPT(0);

  setupDMA(DMA, DMA_CH0_CHAN_ID, (unsigned int)&(ADC0->ULLMEM.MEMRES[0]),
           (unsigned int)&adc_result, ADC_SAMPLE_SIZE);
  DL_Timer_startCounter(TIMER_0_INST);
  while (false == adc_flag) {
    __WFE();
  }
  adc_sample_rate = cal_ADC_SampleRate(10, adc_result, ADC_SAMPLE_SIZE);

  __BKPT(0);

  float fft_inputbuf[FFT_LENGTH * 2];  // FFT输入数组
  float fft_outputbuf[FFT_LENGTH];     // FFT输出数组
  for (int i = 0; i < FFT_LENGTH; i++) // 生成信号序列
  {
    fft_inputbuf[2 * i] = adc_result[i];
    // 信号实部，直流分量100,1HZ信号幅值为10，50HZ信号幅值为20，300HZ信号幅值为30。
    fft_inputbuf[2 * i + 1] = 0; // 信号虚部，全部为0
  }
  arm_cfft_f32(&arm_cfft_sR_f32_len1024, fft_inputbuf, 0, 1);
  arm_cmplx_mag_f32(fft_inputbuf, fft_outputbuf, FFT_LENGTH);
  adc_flag = false;

  thd = calculateTHD(fft_outputbuf);
  __BKPT(0);

  while (1) {
  }
}

void ADC12_0_INST_IRQHandler(void) {
  DL_ADC12_getPendingInterrupt(ADC12_0_INST);
  adc_flag = true;
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
