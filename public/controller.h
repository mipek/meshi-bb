#ifndef _include_controller_h_
#define _include_controller_h_

/**
 * Describes a controller.
 */
class controller
{
public:
	virtual ~controller() {};

	/**
	 * Called once on startup.
	 */
	virtual void on_start() = 0;

	/**
	 * Called each tick to update the controller.
	 */
	virtual void on_tick() =0;

	/**
	 * Checks whether or not the controller is still active
	 */
	virtual bool is_running() const =0;
};

#endif //_include_controller_h_
