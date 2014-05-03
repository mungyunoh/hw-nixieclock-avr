#ifndef SYSTEMTIME_H
#define SYSTEMTIME_H

#include "global.h"

void systemTimeInit(void);
void systemTimeMillisecondsTick(void);
uint32_t systemTimeGetMilliseconds(void);

#endif