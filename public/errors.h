#ifndef _include_public_errors
#define _include_public_errors

enum Error
{
	kError_None = 0,
	kError_NoSuchHostname,
	kError_SocketCreation,
	kError_SocketConnect,
	kError_Webcam
};

const char *ErrorToString(Error error);

#endif //_include_public_errors