#include "util.h"

unsigned getbits(unsigned x, int p, int n)
{
    return (x >> (p+1-n)) & ~(~0 << n);
}

SHORT readShort(FILE * fp)
{
    SHORT s;
    fread(&s,sizeof(SHORT),1,fp);
    return s;
}

LONG readLong(FILE * fp)
{
    LONG s;
    fread(&s,sizeof(LONG),1,fp);
    return s;
}

char readChar(FILE * fp)
{
    char s;
    fread(&s,sizeof(char),1,fp);
    return s;
}

 void readStr(FILE * fp,size_t length,char * str)
{
    fread(str,sizeof(char),length,fp);
}
