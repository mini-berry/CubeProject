#include "music.h"

int8_t BirthdayMusicsheet[28] =
    {-5, -5, -6, -5, 1, -7, 0,
     -5, -5, -6, -5, 2, 1, 0,
     -5, -5, 5, 3, 1, -7, -6,
     4, 4, 3, 1, 2, 1, 0};
music BirthdayMusic =
    {
        .length = 28,
        .beep = 300,
        .sheet = BirthdayMusicsheet};

// 用例PlayMusic(BirthdayMusic, htim1, TIM_CHANNEL_1, GPIOA, GPIO_PIN_9);
// 输入简谱，定时器，VCC端口
// 使用前需要修改所选定时器psc，使频率为1MHz
void PlayMusic(music sheet, TIM_HandleTypeDef htim, unsigned int TIM_CHANNEL, GPIO_TypeDef *GPIO, uint16_t GPIO_PIN)
{
    HAL_GPIO_WritePin(GPIO, GPIO_PIN, GPIO_PIN_RESET);
    uint16_t delaytime = sheet.beep;
    for (int32_t i = 0; i < sheet.length; i++)
    {
        __HAL_TIM_SET_COUNTER(&htim, 0);
        switch (*(BirthdayMusicsheet + i))
        {
        case 6:
            htim.Instance->ARR = 1136;
            __HAL_TIM_SetCompare(&htim, TIM_CHANNEL, 568);
            break;
        case 5:
            htim.Instance->ARR = 1277;
            __HAL_TIM_SetCompare(&htim, TIM_CHANNEL, 638);
            break;
        case 4:
            htim.Instance->ARR = 1432;
            __HAL_TIM_SetCompare(&htim, TIM_CHANNEL, 716);
            break;
        case 3:
            htim.Instance->ARR = 1517;
            __HAL_TIM_SetCompare(&htim, TIM_CHANNEL, 758);
            break;
        case 2:
            htim.Instance->ARR = 1703;
            __HAL_TIM_SetCompare(&htim, TIM_CHANNEL, 851);
            break;
        case 1:
            htim.Instance->ARR = 1912;
            __HAL_TIM_SetCompare(&htim, TIM_CHANNEL, 956);
            break;
        case -7:
            htim.Instance->ARR = 2028;
            __HAL_TIM_SetCompare(&htim, TIM_CHANNEL, 1014);
            break;
        case -6:
            htim.Instance->ARR = 2272;
            __HAL_TIM_SetCompare(&htim, TIM_CHANNEL, 1136);
            break;
        case -5:
            htim.Instance->ARR = 2551;
            __HAL_TIM_SetCompare(&htim, TIM_CHANNEL, 1275);
            break;
        case -4:
            htim.Instance->ARR = 2865;
            __HAL_TIM_SetCompare(&htim, TIM_CHANNEL, 1432);
            break;
        case -3:
            htim.Instance->ARR = 3039;
            __HAL_TIM_SetCompare(&htim, TIM_CHANNEL, 1519);
            break;
        case -2:
            htim.Instance->ARR = 3412;
            __HAL_TIM_SetCompare(&htim, TIM_CHANNEL, 1706);
            break;
        case -1:
            htim.Instance->ARR = 3831;
            __HAL_TIM_SetCompare(&htim, TIM_CHANNEL, 1915);
            break;
        case 0:
            __HAL_TIM_SetCompare(&htim, TIM_CHANNEL, 0);
            HAL_Delay(delaytime);
        default:
            break;
        }
        HAL_Delay(delaytime);
    };
    HAL_GPIO_WritePin(GPIO, GPIO_PIN, GPIO_PIN_SET);
}
