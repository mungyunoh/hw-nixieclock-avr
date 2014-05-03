/*! \file spi.c \brief SPI interface driver. */
//*****************************************************************************
//
// File Name	: 'spi.c'
// Title		: SPI interface driver
// Author		: Pascal Stang - Copyright (C) 2000-2002
// Created		: 11/22/2000
// Revised		: 06/06/2002
// Version		: 0.6
// Target MCU	: Atmel AVR series
// Editor Tabs	: 4
//
// NOTE: This code is currently below version 1.0, and therefore is considered
// to be lacking in some functionality or documentation, or may not be fully
// tested.  Nonetheless, you can expect most functions to work.
//
// This code is distributed under the GNU Public License
//		which can be found at http://www.gnu.org/licenses/gpl.txt
//
//*****************************************************************************

#include <avr/io.h>
#include <avr/interrupt.h>

#include "spi.h"

// access routines
void spiInit()
{
	DDRB |= (1<<2) | (1<<1) | (1<<0);
	DDRB &= ~(1<<3);

	//PORTB |= (1<<0);

	SPCR = (1<<SPE) | (1<<MSTR) | (1<<SPR0);
}

void spiSendByte(uint8_t data)
{
	// send a byte over SPI and ignore reply
	SPDR = data;
	while(!(SPSR & (1<<SPIF)));
}

uint8_t spiTransferByte(uint8_t data)
{
	// send the given data
	SPDR = data;
	// wait for transfer to complete
	while(!(SPSR & (1<<SPIF)));
	// return the received data
	return SPDR;
}

uint16_t spiTransferWord(uint16_t data)
{
	uint16_t rxData = 0;

	// send MS byte of given data
	rxData = (spiTransferByte((data>>8) & 0x00FF))<<8;
	// send LS byte of given data
	rxData |= (spiTransferByte(data & 0x00FF));

	// return the received data
	return rxData;
}
