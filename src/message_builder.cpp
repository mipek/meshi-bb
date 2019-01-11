#include "message_builder.h"
#include "crc32.h"

int message_builder::generate_packet_no()
{
	static uint8_t packet_number_seed = 0;
	return packet_number_seed++;
}

message_builder::message_builder(): payload_offset_(0)
{
	byte_.reserve(20);
}

void message_builder::begin_message(packet_id id, packet_flags flags, int packet_no,
									uint16_t bbid, uint32_t time, position const& pos)
{
	uint8_t packed_byte = (uint8_t)id | ((uint8_t)flags << 4);
	write_byte(packed_byte);
	write_dword(0);
	write_short(bbid);
	write_byte(packet_no);

	// data only being transmitted when doing a c2s send.
	write_dword(time);
	write_float(pos.lat_);
	write_float(pos.lng_);

	payload_offset_ = byte_.size();
}

void message_builder::begin_message(packet_id id, packet_flags flags,
	uint16_t bbid, uint32_t time, position const& pos)
{
	begin_message(id, flags, message_builder::generate_packet_no(), bbid, time, pos);
}

void message_builder::write_byte(uint8_t val)
{
	byte_.push_back(val);
}

void message_builder::write_short(uint16_t val)
{
	uint8_t *p = reinterpret_cast<uint8_t*>(&val);
	byte_.push_back(p[0]);
	byte_.push_back(p[1]);
}

void message_builder::write_dword(uint32_t val)
{
	uint8_t *p = reinterpret_cast<uint8_t*>(&val);
	byte_.push_back(p[0]);
	byte_.push_back(p[1]);
	byte_.push_back(p[2]);
	byte_.push_back(p[3]);
}

void message_builder::write_float(float val)
{
	uint8_t *p = reinterpret_cast<uint8_t*>(&val);
	byte_.push_back(p[0]);
	byte_.push_back(p[1]);
	byte_.push_back(p[2]);
	byte_.push_back(p[3]);
}

void message_builder::write_double(double val)
{
	uint8_t *p = reinterpret_cast<uint8_t*>(&val);
	byte_.push_back(p[0]);
	byte_.push_back(p[1]);
	byte_.push_back(p[2]);
	byte_.push_back(p[3]);
	byte_.push_back(p[4]);
	byte_.push_back(p[5]);
	byte_.push_back(p[6]);
	byte_.push_back(p[7]);
}

size_t message_builder::get_payload_size() const
{
	return byte_.size() - payload_offset_;
}

void message_builder::finalize_message(message &out)
{
	out.data = new uint8_t[byte_.size()];
	out.len = byte_.size();

	uint8_t *header = reinterpret_cast<uint8_t*>(byte_.data());
	uint8_t *payload = header + payload_offset_;
	
	// calculate and write checksum
	int crc_offset = 5;
	crc32(header + crc_offset, (byte_.size() - crc_offset) + payload_offset_,
		reinterpret_cast<uint32_t*>(header + sizeof(uint8_t)));

	memcpy(out.data, byte_.data(), byte_.size());
}