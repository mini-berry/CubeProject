#ifndef _CALLBACK_H_
#define _CALLBACK_H_

#include "main.h"
#include "mpu6050.h"
#include "Kalman.h"

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);

#endif