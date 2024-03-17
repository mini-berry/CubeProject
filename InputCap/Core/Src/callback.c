#include "callback.h"
extern TIM_HandleTypeDef htim4;
extern uint32_t countRising;
extern uint32_t countFalling;
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if (htim == &htim4) {
        if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
            countRising=HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
            countFalling=HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
            __HAL_TIM_SetCounter(htim,0);
            HAL_TIM_IC_Start_IT(htim, TIM_CHANNEL_1 | TIM_CHANNEL_2);
        }
    }
}