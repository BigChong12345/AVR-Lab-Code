/*
 * GccApplication3.c
 *
 * Created: 2/25/2026 1:29:43 PM
 * Author : drcla
 */ 

#define F_CPU 16000000UL


#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "uart.h"

void my_delay(double time_ms)
{
	for (int i=0; i<time_ms; i++)
	_delay_ms(1);
}

void init_clock() {
	CPU_CCP = CCP_IOREG_gc;
	CLKCTRL.XOSCHFCTRLA =  CLKCTRL_FRQRANGE_16M_gc | CLKCTRL_ENABLE_bm;
	CPU_CCP = CCP_IOREG_gc;
	CLKCTRL.MCLKCTRLA = CLKCTRL_CLKSEL_EXTCLK_gc;
	while(!(CLKCTRL.MCLKSTATUS & CLKCTRL_EXTS_bm));
}

volatile char inputchar = 0;

ISR(USART3_RXC_vect)
{
	inputchar = USART3.RXDATAL;
}

int main(void)
{
    /* Replace with your application code */
	
	//part a initialization
	init_clock();
	PORTD.DIR = 0b11111111;
	char position = 1; // this is the initial LED being blinked for part a
	int freq = 2; // set initial frequency to 2 Hz for part a
	
	//uart and interrupt initialization
	uart_init(3, 19200, NULL);
	USART3.CTRLA |= USART_RXCIE_bm; // enable receive interrupt
	sei();
	
	//numeric input initialization
	int num[2] = {-1, -1}; // stores the two digit number
	uint8_t i = 0; // indexes the array
	
	//part d button initialization
	PORTB.DIRCLR = PIN2_bm; //set Pin 2 on port B to an input
	PORTB.PIN2CTRL = PORT_PULLUPEN_bm; // enable pull up
	uint8_t mode = 0; // mode 0 by default
	char buttonHandled = 0;
	int timepassed_ms = 0; // keeps track of time passed
	char mode1input;
	
    while (1) 
    {
        VPORTD.OUT = 1 << position;
        my_delay(500/freq);
        VPORTD.OUT = 0;
        my_delay(500/freq);
		if (mode == 1) {
			timepassed_ms += (1000/freq);
		}
		
		if( !( PORTB.IN & PIN2_bm ) ) {
			_delay_ms(10);
			if (!(PORTB.IN & PIN2_bm)) { // debouncing
				if (!buttonHandled) {
					mode = (mode + 1) % 2; // toggles between mode 0 and mode 1
					printf("Mode has been changed to %d. \n", mode);
					timepassed_ms = 0; //reset this every time we change modes
					buttonHandled = 1;
				}
			}
		}
		else {
			buttonHandled = 0;
		}
		//part b
		if (mode == 1) {
			USART3.CTRLA &= 0 << USART_RXCIE_bp; // turn off interrupts when in mode 1
			//cli();
			if (timepassed_ms > 2000) {// if two seconds have passed
				timepassed_ms = 0;
				printf("Enter a valid key (w,a,s,d) \n");
				scanf(" %c", &mode1input);
				while (mode1input != 'w' && mode1input != 'a' && mode1input != 's' && mode1input != 'd') {
					printf("invalid character for mode 1. Please enter either w,a,s,d \n");
					scanf(" %c", &mode1input);
				}
						
				if (mode1input == 'w') {
					freq += 1;
					printf("New frequency: %d \n", freq); // part d
				}
				else if (mode1input == 'a') {
					position = (position > 0) ? position - 1 : 0; // no further than PD0
				}
				else if (mode1input == 's') {
					freq = (freq > 1) ? freq - 1 : 1; // no less than 1 Hz
					printf("New frequency: %d \n", freq); // part d
				}
				else if (mode1input == 'd') {
					position = (position < 7) ? position + 1 : 7; // no further than PD7
				}
			}
		}
		else { // mode = 0
			USART3_CTRLA |= USART_RXCIE_bm; // turn back on the UART receive interrupt;
			//sei();
			if (inputchar != 0) {
				char c = inputchar;
				inputchar = 0;
			
				if (c == 'w') {
					freq += 1;
					printf("New frequency: %d \n", freq); // part d
				}
				else if (c == 'a') {
					position = (position > 0) ? position - 1 : 0; // no further than PD0
				}
				else if (c == 's') {
					freq = (freq > 1) ? freq - 1 : 1; // no less than 1 Hz
					printf("New frequency: %d \n", freq); // part d
				}
				else if (c == 'd') {
					position = (position < 7) ? position + 1 : 7; // no further than PD7
				}
			

				// part c, taking in numerical input, should handle 2 digit inputs
			
				else if (c >= '0' && c <= '9') { // if it is a number 
					if (i < 2) {
						num[i] = c - '0';
						i++;
					}
					else {
						printf("Two numbers have been entered: %d, %d. Please hit the return key. \n", num[0], num[1]);
					}
				}
			
				else if (c == '\r') {
					if (num[0] == -1) { // user has not given any input
						printf("Please enter a number before pressing return. \n");
					}
					else if (num[0] == 0 && (num[1] == 0 || num[1] == -1)) { // user has given '0' or '0' '0', which are not valid inputs
						printf("Frequency cannot be zero. \n");
					}
					else {
						if (num[1] == -1) { // user has entered a one digit input
							freq = num[0];
						}
						else { // user has entered a two digit input
							freq = (10* num[0]) + (num[1]);		
						}
					
						printf("New frequency: %d \n", freq); // part d
					}
					//reset variables
					num[0] = -1;
					num[1] = -1;
					i = 0;
				}
			
				else {
					printf("invalid character \n");
				}
			} 
		}
    }
}

