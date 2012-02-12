#ifndef _LZ77_DEC_H_
#define _LZ77_DEC_H_

#include <cstdio>

void decompress(signed long windowSize, signed long lookAheadSize, FILE * in, FILE * out);

#endif /* _LZ77_DEC_H_ */
