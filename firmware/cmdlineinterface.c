

#include <avr/io.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <string.h>
#include <util/delay.h>

#include "rprintf.h"
#include "cmdline.h"
#include "usb_serial.h"
#include "systemtime.h"
#include "time.h"
#include "rtc.h"
#include "gps.h"
#include "timezone.h"

#include "cmdlineinterface.h"

uint8_t EEMEM eeCmdlineOwner[12] = "Janis";
uint8_t cmdlineOwner[12];

void cmdlineInterfaceInit(void)
{
	cmdlineInit();
	cmdlineSetOutputFunc(usb_serial_putchar);

	eeprom_read_block(cmdlineOwner, eeCmdlineOwner, sizeof(cmdlineOwner));

	cmdlineAddCommand("exit", exitFunction);
	cmdlineAddCommand("help", helpFunction);

	cmdlineAddCommand("settime", setTimeFunction);
	cmdlineAddCommand("setdst", setDstFunction);
	cmdlineAddCommand("setstd", setStdFunction);
	cmdlineAddCommand("setowner", setOwnerFunction);

	cmdlineAddCommand("rtc", rtcTime);
	cmdlineAddCommand("gps", gpsTime);
	cmdlineAddCommand("sys", systemTime);
	cmdlineAddCommand("local", localTime);
	cmdlineAddCommand("millis", milliview);
	
}

void cmdlineInterfaceProcess(void)
{
	static int8_t configured = FALSE;
	static int8_t control = FALSE;
	// If the Board is powered without a PC connected
	// to the USB port, this will be be false
	if(usb_configured())
	{
		// do something if this is the case for the first time!!
		if(!configured)
		{
			_delay_ms(100);
		}
		configured = TRUE;

		// wait for the user to run their terminal emulator program
		// which sets DTR to indicate it is ready to receive.
		if(usb_serial_get_control() & USB_SERIAL_DTR)
		{
			// do something if this is the case for the first time!!
			if(!control)
			{
				// discard anything that was received prior.  Sometimes the
				// operating system or other software will send a modem
				// "AT command", which can still be buffered.
				usb_serial_flush_input();
				rprintf("Hey ");
				rprintfStr(cmdlineOwner);
				rprintf(", my current RTC time is:\r\n");
				time_t t = rtcGetTime();
				printTime(t);

				// trigger cmd prompt
				cmdlineInputFunc('\r');
			}
			control = TRUE;

			uint8_t c;
			c = usb_serial_getchar();
			if(c != -1)
			{
				cmdlineInputFunc(c);
			}

			cmdlineMainLoop();
		}
		else
		{
			control = FALSE;
		}
	}
	else
	{
		configured = FALSE;
	}
}

void exitFunction(void)
{
	//ndy
}

void helpFunction(void)
{
	rprintfProgStrM("\r\nSet the time (utc):\r\n");
	rprintfProgStrM(" settime - yyyy mm dd hh mm ss\r\n\r\n");

	rprintfProgStrM("Set the DST rule:\r\n");
	rprintfProgStrM(" setdst - offset hour dow week month\r\n");
	rprintfProgStrM("Set the STD rule:\r\n");
	rprintfProgStrM(" setstd - offset hour dow week month\r\n\r\n");
	rprintfProgStrM(" DST example:\r\n");
	rprintfProgStrM(" 120\t- offset in minutes\r\n");
	rprintfProgStrM(" 2\t- hour of day\r\n");
	rprintfProgStrM(" 1\t- day of week (Sun:1 Mon:2 Tue:3 Wed:4 Thu:5 Fri:6 Sat:7)\r\n");
	rprintfProgStrM(" 0\t- week (Last:0 First:1 Second:2 Third:3 Fourth:4)\r\n");
	rprintfProgStrM(" 3\t- month (Jan:1 Feb:2 Mar:3 Apr:4 May:5 Jun:6 Jul:7 Aug:8 Sep:9 Oct:10 Nov:11 Dec:12)\r\n\r\n");

	rprintfProgStrM("Set the clock owner:\r\n");
	rprintfProgStrM(" setowner - name\r\n\r\n");

	//rprintfProgStrM("Get milliseconds of system uptime:\r\n");
	//rprintfProgStrM(" millis\r\n\r\n");

	rprintfProgStrM("Get various times:\r\n");
	rprintfProgStrM(" sys, rtc, gps, local\r\n\r\n");
}

void setTimeFunction(void)
{
	tmElements_t el;
	uint16_t year = cmdlineGetArgInt(1);
	if(year > 99)
	{
		el.Year = cmdlineGetArgInt(1) - 1970;
	}
	else
	{
		el.Year = cmdlineGetArgInt(1) + 30;
	}
	el.Month = cmdlineGetArgInt(2);
	el.Day = cmdlineGetArgInt(3);
	el.Hour = cmdlineGetArgInt(4);
	el.Minute = cmdlineGetArgInt(5);
	el.Second = cmdlineGetArgInt(6);
	rtcSetTime(timeMake(el));

	rtcTime();
}

/*
Set DST change rule via cmdline
Expects command in form of: 

	setdst 120 2 1 0 3
	120 - offset in minutes
	2 - hour of day
	1 - day of week (Sun:1 Mon:2 Tue:3 Wed:4 Thu:5 Fri:6 Sat:7)
	0 - week (Last:0 First:1 Second:2 Third:3 Fourth:4)
	3 - month (Jan:1 Feb:2 Mar:3 Apr:4 May:5 Jun:6 Jul:7 Aug:8 Sep:9 Oct:10 Nov:11 Dec:12)

Rule is saved in the EEPROM by the timezone lib
*/
void setDstFunction(void)
{
	timeChangeRule_t r;

	//r.abbrev = "DUMMY";
	r.offset = cmdlineGetArgInt(1);
	r.hour = cmdlineGetArgInt(2);
	r.dow = cmdlineGetArgInt(3);
	r.week = cmdlineGetArgInt(4);
	r.month = cmdlineGetArgInt(5);

	timezoneSetDst(r);
	getDstFunction();
}

void setStdFunction(void)
{
	timeChangeRule_t r;

	//r.abbrev = "DUMMY";
	r.offset = cmdlineGetArgInt(1);
	r.hour = cmdlineGetArgInt(2);
	r.dow = cmdlineGetArgInt(3);
	r.week = cmdlineGetArgInt(4);
	r.month = cmdlineGetArgInt(5);

	timezoneSetStd(r);
	getStdFunction();
}

void getDstFunction(void)
{
	timeChangeRule_t r = timezoneGetDst();

	//rprintfStr(r.abbrev);
	rprintf("DST - offset: %d", r.offset);
	rprintf(", hour: %d", r.hour);
	rprintf(", dow: %d", r.dow);
	rprintf(", week: %d", r.week);
	rprintf(", month: %d", r.month);
	rprintfCRLF();
}

void getStdFunction(void)
{
	timeChangeRule_t r = timezoneGetStd();

	//rprintfStr(r.abbrev);
	rprintf("STD - offset: %d", r.offset);
	rprintf(", hour: %d", r.hour);
	rprintf(", dow: %d", r.dow);
	rprintf(", week: %d", r.week);
	rprintf(", month: %d", r.month);
	rprintfCRLF();
}

void setOwnerFunction(void)
{
	strcpy(cmdlineOwner, cmdlineGetArgStr(1));
	eeprom_write_block(cmdlineOwner, eeCmdlineOwner, sizeof(cmdlineOwner));

	rprintfCRLF();
	rprintfProgStrM("Hello ");
	rprintfStr(cmdlineOwner);
	rprintfProgStrM(", \n\rhave fun with your new clock! :)\n\rJanis\n\r\n\r");
}

void systemTime(void)
{
	rprintfCRLF();
	rprintf("SYS time (utc):\r\n");

	time_t t = timeNow();
	printTime(t);
}

void rtcTime(void)
{
	rprintfCRLF();
	rprintf("RTC time (utc):\r\n");

	time_t t = rtcGetTime();
	printTime(t);
}

void gpsTime(void)
{
	rprintfCRLF();
	rprintf("GPS time (utc):\r\n");

	time_t t = gpsGetTime();
	printTime(t);
}

void localTime(void)
{
	rprintfCRLF();
	rprintf("Local time\r\n");

	time_t t = timezoneTimeToLocal(timeNow());
	printTime(t);
}

void printTime(time_t t)
{
	uint16_t year = timeGetYear(t);
	uint8_t month = timeGetMonth(t);
	uint8_t day = timeGetDay(t);
	uint8_t hour = timeGetHour(t);
	uint8_t minute = timeGetMinute(t);
	uint8_t second = timeGetSecond(t);

	rprintfNum(10, 4, FALSE, '0', (const long)year);
	rprintf("-");
	rprintfNum(10, 2, FALSE, '0', (const long)month);
	rprintf("-");
	rprintfNum(10, 2, FALSE, '0', (const long)day);
	rprintf("T");
	rprintfNum(10, 2, FALSE, '0', (const long)hour);
	rprintf(":");
	rprintfNum(10, 2, FALSE, '0', (const long)minute);
	rprintf(":");
	rprintfNum(10, 2, FALSE, '0', (const long)second);
	rprintfCRLF();
}

void milliview(void)
{
	uint32_t milli = systemTimeGetMilliseconds();
	rprintfNum(10, 9, FALSE, ' ', (const long)milli);
	rprintfCRLF();
}
