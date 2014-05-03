#include <avr/io.h>
#include "time.h"
#include "systemtime.h"

#include "syncservice.h"
#include "rprintf.h"

#define LED_GREEN_CONFIG	(DDRD |= (1<<5))
#define LED_GREEN_ON		(PORTD &= ~(1<<5))
#define LED_GREEN_OFF		(PORTD |= (1<<5))

typedef struct
{
	uint32_t interval;

} syncservice_t;

syncservice_t syncservice;

static setExternalTime syncReceiverPtr;
static getExternalTime syncProviderHighPtr;
static getExternalTime syncProviderLowPtr;


void timeSyncServiceInit(void)
{
	LED_GREEN_CONFIG;
}

void timeSyncServiceProcess(void)
{
	static uint32_t lastTimeSynced = 0;
	if(systemTimeGetMilliseconds() - lastTimeSynced < syncservice.interval)
		return;

	if(syncReceiverPtr != 0)
	{
		if(syncProviderHighPtr != 0)
		{
			time_t t = syncProviderHighPtr();
			if(t != 0)
			{
				syncReceiverPtr(t);
				timeSetTime(t);
				LED_GREEN_ON;
			}
			else
			{
				LED_GREEN_OFF;
			}
		}
		//else if(syncProviderLowPtr != 0)
		//{
			//time_t t = syncProviderLowPtr();
			//if(t != 0)
			//{
				//syncReceiverPtr(t);
				//timeSetTime(t);
				//lastTimeSynced = systemTimeGetMilliseconds();
			//}
		//}
	}

	// only do it once every syncinterval
	lastTimeSynced = systemTimeGetMilliseconds();
}

void timeSyncServiceSetInterval(uint16_t interval)
{
	syncservice.interval = interval * 1000;
}

void timeSyncServiceSetSyncReceiver(setExternalTime setTimeFunction)
{
	syncReceiverPtr = setTimeFunction;
}

void timeSyncServiceSetSyncProviderHighValidity(getExternalTime getTimeFunction)
{
	syncProviderHighPtr = getTimeFunction;
}

void timeSyncServiceSetSyncProviderLowValidity(getExternalTime getTimeFunction)
{
	syncProviderLowPtr = getTimeFunction;
}