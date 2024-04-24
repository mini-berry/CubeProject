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
uint8_t AHRec       = 0;
uint8_t BHRec       = 0;
uint8_t ADRec       = 1;
uint8_t BDRec       = 1;

float AData[300];
float BData[300];

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
    static uint8_t At   = 0;
    static uint8_t Bt   = 0;
    static float FreqAT = 0;
    static float FreqBT = 0;
    if (htim == &htim2) {
        At++;
        if (htim2.Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
            if (At > 39) {
                uint32_t ICValue = HAL_TIM_ReadCapturedValue(&htim2, TIM_CHANNEL_1) + 2;
                __HAL_TIM_SetCounter(&htim2, 0);
                FreqAT += ICValue;
                FreqA  = 80000000 / 800 / (float)FreqAT * At;
                At     = 0;
                FreqAT = 0;
            }
        }
    }
    if (htim == &htim3) {
        if (htim3.Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
            Bt++;
            if (Bt > 39) {
                uint32_t ICValue = HAL_TIM_ReadCapturedValue(&htim3, TIM_CHANNEL_1) + 2;
                __HAL_TIM_SetCounter(&htim3, 0);
                FreqBT += ICValue;
                FreqB  = 80000000 / 800 / (float)FreqBT * Bt;
                Bt     = 0;
                FreqBT = 0;
            }
        }
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim == &htim6) {
        {
            static uint16_t pData;
            static uint32_t bTime = 0;
            AData[pData]          = FreqA + PX;
            BData[pData]          = FreqB + PX;
            pData++;
            if (pData > 299) pData = 0;

            if (HAL_GetTick() - bTime > 3000) {
                float maxA = 0;
                float maxB = 0;
                float minA = FreqA + PX;
                float minB = FreqB + PX;

                for (uint16_t i = 0; i < 300; i++) {
                    if (AData[i] > maxA) {
                        maxA = AData[i];
                    }
                    if (BData[i] > maxB) {
                        maxB = BData[i];
                    }
                    if (AData[i] < minA) {
                        minA = AData[i];
                    }
                    if (BData[i] < minB) {
                        maxB = BData[i];
                    }
                }
                if (maxA - minA > PD) {
                    NDA++;
                    if (currentPage == RECDPAGE) {
                        sprintf((char *)message, "     NDA=%d          ", NDA);
                        LCD_DisplayStringLine(Line4, message);
                    }
                }
                if (maxB - minB > PD) {
                    NDB++;
                    if (currentPage == RECDPAGE) {
                        sprintf((char *)message, "     NDB=%d          ", NDB);
                        LCD_DisplayStringLine(Line5, message);
                    }
                }
                bTime = HAL_GetTick();
            }
        }
        {
            // 0代表不超，1代表超
            if (AHRec == 0 && FreqA + PX > PH) {
                AHRec = 1;
                NHA++;
                if (currentPage == RECDPAGE) {
                    sprintf((char *)message, "     NHA=%d          ", NHA);
                    LCD_DisplayStringLine(Line6, message);
                }
            } else if (AHRec == 1 && FreqA + PX < PH) {
                AHRec = 0;
            }

            if (BHRec == 0 && FreqB + PX > PH) {
                BHRec = 1;
                NHB++;
                if (currentPage == RECDPAGE) {
                    sprintf((char *)message, "     NHB=%d          ", NHB);
                    LCD_DisplayStringLine(Line7, message);
                }
            } else if (BHRec == 1 && FreqB + PX < PH) {
                BHRec = 0;
            }
            static uint32_t aTime = 0;
            if (HAL_GetTick() - aTime > 100) {
                aTime = HAL_GetTick();
                if (currentPage == DATAPAGE && showMode == FMODE) {
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
                    LCD_ClearLine(Line6);
                    LCD_ClearLine(Line7);
                } else if (currentPage == DATAPAGE && showMode == TMODE) {
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
                        sprintf((char *)message, "     B=NULL");
                        LCD_DisplayStringLine(Line5, message);
                    } else if (FreqB + PX < 1000) {
                        sprintf((char *)message, "     B=%.2fmS          ", 1000 / (FreqB + PX));
                        LCD_DisplayStringLine(Line5, message);
                    } else {
                        sprintf((char *)message, "     B=%.0fuS          ", 1000000 / (FreqB + PX));
                        LCD_DisplayStringLine(Line5, message);
                    }
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
}
void ShowDataPage()
{
    LCD_Clear(Black);
    LCD_DisplayStringLine(Line2, "        DATA");
    if (FreqA + PX < 0) {
        sprintf((char *)message, "     A=NULL");
        LCD_DisplayStringLine(Line4, message);
    } else if (FreqA + PX <= 1000) {
        sprintf((char *)message, "     A=%.0fHz", (FreqA + PX));
        LCD_DisplayStringLine(Line4, message);
    } else {
        sprintf((char *)message, "     A=%.2fKHz", (FreqA + PX) / 1000);
        LCD_DisplayStringLine(Line4, message);
    }

    if (FreqB + PX < 0) {
        sprintf((char *)message, "     B=NULL");
        LCD_DisplayStringLine(Line5, message);
    } else if (FreqB + PX <= 1000) {
        sprintf((char *)message, "     B=%.0fHz", (FreqB + PX));
        LCD_DisplayStringLine(Line5, message);
    } else {
        sprintf((char *)message, "     B=%.2fKHz", (FreqB + PX) / 1000);
        LCD_DisplayStringLine(Line5, message);
    }
}
void ShowParaPage()
{
    LCD_Clear(Black);
    LCD_DisplayStringLine(Line2, "        PARA");
    sprintf((char *)message, "     PD=%dHz", PD);
    LCD_DisplayStringLine(Line4, message);
    sprintf((char *)message, "     PH=%dHz", PH);
    LCD_DisplayStringLine(Line5, message);
    sprintf((char *)message, "     PX=%.0fHz", PX);
    LCD_DisplayStringLine(Line6, message);
}
void ShowRecdPage()
{
    LCD_Clear(Black);
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
    if (currentPage == DATAPAGE) {
        if (HAL_GPIO_ReadPin(B4_GPIO_Port, B4_Pin) == GPIO_PIN_RESET) {
            HAL_Delay(XDTIME);
            if (HAL_GPIO_ReadPin(B4_GPIO_Port, B4_Pin) == GPIO_PIN_RESET) {
                currentPage++;
                showMode = FMODE;
                Fresh();
                HAL_Delay(XDTIME2);
            }
        } else if (HAL_GPIO_ReadPin(B3_GPIO_Port, B3_Pin) == GPIO_PIN_RESET) {
            HAL_Delay(XDTIME);
            if (HAL_GPIO_ReadPin(B3_GPIO_Port, B3_Pin) == GPIO_PIN_RESET) {
                if (showMode == FMODE) {
                    showMode = TMODE;
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
                        sprintf((char *)message, "     B=NULL");
                        LCD_DisplayStringLine(Line5, message);
                    } else if (FreqB + PX < 1000) {
                        sprintf((char *)message, "     B=%.2fms          ", 1000 / (FreqB + PX));
                        LCD_DisplayStringLine(Line5, message);
                    } else {
                        sprintf((char *)message, "     B=%.0fuS          ", 1000000 / (FreqB + PX));
                        LCD_DisplayStringLine(Line5, message);
                    }
                } else {
                    showMode = FMODE;
                    if (FreqA + PX < 0) {
                        sprintf((char *)message, "     A=NULL          ");
                        LCD_DisplayStringLine(Line5, message);
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
                }
                HAL_Delay(XDTIME2);
            }
        }
    } else if (currentPage == PARAPAGE) {
        if (HAL_GPIO_ReadPin(B4_GPIO_Port, B4_Pin) == GPIO_PIN_RESET) {
            HAL_Delay(XDTIME);
            if (HAL_GPIO_ReadPin(B4_GPIO_Port, B4_Pin) == GPIO_PIN_RESET) {
                currentPage++;
                paraChoose = CPD;
                Fresh();
                HAL_Delay(XDTIME2);
            }
        } else if (HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_RESET) {
            HAL_Delay(XDTIME);
            if (HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_RESET) {
                switch (paraChoose) {
                    case CPD:
                        if (PD < 1000) {
                            PD += 100;
                            sprintf((char *)message, "     PD=%dHz     ", PD);
                            LCD_DisplayStringLine(Line4, message);
                        }
                        break;
                    case CPH:
                        if (PH < 10000) {
                            PH += 100;
                            sprintf((char *)message, "     PH=%dHz     ", PH);
                            LCD_DisplayStringLine(Line5, message);
                        }
                        break;
                    case CPX:
                        if (PX < 1000) {
                            PX += 100;
                            sprintf((char *)message, "     PX=%.0fHz     ", PX);
                            LCD_DisplayStringLine(Line6, message);
                        }
                        break;
                }
                HAL_Delay(XDTIME2);
            }
        } else if (HAL_GPIO_ReadPin(B2_GPIO_Port, B2_Pin) == GPIO_PIN_RESET) {
            HAL_Delay(XDTIME);
            if (HAL_GPIO_ReadPin(B2_GPIO_Port, B2_Pin) == GPIO_PIN_RESET) {
                switch (paraChoose) {
                    case CPD:
                        if (PD > 100) {
                            PD -= 100;
                            sprintf((char *)message, "     PD=%dHz     ", PD);
                            LCD_DisplayStringLine(Line4, message);
                        }
                        break;
                    case CPH:
                        if (PH > 1000) {
                            PH -= 100;
                            sprintf((char *)message, "     PH=%dHz     ", PH);
                            LCD_DisplayStringLine(Line5, message);
                        }
                        break;
                    case CPX:
                        if (PX > -1000) {
                            PX -= 100;
                            sprintf((char *)message, "     PX=%.0fHz     ", PX);
                            LCD_DisplayStringLine(Line6, message);
                        }
                        break;
                }
                HAL_Delay(XDTIME2);
            }
        } else if (HAL_GPIO_ReadPin(B3_GPIO_Port, B3_Pin) == GPIO_PIN_RESET) {
            HAL_Delay(XDTIME);
            if (HAL_GPIO_ReadPin(B3_GPIO_Port, B3_Pin) == GPIO_PIN_RESET) {
                paraChoose++;
                if (paraChoose > CPX)
                    paraChoose = CPD;
                HAL_Delay(XDTIME2);
            }
        }
    } else if (currentPage == RECDPAGE) {
        if (HAL_GPIO_ReadPin(B4_GPIO_Port, B4_Pin) == GPIO_PIN_RESET) {
            HAL_Delay(XDTIME);
            if (HAL_GPIO_ReadPin(B4_GPIO_Port, B4_Pin) == GPIO_PIN_RESET) {
                currentPage = 0;
                Fresh();
                HAL_Delay(XDTIME2);
            }
        } else if (HAL_GPIO_ReadPin(B3_GPIO_Port, B3_Pin) == GPIO_PIN_RESET) {
            HAL_Delay(XDTIME);
            uint32_t sTime = HAL_GetTick();
            if (HAL_GPIO_ReadPin(B3_GPIO_Port, B3_Pin) == GPIO_PIN_RESET) {
                while (HAL_GPIO_ReadPin(B3_GPIO_Port, B3_Pin) == GPIO_PIN_RESET) {
                    if (HAL_GetTick() - sTime >= 1000) {
                        NDA = 0;
                        NDB = 0;
                        NHA = 0;
                        NHB = 0;
                        sprintf((char *)message, "     NDA=%d          ", NDA);
                        LCD_DisplayStringLine(Line4, message);
                        sprintf((char *)message, "     NDB=%d          ", NDB);
                        LCD_DisplayStringLine(Line5, message);
                        sprintf((char *)message, "     NHA=%d          ", NHA);
                        LCD_DisplayStringLine(Line6, message);
                        sprintf((char *)message, "     NHB=%d          ", NHB);
                        LCD_DisplayStringLine(Line7, message);
                    }
                }
                HAL_Delay(XDTIME2);
            }
        }
    }
}