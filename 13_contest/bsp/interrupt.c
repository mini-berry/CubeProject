#include "interrupt.h"
#include "usart.h"
#include "main.h"

extern unsigned char incorrect_ref;
struct keys key[5] = {0, 0, 0, 0, 0};

uint16_t led_tt  = 0;
uint16_t led_ref = 0;
uint8_t led_5    = 0;

void LED_real(unsigned char led_bate)
{
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_All, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOC, led_bate << 8, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_RESET);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM4) {
        if (++led_tt == 20) led_tt = 0;
        if (++led_ref == 500) {
            led_ref = 0;
            led_5   = 1;
        }
        key[0].key_str = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0);
        key[1].key_str = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1);
        key[2].key_str = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2);
        key[3].key_str = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);
        for (int i = 0; i < 4; i++) {
            switch (key[i].judge_sta) {
                case 0: {
                    if (key[i].key_str == 0) {
                        key[i].judge_sta = 1;
                        key[i].key_time  = 0;
                    }
                } break;
                case 1: {
                    if (key[i].key_str == 0) {
                        key[i].judge_sta = 2;
                    } else {
                        key[i].judge_sta = 0;
                    }
                } break;
                case 2: {
                    if (key[i].key_str == 1) {
                        if (key[i].key_time < 70) {
                            key[i].sing_flag = 1;
                        }
                        key[i].judge_sta = 0;
                    } else {
                        if (key[i].key_time >= 70) {
                            key[i].long_flag = 1;
                            if (key[i].key_time > 70) key[i].key_time = 70;
                        }
                        key[i].key_time++;
                    }
                } break;
            }
        }
    }
}

char re_date[30];
unsigned char rx_str;
uint8_t rx_date;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    HAL_UART_Receive_IT(&huart1, &rx_date, 1);
    re_date[rx_str++] = rx_date;
}
