#include <avr/eeprom.h>
#include "time.h"
#include "timezone.h"

//Germany
timeChangeRule_t EEMEM EEtimezoneDstRule = {"MESZ", 120, 2, Sun, Last, Mar};
timeChangeRule_t EEMEM EEtimezoneStdRule = {"MEZ", 60, 3, Sun, Last, Oct};

timeChangeRule_t timezoneDstRule;
timeChangeRule_t timezoneStdRule;

time_t timezoneDstStartUtc;
time_t timezoneStdStartUtc;
time_t timezoneDstStartLocal;
time_t timezoneStdStartLocal;

void timezoneInit(void)
{
	eeprom_read_block(&timezoneDstRule, &EEtimezoneDstRule, sizeof(timeChangeRule_t));
	eeprom_read_block(&timezoneStdRule, &EEtimezoneStdRule, sizeof(timeChangeRule_t));
	//maybe validity check here
}

void timezoneSetDst(timeChangeRule_t dst)
{
	timezoneDstRule = dst;
	eeprom_write_block(&timezoneDstRule, &EEtimezoneDstRule, sizeof(timeChangeRule_t));
}

void timezoneSetStd(timeChangeRule_t std)
{
	timezoneStdRule = std;
	eeprom_write_block(&timezoneStdRule, &EEtimezoneStdRule, sizeof(timeChangeRule_t));
}

timeChangeRule_t timezoneGetDst(void)
{
	eeprom_read_block(&timezoneDstRule, &EEtimezoneDstRule, sizeof(timeChangeRule_t));
	return timezoneDstRule;
}

timeChangeRule_t timezoneGetStd(void)
{
	eeprom_read_block(&timezoneStdRule, &EEtimezoneStdRule, sizeof(timeChangeRule_t));
	return timezoneStdRule;
}

void timezoneCalculateStartTime(uint16_t year)
{
	timezoneDstStartLocal = timezoneRuleToTime(timezoneDstRule, year);
	timezoneStdStartLocal = timezoneRuleToTime(timezoneStdRule, year);
	timezoneDstStartUtc = timezoneDstStartLocal - timezoneStdRule.offset * SECS_PER_MIN;
	timezoneStdStartUtc = timezoneStdStartLocal - timezoneDstRule.offset * SECS_PER_MIN;
}

uint8_t timezoneUtcIsDst(time_t utc)
{
	if(timeGetYear(utc) != timeGetYear(timezoneDstStartUtc))
	{
		timezoneCalculateStartTime(timeGetYear(utc));
	}

	if(timezoneStdStartUtc > timezoneDstStartUtc)
	{
		return (utc >= timezoneDstStartUtc && utc < timezoneStdStartUtc);
	}
	else
	{
		return !(utc >= timezoneStdStartUtc && utc < timezoneDstStartUtc);
	}
}

uint8_t timezoneLocalIsDst(time_t local)
{
	if(timeGetYear(local) != timeGetYear(timezoneDstStartLocal))
	{
		timezoneCalculateStartTime(timeGetYear(local));
	}

	if(timezoneStdStartLocal > timezoneDstStartLocal)
	{
		return (local >= timezoneDstStartLocal && local < timezoneStdStartLocal);
	}
	else
	{
		return !(local >= timezoneStdStartLocal && local < timezoneDstStartLocal);
	}
}

time_t timezoneRuleToTime(timeChangeRule_t rule, uint16_t year)
{
	tmElements_t el;
	uint8_t month = rule.month;
	uint8_t week = rule.week;

	if(week == 0)
	{
		if(++month > 12)
		{
			month = 1;
			year++;
		}
		week = 1;
	}

	el.Hour = rule.hour;
	el.Minute = 0;
	el.Second = 0;
	el.Day = 1;
	el.Month = month;
	el.Year = year - 1970;

	time_t t = timeMake(el);
	t += (7 * (week - 1) + (rule.dow - timeGetWeekday(t) + 7) % 7) * SECS_PER_DAY;

	if(rule.week == 0)
	{
		t -= 7 * SECS_PER_DAY;
	}

	return t;
}

time_t timezoneTimeToLocal(time_t utc)
{
	if(timeGetYear(utc) != timeGetYear(timezoneDstStartUtc))
	{
		timezoneCalculateStartTime(timeGetYear(utc));
	}

	if(timezoneUtcIsDst(utc))
	{
		return utc + timezoneDstRule.offset * SECS_PER_MIN;
	}
	else 
	{
		return utc + timezoneStdRule.offset * SECS_PER_MIN;
	}
}

time_t timezoneTimeToUTC(time_t local)
{
	if(timeGetYear(local) != timeGetYear(timezoneDstStartLocal))
	{
		timezoneCalculateStartTime(timeGetYear(local));
	}

	if(timezoneLocalIsDst(local))
	{
		return local - timezoneDstRule.offset * SECS_PER_MIN;
	}
	else
	{
		return local - timezoneStdRule.offset * SECS_PER_MIN;
	}
}