#include "CalcLib_FFT.h"
#include "DrivLib_TM1638.h"
#include "arm_const_structs.h"
#include "arm_math.h"
#include "ti_msp_dl_config.h"
#include <stdio.h>

#define FFT_LENGTH 1024
#define ADC_SAMPLE_SIZE 1024
// unsigned short ADCResult[ADCSize]; // 定义ADC接收缓冲区
uint16_t adc_result[ADC_SAMPLE_SIZE]; // 定义ADC接收缓冲区
volatile bool adc_flag=false;
uint32_t adc_sample_rate;

void setupDMA(DMA_Regs *dma, uint8_t channelNum, unsigned int srcAddr,
              unsigned int destAddr, unsigned int transferSize) {
  DL_DMA_setSrcAddr(dma, channelNum, (unsigned int)srcAddr);
  DL_DMA_setDestAddr(dma, channelNum, (unsigned int)destAddr);
  DL_DMA_setTransferSize(dma, channelNum, transferSize);
  DL_DMA_enableChannel(dma, channelNum);
}

float thd;
// 初始时应当以1MHz采样，TIM的Period应当为1us
uint32_t timer_cnt_reg;

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
  DL_Timer_stopCounter(TIMER_0_INST);
  adc_sample_rate = cal_ADC_SampleRate(10, adc_result, ADC_SAMPLE_SIZE);
  timer_cnt_reg = TIMA0->COUNTERREGS.LOAD;
  __BKPT(0);
  /* second time*/
  adc_flag=false;
  TIMA0->COUNTERREGS.LOAD = (TIMA0->COUNTERREGS.LOAD + 1) * 10 - 1;
  setupDMA(DMA, DMA_CH0_CHAN_ID, (unsigned int)&(ADC0->ULLMEM.MEMRES[0]),
           (unsigned int)&adc_result, ADC_SAMPLE_SIZE);
  DL_ADC12_enableConversions(ADC12_0_INST);
  DL_Timer_startCounter(TIMER_0_INST);

  while (false == adc_flag) {
    __WFE();
  }

  timer_cnt_reg = TIMA0->COUNTERREGS.LOAD;
  adc_sample_rate = cal_ADC_SampleRate(10, adc_result, ADC_SAMPLE_SIZE);

  __BKPT(0);

  //   float fft_inputbuf[FFT_LENGTH * 2];  // FFT输入数组
  //   float fft_outputbuf[FFT_LENGTH];     // FFT输出数组
  //   for (int i = 0; i < FFT_LENGTH; i++) // 生成信号序列
  //   {
  //     fft_inputbuf[2 * i] = adc_result[i];
  //     //
  //     信号实部，直流分量100,1HZ信号幅值为10，50HZ信号幅值为20，300HZ信号幅值为30。
  //     fft_inputbuf[2 * i + 1] = 0; // 信号虚部，全部为0
  //   }
  //   arm_cfft_f32(&arm_cfft_sR_f32_len1024, fft_inputbuf, 0, 1);
  //   arm_cmplx_mag_f32(fft_inputbuf, fft_outputbuf, FFT_LENGTH);
  //   adc_flag = false;

  //   thd = calculateTHD(fft_outputbuf);

  __BKPT(0);

  while (1) {
  }
}

void ADC_get_sample() {}

void ADC12_0_INST_IRQHandler(void) {
  DL_ADC12_getPendingInterrupt(ADC12_0_INST);
  adc_flag = true;
}
