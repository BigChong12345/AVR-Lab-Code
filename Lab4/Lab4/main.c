/*
 * Lab4.c
 *
 * Created: 2/2/2026 2:44:48 PM
 * Author : drcla
 */ 
#define F_CPU 16000000UL

#include "uart.h"
#include <avr/io.h>
#include <stdio.h>
#include <string.h>
#include <util/delay.h>

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
	init_clock();
    /* Replace with your application code */
	uart_init(3, 9600, NULL);
	uint8_t freq = 2;
	double counter = 0;
	char input = ' ';
	uint8_t position = 0;	
	PORTD_DIRSET = 0b11111111;
	PORTD_OUT = 0b00000000;
	while (1) 
    {
		if (counter == 0 || counter >= 5000) {
			counter = 0;
			printf("Do you want to change the frequency or position? \n");
			scanf(" %c", &input);
			while (input != 'F' && input != 'P') {
				printf("Invalid input. Please enter F to change frequency or P to change position. \n Input:");
				scanf(" %c", &input);
			}
			if (input == 'F') {
				printf("Frequency: ");
				scanf("%d", &freq);
				while (freq < 1 || freq > 10) {
					printf("Invalid input. Frequency needs to be between 1-10. \n Frequency: ");
					scanf("%d", &freq);
				}
				printf("Frequency changed to %d. \n", freq);
				
			}
			else if (input == 'P') {
				printf("Position: ");
				scanf("%d", &position);
				while (position < 0 || position > 7) {
					printf("Invalid input. Position needs to be between 0-7. \n Position: ");
					scanf("%d", &position);
				}
				
				printf("Position changed to %d \n", position);
			}

		}
		input = ' ';
		PORTD.OUTSET = 1 << position;
		my_delay(1000.0/(2*freq));
		PORTD_OUTCLR = 1 << position;
		my_delay(1000.0/(2*freq));
		
		counter += 1000.0/freq;
	}
}

