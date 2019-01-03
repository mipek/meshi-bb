#ifndef _include_public_transport_h_
#define _include_public_transport_h_

#include <deque>
#include <stdint.h>

struct latlng
{
	latlng() {}
	latlng(float lat, float lng): latitude(lat), longitude(lng) {}
	float latitude;
	float longitude;
};

class route
{
	uint64_t start_time_;
	std::deque<latlng> pos_;
	std::deque<latlng>::iterator current_;
	bool repeating_;
public:
	route() {
		current_ = pos_.end();
	}
	uint64_t get_start_time() const {
		return start_time_;
	}
	latlng get_destination() const {
		return *current_;
	}
	bool has_next() {
		return !pos_.empty() && current_ != pos_.end();
	}
	void advance() {
		if (has_next()) {
			current_++;
		}
	}
	void reset() {
		current_ = pos_.begin();
	}
	void set_start_time(uint64_t time) {
		start_time_ = time;
	}
	void add_destination(float lat, float lng) {
		pos_.emplace_back(lat, lng);
	}
	void clear_destinations() {
		pos_.clear();
	}
	bool is_repeating() const { return repeating_; }
	void set_repeating(bool repeat) { repeating_ = repeat; }
};

// this class specifies all possible notifications from our transport system
class transport_listener
{
public:
	virtual void on_reach_destination() =0;
};

class transport
{
public:
	virtual void on_start(latlng const& startpos) =0;
	virtual void on_update(latlng const& curpos) =0;
	virtual void set_listener(transport_listener *callback) =0;
	virtual transport_listener *get_listener() =0;
	virtual route *get_route() =0;
};

#endif //_include_public_transport_h_
