#include <transport.h>
#include <stddef.h>
#include <deque>

#define TIME_TO_NEXT_DEST 5000

class transport_debug: public transport
{
	transport_listener *callback_;
	std::deque<latlng> routes_;
	latlng startpos_, destpos_;
	unsigned int dest_time_;
public:
	transport_debug() {}
	virtual void on_start(latlng const& startpos);
	virtual void on_update(latlng const& curpos);
	virtual void set_listener(transport_listener *callback) { callback_ = callback; }
	virtual transport_listener *get_listener() { return callback_; }
	virtual void add_destination(latlng const& dest) { routes_.emplace_back(dest.latitude, dest.longitude); }
	virtual unsigned int get_time_to_next() { TIME_TO_NEXT_DEST; }

private:
	bool get_next_destination(latlng &out);
	void on_reach_destination();
};
