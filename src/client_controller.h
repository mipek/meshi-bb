#ifndef _include_client_controller_
#define _include_client_controller_

#include <controller.h>
#include <string>
#include <vector>

/* forward declaration */
class transmission;
class sensor;

struct ClientControllerOptions
{
	ClientControllerOptions(): port(0), bbid(0), dbgsensors(false)
	{
	}
	std::string host;
	int port;
	int bbid;
	bool dbgsensors;
};

class client_controller: public controller
{
	transmission *trnsmsn_;
	std::vector<sensor*> sensors_;
public:
	client_controller(transmission *trnsmsn): trnsmsn_(trnsmsn)
	{
	}
	virtual ~client_controller();
	void on_tick();

	void register_sensor(sensor *s)
	{
		sensors_.push_back(s);
	}
	size_t get_sensor_count() const
	{
		return sensors_.size();
	}
	sensor *get_sensor(size_t idx)
	{
		return sensors_[idx];
	}
	void destroy_all_sensors();

	static controller *make(ClientControllerOptions const& opts);

private:
	int find_and_add_sensors();
};

#endif //_include_client_controller_