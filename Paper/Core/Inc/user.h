#ifndef _USER_H_
#define _USER_H_
#define ERROR_A -18
#define ERROR_B -21
#define ERROR_C -22

#define RE_LEN  1.244
#define RF_LEN  0.524
#define F_LEN   0.576
#define E_LEN   0.076
#define NAA     -91.0

#define SQRT3   1.7320508075688772935274463415
#define Y1      (-0.5 * F_LEN / SQRT3)
#define PI      3.1415926
#include "stm32f1xx_hal.h"
#include "userGeneral.h"
#include <math.h>

typedef struct
{
    double A;
    double B;
    double C;
} TriAngle;

typedef struct
{
    double X;
    double Y;
    double Z;
} TriPos;

typedef struct
{
    TriPos P1;
    TriPos P2;
    TriPos P3;
    TriPos P4;
    TriPos P5;
} diffPosType;

typedef struct
{
    TriAngle A1;
    TriAngle A2;
    TriAngle A3;
    TriAngle A4;
    TriAngle A5;
} diffAngleType;

diffPosType diffPosCal(double X, double Y, double Z, double newX, double newY, double newZ);
diffAngleType diffAngleCal(diffPosType diffPos);
void PWMStart(void);
void ServoSetAngle(TriAngle Angle);
TriAngle CalAngle(TriPos Pos);
TriPos ReverseCal(TriAngle Angle);

#endif