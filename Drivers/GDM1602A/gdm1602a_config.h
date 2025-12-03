#ifndef GDM1602A_CONFIG_H
#define GDM1602A_CONFIG_H

#include "main.h"

/* Configuration options */
#define LCD_USE_RW_PIN 0
#define LCD_USE_BUSY_FLAG 0
#define LCD_USE_8BIT_MODE 0

/* Data pins */
#if LCD_USE_8BIT_MODE
#define LCD_DB0_PIN LCD_DB0_Pin
#define LCD_DB0_PORT LCD_DB0_GPIO_Port
#define LCD_DB1_PIN LCD_DB1_Pin
#define LCD_DB1_PORT LCD_DB1_GPIO_Port
#define LCD_DB2_PIN LCD_DB2_Pin
#define LCD_DB2_PORT LCD_DB2_GPIO_Port
#define LCD_DB3_PIN LCD_DB3_Pin
#define LCD_DB3_PORT LCD_DB3_GPIO_Port
#endif
#define LCD_DB4_PIN LCD_DB4_Pin
#define LCD_DB4_PORT LCD_DB4_GPIO_Port
#define LCD_DB5_PIN LCD_DB5_Pin
#define LCD_DB5_PORT LCD_DB5_GPIO_Port
#define LCD_DB6_PIN LCD_DB6_Pin
#define LCD_DB6_PORT LCD_DB6_GPIO_Port
#define LCD_DB7_PIN LCD_DB7_Pin
#define LCD_DB7_PORT LCD_DB7_GPIO_Port

/* Control pins */
#define LCD_RS_PIN LCD_RS_Pin
#define LCD_RS_PORT LCD_RS_GPIO_Port
#define LCD_E_PIN LCD_E_Pin
#define LCD_E_PORT LCD_E_GPIO_Port

/* Timing [us] */
#define LCD_DELAY_INIT_1 15000
#define LCD_DELAY_INIT_2 4100
#define LCD_DELAY_INIT_3 100
#define LCD_DELAY_INS_CLEAR_HOME 2000
#define LCD_DELAY_INS 50
#define LCD_DELAY_ENABLE 1

#endif
