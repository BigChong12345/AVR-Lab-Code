/*
 * Lab5.c
 *
 * Created: 2/4/2026 3:15:35 PM
 * Author : drcla
 */ 

#define F_CPU 16000000UL


#include <avr/io.h>
#include "uart.h"
#include <util/delay.h>
#include <avr/interrupt.h>

volatile uint8_t flag;
volatile uint8_t msg;

enum STATE {UNKNOWN, FREQ, POS};


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
	enum STATE currentstate = UNKNOWN;
	init_clock();
	uart_init(3,9600, NULL);
	printf("Do you want to change the frequency or position (F/P)? \n");
	USART3.CTRLA |= (USART_RXCIE_bm);
	PORTD_DIRSET = 0b11111111;
	PORTD.OUT = 0b00000000;
	uint8_t f = 2;
	uint8_t position = 2;
	sei();
    while (1) 
    {
		if (flag == 1)	 {
			cli();
			if (flag == 1) { 
				flag = 0;
			}
			sei();
			if (currentstate == UNKNOWN) {
				if (msg!= 'f' && msg != 'p') {
					printf("Invalid input. Please change the frequency or position. \n");
					currentstate = UNKNOWN;
				}
				else {
					if (msg == 'f') {
						currentstate = FREQ;
						printf("Enter a frequency between 1-10 (Hz): \n");
					}
					else {
						currentstate = POS;
						printf("Enter a position between 0-7: \n");
					}
				}
			}
			else if (currentstate == FREQ) {
				if (msg < '1' || msg > '9')
					printf("Invalid frequency.");
				else {
					f = msg - '0';
					printf("Do you want to change the frequency or position (F/P)? \n");
					currentstate = UNKNOWN;
				}
			}
			else {
				if (msg > '7') 
					printf("Invalid position.");
				else {
					position = msg - '0';
					printf("Do you want to change the frequency or position (F/P)? \n");
					currentstate = UNKNOWN;
				}
			}
		}
	PORTD_OUT = (1 << position);
	my_delay(500/f);
	PORTD_OUT = 0b00000000;
	my_delay(500/f);
    }
}

ISR(USART3_RXC_vect) {
	flag = 1;
	msg = USART3.RXDATAL;
}
