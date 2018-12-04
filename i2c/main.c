/***************************************
 *  I²C slave sample for meshi sensors
 *
 *  Copyright (c) 2018 by Michael Pekar
 *  All rights reserved
 */
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include "usitwislave.h"
#include "meshiprotocol.h"

/* 8bit slave address */
#define 	SLAVE_ADDR       0x55

static void i2c_enumerate(void)
{
	/* tell them about the sensors I have access to. */
	uint8_t mysensors = kSensorType_Temperature | kSensorType_Gas;
	usiTwiTransmitByte(mysensors);
}

static void i2c_getvalue(uint8_t sensors)
{
	if (sensors & kSensorType_Temperature) {
		usiTwiTransmitByte(20);
	}

	if (sensors & kSensorType_Gas) {
		usiTwiTransmitByte(8);
	}
}

int main(void)
{
	/* init I²C/TWI implementation (via USI) */
	usiTwiSlaveInit(SLAVE_ADDR);

	/* The watchdog will restart the MCU when a timeout occurs (ie. during a I²C operation hang). */
	wdt_enable(WDTO_2S);

	/* enable interrupts (required for USI/I²C) */
	sei();

	do
	{
		/* wait untitl we receive some data */
		if (usiTwiDataInReceiveBuffer())
		{
			/* read two bytes */
			uint8_t packetId = usiTwiReceiveByte();
			//usiTwiTransmitByte(packetId);
			switch (packetId)
			{
				case kPacketID_Enumerate:
					i2c_enumerate();
					break;
				case kPacketID_GetValue:
					i2c_getvalue(usiTwiReceiveByte());
					break;
				default:  /* error, unknown packetid */
					usiTwiTransmitByte(0x42);
					break;
			}
		}

		wdt_reset();
	} while(1);
	return 0;
}
