#ifndef _LZ77_ENC_H_
#define _LZ77_ENC_H_

#include <cstdio>

void compress(signed long windowSize, signed long lookAheadSize, FILE * in, FILE * out);

#endif /* _LZ77_ENC_H_ */
