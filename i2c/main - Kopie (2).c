/*************************************+
 *  I²C slave sample
 *  
 *  Copyright (c) 2018 by Michael Pekar
 *  All rights reserved
 */
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include "usitwislave.h"

/* 8bit slave address */
#define 	SLAVE_ADDR       0x55

#define NOP asm("nop");

int main(void)
{
	usiTwiSlaveInit(SLAVE_ADDR);

	/* The watchdog will restart the MCU when a timeout occurs (ie. during a I²C operation). */
	wdt_enable(WDTO_2S);
	
	/* enable interrupts (required for USI/I²C) */
	sei();

	do
	{
		/* wait untitl we receive some data */
		if (usiTwiDataInReceiveBuffer())
		{
			/* read two bytes */
			uint8_t a = usiTwiReceiveByte();
			uint8_t b = usiTwiReceiveByte();

			/* reply with reverse order */
			usiTwiTransmitByte(b);
			usiTwiTransmitByte(a);
		}

		wdt_reset();
	} while(1);
	return 0;
}