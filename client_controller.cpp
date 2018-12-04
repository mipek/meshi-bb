#include "client_controller.h"
#include "public/errors.h"
#include "transmission_debug.h"
#include "cprintf.h"

void client_controller::on_tick()
{
	char data[2] = {0x42, 0x43};
	trnsmsn_->send(data, 2);
}

controller *client_controller::make(ClientControllerOptions const& opts)
{
	transmission *trnsmsn;
	if (!opts.host.empty() && opts.port > 0) {
		c_printf("connecting to %s:%d...\n", opts.host.c_str(), opts.port);
		Error error = transmission_debug::connect_to_master(opts.host.c_str(), opts.port, &trnsmsn);
		if (error != kError_None)
		{
			printf("error: %s\n", ErrorToString(error));
			return nullptr;
		}
		c_printf("successfully connected!\n");
		return new client_controller(trnsmsn);
	} else {
		c_printf("{r}error: {d}please specify debug host and port, LoRa is not yet supported\n");
		return nullptr;
	}
}