#ifndef _LZ77_ENC_H_
#define _LZ77_ENC_H_

#include <cstdio>

void compress(
    unsigned int windowSize,
    unsigned int lookAheadSize,
    FILE * in,
    FILE * out);

#endif /* _LZ77_ENC_H_ */
