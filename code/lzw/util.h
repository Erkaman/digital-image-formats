#ifndef _UTIL_H_
#define _UTIL_H_

#include <stdlib.h>


char * strAppend ( char * str, const char * appendee );
char * strncpyBack( char * destination, const char * source, size_t num );

unsigned int lastNBits(unsigned int num,unsigned int n,unsigned int bits);
unsigned int firstNBits(unsigned int num,unsigned int n);


#endif /* _UTIL_H_ */
