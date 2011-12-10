#include "util.h"



UNSIGNED readUnsigned(FILE * fp)
{
    UNSIGNED s;
    fread(&s,sizeof(UNSIGNED),1,fp);
    return s;
}
