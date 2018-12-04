#ifndef _include_sensor_h_
#define _include_sensor_h_

class sensor_value
{
public:
	struct any {
		union {
			bool bValue;
			int iValue;
			float flValue;
		}
	};
	
	int get_sensor_id() const { .. }
	int get_value_count() const { .. }
	const any& get_value(int idx) const { .. }
	
	
private:
	int sensorid;
	int vcount;
	any *values;
}

class sensor
{
public:
	virtual int id() =0; // id of this sensor
	virtual const char *name() =0; // sensorname, eg. "temperature"
	virtual void get_value(sensor_value &value) =0;
	virtual bool check_value(sensor_value const& value) =0;
}

#endif //_include_sensor_h_