#include "util.h"

unsigned int firstNBits(unsigned int num,unsigned int n)
{
    return num &  (~(~0 << (n)));
}


UNSIGNED readUnsigned(FILE * fp)
{
    UNSIGNED s;
    fread(&s,sizeof(UNSIGNED),1,fp);
    return s;
}
