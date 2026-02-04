/*
 * esp8266_driver.c
 *
 *  Created on: Jan 13, 2026
 *      Author: danie
 */
#include "esp8266ex_driver.h"


/*
@brief Converts n characters of a numeric string to uint16_t
*/
static uint16_t atoi_n(const char *s, uint8_t n)
{
    uint16_t val = 0;
    while (n--) {
        val = val * 10 + (*s++ - '0');
    }
    return val;
}


/*
@brief Helper function to parse datetime string in the format "YYYY-MM-DD HH:MM:SS"
*/
void parse_datetime(const char *dt,
                    uint8_t *year,
                    uint8_t *month,
                    uint8_t *date,
                    uint8_t *hour,
                    uint8_t *minute,
                    uint8_t *second)
{
    *year   = (uint8_t)atoi_n(&dt[2], 2);  // "2026" → "26"
    *month  = (uint8_t)atoi_n(&dt[5], 2);
    *date    = (uint8_t)atoi_n(&dt[8], 2);
    *hour   = (uint8_t)atoi_n(&dt[11], 2);
    *minute = (uint8_t)atoi_n(&dt[14], 2);
    *second = (uint8_t)atoi_n(&dt[17], 2);
}


/*
* @brief - Parses the HTTP response from the time server and extracts the datetime information
*
* @param response - The HTTP response string
*
* @return - Clock structure with parsed date and time
*/
static Clock parse_time_response(char* response)
{
	Clock clock = {0};
    char datetime[32] = {0};

    char weekday;
    uint8_t month, date, hour, minute, second, year;

		// Find beginning of JSON payload
    char* json_start = strchr(response, '{');

    char* dt = strstr(json_start, "\"datetime\": \"");
    if (!dt) return clock;

    dt += strlen("\"datetime\": \"");
    memcpy(datetime, dt, 19);
    datetime[19] = '\0';

    parse_datetime(
    		datetime,
    		&year,
			&month,
			&date,
			&hour,
			&minute,
			&second);

    clock.date.year = year;
    clock.date.date = date;
    clock.date.month = month;
    clock.time.hours = hour;
    clock.time.seconds = second;
    clock.time.minutes = minute;
    clock.time.time_format = TIME_FORMAT_24HRS;

    dt = strstr(json_start, "\"weekday\": ");
    dt += strlen("\"weekday\": ");
    weekday = *(dt);
    clock.date.day = (uint8_t)(weekday - '0'); // Convert character number to uint8

    return clock;
}


static Weather parse_weather_response(char* response)
{
//	printf("%s\n", response);
	Weather w = {0};

	char* json_start = strchr(response, '{');

    char* ptr_start = strstr(json_start, "\"humidity\": ");
    if (!ptr_start) return w;

    // Parsing String Response
    ptr_start += strlen("\"humidity\": ");
    char* ptr_end = strstr(ptr_start, ",");
    uint8_t sz = (uint8_t)(ptr_end - ptr_start);
    memcpy(w.humidity, ptr_start, sz);

    ptr_start = strstr(ptr_start, "\"precip_in\": ");
    ptr_start += strlen("\"precip_in\": ");
    ptr_end = strstr(ptr_start, ",");
    sz = (uint8_t)(ptr_end - ptr_start);
    memcpy(w.precip, ptr_start, sz);

    ptr_start = strstr(ptr_start, "\"temp_f\": ");
    ptr_start += strlen("\"temp_f\": ");
    ptr_end = strstr(ptr_start, "\n");
    sz = (uint8_t)(ptr_end - ptr_start);
    memcpy(w.temperature, ptr_start, sz);

	return w;
}


void esp8266ex_send_command(UART_HandleTypeDef* huart, char* cmd, char* rcv_buf, uint16_t buf_sz, uint32_t timeout)
{
//    printf("Sending: %s\n", cmd);
    HAL_UART_Transmit(huart, (uint8_t*)cmd, strlen(cmd), timeout);
    HAL_UART_Receive(huart, (uint8_t*)rcv_buf, buf_sz, timeout);
//    printf("Received: %s\n", rcv_buf);
}


void esp8266ex_list_available_aps(UART_HandleTypeDef* huart)
{
	esp8266ex_wifi_mode(huart, ESP8266EX_MODE_STATION);

	char rcv_buf[256];
	esp8266ex_send_command(huart, "AT+CWLAP\r\n", rcv_buf, sizeof(rcv_buf), 1000);
}


void esp8266ex_wifi_mode(UART_HandleTypeDef* huart, uint8_t cwmode)
{
	char* cmd;
	switch (cwmode) {
		case ESP8266EX_MODE_NULL:
			cmd = "AT+CWMODE=0\r\n";
			break;
		case ESP8266EX_MODE_STATION:
			cmd = "AT+CWMODE=1\r\n";
			break;
		case ESP8266EX_MODE_SOFTAP:
			cmd = "AT+CWMODE=2\r\n";
			break;
		case ESP8266EX_MODE_SOFTAP_STATION:
			cmd = "AT+CWMODE=3\r\n";
			break;
		default:
            cmd = "AT+CWMODE?\r\n";  // or return without sending
			break;
	}

	char rcv_buf[256];
	esp8266ex_send_command(huart, cmd, rcv_buf, sizeof(rcv_buf), 1000);
}


void esp8266ex_connect_ap(UART_HandleTypeDef* huart, char* ssid, char* password)
{
	char cmd[256];

	snprintf(cmd, sizeof(cmd),
	         "AT+CWJAP=\"%s\",\"%s\"\r\n",
	         ssid, password);

	char rcv_buf[256];
	esp8266ex_send_command(huart, cmd, rcv_buf, sizeof(rcv_buf), 2000);
}


void esp8266ex_dns_resolve(UART_HandleTypeDef* huart, char* domain_name)
{
	char cmd[256];

	snprintf(cmd, sizeof(cmd),
	         "AT+CIPDOMAIN=\"%s\"\r\n",
			 domain_name);

	char rcv_buf[256];
	esp8266ex_send_command(huart, cmd, rcv_buf, sizeof(rcv_buf), 5000);
}


void esp8266ex_cipstart(UART_HandleTypeDef* huart, char* connection_type, char* ip, char* port)
{
	char cmd[256];

	snprintf(cmd, sizeof(cmd),
	         "AT+CIPSTART=\"%s\",\"%s\",%s\r\n",
			 connection_type,ip, port);

	char rcv_buf[256];
	esp8266ex_send_command(huart, cmd, rcv_buf, sizeof(rcv_buf), 2000);
}


void esp8266ex_get_req(UART_HandleTypeDef* huart, char* query, char* rcv_buf, uint16_t buf_sz)
{
	char cmd[256];
	char ip[21];

	strcpy(ip, SERVER_IP);
	*(ip + strlen(SERVER_IP)) = ':';
	strcpy(ip + strlen(SERVER_IP) + 1, SERVER_PORT); // -> xxx.xxx.xxx.xxx:xxxxx

	uint8_t sz = 45 + strlen(query) + strlen(ip);
	snprintf(cmd, sizeof(cmd),
	         "AT+CIPSEND=%d\r\n", sz);
	esp8266ex_send_command(huart, cmd, rcv_buf, buf_sz, 2000);


	memset(cmd, 0, 256);
	snprintf(cmd, sizeof(cmd),
	         "GET /%s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n",
			 query, ip);
	memset(rcv_buf, 0, buf_sz);
	esp8266ex_send_command(huart, cmd, rcv_buf, buf_sz, 1000);
}


Clock esp8266ex_get_time(UART_HandleTypeDef* huart)
{
	char res[1028];
	esp8266ex_get_req(huart, "time", res, sizeof(res));
	return parse_time_response(res);
}


Weather esp8266ex_get_weather(UART_HandleTypeDef* huart)
{
	char res[512];
	char req[13] = "weather/";
	char req2[] = ZIP_CODE;
	strcat(req, req2);
	esp8266ex_get_req(huart, req, res, sizeof(res));
	return parse_weather_response(res);
}


void esp8266ex_firmware_version(UART_HandleTypeDef* huart)
{
	char rcv_buf[256];
	esp8266ex_send_command(huart, "AT+GMR\r\n", rcv_buf, sizeof(rcv_buf), 1000);
}


void esp8266ex_get_ip_address(UART_HandleTypeDef* huart)
{
	char rcv_buf[256];

	esp8266ex_send_command(huart, "AT+CIFSR\r\n", rcv_buf, sizeof(rcv_buf), 300);

}

