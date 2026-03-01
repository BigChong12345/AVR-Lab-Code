/*
 * lab7_pt2.c
 *
 * Created: 3/1/2026 10:44:50 AM
 * Author : drcla
 */ 

#include <avr/io.h>
#include "uart.h"
#include <stdio.h>
#include <avr/interrupt.h>
#include <stdbool.h>

// waveform output timer, no need for interrupt on this one since its just generating pulses
void InitTCA1(void)
{
	TCA1.SINGLE.CTRLB = TCA_SINGLE_WGMODE_FRQ_gc;// FRQ mode
	TCA1.SINGLE.CMP0 = 249; // Set number of ticks for period
	//TCA1.SINGLE.INTCTRL = TCA_SINGLE_CMP0_bm; // Enable TCA0 CMP0 ISR
	TCA1.SINGLE.CTRLB |= TCA_SINGLE_CMP0EN_bm; // Enable waveform output
	TCA1.SINGLE.CTRLA |= (TCA_SINGLE_CLKSEL_DIV256_gc | TCA_SINGLE_ENABLE_bm); // set prescalar to 16 so that every tick is 1 us
	
	PORTMUX.TCAROUTEA |= PORTMUX_TCA1_PORTB_gc;  // waveform will go to PB0
	PORTB.DIRSET = PIN0_bm; // setting PA0 as output;
}

// since we want us accuracy, we need to change the prescalar to 16. 
void InitTCA0(void)
{
	TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_NORMAL_gc; // Normal mode
	TCA0.SINGLE.PER = 999; // overflow every 1ms
	TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm; // Enable TCA0 Overflow ISR
	// Set Prescalar to 16 & enable timer. Each tick is 1us
	TCA0.SINGLE.CTRLA |= (TCA_SINGLE_CLKSEL_DIV16_gc | TCA_SINGLE_ENABLE_bm);
}

volatile uint32_t total_time = 0;
volatile uint32_t sum = 0;
volatile uint32_t start_time = 0; 
volatile uint32_t end_time = 0;
volatile uint16_t pulse_count = 0;

// if this interrupt is triggered that means 65535 us has passed
ISR(TCA0_OVF_vect) {
	total_time += 1000; 
	TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;
}

ISR(PORTB_PORT_vect) {
	if (PORTB.INTFLAGS & PIN2_bm) { // if the interrupt is triggered by PIN2
		PORTB.INTFLAGS |= PIN2_bm; // clear interrupt
		if (PORTB.IN & PIN2_bm) { // this means it was a rising edge
			start_time = total_time + TCA0.SINGLE.CNT; // this CNT register holds the current time elapsed
		}
		else { // this means it was a falling edge
			end_time = total_time + TCA0.SINGLE.CNT;
			sum += (end_time - start_time);
			pulse_count++;
		}
	}
}

// PB0 connects to PB2 via hardware (jumper wire), PB2 needs to be configured to interrupt on both a falling and rising edge in order to be able to measure pulse width
void initPB2() {
	//set both buttons to falling edge
	PORTB.PIN2CTRL = PORT_ISC_BOTHEDGES_gc; // hardware interrupt triggers on both edges
	PORTB.DIRCLR = PIN2_bm; // set both buttons to inputs
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
	init_clock();
	initPB2();
	InitTCA1();
	InitTCA0();
	uart_init(3, 9600, NULL);
	sei();
	uint32_t average;
    while (1) 
    {
		if (total_time == 1000000)  {// 1 second has passed 
			average = sum/pulse_count;
			printf("The average is %d", average);
			total_time = 0;
		}
	}
}

