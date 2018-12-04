/*************************************+
 *  ProximityLed
 *  
 *  Copyright (c) 2016 by Michael Pekar
 *  All rights reserved
 */
#include <avr/io.h>
#include <util/delay.h>
//#include "usi_i2c_slave.h"
#include "usitwislave.h"
#include <avr/io.h>
#include <avr/interrupt.h>

/* 8bit slave address */
#define 	SLAVE_ADDR       0x55

uint8_t lastdata = 1;

uint8_t i2c_read(uint8_t index)
{
	return lastdata;
}

bool i2c_write(uint8_t data, uint8_t index)
{
	lastdata = data;
	return false;
}

void i2c_callback(
	uint8_t input_buffer_length, const uint8_t* input_buffer,
	uint8_t* output_buffer_length, uint8_t* output_buffer)
{
    if(input_buffer_length > 0) {
        *output_buffer_length = 1;
		*output_buffer = 0x42;
		//*(output_buffer+1) = 0x43;
    }
}

void idle(void)
{
	
}
#define NOP asm("nop");
int main(void)
{
	//cli();

	//USI_I2C_Slave_Init(SLAVE_ADDR, i2c_read, i2c_write);
	//usi_twi_slave(SLAVE_ADDR, 0, i2c_callback, idle);

	usiTwiSlaveInit(SLAVE_ADDR);
	
	sei();

	while (1) {
		if (usiTwiDataInReceiveBuffer())
		{
			uint8_t b = usiTwiReceiveByte();
			_delay_ms(25);
			usiTwiTransmitByte(b+1);
		}
		NOP
	}
	return 0;
}
