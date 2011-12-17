#ifndef _IO_H_
#define _IO_H_

#include "defs.h"
#include <stdio.h>

int verbose;

BYTE readByte(FILE * in);
void writeByte(BYTE b,FILE * out);

void readStr(FILE * in,size_t length,char * str);

void assertFile(FILE * fp,char * errorStr);
void assertFileOpened(FILE * fp);

void printByte(void * byte);

void verbosePrint(const char * format, ...);

void printWarning(const char * format, ...);
void printError(const char * format, ...);


#endif /* _IO_H_ */
