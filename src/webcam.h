#ifndef _include_iwebcam_h_
#define _include_iwebcam_h_

#include "errors.h"

class webcam
{
public:
	virtual ~webcam() {}
	virtual int get_width() const = 0;
	virtual int get_height() const = 0;
	virtual int get_device_id() const = 0;
	virtual void capture_frame() = 0;
	virtual size_t get_frame_buffer(void **dest) = 0;
};

Error create_webcam(webcam **cam, int id, int width, int height);

#endif //_include_iwebcam_h_