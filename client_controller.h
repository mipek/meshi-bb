#include "public/controller.h"
#include <string>

class transmission;

struct ClientControllerOptions
{
	ClientControllerOptions(): port(0), bbid(0)
	{
	}
	std::string host;
	int port;
	int bbid;
};

class client_controller: public controller
{
	transmission *trnsmsn_;
public:
	client_controller(transmission *trnsmsn): trnsmsn_(trnsmsn)
	{
	}
	void on_tick();

	static controller *make(ClientControllerOptions const& opts);
};