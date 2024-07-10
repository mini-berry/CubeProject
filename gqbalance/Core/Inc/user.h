#ifndef __USER_H
#define __USER_H
#define USE_UART huart2
#include "main.h"
#include "adc.h"
#include "usart.h"
#include "string.h"
#include "stdio.h"
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc);
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
void ServoSetAngle(float angle);
void ServoControl(float output);
#endif
