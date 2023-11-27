#include "touch.h"

// 读写命令
const uint8_t CMD_RDX = 0XD0;
const uint8_t CMD_RDY = 0X90;

// 带滤波的单方向读取
uint16_t Touch_Read_XOrY(uint8_t cmd)
{
    uint16_t buf[READ_TIMES];
    uint16_t sum = 0;
    uint16_t temp;
    // AD读取一定次数
    for (int i = 0; i < READ_TIMES; i++)
        buf[i] = Touch_Read_SPI(cmd);
    // 冒泡排序
    for (int i = 0; i < READ_TIMES - 1; i++)
    {
        for (int j = i + 1; j < READ_TIMES; j++)
        {
            if (buf[i] > buf[j])
            {
                temp = buf[i];
                buf[i] = buf[j];
                buf[j] = temp;
            }
        }
    }
    // 去掉最大最小的LOST_VAL个值
    for (int i = LOST_VAL; i < READ_TIMES - LOST_VAL; i++)
        sum += buf[i];
    temp = sum / (READ_TIMES - 2 * LOST_VAL);
    return temp;
}

// 双方向读取
uint8_t Touch_Read(uint16_t *x, uint16_t *y)
{
    *x = Touch_Read_SPI(CMD_RDX);
    *y = Touch_Read_SPI(CMD_RDY);
    return 1;
}

uint16_t Touch_Read_SPI(uint8_t CMD)
{
    uint8_t count = 0;
    uint16_t Num = 0;
    HAL_GPIO_WritePin(T_SCK_GPIO, T_SCK_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(T_IN_GPIO, T_IN_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(T_CS_GPIO, T_CS_PIN, GPIO_PIN_RESET);

    Touch_Write_Byte(CMD);

    delay_us(6);
    HAL_GPIO_WritePin(T_SCK_GPIO, T_SCK_PIN, GPIO_PIN_RESET);
    delay_us(1);
    HAL_GPIO_WritePin(T_SCK_GPIO, T_SCK_PIN, GPIO_PIN_SET);
    delay_us(1);
    HAL_GPIO_WritePin(T_SCK_GPIO, T_SCK_PIN, GPIO_PIN_RESET);
    for (count = 0; count < 16; count++)
    {
        Num <<= 1;
        HAL_GPIO_WritePin(T_SCK_GPIO, T_SCK_PIN, GPIO_PIN_RESET);
        delay_us(1);
        HAL_GPIO_WritePin(T_SCK_GPIO, T_SCK_PIN, GPIO_PIN_SET);
        if (HAL_GPIO_ReadPin(T_OUT_GPIO, T_OUT_PIN) == GPIO_PIN_SET)
            Num++;
    }
    Num >>= 4;
    HAL_GPIO_WritePin(T_SCK_GPIO, T_SCK_PIN, GPIO_PIN_SET);
    return (Num);
}

void Touch_Write_Byte(uint8_t num)
{
    uint8_t count;
    for (count = 0; count < 8; ++count)
    {
        if (num & 0x80)
            HAL_GPIO_WritePin(T_IN_GPIO, T_IN_PIN, GPIO_PIN_SET);
        else
            HAL_GPIO_WritePin(T_IN_GPIO, T_IN_PIN, GPIO_PIN_RESET);
        num <<= 1;
        HAL_GPIO_WritePin(T_SCK_GPIO, T_SCK_PIN, GPIO_PIN_RESET);
        delay_us(1);
        HAL_GPIO_WritePin(T_SCK_GPIO, T_SCK_PIN, GPIO_PIN_SET);
    }
}