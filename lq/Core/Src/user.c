#include "user.h"

extern ADC_HandleTypeDef hadc2;

extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim6;

extern UART_HandleTypeDef huart1;

uint8_t currentPage  = 0;
uint8_t R            = 1;
uint8_t K            = 1;
uint8_t kR           = 1;
uint8_t kK           = 1;
uint8_t pwmMode      = 0;
uint32_t switchTimes = 0;
uint32_t pwmTimes    = 0;
uint8_t switchAble   = 1;
uint8_t swithStatus  = 0;
uint8_t lockStatus   = 0;
uint8_t rork         = 0;
uint32_t Speed;
uint32_t maxHigh = 0;
uint32_t maxLow  = 0;

float pulse = 0;

extern UART_HandleTypeDef huart1;
char message[100];
enum uint8_t {
    DATAPAGE = 0,
    PARAPAGE = 1,
    RECDPAGE = 2,
    PWMHIGH  = 0,
    PWMLOW   = 1,
    ABLE     = 1,
    DABLE    = 0,
    NSWITCH  = 0,
    TOHIGH   = 1,
    TOLOW    = 2,
    ISR      = 0,
    ISK      = 1,
};

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM3) {
        if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2) {
            uint32_t Rising = HAL_TIM_ReadCapturedValue(&htim3, TIM_CHANNEL_2) + 2;
            __HAL_TIM_SetCounter(&htim3, 0);
            float Freq = 80000000 / 80 / (float)Rising;
            Speed      = Freq * 2 * 3.14 * R / 100 / K * 10;
        }
        HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_2);
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim == &htim6) {
        static uint32_t oldSpeed = 0;
        static uint8_t sameTimes = 0;
        if (switchAble == DABLE)
            sameTimes = 0;
        if (Speed == oldSpeed) {
            sameTimes++;
            if (sameTimes >= 200) {
                if (pwmMode == PWMHIGH) {
                    if (maxHigh < Speed) {
                        maxHigh = Speed;
                        if (currentPage == RECDPAGE) {
                            sprintf(message, "     MH=%.1f", maxHigh / 10.0);
                            LCD_DisplayStringLine(Line6, (u8 *)message);
                        }
                    }
                }
                if (pwmMode == PWMLOW) {
                    if (maxLow < Speed) {
                        maxLow = Speed;
                        if (currentPage == RECDPAGE) {
                            sprintf(message, "     ML=%.1f", maxLow / 10.0);
                            LCD_DisplayStringLine(Line5, (u8 *)message);
                        }
                    }
                }
                sameTimes = 0;
            }
        } else {
            oldSpeed  = Speed;
            sameTimes = 0;
        }
        static uint32_t aTime = 0;
        if (lockStatus == 0) {
            HAL_ADC_Start(&hadc2);
            uint32_t adcValue = HAL_ADC_GetValue(&hadc2);
            float Volt        = adcValue / 4095.0 * 3.3;
            if (Volt > 3)
                pulse = 0.85;
            else if (Volt < 1)
                pulse = 0.1;
            else
                pulse = (0.85 - 0.1) / (3 - 1) * (Volt - 1) + 0.1;
            __HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_2, htim2.Instance->ARR * pulse);
            aTime++;
            if (aTime > 9) {
                if (currentPage == DATAPAGE) {
                    sprintf(message, "     P=%.0f%%           ", 100 * pulse);
                    LCD_DisplayStringLine(Line5, (u8 *)message);
                }
                aTime = 0;
            }
        }
        static uint32_t bTime = 0;
        bTime++;
        if (bTime > 9) {
            if (currentPage == DATAPAGE) {
                sprintf(message, "     V=%.1f          ", Speed / 10.0);
                LCD_DisplayStringLine(Line6, (u8 *)message);
            }
            bTime = 0;
        }
        HAL_GPIO_WritePin(GPIOC, LED4_Pin | LED5_Pin | LED6_Pin | LED7_Pin | LED8_Pin, GPIO_PIN_SET);
        if (currentPage == DATAPAGE)
            HAL_GPIO_WritePin(GPIOC, LED1_Pin, GPIO_PIN_RESET);
        else {
            HAL_GPIO_WritePin(GPIOC, LED1_Pin, GPIO_PIN_SET);
        }
        if (switchTimes >= 500) {
            pwmTimes++;
            if (currentPage == RECDPAGE) {
                sprintf(message, "     N=%d          ", pwmTimes);
                LCD_DisplayStringLine(Line4, (u8 *)message);
            }
            if (swithStatus == TOHIGH) {
                pwmMode = PWMHIGH;
                __HAL_TIM_SetAutoreload(&htim2, 999);
                __HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_2, 999 * pulse);
                if (currentPage == DATAPAGE) {
                    LCD_DisplayStringLine(Line4, (u8 *)"     M=H          ");
                }
            } else if (swithStatus == TOLOW) {
                pwmMode = PWMLOW;
                __HAL_TIM_SetAutoreload(&htim2, 1999);
                __HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_2, 1999 * pulse);
                if (currentPage == DATAPAGE) {
                    LCD_DisplayStringLine(Line4, (u8 *)"     M=L          ");
                }
            }
            swithStatus = NSWITCH;
            switchAble  = ABLE;
            switchTimes = 0;
        }
        if (swithStatus == TOHIGH) {
            __HAL_TIM_SetAutoreload(&htim2, htim2.Instance->ARR - 2);
            __HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_2, htim2.Instance->ARR * pulse);
            switchTimes++;
            if (switchTimes % 10 == 0)
                HAL_GPIO_TogglePin(GPIOC, LED2_Pin);
        } else if (swithStatus == TOLOW) {
            __HAL_TIM_SetAutoreload(&htim2, htim2.Instance->ARR + 2);
            __HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_2, htim2.Instance->ARR * pulse);
            switchTimes++;
            if (switchTimes % 10 == 0)
                HAL_GPIO_TogglePin(GPIOC, LED2_Pin);
        } else if (swithStatus == NSWITCH) {
            HAL_GPIO_WritePin(GPIOC, LED2_Pin, GPIO_PIN_SET);
        }
        if (lockStatus == 1)
            HAL_GPIO_WritePin(GPIOC, LED3_Pin, GPIO_PIN_RESET);
        else
            HAL_GPIO_WritePin(GPIOC, LED3_Pin, GPIO_PIN_SET);

        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_RESET);
    }
}
void ShowDataPage()
{
    LCD_Clear(Black);
    LCD_DisplayStringLine(Line2, (u8 *)"        DATA");
    if (pwmMode == PWMLOW)
        LCD_DisplayStringLine(Line4, (u8 *)"     M=L");
    else if (pwmMode == PWMHIGH)
        LCD_DisplayStringLine(Line4, (u8 *)"     M=H");
    sprintf(message, "     P=%.0f%%", 100 * pulse);
    LCD_DisplayStringLine(Line5, (u8 *)message);
    sprintf(message, "     V=%.1f", Speed / 10.0);
    LCD_DisplayStringLine(Line6, (u8 *)message);
}

void ShowParaPage()
{
    rork = ISR;
    LCD_Clear(Black);
    LCD_DisplayStringLine(Line2, (u8 *)"        PARA");
    sprintf(message, "     R=%d", R);
    LCD_DisplayStringLine(Line4, (u8 *)message);
    sprintf(message, "     K=%d", K);
    LCD_DisplayStringLine(Line5, (u8 *)message);
}

void ShowRecdPage()
{
    R = kR;
    K = kK;
    LCD_Clear(Black);
    LCD_DisplayStringLine(Line2, (u8 *)"        RECD");
    sprintf(message, "     N=%d", pwmTimes);
    LCD_DisplayStringLine(Line4, (u8 *)message);
    sprintf(message, "     ML=%.1f", maxLow / 10.0);
    LCD_DisplayStringLine(Line5, (u8 *)message);
    sprintf(message, "     MH=%.1f", maxHigh / 10.0);
    LCD_DisplayStringLine(Line6, (u8 *)message);
}

void FreshPage()
{
    switch (currentPage) {
        case DATAPAGE:
            ShowDataPage();
            break;
        case PARAPAGE:
            ShowParaPage();
            break;
        case RECDPAGE:
            ShowRecdPage();
            break;
    }
}

void ScanKey()
{
    if (HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_RESET) {
        HAL_Delay(XDTIME);
        if (HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_RESET) {
            currentPage++;
            if (currentPage > 2)
                currentPage = 0;
            FreshPage();
        }
    } else if (currentPage == DATAPAGE) {
        if (switchAble == ABLE) {
            if (HAL_GPIO_ReadPin(B2_GPIO_Port, B2_Pin) == GPIO_PIN_RESET) {
                HAL_Delay(XDTIME);
                if (HAL_GPIO_ReadPin(B2_GPIO_Port, B2_Pin) == GPIO_PIN_RESET) {
                    if (pwmMode == PWMLOW)
                        swithStatus = TOHIGH;
                    else if (pwmMode == PWMHIGH)
                        swithStatus = TOLOW;
                    switchAble = DABLE;
                }
            }
        }
        if (lockStatus == 0) {
            if (HAL_GPIO_ReadPin(B4_GPIO_Port, B4_Pin) == GPIO_PIN_RESET) {
                uint32_t sTime = HAL_GetTick();
                HAL_Delay(XDTIME);
                while (HAL_GPIO_ReadPin(B4_GPIO_Port, B4_Pin) == GPIO_PIN_RESET) {
                    if (HAL_GetTick() - sTime > 2000) {
                        lockStatus = 1;
                    }
                }
            }
        } else if (lockStatus == 1) {
            if (HAL_GPIO_ReadPin(B4_GPIO_Port, B4_Pin) == GPIO_PIN_RESET) {
                HAL_Delay(XDTIME);
                if (HAL_GPIO_ReadPin(B4_GPIO_Port, B4_Pin) == GPIO_PIN_RESET) {
                    lockStatus = 0;
                }
            }
        }
    } else if (currentPage == PARAPAGE) {
        if (HAL_GPIO_ReadPin(B2_GPIO_Port, B2_Pin) == GPIO_PIN_RESET) {
            HAL_Delay(XDTIME);
            if (HAL_GPIO_ReadPin(B2_GPIO_Port, B2_Pin) == GPIO_PIN_RESET) {
                if (rork == ISR)
                    rork = ISK;
                else if (rork == ISK)
                    rork = ISR;
            }
        }
        if (HAL_GPIO_ReadPin(B3_GPIO_Port, B3_Pin) == GPIO_PIN_RESET) {
            HAL_Delay(XDTIME);
            if (HAL_GPIO_ReadPin(B3_GPIO_Port, B3_Pin) == GPIO_PIN_RESET) {
                if (rork == ISR) {
                    kR++;
                    if (kR > 10)
                        kR = 1;
                    sprintf(message, "     R=%d           ", kR);
                    LCD_DisplayStringLine(Line4, (u8 *)message);
                }
                if (rork == ISK) {
                    kK++;
                    if (kK > 10)
                        kK = 1;
                    sprintf(message, "     K=%d          ", kK);
                    LCD_DisplayStringLine(Line5, (u8 *)message);
                }
            }
        }
        if (HAL_GPIO_ReadPin(B4_GPIO_Port, B4_Pin) == GPIO_PIN_RESET) {
            HAL_Delay(XDTIME);
            if (HAL_GPIO_ReadPin(B4_GPIO_Port, B4_Pin) == GPIO_PIN_RESET) {
                if (rork == ISR) {
                    if (kR == 1)
                        kR = 10;
                    else
                        kR--;;
                    sprintf(message, "     R=%d          ", kR);
                    LCD_DisplayStringLine(Line4, (u8 *)message);
                }

                if (rork == ISK) {
                    if (kK == 1)
                        kK = 10;
                    else
                        kK--;
                    sprintf(message, "     K=%d          ", kK);
                    LCD_DisplayStringLine(Line5, (u8 *)message);
                }
            }
        }
    }
    HAL_Delay(10);
}