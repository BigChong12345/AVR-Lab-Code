/*
 * Lab9_I2C.c
 *
 * Created: 3/3/2026 2:53:41 PM
 * Author : drcla
 */ 

#include <avr/io.h>
#include <util/twi.h>

#define loop_until_bit_is_set(sfr, bit) do { } while (bit_is_clear(sfr, bit))


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
		TWI0.MADDR = (Address << 1) | (mode);
		// Wait for Read/Write Interrupt Flag.
		uint8_t flag = (mode == TW_WRITE) ? TWI_WIF_bp : TWI_RIF_bp;
		loop_until_bit_is_set(TWI0.MSTATUS, flag);
		// if the client didn’t ack, stop the transaction
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
	TWI0.MBAUD = 35; // 16MHz clock; rise time = 10ns
	TWI0.MCTRLA |= TWI_ENABLE_bm; // enable the I2C
	TWI0.MSTATUS |= TWI_BUSSTATE_IDLE_gc; // force the bus state to IDLE
}

int main(void)
{
    /* Replace with your application code */
    while (1) 
    {
		
    }
}

