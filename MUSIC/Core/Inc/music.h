#ifndef _MUSIC_H_
#define _MUSIC_H_

#include "stm32f1xx_hal.h"

typedef struct
{
    uint32_t length;
    uint16_t beep;
    int8_t *sheet;
} music;

void PlayMusic(music sheet, TIM_HandleTypeDef htim, unsigned int TIM_CHANNEL, GPIO_TypeDef * GPIO, uint16_t GPIO_PIN);

#endif