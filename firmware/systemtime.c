#include <avr/io.h>
#include <avr/interrupt.h>
#include "timer32u4.h"
#include "systemtime.h"

volatile uint32_t milliseconds;

void systemTimeInit(void)
{
	cli();
	milliseconds = 0;
	timer3Init();
	timer3SetMode(TIMER_MODE_CTC_OCR);
	timer3SetOutputModeA(TIMER_OUTMODE_TOGGLE);
	timer3SetCompareValueA(249);
	timerAttach(TIMER3OUTCOMPAREA_INT, systemTimeMillisecondsTick);
	sei();
}

void systemTimeMillisecondsTick(void)
{
	cli();
	milliseconds++;
	sei();
}

uint32_t systemTimeGetMilliseconds(void)
{
	return milliseconds;
}