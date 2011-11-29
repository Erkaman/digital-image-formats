#include "print_funcs.h"
#include <stdio.h>
#include "../common.h"

void printByte(void * byte)
{
    verbosePrint("%d\n", *((BYTE *) byte));
}
