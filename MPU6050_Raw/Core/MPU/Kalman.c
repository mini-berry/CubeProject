#include "Kalman.h"

KFHandler hkfpX;
KFHandler hkfpY;
KFHandler hkfpZ;


/// @brief 滤波参数初始化
/// @param KFHandler 
void Kalman_Init(KFHandler *hkfp)
{
    hkfp->Last_P = 1;
    hkfp->Now_P  = 0;
    hkfp->out    = 0;
    hkfp->Kg     = 0;
    hkfp->Q      = 0;
    hkfp->R      = 0.01;
}

/// @brief 卡尔曼滤波
/// @param KFHandler 滤波参数
/// @param input 输入值
/// @return 输出
float KalmanFilter(KFHandler *hkfp, float input)
{
    // 预测协方差方程：k时刻系统估算协方差 = k-1时刻的系统协方差 + 过程噪声协方差
    hkfp->Now_P = hkfp->Last_P + hkfp->Q;
    // 卡尔曼增益方程：卡尔曼增益 = k时刻系统估算协方差 / （k时刻系统估算协方差 + 观测噪声协方差）
    hkfp->Kg = hkfp->Now_P / (hkfp->Now_P + hkfp->R);
    // 更新最优值方程：k时刻状态变量的最优值 = 状态变量的预测值 + 卡尔曼增益 * （测量值 - 状态变量的预测值）
    hkfp->out = hkfp->out + hkfp->Kg * (input - hkfp->out); // 因为这一次的预测值就是上一次的输出值
    // 更新协方差方程: 本次的系统协方差付给 hkfp->LastP 威下一次运算准备。
    hkfp->Last_P = (1 - hkfp->Kg) * hkfp->Now_P;
    return hkfp->out;
}
