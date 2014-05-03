#include <avr/io.h>
#include <avr/pgmspace.h>
#include "time.h"
#include "spi.h"

#include "display.h"


#define RCK_CONFIG	(DDRB |= (1<<5))
#define RCK_ON		(PORTB |= (1<<5))
#define RCK_OFF		(PORTB &= ~(1<<5))

#define SCL_CONFIG	(DDRB |= (1<<4))
#define SCL_ON		(PORTB |= (1<<4))
#define SCL_OFF		(PORTB &= ~(1<<4))

#define EN_CONFIG	(DDRD |= (1<<7))
#define EN_ON		(PORTD |= (1<<7))
#define EN_OFF		(PORTD &= ~(1<<7))

#define HVEN_CONFIG	(DDRC |= (1<<6))
#define HVEN_ON		(PORTC |= (1<<6))
#define HVEN_OFF	(PORTC &= ~(1<<6))


/*
ZM1000 - Shiftregister layout

(  Tube6  )(  Tube5  )  (  Tube4  ) (  Tube3  )  (  Tube2  )(  Tube1  ) <- Tubes
67098123 45670981 2345**67 09812345 67098123 45**6709 81234567 09812345 <- Tube digits
|||||||| |||||||| |||||||| |||||||| |||||||| |||||||| |||||||| |||||||| 
HGFEDCBA HGFEDCBA HGFEDCBA HGFEDCBA HGFEDCBA HGFEDCBA HGFEDCBA HGFEDCBA <- Shiftregister Outputs
 Reg. 8   Reg. 7   Reg. 6   Reg. 5   Reg. 4   Reg. 3   Reg. 2   Reg. 1  <- Shiftregisters

The mapping of the tube pins is repeating in 10bit periods.
Therefore it is possible to use following constants to select the digits to display:

  |     Nixie-tube    |
  \___________________/
   | | | | | | | | | |
   6 7 0 9 8 1 2 3 4 5 

0b 0 0 1 0 0 0 0 0 0 0  0x0080  <- Displays 0
0b 0 0 0 0 0 1 0 0 0 0  0x0010  <- Displays 1
0b 0 0 0 0 0 0 1 0 0 0  0x0008  <- Displays 2
0b 0 0 0 0 0 0 0 1 0 0  0x0004  <- Displays 3
0b 0 0 0 0 0 0 0 0 1 0  0x0002  <- Displays 4
0b 0 0 0 0 0 0 0 0 0 1  0x0001  <- Displays 5
0b 1 0 0 0 0 0 0 0 0 0  0x0200  <- Displays 6
0b 0 1 0 0 0 0 0 0 0 0  0x0100  <- Displays 7
0b 0 0 0 0 1 0 0 0 0 0  0x0020  <- Displays 8
0b 0 0 0 1 0 0 0 0 0 0  0x0040  <- Displays 9

The bitwurst is shifted from right to left in the shiftregisters shown above.
After every two tubes are two dots to seperate the seconds, minutes and hours.
*/

// for ZM1000 
//const uint16_t digitMapping[] PROGMEM = {0x0080, 0x0010, 0x0008, 0x0004, 0x0002, 0x0001, 0x0200, 0x0100, 0x0020, 0x0040};

/*
Z570M - Shiftregister layout

(  Tube6  )(  Tube5  )  (  Tube4  ) (  Tube3  )  (  Tube2  )(  Tube1  ) <- Tubes
12345678 90123456 7890**12 34567890 12345678 90**1234 56789012 34567890 <- Tube digits
|||||||| |||||||| |||||||| |||||||| |||||||| |||||||| |||||||| |||||||| 
HGFEDCBA HGFEDCBA HGFEDCBA HGFEDCBA HGFEDCBA HGFEDCBA HGFEDCBA HGFEDCBA <- Shiftregister Outputs
 Reg. 8   Reg. 7   Reg. 6   Reg. 5   Reg. 4   Reg. 3   Reg. 2   Reg. 1  <- Shiftregisters

The mapping of the tube pins is repeating in 10bit periods.
Therefore it is possible to use following constants to select the digits to display:

  |     Nixie-tube    |
  \___________________/
   | | | | | | | | | |
   1 2 3 4 5 6 7 8 9 0 

0b 0 0 0 0 0 0 0 0 0 1  0x0001  <- Displays 0
0b 1 0 0 0 0 0 0 0 0 0  0x0200  <- Displays 1
0b 0 1 0 0 0 0 0 0 0 0  0x0100  <- Displays 2
0b 0 0 1 0 0 0 0 0 0 0  0x0080  <- Displays 3
0b 0 0 0 1 0 0 0 0 0 0  0x0040  <- Displays 4
0b 0 0 0 0 1 0 0 0 0 0  0x0020  <- Displays 5
0b 0 0 0 0 0 1 0 0 0 0  0x0010  <- Displays 6
0b 0 0 0 0 0 0 1 0 0 0  0x0008  <- Displays 7
0b 0 0 0 0 0 0 0 1 0 0  0x0004  <- Displays 8
0b 0 0 0 0 0 0 0 0 1 0  0x0002  <- Displays 9

The bitwurst is shifted from right to left in the shiftregisters shown above.
After every two tubes are two dots to seperate the seconds, minutes and hours.
*/

// for Z570M 
const uint16_t digitMapping[] PROGMEM = {0x0001, 0x0200, 0x0100, 0x0080, 0x0040, 0x0020, 0x0010, 0x0008, 0x0004, 0x0002};



typedef struct
{
	uint8_t digits[6];	// digits increment from right to left
	uint8_t dotBR:1;	// dot bottom right
	uint8_t dotUR:1;	// dot upper right
	uint8_t dotBL:1;	// dot bottom left
	uint8_t dotUL:1;	// dot upper left
} display_t;

// global cache for actual displayed value
display_t display;


void displayInit(void)
{
	spiInit();
	RCK_CONFIG;
	SCL_CONFIG;
	EN_CONFIG;

	EN_ON;
	SCL_OFF;
	SCL_ON;
	RCK_OFF;

	display.dotBR = 1;
	display.dotUR = 1;
	display.dotBL = 1;
	display.dotUL = 1;

	displayHighVoltageEnable();
}

void displayTime(time_t time)
{
	tmElements_t el;
	timeBreak(time, &el);

	display_t *pdisplay = &display;

	pdisplay->digits[5] = el.Second % 10;
	pdisplay->digits[4] = el.Second / 10;
	pdisplay->digits[3] = el.Minute % 10;
	pdisplay->digits[2] = el.Minute / 10;
	pdisplay->digits[1] = el.Hour % 10;
	pdisplay->digits[0] = el.Hour / 10;

	displayShow();
}


void displayShow()
{
	//Clear all shift register entries (not needed but possible)
	SCL_OFF;
	SCL_ON;

	EN_ON;

	uint8_t byte_out = 0;
	uint8_t byte_out_bit_count = 0;
	uint8_t byte_out_mask = 0x80;

	for (uint8_t digit_count = 0; digit_count < 6; digit_count++)
	{
		// catch separation dots
		// nicer way would be nice
		if(digit_count == 2)
		{
			if(display.dotBR)
			{
				byte_out |= byte_out_mask;
			}
			byte_out_mask = byte_out_mask >> 1;
			byte_out_bit_count++;

			if(display.dotUR)
			{
				byte_out |= byte_out_mask;
			}
			byte_out_mask = byte_out_mask >> 1;
			byte_out_bit_count++;
		}
		else if(digit_count == 4)
		{
			if(display.dotBL)
			{
				byte_out |= byte_out_mask;
			}
			byte_out_mask = byte_out_mask >> 1;
			byte_out_bit_count++;

			if(display.dotUL)
			{
				byte_out |= byte_out_mask;
			}
			byte_out_mask = byte_out_mask >> 1;
			byte_out_bit_count++;
		}

		// get the mapping for the digit to be displayed
		uint16_t digit_value = pgm_read_word(&(digitMapping[display.digits[digit_count]]));

		// regular digit to output conversion
		for(uint8_t digit_bit_count = 0; digit_bit_count < 10; digit_bit_count++)
		{
			if(digit_value & 0x0200) 
			{
				byte_out |= byte_out_mask;
			}
			digit_value = digit_value << 1;
			byte_out_mask = byte_out_mask >> 1;
			byte_out_bit_count++;
			if(byte_out_bit_count == 8)
			{
				spiSendByte(byte_out);
				byte_out_bit_count = 0;
				byte_out = 0;
				byte_out_mask = 0x80;
			}
		}
	}

	RCK_ON;
	RCK_OFF;

	EN_OFF;
}

uint8_t displayHighVoltageRead()
{
	// placeholder. should use a2d, but saving code for now.
	return 0x00;
}

void displayHighVoltageEnable()
{
	HVEN_ON;
}

void displayHighVoltageDisable()
{
	HVEN_OFF;
}