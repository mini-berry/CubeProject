#ifndef USER_H
#define USER_H

#include "main.h"
#include "lcd.h"
#include "stdio.h"
#include "string.h"

#define XDTIME 15
#define XDTIME2 100
void Fresh();
void ScanKey();
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim);
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);

#endif // !USER_H