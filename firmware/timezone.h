#ifndef TIMEZONE_H
#define TIMEZONE_H

#include "global.h"

enum dow_t {Sun=1,Mon,Tue,Wed,Thu,Fri,Sat};
enum week_t {Last,First,Second,Third,Fourth};
enum month_t {Jan=1,Feb,Mar,Apr,May,Jun,Jul,Aug,Sep,Oct,Nov,Dec};

typedef struct
{
	uint8_t abbrev[6];
	int16_t offset;
	uint8_t hour:5;
	uint8_t dow:3;
	uint8_t week:3;
	uint8_t month:4;
} timeChangeRule_t;

void timezoneInit(void);
void timezoneSetDst(timeChangeRule_t dst);
void timezoneSetStd(timeChangeRule_t std);
timeChangeRule_t timezoneGetDst(void);
timeChangeRule_t timezoneGetStd(void);
void timezoneCalculateStartTimes(uint16_t year);
uint8_t timezoneUtcIsDst(time_t utc);
uint8_t timezoneLocalIsDst(time_t local);
time_t timezoneRuleToTime(timeChangeRule_t rule, uint16_t year);
time_t timezoneTimeToLocal(time_t utc);
time_t timezoneTimeToUTC(time_t local);


#endif