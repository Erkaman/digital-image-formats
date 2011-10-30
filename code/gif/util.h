#ifndef _UTIL_H_
#define _UTIL_H_

#include "gif.h"

#define DEBUG 1

unsigned int firstNBits(unsigned int num,unsigned int n);

void debugPrint(const char * format, ...);

UNSIGNED readUnsigned(FILE * fp);

#endif /* _UTIL_H_ */
