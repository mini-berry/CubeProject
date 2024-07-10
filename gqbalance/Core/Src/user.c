#include "user.h"
extern char rx_data[10];
extern float distance;
extern float targetDis;
extern uint32_t sTime;
uint32_t sTime2;

char message[100] = {0};
int8_t receiveLen = 0;

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc == &hadc1) {
        distance = (float)HAL_ADC_GetValue(&hadc1) / 4095 * 3.3;
        if (HAL_GetTick() - sTime2 > 500) {
            sTime2 = HAL_GetTick();
            sprintf(message, "adc:%.2f\n", distance);
            HAL_UART_Transmit(&huart1, (uint8_t *)message, strlen(message), 50);
        }
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == &USE_UART) {
        if (*(rx_data + receiveLen) == 'U') {
            receiveLen = 1;
            rx_data[0] = 'U';
        } else if (rx_data[0] == 'U') {
            receiveLen++;
            if (receiveLen == 3) {
                receiveLen = 0;
                targetDis  = (rx_data[1] - '0') * 10 + (rx_data[2] - '0');
                sprintf(message, "Target distance: %d cm\n", (int)targetDis);
                HAL_UART_Transmit(&huart1, (uint8_t *)message, strlen(message), 50);
                sTime = HAL_GetTick();
            }
        } else {
            receiveLen = 0;
        }
        HAL_UART_Receive_IT(&USE_UART, (uint8_t *)rx_data + receiveLen, 1);
    }
}

void ServoControl(float output)
{
    if (output > 90)
        output = 90;
    else if (output < -90)
        output = -90;
    __HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_1, 124 + output * 125 / 90);
}