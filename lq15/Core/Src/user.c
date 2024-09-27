#include "user.h"

extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim6;
extern UART_HandleTypeDef huart1;

float FreqA = 0;
float FreqB = 0;
u8 message[100];
uint32_t PD         = 1000;
uint32_t PH         = 5000;
float PX            = 0;
uint8_t currentPage = 0;
uint8_t NDA         = 0;
uint8_t NDB         = 0;
uint8_t NHA         = 0;
uint8_t NHB         = 0;
uint8_t paraChoose  = 0;
uint8_t showMode    = 0;

uint32_t HistoryA[12] = {0};
uint8_t HistoryIndexA = 0;
uint32_t HistoryB[12] = {0};
uint8_t HistoryIndexB = 0;

enum uint8_t {
    DATAPAGE = 0,
    PARAPAGE = 1,
    RECDPAGE = 2,
    CPD      = 0,
    CPH      = 1,
    CPX      = 2,
    FMODE    = 0,
    TMODE    = 1,
};

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if (htim == &htim2) {
        if (htim2.Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
            uint32_t ICValue = HAL_TIM_ReadCapturedValue(&htim2, TIM_CHANNEL_1);
            __HAL_TIM_SetCounter(&htim2, 0);
            FreqA = 80000000.0 / 800 / ICValue;
        }
    }

    if (htim == &htim3) {
        if (htim3.Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
            int32_t ICValue = HAL_TIM_ReadCapturedValue(&htim3, TIM_CHANNEL_1) + 2;
            __HAL_TIM_SetCounter(&htim3, 0);
            FreqB = 80000000.0 / 800 / ICValue;
        }
    }
}
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim == &htim6) {
        Fresh();

        HistoryA[HistoryIndexA] = FreqA + PX;
        HistoryIndexA++;
        if (HistoryIndexA >= 12) {
            HistoryIndexA = 0;
        }

        if (HistoryA[0] != 0 && HistoryA[11] != 0) {
            int32_t minHisA = HistoryA[0];
            int32_t maxHisA = 0;
            for (int i = 0; i < 12; i++) {
                if (HistoryA[i] < minHisA) {
                    minHisA = HistoryA[i];
                }
                if (HistoryA[i] > maxHisA) {
                    maxHisA = HistoryA[i];
                }
            }

            if (maxHisA - minHisA > PD) {
                NDA++;
                HistoryIndexA = 0;
                for (int8_t i = 0; i < 12; i++)
                    HistoryA[i] = 0;
                Fresh();
            }
        }

        HistoryB[HistoryIndexB] = FreqB + PX;
        HistoryIndexB++;
        if (HistoryIndexB >= 12) {
            HistoryIndexB = 0;
        }
        if (HistoryB[0] != 0 && HistoryB[11] != 0) {
            int32_t minHisB = HistoryB[0];
            int32_t maxHisB = 0;
            for (int i = 0; i < 12; i++) {
                if (HistoryB[i] < minHisB) {
                    minHisB = HistoryB[i];
                }
                if (HistoryB[i] > maxHisB) {
                    maxHisB = HistoryB[i];
                }
            }

            if (maxHisB - minHisB > PD) {
                NDB++;
                HistoryIndexB = 0;
                for (int8_t i = 0; i < 12; i++)
                    HistoryB[i] = 0;
                Fresh();
            }
        }

        HAL_GPIO_WritePin(LD4_GPIO_Port, LD4_Pin | LD5_Pin | LD6_Pin | LD7_Pin, GPIO_PIN_SET);
        if (currentPage == DATAPAGE) {
            HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_RESET);
        } else {
            HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_SET);
        }
        if (FreqA + PX > PH) {
            HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
        } else {
            HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
        }
        if (FreqB + PX > PH) {
            HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET);
        } else {
            HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET);
        }
        if (NDA >= 3 || NDB >= 3) {
            HAL_GPIO_WritePin(LD8_GPIO_Port, LD8_Pin, GPIO_PIN_RESET);
        } else {
            HAL_GPIO_WritePin(LD8_GPIO_Port, LD8_Pin, GPIO_PIN_SET);
        }
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_RESET);
    }
}

void ShowDataPage()
{
    LCD_ClearLine(Line6);
    LCD_ClearLine(Line7);
    LCD_DisplayStringLine(Line2, "        DATA");
    if (showMode == FMODE) {
        if (FreqA + PX < 0) {
            sprintf((char *)message, "     A=NULL          ");
            LCD_DisplayStringLine(Line4, message);
        } else if (FreqA + PX <= 1000) {
            sprintf((char *)message, "     A=%.0fHz          ", (FreqA + PX));
            LCD_DisplayStringLine(Line4, message);
        } else {
            sprintf((char *)message, "     A=%.2fKHz          ", (FreqA + PX) / 1000);
            LCD_DisplayStringLine(Line4, message);
        }

        if (FreqB + PX < 0) {
            sprintf((char *)message, "     B=NULL          ");
            LCD_DisplayStringLine(Line5, message);
        } else if (FreqB + PX <= 1000) {
            sprintf((char *)message, "     B=%.0fHz          ", (FreqB + PX));
            LCD_DisplayStringLine(Line5, message);
        } else {
            sprintf((char *)message, "     B=%.2fKHz          ", (FreqB + PX) / 1000);
            LCD_DisplayStringLine(Line5, message);
        }
    } else {
        if (FreqA + PX <= 0) {
            sprintf((char *)message, "     A=NULL          ");
            LCD_DisplayStringLine(Line4, message);
        } else if (FreqA + PX < 1000) {
            sprintf((char *)message, "     A=%.2fmS          ", 1000 / (FreqA + PX));
            LCD_DisplayStringLine(Line4, message);
        } else {
            sprintf((char *)message, "     A=%.0fuS          ", 1000000 / (FreqA + PX));
            LCD_DisplayStringLine(Line4, message);
        }

        if (FreqB + PX <= 0) {
            sprintf((char *)message, "     B=NULL          ");
            LCD_DisplayStringLine(Line5, message);
        } else if (FreqB + PX < 1000) {
            sprintf((char *)message, "     B=%.2fms          ", 1000 / (FreqB + PX));
            LCD_DisplayStringLine(Line5, message);
        } else {
            sprintf((char *)message, "     B=%.0fuS          ", 1000000 / (FreqB + PX));
            LCD_DisplayStringLine(Line5, message);
        }
    }
}
void ShowParaPage()
{
    LCD_ClearLine(Line7);
    LCD_DisplayStringLine(Line2, "        PARA");
    sprintf((char *)message, "     PD=%dHz          ", PD);
    LCD_DisplayStringLine(Line4, message);
    sprintf((char *)message, "     PH=%dHz          ", PH);
    LCD_DisplayStringLine(Line5, message);
    sprintf((char *)message, "     PX=%.0fHz          ", PX);
    LCD_DisplayStringLine(Line6, message);
}
void ShowRecdPage()
{
    LCD_DisplayStringLine(Line2, "        RECD");
    sprintf((char *)message, "     NDA=%d          ", NDA);
    LCD_DisplayStringLine(Line4, message);
    sprintf((char *)message, "     NDB=%d          ", NDB);
    LCD_DisplayStringLine(Line5, message);
    sprintf((char *)message, "     NHA=%d          ", NHA);
    LCD_DisplayStringLine(Line6, message);
    sprintf((char *)message, "     NHB=%d          ", NHB);
    LCD_DisplayStringLine(Line7, message);
}
void Fresh()
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
        default:
            break;
    }
}
void ScanKey()
{
    switch (currentPage) {
        case DATAPAGE:
            if (HAL_GPIO_ReadPin(B4_GPIO_Port, B4_Pin) == GPIO_PIN_RESET) {
                HAL_Delay(XDTIME1);
                if (HAL_GPIO_ReadPin(B4_GPIO_Port, B4_Pin) == GPIO_PIN_RESET) {
                    currentPage = PARAPAGE;
                    showMode    = FMODE;
                    Fresh();
                    HAL_Delay(XDTIME2);
                }
            } else if (HAL_GPIO_ReadPin(B3_GPIO_Port, B3_Pin) == GPIO_PIN_RESET) {
                HAL_Delay(XDTIME1);
                if (HAL_GPIO_ReadPin(B3_GPIO_Port, B3_Pin) == GPIO_PIN_RESET) {
                    if (showMode == FMODE) {
                        showMode = TMODE;
                    } else {
                        showMode = FMODE;
                    }
                    Fresh();
                    HAL_Delay(XDTIME2);
                }
            }
            break;
        case PARAPAGE:
            if (HAL_GPIO_ReadPin(B4_GPIO_Port, B4_Pin) == GPIO_PIN_RESET) {
                HAL_Delay(XDTIME1);
                if (HAL_GPIO_ReadPin(B4_GPIO_Port, B4_Pin) == GPIO_PIN_RESET) {
                    currentPage = RECDPAGE;
                    paraChoose  = CPD;
                    Fresh();
                    HAL_Delay(XDTIME2);
                }
            } else if (HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_RESET) {
                HAL_Delay(XDTIME1);
                if (HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_RESET) {
                    switch (paraChoose) {
                        case CPD:
                            if (PD < 1000) {
                                PD += 100;
                            }
                            break;
                        case CPH:
                            if (PH < 10000) {
                                PH += 100;
                            }
                            break;
                        case CPX:
                            if (PX < 1000) {
                                PX += 100;
                            }
                            break;
                    }
                    Fresh();
                    HAL_Delay(XDTIME2);
                }
            } else if (HAL_GPIO_ReadPin(B2_GPIO_Port, B2_Pin) == GPIO_PIN_RESET) {
                HAL_Delay(XDTIME1);
                if (HAL_GPIO_ReadPin(B2_GPIO_Port, B2_Pin) == GPIO_PIN_RESET) {
                    switch (paraChoose) {
                        case CPD:
                            if (PD > 100) {
                                PD -= 100;
                            }
                            break;
                        case CPH:
                            if (PH > 1000) {
                                PH -= 100;
                            }
                            break;
                        case CPX:
                            if (PX > -1000) {
                                PX -= 100;
                            }
                            break;
                    }
                    Fresh();
                    HAL_Delay(XDTIME2);
                }
            } else if (HAL_GPIO_ReadPin(B3_GPIO_Port, B3_Pin) == GPIO_PIN_RESET) {
                HAL_Delay(XDTIME1);
                if (HAL_GPIO_ReadPin(B3_GPIO_Port, B3_Pin) == GPIO_PIN_RESET) {
                    paraChoose++;
                    if (paraChoose > CPX)
                        paraChoose = CPD;
                    HAL_Delay(XDTIME2);
                }
            }
            break;
        case RECDPAGE:
            if (HAL_GPIO_ReadPin(B4_GPIO_Port, B4_Pin) == GPIO_PIN_RESET) {
                HAL_Delay(XDTIME1);
                if (HAL_GPIO_ReadPin(B4_GPIO_Port, B4_Pin) == GPIO_PIN_RESET) {
                    currentPage = 0;
                    Fresh();
                    HAL_Delay(XDTIME2);
                }
            } else if (HAL_GPIO_ReadPin(B3_GPIO_Port, B3_Pin) == GPIO_PIN_RESET) {
                HAL_Delay(XDTIME1);
                uint32_t sTime = HAL_GetTick();
                if (HAL_GPIO_ReadPin(B3_GPIO_Port, B3_Pin) == GPIO_PIN_RESET) {
                    while (HAL_GPIO_ReadPin(B3_GPIO_Port, B3_Pin) == GPIO_PIN_RESET) {
                        if (HAL_GetTick() - sTime >= 1000 + XDTIME1) {
                            NDA = 0;
                            NDB = 0;
                            NHA = 0;
                            NHB = 0;
                            break;
                        }
                    }
                    Fresh();
                    HAL_Delay(XDTIME2);
                }
            }
            break;
    }
}