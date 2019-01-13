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
	int width_, height_;
	int deviceNum_;
	Mat frame_;
    std::vector<uint8_t> frame_buffer_;
public:
	webcam_opencv(int width, int height, int deviceNum): width_(width), height_(height), deviceNum_(deviceNum)
	{
		capture_frame();
	}
	virtual ~webcam_opencv()
	{
		destroy_buffer();
	}
	virtual int get_width() const
	{
		return width_;//(int)cap_.get(CAP_PROP_FRAME_WIDTH);
	}
	virtual int get_height() const
	{
		return height_;//(int)cap_.get(CAP_PROP_FRAME_HEIGHT);
	}
	virtual int get_device_id() const override
	{
		return deviceNum_;
	}
	virtual void capture_frame() override
	{
	    VideoCapture cap;
	    if (cap.open(deviceNum_)) {
		//printf(" %d capturing frame... \n", get_device_id());
	        cap >> frame_;
		//printf(" done!\n");
            if (frame_.empty()) {
                c_printf("{y}warn: {d}end of video stream (device_id=%d)\n", get_device_id());
            }
	        cap.release();
	    }
		//resize(temp, frame_, Size(temp.cols/2, temp.rows/2));
	}
	virtual size_t get_frame_buffer(void **dest) override
	{
        // free old framebuffer (if any)
        //destroy_buffer()
        // encode a new one
		std::vector<int> comp_params;
		comp_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
		comp_params.push_back(9);
        if (imencode(".jpg", frame_, frame_buffer_)) {
            *dest = (void *) &frame_buffer_[0];
            return frame_buffer_.size();
        }
        return 0;
	}

	bool is_frame_event() override
	{
		return false;
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

    width = (int) cap.get(CAP_PROP_FRAME_WIDTH);
	height = (int)cap.get(CAP_PROP_FRAME_HEIGHT);
	cap.release();
	webcam_opencv *webcam = new webcam_opencv(width, height, id);

	*cam = webcam;
	return kError_None;
}
#endif
