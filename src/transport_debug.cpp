#define _USE_MATH_DEFINES
#include "transport_debug.h"
#include "cprintf.h"
#include "plat_compat.h"
#include <cmath>

static const float r_earth = 6371000.0f;
static const float speed_meters = 20;

static double to_rad(double deg) {
	return deg / 180.0f * M_PI;
}

static float get_distance(latlng const& a, latlng const& b) {
	double d_lat = to_rad(b.latitude - a.latitude);
	double d_lng = to_rad(b.longitude - a.longitude);
	double d_a = sin(d_lat/2) * sin(d_lat/2) +
              cos(to_rad(a.latitude)) * cos(to_rad(b.latitude)) *
              sin(d_lng/2) * sin(d_lng/2);
	double c = 2 * atan2(sqrt(d_a), sqrt(1-d_a));
	float dist = (float) (r_earth * c);
	return dist;
}

void transport_debug::on_start(latlng const& startpos) {
	startpos_ = startpos;

	// Kick-off the traveling by simulating a reach-destination-event
	on_reach_destination();
}

void transport_debug::on_update(latlng const& curpos) {
	if (dest_time_ == 0) {
		return;
	}
	if (millis() < route_.get_start_time()) {
		// Not "allowed" to start yet
		return;
	}

	if (millis() >= dest_time_) {
		startpos_ = destpos_;
		on_reach_destination();
	}
}

bool transport_debug::get_next_destination(latlng &pos) {
	if (route_.has_next()) {
		pos = route_.get_destination();
		return true;
	}
	return false;
}

void transport_debug::on_reach_destination() {
	if (get_next_destination(destpos_)) {
		float distance = get_distance(startpos_, destpos_);
		uint64_t travel_duration_secs = (uint64_t)(distance / speed_meters);
		dest_time_ = millis() + travel_duration_secs*1000;
		route_.advance();
		c_printf("{g}info: {d}reached destination, next dest reached in {m}%d {d}seconds (%.2f meters)\n", travel_duration_secs, distance);

		if (get_listener()) {
			get_listener()->on_reach_destination(startpos_);
		}
	} else {
		c_printf("{y}warn: {d}reached last destination, restarting\n");
		dest_time_ = 0;
		get_route()->reset();
		on_start(destpos_);
	}
}

route *transport_debug::get_route() {
	return &route_;
}
