// DrivLib_TM1638.h
#ifndef DRIVLIB_TM1638_H
#define DRIVLIB_TM1638_H

#include <stdint.h>

// 初始化 TM1638
void TM1638_init(void);

// 设置 TM1638 显示
void TM1638_setDisplay(uint8_t position, uint8_t value);

// 其他 TM1638 相关函数声明

#endif // DRIVLIB_TM1638_H