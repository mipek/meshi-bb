#ifndef _inclde_public_transmission
#define _inclde_public_transmission
 
#include <cstdint>
#include <cstring>
#include "protocol.h"

struct message
{
	message() : data(NULL), len(0)
	{
	}
	message(uint8_t *d, size_t n): data(d), len(n)
	{
	}
	message(const message &other): len(other.len)
	{
		data = new uint8_t[other.len];
		memcpy(data, other.data, other.len);
	}
	~message()
	{
		delete[] data;
	}
	
	packet_id get_packet_id() const
	{
		return static_cast<packet_id>(data[0] & 0x0F);
	}

	uint8_t get_flags() const
	{
		return data[0] >> 4;
	}

	uint32_t get_checksum() const
	{
		return *reinterpret_cast<uint32_t*>(data + 1);
	}

	uint16_t get_bbid() const
	{
		return *reinterpret_cast<uint16_t*>(data + 5);
	}

	uint16_t get_packet_number() const
	{
		return *(data + 7);
	}

	uint8_t *get_payload()
	{
		return (data + packet_c2s_header_size);
	}

	uint8_t *data;
	size_t len;
};

class message_listener
{
public:
	virtual ~message_listener() {}
	virtual void on_message(message const& msg) = 0;
};

class transmission
{
public:
	virtual ~transmission() {};
	virtual const char *name() =0;
	/*virtual size_t send(void *data, size_t len) =0;
	virtual size_t receive(void *data, size_t len) =0;

	virtual bool receive_message(message &msg) = 0;*/

	virtual size_t send_message(message const& msg) = 0;
	virtual void update(message_listener *listener) = 0;
};

#endif //_inclde_public_transmission