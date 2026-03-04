/*
 * Lab9_I2C.c
 *
 * Created: 3/3/2026 2:53:41 PM
 * Author : drcla
 */ 
#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/twi.h>
#include "uart.h"
#include <avr/interrupt.h>
#include <stdio.h>

#define PRINT_TIME 1000

#define loop_until_bit_is_set(sfr, bit) do { } while (bit_is_clear(sfr, bit))

void InitTCA0(void)
{
	TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_NORMAL_gc; // Normal mode
	TCA0.SINGLE.PER = 249; // Set number of ticks for period
	TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm; // Enable TCA0 Overflow ISR
	// Set Prescalar to 64 & enable timer. Each tick is 4us
	TCA0.SINGLE.CTRLA |= (TCA_SINGLE_CLKSEL_DIV64_gc | TCA_SINGLE_ENABLE_bm);
}

volatile uint16_t print_timerCount = PRINT_TIME;
// triggers every 1ms
ISR(TCA0_OVF_vect) {
	if (print_timerCount > 0) {
		print_timerCount--;
	}
	
	TCA0.SINGLE.INTFLAGS |= TCA_SINGLE_OVF_bm; // clear interrupt
}

void TWI_Host_Write(uint8_t Address, uint8_t Data)
{
	TWI_Address(Address, TW_WRITE);
	TWI_Transmit_Data(Data);
	TWI_Stop();
}
uint8_t TWI_Host_Read(uint8_t Address)
{
	TWI_Address(Address, TW_READ);
	uint8_t data = TWI_Receive_Data();
	TWI_Stop();
	return data;
}
void TWI_Stop()
{
	TWI0.MCTRLB |= TWI_MCMD_STOP_gc;
}

void TWI_Address(uint8_t Address, uint8_t mode)
{
	while (1) {
		// set addr & R/W bit, starts transfer
		TWI0.MADDR = (Address << 1) | (mode); // when you do this, the hardware pulls SDA down and I2C starts
		// Wait for Read/Write Interrupt Flag.
		uint8_t flag = (mode == TW_WRITE) ? TWI_WIF_bp : TWI_RIF_bp;
		loop_until_bit_is_set(TWI0.MSTATUS, flag);
		// if the client didnt ack, stop the transaction
		if (TWI0.MSTATUS & TWI_RXACK_bm) {
			TWI_Stop();
		}
		// if no bus or arbitration error, all good. otherwise try it again
		if (!(TWI0.MSTATUS & (TWI_ARBLOST_bm | TWI_BUSERR_bm))) {
			break;
		}
	}
}

int TWI_Transmit_Data(uint8_t data)
{
	// start data transfer by writing to MDATA
	TWI0.MDATA = data;
	// Wait for Write Interrupt Flag.
	loop_until_bit_is_set(TWI0.MSTATUS, TWI_WIF_bp);
	// if bus or arbitration error, return error
	return ((TWI0.MSTATUS & (TWI_ARBLOST_bm | TWI_BUSERR_bm)) ? -1 : 0);
}
uint8_t TWI_Receive_Data()
{
	// Wait for Read Interrupt Flag.
	loop_until_bit_is_set(TWI0.MSTATUS, TWI_RIF_bp);
	uint8_t data = TWI0.MDATA;
	// Respond with NACK
	TWI0.MCTRLB |= TWI_ACKACT_bm;
	return data;
}

void TWI_Host_Initialize() //TWI = Two wire interface
{
	TWI0.MBAUD = 96; // 16MHz clock; rise time = 10ns
	TWI0.MCTRLA |= TWI_ENABLE_bm; // enable the I2C
	TWI0.MSTATUS |= TWI_BUSSTATE_IDLE_gc; // force the bus state to IDLE
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

	uart_init(3, 9600, NULL);	//uart init for printing
	init_clock();
	InitTCA0();
	sei();
	
	uint8_t address = 0b01001000; // specified in datasheet
	uint8_t slave_data = 0; // TWI_Host_Read returns uint8

	TWI_Host_Initialize(); // initializes I2C and idle bus state
	//SDA and SCK need pull ups 
	PORTA.DIRCLR = (PIN2_bm | PIN3_bm); // idk if it actually needs to be set as an input or not
	
	PORTA.PIN2CTRL |= PORT_PULLUPEN_bm; // SDA
	PORTA.PIN3CTRL |= PORT_PULLUPEN_bm; // SCL
    /* Replace with your application code */
    while (1) 
    {

		
		if (print_timerCount == 0) {
			TWI_Address(address, TW_WRITE);
			TWI_Transmit_Data(0x00);
			TWI_Address(address, TW_READ);
			slave_data = TWI_Receive_Data(); // read address
			TWI_Stop();
			
			printf("Current data: %d\r\n", slave_data);
			print_timerCount = PRINT_TIME;
		}
    }
}

