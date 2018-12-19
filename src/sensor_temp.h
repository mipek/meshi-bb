#ifndef _include_sensor_temp_h_
#define _include_sensor_temp_h_

#include "sensor.h"
#include "errors.h"

class sensor_temperature : public sensor
{
	int id_;
	sensor_temperature(int id) : id_(id)
	{
	}
public:
	virtual int id() const
	{
		return id_;
	}
	virtual sensor_types classify() const
	{
		return kSensorTemperature;
	}
	virtual void get_value(sensor_value &value);
	virtual bool check_value(sensor_value const& value);

	static Error create_sensor(sensor_temperature **sensor);
};

#endif //_include_sensor_temp_h_
