#include "client_controller.h"
#include "errors.h"
#include "sensor.h"
#include "transmission_debug.h"
#include "cprintf.h"
#include "sensor_temp.h"
#include "sensor_thermal.h"

/**
 * Simple debug implementation for sensors (used for mocking)
 */
class sensor_debug: public sensor
{
	friend class sensor;
	int id_;
public:
	sensor_debug(int id): id_(id)
	{
	}
public:
	virtual int id() const { return id_; }
	virtual sensor_types classify() const { return kSensorTemperature; }
	virtual void get_value(sensor_value &value)
	{
		value.sensorid_ = id();
		value.vcount_ = 1;
		value.values_ = new sensor_value::any[value.vcount_];
		value.values_[0].iValue = 36;
	}
	virtual bool check_value(sensor_value const& value)
	{
		return value.get_value_count() > 0;;
	}
};

client_controller::~client_controller()
{
	destroy_all_sensors();
}

void client_controller::on_tick()
{
	//char data[2] = {0x42, 0x43};
	//trnsmsn_->send(data, 2);

	for (std::vector<sensor*>::size_type i = 0; i < sensors_.size(); ++i) {
		sensor *sensor = sensors_[i];
		sensor_value value;
		sensor->get_value(value);
	}
}

void client_controller::destroy_all_sensors()
{
	for (std::vector<sensor*>::size_type i = 0; i < sensors_.size(); ++i) {
		sensor *sensor = sensors_[i];
		delete sensor;
	}
	sensors_.clear();
}

controller *client_controller::make(ClientControllerOptions const& opts)
{
	transmission *trnsmsn;
	if (!opts.host.empty() && opts.port > 0) {
		c_printf("connecting to %s:%d...\n", opts.host.c_str(), opts.port);
		Error error = transmission_debug::connect_to_master(opts.host.c_str(), opts.port, &trnsmsn);
		if (error != kError_None)
		{
			c_printf("{r}error: {d}%s\n", ErrorToString(error));
			return nullptr;
		}
		c_printf("successfully connected!\n");

		client_controller *controller = new client_controller(trnsmsn);
		if (opts.dbgsensors)
		{
			c_printf("{g}info: {d}adding debug sensor(s)\n");

			sensor_debug *sensor = new sensor_debug(1);
			controller->register_sensor(sensor);
		}

		int foundSenorCount = controller->find_and_add_sensors();
		c_printf("{g}info: {d}found {y}%d {d}sensors\n", foundSenorCount);

		return controller;
	} else {
		c_printf("{r}error: {d}please specify debug host and port, LoRa is not yet supported\n");
		return nullptr;
	}
}

int client_controller::find_and_add_sensors()
{
	sensor *sensor;

	// try USB thermal camera first
	if (sensor_thermal::create_sensor(&sensor) == kError_None)
	{
		register_sensor(sensor);
	}
}