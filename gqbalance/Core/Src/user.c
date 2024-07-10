#include "user.h"
extern char rx_data[10];
extern float distance;
extern float targetDis;
extern float lastAngle;
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
    if (output > lastAngle)
        ServoSetAngle(lastAngle);
}

void ServoSetAngle(float angle)
{
    // 0.5ms-2.5ms
    // 周期设为5ms，频率200Hz
    // 分频72，计数器自动重装载值为5000(1000000/200=5000)
    // 0.5ms对应的计数器值为5000*0.5/20=125
    // 2.5ms对应的计数器值为5000*2.5/20=625
    // 1.5ms对应的计数器值为5000*1.5/20=375
    // 每度对应的计数器值为(625-125)/180=2
    // angle=-90时，计数器值为375,angle=90时，计数器值为625
    __HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_1, 375 + angle * 250 / 90);
}