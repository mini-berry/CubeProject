#ifndef _USERGENERAL_H_
#define _USERGENERAL_H_

#include "stm32f1xx_hal.h"
#include "string.h"
#include "stdio.h"
#include "stdarg.h"

// 启用USB串口输出
#if 0
#define USE_USB
#include "usbd_cdc_if.h"
#endif

// 启用串口输出
#if 1
#ifdef HAL_UART_MODULE_ENABLED
#define USE_UART huart1
#else
#ifdef HAL_USART_MODULE_ENABLED
#define USE_UART huart1
#endif
#endif
#endif

#define MESSAGE_LEN 100



#ifdef USE_USB
void userUSB_Restart(void);
#endif

void userPrint(const char *format, ...);
void userPrintStatus(HAL_StatusTypeDef Status);
void userPrintAddr(char *Addr);

#endif