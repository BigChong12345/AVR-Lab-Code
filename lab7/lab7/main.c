/*
 * lab7.c
 *
 * Created: 2/27/2026 1:40:25 PM
 * Author : drcla
 */ 


//todo: CMP0 is 16 bit and might not be able to take that large values, need to find a way to solve that
//may need to fix race conditions
// still need to implement printing every 5 seconds

#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdbool.h>
#include <avr/interrupt.h>
#include "uart.h"

#define BTN0 (!(PORTB.IN & PIN2_bm)) // btn0 is on board button
#define BTN1 (!(PORTB.IN & PIN5_bm)) // btn1 is off board button on PB5

#define DEBOUNCE_TIME 10
#define PRINT_COUNT 5000

//flags volatile because they go in interrupt
volatile bool btn0Pushed = false;
volatile bool btn1Pushed = false;
volatile uint8_t debounce_timerCount = 0;
volatile uint16_t print_timerCount = PRINT_COUNT;

// 1ms ISR for Timer TCA0 assuming F_CPU = 16MHz
// this is the fixed timer, used for timing print and debounce
void InitTCA0(void)
{
	TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_NORMAL_gc; // Normal mode
	TCA0.SINGLE.PER = 249; // Set number of ticks for period
	TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm; // Enable TCA0 Overflow ISR
	// Set Prescalar to 64 & enable timer. Each tick is 4us
	TCA0.SINGLE.CTRLA |= (TCA_SINGLE_CLKSEL_DIV64_gc | TCA_SINGLE_ENABLE_bm);
}

// freq generation formula: freq = 62500/(2*(1+CMP0)), CMP0 = (31250/freq) - 1, timepassed_ms = 1000/freq = 20*(1+CMP0)/(625)
// this is the waveform generation timer that routes the waveform to PB0, controlling LED blink freq
void InitTCA1(void)
{
	TCA1.SINGLE.CTRLB = TCA_SINGLE_WGMODE_FRQ_gc;// FRQ mode
	TCA1.SINGLE.CMP0 = 249; // Set number of ticks for period
	//TCA1.SINGLE.INTCTRL = TCA_SINGLE_CMP0_bm; // Enable TCA0 CMP0 ISR
	TCA1.SINGLE.CTRLB |= TCA_SINGLE_CMP0EN_bm; // Enable waveform output
	
	TCA1.SINGLE.CTRLA |= (TCA_SINGLE_CLKSEL_DIV256_gc | TCA_SINGLE_ENABLE_bm); // set prescalar to 
	
	PORTMUX.TCAROUTEA |= PORTMUX_TCA1_PORTC_gc;  // waveform will go to PB0
	PORTC.DIRSET = PIN4_bm; // setting PC0 as output;
}

// TCA0 is timer0 and keeps track of things happening at fixed times like debouncing and printing
ISR(TCA0_OVF_vect) {
	if (print_timerCount > 0) {
	 print_timerCount--;
	}
	if (debounce_timerCount > 0) {
		debounce_timerCount--;
		if (debounce_timerCount == 0) { // check both buttons with same flag at same time
			if (BTN0)
			btn0Pushed = true;
			if (BTN1)
			btn1Pushed = true;
		}
	}
	TCA0.SINGLE.INTFLAGS |= TCA_SINGLE_OVF_bm;
}

// this is the ISR handling PB2 and PB5, when the button is pressed a falling edge is triggered and sent to this ISR
// debounce_timerCount should be set here because the debounce timer starts whenever the button is pressed
ISR(PORTB_PORT_vect) // port B interrupt
{
	if (PORTB.INTFLAGS & PIN2_bm) { // interrupt caused by BTN0 being pressed 
		PORTB.INTFLAGS = PIN2_bm; // clear interrupt
		if (debounce_timerCount == 0) {
			debounce_timerCount = DEBOUNCE_TIME;
		}
	}
	
	if (PORTB.INTFLAGS & PIN5_bm) { // interrupt caused by BTN1 being pressed
		PORTB.INTFLAGS = PIN5_bm; // clear interrupt
		if (debounce_timerCount == 0) {
			debounce_timerCount = DEBOUNCE_TIME;
		}
	}	
}

void init_clock() {
	CPU_CCP = CCP_IOREG_gc;
	CLKCTRL.XOSCHFCTRLA =  CLKCTRL_FRQRANGE_16M_gc | CLKCTRL_ENABLE_bm;
	CPU_CCP = CCP_IOREG_gc;
	CLKCTRL.MCLKCTRLA = CLKCTRL_CLKSEL_EXTCLK_gc;
	while(!(CLKCTRL.MCLKSTATUS & CLKCTRL_EXTS_bm));
}

void initButtonInt() {
	//set both buttons to falling edge
	PORTB.PIN2CTRL = PORT_ISC_FALLING_gc | PORT_PULLUPEN_bm; // pull up resistor needed for on board button
	PORTB.PIN5CTRL = PORT_ISC_FALLING_gc; // no need for pull up because there's an external one 

	PORTB.DIRCLR = (PIN2_bm | PIN5_bm); // set both buttons to inputs
}

int main(void)
{
    /* Replace with your application code */
	// initialize stuff
	init_clock();
	uart_init(3, 9600, NULL);
	InitTCA0();
	InitTCA1();
	initButtonInt();
	uint8_t freq = 1; // set freq to 1 Hz initially
	TCA1.SINGLE.CMP0 = (31250/freq) - 1; // see math above
	
	sei(); // turn on interrupts
	printf("Is this thing on? \n");	
    while (1) 
    {
		if (btn0Pushed || btn1Pushed) { // if either button is pushed
			cli(); // avoid race conditions with flags
			if (btn0Pushed) {
				freq++;
				btn0Pushed = false;
			}
			if (btn1Pushed) {
				freq = (freq > 1) ? freq - 1 : 1; // minimum freq of 1 Hz
				btn1Pushed = false;
			}
			TCA1.SINGLE.CMP0 = (31250/freq) - 1;
			sei();
		}
		
		if (print_timerCount == 0) {
			print_timerCount = PRINT_COUNT;
			printf("Current freq is %d\n", freq);
		}
	}
}

