#ifndef _UTIL_H_
#define _UTIL_H_

#include <stdio.h>
#include "tga.h"

SHORT readShort(FILE * fp);
LONG readLong(FILE * fp);
char readChar(FILE * fp);

/* getbits:  get n bits from position p */
unsigned getbits(unsigned x, int p, int n);


#endif /* _UTIL_H_ */
