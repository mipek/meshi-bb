#ifndef _include_i2c_h_
#define _include_i2c_h_

#include "i2cmaster.h"

#if PLAT == PLAT_WINDOWS
// TODO: add support
#else
#	include <unistd.h>
#endif

i2c_master::i2c_master()
{
#if PLAT == PLAT_WINDOWS
#else
	fd_ = open("/dev/i2c-1", O_RDWR);
#endif

}

void i2c_master::set_slave_address(int address)
{
#if PLAT == PLAT_WINDOWS
#else
	ioctl(fd_, I2C_SLAVE, address);
#endif

}

void i2c_master::write_bytes(void *data, size_t length)
{
#if PLAT == PLAT_WINDOWS
#else
	write(fd_, data, length);
#endif

}

size_t i2c_master::read_bytes(void *data, size_t length)
{
#if PLAT == PLAT_WINDOWS
	return 0;
#else
	return read(fd_, data, length);
#endif
}

#endif //_include_i2c_h_