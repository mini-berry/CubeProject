#include "userGeneral.h"

char message[MESSAGE_LEN];
#ifdef USE_UART
extern UART_HandleTypeDef USE_UART;
#endif

void userUSB_Restart(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_Initure;

    GPIO_Initure.Pin   = GPIO_PIN_11 | GPIO_PIN_12;
    GPIO_Initure.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_Initure.Pull  = GPIO_PULLDOWN;
    GPIO_Initure.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_Initure);

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11 | GPIO_PIN_12, GPIO_PIN_RESET);
}

void userPrint(const char *format, ...)
{
    if (strlen(format) > MESSAGE_LEN) {
#ifdef USE_UART
        HAL_UART_Transmit(&USE_UART, (uint8_t *)"String length is not enough.", strlen("String length is not enough."), 50);
#endif

#ifdef USE_USB
        CDC_Transmit_FS((uint8_t *)"String length is not enough.", strlen("String length is not enough."));
#endif
        return;
    }
    // 存储传入参数
    va_list args;
    // 传入参数形成char数组
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);

#ifdef USE_UART
    HAL_UART_Transmit(&USE_UART, (uint8_t *)message, strlen(message), 50);
#endif

#ifdef USE_USB
    CDC_Transmit_FS((uint8_t *)message, strlen(message));
#endif

    va_end(args);
}
void userPrintStatus(HAL_StatusTypeDef Status)
{
    switch (Status) {
        case HAL_OK:
            userPrint("HAL_OK\n");
            break;
        case HAL_ERROR:
            userPrint("HAL_ERROR\n");
            break;
        case HAL_BUSY:
            userPrint("HAL_BUSY\n");
            break;
        case HAL_TIMEOUT:
            userPrint("HAL_TIMEOUT\n");
            break;
        default:
            break;
    }
}