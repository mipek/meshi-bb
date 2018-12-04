#ifndef _include_controller_h_
#define _include_controller_h_

class controller
{
public:
	virtual ~controller() {};
	virtual void on_tick() =0;
};

#endif //_include_controller_h_
