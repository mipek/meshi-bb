#ifndef _include_messagebuilder_h_
#define _include_messagebuilder_h_

#include <cstdint>
#include <vector>
#include <protocol.h>
#include <transmission.h>

class message_builder
{
	std::vector<uint8_t> byte_;
	size_t payload_offset_;
public:
	message_builder();

public: /* header */
	void begin_message(packet_id id, packet_flags flags,
		uint16_t bbid, uint32_t time, position const& latlng);
	void begin_message(packet_id id, packet_flags flags, int packet_no,
					   uint16_t bbid, uint32_t time, position const& latlng);

	static int generate_packet_no();
public: /* payload */
	void write_byte(uint8_t val);
	void write_short(uint16_t val);
	void write_dword(uint32_t val);
	void write_float(float val);
	void write_double(double val);

public:
	void finalize_message(message &out);
};

#endif //_include_messagebuilder_h_