#include "client_controller.h"
#include "errors.h"
#include "sensor.h"
#include "message_builder.h"
#include "transmission_debug.h"
#include "transport_debug.h"
#include "cprintf.h"
#include "sensor_temp.h"
#include "sensor_thermal.h"
#include "sensor_camera.h"
#include "drivers/MAX30105.h"
#include "plat_compat.h"
#include "minmea/minmea.h"
#include "webcam.h"
#include <ctime>

#ifndef min_value
#define min_value(a,b)			(((a) < (b)) ? (a) : (b))
#endif

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
		value.values_[0].iValue = 42;//rand();
		return true;
	}
	virtual bool check_value()
	{
		return false;
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
	sensor_max30105(int bus, uint8_t address) : id_(3), ps_(bus, address)
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
	virtual bool check_value()
	{
		sensor_value v;
		if (get_value(v) && v.vcount_ == 4) {
			return v.values_[0].iValue > 18000 && v.values_[1].iValue > 25000 && v.values_[2].iValue > 1000;
		}
		return false;
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
	if (gpsuart_) {
		fclose(gpsuart_);
		gpsuart_ = NULL;
	}
}

void client_controller::register_sensor(sensor *s)
{
	sensors_.push_back(s);
	sensor_types type = s->classify();
	if (type != sensor_types::thermal && type != sensor_types::color) {
		sensor_count_++;
	}
}

void client_controller::on_start()
{
	transport_->set_listener(this);

	// get GPS data via UART
	// TODO: disabled
	//gpsuart_ = fopen("/dev/ttyS0", "rt");

	// announce what kind of sensors we've got
	announce_sensors();
}

void client_controller::on_tick()
{
	uint64_t now = millis();
	if (now - last_tick_ > 1000)
	{
		update_gps();

		// check if any data is available to be received
		trnsmsn_->update(this);

		transport_->on_update(latlng(lat_, lng_));

		// TODO: this is for debugging only, send measurements every now and then
		//on_reach_destination(latlng(lat_, lng_));

		for (size_t i = 0; i < sensors_.size(); ++i) {
			get_sensor(i)->update();
		}

		last_tick_ = now;
	} else {
		sleep_ms(50);
	}
}

void client_controller::on_message(message const& msg)
{
	const uint8_t *payload = msg.get_s2c_payload();
	switch (msg.get_packet_id()) {
	case packet_id::s2c_sensorreq:
		announce_sensors();
		break;
	case packet_id::s2c_routes:
		on_message_routes(payload);
		break;
	case packet_id::s2c_framereq:
		on_message_framereq(payload);
		break;
	}
}

void client_controller::on_reach_destination(latlng const& pos)
{
	// prepare measurement packet
	uint32_t curtime = (uint32_t)time(NULL);
	message_builder builder;
	builder.begin_message(packet_id::c2s_measurement, packet_flags::none,
		bbid_, curtime, position(pos.latitude, pos.longitude));
	lat_ = pos.latitude;
	lng_ = pos.longitude;

	uint8_t event_id = 0;
	for (std::vector<sensor*>::size_type i = 0; i < sensors_.size(); ++i) {
		if (sensors_[i]->check_value()) {
			c_printf("{r}ATTENTION! ENVIRONMENT EVENT WAS DETECTED\n");
			event_id = 1;
			break;
		}
	}

	builder.write_byte(event_id); // EventID
	builder.write_byte((uint8_t)get_real_sensor_count());

	c_printf("{m}debug: {d}sending measurement (%d sensors in packet)\n", get_real_sensor_count());
	for (std::vector<sensor*>::size_type i = 0; i < sensors_.size(); ++i) {
		sensor *sensor = sensors_[i];
		sensor->update();

		sensor_value value;
		sensor->get_value(value);

		switch (sensor->classify())
		{
		case sensor_types::temperature:
			if (value.get_value_count() == 1) {
				builder.write_byte((uint8_t)sensor->id());

				builder.write_byte(value.get_value(0).iValue);
			}
			break;
		case sensor_types::particles:
			if (value.get_value_count() == 4) {
				builder.write_byte((uint8_t)sensor->id());

				builder.write_byte(value.get_value(0).iValue);
				builder.write_byte(value.get_value(1).iValue);
				builder.write_byte(value.get_value(2).iValue);
				builder.write_byte(value.get_value(3).iValue);
			}
			break;
		default:
			break;
		}
		c_printf("{m}debug: {d}sensor %d - %s (sensorid = %d)\n", i, sensor->name(), sensor->id());
	}

	// send measurement packet
	message msg;
	builder.finalize_message(msg);
	trnsmsn_->send_message(msg);

	// automatically send image frame when we detect a event
	if (event_id != 0) {
		if (!send_frame(2, curtime)) { // Thermal
			send_frame(4, curtime); // RGB
		}
	}
}

void client_controller::on_message_events(const uint8_t *payload)
{
	uint8_t ecount = *payload++;
	for (uint8_t i = 0; i < ecount; ++i) {
		uint8_t eventid = *(uint8_t*)(payload++);
	}
}

void client_controller::on_message_routes(const uint8_t *payload)
{
	transport *transport = get_transport();
	if (transport) {
		route *route = transport->get_route();

		uint8_t rcount = *payload++;
		uint32_t stime = *(uint32_t*)(payload);
		payload += sizeof(uint32_t);

		if (stime == 0) {
			// if we receive a faulty time set start time to "now"
			route->set_start_time((uint32_t)time(NULL));
		} else {
			route->set_start_time(stime);
		}
		printf("received route star time: %d, currenet: %d\n", stime, time(NULL));
		route->clear_destinations();
		for (uint8_t i = 0; i < rcount; ++i) {
			float lat = *(float*)payload;
			payload += sizeof(float);
			float lng = *(float*)payload;
			payload += sizeof(float);
			float alt = *(float*)payload;
			payload += sizeof(float);
			printf(" route[%d] = %f, %f, %f\n", i, lat, lng, alt);
			route->add_destination(lat, lng);
		}

		// first destination is starting point
		if (rcount > 0) {
			route->reset();
			transport->on_start(route->get_destination());
		}
	}
}

void client_controller::on_message_framereq(const uint8_t *payload)
{
	send_frame(*payload);
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
		// establish connection
		c_printf("connecting to %s:%d...\n", opts.host.c_str(), opts.port);
		Error error = transmission_debug::connect_to_master(
			opts.host.c_str(), opts.port, opts.bbid, &trnsmsn);
		if (error != kError_None)
		{
			c_printf("{r}error: {d}%s\n", ErrorToString(error));
			return nullptr;
		}
		c_printf("successfully connected!\n");

		// create client controller and add debug sensors (if neccessary)
		client_controller *controller = new client_controller(trnsmsn);
		if (opts.dbgsensors)
		{
			c_printf("{g}info: {d}adding debug sensor(s)\n");

			sensor_debug *sensor = new sensor_debug(1);
			controller->register_sensor(sensor);
		}

		// create transport interface
		transport *trans = new transport_debug();
		controller->set_transport(trans);
		float p1[2] = {52.460707f, 13.503987f};
		if (opts.dbgroutes)
		{
			// debug routes
			float p2[2] = {52.458865f, 13.506573f}; //270m
			float p3[2] = {52.452931f, 13.515043f}; //350m
			route *r = trans->get_route();
			r->set_start_time(time(NULL) + 10);
			r->add_destination(p1[0], p1[1]);
			r->add_destination(p2[0], p2[1]);
			r->add_destination(p3[0], p3[1]);
			r->reset();
			trans->on_start(latlng(p1[0], p1[1]));
		}

		// initialize sensors
		int found_senor_count = controller->find_and_add_sensors(opts.nocameras);
		c_printf("{g}info: {d}found {y}%d {d}sensors\n", found_senor_count);

		controller->bbid_ = opts.bbid;
		return controller;
	} else {
		c_printf("{r}error: {d}please specify debug host and port, LoRa is not yet supported\n");
		return nullptr;
	}
}

/// Checks whether or not the I2C-Bus at the specified port is accessible.
static bool check_i2c_availability(int bus)
{
	i2c_master i2c(bus);
	return i2c.is_valid();
}

int client_controller::find_and_add_sensors(bool no_cameras)
{
	sensor *sensor;

	int count = 0;

	if (!no_cameras)
	{
		// try USB thermal camera first
		if (sensor_thermal::create_sensor(&sensor) == kError_None)
		{
			register_sensor(sensor);
			++count;
		}
		// real webcam
		if (sensor_camera::create_sensor(&sensor) == kError_None)
		{
			register_sensor(sensor);
			++count;
		}
	}

	const int i2c_bus = 1;
	if (check_i2c_availability(i2c_bus))
	{
		// MAX30105 particle sensor
		sensor_max30105 *max30105 = new sensor_max30105(i2c_bus, 0x57);
		if (max30105->check_connection())
		{
			register_sensor(max30105);
			++count;
		} else {
			delete max30105;
		}
	} else {
		c_printf("{y}warn: {d}could not open i2c bus #%d for reading, skipping i2c sensors\n", i2c_bus);
	}
	
	return count;
}

void client_controller::update_gps()
{
	if (!gpsuart_) return;

	// parse NMEA data from the GPS chip
	char line[MINMEA_MAX_LENGTH];
	while (fgets(line, sizeof(line), gpsuart_) != NULL) {
		switch (minmea_sentence_id(line, false)) {
			case MINMEA_SENTENCE_RMC: {
				struct minmea_sentence_rmc frame;
				if (minmea_parse_rmc(&frame, line)) {
					c_printf("{m}debug: {d}$RMC floating point degree coordinates and speed: (%f,%f) %f\n",
							minmea_tocoord(&frame.latitude),
							minmea_tocoord(&frame.longitude),
							minmea_tofloat(&frame.speed));

					// store latitude and longitude
					lat_ = minmea_tocoord(&frame.latitude);
					lng_ = minmea_tocoord(&frame.longitude);
				}
			} break;
		}
	}
}

/// Translates sensor type to frame type
static uint8_t sensor_type_to_frame_type(sensor_types type)
{
	switch (type)
	{
		case sensor_types::thermal:
			return 2;
		case sensor_types::color:
			return 0;
		default:
			return 0xff;
	}
}

void client_controller::report_error(int errorid)
{
	// build error message and send it to the server
	message_builder builder;
	builder.begin_message(packet_id::c2s_malfunction, packet_flags::reliable,
						  bbid_, (uint32_t)time(NULL), position(lat_, lng_));

	builder.write_short(errorid);

	message msg;
	builder.finalize_message(msg);
	trnsmsn_->send_message(msg);
}

bool client_controller::send_frame(int sensorid, uint32_t etime)
{
	for (std::vector<sensor*>::size_type i = 0; i < sensors_.size(); ++i) {
		sensor *sensor = sensors_[i];
		if (sensor->id() == sensorid) {
			uint8_t frame_type = sensor_type_to_frame_type(sensor->classify());
			if (frame_type != 0xff) {
				return send_frame(sensor, frame_type, etime);
			} else {
				c_printf(
						"{r}error: specified sensor with id=%d is not a camera/thermal sensor (name: %s, classify: %d)\n",
						sensorid, sensor->name(), sensor->classify());
			}
		}
	}
	return false;
}

bool client_controller::send_frame(sensor *sensor, uint8_t frame_type, uint32_t etime)
{
	sensor_value value;
	if (sensor->get_value(value)) {
		webcam *wc = (webcam*)(value.get_value(0).pValuePtr);
		int width = wc->get_width();
		int height = wc->get_height();

		// get frame buffer from camera sensor (thermal, rgb, grayscale)
		uint8_t *bytes;
		int64_t size = (int64_t)wc->get_frame_buffer((void**)&bytes);
		if (size > 0) {
			message_builder builder;
			builder.begin_message(packet_id::c2s_frame, packet_flags::fragmented,
								  bbid_, (uint32_t) time(NULL), position(lat_, lng_));
			builder.write_byte(frame_type);
			builder.write_dword(width);
			builder.write_dword(height);
			builder.write_dword(etime);
			for (int64_t i = 0; i < size; ++i) {
				builder.write_byte(bytes[i]); // image data
			}

			message msg;
			builder.finalize_message(msg);
			trnsmsn_->send_message(msg);

			c_printf("{m}debug: {d}sending frame (%d bytes)\n", size);
			return true;
		} else {
			c_printf("{r}error: {d}couldn't get frame buffer from %s\n", sensor->name());
		}
	} else {
		c_printf("{r}error: {d}failed to access sensor values (%s)\n", sensor->name());
	}
	return false;
}

void client_controller::announce_sensors()
{
	message_builder builder;
	builder.begin_message(packet_id::c2s_sensors, packet_flags::reliable,
						  bbid_, (uint32_t)time(NULL), position(lat_, lng_));

	builder.write_byte((uint8_t)sensors_.size());
	c_printf("{m}debug: {d}sensor count: %d (announce)\n", sensors_.size());
	for (size_t i = 0; i < sensors_.size(); ++i) {
		sensor *sensor = get_sensor(i);
		builder.write_byte((uint8_t)sensor->id());
		builder.write_byte((uint8_t)sensor->classify());
		c_printf("{m}debug: {d}sensor: %s id=%d (announce)\n", sensor->name(), sensor->id());
	}

	message msg;
	builder.finalize_message(msg);
	trnsmsn_->send_message(msg);
}