#include "user.h"

uint8_t receiveData[100] = {0};
uint32_t receiveLen;
float posX, posY, posZ;
uint8_t dataGet = 0;

extern UART_HandleTypeDef huart1;

extern TIM_HandleTypeDef htim1;

TriPos currentPos = {0, 0, 150};

TriPos diffPos[DIFFNUM + 1];
TriAngle diffAngle[DIFFNUM + 1];

#ifdef USE_UART
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (huart == &huart1)
    {
        switch (HAL_UARTEx_GetRxEventType(huart))
        {
        case HAL_UART_RXEVENT_IDLE:
            receiveLen += Size;
            if (receiveData[receiveLen - 1] == ';')
            {
                dataGet = 1;
                receiveData[receiveLen] = '\0';
                receiveLen = 0;
                HAL_UARTEx_ReceiveToIdle_IT(&huart1, receiveData + receiveLen, 50);
            }
            else
                HAL_UARTEx_ReceiveToIdle_IT(&huart1, receiveData + receiveLen, 50);
            break;
        case HAL_UART_RXEVENT_TC:
            break;
        case HAL_UART_RXEVENT_HT:
            break;
        default:
            break;
        }
    }
}
#endif

// A B C
// PA8、PA9、PA10
void PWMStart()
{
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
}

void ServoSetTriAngle(TriAngle Angle)
{
    // -90 0.5ms 90 2.5ms
    // 72000000/72/40000=250Hz
    // 4ms=4000
    // 0.5=500 2.5=2500
    __HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_1, (int16_t)(((Angle.A + ERROR_A) - (-90)) * 2000 / 180 + 500 - 1));
    __HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_2, (int16_t)(((Angle.B + ERROR_B) - (-90)) * 2000 / 180 + 500 - 1));
    __HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_3, (int16_t)(((Angle.C + ERROR_C) - (-90)) * 2000 / 180 + 500 - 1));
}
void ServoSetAngle(float A, float B, float C)
{
    __HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_1, (int16_t)(((A + ERROR_A) - (-90)) * 2000 / 180 + 500 - 1));
    __HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_2, (int16_t)(((B + ERROR_B) - (-90)) * 2000 / 180 + 500 - 1));
    __HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_3, (int16_t)(((C + ERROR_C) - (-90)) * 2000 / 180 + 500 - 1));
}

__STATIC_INLINE double SolveAngle(double X, double Y, double Z)
{
    double A = (X * X + Y * Y + Z * Z + L_LEN * L_LEN - SL_LEN * SL_LEN - Y1 * Y1) / (2 * Z);

    double B = (Y1 - Y) / (Z);
    double delta = -(A + B * Y1) * (A + B * Y1) + L_LEN * (L_LEN * B * B + L_LEN);
    if (delta < 0)
        return -91;
    else
    {
        double YF = (Y1 - A * B - sqrt(delta)) / (B * B + 1);
        double ZF = A + B * YF;
        double theta = 180 * atan(-ZF / (Y1 - YF)) / PI;
        if (YF > Y1)
            theta = theta + 180;
        return theta;
    }
}

TriAngle CalAngle(TriPos Pos)
{
    double X = -Pos.X;
    double Y = Pos.Y;
    double Z = -Pos.Z;
    TriAngle Angle;

    double XA = -X;
    double YA = Y - SR_LEN / SQRT3;
    double ZA = Z;

    double XB = X / 2 + Y / 2 * SQRT3;
    double YB = X / 2 * SQRT3 - Y / 2 - SR_LEN / SQRT3;
    double ZB = Z;

    double XC = X / 2 - Y / 2 * SQRT3;
    double YC = -X / 2 * SQRT3 - Y / 2 - SR_LEN / SQRT3;
    double ZC = Z;

    Angle.A = SolveAngle(XA, YA, ZA);
    Angle.B = SolveAngle(XB, YB, ZB);
    Angle.C = SolveAngle(XC, YC, ZC);

    return Angle;
}

void diffPosCal(TriPos currentPos, TriPos targetPos)
{
    const static float diffnumf = DIFFNUM;
    for (int8_t i = 0; i < diffnumf + 1; ++i)
    {
        diffPos[i].X = i / diffnumf * targetPos.X + (diffnumf - i) / diffnumf * currentPos.X;
        diffPos[i].Y = i / diffnumf * targetPos.Y + (diffnumf - i) / diffnumf * currentPos.Y;
        diffPos[i].Z = i / diffnumf * targetPos.Z + (diffnumf - i) / diffnumf * currentPos.Z;
    }
}

void diffAngleCal(void)
{
    for (int8_t i = 0; i < DIFFNUM + 1; ++i)
    {
        diffAngle[i] = CalAngle(diffPos[i]);
    }
}

void RunTriPos(TriPos targetPos)
{
    diffPosCal(currentPos, targetPos);
    currentPos = targetPos;
    diffAngleCal();
    for (int8_t i = 0; i < DIFFNUM + 1; ++i)
    {
        ServoSetAngle(diffAngle[i].A, diffAngle[i].B, diffAngle[i].C);
        HAL_Delay(10);
    }
}

void RunPos(float X, float Y, float Z)
{
    TriPos targetPos = {X, Y, Z};
    RunTriPos(targetPos);
}