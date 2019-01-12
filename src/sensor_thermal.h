#ifndef _include_sensor_thermal_h_
#define _include_sensor_thermal_h_

#include "sensor.h"
#include "errors.h"

class webcam;

class sensor_thermal : public sensor
{
	int id_;
	webcam *webcam_;
	sensor_thermal(int id, webcam *cam) : id_(id), webcam_(cam)
	{
	}
public:
	virtual int id() const
	{
		return id_;
	}
	virtual sensor_types classify() const
	{
		return sensor_types::thermal;
	}
	virtual const char *name() const
	{
		return "FLIR Lepton 3";
	}
	virtual bool get_value(sensor_value &value);
	virtual bool check_value();
	virtual void update();

	static Error create_sensor(sensor **sensor);
};

#endif //_include_sensor_thermal_h_
