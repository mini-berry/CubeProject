#ifndef USER_H
#define USER_H
#include "main.h"
#include "lcd_hal.h"
#include "stdio.h"
#include "string.h"
#include "math.h"
#define XDTIME 50
void ScanKey();
void FreshPage();
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim);
#endif