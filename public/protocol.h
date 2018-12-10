#ifndef _include_public_protocol_h_
#define _include_public_protocol_h_

#include <stddef.h>

struct Packet
{
	uint8_t head;
	uint32_t checksum;
	uint16_t bbid;
	uint32_t time;
	double latlng;
};

#endif _include_public_protocol_h_