#include "blackbox.h"
#if PLAT != PLAT_WINDOWS
#include <string>
#include "webcam.h"
#include "cprintf.h"
#include <opencv2/opencv.hpp>
using namespace cv;

static uint8_t *mat_to_bytes(Mat const& mat, size_t &size)
{
	size = mat.total() * mat.elemSize();
	uint8_t *bytes = new uint8_t[size];
	memcpy(bytes, mat.data(), size * sizeof(uint8_t));
}

/**
 * Webcam implementation using OpenCV
 */
class webcam_opencv: public webcam
{
	VideoCapture cap_;
	int deviceNum_;
	Mat frame_;
	uint8_t *old_buffer_;
public:
	webcam_opencv(VideoCapture &vc, int deviceNum): cap_(vc), deviceNum_(deviceNum)
	{
		capture_frame();
	}
	virtual ~webcam_opencv()
	{
		destroy_buffer();
	}
	virtual int get_width() const
	{
		return (int)cap_.get(CAP_PROP_FRAME_WIDTH);
	}
	virtual int get_height() const
	{
		return (int)cap_.get(CAP_PROP_FRAME_HEIGHT);
	}
	virtual int get_device_id() const override
	{
		return deviceNum_;
	}
	virtual void capture_frame() override
	{
		cap_ >> frame_;
		if (frame_.empty()) {
			c_printf("{y}warn: {d}end of video stream (device_id=%d)\n", get_device_id());
		}
	}
	virtual size_t get_frame_buffer(void **dest) override
	{
		// destroy the old framebuffer (if any)
		destroy_buffer();
		// allocate a new one
		size_t sz;
		old_buffer_ = mat_to_bytes(frame_, sz);
		// return data
		*dest = (void*)old_buffer_;
		return sz;
	}
private:
	void destroy_buffer()
	{
		if (old_buffer_)
		{
			delete [] old_buffer_;
			old_buffer_ = NULL;
		}
	}
};

static int get_desired_device()
{
	// TODO: make this configurable
	return 0;
}

::Error create_webcam(webcam **cam, int id, int width, int height)
{
	int deviceNum = get_desired_device();
	VideoCapture cap;
	if (!cap.open(deviceNum)) {
		c_printf("{y}warn: {d}couldn't initialize video capture (#%d)\n", deviceNum);
		return kError_Webcam;
	}

	webcam_opencv *webcam = new webcam_opencv(cap, deviceNum);

	*cam = webcam;
	return kError_None;
}
#endif
