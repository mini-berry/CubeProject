#ifndef __INTERRUPT_H
#define __INTERRUPT_H

#include "main.h"
#include "stdbool.h"

struct keys{
bool sing_flag;
bool long_flag;
uint8_t judge_sta;
uint8_t key_str;
uint16_t key_time;
};

void LED_real(unsigned char led_bate);
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);

#endif 
