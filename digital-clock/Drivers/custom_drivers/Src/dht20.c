/*
 * dht20.c
 *
 *  Created on: Jan 30, 2026
 *      Author: danie
 */


#include "dht20.h"

extern DHT20_Handle_t DHT20;


static float C2F(float celcius)
{
	return (celcius * ((float)1.8)) + 32;
}

void DHT20_Init(I2C_HandleTypeDef* pI2Cx)
{
	HAL_Delay(100);
	uint8_t status_word = 0x71;
	HAL_I2C_Master_Transmit(pI2Cx, (DHT20_ADDRESS << 1), &status_word, 1, 10); // Write
	HAL_I2C_Master_Receive(pI2Cx, (DHT20_ADDRESS << 1), &status_word, 1, 10);

	if (status_word != 0x18)
	{
		// Something Wrong
		return;
	}

	HAL_Delay(10);
}


uint8_t DHT20_Read(I2C_HandleTypeDef* pI2Cx)
{
	if (DHT20.state == IDLE)
	{
		uint8_t cmd[3] = {0xAC,0X33, 0x00};
		HAL_I2C_Master_Transmit(pI2Cx, (DHT20_ADDRESS << 1), cmd, 3, 10);
		DHT20.state = ACTIVE;
		DHT20.timestamp = HAL_GetTick();
	}
	else if (DHT20.state == ACTIVE)
	{
		uint8_t results[6];
		uint8_t status_word;
//		HAL_Delay(80);
		if ((HAL_GetTick() - DHT20.timestamp) >= 80)
		{
			HAL_I2C_Master_Receive(pI2Cx, (DHT20_ADDRESS << 1), &status_word, 1, 10);
			if (!(status_word & 0b01000000)) // if bit[7] is 0
			{
				// Completed
				// Read 6 bytes
				HAL_I2C_Master_Receive(pI2Cx, (DHT20_ADDRESS << 1), results, 6, HAL_MAX_DELAY);
			}

			uint32_t s_rh = ((uint32_t)results[1] << 12) |
							((uint32_t)results[2] << 4)  |
							((uint32_t)results[3] >> 4);

			uint32_t s_t = (((uint32_t)results[3] & 0x0F) << 16) |
						   ((uint32_t)results[4] << 8)  |
						   ((uint32_t)results[5] << 0);

			// Convert to actual values
			float humidity = ((float)s_rh / 1048576.0) * 100.0;  // 2^20 = 1048576
			float temperature = (((float)s_t / 1048576.0) * 200.0) - 50.0;

			DHT20.temperature = C2F(temperature);
			DHT20.humidity = humidity;
			DHT20.state = IDLE;
			return 1;
		}
	}

	return 0;
}

