/*
 * lcd.c
 *
 *  Created on: Jan 7, 2026
 *      Author: danie
 */
#include "lcd.h"


static void write_4_bits(uint8_t value);
static void lcd_enable();
extern void delay_us(uint32_t us);



void lcd_send_command(uint8_t cmd)
{
	HAL_GPIO_WritePin(LCD_GPIO_PORT, LCD_GPIO_RS, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_GPIO_PORT, LCD_GPIO_RW, GPIO_PIN_RESET);

	write_4_bits(cmd >> 4);   // higher nibble

	write_4_bits(cmd & 0x0F); // lower nibble
}


void lcd_print_char(uint8_t data)
{
	HAL_GPIO_WritePin(LCD_GPIO_PORT, LCD_GPIO_RS, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_GPIO_PORT, LCD_GPIO_RW, GPIO_PIN_RESET);

	write_4_bits(data >> 4);  // higher nibble
	write_4_bits(data & 0x0F); // lower nibble
}


void lcd_print_string(char *message)
{
	do
	{
		lcd_print_char((uint8_t)*message++);
	}
	while(*message != '\0');
}


void lcd_display_clear()
{
	lcd_send_command(LCD_CMD_DIS_CLEAR);
	HAL_Delay(2);
}


// Cursor returns to home position
void lcd_display_return_home()
{
	lcd_send_command(LCD_CMD_DIS_RETURN_HOME);
	HAL_Delay(2);
}


void lcd_set_cursor(uint8_t row, uint8_t col)
{
	col--;
	switch(row)
	{
		case 1:
			// Set cursor to 1st row address and add index
			lcd_send_command((col |= 0x80));
			break;
		case 2:
			// Set cursor to 2nd row address and add index
			lcd_send_command((col |= 0xC0));
			break;
		default:
			break;
	}
}


void lcd_init()
{
	__HAL_RCC_GPIOB_CLK_ENABLE();

	// Configure GPIO Pins used for LCD Connections
	GPIO_InitTypeDef init_scruct;

	init_scruct.Mode = GPIO_MODE_OUTPUT_PP;
	init_scruct.Pull  = GPIO_NOPULL;
	init_scruct.Speed = GPIO_SPEED_FREQ_HIGH;
	init_scruct.Pin = LCD_GPIO_RS;

	// Enable Peri Clock
	HAL_GPIO_Init(LCD_GPIO_PORT, &init_scruct);

	init_scruct.Pin  = LCD_GPIO_RW;
	HAL_GPIO_Init(LCD_GPIO_PORT, &init_scruct);
	init_scruct.Pin  = LCD_GPIO_EN;
	HAL_GPIO_Init(LCD_GPIO_PORT, &init_scruct);
	init_scruct.Pin  = LCD_GPIO_D4;
	HAL_GPIO_Init(LCD_GPIO_PORT, &init_scruct);
	init_scruct.Pin = LCD_GPIO_D5;
	HAL_GPIO_Init(LCD_GPIO_PORT, &init_scruct);
	init_scruct.Pin  = LCD_GPIO_D6;
	HAL_GPIO_Init(LCD_GPIO_PORT, &init_scruct);
	init_scruct.Pin  = LCD_GPIO_D7;
	HAL_GPIO_Init(LCD_GPIO_PORT, &init_scruct);

	HAL_GPIO_WritePin(LCD_GPIO_PORT, LCD_GPIO_RS, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_GPIO_PORT, LCD_GPIO_RW, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_GPIO_PORT, LCD_GPIO_EN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_GPIO_PORT, LCD_GPIO_D4, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_GPIO_PORT, LCD_GPIO_D5, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_GPIO_PORT, LCD_GPIO_D6, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_GPIO_PORT, LCD_GPIO_D7, GPIO_PIN_RESET);


	// LCD Initialization
	HAL_Delay(40);

	// RS = 0; For LCD Command
	HAL_GPIO_WritePin(LCD_GPIO_PORT, LCD_GPIO_RS, GPIO_PIN_RESET);
	// RW = 0
	HAL_GPIO_WritePin(LCD_GPIO_PORT, LCD_GPIO_RW, GPIO_PIN_RESET);

	write_4_bits(0b0011);
	HAL_Delay(5);
	write_4_bits(0b0011);
	delay_us(150);
	write_4_bits(0b0011);
	write_4_bits(0b0010);

	// function set command
	lcd_send_command(LCD_CMD_4DL_2N_5x8F);

	// display on and cursor off
	lcd_send_command(LCD_CMD_DON_CUROFF);

	// display clear
	lcd_display_clear();

	// entry mode set
	lcd_send_command(LCD_CMD_INCADD);
}


// Writes 4 bits through D4-7
static void write_4_bits(uint8_t value)
{
//	GPIO_WriteToOutputPin(LCD_GPIO_PORT, LCD_GPIO_D4, (value >> 0) & 0x1);
//	GPIO_WriteToOutputPin(LCD_GPIO_PORT, LCD_GPIO_D5, (value >> 1) & 0x1);
//	GPIO_WriteToOutputPin(LCD_GPIO_PORT, LCD_GPIO_D6, (value >> 2) & 0x1);
//	GPIO_WriteToOutputPin(LCD_GPIO_PORT, LCD_GPIO_D7, (value >> 3) & 0x1);

	HAL_GPIO_WritePin(LCD_GPIO_PORT, LCD_GPIO_D4, (value >> 0) & 0x1);
	HAL_GPIO_WritePin(LCD_GPIO_PORT, LCD_GPIO_D5, (value >> 1) & 0x1);
	HAL_GPIO_WritePin(LCD_GPIO_PORT, LCD_GPIO_D6, (value >> 2) & 0x1);
	HAL_GPIO_WritePin(LCD_GPIO_PORT, LCD_GPIO_D7, (value >> 3) & 0x1);

	lcd_enable();
}


static void lcd_enable()
{
	HAL_GPIO_WritePin(LCD_GPIO_PORT, LCD_GPIO_EN, GPIO_PIN_SET);
	delay_us(10);
	HAL_GPIO_WritePin(LCD_GPIO_PORT, LCD_GPIO_EN, GPIO_PIN_RESET);
	delay_us(100);
}
