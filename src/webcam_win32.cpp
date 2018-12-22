#include "blackbox.h"
#if PLAT == PLAT_WINDOWS
#include "webcam.h"
#include "cprintf.h"
#include <string>

// HACK
#include "../libs/escapi.cpp"

/**
 * Webcam implementation for windows
 */
class webcam_win32: public webcam
{
	int deviceNum_;
	SimpleCapParams params_;
public:
	webcam_win32(int deviceNum, int width, int height): deviceNum_(deviceNum)
	{
		params_.mWidth = width;
		params_.mHeight = height;
		params_.mTargetBuf = new int[width * height];
	}
	virtual ~webcam_win32()
	{
		delete [] params_.mTargetBuf;
	}
	virtual int get_width() const
	{
		return params_.mWidth;
	}
	virtual int get_height() const
	{
		return params_.mHeight;
	}
	virtual int get_device_id() const override
	{
		return deviceNum_;
	}
	virtual void capture_frame() override
	{
		if (isCaptureDone(get_device_id()))
		{
			// TODO: shove result somewhere
			doCapture(get_device_id());
		}
	}
	virtual size_t get_frame_buffer(void **dest) override
	{
		*dest = params_.mTargetBuf;
		return get_width() * get_height() * sizeof(int);
	}

public: /* implementation details */
	SimpleCapParams *get_impl()
	{
		return &params_;
	}
};

bool escapiInitialized = false;

static int get_desired_device()
{
	return 0;
}

Error create_webcam(webcam **cam, int id, int width, int height)
{
	if (!escapiInitialized) {
		escapiInitialized = true;
		int devices = setupESCAPI();
		if (devices < 0) {
			c_printf("{r}error: {d}ESCAPI initialization failure.\n");
			return kError_Webcam;
		} else if (devices == 0) {
			c_printf("{y}warn: {d}no UVC devices found.\n");
			return kError_Webcam;
		}
	}

	int deviceNum = get_desired_device();

	webcam_win32 *webcam = new webcam_win32(deviceNum, 140, 120);

	if (initCapture(deviceNum, webcam->get_impl()) == 0)
	{
		c_printf("{r}error: {d}couldn't initialize webcam capturing (is device already in use?).\n");
		delete webcam;
		return kError_Webcam;
	}

	// manually trigger first capture so we get valid data into our buffer
	doCapture(deviceNum);

	*cam = webcam;
	return kError_None;
}
#endif