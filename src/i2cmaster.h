#ifndef _include_i2cmaster_h_
#define _include_i2cmaster_h_

#include <stdint.h>

class i2c_master
{
	int fd_;
public:
	i2c_master();
	void set_slave_address(int id);
	void write_bytes(void *data, size_t length);
	size_t read_bytes(void *data, size_t length);
};

#endif //_include_i2cmaster_h_