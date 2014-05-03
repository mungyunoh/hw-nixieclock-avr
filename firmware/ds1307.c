/*! \file ds1307.c \brief DS1307, support for real-time clock for AVR */
//*****************************************************************************
//
//  File Name       : 'ds1307.c'
//  Title           : DS1307 real-time clock support for AVR
//  Original Author : Alan K. Duncan - Copyright (c) 2012
//  Edited by       : jan1s - Copyright (c) 2013
//  Created         : 2012-03-30
//  Modified        : 2013-09-01
//  Version         : 1.2
//  Target MCU      : Atmel AVR series
//
// This code is distributed under the GNU Public License
//		which can be found at http://www.gnu.org/licenses/gpl.txt
//
//*****************************************************************************


#include "i2c.h"
#include "ds1307.h"

/*  base hardware address of the device */
#define DS1307_BASE_ADDRESS 		0xD0

/*  register addresses  */
#define DS1307_SECONDS_ADDR			0x00
#define DS1307_MINUTES_ADDR			0x01
#define DS1307_HOURS_ADDR			0x02
#define DS1307_DAY_ADDR				0x03
#define DS1307_DATE_ADDR			0x04
#define DS1307_MONTH_ADDR			0x05
#define DS1307_YEAR_ADDR			0x06
#define DS1307_CONTROL_ADDR			0x07

/*  control bits    */
#define DS1307_CLOCK_HALT			0x80
#define DS1307_HOUR_MODE			0x40
#define DS1307_HOUR_PM				0x20
#define DS1307_OUTPUT_CONTROL 		0x80
#define DS1307_SQUAREWAVE_ENABLE	0x10

#define DS1307_RATE_SELECT_MASK 	0x03
#define DS1307_RATE_1HZ				0x00
#define DS1307_RATE_4096HZ			0x01
#define DS1307_RATE_8192HZ			0x02
#define DS1307_RATE_32768HZ			0x03

enum { kDS1307Mode12HR, kDS1307Mode24HR };

/*  private function prototypes     */
uint8_t ds1307ReadRegister(uint8_t reg);
void  ds1307WriteRegister(uint8_t reg, uint8_t data);

static unsigned int uint2bcd(unsigned int ival)
{
	return ((ival / 10) << 4) | (ival % 10);
}

void ds1307Init(void)
{
	i2cInit();
	ds1307EnableOscillator();
	ds1307SetHourMode(kDS1307Mode24HR);
	ds1307SetSquarewaveOutput(DS1307_SQUAREWAVE_ENABLE, DS1307_RATE_1HZ);
}

void ds1307EnableOscillator(void)
{
	/*	To start the oscillator, we need to write CH = 0 (bit 7/reg 0) */
	uint8_t seconds = ds1307ReadRegister(DS1307_SECONDS_ADDR);
	seconds &= ~DS1307_CLOCK_HALT;
	ds1307WriteRegister(DS1307_SECONDS_ADDR,seconds);
}

void ds1307DisableOscillator(void)
{
	/*	To start the oscillator, we need to write CH = 0 (bit 7/reg 0) */
	uint8_t seconds = ds1307ReadRegister(DS1307_SECONDS_ADDR);
	seconds |= DS1307_CLOCK_HALT;
	ds1307WriteRegister(DS1307_SECONDS_ADDR,seconds);
}

void ds1307SetHourMode(uint8_t mode)
{
	/*	set the mode */
	uint8_t hour = ds1307ReadRegister(DS1307_HOURS_ADDR);
	if( mode == kDS1307Mode12HR )
		hour |= DS1307_HOUR_MODE;
	else
		hour &= ~DS1307_HOUR_MODE;
	ds1307WriteRegister(DS1307_HOURS_ADDR, hour);
}

void ds1307SetSquarewaveOutput(uint8_t enable, uint8_t rate)
{
	/* set squarewave output */
	uint8_t control = 0x00;
	control |= rate;
	control |= enable;
	ds1307WriteRegister(DS1307_CONTROL_ADDR, control);
}


uint8_t ds1307GetSeconds(void)
{
	uint8_t seconds_h,seconds_l;
	uint8_t seconds = ds1307ReadRegister(DS1307_SECONDS_ADDR);
	/*	mask the CH bit */
	seconds &= ~DS1307_CLOCK_HALT;
	/*	get the rest of the high nibble */
	seconds_h = seconds >> 4;
	seconds_l = seconds & 0b00001111;
	return seconds_h * 10 + seconds_l;
}

uint8_t ds1307GetMinutes(void)
{
	uint8_t minutes_h,minutes_l;
	uint8_t minutes = ds1307ReadRegister(DS1307_MINUTES_ADDR);
	minutes_h = minutes >> 4;
	minutes_l = minutes & 0b00001111;
	return minutes_h * 10 + minutes_l;
}

uint8_t ds1307GetHours(void)
{
	uint8_t hours_h, hours_l;
	uint8_t hours = ds1307ReadRegister(DS1307_HOURS_ADDR);
	if( hours & DS1307_HOUR_MODE )
	{
		/* 12 hour mode so mask the upper three bits */
		hours &= ~(0b11100000);
	}
	else
	{
		/*	24 hour mode, so mask the two upper bits */
		hours &= ~(0b11000000);
	}
	hours_h = hours >> 4;
	hours_l = hours & 0b00001111;
	return hours_h * 10 + hours_l;
}

uint8_t ds1307GetDate(void)
{
	uint8_t date_h,date_l;
	uint8_t date = ds1307ReadRegister(DS1307_DATE_ADDR);
	/*	mask the uppermost two bits */
	date &= ~(0b11000000);
	date_h = date >> 4;
	date_l = date & 0b00001111;
	return date_h * 10 + date_l;
}

uint8_t ds1307GetMonth(void)
{
	uint8_t month_h,month_l;
	uint8_t month = ds1307ReadRegister(DS1307_MONTH_ADDR);
	/*	mask the uppermost two bits */
	month &= ~(0b11100000);
	month_h = month >> 4;
	month_l = month & 0b00001111;
	return month_h * 10 + month_l;
}

uint8_t ds1307GetYear(void)
{
	uint8_t year_h,year_l;
	uint8_t year = ds1307ReadRegister(DS1307_YEAR_ADDR);
	year_h = year >> 4;
	year_l = year & 0b00001111;
	return year_h * 10 + year_l;
}

void ds1307SetSeconds(uint8_t seconds)
{
	uint8_t bcd_seconds = uint2bcd(seconds);
	/* make sure CH bit is clear */
	bcd_seconds &= ~DS1307_CLOCK_HALT;
	ds1307WriteRegister(DS1307_SECONDS_ADDR,bcd_seconds);
}

void ds1307SetMinutes(uint8_t minutes)
{
	uint8_t bcd_minutes = uint2bcd(minutes);
	/*	make sure upper bit is clear */
	bcd_minutes &= ~(1<<7);
	ds1307WriteRegister(DS1307_MINUTES_ADDR,bcd_minutes);
}

void ds1307SetHours(uint8_t hours)
{
	uint8_t bcd_hours = uint2bcd(hours);
	uint8_t current_hours = ds1307ReadRegister(DS1307_HOURS_ADDR);
	/* check hour mode */
	if( current_hours & DS1307_HOUR_MODE )
	{
		/* 12 hour mode */
		bcd_hours &= ~(0b11100000);
		bcd_hours |= DS1307_HOUR_MODE;
		if(hours > 12) bcd_hours |= DS1307_HOUR_PM;
	}
	else
	{
		/*	24 hour mode */
		bcd_hours &= ~(0b11000000);
	}
	ds1307WriteRegister(DS1307_HOURS_ADDR,bcd_hours);
}

void ds1307SetDate(uint8_t date)
{
	uint8_t bcd_date = uint2bcd(date);
	ds1307WriteRegister(DS1307_DATE_ADDR,bcd_date);
}

void ds1307SetMonth(uint8_t month)
{
	uint8_t bcd_month = uint2bcd(month);
	ds1307WriteRegister(DS1307_MONTH_ADDR,bcd_month);
}

void ds1307SetYear(uint8_t year)
{
	uint8_t bcd_year = uint2bcd(year);
	ds1307WriteRegister(DS1307_YEAR_ADDR,bcd_year);
}


void  ds1307WriteRegister(uint8_t reg, uint8_t data)
{
	uint8_t device_data[2];
	device_data[0] = reg;
	device_data[1] = data;
	i2cMasterSend(DS1307_BASE_ADDRESS,2,device_data);
}

uint8_t ds1307ReadRegister(uint8_t reg)
{
	uint8_t device_data[2];
	device_data[0] = reg;
	i2cMasterSend(DS1307_BASE_ADDRESS,1,device_data);
	i2cMasterReceive(DS1307_BASE_ADDRESS,1,device_data);
	return device_data[0];
}