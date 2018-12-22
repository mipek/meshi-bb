#ifndef _include_plat_compat_h_
#define _include_plat_compat_h_

#include <cstdint>

// Returns current milliseconds. Start of the counter is not specified.
uint64_t millis();

void sleep_ms(int sleepMs);

#endif //_include_plat_compat_h_