#include <avr/io.h>
#include <avr/pgmspace.h>

#include "usb_serial.h"
#include "i2c.h"

#include "cmdlineinterface.h"
#include "rprintf.h"
#include "systemtime.h"
#include "syncservice.h"
#include "time.h"
#include "rtc.h"
#include "gps.h"
#include "display.h"
#include "timezone.h"


#define LED_WHITE_CONFIG	(DDRC |= (1<<7))
#define LED_WHITE_ON		(PORTC &= ~(1<<7))
#define LED_WHITE_OFF		(PORTC |= (1<<7))

#define LED_GREEN_CONFIG	(DDRD |= (1<<5))
#define LED_GREEN_ON		(PORTD &= ~(1<<5))
#define LED_GREEN_OFF		(PORTD |= (1<<5))

#define LED_RED_CONFIG		(DDRB |= (1<<0))
#define LED_RED_ON			(PORTB &= ~(1<<0))
#define LED_RED_OFF			(PORTB |= (1<<0))


int main(void)
{
	LED_WHITE_CONFIG;
	LED_GREEN_CONFIG;
	LED_RED_CONFIG;

	LED_WHITE_OFF;
	LED_GREEN_OFF;
	LED_RED_OFF;

	usb_init();
	rprintfInit(usb_serial_putchar);
	cmdlineInterfaceInit();

	rtcInit();
	gpsInit();

	timeInit();
	timeSetSyncProvider(rtcGetTime);
	timeSetSyncInterval(60);

	timeSyncServiceInit();
	timeSyncServiceSetSyncReceiver(rtcSetTime);
	timeSyncServiceSetSyncProviderHighValidity(gpsGetTime);
	//timeSyncServiceSetSyncProviderLowValidity(ntpGetTime);
	timeSyncServiceSetInterval(30);

	timezoneInit();

	displayInit();
	displayHighVoltageEnable();

	time_t prevDisplayUTC = 0; // when the digital clock was displayed

	while (1)
	{
		LED_WHITE_ON;
		gpsProcess();
		LED_WHITE_OFF;

		cmdlineInterfaceProcess();
		timeSyncServiceProcess();

		//outsource that in display function
		if(timeStatus() != timeNotSet)
		{
			LED_RED_OFF;
			if(timeNow() != prevDisplayUTC)
			{
				prevDisplayUTC = timeNow();
				displayTime(timezoneTimeToLocal(prevDisplayUTC));
			}
		}
		else
		{
			LED_RED_ON;
		}
	}
}