#include "blackbox.h"
#include "plat_compat.h"
#if PLAT == PLAT_WINDOWS
#	include <Windows.h>
#else
#	include <unistd.h>
#	include <sys/time.h>
#endif

// Returns current milliseconds. Start of the counter is not specified.
uint64_t millis()
{
#if PLAT == PLAT_WINDOWS
	return GetTickCount64();
#else
	struct timeval tp;
	gettimeofday(&tp, NULL);
	return tp.tv_sec * 1000 + tp.tv_usec / 1000;
#endif
}

void sleep_ms(int sleepMs)
{
#if PLAT == PLAT_WINDOWS
	Sleep(sleepMs);
#else
	usleep(sleepMs * 1000);
#endif
}