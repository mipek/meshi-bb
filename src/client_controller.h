#ifndef _include_client_controller_
#define _include_client_controller_

#include <controller.h>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>
#include <transmission.h>
#include <transport.h>

/* forward declaration */
class sensor;

struct ClientControllerOptions
{
	ClientControllerOptions(): port(0), bbid(0), dbgsensors(false), dbgroutes(false)
	{
	}
	std::string host;
	int port;
	uint16_t bbid;
	bool dbgsensors;
	bool dbgroutes;
};

class client_controller: public controller, public message_listener, public transport_listener
{
	FILE *gpsuart_;
	transmission *trnsmsn_;
	transport *transport_;
	std::vector<sensor*> sensors_;
	uint64_t last_tick_;
	float lat_, lng_;
	uint16_t bbid_;
	bool running_;
public:
	client_controller(transmission *trnsmsn): gpsuart_(NULL), trnsmsn_(trnsmsn), last_tick_(0), running_(true), lat_(13.45f), lng_(13.51f)
	{
	}
	virtual ~client_controller();
	void on_start();
	void on_tick();

	bool is_running() const { return running_; }
	void set_running(bool running) { running_ = running; }

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

	void set_transport(transport *t) { transport_ = t; }
	transport *get_transport() { return transport_; }

	static controller *make(ClientControllerOptions const& opts);

public:
	void on_message(message const& msg);
	void on_reach_destination(latlng const& pos);

private:
	void on_message_events(const uint8_t *payload);
	void on_message_routes(const uint8_t *payload);
	void on_message_framereq(const uint8_t *payload);

private:
	int find_and_add_sensors();
	void update_gps();
	void report_error(int errorid);
	bool send_frame(int sensorid);
	bool send_frame(sensor *sensor, uint8_t frame_type);
};

#endif //_include_client_controller_
