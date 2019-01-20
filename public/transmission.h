#ifndef _inclde_public_transmission
#define _inclde_public_transmission
 
#include <cstdint>
#include <cstring>
#include "protocol.h"

/**
 * Describes a message
 */
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
	
	/**
	 * Get packet id
	 * @return packet id
	 */
	packet_id get_packet_id() const
	{
		return static_cast<packet_id>(data[0] & 0x0F);
	}

	/**
	 * Get packet flags
	 * @return packet flags
	 */
	uint8_t get_flags() const
	{
		return data[0] >> 4;
	}

	/**
	 * Get packet checksum
	 * @return crc32 that is stored in the packet
	 */
	uint32_t get_checksum() const
	{
		return *reinterpret_cast<uint32_t*>(data + 1);
	}

	/**
	 * Get id of the blackbox this packet is assigned to
	 * @return blackbox id
	 */
	uint16_t get_bbid() const
	{
		return *reinterpret_cast<uint16_t*>(data + 5);
	}

	/**
	 * Get packet number
	 * @return sequence number
	 */
	uint16_t get_packet_number() const
	{
		return *(data + 7);
	}

	/**
	 * Get payload starting byte
	 * @note only works for c2s packets
	 * @return payload data
	 */
	uint8_t *get_payload() const
	{
		return (data + 20);
	}

	/**
	* Get payload starting byte
	* @note only works for s2c packets
	* @return payload data
	*/
	uint8_t *get_s2c_payload() const
	{
		return (data + packet_s2c_header_size);
	}

	/**
	 * Get latitude
	 * @note only available for c2s packets
	 * @return latitude
	 */
	float get_latitude() const
	{
		return *(float*)(data + 12);
	}

	/**
	* Get longitude
	* @note only available for c2s packets
	* @return longitude
	*/
	float get_longitude() const
	{
		return *(float*)(data + 16);
	}

	/**
	 * Get payload size of a c2s packet
	 * @return c2s payload size
	 */
	size_t get_c2s_payload_size() const
	{
		return len - (packet_c2s_header_size + 1);
	}

	uint8_t *data;
	size_t len;
};

/**
 * Message listener interface declaration
 */
class message_listener
{
public:
	virtual ~message_listener() {}

	/**
	 * Called for each received message that needs to be processed.
	 * @param msg Message that shall be processed.
	 */
	virtual void on_message(message const& msg) = 0;
};

/**
 * Describes the transmission interface. No assumptions should be made on the underlying transmission medium (TCP, UDP, LoRa etc.)
 */
class transmission
{
public:
	virtual ~transmission() {};

	/**
	 * Name of the transmission interface implementation.
	 * This is for debugging purpose only.
	 * @return Transmission interface implementation (or NULL if not provided).
	 */
	virtual const char *name() =0;

	/**
	 * Transmit message.
	 * @param msg message that is to be transmitted.
	 * @return count of bytes that have been send
	 */
	virtual size_t send_message(message const& msg) = 0;

	/**
	 * Update transmission interface, pending messages will be received and dispatched into the ::message_listesner.
	 * @param listener message listener to invoke when messages have been received.
	 */
	virtual void update(message_listener *listener) = 0;
};

#endif //_inclde_public_transmission