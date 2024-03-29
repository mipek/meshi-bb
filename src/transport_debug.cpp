#define _USE_MATH_DEFINES
#include "transport_debug.h"
#include "cprintf.h"
#include "plat_compat.h"
#include <cmath>
#include <stdio.h>
#include <time.h>

static const float r_earth = 6371000.0f;
static const float speed_meters = 100; ///< Speed of the drone in meters

/// Degree to radians
static double to_rad(double deg) {
	return deg / 180.0f * M_PI;
}

/// Calculate distance between two positions
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

/// Current time since unix epoch (in seconds)
static uint32_t secs() {
	return (uint32_t)time(NULL);
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

	if (secs() < route_.get_start_time()) {
		// Not "allowed" to start yet
		printf("waiting for start time %d secs\n", (uint32_t)(route_.get_start_time()-secs()));
		return;
	}

	if (secs() >= dest_time_) {
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
		uint32_t travel_duration_secs = (uint32_t)(distance / speed_meters);
		if (distance < 1.0f) {
			travel_duration_secs = 0;
		}
		// Calcualte new destination time and advance route
		dest_time_ = secs() + travel_duration_secs;
		route_.advance();
		c_printf("{g}info: {d}reached destination, next dest reached in {m}%d {d}seconds (%.2f meters)\n", (uint32_t)travel_duration_secs, distance);

		// Notify listener (if any)
		if (get_listener()) {
			get_listener()->on_reach_destination(startpos_);
		}
		did_visit_ = true;
	} else {
		dest_time_ = 0;
		get_route()->reset();
		if (did_visit_) {
			did_visit_ = false;
			c_printf("{y}warn: {d}reached last destination, restarting\n");
			on_start(destpos_);
		}
	}
}

route *transport_debug::get_route() {
	return &route_;
}
