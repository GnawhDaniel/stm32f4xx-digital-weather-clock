/*
 * ds1307.h
 *
 *  Created on: Jan 7, 2026
 *      Author: danie
 */

#ifndef DS1307_H_
#define DS1307_H_

#include "stm32f4xx_hal.h"

// Application Configurable Items
// Register Address
#define DS1307_ADDR_SEC  	0x00
#define DS1307_ADDR_MIN  	0x01
#define DS1307_ADDR_HOURS  	0x02
#define DS1307_ADDR_DAY	 	0x03
#define DS1307_ADDR_DATE  	0x04
#define DS1307_ADDR_MONTH	0x05
#define DS1307_ADDR_YEAR	0x06

#define TIME_FORMAT_12HRS_AM 0
#define TIME_FORMAT_12HRS_PM 1
#define TIME_FORMAT_24HRS	 2

#define DS1307_I2C_ADDRESS	(0b01101000 << 1) //0x68

#define SUNDAY		1
#define MONDAY		2
#define TUESDAY		3
#define WEDNESDAY	4
#define THURSDAY	5
#define FRIDAY		6
#define SATURDAY	7

typedef struct
{
	uint8_t day; // Weekday
	uint8_t month;
	uint8_t year;
	uint8_t date; // 1-31
}RTC_Date_t;


typedef struct
{
	uint8_t seconds;
	uint8_t minutes;
	uint8_t hours;
	uint8_t time_format;
}RTC_Time_t;


// Functions
uint8_t ds1307_init(I2C_HandleTypeDef* hi2cx);
void ds1307_set_current_time(I2C_HandleTypeDef* hi2cx, RTC_Time_t *rtc_time);
void ds1307_get_current_time(I2C_HandleTypeDef* hi2cx, RTC_Time_t *rtc_time);

void ds1307_set_current_date(I2C_HandleTypeDef* hi2cx, RTC_Date_t *rtc_date);
void ds1307_get_current_date(I2C_HandleTypeDef* hi2cx, RTC_Date_t *rtc_date);



#endif /* DS1307_H_ */
