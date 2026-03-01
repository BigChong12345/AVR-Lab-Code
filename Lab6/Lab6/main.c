/*
 * Lab6.c
 *
 * Created: 2/16/2026 9:36:07 AM
 * Author : drcla
 
 
 Implement a system to measure the Human Reaction Time down to a resolution of 1ms.
 In particular:
 1. Print a message on UART for the user to get ready
 2. Wait for some random amount of time, e.g. between 2 to 5 seconds
 3. Turn on a LED & start timer count
 4. The user is supposed to push a button as soon as the LED turns on
 5. Read time count to measure the time between the two events, i.e. turning on the LED
 and detecting a button push
 6. Print the reaction time in milliseconds on UART
 7. Go back to 1.
 */ 
#define F_CPU 16000000UL

#include <avr/io.h>
#include "uart.h"
#include <util/delay.h>
#include <stdlib.h>
#include <stdbool.h>
#include <avr/interrupt.h>

volatile uint16_t timerCount = 0;
volatile bool start_counting = false;
volatile uint16_t reaction_time = 0;

#define BTN1 (!(VPORTB.IN & PIN2_bm))
//#define BTN2 (!(VPORTB.IN & PIN5_bm))

typedef enum {RELEASED, MAYBE_PUSHED,
	PUSHED, MAYBE_RELEASED
} btn_state_t;

typedef enum {START, WAITING_FOR_LED, LED_ON, 
	WAITING_FOR_PRESS, STOP_TIMER
} usr_state_t;

// 1ms ISR for Timer TCA0 assuming F_CPU = 16MHz
void InitTCA0(void)
{
	TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_NORMAL_gc; // Normal mode
	TCA0.SINGLE.PER = 249; // Set number of ticks for period
	TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm; // Enable TCA0 Overflow ISR
	// Set Prescalar to 64 & enable timer. Each tick is 4us
	TCA0.SINGLE.CTRLA |= (TCA_SINGLE_CLKSEL_DIV64_gc | TCA_SINGLE_ENABLE_bm);
}

void init_clock() {
	CPU_CCP = CCP_IOREG_gc;
	CLKCTRL.XOSCHFCTRLA =  CLKCTRL_FRQRANGE_16M_gc | CLKCTRL_ENABLE_bm;
	CPU_CCP = CCP_IOREG_gc;
	CLKCTRL.MCLKCTRLA = CLKCTRL_CLKSEL_EXTCLK_gc;
	while(!(CLKCTRL.MCLKSTATUS & CLKCTRL_EXTS_bm));
}

int main(void)
{
    /* Replace with your application code */
	PORTB.PIN2CTRL = PORT_PULLUPEN_bm;
	PORTB.DIRCLR = (PIN2_bm);
	PORTD.DIRSET = PIN0_bm;
	
	init_clock();
	uart_init(3, 9600, NULL);
	long r;
	InitTCA0();
	sei();
	usr_state_t user_state = START;
	
    while (1) 
    {
		switch(user_state) {
			case START: 
				printf("Get ready... ");
				r = 2000 + random() % 3000;
				timerCount = r;
				reaction_time = 0;
				start_counting = false;
				user_state = WAITING_FOR_LED;
				break;
			case WAITING_FOR_LED:
				if (timerCount == 0) {
					user_state = LED_ON;
				}
				break;
			case LED_ON:
				PORTD.OUTSET = PIN0_bm;
				start_counting = true;
				user_state = WAITING_FOR_PRESS;
				break;
			case WAITING_FOR_PRESS:
				if (BTN1) {
					start_counting = false;
					user_state = STOP_TIMER;
				}
				break;
			case STOP_TIMER: 
				PORTD.OUTCLR = PIN0_bm;
				printf("Your reaction time is: %u ms \r\n", reaction_time);
				user_state = START;
				break;
				
		}
		
    }
}

ISR(TCA0_OVF_vect) {
	if (timerCount > 0)
		timerCount--;
	if (start_counting) {
		reaction_time++;
	}
	TCA0.SINGLE.INTFLAGS |= TCA_SINGLE_OVF_bm;
}
