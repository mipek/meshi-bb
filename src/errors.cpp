#include "errors.h"

const char *ErrorToString(Error error)
{
	switch(error)
	{
		case kError_None:
			return "no error";
		case kError_NoSuchHostname:
			return "no such hostname";
		case kError_SocketCreation:
			return "socket creation failed (debug)";
		case kError_SocketConnect:
			return "socket connect failed (debug)";
	}
	return "<unknown error>";
}