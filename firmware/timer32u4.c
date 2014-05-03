/*! \file timerx8.c \brief Timer function library for ATmegaXX8 Processors. */
//*****************************************************************************
//
// File Name	: 'timerx8.c'
// Title		: Timer function library for ATmegaXX8 Processors
// Author		: Pascal Stang - Copyright (C) 2000-2005
// Created		: 11/22/2000
// Revised		: 06/15/2005
// Version		: 1.0
// Target MCU	: Atmel AVR Series
// Editor Tabs	: 4
//
// This code is distributed under the GNU Public License
//		which can be found at http://www.gnu.org/licenses/gpl.txt
//
//*****************************************************************************

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/sleep.h>

#include "global.h"
#include "timer32u4.h"

// Program ROM constants
// the prescale division values stored in order of timer control register index
// STOP, CLK, CLK/8, CLK/64, CLK/256, CLK/1024
const uint16_t TimerPrescaleFactor[] PROGMEM = {0,1,8,64,256,1024};
// the prescale division values stored in order of timer control register index
// STOP, CLK, CLK/8, CLK/32, CLK/64, CLK/128, CLK/256, CLK/1024
const uint16_t TimerRTCPrescaleFactor[] PROGMEM = {0,1,8,32,64,128,256,1024};

// Global variables
// time registers
volatile uint32_t TimerPauseReg;
volatile uint32_t Timer0Reg0;
volatile uint32_t Timer1Reg0;
volatile uint32_t Timer3Reg0;
volatile uint32_t Timer4Reg0;

typedef void (*voidFuncPtr)(void);
volatile static voidFuncPtr TimerIntFunc[TIMER_NUM_INTERRUPTS];

/*
// delay for a minimum of <us> microseconds
// the time resolution is dependent on the time the loop takes
// e.g. with 4Mhz and 5 cycles per loop, the resolution is 1.25 us
void delay_us(uint16_t time_us)
{
	uint16_t delay_loops;
	register uint16_t i;

	delay_loops = (time_us+3)/5*CYCLES_PER_US; // +3 for rounding up (dirty)

	// one loop takes 5 cpu cycles
	for (i=0; i < delay_loops; i++) {};
}
*/
/*
void delay_ms(unsigned char time_ms)
{
	unsigned short delay_count = F_CPU / 4000;

	unsigned short cnt;
	asm volatile ("\n"
                  "L_dl1%=:\n\t"
                  "mov %A0, %A2\n\t"
                  "mov %B0, %B2\n"
                  "L_dl2%=:\n\t"
                  "sbiw %A0, 1\n\t"
                  "brne L_dl2%=\n\t"
                  "dec %1\n\t" "brne L_dl1%=\n\t":"=&w" (cnt)
                  :"r"(time_ms), "r"((unsigned short) (delay_count))
	);
}
*/
void timerInit(void)
{
	uint8_t intNum;
	// detach all user functions from interrupts
	for(intNum=0; intNum<TIMER_NUM_INTERRUPTS; intNum++)
		timerDetach(intNum);

	// initialize all timers
	timer0Init();
	timer1Init();
	timer3Init();
	timer4Init();

	// enable interrupts
	sei();
}

void timer0Init()
{
	// initialize timer 0
	timer0SetPrescaler( TIMER0PRESCALE );	// set prescaler
	TCNT0 = 0;								// reset TCNT0
	sbi(TIMSK0, TOIE0);						// enable TCNT0 overflow interrupt

	timer0ClearOverflowCount();				// initialize time registers
}

void timer1Init(void)
{
	// initialize timer 1
	timer1SetPrescaler( TIMER1PRESCALE );	// set prescaler
	TCNT1 = 0;								// reset TCNT1
	sbi(TIMSK1, TOIE1);						// enable TCNT1 overflow
}

void timer3Init(void)
{
	// initialize timer 2
	timer3SetPrescaler( TIMER3PRESCALE );	// set prescaler
	TCNT3 = 0;								// reset TCNT3
	sbi(TIMSK3, TOIE3);						// enable TCNT3 overflow

	timer3ClearOverflowCount();				// initialize time registers
}

void timer4Init(void)
{
	// initialize timer 2
	timer4SetPrescaler( TIMER4PRESCALE );	// set prescaler
	TCNT4 = 0;								// reset TCNT3
	sbi(TIMSK4, TOIE4);						// enable TCNT3 overflow

	timer4ClearOverflowCount();				// initialize time registers
}



void timer0SetPrescaler(uint8_t prescale)
{
	// set prescaler on timer 0
	TCCR0B = ((TCCR0B & ~TIMER_PRESCALE_MASK) | prescale);
}

void timer1SetPrescaler(uint8_t prescale)
{
	// set prescaler on timer 1
	TCCR1B = ((TCCR1B & ~TIMER_PRESCALE_MASK) | prescale);
}

void timer3SetPrescaler(uint8_t prescale)
{
	// set prescaler on timer 2
	TCCR3B = ((TCCR3B & ~TIMER_PRESCALE_MASK) | prescale);
}

void timer4SetPrescaler(uint8_t prescale)
{
	// set prescaler on timer 2
	TCCR4B = ((TCCR4B & ~TIMER_PRESCALE_MASK) | prescale);
}


uint16_t timer0GetPrescaler(void)
{
	// get the current prescaler setting
	return (pgm_read_word(TimerPrescaleFactor+(TCCR0B & TIMER_PRESCALE_MASK)));
}

uint16_t timer1GetPrescaler(void)
{
	// get the current prescaler setting
	return (pgm_read_word(TimerPrescaleFactor+(TCCR1B & TIMER_PRESCALE_MASK)));
}

uint16_t timer3GetPrescaler(void)
{
	// get the current prescaler setting
	return (pgm_read_word(TimerPrescaleFactor+(TCCR3B & TIMER_PRESCALE_MASK)));
}

uint16_t timer4GetPrescaler(void)
{
	// get the current prescaler setting
	return (pgm_read_word(TimerPrescaleFactor+(TCCR4B & TIMER_PRESCALE_MASK)));
}


void timerAttach(uint8_t interruptNum, void (*userFunc)(void) )
{
	// make sure the interrupt number is within bounds
	if(interruptNum < TIMER_NUM_INTERRUPTS)
	{
		// set the interrupt function to run
		// the supplied user's function
		TimerIntFunc[interruptNum] = userFunc;
	}
}

void timerDetach(uint8_t interruptNum)
{
	// make sure the interrupt number is within bounds
	if(interruptNum < TIMER_NUM_INTERRUPTS)
	{
		// set the interrupt function to run nothing
		TimerIntFunc[interruptNum] = 0;
	}
}
/*
uint32_t timerMsToTics(uint16_t ms)
{
	// calculate the prescaler division rate
	uint16_t prescaleDiv = 1<<(pgm_read_byte(TimerPrescaleFactor+inb(TCCR0)));
	// calculate the number of timer tics in x milliseconds
	return (ms*(F_CPU/(prescaleDiv*256)))/1000;
}

uint16_t timerTicsToMs(uint32_t tics)
{
	// calculate the prescaler division rate
	uint16_t prescaleDiv = 1<<(pgm_read_byte(TimerPrescaleFactor+inb(TCCR0)));
	// calculate the number of milliseconds in x timer tics
	return (tics*1000*(prescaleDiv*256))/F_CPU;
}
*/
void timerPause(unsigned short pause_ms)
{
	// pauses for exactly <pause_ms> number of milliseconds
	uint8_t timerThres;
	uint32_t ticRateHz;
	uint32_t pause;

	// capture current pause timer value
	timerThres = TCNT0;
	// reset pause timer overflow count
	TimerPauseReg = 0;
	// calculate delay for [pause_ms] milliseconds
	// prescaler division = 1<<(pgm_read_byte(TimerPrescaleFactor+inb(TCCR0)))
	ticRateHz = F_CPU/timer0GetPrescaler();
	// precision management
	// prevent overflow and precision underflow
	//	-could add more conditions to improve accuracy
	if( ((ticRateHz < 429497) && (pause_ms <= 10000)) )
		pause = (pause_ms*ticRateHz)/1000;
	else
		pause = pause_ms*(ticRateHz/1000);

	// loop until time expires
	while( ((TimerPauseReg<<8) | (TCNT0)) < (pause+timerThres) )
	{
		if( TimerPauseReg < (pause>>8));
		{
			// save power by idling the processor
			set_sleep_mode(SLEEP_MODE_IDLE);
			sleep_mode();
		}
	}

	/* old inaccurate code, for reference

	// calculate delay for [pause_ms] milliseconds
	uint16_t prescaleDiv = 1<<(pgm_read_byte(TimerPrescaleFactor+inb(TCCR0)));
	uint32_t pause = (pause_ms*(F_CPU/(prescaleDiv*256)))/1000;

	TimerPauseReg = 0;
	while(TimerPauseReg < pause);

	*/
}

void timer0ClearOverflowCount(void)
{
	// clear the timer overflow counter registers
	Timer0Reg0 = 0;	// initialize time registers
}

uint32_t timer0GetOverflowCount(void)
{
	// return the current timer overflow count
	// (this is since the last timer0ClearOverflowCount() command was called)
	return Timer0Reg0;
}

void timer1ClearOverflowCount(void)
{
	// clear the timer overflow counter registers
	Timer1Reg0 = 0;	// initialize time registers
}

uint32_t timer1GetOverflowCount(void)
{
	// return the current timer overflow count
	// (this is since the last timer1ClearOverflowCount() command was called)
	return Timer1Reg0;
}

void timer3ClearOverflowCount(void)
{
	// clear the timer overflow counter registers
	Timer3Reg0 = 0;	// initialize time registers
}

uint32_t timer3GetOverflowCount(void)
{
	// return the current timer overflow count
	// (this is since the last timer3ClearOverflowCount() command was called)
	return Timer3Reg0;
}

void timer4ClearOverflowCount(void)
{
	// clear the timer overflow counter registers
	Timer4Reg0 = 0;	// initialize time registers
}

uint32_t timer4GetOverflowCount(void)
{
	// return the current timer overflow count
	// (this is since the last timer4ClearOverflowCount() command was called)
	return Timer4Reg0;
}


// timer 0 configuration

void timer0SetMode(uint8_t mode)
{
	TCCR0A &= ~TIMER_MODE_TCCRNA_MASK;
	TCCR0A |= (mode & TIMER_MODE_TCCRNA_MASK);

	TCCR0B &= ~TIMER_MODE_TCCRNB_MASK;
	TCCR0B |= (mode & TIMER_MODE_TCCRNB_MASK);
	/*
	if(mode == TIMER_MODE_NORMAL)
	{
		cbi(TCCR0A,WGM00);
		cbi(TCCR0A,WGM01);
		cbi(TCCR0B,WGM02);
	}
	else if(mode == TIMER_MODE_PWM_PHASECORRECT_8BIT)
	{
		sbi(TCCR0A,WGM00);
		cbi(TCCR0A,WGM01);
		cbi(TCCR0B,WGM02);
	}
	else if(mode == TIMER_MODE_CTC_OCR)
	{
		cbi(TCCR0A,WGM00);
		sbi(TCCR0A,WGM01);
		cbi(TCCR0B,WGM02);
	}
	else if(mode == TIMER_MODE_FASTPWM_8BIT)
	{
		sbi(TCCR0A,WGM00);
		sbi(TCCR0A,WGM01);
		cbi(TCCR0B,WGM02);
	}
	else if(mode == TIMER_MODE_PWM_PHASECORRECT_OCR)
	{
		sbi(TCCR0A,WGM00);
		cbi(TCCR0A,WGM01);
		sbi(TCCR0B,WGM02);
	}
	else if(mode == TIMER_MODE_FASTPWM_OCR)
	{
		sbi(TCCR0A,WGM00);
		sbi(TCCR0A,WGM01);
		sbi(TCCR0B,WGM02);
	}
	*/
}

void timer0SetOutputModeA(uint8_t mode)
{
	if(mode == TIMER_OUTMODE_DISCONNECTED)
	{
		cbi(TCCR0A,COM0A0);
		cbi(TCCR0A,COM0A1);
	}
	else if(mode == TIMER_OUTMODE_TOGGLE)
	{
		sbi(TCCR0A,COM0A0);
		cbi(TCCR0A,COM0A1);
	}
	else if(mode == TIMER_OUTMODE_CLEAR)
	{
		cbi(TCCR0A,COM0A0);
		sbi(TCCR0A,COM0A1);
	}
	else if(mode == TIMER_OUTMODE_SET)
	{
		sbi(TCCR0A,COM0A0);
		sbi(TCCR0A,COM0A1);
	}
}

void timer0SetOutputModeB(uint8_t mode)
{
	if(mode == TIMER_OUTMODE_DISCONNECTED)
	{
		cbi(TCCR0A,COM0B0);
		cbi(TCCR0A,COM0B1);
	}
	else if(mode == TIMER_OUTMODE_TOGGLE)
	{
		sbi(TCCR0A,COM0B0);
		cbi(TCCR0A,COM0B1);
	}
	else if(mode == TIMER_OUTMODE_CLEAR)
	{
		cbi(TCCR0A,COM0B0);
		sbi(TCCR0A,COM0B1);
	}
	else if(mode == TIMER_OUTMODE_SET)
	{
		sbi(TCCR0A,COM0B0);
		sbi(TCCR0A,COM0B1);
	}
}


// timer 1 configuration

void timer1SetMode(uint8_t mode)
{
	TCCR1A &= ~TIMER_MODE_TCCRNA_MASK;
	TCCR1A |= (mode & TIMER_MODE_TCCRNA_MASK);

	TCCR1B &= ~TIMER_MODE_TCCRNB_MASK;
	TCCR1B |= (mode & TIMER_MODE_TCCRNB_MASK);
	/*
	if(mode == TIMER_MODE_NORMAL)
	{
		cbi(TCCR1A,WGM10);
		cbi(TCCR1A,WGM11);
		cbi(TCCR1B,WGM12);
		cbi(TCCR1B,WGM13);
	}
	else if(mode == TIMER_MODE_PWM_PHASECORRECT_8BIT)
	{
		sbi(TCCR1A,WGM10);
		cbi(TCCR1A,WGM11);
		cbi(TCCR1B,WGM12);
		cbi(TCCR1B,WGM13);
	}
	else if(mode == TIMER_MODE_PWM_PHASECORRECT_9BIT)
	{
		cbi(TCCR1A,WGM10);
		sbi(TCCR1A,WGM11);
		cbi(TCCR1B,WGM12);
		cbi(TCCR1B,WGM13);
	}
	else if(mode == TIMER_MODE_PWM_PHASECORRECT_10BIT)
	{
		sbi(TCCR1A,WGM10);
		sbi(TCCR1A,WGM11);
		cbi(TCCR1B,WGM12);
		cbi(TCCR1B,WGM13);
	}
	else if(mode == TIMER_MODE_CTC_OCR)
	{
		cbi(TCCR1A,WGM10);
		cbi(TCCR1A,WGM11);
		sbi(TCCR1B,WGM12);
		cbi(TCCR1B,WGM13);
	}
	else if(mode == TIMER_MODE_FASTPWM_8BIT)
	{
		sbi(TCCR1A,WGM10);
		cbi(TCCR1A,WGM11);
		sbi(TCCR1B,WGM12);
		cbi(TCCR1B,WGM13);
	}
	else if(mode == TIMER_MODE_FASTPWM_9BIT)
	{
		cbi(TCCR1A,WGM10);
		sbi(TCCR1A,WGM11);
		sbi(TCCR1B,WGM12);
		cbi(TCCR1B,WGM13);
	}
	else if(mode == TIMER_MODE_FASTPWM_10BIT)
	{
		sbi(TCCR1A,WGM10);
		sbi(TCCR1A,WGM11);
		sbi(TCCR1B,WGM12);
		cbi(TCCR1B,WGM13);
	}
	else if(mode == TIMER_MODE_PWM_PHASECORRECT_FREQUENCYCORRECT_ICR)
	{
		cbi(TCCR1A,WGM10);
		cbi(TCCR1A,WGM11);
		cbi(TCCR1B,WGM12);
		sbi(TCCR1B,WGM13);
	}
	else if(mode == TIMER_MODE_PWM_PHASECORRECT_FREQUENCYCORRECT_OCR)
	{
		sbi(TCCR1A,WGM10);
		cbi(TCCR1A,WGM11);
		cbi(TCCR1B,WGM12);
		sbi(TCCR1B,WGM13);
	}
	else if(mode == TIMER_MODE_PWM_PHASECORRECT_ICR)
	{
		cbi(TCCR1A,WGM10);
		sbi(TCCR1A,WGM11);
		cbi(TCCR1B,WGM12);
		sbi(TCCR1B,WGM13);
	}
	else if(mode == TIMER_MODE_PWM_PHASECORRECT_OCR)
	{
		sbi(TCCR1A,WGM10);
		sbi(TCCR1A,WGM11);
		cbi(TCCR1B,WGM12);
		sbi(TCCR1B,WGM13);
	}
	else if(mode == TIMER_MODE_CTC_ICR)
	{
		cbi(TCCR1A,WGM10);
		cbi(TCCR1A,WGM11);
		sbi(TCCR1B,WGM12);
		sbi(TCCR1B,WGM13);
	}
	else if(mode == TIMER_MODE_FASTPWM_ICR)
	{
		cbi(TCCR1A,WGM10);
		sbi(TCCR1A,WGM11);
		sbi(TCCR1B,WGM12);
		sbi(TCCR1B,WGM13);
	}
	else if(mode == TIMER_MODE_FASTPWM_OCR)
	{
		sbi(TCCR1A,WGM10);
		sbi(TCCR1A,WGM11);
		sbi(TCCR1B,WGM12);
		sbi(TCCR1B,WGM13);
	}
	*/
}

void timer1SetOutputModeA(uint8_t mode)
{
	if(mode == TIMER_OUTMODE_DISCONNECTED)
	{
		cbi(TCCR1A,COM1A0);
		cbi(TCCR1A,COM1A1);
	}
	else if(mode == TIMER_OUTMODE_TOGGLE)
	{
		sbi(TCCR1A,COM1A0);
		cbi(TCCR1A,COM1A1);
	}
	else if(mode == TIMER_OUTMODE_CLEAR)
	{
		cbi(TCCR1A,COM1A0);
		sbi(TCCR1A,COM1A1);
	}
	else if(mode == TIMER_OUTMODE_SET)
	{
		sbi(TCCR1A,COM1A0);
		sbi(TCCR1A,COM1A1);
	}
}

void timer1SetOutputModeB(uint8_t mode)
{
	if(mode == TIMER_OUTMODE_DISCONNECTED)
	{
		cbi(TCCR1A,COM1B0);
		cbi(TCCR1A,COM1B1);
	}
	else if(mode == TIMER_OUTMODE_TOGGLE)
	{
		sbi(TCCR1A,COM1B0);
		cbi(TCCR1A,COM1B1);
	}
	else if(mode == TIMER_OUTMODE_CLEAR)
	{
		cbi(TCCR1A,COM1B0);
		sbi(TCCR1A,COM1B1);
	}
	else if(mode == TIMER_OUTMODE_SET)
	{
		sbi(TCCR1A,COM1B0);
		sbi(TCCR1A,COM1B1);
	}
}

void timer1SetOutputModeC(uint8_t mode)
{
	if(mode == TIMER_OUTMODE_DISCONNECTED)
	{
		cbi(TCCR1A,COM1C0);
		cbi(TCCR1A,COM1C1);
	}
	else if(mode == TIMER_OUTMODE_TOGGLE)
	{
		sbi(TCCR1A,COM1C0);
		cbi(TCCR1A,COM1C1);
	}
	else if(mode == TIMER_OUTMODE_CLEAR)
	{
		cbi(TCCR1A,COM1C0);
		sbi(TCCR1A,COM1C1);
	}
	else if(mode == TIMER_OUTMODE_SET)
	{
		sbi(TCCR1A,COM1C0);
		sbi(TCCR1A,COM1C1);
	}
}

void timer1SetCompareValueA(uint16_t compareValue)
{
	// set PWM (output compare) duty for channel A
	// this PWM output is generated on OC1A pin
	// NOTE:	pwmDuty should be in the range 0-255 for 8bit PWM
	//			pwmDuty should be in the range 0-511 for 9bit PWM
	//			pwmDuty should be in the range 0-1023 for 10bit PWM
	OCR1A = compareValue;
}

void timer1SetCompareValueB(uint16_t compareValue)
{
	// set PWM (output compare) duty for channel B
	// this PWM output is generated on OC1B pin
	// NOTE:	pwmDuty should be in the range 0-255 for 8bit PWM
	//			pwmDuty should be in the range 0-511 for 9bit PWM
	//			pwmDuty should be in the range 0-1023 for 10bit PWM
	OCR1B = compareValue;
}

void timer1SetCompareValueC(uint16_t compareValue)
{
	// set PWM (output compare) duty for channel C
	// this PWM output is generated on OC1C pin
	// NOTE:	pwmDuty should be in the range 0-255 for 8bit PWM
	//			pwmDuty should be in the range 0-511 for 9bit PWM
	//			pwmDuty should be in the range 0-1023 for 10bit PWM
	OCR1C = compareValue;
}


// timer 3 configuration

void timer3SetMode(uint8_t mode)
{
	TCCR3A &= ~TIMER_MODE_TCCRNA_MASK;
	TCCR3A |= (mode & TIMER_MODE_TCCRNA_MASK);

	TCCR3B &= ~TIMER_MODE_TCCRNB_MASK;
	TCCR3B |= (mode & TIMER_MODE_TCCRNB_MASK);
	/*
	if(mode == TIMER_MODE_NORMAL)
	{
		cbi(TCCR3A,WGM30);
		cbi(TCCR3A,WGM31);
		cbi(TCCR3B,WGM32);
		cbi(TCCR3B,WGM33);
	}
	else if(mode == TIMER_MODE_PWM_PHASECORRECT_8BIT)
	{
		sbi(TCCR3A,WGM30);
		cbi(TCCR3A,WGM31);
		cbi(TCCR3B,WGM32);
		cbi(TCCR3B,WGM33);
	}
	else if(mode == TIMER_MODE_PWM_PHASECORRECT_9BIT)
	{
		cbi(TCCR3A,WGM30);
		sbi(TCCR3A,WGM31);
		cbi(TCCR3B,WGM32);
		cbi(TCCR3B,WGM33);
	}
	else if(mode == TIMER_MODE_PWM_PHASECORRECT_10BIT)
	{
		sbi(TCCR3A,WGM30);
		sbi(TCCR3A,WGM31);
		cbi(TCCR3B,WGM32);
		cbi(TCCR3B,WGM33);
	}
	else if(mode == TIMER_MODE_CTC_OCR)
	{
		cbi(TCCR3A,WGM30);
		cbi(TCCR3A,WGM31);
		sbi(TCCR3B,WGM32);
		cbi(TCCR3B,WGM33);
	}
	else if(mode == TIMER_MODE_FASTPWM_8BIT)
	{
		sbi(TCCR3A,WGM30);
		cbi(TCCR3A,WGM31);
		sbi(TCCR3B,WGM32);
		cbi(TCCR3B,WGM33);
	}
	else if(mode == TIMER_MODE_FASTPWM_9BIT)
	{
		cbi(TCCR3A,WGM30);
		sbi(TCCR3A,WGM31);
		sbi(TCCR3B,WGM32);
		cbi(TCCR3B,WGM33);
	}
	else if(mode == TIMER_MODE_FASTPWM_10BIT)
	{
		sbi(TCCR3A,WGM30);
		sbi(TCCR3A,WGM31);
		sbi(TCCR3B,WGM32);
		cbi(TCCR3B,WGM33);
	}
	else if(mode == TIMER_MODE_PWM_PHASECORRECT_FREQUENCYCORRECT_ICR)
	{
		cbi(TCCR3A,WGM30);
		cbi(TCCR3A,WGM31);
		cbi(TCCR3B,WGM32);
		sbi(TCCR3B,WGM33);
	}
	else if(mode == TIMER_MODE_PWM_PHASECORRECT_FREQUENCYCORRECT_OCR)
	{
		sbi(TCCR3A,WGM30);
		cbi(TCCR3A,WGM31);
		cbi(TCCR3B,WGM32);
		sbi(TCCR3B,WGM33);
	}
	else if(mode == TIMER_MODE_PWM_PHASECORRECT_ICR)
	{
		cbi(TCCR3A,WGM30);
		sbi(TCCR3A,WGM31);
		cbi(TCCR3B,WGM32);
		sbi(TCCR3B,WGM33);
	}
	else if(mode == TIMER_MODE_PWM_PHASECORRECT_OCR)
	{
		sbi(TCCR3A,WGM30);
		sbi(TCCR3A,WGM31);
		cbi(TCCR3B,WGM32);
		sbi(TCCR3B,WGM33);
	}
	else if(mode == TIMER_MODE_CTC_ICR)
	{
		cbi(TCCR3A,WGM30);
		cbi(TCCR3A,WGM31);
		sbi(TCCR3B,WGM32);
		sbi(TCCR3B,WGM33);
	}
	else if(mode == TIMER_MODE_FASTPWM_ICR)
	{
		cbi(TCCR3A,WGM30);
		sbi(TCCR3A,WGM31);
		sbi(TCCR3B,WGM32);
		sbi(TCCR3B,WGM33);
	}
	else if(mode == TIMER_MODE_FASTPWM_OCR)
	{
		sbi(TCCR3A,WGM30);
		sbi(TCCR3A,WGM31);
		sbi(TCCR3B,WGM32);
		sbi(TCCR3B,WGM33);
	}
	*/
}

void timer3SetOutputModeA(uint8_t mode)
{
	if(mode == TIMER_OUTMODE_DISCONNECTED)
	{
		cbi(TCCR3A,COM3A0);
		cbi(TCCR3A,COM3A1);
	}
	else if(mode == TIMER_OUTMODE_TOGGLE)
	{
		sbi(TCCR3A,COM3A0);
		cbi(TCCR3A,COM3A1);
	}
	else if(mode == TIMER_OUTMODE_CLEAR)
	{
		cbi(TCCR3A,COM3A0);
		sbi(TCCR3A,COM3A1);
	}
	else if(mode == TIMER_OUTMODE_SET)
	{
		sbi(TCCR3A,COM3A0);
		sbi(TCCR3A,COM3A1);
	}
}

void timer3SetOutputModeB(uint8_t mode)
{
	if(mode == TIMER_OUTMODE_DISCONNECTED)
	{
		cbi(TCCR3A,COM3B0);
		cbi(TCCR3A,COM3B1);
	}
	else if(mode == TIMER_OUTMODE_TOGGLE)
	{
		sbi(TCCR3A,COM3B0);
		cbi(TCCR3A,COM3B1);
	}
	else if(mode == TIMER_OUTMODE_CLEAR)
	{
		cbi(TCCR3A,COM3B0);
		sbi(TCCR3A,COM3B1);
	}
	else if(mode == TIMER_OUTMODE_SET)
	{
		sbi(TCCR3A,COM3B0);
		sbi(TCCR3A,COM3B1);
	}
}

void timer3SetOutputModeC(uint8_t mode)
{
	if(mode == TIMER_OUTMODE_DISCONNECTED)
	{
		cbi(TCCR3A,COM3C0);
		cbi(TCCR3A,COM3C1);
	}
	else if(mode == TIMER_OUTMODE_TOGGLE)
	{
		sbi(TCCR3A,COM3C0);
		cbi(TCCR3A,COM3C1);
	}
	else if(mode == TIMER_OUTMODE_CLEAR)
	{
		cbi(TCCR3A,COM3C0);
		sbi(TCCR3A,COM3C1);
	}
	else if(mode == TIMER_OUTMODE_SET)
	{
		sbi(TCCR3A,COM3C0);
		sbi(TCCR3A,COM3C1);
	}
}

void timer3SetCompareValueA(uint16_t compareValue)
{
	// set PWM (output compare) duty for channel A
	// this PWM output is generated on OC3A pin
	// NOTE:	pwmDuty should be in the range 0-255 for 8bit PWM
	//			pwmDuty should be in the range 0-511 for 9bit PWM
	//			pwmDuty should be in the range 0-1023 for 10bit PWM
	OCR3A = compareValue;

	// test
	sbi(TIMSK3,OCIE3A);
}

void timer3SetCompareValueB(uint16_t compareValue)
{
	// set PWM (output compare) duty for channel B
	// this PWM output is generated on OC3B pin
	// NOTE:	pwmDuty should be in the range 0-255 for 8bit PWM
	//			pwmDuty should be in the range 0-511 for 9bit PWM
	//			pwmDuty should be in the range 0-1023 for 10bit PWM
	OCR3B = compareValue;
}

void timer3SetCompareValueC(uint16_t compareValue)
{
	// set PWM (output compare) duty for channel C
	// this PWM output is generated on OC3C pin
	// NOTE:	pwmDuty should be in the range 0-255 for 8bit PWM
	//			pwmDuty should be in the range 0-511 for 9bit PWM
	//			pwmDuty should be in the range 0-1023 for 10bit PWM
	OCR3C = compareValue;
}



//! Interrupt handler for tcnt0 overflow interrupt
TIMER_INTERRUPT_HANDLER(TIMER0_OVF_vect)
{
	Timer0Reg0++;			// increment low-order counter

	// increment pause counter
	TimerPauseReg++;

	// if a user function is defined, execute it too
	if(TimerIntFunc[TIMER0OVERFLOW_INT])
		TimerIntFunc[TIMER0OVERFLOW_INT]();
}

TIMER_INTERRUPT_HANDLER(TIMER0_COMPA_vect)
{
	// if a user function is defined, execute it
	if(TimerIntFunc[TIMER0OUTCOMPAREA_INT])
		TimerIntFunc[TIMER0OUTCOMPAREA_INT]();
}

TIMER_INTERRUPT_HANDLER(TIMER0_COMPB_vect)
{
	// if a user function is defined, execute it
	if(TimerIntFunc[TIMER0OUTCOMPAREB_INT])
		TimerIntFunc[TIMER0OUTCOMPAREB_INT]();
}

//! Interrupt handler for tcnt1 overflow interrupt
TIMER_INTERRUPT_HANDLER(TIMER1_OVF_vect)
{
	Timer1Reg0++;			// increment low-order counter

	// if a user function is defined, execute it
	if(TimerIntFunc[TIMER1OVERFLOW_INT])
		TimerIntFunc[TIMER1OVERFLOW_INT]();
}

//! Interrupt handler for CutputCompare1A match (OC1A) interrupt
TIMER_INTERRUPT_HANDLER(TIMER1_COMPA_vect)
{
	// if a user function is defined, execute it
	if(TimerIntFunc[TIMER1OUTCOMPAREA_INT])
		TimerIntFunc[TIMER1OUTCOMPAREA_INT]();
}

//! Interrupt handler for OutputCompare1B match (OC1B) interrupt
TIMER_INTERRUPT_HANDLER(TIMER1_COMPB_vect)
{
	// if a user function is defined, execute it
	if(TimerIntFunc[TIMER1OUTCOMPAREB_INT])
		TimerIntFunc[TIMER1OUTCOMPAREB_INT]();
}

//! Interrupt handler for OutputCompare1B match (OC1C) interrupt
TIMER_INTERRUPT_HANDLER(TIMER1_COMPC_vect)
{
	// if a user function is defined, execute it
	if(TimerIntFunc[TIMER1OUTCOMPAREC_INT])
		TimerIntFunc[TIMER1OUTCOMPAREC_INT]();
}

//! Interrupt handler for InputCapture1 (IC1) interrupt
TIMER_INTERRUPT_HANDLER(TIMER1_CAPT_vect)
{
	// if a user function is defined, execute it
	if(TimerIntFunc[TIMER1INPUTCAPTURE_INT])
		TimerIntFunc[TIMER1INPUTCAPTURE_INT]();
}

//! Interrupt handler for tcnt3 overflow interrupt
TIMER_INTERRUPT_HANDLER(TIMER3_OVF_vect)
{
	Timer3Reg0++;			// increment low-order counter

	// if a user function is defined, execute it
	if(TimerIntFunc[TIMER3OVERFLOW_INT])
		TimerIntFunc[TIMER3OVERFLOW_INT]();
}

//! Interrupt handler for CutputCompare3A match (OC3A) interrupt
TIMER_INTERRUPT_HANDLER(TIMER3_COMPA_vect)
{
	// if a user function is defined, execute it
	if(TimerIntFunc[TIMER3OUTCOMPAREA_INT])
		TimerIntFunc[TIMER3OUTCOMPAREA_INT]();
}

//! Interrupt handler for OutputCompare3B match (OC3B) interrupt
TIMER_INTERRUPT_HANDLER(TIMER3_COMPB_vect)
{
	// if a user function is defined, execute it
	if(TimerIntFunc[TIMER3OUTCOMPAREB_INT])
		TimerIntFunc[TIMER3OUTCOMPAREB_INT]();
}

//! Interrupt handler for OutputCompare3C match (OC3C) interrupt
TIMER_INTERRUPT_HANDLER(TIMER3_COMPC_vect)
{
	// if a user function is defined, execute it
	if(TimerIntFunc[TIMER3OUTCOMPAREC_INT])
		TimerIntFunc[TIMER3OUTCOMPAREC_INT]();
}

//! Interrupt handler for InputCapture3 (IC3) interrupt
TIMER_INTERRUPT_HANDLER(TIMER3_CAPT_vect)
{
	// if a user function is defined, execute it
	if(TimerIntFunc[TIMER3INPUTCAPTURE_INT])
		TimerIntFunc[TIMER3INPUTCAPTURE_INT]();
}

//! Interrupt handler for tcnt1 overflow interrupt
TIMER_INTERRUPT_HANDLER(TIMER4_OVF_vect)
{
	Timer4Reg0++;			// increment low-order counter

	// if a user function is defined, execute it
	if(TimerIntFunc[TIMER4OVERFLOW_INT])
		TimerIntFunc[TIMER4OVERFLOW_INT]();
}

//! Interrupt handler for CutputCompare4A match (OC4A) interrupt
TIMER_INTERRUPT_HANDLER(TIMER4_COMPA_vect)
{
	// if a user function is defined, execute it
	if(TimerIntFunc[TIMER4OUTCOMPAREA_INT])
		TimerIntFunc[TIMER4OUTCOMPAREA_INT]();
}

//! Interrupt handler for OutputCompare4B match (OC4B) interrupt
TIMER_INTERRUPT_HANDLER(TIMER4_COMPB_vect)
{
	// if a user function is defined, execute it
	if(TimerIntFunc[TIMER4OUTCOMPAREB_INT])
		TimerIntFunc[TIMER4OUTCOMPAREB_INT]();
}

//! Interrupt handler for OutputCompare4D match (OC4D) interrupt
TIMER_INTERRUPT_HANDLER(TIMER4_COMPD_vect)
{
	// if a user function is defined, execute it
	if(TimerIntFunc[TIMER4OUTCOMPARED_INT])
		TimerIntFunc[TIMER4OUTCOMPARED_INT]();
}

//! Interrupt handler for Fault Protection interrupt
TIMER_INTERRUPT_HANDLER(TIMER4_FPF_vect)
{
	// if a user function is defined, execute it
	if(TimerIntFunc[TIMER4FAULTPROTECT_INT])
		TimerIntFunc[TIMER4FAULTPROTECT_INT]();
}


