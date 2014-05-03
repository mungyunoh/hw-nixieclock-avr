/*! \file gps.c \brief GPS position storage and processing library. */
//*****************************************************************************
//
// File Name	: 'gps.c'
// Title		: GPS position storage and processing function library
// Author		: Pascal Stang - Copyright (C) 2002-2005
// Created		: 2005.01.14
// Revised		: 2002.07.17
// Version		: 0.1
// Target MCU	: Atmel AVR Series
// Editor Tabs	: 4
//
// NOTE: This code is currently below version 1.0, and therefore is considered
// to be lacking in some functionality or documentation, or may not be fully
// tested.  Nonetheless, you can expect most functions to work.
//
// This code is distributed under the GNU Public License
//		which can be found at http://www.gnu.org/licenses/gpl.txt
//
//*****************************************************************************

#include <avr/io.h>
#include "global.h"
#include "uart.h"
#include "rprintf.h"
#include "nmea.h"
#include "systemtime.h"
#include "time.h"

#include "gps.h"


#define TRS_3V3_EN_CONFIG	(DDRB |= (1<<6))
#define TRS_3V3_EN_ON		(PORTB |= (1<<6))
#define TRS_3V3_EN_OFF		(PORTB &= ~(1<<6))

// Global variables
GpsInfoType GpsInfo;

// Functions
void gpsInit(void)
{
	uartInit();
	uartSetBaudRate(9600);
	GpsInfo.validTimeReceivedMillis = 0;

	TRS_3V3_EN_CONFIG;
	TRS_3V3_EN_OFF;

	gpsPowerEnable();
}

GpsInfoType* gpsGetInfo(void)
{
	return &GpsInfo;
}

void gpsProcess(void)
{
	nmeaProcess(uartGetRxBuffer());
}

time_t gpsGetTime(void)
{
	if((systemTimeGetMilliseconds() - GpsInfo.validTimeReceivedMillis) > 2000 && (systemTimeGetMilliseconds > 2000))
		return 0;

	tmElements_t el;
	uint32_t gpstime = (uint32_t)GpsInfo.PosLLA.TimeOfFix.f;
	//rprintfNum(10, 6, FALSE, '0', (const long)gpstime);
	el.Second = gpstime % 100;
	el.Minute = (gpstime / 100) % 100;
	el.Hour = (gpstime / 10000);
	uint32_t gpsdate = GpsInfo.UtDate;
	//rprintfNum(10, 6, FALSE, '0', (const long)gpsdate);
	el.Year = (gpsdate % 100) + 30;
	el.Month = (gpsdate / 100) % 100;
	el.Day = (gpsdate / 10000);
	return timeMake(el);
}

void gpsPowerEnable(void)
{
	TRS_3V3_EN_ON;
}

void gpsPowerDisable(void)
{
	TRS_3V3_EN_OFF;
}
