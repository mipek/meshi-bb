#ifndef _include_i2c_h_
#define _include_i2c_h_

#include "i2cmaster.h"
#include <unistd.h>

i2c_master::i2c_master()
{
	fd_ = open("/dev/i2c-1", O_RDWR);
}

void i2c_master::set_slave_address(int address)
{
	ioctl(fd_, I2C_SLAVE, address);
}

void i2c_mater::write(void *data, size_t length)
{
	write(fd_, data, length);
}

void i2c_master::read(void *data, size_t length)
{
	read(fd_, data, length);
}

#endif //_include_i2c_h_