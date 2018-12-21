#ifndef _include_sensor_h_
#define _include_sensor_h_

#include <stddef.h>

enum sensor_types
{
	kSensorNone = 0,
	kSensorTemperature = 1,
	kSensorGas = 2,
	kSensorThermal = 3,
	kSensorColor = 4,
	kSensorParticles = 5
};

struct sensor_value
{
	struct any {
		union {
			bool bValue;
			int iValue;
			float flValue;
		};
	};

	sensor_value(): sensorid_(0), vcount_(0), values_(NULL)
	{
	}

	~sensor_value()
	{
		if (values_)
		{
			delete [] values_;
		}
	}
	
	int get_sensor_id() const { return sensorid_; }
	int get_value_count() const { return vcount_; }
	const any& get_value(int idx) const { return values_[idx]; }

	int sensorid_;
	int vcount_;
	any *values_;
};

class sensor
{
public:
	virtual int id() const =0; // id of this sensor
	virtual sensor_types classify() const = 0;
	virtual const char *name() const = 0;
	virtual bool get_value(sensor_value &value) =0;
	virtual bool check_value(sensor_value const& value) =0;
	virtual void update() {}
};

#endif //_include_sensor_h_