// ------- Preamble -------- //
#define F_CPU 4000000UL /* Tells the Clock Freq to the Compiler. */
#include <avr/io.h> /* Defines pins, ports etc. */
#include <util/delay.h> /* Functions to waste time */
int main(void) {
	// -------- Inits --------- //
	/* Data Direction Register D: writing a one to the bit enables output. */
	VPORTD.DIR = 0b11111111;
	VPORTD.OUT = 0b00000100;
	// ------ Event loop ------ //
	while (1) {
		VPORTD.OUT = 0b11111111; /* Turn on the LED bits/pins in PORTD */
		_delay_ms(1000); /* wait for 1 second */
		VPORTD.OUT = 0b00000000; /* Turn off all D pins/LEDs */
		_delay_ms(1000); /* wait for 1 second */
		} /* End event loop */
		return (0); /* This line is never reached */
	}