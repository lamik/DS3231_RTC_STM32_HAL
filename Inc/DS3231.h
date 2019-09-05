/*
 * DS3231.h
 *
 *	The MIT License.
 *  Created on: 28.08.2019
 *      Author: Mateusz Salamon
 *		 mateusz@msalamon.pl
 *
 *      Website: https://msalamon.pl/piekielnie-dokladny-rtc-ds3231-na-stm32/
 *      GitHub:  https://github.com/lamik/DS3231_RTC_STM32_HAL
 *      Contact: mateusz@msalamon.pl
 */

#ifndef DS3231_H_
#define DS3231_H_

//
//	Uncomment when you are using DMA reading
//
#define DS3231_USE_DMA

#define DS3231_ADDRESS              (0x68<<1)
#define DS3231_I2C_TIMEOUT			100

#define DS3231_REG_TIME             0x00
#define DS3231_REG_SECONDS          0x00
#define DS3231_REG_MINUTES          0x01
#define DS3231_REG_HOURS          	0x02
#define DS3231_REG_DAY              0x03
#define DS3231_REG_DATE             0x04
#define DS3231_REG_MONTH            0x05
#define DS3231_REG_YEAR             0x06
#define DS3231_REG_ALARM_1          0x07
#define DS3231_REG_ALARM_2          0x0B
#define DS3231_REG_CONTROL          0x0E
#define DS3231_REG_STATUS           0x0F
#define DS3231_REG_TEMPERATURE      0x11

//
//	Controll register 0x0E
//
#define DS3231_CONTROL_ENABLE_OSCILLATOR_BIT	7
#define DS3231_CONTROL_BBSQW_BIT				6
#define DS3231_CONTROL_CONVERT_TEMPERATIRE_BIT	5
#define DS3231_CONTROL_RATE_SELECT_BIT			3
#define DS3231_CONTROL_INTERRUPT_CONTROL_BIT	2
#define DS3231_CONTROL_ALARM2_INTERRUPT_EN_BIT	1
#define DS3231_CONTROL_ALARM1_INTERRUPT_EN_BIT	0

//
//	Controll/Status register 0x0F
//
#define DS3231_STATUS_ENABLE_32KHZ				4

typedef enum
{
	SQW_RATE_1HZ = 0,
	SQW_RATE_1024HZ = 1,
	SQW_RATE_4096HZ = 2,
	SQW_RATE_8192HZ = 3
}SQW_Rate;

typedef struct
{
	uint16_t 	Year;
	uint8_t  	Month;
	uint8_t		Day;
	uint8_t		Hour;
	uint8_t		Minute;
	uint8_t		Second;
	uint8_t		DayOfWeek;
}RTCDateTime;

typedef struct
{
	uint8_t		Day;
	uint8_t		Hour;
	uint8_t		Minute;
	uint8_t		Second;
}RTCAlarmTime;


void DS3231_EnableOscillatorEOSC(uint8_t Enable);
void DS3231_EnableBaterryBackedSQW(uint8_t Enable);
void DS3231_EnableConvertTemperature(uint8_t Enable);
void DS3231_SQWRateSelect(uint8_t Rate);
void DS3231_EnableInterrupt(uint8_t Enable);
void DS3231_EnableAlarm1Interrupt(uint8_t Enable);
void DS3231_EnableAlarm2Interrupt(uint8_t Enable);

void DS3231_TurnOnOscillator(uint8_t OnOff, uint8_t OnOffBattery, SQW_Rate Frequency);

void DS3231_Init(I2C_HandleTypeDef *hi2c);

void DS3231_SetDateTime(RTCDateTime *DateTime);
#ifdef DS3231_USE_DMA
void DS3231_ReceiveDateTimeDMA(void);	// Use in DS3231 Interrupt handler
void DS3231_CalculateDateTime(RTCDateTime *DateTime);	// Use in DMA Complete Receive interrupt
#else
void DS3231_GetDateTime(RTCDateTime *DateTime);	// Use in blocking/interrupt mode in DS3231_INT EXTI handler
#endif
#endif /* DS3231_H_ */
