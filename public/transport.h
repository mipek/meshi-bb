#ifndef _include_public_transport_h_
#define _include_public_transport_h_

#include <deque>
#include <stdint.h>

/** 
 * Position that is identified by latitude and longitude
 */
struct latlng
{
	latlng() {}
	latlng(float lat, float lng): latitude(lat), longitude(lng) {}
	float latitude; ///< Latitude
	float longitude; ///< Longitude
};

/**
 * Encapsulates a route that consists of multiple positions and a starting time.
 */
class route
{
	uint32_t start_time_;
	std::deque<latlng> pos_;
	std::deque<latlng>::iterator current_;
	bool repeating_;
public:
	route(): repeating_(true) {
		current_ = pos_.end();
	}
	uint32_t get_start_time() const {
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
	void set_start_time(uint32_t time) {
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

/**
 * Interface that specifies all possible notifications from our transport system
 */
class transport_listener
{
public:
	/**
	 * Called when the underlying transport interface implementation wants to notify that a destination has been reached.
	 * @param pos Position of the reached destination.
	 */
	virtual void on_reach_destination(latlng const& pos) =0;
};

/**
 * Describes the transport interface.
 */
class transport
{
public:
	/**
	 * Called on startup.
	 * @param startpos current position
	 */
	virtual void on_start(latlng const& startpos) =0;

	/**
	 * Called each tick to update the transport interface(/implementation)
	 * @param curpos current position.
	 */
	virtual void on_update(latlng const& curpos) =0;

	/**
	 * Set the ::transport_listener object
	 * @param callback listener object
	 */
	virtual void set_listener(transport_listener *callback) =0;

	/**
	 * Return the currently active ::transport_listener object
	 * @return active listener
	 */
	virtual transport_listener *get_listener() =0;

	/**
	 * Returns the currently active route. A valid route object is always returned even if the route is empty.
	 */
	virtual route *get_route() =0;
};

#endif //_include_public_transport_h_
