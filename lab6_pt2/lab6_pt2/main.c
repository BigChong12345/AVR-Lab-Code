/*
 * lab6_pt2.c
 *
 * Created: 2/16/2026 11:49:46 AM
 * Author : drcla
 
 Implement blink code using 2 tasks
  subTaskA is the blink on code
  subTaskB is the blink off code
 ?Use two buttons to set the frequency – one to
 increment and one to decrement.
 ?Every 5s, output the current frequency to the serial port
  Do not use delay_ms
  Implement as a second task
 */ 

#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include "uart.h"
#include <stdbool.h>

#define DEBOUNCE_TIME 10
#define PRINT_TIME 5000
volatile uint8_t debounce_timerCount = DEBOUNCE_TIME;
volatile uint16_t print_timerCount = PRINT_TIME;
volatile uint16_t blink_timerCount = 0;

volatile bool btnPushed1 = false;
#define BTN1 (!(VPORTB.IN & PIN2_bm))
volatile bool btnPushed2 = false;
#define BTN2 (!(VPORTB.IN & PIN5_bm))

typedef enum {RELEASED, MAYBE_PUSHED,
	PUSHED, MAYBE_RELEASED
} btn_state_t;
void buttonSM1(void)
{
	static btn_state_t state = RELEASED;
	switch (state)
	{
		case RELEASED:
		if (BTN1)
		state = MAYBE_PUSHED;
		break;
		case MAYBE_PUSHED:
		if (BTN1) {
			state = PUSHED;
			btnPushed1 = true;
			} else {
			state = RELEASED;
		}
		break;
		case PUSHED:
		if (!BTN1)
		state = MAYBE_RELEASED;
		break;
		case MAYBE_RELEASED:
		if (BTN1)
		state = PUSHED;
		else
		state = RELEASED;
		break;
	}
} 

void buttonSM2(void)
{
	static btn_state_t state = RELEASED;
	switch (state)
	{
		case RELEASED:
		if (BTN2)
		state = MAYBE_PUSHED;
		break;
		case MAYBE_PUSHED:
		if (BTN2) {
			state = PUSHED;
			btnPushed2 = true;
			} else {
			state = RELEASED;
		}
		break;
		case PUSHED:
		if (!BTN2)
		state = MAYBE_RELEASED;
		break;
		case MAYBE_RELEASED:
		if (BTN2)
		state = PUSHED;
		else
		state = RELEASED;
		break;
	}
}

void init_clock() {

	CPU_CCP = CCP_IOREG_gc;
	CLKCTRL.XOSCHFCTRLA =  CLKCTRL_FRQRANGE_16M_gc | CLKCTRL_ENABLE_bm;
	CPU_CCP = CCP_IOREG_gc;
	CLKCTRL.MCLKCTRLA = CLKCTRL_CLKSEL_EXTCLK_gc;
	while(!(CLKCTRL.MCLKSTATUS & CLKCTRL_EXTS_bm));
}

// 1ms ISR for Timer TCA0 assuming F_CPU = 16MHz
void InitTimerTCA0(void)
{
	TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_NORMAL_gc; // Normal mode
	TCA0.SINGLE.PER = 249; // Set number of ticks for period
	TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm; // Enable TCA0 Overflow ISR
	// Set Prescalar to 64 & enable timer. Each tick is 4us
	TCA0.SINGLE.CTRLA |= (TCA_SINGLE_CLKSEL_DIV64_gc | TCA_SINGLE_ENABLE_bm);
}

int main(void)
{
    /* Replace with your application code */
	init_clock();
	uart_init(3, 9600, NULL);
	uint8_t freq = 1;
	PORTB.PIN2CTRL = PORT_PULLUPEN_bm;
	PORTB.DIRCLR = (PIN2_bm | PIN5_bm);
	PORTD.DIRSET = PIN0_bm;
	PORTD.OUTCLR = PIN0_bm;
	InitTimerTCA0();
	sei();
	blink_timerCount = 500/freq;
	
    while (1) 
    {
		if (btnPushed1) {
			btnPushed1 = false;
			freq++;	
			blink_timerCount = 0;
		}
		if (btnPushed2) {
			btnPushed2 = false;
			if (freq != 1)
				freq--;
			blink_timerCount = 0;
		}
		
		if (blink_timerCount == 0) {
			PORTD.OUTTGL = PIN0_bm;
			blink_timerCount = 500/freq;
		}
		
		if (print_timerCount == 0) {
			printf("Current freq is: %u Hz \r\n", freq);
			print_timerCount = PRINT_TIME;
		}
	}
}
ISR(TCA0_OVF_vect) {
	if (print_timerCount > 0) print_timerCount--;
	if (blink_timerCount > 0) blink_timerCount--;
	if (debounce_timerCount > 0) debounce_timerCount--;
	else {
		buttonSM1();
		buttonSM2();
		debounce_timerCount = DEBOUNCE_TIME;
	}
	TCA0.SINGLE.INTFLAGS |= TCA_SINGLE_OVF_bm; // must clear the interrupt
}
