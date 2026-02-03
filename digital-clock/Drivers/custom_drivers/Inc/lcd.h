/*
 * lcd.h
 *
 *  Created on: Jan 8, 2026
 *      Author: danie
 */

#ifndef LCD_H_
#define LCD_H_

#include "stm32f4xx_hal.h"

void lcd_init();
void lcd_send_command(uint8_t cmd);
void lcd_print_char(uint8_t data);
void lcd_display_clear();
void lcd_print_string(char* message);
void lcd_display_return_home();
void lcd_set_cursor(uint8_t row, uint8_t col);


#define LCD_GPIO_PORT	GPIOB
#define LCD_GPIO_RS		GPIO_PIN_0
#define LCD_GPIO_RW		GPIO_PIN_1
#define LCD_GPIO_EN		GPIO_PIN_2
#define LCD_GPIO_D4		GPIO_PIN_3
#define LCD_GPIO_D5		GPIO_PIN_4
#define LCD_GPIO_D6		GPIO_PIN_5
#define LCD_GPIO_D7		GPIO_PIN_6


#define LCD_CMD_4DL_2N_5x8F		0x28
#define LCD_CMD_DON_CURON		0x0E
#define LCD_CMD_DON_CUROFF		0x0C
#define LCD_CMD_INCADD			0x06
#define LCD_CMD_DIS_CLEAR		0x01
#define LCD_CMD_DIS_RETURN_HOME	0x02

#endif /* LCD_H_ */
