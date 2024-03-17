#include "screen.h"

int8_t pwmMode         = 0;
int8_t paraR           = 1;
int8_t paraK           = 1;
int8_t currentPage     = 1;
int8_t isROrK          = 1;
int8_t pwmSwithable    = 1;
int32_t switchTimes    = 0;
int8_t ld2Con          = 0;
int32_t pwmSwitchCount = 0;
int8_t lockStatus      = 0;
int8_t pwmSwitchType   = 0;
double pulse           = 0;
char message[100];

extern TIM_HandleTypeDef htim6;
extern TIM_HandleTypeDef htim2;
extern ADC_HandleTypeDef hadc2;

enum int8_t {
    // 显示页面
    DATAPAGE = 1,
    PARAPAGE = 2,
    RECDPAGE = 3,
    // PWM工作模式
    HIGH = 1,
    LOW  = 0,
    // PWM改变模式
    TOHIGH = 0,
    TOLOW  = 1,
    // 选中R还是K
    ISR = 1,
    ISK = 0,
};

void ledCon()
{
    // 设定不亮的灯
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15 | GPIO_PIN_11 | GPIO_PIN_12, GPIO_PIN_SET);
    // DataPage灯
    if (currentPage == DATAPAGE) {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);
    } else {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_SET);
    }
    // pwm灯，闪烁
    if (pwmSwithable == 0) {
        if (ld2Con == 1) {
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_SET);
            ld2Con = 0;
        } else {
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET);
            ld2Con = 1;
        }
    } else {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_SET);
        ld2Con = 0;
    }
    // 锁定灯
    if (lockStatus == 0) {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10, GPIO_PIN_RESET);
    }
    // 锁存
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_RESET);
}

void ShowDataPage()
{
    LCD_DisplayStringLine(Line2, (uint8_t *)"        DATA");
    if (pwmMode == HIGH) {
        LCD_DisplayStringLine(Line4, (uint8_t *)"     M=H");
    } else {
        LCD_DisplayStringLine(Line4, (uint8_t *)"     M=L");
    }
    // 占空比在定时器完成
}

void ShowParaPage()
{
    isROrK = ISR;
    LCD_DisplayStringLine(Line2, (uint8_t *)"        PARA");
    sprintf(message, "%s%d", "     R=", paraR);
    LCD_DisplayStringLine(Line4, (uint8_t *)message);
    sprintf(message, "%s%d", "     K=", paraK);
    LCD_DisplayStringLine(Line5, (uint8_t *)message);
}

void ShowRecdPage()
{
    LCD_DisplayStringLine(Line2, (uint8_t *)"        RECD");
    sprintf(message, "%s%d", "     N=", switchTimes);
    LCD_DisplayStringLine(Line4, (uint8_t *)message);
}

// 页面控制
void ShowCurrentPage()
{
    ledCon();
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
            currentPage = 1;
            ShowDataPage();
            break;
    }
}

void ScanKey()
{
    // 扫描到B1切换
    if (HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_RESET) {
        // 切换屏幕
        // 必须是先++，否则Clear后定时器触发，显示错误
        currentPage++;
        LCD_Clear(Black);
        if (currentPage > 3)
            currentPage = DATAPAGE;
        ShowCurrentPage();
    }
    // B2功能
    // 数据界面切换高低频
    if (currentPage == DATAPAGE) {
        if (HAL_GPIO_ReadPin(B2_GPIO_Port, B2_Pin) == GPIO_PIN_RESET) {
            if (pwmSwithable == 1) {
                // 反转工作模式
                pwmMode = pwmMode == HIGH ? LOW : HIGH;
                switchTimes++;
                if (pwmMode == LOW) {
                    pwmSwitchType = TOLOW;
                    LCD_DisplayStringLine(Line4, (uint8_t *)"     M=L");
                } else {
                    pwmSwitchType = HIGH;
                    LCD_DisplayStringLine(Line4, (uint8_t *)"     M=H");
                }
                // 全部设置完再开启定时器
                pwmSwithable = 0;
            }
        }
    }
    // 参数节目改变R还是K
    if (currentPage == PARAPAGE) {
        if (HAL_GPIO_ReadPin(B2_GPIO_Port, B2_Pin) == GPIO_PIN_RESET) {
            HAL_Delay(XDTIME);
            isROrK = isROrK == ISR ? ISK : ISR;
        }
    }

    // B3功能
    if (currentPage == PARAPAGE) {
        if (HAL_GPIO_ReadPin(B3_GPIO_Port, B3_Pin) == GPIO_PIN_RESET) {
            if (isROrK == ISR) {
                paraR++;
                if (paraR > 10)
                    paraR = 1;
                // 清行防止10的0残留
                LCD_ClearLine(Line4);
                sprintf(message, "%s%d", "     R=", paraR);
                LCD_DisplayStringLine(Line4, (uint8_t *)message);
            }
            if (isROrK == ISK) {
                paraK++;
                if (paraK > 10)
                    paraK = 1;
                LCD_ClearLine(Line5);
                sprintf(message, "%s%d", "     K=", paraK);
                LCD_DisplayStringLine(Line5, (uint8_t *)message);
            }
            // 加长消抖防止连按
            HAL_Delay(XDTIME);
        }
    }

    // B4功能
    // 参数调整
    if (currentPage == PARAPAGE) {
        if (HAL_GPIO_ReadPin(B4_GPIO_Port, B4_Pin) == GPIO_PIN_RESET) {
            if (isROrK == ISR) {
                paraR--;
                if (paraR < 1)
                    paraR = 10;
                LCD_ClearLine(Line4);
                sprintf(message, "%s%d", "     R=", paraR);
                LCD_DisplayStringLine(Line4, (uint8_t *)message);
            }
            if (isROrK == ISK) {
                paraK--;
                if (paraK < 1)
                    paraK = 10;
                LCD_ClearLine(Line5);
                sprintf(message, "%s%d", "     K=", paraK);
                LCD_DisplayStringLine(Line5, (uint8_t *)message);
            }
            HAL_Delay(XDTIME);
        }
    }
    // 锁定
    if (currentPage == DATAPAGE) {
        // 短按
        if (lockStatus == 1) {
            if (HAL_GPIO_ReadPin(B4_GPIO_Port, B4_Pin) == GPIO_PIN_RESET) {
                lockStatus = 0;
                ledCon();
            }
        }
        // 长按
        if (lockStatus == 0) {
            if (HAL_GPIO_ReadPin(B4_GPIO_Port, B4_Pin) == GPIO_PIN_RESET) {
                uint32_t sTime = HAL_GetTick();
                // 消抖后再计时
                HAL_Delay(XDTIME);
                // 等待2s或弹起
                while (HAL_GPIO_ReadPin(B4_GPIO_Port, B4_Pin) == GPIO_PIN_RESET) {
                    uint32_t pTime = HAL_GetTick() - sTime;
                    if (pTime > 2000) {
                        lockStatus = 1;
                        ledCon();
                        break;
                    }
                }
                // 等待弹起
                // 弹起跳出则直接pass
                // 2s跳出则等待弹起
                while (HAL_GPIO_ReadPin(B4_GPIO_Port, B4_Pin) == GPIO_PIN_RESET)
                    ;
            }
        }
    }
}

// ARR从999到1999，增长1000
// 5s,定时器为0.1s
// 触发50次
// 每次20
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim == &htim6) {
        // pwm部分
        if (pwmSwithable == 0) {
            pwmSwitchCount++;
            ledCon();
            // TOHIGH是减少
            if (pwmSwitchType == TOHIGH)
                htim2.Instance->ARR -= 20;
            else
                htim2.Instance->ARR += 20;

            if (pwmSwitchCount >= 50) {
                pwmSwitchCount = 0;
                pwmSwithable   = 1;
                ledCon();
            }
        }
        // adc部分
        // 未锁定则改变占空比
        HAL_ADC_Start(&hadc2);
        double adcValue = (double)HAL_ADC_GetValue(&hadc2) / 4095 * 3.3;
        // 未锁定则改变占空比
        if (lockStatus == 0) {
            if (adcValue > 3)
                pulse = 0.83;
            else if (adcValue < 1)
                pulse = 0.1;
            else
                pulse = (0.83 - 0.1) / 2 * (adcValue - 1) + 0.1;
            __HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_2, pulse * htim2.Instance->ARR);
        }

        if (currentPage == DATAPAGE) {
            sprintf(message, "%s%d%c", "     P=", (int)(pulse * 100), '%');
            LCD_DisplayStringLine(Line5, (uint8_t *)message);
        }
    }
}
