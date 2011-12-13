#include "print_funcs.h"
#include <stdio.h>
#include "defs.h"
#include <stdarg.h>

void printByte(void * byte)
{
    verbosePrint("%d\n", *((BYTE *) byte));
}

void verbosePrint(const char * format, ...)
{
    va_list vl;

    if(verbose){
        va_start(vl, format);
        vprintf(format, vl);
        va_end(vl);
    }
}

void printWarning(const char * format, ...)
{
    va_list vl;

    printf("Warning: ");

    va_start(vl, format);
    vprintf(format, vl);
    va_end(vl);
}

void printError(const char * format, ...)
{
    va_list vl;

    printf("ERROR: ");

    va_start(vl, format);
    vprintf(format, vl);
    va_end(vl);
}
