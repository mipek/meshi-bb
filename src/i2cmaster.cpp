#ifndef _include_i2c_h_
#define _include_i2c_h_

#include "i2cmaster.h"
#include <cstdio>

#if PLAT == PLAT_WINDOWS
// TODO: add support
#else
#	include <unistd.h>
#endif

i2c_master::i2c_master(int bus)
{
#if PLAT == PLAT_WINDOWS
#else
	char buffer[32];
	snprintf(buffer, sizeof(buffer), "/dev/i2c-%d", id);
	fd_ = open(buffer, O_RDWR);
#endif
}

void i2c_master::set_slave_address(uint8_t address)
{
#if PLAT == PLAT_WINDOWS
#else
	ioctl(fd_, I2C_SLAVE, address);
#endif

}

void i2c_master::write_bytes(void *data, uint8_t length)
{
#if PLAT == PLAT_WINDOWS
#else
	write(fd_, data, length);
#endif
}

size_t i2c_master::read_bytes(void *data, uint8_t length)
{
#if PLAT == PLAT_WINDOWS
	return 0;
#else
	return read(fd_, data, length);
#endif
}

void i2c_master::write_register(uint8_t reg, uint8_t value)
{
	uint8_t buffer[2] = { reg, value };
	write_bytes(buffer, 2);
}

void i2c_master::write_register16(uint8_t reg, uint16_t value)
{
	uint8_t buffer[3];
	buffer[0] = reg;
	buffer[1] = value & 0xFF; //low
	buffer[2] = (value >> 8) & 0xFF; //high
	write_bytes(buffer, 3);
}

#endif //_include_i2c_h_