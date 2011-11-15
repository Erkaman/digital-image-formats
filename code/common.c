#include "common.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

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

char * changeExtension(char * fileName,char * newExtension)
{
    char * changedFileName;
    int size;
    char * extensionEnd;

    extensionEnd = strrchr(fileName,'.');

    if(extensionEnd == NULL)
        extensionEnd = fileName + strlen(fileName);

    size = (extensionEnd - fileName + 1) + strlen(newExtension);

    changedFileName = (char *) malloc(size);

    strncpy(changedFileName, fileName, extensionEnd - fileName);

    changedFileName[extensionEnd- fileName] = '.';
    changedFileName[extensionEnd- fileName + 1] = '\0';

    strcat(changedFileName, newExtension);

    return changedFileName;
}

BYTE readByte(FILE * fp)
{
    BYTE s;
    fread(&s,sizeof(BYTE),1,fp);
    return s;
}

void writeByte(BYTE b,FILE * fp)
{
    fwrite(&b,sizeof(unsigned char),1,fp);
}

void readStr(FILE * fp,size_t length,char * str)
{
    fread(str,sizeof(char),length,fp);
}

BYTE firstNBits(BYTE num,unsigned int n)
{
    return num &  (~(~0 << n));
}

BYTE lastNBits(BYTE num,unsigned int n)
{
    return (num & (~0 << (8 - n))) >> (8 - n);
}

char * strAppend ( char * str, const char * appendee )
{
    char * appended;

    appended = (char *)malloc(strlen(str) + strlen(appendee));
    strcpy(appended,str);
    strcat(appended,appendee);

    return appended;
}

char * strncpyBack( char * destination, const char * source, size_t num )
{
    int skip;

    skip = strlen(source) - num;

    source += skip;

    strncpy(destination,source,num + 1);

    return destination;
}

void verbosePrint(const char * format, ...)
{
    va_list vl;

    if(verbose){
        va_start(vl, format);
	printf("verbose:%d\n",verbose);
        vprintf(format, vl);
        va_end(vl);
    }
}
