#ifndef RTC_H
#define RTC_H

#include "global.h"

void rtcInit(void);
time_t rtcGetTime(void);
void rtcSetTime(time_t time);

#endif