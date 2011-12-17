#include "io.h"
#include <stdlib.h>
#include <stdarg.h>


void assertFile(FILE * fp,char * errorStr)
{
    if(fp == NULL){
        perror(errorStr);
        exit(1);
    }
}

void assertFileOpened(FILE * fp)
{
    assertFile(fp,"An error occurred opening the file");
}

BYTE readByte(FILE * in)
{
    BYTE s;
    fread(&s,sizeof(BYTE),1,in);
    return s;
}

void writeByte(BYTE b,FILE * out)
{
    fwrite(&b,sizeof(BYTE),1,out);
}

void readStr(FILE * in,size_t length,char * str)
{
    fread(str,sizeof(char),length,in);
}
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
