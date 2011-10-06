#include "util.h"
#include <string.h>

char * strAppend ( char * str, const char * appendee )
{
    char * appended;

    appended = (char *)malloc(strlen(str) + strlen(appendee));
    strcpy(appended,str);
    strcat(appended,appendee);

    return appended;
}

char * strncpyBack( char * destination, const char * source, size_t num )
{
    int skip;

    skip = strlen(source) - num;

    source += skip;

    strncpy(destination,source,num + 1);

    return destination;
}

unsigned int lastNBits(unsigned int num,unsigned int n,unsigned int bits)
{
    return (num & (~0 << (bits - n))) >> (bits - n);
}

unsigned int firstNBits(unsigned int num,unsigned int n)
{
    return (num & ~(~0 << n));
}
