#include "sensor_thermal.h"
#include "webcam.h"

Error sensor_thermal::create_sensor(sensor **sensor)
{
	webcam *cam;
	Error err = create_webcam(&cam, 0, 140, 120);
	if (err != kError_None)
	{
		return err;
	}

	*sensor = new sensor_thermal(1, cam);
	return kError_None;
}