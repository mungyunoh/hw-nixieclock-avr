#ifndef CMDLINE_INTERFACE_H
#define CMDLINE_INTERFACE_H

#include "global.h"

void cmdlineInterfaceInit(void);
void cmdlineInterfaceProcess(void);
void exitFunction(void);
void helpFunction(void);
void setTimeFunction(void);
void setDstFunction(void);
void setStdFunction(void);
void getDstFunction(void);
void getStdFunction(void);
void setOwnerFunction(void);
void systemTime(void);
void rtcTime(void);
void gpsTime(void);
void localTime(void);
void printTime(time_t t);
void milliview(void);
void gpsInfoPrint(void);



#endif