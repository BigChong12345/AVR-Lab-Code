/*
 * Lab2.c
 *
 * Created: 1/27/2026 6:56:35 PM
 * Author : drcla
 */ 
#define F_CPU 4000000UL /* Tells the Clock Freq to the Compiler. */
#include <util/delay.h> /* Functions to waste time */
#include <avr/io.h>


int main(void)
{
    /* Replace with your application code */
	VPORTD.DIR = 0b11111111;
	VPORTD.OUT = 0b00000000;
	uint16_t seconds = 0;
	uint8_t shift = 0;
    while (1) 
    {
		//
		while (seconds != 2000) {
			VPORTD.OUT |= (1 << shift);
			_delay_ms(250);
			VPORTD.OUT &= ~(1 << shift);
			_delay_ms(250);
			seconds += 500;
		}
		seconds = 0;
		
		while (seconds != 2000) {
			VPORTD.OUT |= (1 << shift);
			_delay_ms(125);
			VPORTD.OUT &= ~(1 << shift);
			_delay_ms(125);
			seconds += 250;
		}
		
		shift = (shift == 7) ? 0 : shift + 1;
		seconds = 0;
    }
}

