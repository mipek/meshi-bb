#ifndef _include_public_protocol_h_
#define _include_public_protocol_h_

#include <cstdint>

// size is always specified in bytes
static const int packet_c2s_header_size = 19;
static const int packet_s2c_header_size = 7;
static const int packet_max_size = 1024;

static const int packet_reliable_timeout = 15000; // after how many millis a reliable packet is considered lost.

enum class packet_id
{
	c2s_sensors = 1,
	s2c_events = 2,
	s2c_routes = 3,
	c2s_update = 4,
	c2s_measurement = 5,
	c2s_frame = 6,
	c2s_malfunction = 7
};

enum class packet_flags
{
	none = 0,
	reliable = 1,
	ack = 2,
	fragmented = 4,
	reserved = 8
};

struct position
{
	position(float lat, float lng): lat_(lat), lng_(lng)
	{
	}
	float lat_;
	float lng_;
};

struct packet
{
	union packed_head {
		struct {
			uint8_t packet_id : 4, flags : 4;
		};
		uint8_t byte;
	} head;
	uint32_t checksum;
	uint16_t bbid;
	uint32_t time;
	position pos;
};

#endif //_include_public_protocol_h_