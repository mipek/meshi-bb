#ifndef _include_i2csensor_h_
#define _include_i2csensor_h_

#include "public/sensor.h"

#define I2C_MAX_SENSORCOUNT 127

class sensor_i2c: public sensor
{
	int id_;
public:
	sensor_i2c(int id): id_(id) {}
	virtual int id()
	{
		return id_;
	}
	virtual const char *name()
	{
		return "TODO";
	}
	virtual void get_value(sensor_value &value)
	{

	}
	virtual bool check_value(sensor_value const& value)
	{
		return false;
	}

public:
	static int enumerate_sensors(sensor_i2c *buf, int maxcount);
}


#endif //_include_i2csensor_h_
