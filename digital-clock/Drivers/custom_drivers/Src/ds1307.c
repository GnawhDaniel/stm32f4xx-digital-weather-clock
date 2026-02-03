/*
 * ds1307.c
 *
 *  Created on: Jan 7, 2026
 *      Author: danie
 */
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "ds1307.h"


static uint8_t ds1307_read(I2C_HandleTypeDef* hi2cx, uint8_t reg_address);
static void ds1307_write(I2C_HandleTypeDef* hi2cx, uint8_t value, uint8_t reg_address);
static uint8_t binary_to_bcd(uint8_t value);
static uint8_t bcd_to_binary(uint8_t value);


// Returns 1 : CH = 1; init failed
// Returns 0 : CH = 0; init success
uint8_t ds1307_init(I2C_HandleTypeDef* hi2cx)
{
	// Make clock halt = 0
	uint8_t sec = ds1307_read(hi2cx, DS1307_ADDR_SEC);
	sec &= ~(1 << 7);   // Clear CH

	ds1307_write(hi2cx, sec, DS1307_ADDR_SEC);

	// Read Back Clock Halt Bit
	uint8_t clock_state = ds1307_read(hi2cx, DS1307_ADDR_SEC);

	return (clock_state >> 7) & 0x1;
}


void ds1307_set_current_time(I2C_HandleTypeDef* hi2cx, RTC_Time_t *rtc_time)
{
	uint8_t seconds, hrs;
	seconds = binary_to_bcd(rtc_time->seconds);
	seconds &= ~(1 << 7);
	ds1307_write(hi2cx, seconds, DS1307_ADDR_SEC);

	ds1307_write(hi2cx, binary_to_bcd(rtc_time->minutes), DS1307_ADDR_MIN);

	hrs = binary_to_bcd(rtc_time->hours);
	if(rtc_time->time_format == TIME_FORMAT_24HRS)
	{
		hrs &= ~(1 << 6);
	}
	else
	{
		hrs |= (1 << 6);
		hrs = (rtc_time->time_format == TIME_FORMAT_12HRS_PM) ? hrs | (1 << 5) : hrs & ~(1 << 5);
	}
	ds1307_write(hi2cx, hrs, DS1307_ADDR_HOURS);

}


void ds1307_get_current_time(I2C_HandleTypeDef* hi2cx, RTC_Time_t *rtc_time)
{
	uint8_t seconds, hrs;
	seconds = ds1307_read(hi2cx,DS1307_ADDR_SEC);
	seconds &= ~(1 << 7); // Clear 7th bit
	rtc_time->seconds = bcd_to_binary(seconds);
	rtc_time->minutes = bcd_to_binary(ds1307_read(hi2cx, DS1307_ADDR_MIN));

	hrs = ds1307_read(hi2cx, DS1307_ADDR_HOURS);
	if(hrs & (1 << 6))
	{
		// 12 hour format
		rtc_time->time_format = !((hrs & (1 << 5)) == 0);
		hrs &= ~(0x3 << 5); // clear 5th and 6th position
	}
	else
	{
		// 24 hour format
		rtc_time->time_format = TIME_FORMAT_24HRS;
	}
	rtc_time->hours = bcd_to_binary(hrs);
}


void ds1307_set_current_date(I2C_HandleTypeDef* hi2cx, RTC_Date_t *rtc_date)
{

	ds1307_write(hi2cx, binary_to_bcd(rtc_date->date), DS1307_ADDR_DATE);
	ds1307_write(hi2cx, binary_to_bcd(rtc_date->month), DS1307_ADDR_MONTH);
	ds1307_write(hi2cx, binary_to_bcd(rtc_date->year), DS1307_ADDR_YEAR);
	ds1307_write(hi2cx, binary_to_bcd(rtc_date->day), DS1307_ADDR_DAY);
}


void ds1307_get_current_date(I2C_HandleTypeDef* hi2cx, RTC_Date_t *rtc_date)
{
	rtc_date->date = bcd_to_binary(ds1307_read(hi2cx, DS1307_ADDR_DATE));
	rtc_date->month = bcd_to_binary(ds1307_read(hi2cx, DS1307_ADDR_MONTH));
	rtc_date->year = bcd_to_binary(ds1307_read(hi2cx, DS1307_ADDR_YEAR));
	rtc_date->day = bcd_to_binary(ds1307_read(hi2cx, DS1307_ADDR_DAY));
}

static void ds1307_write(I2C_HandleTypeDef* hi2cx, uint8_t value, uint8_t reg_address)
{
	HAL_I2C_Mem_Write(
		hi2cx,
	    DS1307_I2C_ADDRESS,
	    reg_address,
	    I2C_MEMADD_SIZE_8BIT,
	    &value,
	    1,
	    1000
	);
}


static uint8_t ds1307_read(I2C_HandleTypeDef* hi2cx, uint8_t reg_address)
{
	uint8_t rx;
	HAL_I2C_Mem_Read(
		hi2cx,
	    DS1307_I2C_ADDRESS,
	    reg_address,
	    I2C_MEMADD_SIZE_8BIT,
	    &rx,
	    1,
	    1000
	);
	return rx;
}


static uint8_t binary_to_bcd(uint8_t value)
{
	uint8_t m, n;
	uint8_t bcd;
	bcd = value;
	if(value >= 10)
	{
		m = value / 10;
		n = value % 10;
		bcd = (uint8_t) ((m << 4) | n);
	}

	return bcd;
}


static uint8_t bcd_to_binary(uint8_t value)
{
	return (uint8_t)(((value >> 4) * 10) + (value & (uint8_t)0x0F));
}

