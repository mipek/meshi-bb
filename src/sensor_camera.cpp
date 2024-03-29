#include "sensor_camera.h"
#include "webcam.h"

bool sensor_camera::get_value(sensor_value &value)
{
	value.sensorid_ = id();
	value.vcount_ = 1;
	value.values_ = new sensor_value::any[value.vcount_];
	value.values_[0].pValuePtr = webcam_;
	return true;
}

bool sensor_camera::check_value()
{
	return webcam_->is_frame_event();
}

void sensor_camera::update()
{
	webcam_->capture_frame();
}

Error sensor_camera::create_sensor(sensor **sensor)
{
	webcam *cam;
	Error err = create_webcam(&cam, 1, 140, 120);
	if (err != kError_None)
	{
		return err;
	}

	*sensor = new sensor_camera(4, cam);
	return kError_None;
}
