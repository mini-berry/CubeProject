#include "user.h"
extern char rx_data[10];
extern float distance;
extern float targetDis;
extern uint32_t sTime;

int8_t receiveLen = 0;

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc == &hadc1) {
        distance = (float)HAL_ADC_GetValue(&hadc1) / 4095 * 3.3;
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == &huart1) {
        if (*(rx_data + receiveLen) == 'U') {
            receiveLen = 1;
        } else if (rx_data[0] == 'U') {
            receiveLen++;
            if (receiveLen == 3) {
                rx_data[0] = 0;
                receiveLen = 0;
                targetDis  = (rx_data[1] - '0') * 10 + (rx_data[2] - '0');
                sTime      = HAL_GetTick();
            }
        } else {
            receiveLen = 0;
        }
        HAL_UART_Receive_IT(&huart1, (uint8_t *)rx_data + receiveLen, 1);
    }
}