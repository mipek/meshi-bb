#ifndef _include_sensor_h_
#define _include_sensor_h_

#include <stddef.h>

class sensor_value
{
public:
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
	
	
//private:
	int sensorid_;
	int vcount_;
	any *values_;
};

class sensor
{
public:
	virtual int id() const =0; // id of this sensor
	virtual const char *name() =0; // sensorname, eg. "temperature"
	virtual void get_value(sensor_value &value) =0;
	virtual bool check_value(sensor_value const& value) =0;
};

#endif //_include_sensor_h_