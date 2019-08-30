/*
 * DS3231.c
 *
 *  Created on: 28 sie 2019
 *      Author: Mateusz Salamon
 */

#include "main.h"
#include "DS3231.h"

I2C_HandleTypeDef *hi2c_ds3231;

void DS3231_SetControlRegister(uint8_t Value)
{
	HAL_I2C_Mem_Write(hi2c_ds3231, DS3231_ADDRESS, DS3231_REG_CONTROL, 1, &Value, 1, DS3231_I2C_TIMEOUT);
}

void DS3231_GetControlRegister(uint8_t *Value)
{
	HAL_I2C_Mem_Read(hi2c_ds3231, DS3231_ADDRESS, DS3231_REG_CONTROL, 1, Value, 1, DS3231_I2C_TIMEOUT);
}

void DS3231_GetControlStatusRegister(uint8_t *Value)
{
	HAL_I2C_Mem_Read(hi2c_ds3231, DS3231_ADDRESS, DS3231_REG_STATUS, 1, Value, 1, DS3231_I2C_TIMEOUT);
}

void DS3231_SetControlStatusRegister(uint8_t Value)
{
	HAL_I2C_Mem_Write(hi2c_ds3231, DS3231_ADDRESS, DS3231_REG_STATUS, 1, &Value, 1, DS3231_I2C_TIMEOUT);
}

void WriteBitToControlRegister(uint8_t BitNumber, uint8_t Value)
{
	uint8_t tmp;

	if(Value>1) Value = 1;

	DS3231_GetControlRegister(&tmp);
	tmp &= ~(1<<BitNumber);
	tmp |= (Value<<BitNumber);
	DS3231_SetControlRegister(tmp);
}

void DS3231_EnableOscillatorEOSC(uint8_t Enable)
{
	WriteBitToControlRegister(DS3231_CONTROL_ENABLE_OSCILLATOR_BIT, Enable);
}

void DS3231_EnableBaterryBackedSQW(uint8_t Enable)
{
	WriteBitToControlRegister(DS3231_CONTROL_BBSQW_BIT, Enable);
}

void DS3231_EnableConvertTemperature(uint8_t Enable)
{
	WriteBitToControlRegister(DS3231_CONTROL_CONVERT_TEMPERATIRE_BIT, Enable);
}

void DS3231_SQWRateSelect(SQW_Rate Rate)
{
	uint8_t tmp;

	if(Rate>3) Rate = 3;

	DS3231_GetControlRegister(&tmp);
	tmp &= ~(3<<DS3231_CONTROL_RATE_SELECT_BIT);
	tmp |= (Rate<<DS3231_CONTROL_RATE_SELECT_BIT);
	DS3231_SetControlRegister(tmp);
}

void DS3231_EnableInterrupt(uint8_t Enable)
{
	WriteBitToControlRegister(DS3231_CONTROL_INTERRUPT_CONTROL_BIT, Enable);
}

void DS3231_EnableAlarm1Interrupt(uint8_t Enable)
{
	WriteBitToControlRegister(DS3231_CONTROL_ALARM1_INTERRUPT_EN_BIT, Enable);
}

void DS3231_EnableAlarm2Interrupt(uint8_t Enable)
{
	WriteBitToControlRegister(DS3231_CONTROL_ALARM2_INTERRUPT_EN_BIT, Enable);
}

void DS3231_TurnOnOscillator(uint8_t OnOff, uint8_t OnOffBattery, SQW_Rate Frequency)
{
	if(OnOffBattery)
	{
		DS3231_EnableBaterryBackedSQW(1);
	}
	else
	{
		DS3231_EnableBaterryBackedSQW(0);
	}

	if(OnOff)
	{
		DS3231_EnableOscillatorEOSC(0);
		DS3231_EnableInterrupt(0);
	}
	else
	{
		DS3231_EnableOscillatorEOSC(1);
	}

	DS3231_SQWRateSelect(Frequency);
}

void DS3231_Enable32kHzOutput(uint8_t Enable)
{
	uint8_t tmp;

	if(Enable>1) Enable = 1;

	DS3231_GetControlStatusRegister(&tmp);
	tmp &= ~(1<<DS3231_STATUS_ENABLE_32KHZ);
	tmp |= (Enable<<DS3231_STATUS_ENABLE_32KHZ);
	DS3231_SetControlStatusRegister(tmp);
}

uint8_t bcd2dec(uint8_t BCD)
{
	return (((BCD & 0xF0)>>4) *10) + (BCD & 0xF);
}

uint8_t dec2bcd(uint8_t DEC)
{
	return ((DEC / 10)<<4) + (DEC % 10);
}

int dayofweek(int Day, int Month, int Year)
{
    int Y, C, M, N, D;
    M = 1 + (9 + Month) % 12;
    Y = Year - (M > 10);
    C = Y / 100;
    D = Y % 100;
    N = ((13 * M - 1) / 5 + D + D / 4 + 6 * C + Day + 5) % 7;
    return (7 + N) % 7;
}

void DS3231_GetDateTime(RTCDateTime *DateTime)
{
	uint8_t tmp[7];

	HAL_I2C_Mem_Read(hi2c_ds3231, DS3231_ADDRESS, DS3231_REG_TIME, 1, tmp, 7, DS3231_I2C_TIMEOUT);

	DateTime->Second = bcd2dec(tmp[0]);
	DateTime->Minute = bcd2dec(tmp[1]);
	DateTime->Hour = bcd2dec(tmp[2] & 0x3F);
	DateTime->DayOfWeek = tmp[3];
	DateTime->Day = bcd2dec(tmp[4]);
	DateTime->Month = bcd2dec(tmp[5] & 0x1F);
	DateTime->Year = 2000 + bcd2dec(tmp[6]);
}

void DS3231_SetDateTime(RTCDateTime *DateTime)
{
	uint8_t tmp[7];

	if(DateTime->Second > 59) DateTime->Second = 59;
	if(DateTime->Minute > 59) DateTime->Minute = 59;
	if(DateTime->Hour > 23) DateTime->Hour = 23;
	if(DateTime->Day > 31) DateTime->Day = 31;
	if(DateTime->Month > 12) DateTime->Month = 12;
	if(DateTime->Year> 2099) DateTime->Year = 2099;

	tmp[0] = dec2bcd(DateTime->Second);
	tmp[1] = dec2bcd(DateTime->Minute);
	tmp[2] = dec2bcd(DateTime->Hour);
	tmp[3] = dayofweek(DateTime->Day, DateTime->Month, DateTime->Year) + 1;
	tmp[4] = dec2bcd(DateTime->Day);
	tmp[5] = dec2bcd(DateTime->Month);
	tmp[6] = dec2bcd(DateTime->Year - 2000);

	HAL_I2C_Mem_Write(hi2c_ds3231, DS3231_ADDRESS, DS3231_REG_TIME, 1, tmp, 7, DS3231_I2C_TIMEOUT);
}

void DS3231_Init(I2C_HandleTypeDef *hi2c)
{
	hi2c_ds3231 = hi2c;

	DS3231_TurnOnOscillator(1, 1, SQW_RATE_1HZ); // Turn on 1Hz SQW for interrupt
	DS3231_Enable32kHzOutput(0);				 // Turn off 32kHz oscillator output
}
