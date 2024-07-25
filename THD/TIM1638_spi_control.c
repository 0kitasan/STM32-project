#include "ti/driverlib/dl_gpio.h"
#include "ti_msp_dl_config.h"
#include <string.h>

// 引脚定义
// 可以尝试用GPIO控制CS
// #define GPIO_SPI_0_CS0_PORT GPIOB #define GPIO_SPI_0_CS0_PIN DL_GPIO_PIN_6
// PB9 SCLK
// PB8 MOSI
// PB7 MISO
// PB6 CS0

// PB13 GPIO_CS

uint8_t DIGITS[] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f};

#define CHAR_TABLE_SIZE 128
const uint8_t charTable[CHAR_TABLE_SIZE] = {
    ['0'] = 0x3f, // 0b00111111
    ['1'] = 0x06, // 0b00000110
    ['2'] = 0x5b, // 0b01011011
    ['3'] = 0x4f, // 0b01001111
    ['4'] = 0x66, // 0b01100110
    ['5'] = 0x6d, // 0b01101101
    ['6'] = 0x7d, // 0b01111101
    ['7'] = 0x07, // 0b00000111
    ['8'] = 0x7f, // 0b01111111
    ['9'] = 0x6f, // 0b01101111
    ['A'] = 0x77, // 0b01110111
    ['b'] = 0x7C, // 0b01111100
    ['C'] = 0x39, // 0b00111001
    ['d'] = 0x5E, // 0b01011110
    ['E'] = 0x79, // 0b01111001
    ['F'] = 0x71, // 0b01110001
    ['H'] = 0x76, // 0b01110110
    ['I'] = 0x30, // 0b00110000
    ['J'] = 0x1E, // 0b00011110
    ['L'] = 0x38, // 0b00111000
    ['o'] = 0x5C, // 0b01011100
    ['P'] = 0x73, // 0b01110011
    ['r'] = 0x50, // 0b01010000
    ['U'] = 0x3E, // 0b00111110
    [' '] = 0x00, // 0b00000000
    ['.'] = 0x80  // 0b10000000
};

// 将单个字符转换为七段显示器编码
uint8_t charToSegment(char c) {
  if (c < CHAR_TABLE_SIZE) {
    return charTable[(uint8_t)c];
  }
  return 0x00; // 对于不在表中的字符，返回空显示编码
}

// 将字符串转换为七段显示器编码数组
void stringToSegments(const char *str, uint8_t *segments, size_t length) {
  for (size_t i = 0; i < length; i++) {
    segments[i] = charToSegment(str[i]);
  }
}

void spi_transmit_byte(uint8_t data) {
  while (DL_SPI_isBusy(SPI_0_INST))
    ;
  DL_SPI_transmitData8(SPI_0_INST, data);
  while (DL_SPI_isBusy(SPI_0_INST))
    ;
}

// 由于自增模式出现了显示起点偏移的bug，因此只能使用固定地址模式
void spi_send_cmd(uint8_t cmd) {
  DL_GPIO_clearPins(GPIO_CS_PORT, GPIO_CS_PIN_PIN);
  spi_transmit_byte(cmd);
  DL_GPIO_setPins(GPIO_CS_PORT, GPIO_CS_PIN_PIN);
}

uint8_t TM1638_INIT_ADDRESS = 0xC0;

void TM1638_write_segment(uint8_t pos, uint8_t segment_code) {
  DL_GPIO_clearPins(GPIO_CS_PORT, GPIO_CS_PIN_PIN);
  spi_transmit_byte(TM1638_INIT_ADDRESS + pos);
  spi_transmit_byte(segment_code);
  DL_GPIO_setPins(GPIO_CS_PORT, GPIO_CS_PIN_PIN);
}

void TM1638_reset(void) {
  spi_send_cmd(0x44);
  for (int i = 0; i < 16; i++) {
    TM1638_write_segment(i, 0x00);
  }
  spi_send_cmd(0x8F);
}

void TM1638_disp_num(uint32_t dis_num) {
  uint8_t digits[8] = {0};           // TM1638 支持最多 8 位数字显示
  static uint8_t data_set[16] = {0}; // 16 个显示寄存器
  // 提取每一位数字，从高位到低位存储
  for (int i = 0; i < 8; ++i) {
    digits[7 - i] = dis_num % 10;
    dis_num /= 10;
  }
  // 将数字转换为 TM1638 的显示数据格式
  for (int i = 0; i < 8; ++i) {
    data_set[i * 2] = DIGITS[digits[i]];
  }
  spi_send_cmd(0x44);
  for (int i = 0; i < 16; i++) {
    TM1638_write_segment(i, data_set[i]);
  }
  spi_send_cmd(0x8F); // 打开显示并设置亮度
}

void TM1638_disp_str(const char *disp_str) {
  uint8_t length = strlen(disp_str);
  uint8_t text_on_segments[length];
  uint8_t segments[16] = {0}; // 初始化为0

  // 将字符串转换为七段显示器编码
  stringToSegments(disp_str, text_on_segments, length);
  // 填充 segments 数组
  for (int i = 0; i < length && i < 8; i++) {
    // 最多填充8个字符
    segments[i * 2] = text_on_segments[i];
  }
  spi_send_cmd(0x44);
  for (int i = 0; i < 16; i++) {
    TM1638_write_segment(i, segments[i]);
  }
  spi_send_cmd(0x8F);
}


int main(void) {
  SYSCFG_DL_init();

  /* Set LED to indicate start of transfer */
  DL_GPIO_clearPins(GPIO_LEDS_PORT, GPIO_LEDS_USER_LED_1_PIN);

  DL_GPIO_setPins(GPIO_CS_PORT, GPIO_CS_PIN_PIN);
  TM1638_reset();
  delay_cycles(16000000);
  TM1638_disp_str("TESTHELLo");

  __BKPT();
  /* After all data is transmitted, toggle LED */
  while (1) {
    DL_GPIO_togglePins(GPIO_LEDS_PORT, GPIO_LEDS_USER_LED_1_PIN);
    delay_cycles(16000000);
  }
}
