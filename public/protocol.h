#ifndef _include_public_protocol_h_
#define _include_public_protocol_h_

#include <cstdint>

// size is always specified in bytes
static const int packet_c2s_header_size = 19;
static const int packet_s2c_header_size = 8;
static const int packet_max_size = 1024;

static const int packet_reliable_timeout = 800; ///< Determines after how many millis a reliable packet is considered lost.

/**
 * Lists all available packet ids
 */
enum class packet_id
{
	s2c_sensorreq = 1,
	c2s_sensors = 1,
	s2c_events = 2,
	s2c_routes = 3,
	s2c_framereq = 4,
	c2s_measurement = 5,
	c2s_frame = 6,
	c2s_malfunction = 7
};

/**
 * Lists supported packet flags
 */
enum class packet_flags
{
	none = 0,		///< None special flags set.
	reliable = 1,	///< Request acknowledgement from server
	ack = 2,		///< Acknowledgement
	fragmented = 4,	///< Packet is transmitted in multiple parts
	reserved = 8	///< Usageis reserved for future use.
};

/**
 * Describes a position.
 */
struct position
{
	position(float lat, float lng): lat_(lat), lng_(lng)
	{
	}
	float lat_; ///< Latitude
	float lng_; ///< Longitude
};

/**
 * Packet structure
 */
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