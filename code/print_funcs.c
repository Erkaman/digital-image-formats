#include "print_funcs.h"
#include <stdio.h>
#include "defs.h"

void printByte(void * byte)
{
    verbosePrint("%d\n", *((BYTE *) byte));
}
