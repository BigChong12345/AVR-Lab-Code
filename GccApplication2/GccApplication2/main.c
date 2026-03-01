/*
 * GccApplication2.c
 *
 * Created: 2/9/2026 1:32:04 PM
 * Author : drcla
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>

#define BTN1 (!(VPORTB.IN & PIN2_bm)) // macro checks if on-board button pressed
#define BTN2 (!(VPORTB.IN & PIN5_bm)) // macro checks if off-board button pressed

void init_clock() {
	CPU_CCP = CCP_IOREG_gc;
	CLKCTRL.XOSCHFCTRLA =  CLKCTRL_FRQRANGE_16M_gc | CLKCTRL_ENABLE_bm;
	CPU_CCP = CCP_IOREG_gc;
	CLKCTRL.MCLKCTRLA = CLKCTRL_CLKSEL_EXTCLK_gc;
	while(!(CLKCTRL.MCLKSTATUS & CLKCTRL_EXTS_bm));
}

void my_delay(double time_ms)
{
	for (int i=0; i<time_ms; i++) {
		_delay_ms(1);
	}
}

int main(void)
{
    /* Replace with your application code */
    init_clock();
	PORTB.PIN2CTRL |= PORT_PULLUPEN_bm;
	PORTB.DIRCLR = (PIN2_bm | PIN5_bm);
	PORTD.DIR = 0b11111111; //setting PD0 to PD7 as output;
	PORTD.OUT = 0b00000000; // initializing every LED to off;
	uint8_t step = 0b00000001;
	uint8_t initial = PIN2_bm;
	uint8_t mode = 0;
	uint8_t direction = 1;
	double freq = 2.0;
	bool button1handled = false;
	bool button2handled = false;
	uint8_t sequence[4] = {0};
	uint8_t count = 0;
	uint8_t wrap = 0;
	while (1) 
    {	
		if(BTN2) { // button has been pressed
			if (button2handled == false) {
				for (int i = 3; i > 0; i--) {
					sequence[i] = sequence[i-1];
				}
				sequence[0] = 0;
				//sequence[count] = 0;
				//count = (count + 1) % 4;
				button2handled = true;
				freq = (freq > 3.9) ? 0.5 : freq * 2;
				}
			
		}
		else {
			button2handled = false;
		} 

		if(BTN1) { // button has been pressed
			if (button1handled == false) {
				for (int i = 3; i > 0; i--) {
					sequence[i] = sequence[i-1];
				}
				sequence[0] = 1;
				button1handled = true;
				mode = (mode == 2) ? 0 : mode + 1;
				if (mode == 2) {
					direction = 1;
				}
			}
		}
		else {
			button1handled = false;
		}
		if (sequence[0] == 1 && sequence[1] == 1 && sequence[2] == 0 && sequence[3] == 1) {
			initial = 0;
			for (int i = 0; i < 4; i++) {
				sequence[i] = 0;
			}
		}
		PORTD.OUT = initial;
		my_delay(1000/freq);
	
		if (mode == 0) {
			if (initial == 0b11111111)
				initial = 0b00000000;
			else
				initial += step;
		}
		else if (mode == 1) {
			wrap = initial >> 7;
			initial = initial << 1;
			initial |= wrap;
		}
		else {
			if (initial == 0b11111111) {
				direction = 0;
			}
			else if (initial == 0b00000000) {
				direction = 1;
			}
			if (direction == 1) {
				initial = initial << 1;
				initial |= 1;
			}
			else { // direction is zero
				initial = initial >> 1;
			}
		}
		
    }
}

