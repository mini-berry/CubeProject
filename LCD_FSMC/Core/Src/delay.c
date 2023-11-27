#include "delay.h"

// 输入值需要小于1000，大于1000请使用HAL_Delay
void delay_us(uint32_t udelay)
{
    uint32_t wait;

    uint32_t startval = SysTick->VAL;
    uint32_t tickn = HAL_GetTick();
    uint32_t delays = udelay * 72;
    if (delays > startval)
    {
        wait = 72000 + startval - delays;
        while (HAL_GetTick() == tickn)
        {
        }
        while (wait < SysTick->VAL)
        {
        }
    }
    else
    {
        wait = startval - delays;
        while (wait < SysTick->VAL && HAL_GetTick() == tickn)
        {
        }
    }
}
// void delay_us(uint32_t t)
// {
//     int32_t end = SysTick->VAL - t * 72;
//     uint32_t tick = HAL_GetTick();
//     if (end < 0)
//     {
//         end = 72000 + end;
//         while (tick == HAL_GetTick())
//             ;
//         while (SysTick->VAL > end)
//             ;
//     }
//     else
//     {
//         while (SysTick->VAL > end && HAL_GetTick() == tick)
//             ;
//     }
// }