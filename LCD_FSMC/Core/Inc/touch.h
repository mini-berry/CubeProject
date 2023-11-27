#ifndef __TOUCH_H__
#define __TOUCH_H__
#include "stm32f1xx_hal.h"
#include "delay.h"

// 滤波参数
#define READ_TIMES 5
#define LOST_VAL 1
// 软SPI定义

#define T_IN_GPIO GPIOF
#define T_OUT_GPIO GPIOB
#define T_SCK_GPIO GPIOB
#define T_CS_GPIO GPIOF

#define T_IN_PIN GPIO_PIN_9
#define T_OUT_PIN GPIO_PIN_2
#define T_SCK_PIN GPIO_PIN_1
#define T_CS_PIN GPIO_PIN_11

uint16_t Touch_Read_XOrY(uint8_t cmd);
uint8_t Touch_Read(uint16_t *x, uint16_t *y);
uint16_t Touch_Read_SPI(uint8_t CMD);
void Touch_Write_Byte(uint8_t num);

#endif
