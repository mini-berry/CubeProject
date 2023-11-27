#ifndef _USER_H_
#define _USER_H_
#define ERROR_A -18
#define ERROR_B -21
#define ERROR_C -22

#define RE_LEN 1.244
#define RF_LEN 0.524
#define F_LEN 0.576
#define E_LEN 0.076
#define NAA -91.0

#define SQRT3 1.7320508075688772935274463415
#define Y1 (-0.5 * F_LEN / SQRT3)
#define PI 3.1415926
#include "stm32f1xx_hal.h"
#include "userGeneral.h"
#include <math.h>

typedef struct
{
    double A;
    double B;
    double C;
} TriAngle;

void PWMStart(void);
void ServoSetAngel(TriAngle Angel);
TriAngle CalAngel(double x, double y, double z);

#endif