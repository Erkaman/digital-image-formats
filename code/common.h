#ifndef _COMMON_H_
#define _COMMON_H_

#include "data_list.h"
#include "print_funcs.h"
#include "data_stream.h"

#include <stdio.h>

/* To get the htonl, ntohl, htons and ntohs functions portably this is done. */
#ifdef WIN32
#include <winsock2.h>
#else
#include <netinet/in.h>
#endif

typedef struct{
    BYTE * data;
    unsigned long size;
} DataContainer;

void freeDataContainer(DataContainer data);
DataContainer allocateDataContainer(unsigned long size);

DataContainer getEmptyDataContainer(void);

DataContainer accommodateDataContainer(DataContainer data, unsigned long newSize);

BYTE readNextByte(DataContainer data);

void printData(DataContainer data);

void writeData(DataContainer data, FILE * out);

BYTE readByte(FILE * in);
void writeByte(BYTE b,FILE * out);

void readStr(FILE * in,size_t length,char * str);

void assertFile(FILE * fp,char * errorStr);
void assertFileOpened(FILE * fp);

BYTE firstNBits(BYTE num,unsigned int n);
BYTE lastNBits(BYTE num,unsigned int n);

/* change the file extension of a file name.
   If it didn't have a extension to begin with it is added.
   Do remember to the free allocated for the returned string.*/
char * changeExtension(char * fileName,char * newExtension);

char * strAppend ( char * str, const char * appendee );
char * strncpyBack( char * destination, const char * source, size_t num );


#endif /* _COMMON_H_ */
