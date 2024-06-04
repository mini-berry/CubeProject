/**
 ******************************************************************************
 * @file           : ws281x.c
 * @brief          : ws281x LED Driver
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2020 SYEtronix Inc.
 * δ��������ɣ��������������κ���;
 * ��������:2020/2/18
 * �汾��V1.0
 * ��Ȩ���У�����ؾ���
 * Copyright(C) ������ʤ�������豸���޹�˾ SYEtronix Inc.
 * All rights reserved
 *
 ******************************************************************************
 */

#include "ws281x.h"
#include <string.h>
#include "tim.h"

// ����Ϊ24V��RGB��8λ
// NUMΪ����������ÿ������ռ24λ�����ϸ�λ�ź�300us
// ��Ҫ��ʱ��Ƶ������Ϊ800KHz
// PWM�Ĵ���Ϊ16λ����Ҫһ�η���һ������
uint16_t send_Buf[NUM];

// #define PROCESS_DELAY 5
// #define SECTION_DELAY 100

void WS_Load(void)
{
    // ��������
    HAL_TIM_PWM_Start_DMA(&htim3, TIM_CHANNEL_1, (uint32_t *)send_Buf, NUM);
}
void WS_CloseAll(void)
{
    uint16_t i;
    // ǰ��д���߼�0
    for (i = 0; i < PIXEL_NUM * 24; i++)
        send_Buf[i] = WS0;
    // ���д��͵�ƽ
    for (i = PIXEL_NUM * 24; i < NUM; i++)
        send_Buf[i] = 0;
    WS_Load();
}
// ���ݸ�ʽΪWS2812B��ÿ������ռ24λ���ߵ�ƽ0.8us���͵�ƽ0.45us
// WS1��ʾ1��WS0��ʾ0
void WS_WriteAll_RGB(uint8_t n_R, uint8_t n_G, uint8_t n_B)
{
    uint16_t i, j;
    uint8_t dat[24];
    // ��RGB���ݽ���ת��
    for (i = 0; i < 8; i++) {
        dat[i] = ((n_G & 0x80) ? WS1 : WS0);
        n_G <<= 1;
    }
    for (i = 0; i < 8; i++) {
        dat[i + 8] = ((n_R & 0x80) ? WS1 : WS0);
        n_R <<= 1;
    }
    for (i = 0; i < 8; i++) {
        dat[i + 16] = ((n_B & 0x80) ? WS1 : WS0);
        n_B <<= 1;
    }
    for (i = 0; i < PIXEL_NUM; i++) {
        for (j = 0; j < 24; j++) {
            send_Buf[i * 24 + j] = dat[j];
        }
    }
	// ʣ�����Ϊ0
    for (i = PIXEL_NUM * 24; i < NUM; i++)
        send_Buf[i] = 0; // ռ�ձȱ�Ϊ0��ȫΪ�͵�ƽ
    WS_Load();
}

uint32_t WS281x_Color(uint8_t red, uint8_t green, uint8_t blue)
{
    return green << 16 | red << 8 | blue;
}
// ��������ĳ�����飬ʹ��Colorֵ
void WS281x_SetPixelColor(uint16_t n, uint32_t GRBColor)
{
    uint8_t i;
    if (n < PIXEL_NUM) {
        for (i = 0; i < 24; ++i)
            send_Buf[24 * n + i] = (((GRBColor << i) & 0X800000) ? WS1 : WS0);
    }
}
// ��������ĳ�������RGBֵ
void WS281x_SetPixelRGB(uint16_t n, uint8_t red, uint8_t green, uint8_t blue)
{
    uint8_t i;

    if (n < PIXEL_NUM) {
        for (i = 0; i < 24; ++i)
            send_Buf[24 * n + i] = (((WS281x_Color(red, green, blue) << i) & 0X800000) ? WS1 : WS0);
    }
}

uint32_t Wheel(uint8_t WheelPos)
{
    WheelPos = 255 - WheelPos;
    if (WheelPos < 85) {
        return WS281x_Color(255 - WheelPos * 3, 0, WheelPos * 3);
    }
    if (WheelPos < 170) {
        WheelPos -= 85;
        return WS281x_Color(0, WheelPos * 3, 255 - WheelPos * 3);
    }
    WheelPos -= 170;
    return WS281x_Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void rainbow(uint8_t wait)
{
    uint32_t timestamp = HAL_GetTick();
    uint16_t i;
    static uint8_t j;
    static uint32_t next_time = 0;

    uint32_t flag = 0;
    if (next_time < wait) {
        if ((uint64_t)timestamp + wait - next_time > 0)
            flag = 1;
    } else if (timestamp > next_time) {
        flag = 1;
    }
    if (flag) // && (timestamp - next_time < wait*5))
    {
        j++;
        next_time = timestamp + wait;
        for (i = 0; i < PIXEL_NUM; i++) {
            WS281x_SetPixelColor(i, Wheel((i + j) & 255));
        }
    }
    WS_Load();
}

void rainbowCycle(uint8_t wait)
{
    uint32_t timestamp = HAL_GetTick();
    uint16_t i;
    static uint8_t j;
    static uint32_t next_time = 0;

    static uint8_t loop = 0;
    if (loop == 0)
        next_time = timestamp;
    loop = 1; // �״ε��ó�ʼ��

    if ((timestamp > next_time)) // && (timestamp - next_time < wait*5))
    {
        j++;
        next_time = timestamp + wait;
        for (i = 0; i < PIXEL_NUM; i++) {
            WS281x_SetPixelColor(i, Wheel(((i * 256 / (PIXEL_NUM)) + j) & 255));
        }
    }
    WS_Load();
}
