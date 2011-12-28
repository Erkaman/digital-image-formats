#ifndef _BITWISE_H_
#define _BITWISE_H_

#include "defs.h"

BYTE firstNBits(BYTE num,unsigned int n);
BYTE lastNBits(BYTE num,unsigned int n, int bits);
int getbits(int b, int start, int end);

#endif /* _BITWISE_H_ */
