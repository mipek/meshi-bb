#include "sensor_temp.h"

bool sensor_temperature::get_value(sensor_value &value)
{
	value.sensorid_ = id();
	value.vcount_ = 1;
	value.values_ = new sensor_value::any[value.vcount_];
	value.values_[0].iValue = 36;
	return true;
}

bool sensor_temperature::check_value()
{
	return false;
}

Error sensor_temperature::create_sensor(sensor_temperature **sensor)
{
	*sensor = new sensor_temperature(1);
	return kError_None;
}
