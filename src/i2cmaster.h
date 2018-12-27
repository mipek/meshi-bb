#ifndef _include_i2cmaster_h_
#define _include_i2cmaster_h_

#include "blackbox.h"
#include <cstdint>

class i2c_master
{
	int fd_;
public:
	i2c_master(int bus);
	~i2c_master();
	bool is_valid() const;
	void set_slave_address(uint8_t id);
	void write_bytes(void *data, uint8_t length);
	uint8_t read_bytes(void *data, uint8_t length);
	void write_register(uint8_t reg, uint8_t value);
	void write_register16(uint8_t reg, uint16_t value);
	
	template<typename T>
	T read_register(uint8_t reg)
	{
		// select register
		write_bytes(&reg, 1);
		// read data
		T buffer;
		read_bytes(&buffer, sizeof(T));
		return buffer;
	}
};

#endif //_include_i2cmaster_h_