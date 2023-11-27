#ifndef _KALMANF_H_
#define _KALMANF_H_

typedef struct
{
    float Last_P; // 上次估算协方差 不可以为0 ! ! ! ! !
    float Now_P;  // 当前估算协方差
    float out;    // 卡尔曼滤波器输出
    float Kg;     // 卡尔曼增益
    float Q;      // 过程噪声协方差
    float R;      // 观测噪声协方差
} KFHandler;

extern KFHandler hkfpX;
extern KFHandler hkfpY;
extern KFHandler hkfpZ;

void Kalman_Init(KFHandler *hkfp);
float KalmanFilter(KFHandler *hkfp, float input);

#endif