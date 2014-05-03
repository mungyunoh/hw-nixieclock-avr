/*
  time.c - low level time and date functions
  Copyright (c) Michael Margolis 2009
  (modified by jan1s)

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  6  Jan 2010 - initial release
  12 Feb 2010 - fixed leap year calculation error
  1  Nov 2010 - fixed setTime bug (thanks to Korman for this)
  3  Sep 2013 - crippled to C by jan1s
*/

#define LED_RED_CONFIG		(DDRB |= (1<<0))
#define LED_RED_ON			(PORTB &= ~(1<<0))
#define LED_RED_OFF			(PORTB |= (1<<0))

#include <avr/io.h>
#include "systemtime.h"
#include "time.h"


static tmElements_t tm;          // a cache of time elements

void timeInit(void)
{
	systemTimeInit();
}

void timeRefreshCache( time_t t)
{
	static time_t cacheTime;   // the time the cache was updated
	if( t != cacheTime )
	{
		timeBreak(t, &tm);
		cacheTime = t;
	}
}

uint8_t timeGetHour(time_t t)   // the hour for the given time
{
	timeRefreshCache(t);
	return tm.Hour;
}

uint8_t timeGetHourFormat12(time_t t)   // the hour for the given time in 12 hour format
{
	timeRefreshCache(t);
	if( tm.Hour == 0 )
		return 12; // 12 midnight
	else if( tm.Hour  > 12)
		return tm.Hour - 12 ;
	else
		return tm.Hour ;
}

uint8_t timeIsAM(time_t t)   // returns true if given time is AM
{
	return !timeIsPM(t);
}

uint8_t timeIsPM(time_t t)   // returns true if PM
{
	return (timeGetHour(t) >= 12);
}

uint8_t timeGetMinute(time_t t)   // the minute for the given time
{
	timeRefreshCache(t);
	return tm.Minute;
}

uint8_t timeGetSecond(time_t t)    // the second for the given time
{
	timeRefreshCache(t);
	return tm.Second;
}

uint8_t timeGetDay(time_t t)   // the day for the given time (0-6)
{
	timeRefreshCache(t);
	return tm.Day;
}

uint8_t timeGetWeekday(time_t t)
{
	timeRefreshCache(t);
	return tm.Wday;
}

uint8_t timeGetMonth(time_t t)    // the month for the given time
{
	timeRefreshCache(t);
	return tm.Month;
}

uint16_t timeGetYear(time_t t)   // the year for the given time
{
	timeRefreshCache(t);
	return tmYearToCalendar(tm.Year);
}

/*============================================================================*/
/* functions to convert to and from system time */
/* These are for interfacing with time serivces and are not normally needed in a sketch */

// leap year calulator expects year argument as years offset from 1970
#define LEAP_YEAR(Y)     ( ((1970+Y)>0) && !((1970+Y)%4) && ( ((1970+Y)%100) || !((1970+Y)%400) ) )

static  const uint8_t monthDays[]= {31,28,31,30,31,30,31,31,30,31,30,31}; // API starts months from 1, this array starts from 0

void timeBreak(time_t time, tmElements_t* el)
{
// break the given time_t into time components
// this is a more compact version of the C library localtime function
// note that year is offset from 1970 !!!

	uint8_t year;
	uint8_t month, monthLength;
	unsigned long days;

	el->Second = time % 60;
	time /= 60; // now it is minutes
	el->Minute = time % 60;
	time /= 60; // now it is hours
	el->Hour = time % 24;
	time /= 24; // now it is days
	el->Wday = ((time + 4) % 7) + 1;  // Sunday is day 1

	year = 0;
	days = 0;
	while((unsigned)(days += (LEAP_YEAR(year) ? 366 : 365)) <= time)
	{
		year++;
	}
	el->Year = year; // year is offset from 1970

	days -= LEAP_YEAR(year) ? 366 : 365;
	time  -= days; // now it is days in this year, starting at 0

	days=0;
	month=0;
	monthLength=0;
	for (month=0; month<12; month++)
	{
		if (month==1)   // february
		{
			if (LEAP_YEAR(year))
			{
				monthLength=29;
			}
			else
			{
				monthLength=28;
			}
		}
		else
		{
			monthLength = monthDays[month];
		}

		if (time >= monthLength)
		{
			time -= monthLength;
		}
		else
		{
			break;
		}
	}
	el->Month = month + 1;  // jan is month 1
	el->Day = time + 1;     // day of month
}

time_t timeMake(tmElements_t el)
{
// assemble time elements into time_t
// note year argument is offset from 1970 (see macros in time.h to convert to other formats)
// previous version used full four digit year (or digits since 2000),i.e. 2009 was 2009 or 9

	int i;
	time_t seconds;

	// seconds from 1970 till 1 jan 00:00:00 of the given year
	seconds= el.Year*(SECS_PER_DAY * 365);
	for (i = 0; i < el.Year; i++)
	{
		if (LEAP_YEAR(i))
		{
			seconds +=  SECS_PER_DAY;   // add extra days for leap years
		}
	}

	// add days for this year, months start from 1
	for (i = 1; i < el.Month; i++)
	{
		if ( (i == 2) && LEAP_YEAR(el.Year))
		{
			seconds += SECS_PER_DAY * 29;
		}
		else
		{
			seconds += SECS_PER_DAY * monthDays[i-1];  //monthDay array starts from 0
		}
	}
	seconds+= (el.Day-1) * SECS_PER_DAY;
	seconds+= el.Hour * SECS_PER_HOUR;
	seconds+= el.Minute * SECS_PER_MIN;
	seconds+= el.Second;
	return seconds;
}
/*=====================================================*/
/* Low level system time functions  */


typedef struct
{
	time_t sysTime;
	time_t syncInterval;  // time sync will be attempted after this many seconds
	time_t nextSyncTime;
	uint32_t prevMilliseconds;
	timeStatus_t status;
} timesync_t;

timesync_t timesync = {0,300,0,0,timeNotSet};

getExternalTime getTimePtr;  // pointer to external sync function
//setExternalTime setTimePtr; // not used in this version

time_t timeNow(void)
{
	while( (int64_t)systemTimeGetMilliseconds() - (int64_t)timesync.prevMilliseconds >= 1000)
	{
		LED_RED_ON;
		timesync.sysTime++;
		timesync.prevMilliseconds += 1000;
		LED_RED_OFF;
	}
	if(timesync.nextSyncTime <= timesync.sysTime)
	{
		if(getTimePtr != 0)
		{
			time_t t = getTimePtr();
			if(t != 0)
			{
				timeSetTime(t);
			}
			else
				timesync.status = (timesync.status == timeNotSet) ?  timeNotSet : timeNeedsSync;
		}
	}
	return timesync.sysTime;
}

void timeSetTime(time_t t)
{
	timesync.sysTime = t;
	timesync.nextSyncTime = t + timesync.syncInterval;
	timesync.status = timeSet;
	timesync.prevMilliseconds = systemTimeGetMilliseconds();  // restart counting from now (thanks to Korman for this fix)
}

/*
void  setTime(uint8_t hr,uint8_t min,uint8_t sec,uint8_t dy, uint8_t mnth, uint16_t yr){
 // year can be given as full four digit year or two digts (2010 or 10 for 2010);
 //it is converted to years since 1970
  if( yr > 99)
      yr = yr - 1970;
  else
      yr += 30;
  tm.Year = yr;
  tm.Month = mnth;
  tm.Day = dy;
  tm.Hour = hr;
  tm.Minute = min;
  tm.Second = sec;
  setTime(makeTime(tm));
}
*/

void timeAdjust(int32_t adjustment)
{
	timesync.sysTime += adjustment;
}

timeStatus_t timeStatus(void)  // indicates if time has been set and recently synchronized
{
	return timesync.status;
}

void timeSetSyncProvider( getExternalTime getTimeFunction)
{
	getTimePtr = getTimeFunction;
	timesync.nextSyncTime = timesync.sysTime;
	timeNow(); // this will sync the clock
}

void timeSetSyncInterval(time_t interval)  // set the number of seconds between re-sync
{
	timesync.syncInterval = interval;
}