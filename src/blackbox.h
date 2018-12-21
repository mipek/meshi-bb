#ifndef _include_blackbox_h_
#define _include_blackbox_h_

#define PLAT_POSIX 1
#define PLAT_WINDOWS 2

#if defined(_WIN32)
#	define PLAT PLAT_WINDOWS
#	if !defined(WIN32_LEAN_AND_MEAN)
#		define WIN32_LEAN_AND_MEAN
#	endif //WIN32_LEAN_AND_MEAN
#else
#	define PLAT PLAT_POSIX
#endif

#endif //_include_blackbox_h_