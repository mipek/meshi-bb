#ifndef _include_crc32_h_
#define _include_crc32_h_

#include <cstdint>
#include <cstddef>

void crc32(const void *data, size_t n_bytes, uint32_t *crc);

#endif //_include_crc32_h_