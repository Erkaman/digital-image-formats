#include "print_funcs.h"
#include <stdio.h>
#include "../common.h"

void printByte(void * byte)
{
    printf("%d\n", *((BYTE *) byte));
}
