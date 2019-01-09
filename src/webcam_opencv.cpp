#include "blackbox.h"
#if PLAT != PLAT_WINDOWS
#include <string>
#include "webcam.h"
#include "cprintf.h"
#include <opencv2/opencv.hpp>
#include <vector>
using namespace cv;

/**
 * Webcam implementation using OpenCV
 */
class webcam_opencv: public webcam
{
	VideoCapture cap_;
	int deviceNum_;
	Mat frame_;
    std::vector<uint8_t> frame_buffer_;
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
        // free old framebuffer (if any)
        destroy_buffer();
        // encode a new one
        if (imencode(".jpg", frame_, frame_buffer_)) {
            *dest = (void *) &frame_buffer_[0];
            return frame_buffer_.size();
        }
        return 0;
	}
private:
	void destroy_buffer()
	{
		frame_buffer_.clear();
	}
};

::Error create_webcam(webcam **cam, int id, int width, int height)
{
	VideoCapture cap;
	if (!cap.open(id)) {
		c_printf("{y}warn: {d}couldn't initialize video capture (#%d)\n", id);
		return kError_Webcam;
	}

	webcam_opencv *webcam = new webcam_opencv(cap, id);

	*cam = webcam;
	return kError_None;
}
#endif
