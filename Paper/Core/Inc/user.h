#ifndef _USER_H_
#define _USER_H_

#define ERROR_A -14
#define ERROR_B -19
#define ERROR_C -18

#define L_LEN   114
#define SL_LEN  125
#define R_LEN   48.586
#define SR_LEN  31.586
#define DIFFNUM 50

#define SQRT3   1.7320508075688772935274463415
#define Y1      (-R_LEN / SQRT3)
#define PI      3.1415926
#include "main.h"
#include "userGeneral.h"
#include <math.h>

typedef struct
{
    float A;
    float B;
    float C;
} TriAngle;

typedef struct
{
    float X;
    float Y;
    float Z;
} TriPos;

#ifdef USE_UART
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size);
#endif

void diffPosCal(TriPos currentPos, TriPos targetPos);
void diffAngleCal(void);
void PWMStart(void);
void ServoSetTriAngle(TriAngle Angle);
void ServoSetAngle(float A, float B, float C);
TriAngle CalAngle(TriPos Pos);
// TriPos ReverseCal(TriAngle Angle);
void RunPos(float X, float Y, float Z);
void RunTriPos(TriPos targetPos);

#endif