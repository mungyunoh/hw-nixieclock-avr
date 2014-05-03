#include "time.h"
#include "ds1307.h"
#include "rtc.h"

void rtcInit(void)
{
	ds1307Init();
}

time_t rtcGetTime(void)
{
	tmElements_t el;
	el.Second = ds1307GetSeconds();
	el.Minute = ds1307GetMinutes();
	el.Hour = ds1307GetHours();
	el.Day = ds1307GetDate();
	el.Month = ds1307GetMonth();
	el.Year = ds1307GetYear() + 30;
	return timeMake(el);
}

void rtcSetTime(time_t time)
{
	tmElements_t el;
	timeBreak(time, &el);
	ds1307SetSeconds(el.Second);
	ds1307SetMinutes(el.Minute);
	ds1307SetHours(el.Hour);
	ds1307SetDate(el.Day);
	ds1307SetMonth(el.Month);
	ds1307SetYear(el.Year - 30);

	timeSetTime(time);
}