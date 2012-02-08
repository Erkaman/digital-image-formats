#include "io.h"
#include <cstdlib>
#include <cstdarg>

int verbose;

void assertFile(FILE * fp,const char * errorStr)
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

void readStr(FILE * in,size_t length,char * str)
{
    fread(str,sizeof(char),length,in);
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

size_t getFileSize(FILE * fp)
{
    fpos_t orig;
    size_t size;
    fgetpos(fp, &orig);
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fsetpos(fp, &orig);
    return size;

}
