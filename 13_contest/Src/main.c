/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "lcd_hal.h"
#include "stdio.h"
#include "string.h"
#include "interrupt.h"

extern struct keys key[5];
extern char re_date[30];
extern unsigned char rx_str;
extern uint8_t rx_date;

extern uint16_t led_tt;
extern uint16_t led_ref;
extern uint8_t led_5;

uint8_t LCD_show = 0;

char lcd_txt[30];
struct bpsd {
    unsigned char bpsd_1;
    unsigned char bpsd_2;
};

bool password_right = 0;

char password_text[4]   = {'0', '0', '0', '\0'};
char password[4]        = {'1', '2', '3', '\0'};
char password_incorrect = 0;
char incorrect_ref      = 0;

struct bpsd psd[3] = {0, 0};

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void LCD_Display()
{
    if (LCD_show == 0) {
        sprintf(lcd_txt, "       PSD          ");
        LCD_DisplayStringLine(Line1, (uint8_t *)lcd_txt);
        if (psd[0].bpsd_1 == 0) {
            sprintf(lcd_txt, "    B1:@           ");
            LCD_DisplayStringLine(Line3, (uint8_t *)lcd_txt);
        }
        if (psd[1].bpsd_1 == 0) {
            sprintf(lcd_txt, "    B2:@           ");
            LCD_DisplayStringLine(Line4, (uint8_t *)lcd_txt);
        }
        if (psd[2].bpsd_1 == 0) {
            sprintf(lcd_txt, "    B3:@           ");
            LCD_DisplayStringLine(Line5, (uint8_t *)lcd_txt);
        }
        if (psd[0].bpsd_1 != 0) {
            sprintf(lcd_txt, "    B1:%c           ", psd[0].bpsd_1);
            LCD_DisplayStringLine(Line3, (uint8_t *)lcd_txt);
        }
        if (psd[1].bpsd_1 != 0) {
            sprintf(lcd_txt, "    B2:%c           ", psd[1].bpsd_1);
            LCD_DisplayStringLine(Line4, (uint8_t *)lcd_txt);
        }
        if (psd[2].bpsd_1 != 0) {
            sprintf(lcd_txt, "    B3:%c           ", psd[2].bpsd_1);
            LCD_DisplayStringLine(Line5, (uint8_t *)lcd_txt);
        }
    } else if (LCD_show == 1) {
        sprintf(lcd_txt, "       STA          ");
        LCD_DisplayStringLine(Line1, (uint8_t *)lcd_txt);
        sprintf(lcd_txt, "    F:2000HZ        ");
        LCD_DisplayStringLine(Line3, (uint8_t *)lcd_txt);
        sprintf(lcd_txt, "    F:10%%          ");
        LCD_DisplayStringLine(Line4, (uint8_t *)lcd_txt);
        sprintf(lcd_txt, "                    ");
        LCD_DisplayStringLine(Line5, (uint8_t *)lcd_txt);
    }
}
void pds_real()
{
    for (int i = 0; i < 3; i++) {
        switch (psd[i].bpsd_2) {
            case 1:
                psd[i].bpsd_1 = '0';
                break;
            case 2:
                psd[i].bpsd_1 = '1';
                break;
            case 3:
                psd[i].bpsd_1 = '2';
                break;
            case 4:
                psd[i].bpsd_1 = '3';
                break;
            case 5:
                psd[i].bpsd_1 = '4';
                break;
            case 6:
                psd[i].bpsd_1 = '5';
                break;
            case 7:
                psd[i].bpsd_1 = '6';
                break;
            case 8:
                psd[i].bpsd_1 = '7';
                break;
            case 9:
                psd[i].bpsd_1 = '8';
                break;
            case 10:
                psd[i].bpsd_1 = '9';
                break;
        }
    }
}
void scan_real()
{
    if (key[0].sing_flag == 1) {
        if (LCD_show == 0) {
            psd[0].bpsd_2++;
            if (psd[0].bpsd_2 == 11) psd[0].bpsd_2 = 1;
        }
        key[0].sing_flag = 0;
    }
    if (key[1].sing_flag == 1) {
        if (LCD_show == 0) {
            psd[1].bpsd_2++;
            if (psd[1].bpsd_2 == 11) psd[1].bpsd_2 = 1;
        }
        key[1].sing_flag = 0;
    }
    if (key[2].sing_flag == 1) {
        if (LCD_show == 0) {
            psd[2].bpsd_2++;
            if (psd[2].bpsd_2 == 11) psd[2].bpsd_2 = 1;
        }
        key[2].sing_flag = 0;
    }
    if (key[3].sing_flag == 1) {
        if ((psd[0].bpsd_1 == password[0]) && (psd[1].bpsd_1 == password[1]) && (psd[2].bpsd_1 == password[2])) {
            password_right = 1;
            led_5          = 0;
            led_ref        = 0;
            if (++LCD_show == 2) {
                LCD_show       = 0;
                password_right = 0;
                psd[0].bpsd_1  = 0;
                psd[0].bpsd_2  = 0;
                psd[1].bpsd_1  = 0;
                psd[1].bpsd_2  = 0;
                psd[2].bpsd_1  = 0;
                psd[2].bpsd_2  = 0;
            }
        } else if ((psd[0].bpsd_1 != password[0]) || (psd[1].bpsd_1 != password[1]) || (psd[2].bpsd_1 != password[2])) {
            psd[0].bpsd_1 = 0;
            psd[0].bpsd_2 = 0;
            psd[1].bpsd_1 = 0;
            psd[1].bpsd_2 = 0;
            psd[2].bpsd_1 = 0;
            psd[2].bpsd_2 = 0;
            password_incorrect++;
            if (password_incorrect == 3) {
                password_incorrect = 0;
                incorrect_ref      = 1;
                led_ref            = 0;
            }
        }
        key[3].sing_flag = 0;
    }
}

void Receive_real()
{
    if (rx_str > 0) {
        if (rx_str == 7) {
            sscanf(re_date, "%3s", password_text);
            if ((password_text[0] == password[0]) && (password_text[1] == password[1]) && (password_text[2] == password[2])) {
                sscanf(re_date, "%*[^-]-%s", password);
                sprintf(lcd_txt, "change password successful\r\n");
                HAL_UART_Transmit(&huart1, (uint8_t *)lcd_txt, strlen(lcd_txt), 50);
            } else if ((password_text[0] != password[0]) || (password_text[1] != password[1]) || (password_text[2] != password[2])) {
                sprintf(lcd_txt, "password Error\r\n");
                HAL_UART_Transmit(&huart1, (uint8_t *)lcd_txt, strlen(lcd_txt), 50);
            }
        } else {
            sprintf(lcd_txt, "Error!!!\r\n");
            HAL_UART_Transmit(&huart1, (uint8_t *)lcd_txt, strlen(lcd_txt), 50);
        }
        rx_str = 0;
        memset(re_date, 0, 30);
    }
}
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
    /* USER CODE BEGIN 1 */

    /* USER CODE END 1 */

    /* MCU Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* USER CODE BEGIN Init */

    /* USER CODE END Init */

    /* Configure the system clock */
    SystemClock_Config();

    /* USER CODE BEGIN SysInit */

    /* USER CODE END SysInit */

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_USART1_UART_Init();
    MX_TIM2_Init();
    MX_TIM4_Init();
    /* USER CODE BEGIN 2 */
    HAL_TIM_Base_Start_IT(&htim4);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
    __HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_2, 500);

    LED_real(0x01);

    LCD_Init();
    LCD_Clear(Black);
    LCD_SetTextColor(White);
    LCD_SetBackColor(Black);
    HAL_UART_Receive_IT(&huart1, &rx_date, 1);

    //	sscanf((char*)123,"%3s",password);
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1) {
        LCD_Display();
        pds_real();
        scan_real();
        if (rx_str != 0) {
            int temp = rx_date;
            HAL_Delay(1);
            if (temp == rx_date) Receive_real();
        }
        if (incorrect_ref == 1) {
            if (led_tt <= 10) {
                LED_real(0x02);
            } else if (led_tt > 10) {
                LED_real(0x00);
            }
            if (led_5 == 1) {
                incorrect_ref = 0;
                LED_real(0x00);
                led_5 = 0;
            }
        }
        if (password_right == 1) {
            LED_real(0x01);
            __HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_2, 50);
            if (led_5 == 1) {
                password_right = 0;
                LED_real(0x00);
                led_5         = 0;
                LCD_show      = 0;
                psd[0].bpsd_1 = 0;
                psd[0].bpsd_2 = 0;
                psd[1].bpsd_1 = 0;
                psd[1].bpsd_2 = 0;
                psd[2].bpsd_1 = 0;
                psd[2].bpsd_2 = 0;
            }
        } else if (incorrect_ref != 2) {
            __HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_2, 500);
        }
        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
    }
    /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /** Configure the main internal regulator output voltage
     */
    HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

    /** Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState       = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState   = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource  = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM       = RCC_PLLM_DIV3;
    RCC_OscInitStruct.PLL.PLLN       = 20;
    RCC_OscInitStruct.PLL.PLLP       = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ       = RCC_PLLQ_DIV2;
    RCC_OscInitStruct.PLL.PLLR       = RCC_PLLR_DIV2;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
        Error_Handler();
    }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */

    /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
       tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
