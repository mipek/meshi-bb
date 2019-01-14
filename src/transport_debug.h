#include <transport.h>
#include <stddef.h>
#include <stdint.h>
#include <deque>

class transport_debug: public transport
{
	transport_listener *callback_;
	route route_;
	latlng startpos_, destpos_;
	uint64_t dest_time_;
	bool did_visit_;
public:
	transport_debug(): callback_(NULL), dest_time_(0), did_visit_(true)
	{
	}
	virtual void on_start(latlng const& startpos);
	virtual void on_update(latlng const& curpos);
	virtual void set_listener(transport_listener *callback) { callback_ = callback; }
	virtual transport_listener *get_listener() { return callback_; }
	virtual route *get_route();

private:
	bool get_next_destination(latlng &out);
	void on_reach_destination();
};
