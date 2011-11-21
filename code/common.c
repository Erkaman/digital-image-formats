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

    size = (extensionEnd - fileName + 1) + strlen(newExtension) + 1;

    verbosePrint("s1:%d\n",extensionEnd - fileName + 1);
    verbosePrint("s2:%d\n",strlen(newExtension));

    verbosePrint("s:%d\n",size);

    changedFileName = (char *) malloc(sizeof(char) * size);

    strncpy(changedFileName, fileName, extensionEnd - fileName);

    changedFileName[extensionEnd- fileName] = '.';
    changedFileName[extensionEnd- fileName + 1] = '\0';

    strcat(changedFileName, newExtension);

    return changedFileName;
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
        vprintf(format, vl);
        va_end(vl);
    }
}

void freeDataContainer(DataContainer data)
{
    if(data.data != NULL || data.size != 0){
	free(data.data);
    }
    data.data = NULL;

    data.size = 0;
}

DataContainer allocateDataContainer(unsigned long size)
{
    DataContainer data;

    data.size = size;
    data.data = (BYTE *) malloc(sizeof(BYTE) * size);

    return data;
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

BYTE readNextByte(DataContainer data)
{
    BYTE ret;

    if(data.size == 0){
	printError("common.c readNextByte: Reached end of data stream.\n");
	exit(1);
    }

    ret = *data.data;
    ++data.data;

    return ret;
}

void writeData(DataContainer data, FILE * out)
{
    unsigned long i;

    for(i = 0; i < data.size; ++i){
	putc(data.data[i],out);
    }
}

void printData(DataContainer data)
{
    unsigned long i;

    for(i = 0; i < data.size; ++i)
	verbosePrint("%d\n",data.data[i]);
}

DataContainer accommodateDataContainer(DataContainer data, unsigned long newSize)
{
    unsigned long i;
    unsigned long transfer;
    DataContainer newData;

    newData = allocateDataContainer(newSize);

    /* If all of the data in the old container fit into the new. */
    if(data.size < newSize)
	transfer = data.size;
    else
	/* If not all the data in the old container fit in the new. */
	transfer = newSize;

    for(i = 0; i < transfer; ++i)
	newData.data[i] = data.data[i];

    /* Free any memory in the previous container. */
    freeDataContainer(data);

    return newData;
}

DataContainer getEmptyDataContainer(void)
{
    DataContainer data;

    data.data = NULL;
    data.size = 0;

    return data;
}
