#include "sensor_thermal.h"
#include "webcam.h"

bool sensor_thermal::get_value(sensor_value &value)
{
	value.sensorid_ = id();
	value.vcount_ = 1;
	value.values_ = new sensor_value::any[value.vcount_];
	value.values_[0].pValuePtr = webcam_;
	return true;
}

void sensor_thermal::update()
{
	webcam_->capture_frame();
}

Error sensor_thermal::create_sensor(sensor **sensor)
{
	webcam *cam;
	Error err = create_webcam(&cam, 0, 140, 120);
	if (err != kError_None)
	{
		return err;
	}

	*sensor = new sensor_thermal(2, cam);
	return kError_None;
}