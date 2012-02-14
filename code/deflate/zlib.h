#ifndef _ZLIB_H_
#define _ZLIB_H_

#include <vector>
#include "../defs.h"

std::vector<BYTE> ZLIB_Decompress(const std::vector<BYTE> & data);
std::vector<BYTE> ZLIB_Compress(const std::vector<BYTE> & data);

#endif /* _ZLIB_H_ */
