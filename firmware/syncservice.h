#ifndef SYNCSERVICE_H
#define SYNCSERVICE_H

#include "global.h"

typedef void (*setExternalTime)(time_t t);
typedef time_t (*getExternalTime)(void);

void timeSyncServiceInit(void);
void timeSyncServiceProcess(void);
void timeSyncServiceSetInterval(uint16_t interval);
void timeSyncServiceSetSyncReceiver(setExternalTime setTimeFunction);
void timeSyncServiceSetSyncProviderHighValidity(getExternalTime getTimeFunction);
void timeSyncServiceSetSyncProviderLowValidity(getExternalTime getTimeFunction);

#endif