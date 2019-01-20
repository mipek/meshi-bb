/**
*  @file sensor.h
*  @brief Sensor related informations
*  @author Michael Pekar
*/
#ifndef _include_sensor_h_
#define _include_sensor_h_

#include <stddef.h>

/**
 * Enumerates all known sensor types
 */
enum class sensor_types
{
	none = 0,
	temperature = 1,
	gas = 2,
	thermal = 3,
	color = 4,
	particles = 5
};

/**
 * Encapsulates one (or multiple) sensor values in a single struct.
 */
struct sensor_value
{
	struct any {
		union {
			bool bValue;
			int iValue;
			float flValue;
			void *pValuePtr;
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

/**
 * Describes a sensor and provides access to the data.
 */
class sensor
{
public:
	/**
	 * Returns the sensor id.
	 * @return ID of the sensor.
	 */
	virtual int id() const =0;

	/**
	 * Classifies the sensor.
	 * @return Current sensor classification. Enumerated by ::sensor_types. 
	 */
	virtual sensor_types classify() const = 0;

	/**
	 * Returns the name of the sensor (if any).
	 * @return Null-terminated sensor name or NULL if name is not known.
	 */
	virtual const char *name() const = 0;

	/**
	 * Retrieves the sensor value that was measured last.
	 * @param value ::sensor_value struct to store the value into.
	 * @return <code>true</code> when sensor value was retrieved, <code>false</code> otherwise.
	 */
	virtual bool get_value(sensor_value &value) =0;

	/**
	 * Check if sensor value is triggering a event.
	 * @return <code>true</code> is returned if, and only if, sensor values trigger a event (limit was reached etc.)
	 */
	virtual bool check_value() =0;

	/**
	 * Update sensor value
	 */
	virtual void update() {}
};

#endif //_include_sensor_h_