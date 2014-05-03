/*! \file ds1307.h \brief DS1307, support for real-time clock for AVR */
//*****************************************************************************
//
//  File Name       : 'ds1307.h'
//  Title           : DS1307 real-time clock support for AVR
//  Original Author : Alan K. Duncan - Copyright (c) 2012
//  Edited by       : jan1s - Copyright (c) 2013
//  Created         : 2012-03-30
//  Modified        : 2013-09-01
//  Version         : 1.2
//  Target MCU      : Atmel AVR series
//
// This code is distributed under the GNU Public License
//		which can be found at http://www.gnu.org/licenses/gpl.txt
//
//*****************************************************************************

#ifndef DS1307_H_
#define DS1307_H_

#include <stdint.h>
#include "global.h"

//! Initialize the DS1307 device with hour mode
void ds1307Init(void);
void ds1307EnableOscillator(void);
void ds1307DisableOscillator(void);
void ds1307SetHourMode(uint8_t mode);
void ds1307SetSquarewaveOutput(uint8_t enable, uint8_t rate);

uint8_t ds1307GetSeconds(void);
uint8_t ds1307GetMinutes(void);
uint8_t ds1307GetHours(void);
uint8_t ds1307GetDate(void);
uint8_t ds1307GetMonth(void);
uint8_t ds1307GetYear(void);

void ds1307SetSeconds(uint8_t seconds);
void ds1307SetMinutes(uint8_t minutes);
void ds1307SetHours(uint8_t hours);
void ds1307SetDate(uint8_t date);
void ds1307SetMonth(uint8_t month);
void ds1307SetYear(uint8_t year);

#endif /* DS1307_H_ */