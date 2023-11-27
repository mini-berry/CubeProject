#include "callback.h"

extern TIM_HandleTypeDef htim6;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim == &htim6) {
        MPU6050_Read_Accel();
        // KalmanFilter(&hkfpX, RawData.Accel_X);
        // KalmanFilter(&hkfpY, RawData.Accel_Y);
        // KalmanFilter(&hkfpZ, RawData.Accel_Z);
    }
}