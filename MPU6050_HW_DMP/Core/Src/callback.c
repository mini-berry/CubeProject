#include "callback.h"

extern TIM_HandleTypeDef htim6;
extern pose_msg_t mpu_pose_msg; // 姿态数据
extern accel_msg_t mpu_accel_msg;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim == &htim6) {
        // 1000Hz读取
        read_dmp(&mpu_pose_msg, &mpu_accel_msg);
    }
}