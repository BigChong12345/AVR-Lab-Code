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

#define DEBOUNCE_TIME 10
#define PRINT_COUNT 1000
#define BTN0 (!(PORTB.IN & PIN2_bm)) // btn0 is on board button
#define BTN1 (!(PORTB.IN & PIN5_bm)) // btn1 is off board button on PB5

volatile bool btn0Pushed = false;
volatile bool btn1Pushed = false;
volatile uint8_t debounce_timerCount = 0;

//todo: still need to do stuff

// waveform output timer, no need for interrupt on this one since its just generating pulses
void InitTCA1(void)
{
	TCA1.SINGLE.CTRLB = TCA_SINGLE_WGMODE_FRQ_gc;// FRQ mode
	TCA1.SINGLE.CMP0 = 249; // Set number of ticks for period
	//TCA1.SINGLE.INTCTRL = TCA_SINGLE_CMP0_bm; // Enable TCA0 CMP0 ISR
	TCA1.SINGLE.CTRLB |= TCA_SINGLE_CMP0EN_bm; // Enable waveform output
	TCA1.SINGLE.CTRLA |= (TCA_SINGLE_CLKSEL_DIV256_gc | TCA_SINGLE_ENABLE_bm); 
	
	PORTMUX.TCAROUTEA |= PORTMUX_TCA1_PORTC_gc;  // waveform will go to PB0
	PORTC.DIRSET = PIN4_bm; // setting PA0 as output;
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
volatile uint16_t printTimer_count = PRINT_COUNT;
volatile bool wait = false;

// if this interrupt is triggered that means 1 ms has passed
ISR(TCA0_OVF_vect) {
	if (printTimer_count > 0) { printTimer_count--; }
	total_time += 1000; 
	
	if (debounce_timerCount > 0) {
		debounce_timerCount--;
		if (debounce_timerCount == 0) { // check both buttons with same flag at same time
			if (BTN0)
			btn0Pushed = true;
			if (BTN1)
			btn1Pushed = true;
		}
	}
	
	TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;
}

ISR(PORTB_PORT_vect) {
	uint8_t flags = PORTB.INTFLAGS;
	
	if (flags & PIN4_bm) { // if the interrupt is triggered by PIN2
		PORTB.INTFLAGS = PIN4_bm;
		uint32_t current_time = total_time;
		uint16_t cnt = TCA0.SINGLE.CNT;
		
		if (PORTB.IN & PIN4_bm) { // this means it was a rising edge
			if (!wait) {
				start_time = current_time + cnt; // this CNT register holds the current time elapsed
				wait = true;
			}
		}
		else { // this means it was a falling edge
			if (wait) {
				end_time = current_time + cnt;
				sum += (end_time - start_time);
				pulse_count++;
				wait = false;
			}
		}
	}
	
	if (flags & PIN2_bm) { // interrupt caused by BTN0 being pressed
		PORTB.INTFLAGS = PIN2_bm;
		if (debounce_timerCount == 0) {
			debounce_timerCount = DEBOUNCE_TIME;
		}
	}
		
	if (flags & PIN5_bm) { // interrupt caused by BTN1 being pressed
		PORTB.INTFLAGS = PIN5_bm;
		if (debounce_timerCount == 0) {
			debounce_timerCount = DEBOUNCE_TIME;
		}
	}
}

// PC4 connects to Pb4 via hardware (jumper wire), PB2 needs to be configured to interrupt on both a falling and rising edge in order to be able to measure pulse width
void initPB2() {
	//set both buttons to falling edge
	PORTB.PIN4CTRL = PORT_ISC_BOTHEDGES_gc; // hardware interrupt triggers on both edges
	PORTB.DIRCLR = PIN4_bm; // set both buttons to inputs
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
	init_clock();
	initButtonInt();
	initPB2();
	InitTCA1();
	InitTCA0();
	uart_init(3, 9600, NULL);
	sei();
	uint32_t average = 0;
	uint8_t freq = 1; // set freq to 1 Hz initially
	TCA1.SINGLE.CMP0 = (31250/freq) - 1; // see math above

    while (1) 
    {
		if (printTimer_count == 0)  {// 1 second has passed 
			//cli(); //turn off interrupts briefly to avoid race conditions
			if (pulse_count > 0) {
				average = sum/pulse_count; // avoid divide by zero though this shouldnt happen 
			}
			sum = 0;
			pulse_count = 0;
			printTimer_count = PRINT_COUNT;
			//sei(); // printf takes a long time so need to turn on interrupts before the print statement
			printf("The average is %lu \n", average);
		}
		if (btn0Pushed || btn1Pushed) { // if either button is pushed
			//cli(); // avoid race conditions with flags
			if (btn0Pushed) {
				freq++;
				btn0Pushed = false;
			}
			if (btn1Pushed) {
				freq = (freq > 1) ? freq - 1 : 1; // minimum freq of 1 Hz
				btn1Pushed = false;
			}
			TCA1.SINGLE.CMP0 = (31250/freq) - 1;
			//sei();
		}
	}
}

