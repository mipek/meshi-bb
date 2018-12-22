#include "client_controller.h"
#include "errors.h"
#include "sensor.h"
#include "message_builder.h"
#include "transmission_debug.h"
#include "cprintf.h"
#include "sensor_temp.h"
#include "sensor_thermal.h"
#include "drivers/MAX30105.h"
#include "plat_compat.h"
#include <ctime>

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
	virtual sensor_types classify() const { return sensor_types::temperature; }
	virtual const char *name() const { return "debug"; }
	virtual bool get_value(sensor_value &value)
	{
		value.sensorid_ = id();
		value.vcount_ = 1;
		value.values_ = new sensor_value::any[value.vcount_];
		value.values_[0].iValue = 22;//rand();
		return true;
	}
	virtual bool check_value(sensor_value const& value)
	{
		return value.get_value_count() > 0;;
	}
};

/**
 * MAX30105 is a particle sensor with build-in temperature sensing capabilities.
 */
class sensor_max30105 : public sensor
{
	int id_;
	MAX30105 ps_;
public:
	sensor_max30105(int bus, uint8_t address) : ps_(bus, address)
	{
		ps_.setup(); // TODO: make params configurable?
	}
	virtual int id() const { return id_; }
	virtual sensor_types classify() const { return sensor_types::particles; }
	virtual const char *name() const { return "MAX30105"; }
	virtual bool get_value(sensor_value &value)
	{
		MAX30105::measurement msrmnt;
		if (ps_.read_sensor(msrmnt, 0))
		{
			value.sensorid_ = id();
			value.vcount_ = 4;
			value.values_ = new sensor_value::any[value.vcount_];
			value.values_[0].iValue = msrmnt.red;
			value.values_[1].iValue = msrmnt.ir;
			value.values_[2].iValue = msrmnt.green;
			value.values_[3].iValue = (int)ps_.read_temperature();
			
			return true;
		}
		return false;
	}
	virtual bool check_value(sensor_value const& value)
	{
		return value.get_value_count() >= 3;
	}
public:
	bool check_connection()
	{
		return ps_.check_connection();
	}
};

client_controller::~client_controller()
{
	destroy_all_sensors();
}

void client_controller::on_start()
{
	// announce what kind of sensors we've got
	message_builder builder;
	builder.begin_message(packet_id::c2s_sensors, packet_flags::reliable,
		bbid_, (uint32_t)time(NULL), Position(52.31f, 13.63f));

	builder.write_byte((uint8_t)get_sensor_count());
	for (size_t i = 0; i < get_sensor_count(); ++i) {
		sensor_types stype = get_sensor(i)->classify();
		builder.write_byte((uint8_t)stype);
	}

	message msg;
	builder.finalize_message(msg);
	trnsmsn_->send_message(msg);
}

void client_controller::on_tick()
{
	uint64_t now = millis();
	if (now - last_tick_ < 100)
	{
		// check if any data is available to be received
		trnsmsn_->update(this);

		// prepare measurement packet
		message_builder builder;
		builder.begin_message(packet_id::c2s_measurement, packet_flags::none,
			bbid_, (uint32_t)time(NULL), Position(52.31f, 13.63f));

		builder.write_byte(0); // EventID
		builder.write_byte((uint8_t)get_sensor_count());

		bool invalid_sensor_type = false;
		for (std::vector<sensor*>::size_type i = 0; i < sensors_.size(); ++i) {
			sensor *sensor = sensors_[i];
			sensor_value value;
			sensor->get_value(value);

			switch (sensor->classify())
			{
			case sensor_types::temperature:
				if (value.get_value_count() == 1) {
					builder.write_byte(value.get_value(0).iValue);
				} else {
					invalid_sensor_type = true;
				}
				break;
			case sensor_types::particles:
				if (value.get_value_count() == 4) {
					builder.write_byte(value.get_value(0).iValue);
					builder.write_byte(value.get_value(1).iValue);
					builder.write_byte(value.get_value(2).iValue);
					builder.write_byte(value.get_value(3).iValue);
				}
				else {
					invalid_sensor_type = true;
				}
				break;
			default:
				invalid_sensor_type = true; // sensor type currently unsupported
				break;
			}
		}

		if (!invalid_sensor_type) {
			// send measurement packet
			message msg;
			builder.finalize_message(msg);
			trnsmsn_->send_message(msg);
		}

		last_tick_ = now;
	} else {
		sleep_ms(50);
	}
}

void client_controller::on_message(message const& msg)
{
	// TODO: handle incoming messages
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
		Error error = transmission_debug::connect_to_master(
			opts.host.c_str(), opts.port, opts.bbid, &trnsmsn);
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

		controller->bbid_ = opts.bbid;
		return controller;
	} else {
		c_printf("{r}error: {d}please specify debug host and port, LoRa is not yet supported\n");
		return nullptr;
	}
}

int client_controller::find_and_add_sensors()
{
	sensor *sensor;

	int count = 0;

	// try USB thermal camera first
	if (sensor_thermal::create_sensor(&sensor) == kError_None)
	{
		register_sensor(sensor);
		++count;
	}

	// MAX30105 particle sensor
	sensor_max30105 *max30105 = new sensor_max30105(1, 0x57);
	if (max30105->check_connection())
	{
		register_sensor(max30105);
		++count;
	} else {
		delete max30105;
	}
	
	return count;
}