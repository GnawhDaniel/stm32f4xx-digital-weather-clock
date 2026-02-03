/*
 * esp8266ex_driver.h
 *
 *  Created on: Jan 13, 2026
 *      Author: danie
 */

#ifndef CUSTOM_DRIVERS_ESP8266EX_DRIVER_H_
#define CUSTOM_DRIVERS_ESP8266EX_DRIVER_H_

#include <string.h>
#include <stdio.h>
#include "stm32f4xx_hal.h"
#include "esp8266ex_driver.h"
#include "config_private.h"
#include "ds1307.h"

typedef struct
{
	RTC_Date_t date;
	RTC_Time_t time;
} Clock;


typedef struct
{
	char humidity[8];
	char precip[8];
	char temperature[8];
} Weather;


// general
void esp8266ex_firmware_version(UART_HandleTypeDef* huart);
void esp8266ex_send_command(UART_HandleTypeDef* huart, char* cmd, char* rcv_buf, uint16_t buf_sz, uint32_t timeout);

//wifi
void esp8266ex_list_available_aps(UART_HandleTypeDef* huart);
void esp8266ex_connect_ap(UART_HandleTypeDef* huart, char* ssid, char* password);
void esp8266ex_wifi_mode(UART_HandleTypeDef* huart, uint8_t cwmode);

//tcp
void esp8266ex_dns_resolve(UART_HandleTypeDef* huart, char* domain_name);
void esp8266ex_get_ip_address(UART_HandleTypeDef* huart);
void esp8266ex_cipstart(UART_HandleTypeDef* huart, char* connection_type, char* ip, char* port);
void esp8266ex_get_req(UART_HandleTypeDef* huart, char* query, char* rcv_buf, uint16_t buf_sz);
Clock esp8266ex_get_time(UART_HandleTypeDef* huart);
Weather esp8266ex_get_weather(UART_HandleTypeDef* huart);


#define ESP8266EX_MODE_NULL				0
#define ESP8266EX_MODE_STATION			1
#define ESP8266EX_MODE_SOFTAP			2
#define ESP8266EX_MODE_SOFTAP_STATION	3

#define ESP8266EX_OPT_HEAD				1
#define ESP8266EX_OPT_GET				2
#define ESP8266EX_OPT_POST				3
#define ESP8266EX_OPT_PUT				4
#define ESP8266EX_OPT_DELETE			5

#define ESP8266EX_CONTENT_URLENCODED	0
#define ESP8266EX_CONTENT_JSON			1
#define ESP8266EX_CONTENT_FORMDATA		2
#define ESP8266EX_CONTENT_XML			3

#define ESP8266EX_TRANSPORT_TCP			"TCP"
#define ESP8266EX_TRANSPORT_UDP			"UDP"
#define ESP8266EX_TRANSPORT_SSL			"SSL"


#endif /* CUSTOM_DRIVERS_ESP8266EX_DRIVER_H_ */
