#ifndef _include_public_transport_h_
#define _include_public_transport_h_

struct latlng
{
	float latitude;
	float longitude;
}

// this class specifies all possible notifications from our transport system
class transport_listener
{
public:
	virtual void on_reach_destination() =0;
};

class transport
{
public:
	virtual void set_listener(transport_listener *callback) =0;
	virtual transport_listener *get_listener() =0;
	virtual void add_destination(latlng const& dest) =0;
	virtual unsigned int get_time_to_next() =0;
}

#endif //_include_public_transport_h_
