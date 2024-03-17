#include "user.h"

extern char receiveData[];
extern uint32_t receiveLen;
extern UART_HandleTypeDef huart1;

extern TIM_HandleTypeDef htim1;
#ifdef USE_UART
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (huart == &huart1) {
        switch (HAL_UARTEx_GetRxEventType(huart)) {
            case HAL_UART_RXEVENT_IDLE:
                receiveLen = Size;
                HAL_UARTEx_ReceiveToIdle_IT(huart, (uint8_t *)receiveData, 100);
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

diffPosType diffPosCal(double X, double Y, double Z, double newX, double newY, double newZ)
{
    diffPosType diffPos;

    diffPos.P1.X = X;
    diffPos.P1.Y = Y;
    diffPos.P1.Z = Z;

    diffPos.P1.X = 0.8 * X + 0.2 * newX;
    diffPos.P1.Y = 0.8 * Y + 0.2 * newY;
    diffPos.P1.Z = 0.8 * Z + 0.2 * newZ;

    diffPos.P2.X = 0.6 * X + 0.4 * newX;
    diffPos.P2.Y = 0.6 * Y + 0.4 * newY;
    diffPos.P2.Z = 0.6 * Z + 0.4 * newZ;

    diffPos.P3.X = 0.4 * X + 0.6 * newX;
    diffPos.P3.Y = 0.4 * Y + 0.6 * newY;
    diffPos.P3.Z = 0.4 * Z + 0.6 * newZ;

    diffPos.P4.X = 0.2 * X + 0.8 * newX;
    diffPos.P4.Y = 0.2 * Y + 0.8 * newY;
    diffPos.P4.Z = 0.2 * Z + 0.8 * newZ;

    diffPos.P5.X = newX;
    diffPos.P5.Y = newY;
    diffPos.P5.Z = newZ;
    return diffPos;
}

void PWMStart()
{
    TriAngle Angle;

    Angle.A = 30;
    Angle.B = 30;
    Angle.C = 30;

    ServoSetAngle(Angle);

    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
}

// PA8、PA9、PA10
void ServoSetAngle(TriAngle Angle)
{
    if (Angle.A < -90 | Angle.A > 90) {
        userPrint("A Out.\n");
        return;
    }
    if (Angle.B < -90 | Angle.B > 90) {
        userPrint("B Out.\n");
        return;
    }
    if (Angle.C < -90 | Angle.C > 90) {
        userPrint("C Out.\n");
        return;
    }

    // -90 0.5ms 90 2.5ms
    // 72000000/72/40000=250Hz
    // 4ms=4000
    // 0.5=500 2.5=2500
    __HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_1, (int16_t)(((Angle.A + ERROR_A) - (-90)) * 2000 / 180 + 500 - 1));
    __HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_2, (int16_t)(((Angle.B + ERROR_B) - (-90)) * 2000 / 180 + 500 - 1));
    __HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_3, (int16_t)(((Angle.C + ERROR_C) - (-90)) * 2000 / 180 + 500 - 1));
}

__STATIC_INLINE double SolveAngle(double X, double Y, double Z)
{
    double A     = (X * X + Y * Y + Z * Z + RF_LEN * RF_LEN - RE_LEN * RE_LEN - Y1 * Y1) / (2 * Z);
    double B     = (Y1 - Y) / (Z);
    double delta = -(A + B * Y1) * (A + B * Y1) + RF_LEN * (RF_LEN * B * B + RF_LEN);
    if (delta < 0)
        return -91;
    else {
        double YF    = (Y1 - A * B - sqrt(delta)) / (B * B + 1);
        double ZF    = A + B * YF;
        double theta = 180 * atan(-ZF / (Y1 - YF)) / PI;
        if (YF > Y1)
            theta = theta + 180;
        return theta;
    }
}

TriAngle CalAngle(TriPos Pos)
{
    double X = Pos.X;
    double Y = Pos.Y;
    double Z = Pos.Z;
    TriAngle Angle;

    double XA = X;
    double YA = Y - E_LEN / SQRT3 / 2;
    double ZA = Z;

    double XB = -X / 2 - Y / 2 * SQRT3;
    double YB = X / 2 * SQRT3 - Y / 2 - E_LEN / SQRT3 / 2;
    double ZB = Z;

    double XC = -X / 2 + Y / 2 * SQRT3;
    double YC = -X / 2 * SQRT3 - Y / 2 - E_LEN / SQRT3 / 2;
    double ZC = Z;

    Angle.A = SolveAngle(XA, YA, ZA);
    Angle.B = SolveAngle(XB, YB, ZB);
    Angle.C = SolveAngle(XC, YC, ZC);

    return Angle;
}

diffAngleType diffAngleCal(diffPosType diffPos)
{
    TriAngle Angle;
    diffAngleType diffAngle;

    diffAngle.A1 = CalAngle(diffPos.P1);
    diffAngle.A2 = CalAngle(diffPos.P2);
    diffAngle.A3 = CalAngle(diffPos.P3);
    diffAngle.A4 = CalAngle(diffPos.P4);
    diffAngle.A5 = CalAngle(diffPos.P5);

    return diffAngle;
}

TriPos ReverseCal(TriAngle Angle)
{
    TriPos Pos;
    double A, B, C;
    A = Angle.A;
    B = Angle.B;
    C = Angle.C;

    // To be done

    return Pos;
}