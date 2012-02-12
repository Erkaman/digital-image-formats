#ifndef _LZ77_DEC_H_
#define _LZ77_DEC_H_

#include <cstdio>

void decompress(
    unsigned int windowSize,
    unsigned int lookAheadSize,
    FILE * in,
    FILE * out);

#endif /* _LZ77_DEC_H_ */
