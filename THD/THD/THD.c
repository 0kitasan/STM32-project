#include "CalcLib_FFT.h"
#include "DrivLib_TM1638.h"
#include "arm_const_structs.h"
#include "arm_math.h"
#include "ti_msp_dl_config.h"
#include <stdio.h>

/*
PA15 DAC12
PA22 DAC8(OPA)
PA25 ADC
*/

#define FFT_LENGTH 1024
#define ADC_SAMPLE_SIZE 1024
// unsigned short ADCResult[ADCSize]; // 定义ADC接收缓冲区
uint16_t adc_result[ADC_SAMPLE_SIZE]; // 定义ADC接收缓冲区
volatile bool adc_flag = false;
uint32_t adc_sample_rate;

void setupDMA(DMA_Regs *dma, uint8_t channelNum, unsigned int srcAddr,
              unsigned int destAddr, unsigned int transferSize) {
  DL_DMA_setSrcAddr(dma, channelNum, (unsigned int)srcAddr);
  DL_DMA_setDestAddr(dma, channelNum, (unsigned int)destAddr);
  DL_DMA_setTransferSize(dma, channelNum, transferSize);
  DL_DMA_enableChannel(dma, channelNum);
}

void dac8_byOPA_output(COMP_Regs *comp, OA_Regs *opa, uint32_t value) {
  static uint16_t COMP_REF_VOLTAGE_mV = 3300;
  uint16_t dacValue = value * 255 / COMP_REF_VOLTAGE_mV;
  DL_COMP_setDACCode0(comp, dacValue);
  DL_COMP_enable(comp);
  DL_OPA_enable(opa);
}

void dac12_output(DAC12_Regs *dac12, uint32_t value) {
  static uint16_t DAC12_REF_VOLTAGE_mV = 2500;
  uint16_t dacValue = value * 4095 / DAC12_REF_VOLTAGE_mV;
  DL_DAC12_output12(dac12, dacValue);
  DL_DAC12_enable(dac12);
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
  adc_flag = false;
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
  DL_GPIO_setPins(GPIO_OUTPUT_PORT, GPIO_OUTPUT_DIODE_PIN);
  DL_GPIO_clearPins(GPIO_OUTPUT_PORT, GPIO_OUTPUT_DIODE_PIN);

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
  //   for (int i = 0; i < FFT_LENGTH / 2; i++) // 生成信号序列
  //   {
  //     fft_int1[i] = (uint16_t)fft_outputbuf1[i];
  //   }
  //   fft_int1[0] = 0;

  //   int max1 = 15000 / findMaxIndexInRange(fft_int1, 0, 511);
  //   TIM6->ARR = max1 - 1;
  //   // control mode
  //   HAL_GPIO_WritePin(SW1_GPIO_Port, SW1_Pin, GPIO_PIN_RESET);
  //   HAL_DAC_SetValue(&hdac2, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 2048); // offset
  //   HAL_DAC_Start(&hdac2, DAC_CHANNEL_1);
  //   int v = 1100;
  //   int vl = 200;
  //   int vr = 2000;
  //   int x = v * 4096 / 3300;
  //   int xl = vl * 4096 / 3300;
  //   int xr = vr * 4096 / 3300;
  //   uint16_t vpp = 0;
  //   HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_12B_R, x); // gain
  //   HAL_DAC_Start(&hdac1, DAC_CHANNEL_1);
  //   HAL_Delay(20);
  //   for (int i = 0; i < 2; i++) {
  //     HAL_TIM_Base_Start(&htim6);
  //     HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_buffer, samples);
  //     adc_ongoing = 1;
  //     while (adc_ongoing)
  //       ;
  //     HAL_Delay(30);
  //   }
  //   vpp = findMax(adc_buffer, samples) - findMin(adc_buffer, samples);
  //   for (int i = 0; i < 10; i++) {
  //     if (vpp > 3000 * 4096 / 3300) {
  //       xl = x;
  //       x = (x + xr) / 2;
  //     } else {
  //       xr = x;
  //       x = (x + xl) / 2;
  //     }
  //     HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_12B_R, x);
  //     HAL_DAC_Start(&hdac1, DAC_CHANNEL_1);
  //     for (int i = 0; i < 2; i++) {
  //       HAL_TIM_Base_Start(&htim6);
  //       HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_buffer, samples);
  //       adc_ongoing = 1;
  //       while (adc_ongoing)
  //         ;
  //       HAL_Delay(30);
  //     }
  //     vpp = findMax(adc_buffer, samples) - findMin(adc_buffer, samples);
  //   }
  //   HAL_DAC_SetValue(&hdac2, DAC_CHANNEL_1, DAC_ALIGN_12B_R,
  //                    (int)2200 * 4096 / 3300);
  //   HAL_DAC_Start(&hdac2, DAC_CHANNEL_1);

  dac8_byOPA_output(COMP_0_INST, OPA_0_INST, 1500);
  dac12_output(DAC0, 1400);
  __BKPT(0);

  // thd = calculateTHD(fft_outputbuf);
  while (1) {
  }
}

void ADC_get_sample() {}

void ADC12_0_INST_IRQHandler(void) {
  DL_ADC12_getPendingInterrupt(ADC12_0_INST);
  adc_flag = true;
}
