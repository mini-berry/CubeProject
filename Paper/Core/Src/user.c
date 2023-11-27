#include "user.h"

extern TIM_HandleTypeDef htim1;

void PWMStart()
{
    TriAngle Angle;
    Angle.A = 30;
    Angle.B = 30;
    Angle.C = 30;
    ServoSetAngel(Angle);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
}
// PA8、PA9、PA10
void ServoSetAngel(TriAngle Angel)
{
    if (Angel.A<-90 | Angel.A> 90)
    {
        userPrint("A Out.\n");
        return;
    }
    if (Angel.B<-90 | Angel.B> 90)
    {
        userPrint("B Out.\n");
        return;
    }
    if (Angel.C<-90 | Angel.C> 90)
    {
        userPrint("C Out.\n");
        return;
    }

    // -90 0.5ms 90 2.5ms
    // 72000000/72/40000=250Hz
    // 4ms=4000
    // 0.5=500 2.5=2500
    __HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_1, (int16_t)(((Angel.A + ERROR_A) - (-90)) * 2000 / 180 + 500 - 1));
    __HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_2, (int16_t)(((Angel.B + ERROR_B) - (-90)) * 2000 / 180 + 500 - 1));
    __HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_3, (int16_t)(((Angel.C + ERROR_C) - (-90)) * 2000 / 180 + 500 - 1));
}
__STATIC_INLINE double SolveAngel(double X, double Y, double Z)
{
    double A = (X * X + Y * Y + Z * Z + RF_LEN * RF_LEN - RE_LEN * RE_LEN - Y1 * Y1) / (2 * Z);
    double B = (Y1 - Y) / (Z);
    double delta = -(A + B * Y1) * (A + B * Y1) + RF_LEN * (RF_LEN * B * B + RF_LEN);
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
TriAngle CalAngel(double X, double Y, double Z)
{
    TriAngle Angel;

    double XA = X;
    double YA = Y - E_LEN / SQRT3 / 2;
    double ZA = Z;

    double XB = -X / 2 - Y / 2 * SQRT3;
    double YB = X / 2 * SQRT3 - Y / 2 - E_LEN / SQRT3 / 2;
    double ZB = Z;

    double XC = -X / 2 + Y / 2 * SQRT3;
    double YC = -X / 2 * SQRT3 - Y / 2 - E_LEN / SQRT3 / 2;
    double ZC = Z;

    Angel.A = SolveAngel(XA, YA, ZA);
    Angel.B = SolveAngel(XB, YB, ZB);
    Angel.C = SolveAngel(XC, YC, ZC);

    return Angel;
}