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
    VideoCapture cap_;
public:
    webcam_opencv(int width, int height, int deviceNum, VideoCapture &cap):
            width_(width), height_(height), deviceNum_(deviceNum), cap_(cap)
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
        if (cap_.isOpened()) {
            try {
                cap_ >> frame_;
            } catch(cv::Exception e) {
                c_printf("{e}error: caught exception in frame cap\n");
            }
            if (frame_.empty()) {
                c_printf("{y}warn: {d}end of video stream (device_id=%d)\n", get_device_id());
            }
        }
    }
    virtual size_t get_frame_buffer(void **dest) override
    {
        // encode a new one
        std::vector<int> comp_params;

        c_printf("{m}debug: {d}%s device_id = %d %dx%d\n", __FUNCTION__, get_device_id(), get_width(), get_height());

        Mat temp = frame_;
        comp_params.push_back(IMWRITE_JPEG_QUALITY);
        if (get_width() > 100 && get_height() > 100) {

            resize(frame_, temp, Size(), 0.25, 0.25);
            comp_params.push_back(38);
        } else {
            comp_params.push_back(95);
        }
        if (temp.empty()) {
            return 0;
        }
        if (imencode(".jpg", temp, frame_buffer_, comp_params)) {
            *dest = (void *) &frame_buffer_[0];
            return frame_buffer_.size();
        }
        return 0;
    }
    bool is_frame_event() override
    {
        if (get_device_id() == 1) {

        }
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
    webcam_opencv *webcam = new webcam_opencv(width, height, id, cap);

    *cam = webcam;
    return kError_None;
}
#endif
