# Digital Clock based on STM32F411xE

Digital clock system built on an STM32 Nucleo-F411xE (Arm® Cortex-M4) platform that synchronizes time over Wi-Fi using an ESP-01S module and displays it on a 16×2 LCD.

The system retrieves weather, date and time data from a web API over a TCP connection using ESP-01S AT commands. Custom STM32 drivers were implemented to transmit and parse AT commands over UART. A dedicated 3.3V linear voltage regulator powers the ESP-01S from the MCU board’s 5V supply.

A DHT20 retrieves temperature and humidity data within the space the embedded system is in.

A DS1307 real-time clock (RTC) with a backup CMOS battery maintains accurate timekeeping when the microcontroller is powered off.

## Demonstration
https://github.com/user-attachments/assets/f1194003-7ca3-4a72-b94b-5bdfc165cecd



## Images
<p align="center">
  <img src="imgs/front.jpg" width="48%" />
  <img src="imgs/top.jpg" width="48%" />
</p>


## Components
- STM32 Nucleo-F411RE
- 16x2 LCD
- B10K Potentiometer
- Linear Voltage Regulator (5.5V -> 3.3V)
- ESP-01S Wi-Fi Module
- DS1307 RTC Module (with CMOS battery)
- DHT20

## Wiring
| From      | Pin  | To       | Pin | Signal |
| --------- | ---- | -------- | --- | ------ | 
| STM32F4   | PB0  | 16x2 LCD | RS  | GPIO   |
| STM32F4   | PB1  | 16x2 LCD | RW  | GPIO   |
| STM32F4   | PB2  | 16x2 LCD | EN  | GPIO   |
| STM32F4   | PB3  | 16x2 LCD | D4  | GPIO   |
| STM32F4   | PB4  | 16x2 LCD | D5  | GPIO   |
| STM32F4   | PB5  | 16x2 LCD | D6  | GPIO   |
| STM32F4   | PB6  | 16x2 LCD | D7  | GPIO   |
| STM32F4   | PB8  | DS1307   | SCL | I2C    |
| STM32F4   | PB9  | DS1307   | SDA | I2C    |
| STM32F4   | PB8  | DHT20    | SCL | I2C    |
| STM32F4   | PB9  | DHT20    | SDA | I2C    |
| STM32F4   | PC6  | ESP-01S  | RX  | UART   |
| STM32F4   | PC7  | ESP-01S  | TX  | UART   |
| STM32F4   | 5V   | Regulator| Vin | Power  |
| STM32F4   | 3.3V | DHT20    | Vin | Power  |
| STM32F4   | 3.3V | DS1307   | Vin | Power  |
| Regulator | 3.3V | ESP-01S  | VCC | Power  |
| Potentiometer | Vin | STM32F4  | 5V | Power |
| Potentiometer | Wiper | 16x2 LCD | V0 | Contrast |
| STM32F4   |  GND   | All Modules | GND | Ground |


## Interfaces
- UART: ESP-01S Wi-Fi module (AT command interface)
- I2C: DS1307 RTC & DHT20
- GPIO: 16×2 LCD (4-bit mode)

## Design Notes
- Time synchronization & regional weather data retrieval is user-initiated via an external interrupt from the on-board button.
- Persistence of timekeeping is maintained independently of network operations.
- LCD screen operates in 4-bit mode.
