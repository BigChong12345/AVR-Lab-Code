/*
 * Lab8.c
 *
 * Created: 3/1/2026 6:53:50 PM
 * Author : drcla
 */ 

#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <avr/interrupt.h>
#include "uart.h"

#define ADC_time 1000

volatile uint16_t ADC_timercount = ADC_time;

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

// triggers every 1ms
ISR(TCA0_OVF_vect) {
	if (ADC_timercount > 0) {
		ADC_timercount--;
	}
	
	TCA0.SINGLE.INTFLAGS |= TCA_SINGLE_OVF_bm; // clear interrupt
}

int main(void)
{
    /* Replace with your application code */
	init_clock();
	InitTCA0();
	uart_init(3, 9600, NULL);
	
	float voltage = 0.0;
	uint16_t Ain;
	
	PORTE.DIRCLR = PIN0_bm; // this is our ADC input; 
	ADC0.MUXPOS = ADC_MUXPOS_AIN8_gc; // set it as ADC input
	ADC0.CTRLC = ADC_PRESC_DIV16_gc; // 1 MHz sampling
	ADC0.CTRLD = ADC_INITDLY_DLY16_gc; // Delay 16 CLK_ADC cycles – 16 us
	VREF.ADC0REF = VREF_REFSEL_VDD_gc; // VDD as reference
	ADC0.CTRLA = ADC_ENABLE_bm; 
	ADC0.COMMAND = ADC_STCONV_bm; // start first ADC conversion
	
	sei();
    while (1) 
    {
		if (ADC_timercount == 0) { // 1 second has passed
			ADC_timercount = ADC_time;
			Ain = ADC0.RES; 
			voltage = (float) (Ain*3.3)/4096; // 12 bit ADC
			
			ADC0.COMMAND = ADC_STCONV_bm; // starts next conversion
			char voltagebuffer[6];
			dtostrf(voltage, 4, 2, voltagebuffer);
			printf("Voltage: %s\n\r", voltagebuffer);
			
		}
    }
}

