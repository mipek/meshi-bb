#include <stdio.h>
#include "blackbox.h"
#include "client_controller.h"
#include "cprintf.h"

#if PLAT == PLAT_WINDOWS
#	include "wincompat/getopt.h"
#else
#	include <unistd.h>
#endif

#define DEBUG_MASTER "127.0.0.1"
#define DEBUG_PORT 13337

extern "C" int main(int argc, char *argv[])
{
	printf("meshi-bb v1.0\n");

	ClientControllerOptions ccopts;
	int opt;
	while ((opt = getopt (argc, argv, "hs:p:i:drc")) != -1)
	{
		switch (opt)
		{
		case 'h':
			c_printf("-r use debug routes\n-d create debug sensors\n-c disable camera support\n-i <BlackBox-ID>\nDebug-Interface:\n\t-s <HOST_ADDRESS>\n\t-p <HOST_PORT>\n");
			return 0;
		case 's':
			ccopts.host = optarg;
			break;
		case 'p':
			ccopts.port = atoi(optarg);
			break;
		case 'i':
			ccopts.bbid = (uint16_t)atoi(optarg);
			break;
		case 'd':
			ccopts.dbgsensors = true;
			break;
		case 'r':
			ccopts.dbgroutes = true;
			break;
		case 'c':
			ccopts.nocameras = true;
			c_printf("{y}warn: {d}disabling cameras\n");
			break;
		}
	}

	if (ccopts.bbid <= 0) {
		c_printf("{r}error: {d}please specify a blackbox id > 0\n");
		return 1;
	}

	controller *controller = client_controller::make(ccopts);
	if (!controller) {
		return 2;
	}

	// start and begin simulating
	controller->on_start();
	while (controller->is_running()) {
		controller->on_tick();
	}

	delete controller;
	return 0;
}