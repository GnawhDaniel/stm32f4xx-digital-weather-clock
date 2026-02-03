/*
 * dht20.h
 *
 *  Created on: Jan 30, 2026
 *      Author: danie
 */

#ifndef DHT20_DHT20_H_
#define DHT20_DHT20_H_

#include "stm32f4xx_hal.h"

#define DHT20_ADDRESS 	(0x38)


typedef enum
{
	IDLE,
	ACTIVE,
} DHT20_State_t;


typedef struct
{
	DHT20_State_t state;
	uint32_t timestamp;
	float temperature;
	float humidity;
} DHT20_Handle_t;


void DHT20_Init(I2C_HandleTypeDef* pI2Cx);
uint8_t DHT20_Read(I2C_HandleTypeDef* pI2Cx);


#endif /* DHT20_DHT20_H_ */
