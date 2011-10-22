#include "util.h"
#include <stdarg.h>

unsigned int firstNBits(unsigned int num,unsigned int n)
{
    return num &  (~(~0 << (n)));
}

void debugPrint(const char * format, ...)
{
    va_list vl;

    if(DEBUG){
        va_start(vl, format);
        vprintf(format, vl);
        va_end(vl);
    }
}

UNSIGNED readUnsigned(FILE * fp)
{
    UNSIGNED s;
    fread(&s,sizeof(UNSIGNED),1,fp);
    return s;
}
