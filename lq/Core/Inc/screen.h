#ifndef SCREEN_H
#define SCREEN_H

#include "lcd_hal.h"
#include "stm32g4xx.h"
#include "stdio.h"
#include "string.h"

#define XDTIME 60
#define PI 3.14

void ScanKey();
void ShowCurrentPage();
void ledCon();

#endif