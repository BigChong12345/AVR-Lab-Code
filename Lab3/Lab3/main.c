/*
 * Lab3.c
 *
 * Created: 1/28/2026 2:23:38 PM
 * 
 */ 

#define F_CPU 16000000UL /* Tells the Clock Freq to the Compiler. */
#include <avr/io.h> /* Defines pins, ports etc. */
#include <util/delay.h> /* Functions to waste time */

void my_delay(double time_ms)
{
	for (int i=0; i<time_ms; i++) {
		_delay_ms(1);
	}
}

int main(void)
{
    /* Replace with your application code */
	PORTB.PIN2CTRL |= PORT_PULLUPEN_bm;
	PORTB.DIRCLR = (PIN2_bm) | (PIN5_bm);
	
	VPORTD.DIR = 0b11111111;
	VPORTD.OUT = 0b00100000;
	int freq = 1;
	int buttonHandled = 0;
	int buttonHandled2 = 0;
	int bothButtonsHandled = 0;
	int position = 5;
	int direction = -1;
    while (1) 
    {
		if(!(PORTB.IN & PIN2_bm)) {
			_delay_ms(10);
			if(!(PORTB.IN & PIN2_bm)) {
				if (!buttonHandled) {
					buttonHandled = 1;
					freq++;
				}
			}
		}
		else {
			buttonHandled = 0;
		}		
		
		if(!(PORTB.IN & PIN5_bm)) {
			_delay_ms(10);
			if (!(PORTB.IN & PIN5_bm)) {
				if (!buttonHandled2) {
					buttonHandled2 = 1;
					freq = (freq != 1) ? freq - 1 : 1;
				}
			}
		}
		else {
			buttonHandled2 = 0;
		}		
		
		if (buttonHandled && buttonHandled2) {
			_delay_ms(10);
			if (buttonHandled && buttonHandled2) {
				if (!bothButtonsHandled) {
					bothButtonsHandled = 1;	
					if (position == 0) {
						direction = 1;
					}
					else if (position == 7) {
						direction = -1;
					}
					position += direction;
				}
			}
		}
		
		else if (!buttonHandled && !buttonHandled2) {
			bothButtonsHandled = 0;
		}
		
		VPORTD.OUT |= (1 << position);
		my_delay(1000.0/(2*freq));
		VPORTD.OUT &= ~(1 << position);
		my_delay(1000.0/(2*freq));
		
	}
	
}

